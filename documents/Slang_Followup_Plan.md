# Slang 迁移后续方案

## 当前状态 (2026-05-26)

| 阶段 | 状态 | 说明 |
|------|------|------|
| Phase 1 (Slang 替换 ShaderConductor) | 完成 | slangc API 替代 ShaderConductor，D3D11 编译通过，Slang Shader 端到端验证成功 |
| Phase 2 (变体系统) | 调整中 | 从 Slang SpecializationConstant 方案回退到 PREDEFINE 预处理方案。ShaderCompiler 为每个变体创建独立 Slang session 避免模块缓存问题 |
| Phase 3 (模块系统 import) | 未开始 | `#include` → `import`，模块缓存 + 增量编译 |
| D3D12 后端 | 暂时禁用 | CMakeLists.txt 中所有 d3d12 源文件已注释；Vulkan 也禁用 |
| D3D11 后端 | 正常运行 | 当前唯一活跃的后端 |

## 后续三步计划

### 第一步：完善当前 PREDEFINE 方案
- 验证所有 shader 变体组合正确编译
- 运行 Sample 1-8 D3D11 渲染验证
- 检查是否有遗留的 `[SpecializationConstant]` 声明

### 第二步：恢复 D3D12 后端
- 取消 CMakeLists.txt 中 17 个 D3D12 文件的注释
- 简化 D3D12 PSO 特化逻辑（不再需要 SpecializationConstant）
- 验证 D3D12 后端基础渲染（至少 Sample 1）

### 第三步：Phase 3 模块系统
- `.dsh` → `.slang`，`.dsf` → `.slang`
- `#include` → `import`
- 利用 Slang 模块缓存加速增量编译

## 设计决策

PREDEFINE 方案更简单可靠，编译期分支避免了运行时特化常量的复杂性（session 缓存、PSO 特化等）。先稳住 D3D11 完整验证，再逐步恢复 D3D12，最后用 Phase 3 模块系统收尾。
