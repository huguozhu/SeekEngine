# D3D12 后端 MVP 完善方案

## 目标

让 Sample 1 (Tutorial) 在 D3D12 后端成功运行，验证 D3D12 管线（Device → CommandList → PSO → Draw → Present）全链路正确性。

## 策略：预定义 Root Signature

为 MVP 阶段定义一个固定的 Root Signature 布局，适用于 Sample 1 的简单 Shader（MeshRenderingVS + ForwardRenderingCommonPS），不依赖 Shader 反射自动生成。

### Root Signature 布局

| 参数索引 | 类型 | 绑定 | 说明 |
|----------|------|------|------|
| 0 | CBV Root Descriptor | b0 | ModelInfo |
| 1 | CBV Root Descriptor | b1 | ViewInfo (CameraInfo) |
| 2 | CBV Root Descriptor | b2 | LightInfo[6] |
| 3 | CBV Root Descriptor | b3 | MaterialInfo |
| 4 | CBV Root Descriptor | b4 | MorphTarget / 扩展 |
| 5 | SRV Descriptor Table | t0+ (unbounded) | 纹理 SRV（MVP 阶段占位） |

**Static Samplers：**
| s0 | linear_wrap | 线性过滤 + Wrap |
| s1 | point_clamp | 点采样 + Clamp |
| s2 | shadow_cmp | 阴影比较采样 |

## 实现清单

### 已修改的 9 个文件：

1. **`engine/rhi/d3d12/d3d12_shader.h`**
   - 添加 `ID3DBlobPtr m_pShaderBlob` — 存储已编译的 DXIL 二进制
   - 添加 `D3D12_SHADER_BYTECODE m_ShaderByteCode` — PSO 使用的字节码引用
   - 添加 `size_t m_iPsoHashValue` — PSO 缓存哈希值
   - 添加 `PsoHashValue()` / `GetShaderByteCode()` 公共接口

2. **`engine/rhi/d3d12/d3d12_shader.cpp`**
   - 实现 `OnCompile()`：
     - 使用 `dxcompiler.dll` (IDxcCompiler) 编译 HLSL → DXIL
     - 编译目标：vs_6_0 / ps_6_0 / cs_6_0 等
     - Debug 模式：`-Zi -Od`（调试信息 + 禁用优化）
     - Release 模式：`-O3`（最高优化）
     - 支持宏定义（m_vMacros）和变体预定义（m_vPredefines）传递
     - 添加 `PLATFORM_HLSL=1` 平台宏
     - 错误时通过 LOG_ERROR 输出编译错误
   - 实现 `UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC&)` — 按 Shader Stage 设置 VS/PS/GS/HS/DS 字节码
   - 实现 `UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC&)` — 设置 CS 字节码
   - 实现 `PsoHashValue()` — 对编译后 DXIL 二进制做哈希

3. **`engine/rhi/d3d12/d3d12_context.h`**
   - 添加根签名相关常量：`ROOT_PARAM_CBV_B0..B4`、`ROOT_PARAM_SRV_TABLE`、`NUM_ROOT_PARAMS`、`NUM_STATIC_SAMPLERS`
   - 添加 `CreateRootSignature()` 声明
   - 添加 `GetGraphicsRootSignature()` / `GetComputeRootSignature()` 公共访问器
   - 添加 `CurFrameIndex()` / `FrameFenceValue()` 公开访问器
   - 添加 `ID3D12RootSignaturePtr m_pGraphicsRootSignature / m_pComputeRootSignature` 成员
   - 添加 `ID3D12DescriptorHeapPtr m_pCurSrvUavHeap / m_pCurSamplerHeap` 缓存

