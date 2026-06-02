#include "effect/gpu_mesh_registry.h"
#include "kernel/context.h"
#include "resource/resource_mgr.h"
#include "utils/log.h"
#include "math/aabbox.h"
#include <cstring>

#include "effect/gpu_culling_types.h"

#define SEEK_MACRO_FILE_UID 85     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

GpuMeshRegistry::GpuMeshRegistry(Context* context)
    : m_pContext(context)
{
}

uint64_t GpuMeshRegistry::ComputeVertexFormatHash(RHIMeshPtr mesh)
{
    // 基于顶点流布局计算哈希值
    // 哈希因子：流数量、每个流的 stride、layout（usage, format, buffer_offset, usage_index）
    VertexAttributeResource& attrRes = mesh->GetVertexAttributeResource();

    uint64_t hash = 0;
    auto hash_combine = [&hash](uint64_t val)
    {
        hash ^= val + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
    };

    hash_combine(attrRes._vertexStreams.size());

    for (auto& vs : attrRes._vertexStreams)
    {
        hash_combine(vs.stride);
        hash_combine(vs.is_instance_stream ? 1ULL : 0ULL);
        for (auto& layout : vs.layouts)
        {
            hash_combine(static_cast<uint64_t>(layout.usage));
            hash_combine(static_cast<uint64_t>(layout.format));
            hash_combine(layout.buffer_offset);
            hash_combine(layout.usage_index);
            hash_combine(layout.is_instance_attrib ? 1ULL : 0ULL);
        }
    }

    // 索引格式也纳入分组（UInt16 和 UInt32 不能混用）
    IndexBufferType ibType = mesh->GetIndexBufferType();
    hash_combine(static_cast<uint64_t>(ibType));

    return hash;
}

uint32_t GpuMeshRegistry::RegisterStaticMesh(RHIMeshPtr mesh)
{
    if (!mesh || !mesh->IsUseIndices())
    {
        LOG_WARNING("GpuMeshRegistry::RegisterStaticMesh: invalid mesh or no index buffer");
        return UINT32_MAX;
    }

    // 只支持单顶点流的静态 mesh（多流 mesh 走传统渲染路径）
    VertexAttributeResource& attrRes = mesh->GetVertexAttributeResource();
    if (attrRes._vertexStreams.size() != 1)
    {
        LOG_WARNING("GpuMeshRegistry::RegisterStaticMesh: mesh has %zu vertex streams, only 1 is supported for GPU-driven path",
            attrRes._vertexStreams.size());
        return UINT32_MAX;
    }

    uint64_t formatHash = ComputeVertexFormatHash(mesh);

    // 查找或创建格式分组
    uint32_t groupIndex = 0;
    auto hashIt = m_hashToGroup.find(formatHash);
    if (hashIt == m_hashToGroup.end())
    {
        groupIndex = static_cast<uint32_t>(m_formatGroups.size());
        GpuFormatGroup group;
        group.formatHash = formatHash;
        group.totalVertexBytes = 0;
        group.totalIndexBytes = 0;

        // 确定索引步长
        IndexBufferType ibType = mesh->GetIndexBufferType();
        group.indexStride = (ibType == IndexBufferType::UInt32) ? 4U : 2U;

        m_formatGroups.push_back(group);
        m_hashToGroup[formatHash] = groupIndex;
    }
    else
    {
        groupIndex = hashIt->second;
    }

    GpuFormatGroup& group = m_formatGroups[groupIndex];

    // 计算该 mesh 在当前分组中的偏移
    VertexStream& vs = attrRes._vertexStreams[0];
    BufferResource& vbBuf = *attrRes._vertexBuffers[0];
    uint32_t vertexByteSize = static_cast<uint32_t>(vbBuf._size);
    uint32_t vertexCount = vertexByteSize / vs.stride;

    VertexIndicesResource& indicesRes = mesh->GetVertexIndicesResource();
    uint32_t indexByteSize = static_cast<uint32_t>(indicesRes._size);
    uint32_t indexCount = indicesRes._indexCount;

    // 创建 entry
    GpuMeshEntry entry;
    entry.mesh             = mesh.get();
    entry.formatGroup      = groupIndex;
    entry.vertexByteOffset = group.totalVertexBytes;
    entry.indexByteOffset  = group.totalIndexBytes;
    entry.vertexCount      = vertexCount;
    entry.indexCount       = indexCount;
    entry.vertexStride     = vs.stride;
    entry.indexStride      = group.indexStride;
    entry.materialIndex    = 0;  // 后续阶段填充
    entry.topologyType     = mesh->GetTopologyType();
    entry.indexType        = mesh->GetIndexBufferType();
    entry.aabbLocal        = mesh->GetAABBox();

    uint32_t entryIndex = static_cast<uint32_t>(m_entries.size());
    m_entries.push_back(entry);
    m_meshToIndex[mesh.get()] = entryIndex;
    group.entryIndices.push_back(entryIndex);

    // 累加分组总大小
    group.totalVertexBytes += vertexByteSize;
    group.totalIndexBytes  += indexByteSize;

    LOG_INFO("GpuMeshRegistry: registered mesh [%u] in format group %u (hash=0x%llX), vtx=%u idx=%u",
        entryIndex, groupIndex, formatHash, vertexCount, indexCount);

    return entryIndex;
}

