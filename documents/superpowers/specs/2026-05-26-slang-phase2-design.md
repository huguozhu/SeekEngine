# Slang Phase 2: SpecializationConstant 替代 PREDEFINE 变体系统

## 目标

用 Slang 的 `[SpecializationConstant]` 特性替代 `//PREDEFINE:` 笛卡尔积变体系统，将 78 个变体的独立编译降为 9 个 Shader + PSO 链接时特化。保持所有 Sample 渲染结果不变。

## 架构

```
.dsf 源码                               引擎运行时
  │                                       │
  │ [SpecializationConstant]              │ MeshComponent::OnRenderBegin()
  │   int g_JointBindSize = 0;            │ → 查询当前材质/骨骼/TAA状态
  │                                       │ → 写入 Technique specialization
  │ if (g_JointBindSize > 0) { ... }     │
  │                                       ▼
  ▼                                     Technique
Slang 编译                                │ specialization values
  │                                       ▼
  ▼                                     RHIProgram / PSO
.reflect JSON                             │ CreateGraphicsPipelineState()
  "specializations": [                   │ with specialization constants
    {"name":"g_JointBindSize",            ▼
     "type":"int", "default":"0"},      实际 PSO（链接时特化代码）
    ...
  ]
```

## 数据结构

### ReflectInfo 扩展 (shader_helper.h)

```cpp
struct SpecializationConstantInfo {
    std::string name;         // "g_JointBindSize"
    std::string typeName;     // "int", "bool", "float"
    std::string defaultValue; // "0", "false"
};

// ReflectInfo 新增字段
std::vector<SpecializationConstantInfo> specializations;
```

### EffectParam 扩展 (effect/parameter.h)

```cpp
struct SpecializationValue {
    std::string name;   // 匹配 SpecializationConstantInfo::name
    int32_t intValue;
};

// EffectParam 新增类型
enum class EffectDataType : uint8_t {
    // ... existing types ...
    Specialization,   // NEW
};
```

## Shader 改写清单（9 个文件）

| 文件 | 删除 PREDEFINE | 新增 SpecializationConstant |
|------|---------------|---------------------------|
| MeshRenderingVS | JOINT_BIND_SIZE(3)/ENABLE_TAA/HAS_MATERIAL_NORMAL/MORPH_TYPE | g_JointBindSize(int)/g_EnableTAA(bool)/g_HasNormal(bool)/g_MorphType(int) |
| PreZMeshRenderingVS | JOINT_BIND_SIZE(3)/MORPH_TYPE | g_JointBindSize(int)/g_MorphType(int) |
| ForwardRenderingCommonPS | LIGHT_MODE/HAS_MATERIAL_ALBEDO/NORMAL/METALLIC_ROUGHNESS/NORMAL_MASK | g_LightMode(int)/g_HasAlbedoTex(bool)/g_HasNormalTex(bool)/g_HasMetallicRoughTex(bool)/g_HasNormalMaskTex(bool) |
| GenerateGBufferPS | HAS_MATERIAL_NORMAL/ENABLE_TAA | g_HasNormal(bool)/g_EnableTAA(bool) |
| GenerateRsmPS | HAS_MATERIAL_NORMAL | g_HasNormal(bool) |
| DeferredLightingPS | HAS_SHADOW/TILE_CULLING | g_HasShadow(bool)/g_TileCulling(bool) |
| ParticleRenderPS | HAS_TEX | g_HasTex(bool) |
| ParticleRenderVS | HAS_TEX | g_HasTex(bool) |
| ShadowingDirectionalPS | USE_CSM | g_UseCsm(bool) |

## 引擎改动（5 个文件）

### 1. shader_helper.h — 反射数据结构
- 新增 `SpecializationConstantInfo` struct
- `ReflectInfo` 新增 `vector<SpecializationConstantInfo> specializations`
- `ReflectJsonWriter/Reader` 新增 specialization 序列化

### 2. ShaderCompiler.cpp — Slang 反射提取
- `ParseSlangReflection()` 新增 specialization constants 提取
- 遍历 `layout->getParameterCount()` → 识别 `ParameterCategory::SpecializationConstant` → 写入 ReflectInfo

### 3. effect/parameter.h — 效果参数扩展
- `EffectDataType` 新增 `Specialization`
- `EffectParam` 新增 `std::vector<SpecializationValue> specializationValues`

### 4. effect/technique.cpp — 特化值绑定
- `Technique::Build()` 读取 ReflectInfo.specializations → 创建 EffectParam
- `Technique::SetSpecializationConstant(name, value)` 公共接口
- 绘制前从 Mesh/Context 查询当前值并写入

### 5. rhi/d3d12/d3d12_program.cpp — PSO 特化
- `D3D12Program` 存储 specialization map
- `CreateGraphicsPipelineState` 时传入 specialization 数据
- D3D11/Vulkan 的 Program 同理（后续补齐）

## 关键设计决策

### 1. `#if` → `if` 的保证
Slang 对 `if (specializationConstant)` 做**编译时常量折叠**——PSO 链接时确定值，生成代码与 `#if` 版本等效（无运行时分支开销）。

### 2. 向后兼容
`//PREDEFINE:` 注解在 Phase 2 **不删除解析代码**（iterateFunc 仍可用）。Shaders 逐个迁移，PREDEFINE 和 SpecializationConstant 可共存于同一 ShaderCompiler 进程。

### 3. 反射序列化
```json
{
  "reflect": {
    "specializations": [
      {"name": "g_JointBindSize", "type": "int", "default": "0"},
      {"name": "g_EnableTAA", "type": "bool", "default": "false"}
    ]
  }
}
```
引擎通过 name 匹配（而非 binding slot）来设置特化值。

## 验证计划

1. 每个 Shader 改写后：`ShaderCompiler --input X.dsf --target hlsl` → 确认输出无 PREDEFINE 宏展开（只有 1 个变体文件）
2. 所有 9 个 Shader 改写后：`MeshRenderingVS.dsf` 应输出仅 1 个变体（无 `_<hash>` 后缀的文件名）
3. 引擎侧：Sample 1 (Tutorial) 渲染结果截图对比（改造前后一致）
4. Sample 4 (Lighting) 不同光源类型下的阴影效果回归测试

## 风险

| 风险 | 缓解 |
|------|------|
| Slang specialization constant API 变更 | Phase 1 已集成 Slang，API 稳定性已验证 |
| 特化值传递链路长（Mesh→Technique→Program→PSO） | 每步有默认值，链路断裂不崩溃 |
| `if(SC)` 在旧 D3D11 后端可能有运行时开销 | Slang 输出 HLSL 源码，D3DCompile 会做常量折叠 |

## 成功标准

- [ ] 所有 9 个带 PREDEFINE 的 .dsf 文件改为 SpecializationConstant
- [ ] ShaderCompiler 编译每个 Shader 仅产生 1 个变体
- [ ] .reflect 文件包含 specializations 字段
- [ ] 引擎 Technique 支持查询和设置 specialization values
- [ ] Sample 1-5 渲染结果与 Phase 1 一致
