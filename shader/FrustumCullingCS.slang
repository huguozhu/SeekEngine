//STAGE:cs

#include "Common.slangh"
#include "GPUCulling.slangh"

// 视锥体剔除参数
cbuffer cb_FrustumCullingParams : register(b6)
{
    uint   g_ObjectCount;       // 场景中待剔除的对象总数
    uint   g_TotalVisibleCount; // CS 输出：剔除后可见对象总数
    uint   g_padding[2];
    float4 g_FrustumPlanes[6];  // 视锥体六个面（已归一化，xyz 为法线，w 为 d）
};

// 输入：每对象数据（world AABB、world matrix 等），CS 可写 visible 字段
RWStructuredBuffer<GPUObjectData> objectDataBuffer : register(u0);

// 输出：可见对象索引列表（密集排列，计数由 g_TotalVisibleCount 给出）
RWStructuredBuffer<uint> visibleIndexBuffer : register(u1);

// 输出：可见对象计数器（uint[1]，实际使用 InterlockedAdd）
RWStructuredBuffer<uint> visibleCounter : register(u2);

// 测试 AABB 是否与视锥体相交
// 返回 true 表示可见（与 Frustum::Intersect 算法一致）
bool TestAABBVisible(float4 aabbMin, float4 aabbMax)
{
    // 视锥体6个面检测（不含[unroll]，让D3D编译器自行决定优化策略）
    for (int p = 0; p < 6; p++)
    {
        float4 plane = g_FrustumPlanes[p];

        // 选择 AABB 中距离平面最近的顶点（n-p vertex）
        float3 pVertex = float3(
            plane.x > 0.0f ? aabbMax.x : aabbMin.x,
            plane.y > 0.0f ? aabbMax.y : aabbMin.y,
            plane.z > 0.0f ? aabbMax.z : aabbMin.z
        );

        // 如果最近顶点在平面外侧，则 AABB 完全在视锥体外
        if (dot(plane.xyz, pVertex) + plane.w < 0.0f)
            return false;
    }
    return true;
}

[numthreads(64, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint objIndex = dtid.x;
    if (objIndex >= g_ObjectCount)
        return;

    // 读取对象数据
    GPUObjectData obj = objectDataBuffer[objIndex];

    // AABB vs 视锥体检测
    bool visible = TestAABBVisible(obj.aabbMinWorld, obj.aabbMaxWorld);

    // 写入可见性标记
    obj.visible = visible ? 1U : 0U;
    objectDataBuffer[objIndex] = obj;

    // 可见对象追加到密集列表
    if (visible)
    {
        uint writeIndex;
        InterlockedAdd(visibleCounter[0], 1U, writeIndex);
        visibleIndexBuffer[writeIndex] = objIndex;
    }
}
