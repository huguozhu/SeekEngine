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
#include <cstring>

#include "rhi/d3d11/d3d11_gpu_buffer.h"
#include "rhi/d3d11/d3d11_context.h"

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

    // 更新 CPU 侧对象→Registry 映射
    m_objectRegIndices.resize(m_objectCount);

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

    m_pCullingTechnique->SetParam("objectDataBuffer", m_objectDataBuffer);
    m_pCullingTechnique->SetParam("visibleIndexBuffer", m_visibleIndexBuffer);
    m_pCullingTechnique->SetParam("visibleCounter", m_visibleCounterBuffer);
    m_pCullingTechnique->SetParam("cb_FrustumCullingParams", m_frustumParamsCB);

    uint32_t threadGroups = (m_objectCount + 63) / 64;
    m_pCullingTechnique->Dispatch(threadGroups, 1, 1);
}

void GpuCullingManager::GenerateIndirectArgs()
{
    if (!m_pIndirectArgsTechnique || m_objectCount == 0)
        return;

    GpuMeshRegistry& registry = m_pContext->GpuMeshRegistryInstance();

    // 获取索引步长
    uint32_t indexStride = 4;
    if (registry.GetFormatGroupCount() > 0)
    {
        indexStride = registry.GetFormatGroup(0).indexStride;
    }

    // 上传常量（g_ObjectCount + g_IndexStride）
    struct {
        uint32_t objectCount;
        uint32_t indexStride;
        uint32_t padding[2];
    } argsParams;
    argsParams.objectCount = m_objectCount;
    argsParams.indexStride = indexStride;

    // 绑定参数
    m_pIndirectArgsTechnique->SetParam("objectDataBuffer", m_objectDataBuffer);
    m_pIndirectArgsTechnique->SetParam("meshDataBuffer", registry.GetMeshDataBuffer());
    m_pIndirectArgsTechnique->SetParam("indirectArgsBuffer", m_indirectArgsBuffer);

    // 每个线程处理一个对象，64 线程一组
    uint32_t groupCount = (m_objectCount + 63) / 64;
    m_pIndirectArgsTechnique->Dispatch(groupCount, 1, 1);

    LOG_INFO("GpuCullingManager::GenerateIndirectArgs: dispatched %u groups for %u objects", groupCount, m_objectCount);
}

void GpuCullingManager::ExecuteIndirectDraws()
{
    if (!m_drawIndirectBuffer || m_objectCount == 0)
        return;

    GpuMeshRegistry& registry = m_pContext->GpuMeshRegistryInstance();
    if (!registry.IsBuilt())
        return;

    RHIContext& rc = m_pContext->RHIContextInstance();

    // Step 1: CopyResource — 将 CS 生成的 args 复制到带 DRAW_INDIRECT_ARGS 标志的 buffer
    {
        D3D11Context& d3d11rc = static_cast<D3D11Context&>(rc);
        ID3D11DeviceContext* d3dCtx = d3d11rc.GetD3D11DeviceContext();
        D3D11GpuBuffer* src = static_cast<D3D11GpuBuffer*>(m_indirectArgsBuffer.get());
        D3D11GpuBuffer* dst = static_cast<D3D11GpuBuffer*>(m_drawIndirectBuffer.get());
        d3dCtx->CopyResource(dst->GetD3DBuffer(), src->GetD3DBuffer());
    }

    // Step 2: 按格式分组绘制
    // 每个格式分组绑定其统一的 VB/IB，然后对组内所有对象执行 DrawIndexedInstancedIndirect
    // 被 GPU 剔除的对象 args.indexCountPerInstance=0，D3D11 自动跳过
    uint32_t formatGroupCount = registry.GetFormatGroupCount();

    for (uint32_t fgIdx = 0; fgIdx < formatGroupCount; fgIdx++)
    {
        const GpuFormatGroup& group = registry.GetFormatGroup(fgIdx);
        if (!group.unifiedVB || !group.unifiedIB)
            continue;

        // 绑定该格式分组的统一 VB/IB
        // TODO: 需要通过 D3D11Mesh 设置 InputLayout，Phase 6 实现
        // 当前仅验证 pipeline 完整性

        // 遍历组内所有 mesh entry，对应查找对象
        for (uint32_t entryIdx : group.entryIndices)
        {
            const GpuMeshEntry& meshEntry = registry.GetMeshEntry(entryIdx);

            // 在 m_objectRegIndices 中查找使用此 mesh 的对象
            for (uint32_t objIdx = 0; objIdx < m_objectCount; objIdx++)
            {
                if (m_objectRegIndices[objIdx] != entryIdx)
                    continue;

                // 执行单个对象的间接绘制
                uint32_t argsOffset = objIdx * sizeof(DrawIndexedIndirectArgs);
                // D3D11 的 DrawIndexedInstancedIndirect 需要 VB/IB/InputLayout 已绑定
                // Phase 6: 通过 Technique 绑定完整的渲染状态
                (void)argsOffset; // 抑制未使用警告
            }
        }
    }

    LOG_INFO("GpuCullingManager::ExecuteIndirectDraws: processed %u format groups, %u objects",
        formatGroupCount, m_objectCount);
}

SEEK_NAMESPACE_END
