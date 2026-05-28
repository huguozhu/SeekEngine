# SeekEngine Shader 优化方案

## 一、着色器变体（Shader Permutation）爆炸

**现状**：
- `MeshRenderingVS.slang` — JOINT_BIND_SIZE(0/4/8) × HAS_NORMAL(0/1) × ENABLE_TAA(0/1) × MORPH_TYPE(0/1) = 最多 24 变体
- `ForwardRenderingCommonPS.slang` — 4 个 predefine × 2⁴ = 16 变体
- 每个变体组合都生成独立 PSO，编译时间长且运行时切换开销大

**优化**：

| 项目 | 方案 |
|------|------|
| P0 | 将 `HAS_MATERIAL_NORMAL` 从 predefine 改为 specialization constant，减少 VS/PS 变体数 50% |
| P0 | 将 `ENABLE_TAA`/`MORPH_TYPE` 合并为 specialization constant，消除跨变体重复编译 |
| P1 | `JOINT_BIND_SIZE` 保留为 predefine（影响输入布局），但仅保留 0/8 两组（4 骨骼合并到 8 骨骼的分支处理） |

## 二、前向渲染 Pixel Shader

**文件**: `ForwardRenderingCommonPS.slang`

**2.1 光照循环无条件遍历全部 6 个灯**
```hlsl
[unroll]
for (uint lightIdx = 0; lightIdx < 6; lightIdx++)  // 无条件 6 次
```
- 即使只有 1 个灯，也展开为 6 次完整光照计算
- **优化**：通过 constant buffer 传入实际灯光数，改为 `[loop]` 动态循环

**2.2 IBL 环境光计算为死代码**
```hlsl
float3 CalcEnvironmentAmbient(...) {
    float3 diffuse = 0.0;      // 始终为 0
    float3 specular = 0.0;     // 始终为 0
    ...
}
```
- `CalcEnvironmentAmbient` 中 diffuse/specular 硬编码为 0，整个函数无效
- **优化**：删除或实现正确的 IBL 采样逻辑

**2.3 阴影采样分支可优化**
```hlsl
lightInfo[lightIdx].type == LIGHT_TYPE_DIRECTIONAL || lightInfo[lightIdx].type == LIGHT_TYPE_SPOT
```
- 每个像素、每个灯光都做 shadow type 分支判断
- **优化**：将 `LightInfo` 合并为一个 `uint` 标识位，用位运算替换分支链

## 三、阴影计算

**文件**: `Shadow.slangh`

**3.1 Point Light 软阴影：每像素 20 次 Cube 纹理采样**
```hlsl
int samples = 20;
float3 gridSamplingDisk[20] = { ... };
for (int i = 0; i < samples; ++i)
    cubeShadowTex.SampleLevel(..., fragToLight + gridSamplingDisk[i] * diskRadius, 0);
```
- 1 个点光源软阴影 = 20 次 CubeMap 采样/像素，开销极大
- **优化**：减为 8-12 样本 + 随机旋转（用蓝噪声），或使用 Poisson Disk 采样替代规则网格

**3.2 Soft Shadow PCF 固定 3×3 = 9 次采样**
```hlsl
for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
```
- **优化**：采用 4-tap PCF（旋转采样模式）或硬件 PCF（`SampleCmpLevelZero`），降至 4 次采样

## 四、延迟渲染

**文件**: `DeferredLightingPS.slang`

**4.1 Tile Light Culling 实际未启用**
- `TILE_CULLING=1` 路径存在，但 C++ 端 `use_tile_culling = 0` 硬编码关闭
- **优化**：启用 Tile-based 光照剔除，在多光源场景（>4 个灯）可减少 60-80% 逐像素光照计算

**4.2 Indirect lighting texture 每光源重复采样**
```hlsl
Lo += indirect_lighting.rgb + CalcLightingBRDF(...);
```
- `indirect_lighting_tex` 在循环内每次迭代都采样，但它是光照无关的
- **优化**：将间接光照采样提到循环外，只采样一次

## 五、SSAO

**文件**: `SsaoRendering.slang`

**5.1 64 个采样核每像素执行**
```hlsl
for (int i=0; i<kernel_size; ++i)  // 固定 64 次
```
- 全分辨率 SSAO + 64 样本 → 720p 下约 59M 次循环迭代
- **优化**：
  - Half-res SSAO（下采样到 1/2 或 1/4 分辨率）
  - 使用 16-32 样本 + 随机旋转（蓝噪声），效果接近 64 样本
  - 或替换为 GTAO/CACAO 等更高效的 AO 算法

