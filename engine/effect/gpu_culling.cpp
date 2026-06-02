#include "effect/gpu_culling.h"
#include "effect/gpu_mesh_registry.h"
#include "effect/effect.h"
#include "kernel/context.h"
#include "scene_manager/scene_manager.h"
#include "components/mesh_component.h"
#include "components/camera_component.h"
#include "math/frustum.h"
#include "math/plane.h"
#include "utils/log.h"
#include <algorithm>
#include <cstring>

#include "rhi/d3d11/d3d11_gpu_buffer.h"
#include "rhi/d3d11/d3d11_context.h"
#include "effect/scene_renderer.h"

#include "effect/gpu_culling_types.h"

#define SEEK_MACRO_FILE_UID 84     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

GpuCullingManager::GpuCullingManager(Context* context)
    : m_pContext(context)
{
}

SResult GpuCullingManager::Init()
{
    Effect& effect = m_pContext->EffectInstance();

    // FrustumCullingCS
    m_pCullingVirtualTech = effect.GetVirtualTechnique("FrustumCulling");
    if (!m_pCullingVirtualTech)
    {
        LOG_ERROR("GpuCullingManager::Init: FrustumCulling VirtualTechnique not found");
        return ERR_INVALID_ARG;
    }
    m_pCullingTechnique = m_pCullingVirtualTech->Concrete();
    if (!m_pCullingTechnique)
    {
        LOG_ERROR("GpuCullingManager::Init: failed to concrete FrustumCulling technique");
        return ERR_INVALID_ARG;
    }

    // GenerateIndirectArgsCS
    m_pIndirectArgsVirtualTech = effect.GetVirtualTechnique("GenerateIndirectArgs");
    if (!m_pIndirectArgsVirtualTech)
    {
        LOG_ERROR("GpuCullingManager::Init: GenerateIndirectArgs VirtualTechnique not found");
        return ERR_INVALID_ARG;
    }
    m_pIndirectArgsTechnique = m_pIndirectArgsVirtualTech->Concrete();
    if (!m_pIndirectArgsTechnique)
    {
        LOG_ERROR("GpuCullingManager::Init: failed to concrete GenerateIndirectArgs technique");
        return ERR_INVALID_ARG;
    }

    LOG_INFO("GpuCullingManager::Init: techniques ready");
    return S_Success;
}

void GpuCullingManager::ResetVisibleCounter()
{
    if (!m_visibleCounterBuffer || !m_pContext)
        return;

    uint32_t zero = 0;
    m_visibleCounterBuffer->Update(&zero, sizeof(uint32_t));
}

