# SeekEngine Shader 优化方案

> 最后更新：2026-06-01 | 基于全量 shader 代码审查 (60+ .slang/.slangh 文件)

---

## 一、高优先级（性能收益大）

### 1. Vertex Shader：顶点级矩阵求逆（严重性能杀手）

**文件：** `MeshRenderingVS.slang:75/83`

```hlsl
normal_trans = transpose(MatrixInverse(mul(skin_mat, modelInfo.modelMatrix)));
```

`MatrixInverse` 是一个 80+ 行浮点运算的完整 4×4 矩阵求逆，**每个蒙皮顶点都执行一次**。10000 顶点的模型 = 10000 次求逆。

- **优化：** CPU 端预计算 `inverseTranspose` 传入 Constant Buffer，VS 端直接使用
- **预期收益：** 骨骼动画 VS 开销减少 30-50%

---

### 2. Forward 渲染：展开 6 灯循环 + 大面积无用分支

**文件：** `ForwardRenderingCommonPS.slang:179-197`

```hlsl
[unroll]
for (uint lightIdx = 0; lightIdx < 6; lightIdx++)
{
    [branch]
    if (lightInfo[lightIdx].type >= 0)  // 每个像素都判断
```

- `[unroll]` 导致编译器生成 6 份完整光照计算代码，GPR 压力极大
- 场景往往只启用 1-2 个光源，4-5 个槽位完全浪费

**优化：**
- 移除 `[unroll]`，改用 `[loop]`，添加 early-out 标记（type=-1 时 break）
- 或 CPU 端传入 `active_light_count` 作为 loop 上限
- 将 LightInfo 改为 `StructuredBuffer` 动态大小，消除固定 6 灯限制
- **预期收益：** 少光源场景 20% 提升

---

### 3. 阴影：硬编码大数组 + 20 次 Cube Sample

**文件：** `Shadow.slangh:64-84`

```hlsl
float3 gridSamplingDisk[20] = { ... };  // 每像素构建 60 个 float
[unroll]
for (int i = 0; i < samples; ++i)
    float depth = cubeShadowTex.SampleLevel(...);  // 20次Cube纹理采样
```

- `gridSamplingDisk[20]` 每个 PS 调用占 60×4 = 240 字节寄存器
- 20 次 Cube SampleLevel 是带宽瓶颈
- PCF 软阴影固定 3×3 = 9 次采样（line 31-41）

**优化：**
- 将 `gridSamplingDisk` 改为 `static const` 全局数组
- 实现 Poisson Disk 采样（8-12 样本即可达到 20 样本质量）
- 方向光/聚光灯 PCF 降至 4-tap 旋转采样（或 `SampleCmpLevelZero`）
- 硬阴影路径的 `shadowTex.GetDimensions()` 移到 constant buffer
- 添加 early-out：NoL ≤ 0 时完全跳过阴影计算
- 使用 slope-scaled bias 替代固定 bias
- **预期收益：** 点光阴影 40%，方向光阴影 50%

---

### 4. 延迟渲染：间接光照重复采样

**文件：** `DeferredLightingPS.slang:101`

```hlsl
[loop]
for (uint i=0; ...; i++)
{
    if (NoL > 0.0)
    {
        float4 indirect_lighting = indirect_lighting_tex.SampleLevel(point_sampler, iTc, 0);  // ← 循环内！
        Lo += indirect_lighting.rgb + CalcLightingBRDF(...);
    }
}
```

间接光照 `indirect_lighting_tex.SampleLevel` 在**每个光源循环内部**执行，同一像素被重复采样最多 1024 次。

**优化：** 将间接光照采样移到循环外部，循环内只累加直接光照
- **预期收益：** 延迟渲染 2-10x 加速（取决于光源数）

---

### 5. RSM GI：VPL 采样密度过高

**文件：** `GiRsmPS.slang:59-71`

```hlsl
for (int i=0; i<VPL_NUM; i++)
{
    float3 rsm0 = rsm_color0.Sample(shadow_map_sampler, ...);  // 3次纹理读取
    float3 rsm1 = rsm_color1.Sample(shadow_map_sampler, ...);  // 每VPL
    float3 rsm2 = rsm_color2.Sample(shadow_map_sampler, ...);
```

每个 VPL 3 次纹理采样，100 VPL = 300 采样/像素。

**优化：**
- 实现 interleaved sampling：每像素仅采样 1/4 VPL，用 4 帧时序累积
- `CalcVPLIrradiance` 中的 `dot(vpl2frag, vpl2frag)^2` 可用 `rsqrt` 近似
- **预期收益：** 4x GI 性能

