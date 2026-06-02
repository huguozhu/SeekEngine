---
name: d3d11-leak-fix-imgui-sprite2d
description: Fixed D3D11/DXGI leaks: ImGui resources not shut down, Sprite2DRenderer not cleaned up in Uninit
metadata:
  type: project
---

## D3D11 泄露问题修复

### 问题 1: ImGui D3D11 资源未释放（11 个 D3D11 子对象泄露）
- **文件**: `samples/common/app_framework.cpp`
- **原因**: `AppFramework::Run()` 中通过 `ImGui_ImplDX11_Init()` / `ImGui_ImplWin32_Init()` 初始化了 ImGui，但消息循环退出后从未调用对应的 Shutdown 函数
- **泄露对象**: ID3D11DepthStencilState, VertexShader, InputLayout, 3×Buffer, PixelShader, BlendState, RasterizerState, ShaderResourceView, Sampler（恰好对应 ImGui 创建的字体纹理、Shader、顶点/索引/常量缓冲区、渲染状态）
- **修复**: 在 `OnDestroy()` 之后、`DestroyWindow()` 之前添加:
  ```cpp
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
  ```

### 问题 2: Sprite2DRenderer 未在 Context::Uninit() 中清理
- **文件**: `engine/kernel/context.cpp`
- **原因**: `Context::Init()` 创建了 `m_pSprite2DRenderer`，但 `Context::Uninit()` 未重置它。它的生命周期延续到 `~Context()` 完成之后，在 DXGI 泄露报告（`OutputD3DCommonDebugInfo()`）之后才释放
- **修复**: 在 `Context::Uninit()` 中添加 `m_pSprite2DRenderer.reset()`，放在 `m_pSceneRenderer.reset()` 之前

### 验证结果
修复后运行 05.DeferredShading，DXGI ReportLiveObjects 不再报告任何泄露对象。

**Why:** ImGui 的 D3D11 后端在初始化时创建了大量 D3D11 对象（字体纹理 SRV/Sampler、Shader、Buffer、BlendState 等），这些对象内部持有 ID3D11Device 引用。不释放这些对象导致 Device 无法释放，进而导致 DXGI Adapter/Factory 也无法释放，形成连锁泄露。

**How to apply:** 所有使用 AppFramework 的 sample 都自动受益于 ImGui 修复。如果添加新的渲染器类型，确保在 Context::Uninit() 中按正确顺序（上层对象先释放，RHIContext 最后释放）添加对应的 reset() 调用。