void GpuCullingManager::UploadObjectData(const std::vector<MeshPair>& meshPairs)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    GpuMeshRegistry& registry = m_pContext->GpuMeshRegistryInstance();

    if (meshPairs.empty())
    {
        m_objectCount = 0;
        return;
    }

    m_objectCount = static_cast<uint32_t>(meshPairs.size());

    // 更新 CPU 侧映射
    m_objectRegIndices.resize(m_objectCount);
    m_objectMeshPairs = meshPairs;  // 保存副本供 ExecuteIndirectDraws 使用

    // 分配/扩容 GPUObjectData buffer
    uint32_t bufferSize = static_cast<uint32_t>(sizeof(GPUObjectData) * m_objectCount);
    bool needRecreate = !m_objectDataBuffer || m_objectDataBuffer->GetSize() < bufferSize;

    if (needRecreate)
    {
        m_objectDataBuffer = rc.CreateGpuBuffer(bufferSize,
            RESOURCE_FLAG_GPU_STRUCTURED | RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_UAV,
            sizeof(GPUObjectData));
        m_visibleIndexBuffer = rc.CreateGpuBuffer(
            static_cast<uint32_t>(sizeof(uint32_t) * m_objectCount),
            RESOURCE_FLAG_GPU_STRUCTURED | RESOURCE_FLAG_UAV,
            sizeof(uint32_t));
        m_visibleCounterBuffer = rc.CreateGpuBuffer(sizeof(uint32_t),
            RESOURCE_FLAG_GPU_STRUCTURED | RESOURCE_FLAG_UAV,
            sizeof(uint32_t));
        m_frustumParamsCB = rc.CreateConstantBuffer(sizeof(FrustumCullingParams),
            RESOURCE_FLAG_CPU_WRITE);
        // Indirect Args Buffer：CS 写入用的 StructuredBuffer<DrawIndexedIndirectArgs>
        m_indirectArgsBuffer = rc.CreateGpuBuffer(
            static_cast<uint32_t>(sizeof(DrawIndexedIndirectArgs) * m_objectCount),
            RESOURCE_FLAG_GPU_STRUCTURED | RESOURCE_FLAG_UAV,
            sizeof(DrawIndexedIndirectArgs));
        // 实际绘制用的 indirect buffer：带 DRAW_INDIRECT_ARGS 标志（不能与 STRUCTURED 共存）
        m_drawIndirectBuffer = rc.CreateGpuBuffer(
            static_cast<uint32_t>(sizeof(DrawIndexedIndirectArgs) * m_objectCount),
            RESOURCE_FLAG_DRAW_INDIRECT_ARGS | RESOURCE_FLAG_UAV,
            0);
        // GenerateIndirectArgsCS 的常量缓冲区：g_ObjectCount + g_IndexStride
        m_indirectArgsCB = rc.CreateConstantBuffer(16, RESOURCE_FLAG_CPU_WRITE);

        // 创建 UAV / SRV 视图（SetParam 需要正确的类型，直接传 RHIGpuBufferPtr 会静默失败）
        m_objectDataBufferUav = rc.CreateBufferUav(m_objectDataBuffer, PixelFormat::Unknown, 0, m_objectCount);
        m_visibleIndexBufferUav = rc.CreateBufferUav(m_visibleIndexBuffer, PixelFormat::Unknown, 0, m_objectCount);
        m_visibleCounterBufferUav = rc.CreateBufferUav(m_visibleCounterBuffer, PixelFormat::Unknown, 0, 1);
        m_objectDataBufferSrv = rc.CreateBufferSrv(m_objectDataBuffer, PixelFormat::Unknown, 0, m_objectCount);
        m_indirectArgsBufferUav = rc.CreateBufferUav(m_indirectArgsBuffer, PixelFormat::Unknown, 0, m_objectCount);
    }

    // CPU 侧填充 ObjectData
    std::vector<GPUObjectData> cpuData(m_objectCount);
    for (uint32_t i = 0; i < m_objectCount; i++)
    {
        MeshComponent* comp = meshPairs[i].first;
        uint32_t meshIdx = meshPairs[i].second;
        RHIMeshPtr mesh = comp->GetMeshByIndex(meshIdx);
        if (!mesh)
            continue;

        GPUObjectData& obj = cpuData[i];

        // 世界矩阵
        Matrix4 worldMat = comp->GetWorldMatrix();
        obj.worldMatrix = worldMat.Transpose();

        // 法线矩阵（逆转置）
        Matrix4 normalMat = worldMat.Inverse();
        obj.normalMatrix = normalMat;

        // 世界空间 AABB
        AABBox worldBox = mesh->GetAABBoxWorld();
        obj.aabbMinWorld = float4(worldBox.Min().x(), worldBox.Min().y(), worldBox.Min().z(), 0.0f);
        obj.aabbMaxWorld = float4(worldBox.Max().x(), worldBox.Max().y(), worldBox.Max().z(), 0.0f);

        // Mesh 数据索引
        uint32_t regIndex = registry.GetMeshRegistryIndex(mesh.get());
        obj.meshDataIndex = (regIndex != UINT32_MAX) ? regIndex : 0;
        obj.visible = 0;

        // 存储 CPU 侧映射（供 ExecuteIndirectDraws 查找格式分组）
        m_objectRegIndices[i] = obj.meshDataIndex;
    }

    // 上传到 GPU
    m_objectDataBuffer->Update(cpuData.data(), bufferSize);
}