---

## 二、中优先级

### 6. SSAO：64 核 × 每样本矩阵运算

**文件：** `SsaoRendering.slang:70-88`

```hlsl
for (int i=0; i<kernel_size; ++i)  // 64次
{
    float3 sample_pos_view = mul(ssao_sample_kernels[i].xyz, TBN);
    float4 pos_proj = mul(float4(sample_pos_view, 1.0), ps_param.proj_matrix);
    float sample_depth = depth_tex.Sample(point_sampler, tc).r;
    sample_depth = ViewSpaceDepth(sample_depth, ...);  // 每样本重复
}
```

720p 下约 59M 次循环迭代，每样本 2 次矩阵乘法 + 1 次深度采样 + ViewSpaceDepth 计算。

**优化：**
- Half-res SSAO（下采样到 1/2 或 1/4 分辨率）
- 降至 32 核 + 4 帧时序累积（Temporal SSAO）
- ViewSpaceDepth 使用线性近似替代精确公式
- 可选替换为 GTAO/CACAO 等更高效算法
- **预期收益：** SSAO 开销减少 75%

---

### 7. TAA：9 点邻域全采样 + 颜色空间转换

**文件：** `TaaPS.slang:131-152`

```hlsl
for (y = -1; y <= 1; ++y) {
    for (x = -1; x <= 1; ++x) {
        float3 NeighborhoodSamp = currentTex.Sample(linear_sampler, sampleUV).rgb;
        NeighborhoodSamp = RGB2YCoCgR(NeighborhoodSamp);  // 每邻居全转换
```

- 12 次纹理采样（3 基础 + 9 邻域）
- 9 次 RGB↔YCoCgR 转换

**优化：**
- 仅对中心像素做 YCoCg 转换，邻域只存 Y 通道做方差
- 使用 5-tap 十字邻域替代 3×3（减少 4 次采样）
- 或改用 luma-based clip（`dot(RGB_TO_LUM, ...)` 替代色彩空间转换）
- 固定 blend factor 0.05 应根据 velocity 大小动态调整
- **预期收益：** TAA 开销减少 40%

---

### 8. Constant Buffer 布局浪费

**文件：** `shared/CommonTypes.slang`

`LightInfo` 结构体大小约 **160 bytes**（含 float4x4 + 大量 padding）：
- 前向渲染用 6 个 = 960 bytes
- 延迟渲染 StructuredBuffer 存储 1024 个 = 160 KB
- `lightViewProj` 仅方向光/聚光灯需要，点光源浪费 64 bytes/光源
- 颜色使用 float3（12 bytes），可用 `packed half` 或 R11G11B10 节省

**优化：**
- 拆分 LightInfo 为 `DirectionalLightData` / `PointLightData` / `SpotLightData`
- 将 LightInfo 合并为 uint 位标识，用位运算替代分支链
- **预期收益：** 带宽减半

---

### 9. FXAA：静态数据运行时构造 + 重复采样

**文件：** `FxaaPS.slang:76-88, 102`

```hlsl
float2 KERNEL_STEP_MAT[9];           // 每像素重新构造
KERNEL_STEP_MAT[0] = float2(-1.0f, 1.0f);
...
output.color = currentTex.Sample(linear_sampler, input.texCoord);  // 已有 luma，却重新采样
```

**优化：**
- `KERNEL_STEP_MAT` 和 `average_weight_mat` 改为 `static const`
- `QUALITY(i)` 函数用常量数组替代 if-else 链
- 缓存中心像素颜色，避免早期返回时重复采样
- **预期收益：** FXAA 开销减少 5-15%

---

### 10. BRDF 库：未使用函数膨胀

**文件：** `BRDF.slangh`

文件中包含 OrenNayar、Blinn、Charlie、Kelemen 等多个未被调用的 BRDF 函数。`CalcDiffuseBRDF` 只用了最简单的 `Diffuse_Lambertian()`。

**优化：** 用 `#ifdef` 宏包裹，或拆分为独立 `#include` 按需组合

---

### 11. 粒子系统：原子操作串行化

**文件：** `Particles/ParticleSimulateCS.slang`

```hlsl
InterlockedAdd(particle_counters[0].dead_count, 1, insert_index);
InterlockedAdd(particle_counters[0].alive_count[post_sim_index], 1, insert_index);
InterlockedAdd(particle_counters[0].alive_count[pre_sim_index], -1, index);
```