void GpuMeshRegistry::Build()
{
    if (m_entries.empty())
    {
        LOG_WARNING("GpuMeshRegistry::Build: no meshes registered");
        m_built = false;  // 没有注册任何 mesh，标记为未构建，避免后续 GPU Driven 路径使用空指针
        return;
    }

    RHIContext& rc = m_pContext->RHIContextInstance();

    // === Step 1: 为每个格式分组构建合并后的 VB 和 IB ===
    for (auto& group : m_formatGroups)
    {
        if (group.entryIndices.empty())
            continue;

        // 分配 CPU 暂存内存
        std::vector<uint8_t> stagingVB(group.totalVertexBytes);
        std::vector<uint8_t> stagingIB(group.totalIndexBytes);

        // 逐个复制每个 mesh 的数据到暂存缓冲区
        for (uint32_t entryIdx : group.entryIndices)
        {
            GpuMeshEntry& entry = m_entries[entryIdx];
            RHIMesh* mesh = entry.mesh;

            // 复制顶点数据
            VertexAttributeResource& attrRes = mesh->GetVertexAttributeResource();
            BufferResource& vbBuf = *attrRes._vertexBuffers[0];
            uint32_t vbSize = static_cast<uint32_t>(vbBuf._size);
            std::memcpy(stagingVB.data() + entry.vertexByteOffset, vbBuf._data, vbSize);

            // 复制索引数据
            VertexIndicesResource& indicesRes = mesh->GetVertexIndicesResource();
            uint32_t ibSize = static_cast<uint32_t>(indicesRes._size);
            std::memcpy(stagingIB.data() + entry.indexByteOffset, indicesRes._data, ibSize);
        }

        // 创建合并后的 GPU buffer
        {
            RHIGpuBufferData vbData(group.totalVertexBytes, stagingVB.data());
            group.unifiedVB = rc.CreateVertexBuffer(group.totalVertexBytes, &vbData);
        }
        {
            RHIGpuBufferData ibData(group.totalIndexBytes, stagingIB.data());
            group.unifiedIB = rc.CreateIndexBuffer(group.totalIndexBytes, &ibData);
        }

        LOG_INFO("GpuMeshRegistry: format group %zu built — VB=%u bytes, IB=%u bytes, %zu meshes",
            &group - &m_formatGroups[0], group.totalVertexBytes, group.totalIndexBytes, group.entryIndices.size());
    }

    // === Step 2: 填充并上传 GPUMeshData buffer ===
    {
        std::vector<GPUMeshData> meshDataVec(m_entries.size());
        for (size_t i = 0; i < m_entries.size(); i++)
        {
            GpuMeshEntry& entry = m_entries[i];
            GPUMeshData& gpuData = meshDataVec[i];

            gpuData.indexOffset      = entry.indexByteOffset;
            gpuData.vertexOffset     = entry.vertexByteOffset / entry.vertexStride;  // 转换为元素偏移
            gpuData.indexCount       = entry.indexCount;
            gpuData.vertexCount      = entry.vertexCount;
            gpuData.materialIndex    = entry.materialIndex;
            gpuData.topologyType     = static_cast<uint32_t>(entry.topologyType);
            gpuData.indexBufferType  = (entry.indexType == IndexBufferType::UInt32) ? 1U : 0U;
            gpuData.vertexStride     = entry.vertexStride;
            gpuData.aabbMinLocal     = float4(entry.aabbLocal.Min().x(), entry.aabbLocal.Min().y(),
                                               entry.aabbLocal.Min().z(), 0.0f);
            gpuData.aabbMaxLocal     = float4(entry.aabbLocal.Max().x(), entry.aabbLocal.Max().y(),
                                               entry.aabbLocal.Max().z(), 0.0f);
        }

        RHIGpuBufferData meshDataBuf(sizeof(GPUMeshData) * meshDataVec.size(), meshDataVec.data());
        m_meshDataBuffer = rc.CreateGpuBuffer(
            static_cast<uint32_t>(sizeof(GPUMeshData) * meshDataVec.size()),
            RESOURCE_FLAG_GPU_STRUCTURED | RESOURCE_FLAG_GPU_READ,
            sizeof(GPUMeshData),
            &meshDataBuf
        );

        LOG_INFO("GpuMeshRegistry: MeshData buffer created — %zu entries, %u bytes",
            meshDataVec.size(), static_cast<uint32_t>(sizeof(GPUMeshData) * meshDataVec.size()));
    }

    m_built = true;
}

void GpuMeshRegistry::Clear()
{
    m_entries.clear();
    m_formatGroups.clear();
    m_meshToIndex.clear();
    m_hashToGroup.clear();
    m_meshDataBuffer.reset();
    m_built = false;
}

uint32_t GpuMeshRegistry::GetMeshRegistryIndex(RHIMesh* mesh) const
{
    auto it = m_meshToIndex.find(mesh);
    if (it != m_meshToIndex.end())
        return it->second;
    return UINT32_MAX;
}

SEEK_NAMESPACE_END