void GpuCullingManager::ExtractFrustumPlanes(CameraComponent* camera, float4 outPlanes[6])
{
    Matrix4 clipMatrix = camera->GetViewProjMatrix();

    float4 col1 = clipMatrix.Col(0);
    float4 col2 = clipMatrix.Col(1);
    float4 col3 = clipMatrix.Col(2);
    float4 col4 = clipMatrix.Col(3);

    float4 rawPlanes[6];
    rawPlanes[0] = col4 - col1;
    rawPlanes[1] = col4 + col1;
    rawPlanes[2] = col4 - col2;
    rawPlanes[3] = col4 + col2;
    rawPlanes[4] = col4 - col3;
    rawPlanes[5] = col3;

    for (int i = 0; i < 6; i++)
    {
        float len = sqrtf(rawPlanes[i].x() * rawPlanes[i].x() +
                          rawPlanes[i].y() * rawPlanes[i].y() +
                          rawPlanes[i].z() * rawPlanes[i].z());
        outPlanes[i] = rawPlanes[i] / len;
    }
}

void GpuCullingManager::Cull(CameraComponent* camera)
{
    if (!m_pCullingTechnique || m_objectCount == 0)
        return;

    ResetVisibleCounter();

    FrustumCullingParams params;
    params.objectCount = m_objectCount;
    params.totalVisibleCount = 0;
    ExtractFrustumPlanes(camera, params.frustumPlanes);
    m_frustumParamsCB->Update(&params, sizeof(FrustumCullingParams));

    m_pCullingTechnique->SetParam("objectDataBuffer", m_objectDataBufferUav);
    m_pCullingTechnique->SetParam("visibleIndexBuffer", m_visibleIndexBufferUav);
    m_pCullingTechnique->SetParam("visibleCounter", m_visibleCounterBufferUav);
    m_pCullingTechnique->SetParam("cb_FrustumCullingParams", m_frustumParamsCB);

    uint32_t threadGroups = (m_objectCount + 63) / 64;
    m_pCullingTechnique->Dispatch(threadGroups, 1, 1);
}

void GpuCullingManager::GenerateIndirectArgs()
{
    if (!m_pIndirectArgsTechnique || m_objectCount == 0)
        return;

    GpuMeshRegistry& registry = m_pContext->GpuMeshRegistryInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();

    // 获取索引步长
    uint32_t indexStride = 4;
    if (registry.GetFormatGroupCount() > 0)
    {
        indexStride = registry.GetFormatGroup(0).indexStride;
    }

    // 为 GpuMeshRegistry 的 meshDataBuffer 创建 SRV（如果尚未创建或 buffer 已更新）
    RHIGpuBufferPtr meshBuf = registry.GetMeshDataBuffer();
    if (!meshBuf)
    {
        LOG_ERROR("GpuCullingManager::GenerateIndirectArgs: meshDataBuffer is null, skip");
        return;
    }
    if (!m_meshDataBufferSrv || meshBuf != m_cachedMeshDataBuffer)
    {
        m_meshDataBufferSrv = rc.CreateBufferSrv(meshBuf, PixelFormat::Unknown, 0,
            static_cast<uint32_t>(meshBuf->GetSize() / sizeof(GPUMeshData)));
        m_cachedMeshDataBuffer = meshBuf;
    }

    // 上传常量到 GPU 常量缓冲区（仅 g_ObjectCount，indexStride 改为 per-mesh 从 GPUMeshData 读取）
    struct {
        uint32_t objectCount;
        uint32_t padding[3];
    } argsParams;
    argsParams.objectCount = m_objectCount;
    argsParams.padding[0] = 0;
    argsParams.padding[1] = 0;
    argsParams.padding[2] = 0;
    m_indirectArgsCB->Update(&argsParams, sizeof(argsParams));

    // 绑定参数（使用正确的 SRV / UAV 视图类型，而非 RHIGpuBufferPtr）
    m_pIndirectArgsTechnique->SetParam("objectDataBuffer", m_objectDataBufferSrv);
    m_pIndirectArgsTechnique->SetParam("meshDataBuffer", m_meshDataBufferSrv);
    m_pIndirectArgsTechnique->SetParam("indirectArgsBuffer", m_indirectArgsBufferUav);
    m_pIndirectArgsTechnique->SetParam("cb_IndirectArgsParams", m_indirectArgsCB);

    // 每个线程处理一个对象，64 线程一组
    uint32_t groupCount = (m_objectCount + 63) / 64;
    m_pIndirectArgsTechnique->Dispatch(groupCount, 1, 1);

}