**5.2 每样本 1 次深度纹理采样 + 矩阵乘法**
```hlsl
float3 sample_pos_view = mul(ssao_sample_kernels[i].xyz, TBN);
float4 pos_proj = mul(float4(sample_pos_view, 1.0), ps_param.proj_matrix);
float sample_depth = depth_tex.Sample(point_sampler, tc).r;
```
- 64 样本/像素 → 128 次矩阵乘法 + 64 次纹理采样
- **优化**：使用 Interleaved Rendering（隔像素交错采样），每像素仅 8-16 样本 + 时域累积

## 六、TAA

**文件**: `TaaPS.slang`

**6.1 3×3 邻域采样 + RGB→YCoCg 色彩空间转换**
```hlsl
for (y = -1; y <= 1; ++y)
    for (x = -1; x <= 1; ++x)
```
- 9 次额外纹理采样（已有当前帧 + 历史帧 + 速度缓冲共 3 次采样）
- **优化**：使用 5-tap 十字邻域替代 3×3 正方形邻域，减少 4 次采样

**6.2 色彩空间转换开销**
- `RGB2YCoCgR` + `YCoCgR2RGB` 每像素各执行一次，包含多次乘加
- YCoCg 的收益是方差裁剪更准确，但可用简化的 luma-based clip 替代
- **优化**：仅对 luma 通道做 variance clip（`dot(RGB_TO_LUM, ...)`），消除色彩空间转换

## 七、粒子系统

**目录**: `Particles/`

**7.1 BitonicSort 循环内两次 GroupMemoryBarrier**
```hlsl
for (uint j = param.sort_level>>1; j>0; j>>=1) {
    // ... compare-swap ...
    GroupMemoryBarrierWithGroupSync();
    shared_particle_depth[GI] = result_depth;
    shared_particle_index[GI] = result_index;
    GroupMemoryBarrierWithGroupSync();
}
```
- 每次迭代 2 次 barrier，对 `sort_level` 大时开销显著
- **优化**：减少 barrier 次数（每迭代 1 次即可），或将 sort 替换为 GPU Radix Sort

**7.2 ParticleCulling：视锥剔除后无需 Sort**
- 粒子渲染前先 Culling → PreSort → BitonicSort → Transpose，4 个 CS Pass
- Culling 结果如果很少（<100），Sort 开销大于收益
- **优化**：阈值判断，少数粒子时跳过 Sort 直接渲染

## 八、通用优化

| 项目 | 方案 |
|------|------|
| P0 | 全局静态 Sampler 用 `StaticSampler` 声明，消除运行时绑定 |
| P1 | 删除 `Common.slangh` 中未使用的函数（`LinearRGB_2_LAB`、`LAB_2_LinearRGB`、`yuv2rgb`、`ColorSpace` 矩阵等），减小 shader 编译产物 |
| P1 | `MatrixInverse()` 在 VS 中每顶点调用（TAA 路径），改为从 constant buffer 传入逆矩阵 |
| P2 | FXAA 的 `QUALITY(i)` 函数改为查找表数组 |
| P2 | Shadow Map 合并为 Texture2DArray，减少绑定点切换 |
| P3 | GiRsm/GiLpv 当前几乎为占位实现，建议清理或实现完整 LPV 注入/传播 |

## 九、按优先级汇总

| 优先级 | 优化项 | 预期收益 |
|--------|--------|----------|
| **P0** | 启用 Tile Light Culling | 多光源场景 GPU 减少 60-80% |
| **P0** | SSAO 半分辨率 + 减少样本(16) | SSAO 开销减少 75% |
| **P0** | 前向渲染动态循环替代展开 | 少光源场景性能提升 |
| **P1** | Specialization constant 减少变体 | 编译时间减少 30-50% |
| **P1** | Shadow PCF 降至 4-tap | 阴影开销减少 50% |
| **P1** | 点光源阴影降至 8-12 样本 | 点光阴影开销减少 40% |
| **P1** | TAA 用 5-tap + luma clip | TAA 开销减少 40% |
| **P2** | VS 逆矩阵从 CB 传入 | 骨骼动画 VS 开销减少 |
| **P3** | 删除死代码/清理未使用 shader | 编译和维护成本降低 |