4. **`engine/rhi/d3d12/d3d12_context.cpp`**
   - 实现 `CreateRootSignature()`：
     - 5 个 CBV Root Descriptor Parameters
     - 1 个 SRV Descriptor Table (unbounded)
     - 3 个 Static Samplers
     - 分别创建 Graphics（带 IA flag）和 Compute 版本
   - 实现 `UpdateRenderPso()`：
     - 从 RenderState 获取/创建 PSO（含根签名）
     - 设置 PipelineState + RootSignature 到 CommandList
   - 实现 `BindConstantBuffer()`：
     - 通过 `SetGraphicsRootConstantBufferView(rootParamIndex, gpuVAddr)` 绑定 CBV
   - 修改 `BeginRenderPass()`：
     - 设置 RenderTargets（OMSetRenderTargets + RSSetViewports）
     - 应用 Resource Barriers（FB→RENDER_TARGET）
     - 调用 FrameBuffer::Clear()
     - 重置 PSO 状态追踪
   - 在 `Init()` 中调用 `CreateRootSignature()`

5. **`engine/rhi/d3d12/d3d12_window.cpp`**
   - 重写 `SwapBuffers()` 为完整帧闭环：
     1. BackBuffer → PRESENT barrier
     2. Close CommandList
     3. Execute CommandList on Queue
     4. Present SwapChain
     5. Signal Fence → Sync GPU
     6. 前进帧索引 → Reset Allocator → Reset CommandList（为下一帧准备）

6. **`engine/rhi/d3d12/d3d12_render_state.h/cpp`**
   - `GetGraphicPso()` 接口改为接受 `RHIProgram&`（而非单个 `RHIShader&`）
   - 遍历所有 Shader Stage（VS/PS/GS/HS/DS）填入 PSO Desc
   - PSO Desc 设置根签名 `pso_desc.pRootSignature`
   - Hash 计算包含所有 Shader Stage 的字节码
   - 创建失败时打印错误日志
   - 添加 `#include "utils/log.h"`

7. **`engine/rhi/d3d12/d3d12_framebuffer.cpp`**
   - 实现 `Clear(flags, clr, depth, stencil)`：
     - `ClearRenderTargetView` 逐 RTV 清除颜色
     - `ClearDepthStencilView` 清除深度/模板
   - 实现 `ClearRenderTarget(att, clr)`：单个 RTV 清除
   - 修改 `SetRenderTargets()`：实际调用 `OMSetRenderTargets` + `RSSetViewports`

8. **`engine/rhi/d3d12/d3d12_gpu_buffer.cpp`**
   - 实现 `Update(buffer_data)`：
     - Upload Heap 路径：直接 `memcpy` 到映射的 CPU 地址（最快）
     - Default Heap 路径：Staging Upload → CopyBufferRegion → Resource Barrier

9. **`engine/CMakeLists.txt`**
   - `SEEK_STATICLIB_DEPENDENT_LIBS` 添加 `d3d12` 和 `dxcompiler`

## 执行流程

```
Context::Update()
  → BeginRender()             — 视口设置
  → RenderFrame()
    → BuildRenderJobList()
    → DoRenderJob() loop
      → RenderSceneJob()
        → BeginRenderPass()   — 设置 RT + Clear + Barrier
        → RenderScene()
          → mesh->Render()
            → rs->Active()    — StencilRef + BlendFactor
            → program->Active() — Shader 延迟编译 (OnCompile: HLSL→DXIL)
            → mesh->Active()  — 绑定 VB/IB
            → UpdateRenderPso() — 创建/缓存 PSO + SetRootSignature + SetDescriptorHeaps
            → BindConstantBuffer(b0-b4) — SetRootConstantBufferView
            → DrawIndexedInstanced()
        → EndRenderPass()
  → EndRender()
    → SwapBuffers()
      → PRESENT barrier
      → Close + Execute CmdList
      → Present()
      → Signal + Sync Fence
      → Reset for next frame
```

## 后续工作（功能对等阶段）

- [ ] Shader 反射驱动 Root Signature（替代固定布局）
- [ ] Compute Shader 完整支持（Dispatch + UAV 绑定）
- [ ] 纹理 SRV/Sampler 动态绑定（非 Static Sampler）
- [ ] TimeQuery 实现
- [ ] Texture Copy / Sync / Update / Dump
- [ ] Indirect Draw/Dispatch 实现
- [ ] MSAA 支持
- [ ] PSO 缓存持久化
- [ ] GPU 调试标记和性能事件