每个粒子操作都需要全局原子操作，导致整个 wave 串行化。

**优化：**
- 使用 wave-level prefix sum 替代全局原子操作
- 将状态变化收集到 groupshared，批量提交
- Culling 后粒子数少于阈值（<100）时跳过 Sort 直接渲染
- BitonicSort 可考虑 Radix Sort 减少 pass 数
- BitonicSort 每次迭代 2 次 GroupMemoryBarrier，可减少为 1 次

---

### 12. LightCullingCS：深度范围追踪禁用

**文件：** `LightCullingCS.slang:84-95`

深度范围计算的代码被整段注释，导致 `min_z = nearPlane, max_z = farPlane`，Tile Frustum 退化为完整视锥体，culling 效率大打折扣。

**优化：**
- 恢复深度追踪，使用 `WaveActiveMin/Max` 替代 `InterlockedMin/Max`
- 启用 Tile Light Culling 后，多光源场景可减少 60-80% 逐像素光照计算

---

## 三、低优先级 / 架构级

### 13. 着色器变体（Permutation）优化

**现状：**
- `MeshRenderingVS.slang`：JOINT_BIND_SIZE(0/4/8) × HAS_NORMAL × ENABLE_TAA × MORPH_TYPE = 最多 24 变体
- `ForwardRenderingCommonPS.slang`：4 个 predefine × 2⁴ = 16 变体

**优化：**
- HAS_MATERIAL_NORMAL 从 predefine 改为 specialization constant，减少 50% 变体
- ENABLE_TAA / MORPH_TYPE 合并为 specialization constant
- JOINT_BIND_SIZE 保留为 predefine，但仅保留 0/8 两组

---

### 14. 死代码清理

| 位置 | 说明 |
|------|------|
| `Common.slangh` | `LinearRGB_2_LAB`, `LAB_2_LinearRGB`, `yuv2rgb`, 色彩空间矩阵 — 从未被调用 |
| `ForwardRenderingCommonPS.slang:42-56` | `CalcEnvironmentAmbient` 中 diffuse/specular 硬编码为 0，整个函数无效 |
| `BRDF.slangh` | 未使用的 OrenNayar, Blinn, Charlie, Kelemen BRDF |
| `GiLpvPS.slang` / `LPV*.slang` | LPV GI 几乎为占位实现，建议清理或完整实现 |

---

### 15. 其他小节

| 项目 | 说明 |
|------|------|
| **ToneMapping** | `adapted_lum=1.0` 硬编码，应实现基于 histogram 的自动曝光 |
| **Material CB** | `hasBasicTex.x == 1234` 魔法数字应替换为语义化枚举 |
| **PI 精度** | 使用 `3.1415926` 而非 `3.14159265359`，高粗糙度下精度可能不足 |
| **Normal Encode** | `BestFitNormal_Encode` 在 GPU 上不如 `octahedron encode` 快速 |
| **Shadow Map** | 可合并为 Texture2DArray，减少绑定点切换 |
| **全局 Sampler** | 用 `StaticSampler` 声明，消除运行时绑定 |

---

## 四、按优先级汇总（投入产出比排序）

| 优先级 | 优化项 | 预期收益 | 工作量 |
|--------|--------|----------|--------|
| 🔴 P0 | 延迟渲染：间接光照移出光源循环 | 2-10x 加速 | 极小 |
| 🔴 P0 | VS：矩阵求逆移 CPU | 30-50% VS | 小 |
| 🔴 P0 | 启用 Tile Light Culling | 多光源场景 60-80% | 小 |
| 🟠 P1 | 阴影：Poisson Disk + early-out | 30-40% 阴影 | 中 |
| 🟠 P1 | Forward：动态循环替代展开 | 少光源 20% | 小 |
| 🟠 P1 | RSM GI：交错采样 | 4x GI | 中 |
| 🟡 P2 | SSAO：32 核 + 半分辨率 + 时序 | 75% SSAO | 中 |
| 🟡 P2 | TAA：5-tap + luma clip | 40% TAA | 中 |
| 🟡 P2 | LightInfo CB 拆分 | 带宽减半 | 大 |
| 🟡 P2 | 粒子 Sort 阈值跳过 | 少量粒子加速 | 中 |
| 🟢 P3 | FXAA / BRDF 微观优化 | 5-15% | 小 |
| 🟢 P3 | Dead code 清理 | 编译/维护 | 小 |
| 🟢 P3 | Shader permutation 优化 | 编译时间 30-50% | 中 |
