# Slang 着色器语言迁移方案

## 目标

将 SeekEngine 的 153 个 HLSL 着色器文件 (.dsf/.dsh) 渐进式迁移到 Slang 着色器语言，最终实现：
1. 用 slangc 编译器替代 ShaderConductor 作为交叉编译后端
2. 用 Slang 泛型/接口/特化常量替代 `//PREDEFINE:` 笛卡尔积变体系统
3. 用 Slang 模块系统 (import) 替代 C 风格 #include

## 当前系统分析

### Shader 文件结构（153 个文件）

| 类别 | 数量 | 示例 |
|------|------|------|
| .dsf 入口文件 | ~45 | MeshRenderingVS, ForwardRenderingCommonPS, ParticleEmitCS |
| .dsh 头文件 | ~12 | Common.dsh, Lighting.dsh, Shadow.dsh, BRDF.dsh |
| shared/*.h 共享结构体 | ~7 | common.h, ParticleCommon.h, GI.h |

### 编译管线

```
.dsf 源文件 (HLSL + 注解)
  │
  ├─ 解析: //PREDEFINE:NAME=V1,V2,...  → 生成变体矩阵
  ├─ 解析: //STAGE:vs/ps/cs             → 确定 Shader Stage
  ├─ 解析: #include "Common.dsh"        → 头文件包含图
  │
  ▼
ShaderCompiler.exe (基于 ShaderConductor)
  │
  ├─ ShaderConductor::Compiler::Compile()
  │   ├─ HLSL 预处理器展开 (宏 + include)
  │   ├─ HLSL → 目标平台编译
  │   └─ 反射提取 (Resources, Inputs, Outputs, BlockSize)
  │
  ├─ 输出: .hlsl/.dxil/.glsl/.msl (源码或字节码)
  ├─ 输出: .reflect (JSON: 资源绑定、IO签名、线程组大小)
  ├─ 输出: .meta (JSON: Stage、Predefine 候选值)
  └─ 输出: .hpp (内嵌 C 数组，编译进引擎)
```

### 运行时加载

```
引擎启动
  → ResourceManager::LoadShaderResource(name)
    → RHIQueryGeneratedShaderCodeContent(language, name)
      → 查表获取内嵌的 C 数组指针 + 大小
    → 返回 ShaderResource{sourceCode, sourceCodeSize, reflectInfo}

Technique::Build()
  → 对每个 ShaderStage (VS/PS/CS...):
    → RHIContext::CreateShader(type, name, entryPoint, code)
      → D3D11: D3DCompile(sourceCode) → CreateVertexShader/PixelShader/...
      → D3D12: IDxcCompiler::Compile(sourceCode) → CreateRootSignature + PSO
```

### 变体系统

```
//PREDEFINE:JOINT_BIND_SIZE=0,4,8
//PREDEFINE:ENABLE_TAA=0,1
//PREDEFINE:HAS_MATERIAL_NORMAL=0,1
//PREDEFINE:MORPH_TYPE=0,1
```
生成 3×2×2×2 = 24 个独立编译的变体，文件名如 `MeshRenderingVS_<hash>`。

每个 `#include` 的头文件又可能有自己的 `//PREDEFINE`，导致组合爆炸。

---

## 三阶段路线

```
Phase 1: 替换编译器后端     [改动范围: ShaderCompiler + slang.dll]
  输入: .dsf/.dsh (HLSL, 格式不变)
  编译: ShaderConductor → slang API
  输出: .compile/.reflect/.meta (格式不变)
  引擎: 零改动

Phase 2: 泛型替代宏变体      [改动范围: Shader 源码 + Technique/Effect 层]
  输入: .dsf 中 PREDEFINE 逐步替换为 Slang generics
  编译: slangc specialization constant 支持
  输出: .reflect 增加 specialization_info 节
  引擎: Technique 支持 SpecializationConstant 绑定

Phase 3: 模块系统            [改动范围: Shader 源码格式]
  输入: .slang 格式 (import 替代 #include)
  编译: slangc 模块缓存 + 增量编译
  输出: 格式不变
  引擎: 零改动 (输出接口不变)
```

---

## Phase 1: 替换编译器后端

### 技术映射

| ShaderConductor API | Slang API | 说明 |
|---|---|---|
| `Compiler::SourceDesc` | `IGlobalSession::createSession()` → `ISession::loadModule()` | 源码加载与编译 |
| `Compiler::TargetDesc` | `IComponentType::getTargetCode(targetIndex)` | 目标平台选择 |
| `Compiler::Options` | `SessionDesc::compilerOptionEntries` | 优化级别、调试信息 |
| `Compiler::Compile()` | `getEntryPointCode()` + `getTargetCode()` | 获取编译结果 |
| `Reflection::NumResources()` | `ProgramLayout::getParameterCount()` | 资源数量 |
| `Reflection::ResourceByIndex()` | `ProgramLayout::getParameterByIndex()` | 逐个资源信息 |
| `Reflection::InputParameter()` | `IEntryPoint::getMetadata()` | 输入签名 |
| `Reflection::CSBlockSize()` | EntryPoint compute info | 计算线程组大小 |

### Slang 编译流程（四步法）

```cpp
// 1. 创建全局 Session（只需一次）
slang::IGlobalSession* gSession;
slang::createGlobalSession(...);

// 2. 编译源码 → Module
slang::IModule* module;
session->loadModule(sourcePath, sourceBlob, module);

// 3. 查找入口点
slang::IEntryPoint* entryPoint;
module->findEntryPointByName("main", entryPoint);

// 4. 创建组合 → 编译 → 获取结果
ComPtr<slang::IComponentType> program;
session->createCompositeComponentType({module, entryPoint}, program);

ComPtr<slang::IBlob> codeBlob, diagnostics;
program->getEntryPointCode(0, targetIndex, codeBlob, diagnostics);

// 反射
slang::ProgramLayout* layout = program->getLayout();
```

### 目标平台映射

| SeekEngine Target | slang Target Code | 说明 |
|---|---|---|
| `hlsl` (D3D11) | `SLANG_HLSL` | HLSL 源码 |
| `dxil` (D3D12) | `SLANG_DXIL` | DXIL 字节码 |
| `essl` (OpenGL ES) | `SLANG_GLSL` | GLSL ES 源码 |
| `spirv` (Vulkan) | `SLANG_SPIRV` | SPIR-V 字节码 |
| `msl_macos` (Metal Mac) | `SLANG_METAL` | Metal Shading Language |
| `msl_ios` (Metal iOS) | `SLANG_METAL` + iOS 配置 | Metal Shading Language |

### 反射信息映射

| ShaderConductor Reflection | Slang Layout | Transform |
|---|---|---|
| `ResourceType::ConstantBuffer` | `ParameterInfo::category == Uniform` with `type->getKind() == ConstantBuffer` | 直接映射 |
| `ResourceType::Texture` | `ParameterInfo::category == ShaderResource` with texture type | 直接映射 |
| `ResourceType::Sampler` | `ParameterInfo::category == SamplerState` | 直接映射 |
| `ResourceType::SampledTexture` | Combined texture+sampler (需拆分) | 回退名处理 |
| `ResourceType::Buffer` | `ParameterInfo::category == ShaderResource` with buffer type | 直接映射 |
| `ResourceType::RWBuffer` | `ParameterInfo::category == UnorderedAccess` with buffer type | 直接映射 |
| `ResourceType::RWTexture` | `ParameterInfo::category == UnorderedAccess` with texture type | 直接映射 |
| `InputParameters` | `IEntryPoint::getMetadata("inputSemantics")` | 语义提取 |
| `OutputParameters` | `IEntryPoint::getMetadata("outputSemantics")` | 语义提取 |
| `CSBlockSize` | `IEntryPoint::getMetadata("threadGroupSize")` | 直接映射 |

### Phase 1 改动清单

**修改文件：**
- `tools/ShaderCompiler/ShaderCompiler.cpp` — 核心编译逻辑重写
- `tools/ShaderCompiler/CMakeLists.txt` — 链接 slang.dll 替换 ShaderConductor
- `engine/CMakeLists.txt` — 移除 ShaderConductor 链接

**新增文件：**
- `third_party/slang/` — Slang SDK (预编译库 + 头文件)

**删除/移除：**
- `third_party/shaderconductor/` — 完全替换（Phase 1 完成后）

**不修改：**
- 所有 .dsf/.dsh 文件（HLSL 语法 Slang 完全兼容）
- shader/CMakeLists.txt（构建流程不变）
- engine/resource/（运行时加载不变）
- engine/rhi/（RHI 层不变）
- engine/effect/（技术/效果层不变）
- .reflect / .meta 文件格式（保持不变）

---

## Phase 2: 泛型替代宏变体

### 变体分类与替换策略

| 变体类型 | 示例 | Slang 替代 |
|----------|------|------------|
| **材质贴图开关** | HAS_MATERIAL_ALBEDO/NORMAL/METALLIC/NORMAL_MASK | `interface IMaterialTextureSet` + generics |
| **光照模型选择** | LIGHT_MODE (Phong/PBR) | `interface ILightingModel` + generics |
| **骨骼绑定数量** | JOINT_BIND_SIZE (0/4/8) | `[SpecializationConstant] int g_JointBindSize` |
| **Morph Target** | MORPH_TYPE (0/1/2) | `[SpecializationConstant] int g_MorphType` |
| **TAA 开关** | ENABLE_TAA (0/1) | `[SpecializationConstant] bool g_EnableTAA` |
| **阴影类型** | SHADOW_MODE | `interface IShadowEvaluator` + generics |
| **GI 模式** | GI_MODE | `interface IGIProvider` + generics |

### Slang 泛型示例

**当前（宏变体）：**
```hlsl
// MeshRenderingVS.dsf — 需要 24 个变体
//PREDEFINE:JOINT_BIND_SIZE=0,4,8
//PREDEFINE:HAS_MATERIAL_NORMAL=0,1

#if JOINT_BIND_SIZE > 0
    float4x4 skin_mat = CalcSkeletonMatrix(...);
    pos = mul(pos, skin_mat);
#endif

#if HAS_MATERIAL_NORMAL
    output.tangent = input.tangent;
#endif
```

**Phase 2 目标（Slang 泛型 + 特化常量）：**
```slang
// MeshRenderingVS.slang — 单文件，编译时 + 链接时特化

// 链接时特化：PSO 创建时绑定值
[SpecializationConstant]
int g_JointBindSize = 0;

interface IVertexAttributes
{
    float4 getTangent(float4 inputTangent);
}

// 编译时特化：不同材质类生成不同代码
struct PBRSkinnedVertex : IVertexAttributes { ... }
struct SimpleVertex : IVertexAttributes { ... }

[shader("vertex")]
VSOutput main<T : IVertexAttributes>(VSInput input)
{
    if (g_JointBindSize > 0)
    {
        float4x4 skin_mat = CalcSkeletonMatrix(input, g_JointBindSize);
        pos = mul(pos, skin_mat);
    }
    output.tangent = T.getTangent(input.tangent);
}
```

### SpecializationConstant 的引擎集成

**新增反射信息（.reflect 扩展）：**
```json
{
  "reflect": {
    "stage": "vs",
    "resources": [...],
    "specializations": [
      {"name": "g_JointBindSize", "type": "int", "default": 0, "binding": 100},
      {"name": "g_MorphType", "type": "int", "default": 0, "binding": 101},
      {"name": "g_EnableTAA", "type": "bool", "default": false, "binding": 102}
    ]
  }
}
```

**引擎 PSO 创建时绑定：**
```cpp
// Technique::Build() 中
std::vector<SpecializationConstant> specs;
specs.push_back({"g_JointBindSize", mesh->GetJointBindSize()});
specs.push_back({"g_EnableTAA", context->GetAntiAliasingMode() == TAA});
// ...
program->CreateWithSpecialization(specs);
```

### Phase 2 改动清单

**新增文件：**
- `shader/interfaces/` — 接口定义模块（IMaterialTextureSet, ILightingModel, etc.）
- `engine/effect/specialization.h` — SpecializationConstant 绑定 API

**修改文件：**
- .dsf 文件 — 逐步替换 PREDEFINE 为泛型/特化常量（按复杂度和收益排序）
- `engine/effect/technique.cpp` — Technique 构建时传递 SpecializationConstants
- `engine/rhi/base/rhi_program.h` — Program 支持 specialization 参数
- `engine/rhi/d3d12/d3d12_program.cpp` — PSO 创建时传入 specialization 数据
- D3D11/Vulkan 的 Program 实现同理

**保持不变：**
- ShaderCompiler 输出格式（仅 .reflect 增加 specialization_info 节）
- ResourceManager 加载逻辑
- 所有非变体 Shader

---

## Phase 3: 模块系统

### 当前 include 链

```
MeshRenderingVS.dsf
  ├─ #include "Common.dsh"        (256 行: sampler, MatrixInverse, 色彩空间...)
  ├─ #include "MorphTarget.dsh"   (Morph Target 计算)
  ├─ #include "Skeleton.dsh"      (骨骼蒙皮)
  └─ #include "shared/common.h"   (通用结构体: ModelInfo, MaterialInfo, LightInfo...)
```

### Phase 3 目标：Slang 模块

```slang
// Common.slang — 公共数学/颜色工具
public float3 LinearRGB_2_XYZ(float3 c) { ... }
public float ViewSpaceDepth(float z, float n, float f) { ... }

// BRDF.slang — BRDF 光照模型
import Common;
public float3 CalcSpecularBRDF(...) { ... }
public float3 CalcDiffuseBRDF(...) { ... }

// Lighting.slang — 完整光照计算
import Common;
import BRDF;
public float3 CalcLightingPhong(...) { ... }
public float3 CalcLightingBRDF(...) { ... }

// MeshRenderingVS.slang — 顶点着色器入口
import Common;
import MorphTarget;
import Skeleton;

[shader("vertex")]
VSOutput main(VSInput input) { ... }
```

### 模块与 #include 对比

| | `#include` | `import` |
|---|---|---|
| 语义 | 文本拼接（C preprocessor） | 符号导入（命名空间语义） |
| 编译模型 | 每文件独立做预处理 | 模块独立编译，结果可缓存 |
| 可见性 | 全部公开 | `public`/`internal` 控制导出 |
| 依赖图 | 隐式 (include 链) | 显式 (import 声明) |
| 增量编译 | 不可（任何 include 变更触发全量重编译） | 可（模块级缓存，只重编译消费者） |
| 符号冲突 | 宏/命名冲突风险 | 模块命名空间隔离 |
| 重定义保护 | `#pragma once`（容易遗漏） | 语言级保证 |

### Phase 3 改动清单

**文件重命名：**
- `Common.dsh` → `Common.slang`
- `BRDF.dsh` → `BRDF.slang`
- `Lighting.dsh` → `Lighting.slang`
- `Shadow.dsh` → `Shadow.slang`
- `IBL.dsh` → `IBL.slang`
- `MorphTarget.dsh` → `MorphTarget.slang`
- `Skeleton.dsh` → `Skeleton.slang`
- `ToneMapping.dsh` → `ToneMapping.slang`
- `ParticleCommon.dsh` → `ParticleCommon.slang`
- `CalcSDF.dsh` → `CalcSDF.slang`
- 所有 `.dsf` 入口文件 → `.slang`
- `shared/*.h` → 合并到对应 `.slang` 模块或改为公共模块

**语法变更（最小）：**
- `#include "X.dsh"` → `import X;`
- `#pragma once` → 删除（模块自然隔离）
- `#define` 常量 → `static const` 或枚举

**ShaderCompiler 适配：**
- 模块依赖图构建 → 增量编译
- `loadModule` 改为按依赖顺序加载
- 添加 `--cache-dir` 选项支持模块编译缓存

---

## 风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| Slang 对某些 HLSL 语法的兼容性不足 | 部分 Shader 编译失败 | Phase 1 中逐文件验证，保留 ShaderConductor 作为应急回退 |
| Slang 反射信息与 ShaderConductor 差异 | 引擎参数绑定错乱 | Phase 1 完成后用现有 Sample 做 Golden Image 回归测试 |
| 泛型接口增加 Shader 复杂度 | 调试困难 | Phase 2 每批次只迁移 2-3 个 Shader，逐批验证 |
| slang.dll 平台兼容性 | macOS/Linux 构建问题 | 验证 Slang 在目标平台的预编译库可用性 |
| 变体系统迁移不完整 | 某些边缘组合渲染错误 | Phase 2 不删除 `//PREDEFINE:` 支持，泛型和宏变体可共存 |

## 成功标准

**Phase 1：**
- [ ] ShaderCompiler 成功用 slangc 编译所有 153 个 Shader 文件
- [ ] 输出 .compile / .reflect / .meta 格式与原 ShaderConductor 输出一致
- [ ] 所有 8 个 Sample 在 D3D11 后端渲染结果与迁移前完全一致（截图对比）
- [ ] ShaderConductor 依赖完全移除

**Phase 2：**
- [ ] 至少 80% 的 `//PREDEFINE:` 被 Slang generics/specialization 替代
- [ ] Shader 变体编译总数减少 50% 以上
- [ ] 所有 Sample 渲染结果不变
- [ ] D3D12 后端启动时间有可测量的减少（变体减少导致 PSO 缓存命中率提升）

**Phase 3：**
- [ ] 所有 `#include` 替换为 `import`
- [ ] 增量 Shader 编译时间减少 50% 以上
- [ ] 所有 Sample 渲染结果不变
