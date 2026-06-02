//STAGE:cs

#include "Common.slangh"
#include "GPUCulling.slangh"

// 输入常量
cbuffer cb_IndirectArgsParams : register(b6)
{
    uint   g_ObjectCount;   // 对象总数
    uint   g_Pad0;
    uint   g_Pad1;
    uint   g_Pad2;
};

// 输入：每对象动态数据（含 FrustumCullingCS 写入的 visible 标志）
StructuredBuffer<GPUObjectData> objectDataBuffer : register(t0);

// 输入：静态网格元数据
StructuredBuffer<GPUMeshData> meshDataBuffer : register(t1);

// 输出：间接绘制参数（每个对象一个 entry，对齐 D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS）
RWStructuredBuffer<DrawIndexedIndirectArgs> indirectArgsBuffer : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint objIndex = dtid.x;
    if (objIndex >= g_ObjectCount)
        return;

    // 读取对象数据和可见性标志
    GPUObjectData objData = objectDataBuffer[objIndex];

    // 读取网格元数据
    GPUMeshData meshData = meshDataBuffer[objData.meshDataIndex];

    // 索引步长取每个 mesh 自带的 indexBufferType（0=UInt16=2字节, 1=UInt32=4字节）
    uint indexStride = (meshData.indexBufferType == 1) ? 4 : 2;

    DrawIndexedIndirectArgs args;

    if (objData.visible != 0)
    {
        // 可见对象：填充正常的绘制参数
        args.indexCountPerInstance = meshData.indexCount;
        args.instanceCount         = 1;
        args.startIndexLocation    = meshData.indexOffset / indexStride;
        args.baseVertexLocation    = (int)meshData.vertexOffset;
        args.startInstanceLocation = objIndex;
    }
    else
    {
        // 被剔除的对象：indexCount=0，D3D11 自动跳过该绘制调用
        args.indexCountPerInstance = 0;
        args.instanceCount         = 0;
        args.startIndexLocation    = 0;
        args.baseVertexLocation    = 0;
        args.startInstanceLocation = 0;
    }

    indirectArgsBuffer[objIndex] = args;
}
