#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "rhi/base/rhi_shader.h"
#include "rhi/base/rhi_program.h"
#include "rhi/base/rhi_render_view.h"
#include "effect/technique.h"
#include "components/camera_component.h"
#include "scene_manager/scene_manager.h"
#include <vector>

SEEK_NAMESPACE_BEGIN

// ============================================================================
// GpuCullingManager — 管理 GPU 端视锥体剔除全流程
//
// 每帧数据流：
//   1. UploadObjectData() → CPU 填充所有对象的 world matrix / AABB / meshDataIndex
//   2. Cull() → Dispatch FrustumCullingCS，GPU 端 AABB vs 视锥体检测
//   3. 输出：visibleIndexBuffer（紧凑可见索引列表）+ visibleCounter（可见数量）
//      供后续 GenerateIndirectArgsCS 使用
// ============================================================================
class GpuCullingManager
{
public:
    explicit GpuCullingManager(Context* context);
    ~GpuCullingManager() = default;

    // 初始化 CS 所需的 shader/program/technique（在 RHI 准备好后调用一次）
    SResult Init();

    // 每帧开始时，重置可见计数器
    void ResetVisibleCounter();

    // 从场景的 MeshPair 列表填充 ObjectData buffer
    // 仅上传已在 GpuMeshRegistry 中注册的 mesh
    void UploadObjectData(const std::vector<MeshPair>& meshPairs);

    // 执行 GPU 视锥体剔除
    void Cull(CameraComponent* camera);

    // 根据可见对象列表生成 Indirect Draw 参数
    void GenerateIndirectArgs();

    // 执行 Indirect Draw（按格式分组批量绘制）
    void ExecuteIndirectDraws();

    // === 查询接口 ===

    uint32_t GetObjectCount() const { return m_objectCount; }

    // GPU 端 buffer（供后续 CS 阶段消费）
    RHIGpuBufferPtr GetObjectDataBuffer()    const { return m_objectDataBuffer; }
    RHIGpuBufferPtr GetVisibleIndexBuffer()  const { return m_visibleIndexBuffer; }
    RHIGpuBufferPtr GetVisibleCounterBuffer() const { return m_visibleCounterBuffer; }

private:
    // 从 CameraComponent 提取 6 个视锥体平面
    static void ExtractFrustumPlanes(CameraComponent* camera, float4 outPlanes[6]);

    Context* m_pContext = nullptr;

    // GPU Buffer
    RHIGpuBufferPtr m_objectDataBuffer;      // StructedBuffer<GPUObjectData>，SRV + UAV
    RHIGpuBufferPtr m_visibleIndexBuffer;    // RWStructuredBuffer<uint>，UAV
    RHIGpuBufferPtr m_visibleCounterBuffer;  // RWStructuredBuffer<uint>，UAV，单元素
    RHIGpuBufferPtr m_frustumParamsCB;       // FrustumCullingParams constant buffer
    RHIGpuBufferPtr m_indirectArgsBuffer;    // CS 写入：StructuredBuffer<DrawIndexedIndirectArgs>
    RHIGpuBufferPtr m_drawIndirectBuffer;    // 实际绘制：带 DRAW_INDIRECT_ARGS 标志的 buffer
    RHIGpuBufferPtr m_indirectArgsCB;        // GenerateIndirectArgsCS 的常量缓冲区（g_ObjectCount + g_IndexStride）

    // UAV / SRV 视图（不能直接传 RHIGpuBufferPtr 给 SetParam，类型不匹配会导致静默失败）
    RHIUnorderedAccessViewPtr   m_objectDataBufferUav;     // FrustumCullingCS u0
    RHIUnorderedAccessViewPtr   m_visibleIndexBufferUav;   // FrustumCullingCS u1
    RHIUnorderedAccessViewPtr   m_visibleCounterBufferUav; // FrustumCullingCS u2
    RHIUnorderedAccessViewPtr   m_indirectArgsBufferUav;   // GenerateIndirectArgsCS u0
    RHIShaderResourceViewPtr    m_objectDataBufferSrv;     // GenerateIndirectArgsCS t0
    RHIShaderResourceViewPtr    m_meshDataBufferSrv;       // GenerateIndirectArgsCS t1（从 GpuMeshRegistry 的 buffer 创建）
    RHIGpuBufferPtr             m_cachedMeshDataBuffer;     // 用于检测 meshDataBuffer 是否已更新，避免每帧重建 SRV

    // Shader / Technique
    VirtualTechnique*  m_pCullingVirtualTech = nullptr;
    Technique*         m_pCullingTechnique = nullptr;
    VirtualTechnique*  m_pIndirectArgsVirtualTech = nullptr;
    Technique*         m_pIndirectArgsTechnique = nullptr;

    // CPU 侧映射：objectIndex → registryIndex（UploadObjectData 时填充）
    std::vector<uint32_t> m_objectRegIndices;
    // CPU 侧映射：objectIndex → MeshPair（供 ExecuteIndirectDraws 访问 MeshComponent 和 mesh）
    std::vector<MeshPair> m_objectMeshPairs;

    uint32_t m_objectCount = 0;
};

CLASS_PTR_UNIQUE(GpuCullingManager)
SEEK_NAMESPACE_END
