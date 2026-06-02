#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_definition.h"
#include <vector>
#include <unordered_map>

SEEK_NAMESPACE_BEGIN

// GPU 端数据结构定义（由 CMake 全局 SEEK_CPP 宏控制 C++ 构造函数可见性）
#include "shader/GPUCulling.slangh"

// ============================================================================
// GpuMeshEntry — 单个静态 Mesh 在合并缓冲区中的元数据
// ============================================================================
struct GpuMeshEntry
{
    RHIMesh*        mesh = nullptr;        // 原始 RHIMesh 指针（用于绑定时查找）
    uint32_t        formatGroup = 0;       // 所属顶点格式分组
    uint32_t        vertexByteOffset = 0;  // 统一 VB 中的字节偏移
    uint32_t        indexByteOffset = 0;   // 统一 IB 中的字节偏移
    uint32_t        vertexCount = 0;       // 顶点数量
    uint32_t        indexCount = 0;        // 索引数量
    uint32_t        vertexStride = 0;      // 顶点步长（字节）
    uint32_t        indexStride = 0;       // 索引步长（2=UInt16, 4=UInt32）
    uint32_t        materialIndex = 0;     // 材质索引
    MeshTopologyType topologyType = MeshTopologyType::Triangles;
    IndexBufferType  indexType = IndexBufferType::UInt16;
    AABBox          aabbLocal;
};

// ============================================================================
// GpuFormatGroup — 共享同一顶点格式的一组 Mesh 的合并缓冲区
// ============================================================================
struct GpuFormatGroup
{
    uint64_t        formatHash = 0;         // 顶点格式哈希
    uint32_t        totalVertexBytes = 0;   // 合并 VB 的总字节数
    uint32_t        totalIndexBytes = 0;    // 合并 IB 的总字节数
    uint32_t        indexStride = 0;        // 索引步长（2 或 4）
    RHIGpuBufferPtr unifiedVB = nullptr;    // 合并后的顶点缓冲
    RHIGpuBufferPtr unifiedIB = nullptr;    // 合并后的索引缓冲
    std::vector<uint32_t> entryIndices;     // 该分组中的 GpuMeshEntry 索引
};

// ============================================================================
// GpuMeshRegistry — 管理合并后的顶点/索引缓冲和 GPU 端 MeshData
//
// 用途：Phase 2 的核心组件，负责：
//   1. 收集场景中所有静态 mesh，按顶点格式分组
//   2. 将同一分组的顶点/索引数据合并到连续的统一缓冲区
//   3. 生成 StructuredBuffer<GPUMeshData> 供 GPU Compute Shader 访问
//   4. 提供从 RHIMesh* 到 registry index 的反向映射
// ============================================================================
class GpuMeshRegistry
{
public:
    explicit GpuMeshRegistry(Context* context);
    ~GpuMeshRegistry() = default;

    // 注册一个静态 mesh（仅在 mesh 加载时调用一次）
    // 返回 registry 中的索引
    uint32_t RegisterStaticMesh(RHIMeshPtr mesh);

    // 在所有 mesh 注册完毕后调用，构建合并缓冲区和 GPU 数据
    void Build();

    // 清空所有已注册数据（用于场景重载）
    void Clear();

    // === 查询接口 ===

    // Mesh 总数
    uint32_t GetMeshCount() const { return static_cast<uint32_t>(m_entries.size()); }

    // 格式分组数量（即需要多少次 DrawIndexedInstancedIndirect 调用）
    uint32_t GetFormatGroupCount() const { return static_cast<uint32_t>(m_formatGroups.size()); }

    // 获取某个格式分组
    const GpuFormatGroup& GetFormatGroup(uint32_t index) const { return m_formatGroups[index]; }

    // 获取某个 MeshEntry
    const GpuMeshEntry& GetMeshEntry(uint32_t index) const { return m_entries[index]; }

    // 从 RHIMesh* 反查 registry index
    uint32_t GetMeshRegistryIndex(RHIMesh* mesh) const;

    // 获取 GPUMeshData 的 GPU Buffer（StructuredBuffer）
    RHIGpuBufferPtr GetMeshDataBuffer() const { return m_meshDataBuffer; }

    // 检查是否已构建
    bool IsBuilt() const { return m_built; }

private:
    // 计算顶点格式哈希（基于 VertexStream 的 layout 描述）
    static uint64_t ComputeVertexFormatHash(RHIMeshPtr mesh);

    Context* m_pContext = nullptr;
    bool m_built = false;

    std::vector<GpuMeshEntry>  m_entries;
    std::vector<GpuFormatGroup> m_formatGroups;
    std::unordered_map<RHIMesh*, uint32_t> m_meshToIndex;
    std::unordered_map<uint64_t, uint32_t> m_hashToGroup;  // formatHash → groupIndex

    // GPU 端数据缓冲
    RHIGpuBufferPtr m_meshDataBuffer;  // StructuredBuffer<GPUMeshData>
};

CLASS_PTR_UNIQUE(GpuMeshRegistry)

SEEK_NAMESPACE_END