void GpuCullingManager::ExecuteIndirectDraws()
{
    if (!m_drawIndirectBuffer || m_objectCount == 0)
        return;

    GpuMeshRegistry& registry = m_pContext->GpuMeshRegistryInstance();
    if (!registry.IsBuilt())
        return;

    RHIContext& rc = m_pContext->RHIContextInstance();
    SceneRenderer& sceneRenderer = m_pContext->SceneRendererInstance();

    // Step 1: CopyResource — 将 CS 生成的 args 复制到带 DRAW_INDIRECT_ARGS 标志的 buffer
    {
        D3D11Context& d3d11rc = static_cast<D3D11Context&>(rc);
        ID3D11DeviceContext* d3dCtx = d3d11rc.GetD3D11DeviceContext();
        D3D11GpuBuffer* src = static_cast<D3D11GpuBuffer*>(m_indirectArgsBuffer.get());
        D3D11GpuBuffer* dst = static_cast<D3D11GpuBuffer*>(m_drawIndirectBuffer.get());
        d3dCtx->CopyResource(dst->GetD3DBuffer(), src->GetD3DBuffer());
    }

    // Step 2: 收集所有有效对象的绘制信息并解析 Technique
    struct DrawEntry
    {
        uint32_t        objIdx;
        Technique*      tech;
        MeshComponent*  comp;
        RHIMeshPtr      mesh;
    };
    std::vector<DrawEntry> entries;
    entries.reserve(m_objectCount);

    for (uint32_t objIdx = 0; objIdx < m_objectCount; objIdx++)
    {
        MeshComponent* comp = m_objectMeshPairs[objIdx].first;
        uint32_t meshIdx = m_objectMeshPairs[objIdx].second;
        RHIMeshPtr mesh = comp->GetMeshByIndex(meshIdx);
        if (!mesh)
            continue;

        Technique* tech = nullptr;
        SResult ret = sceneRenderer.GetEffectTechniqueToRender(mesh, &tech);
        if (SEEK_CHECKFAILED(ret) || !tech)
            continue;

        entries.push_back({ objIdx, tech, comp, mesh });
    }

    if (entries.empty())
        return;

    // Step 3: 将已注册 mesh 的 VB/IB 临时替换为统一缓冲区（绘制后恢复，不影响传统渲染路径）
    registry.SwapToUnifiedBuffers();

    // Step 3: 按 Technique 指针排序，将相同 Shader/Material 的对象聚集为批次
    // VirtualTechnique::Concrete 对相同 predefines 返回同一指针，天然分组
    std::sort(entries.begin(), entries.end(),
        [](const DrawEntry& a, const DrawEntry& b) {
            return a.tech < b.tech;
        });

    // Step 4: 分批绘制
    // 同组首对象：完整 OnRenderBegin（设置光照/材质/阴影等共享状态）
    // 同组后续：仅 UpdateModelInfo（世界矩阵），跳过 90% 的重复参数绑定
    Technique* currentTech = nullptr;
    for (const auto& entry : entries)
    {
        if (entry.tech != currentTech)
        {
            currentTech = entry.tech;
            // 新批次：完整设置所有制参数
            entry.comp->OnRenderBegin(currentTech, entry.mesh);
        }
        else
        {
            // 同批次：仅更新 ModelInfo 常量缓冲区（世界矩阵）
            entry.comp->UpdateModelInfo(currentTech, entry.mesh);
        }

        // 执行间接绘制（参数来自 GPU buffer，被剔除对象 indexCount=0，D3D11 自动跳过）
        uint32_t argsOffset = entry.objIdx * sizeof(DrawIndexedIndirectArgs);
        currentTech->DrawIndexedIndirect(m_drawIndirectBuffer, entry.mesh, argsOffset);
    }

    // Step 5: 恢复原始 VB/IB，保证传统渲染路径后续正确
    registry.RestoreOriginalBuffers();
}

SEEK_NAMESPACE_END
