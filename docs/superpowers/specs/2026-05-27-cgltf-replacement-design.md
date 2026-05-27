# 用 cgltf 替换自研 glTF 加载器 — 设计方案

## 目标

用开源单头文件 C 库 [cgltf](https://github.com/jkuhlmann/cgltf) 替换 `engine/importer/gltf2_loader.cpp` 中基于 rapidjson 的手写 glTF 2.0 解析逻辑，保留将数据转换为引擎内部类型的转换层。

## 动机

- 当前 loader ~1600 行手写 JSON 解析 + GLB 解析 + buffer 加载
- 扩展支持不完整（KHR_lights_punctual 被注释掉，不支持 Draco/BasisU）
- cgltf 生态广泛、零依赖、扩展支持全面

## 改动范围

### 涉及文件

| 文件 | 改动类型 |
|------|----------|
| `engine/importer/loader.h` | 简化：移除 GetModelDoc()、TickSceneFromJson()、rapidjson 依赖 |
| `engine/importer/gltf2_loader.h` | 重写：用 cgltf_data 替换中间类型 |
| `engine/importer/gltf2_loader.cpp` | 重写：解析层换成 cgltf 调用，保留转换层 |
| `engine/importer/gltf2.h` | 保留：枚举转换函数保留，去掉不再需要的宏 |
| `engine/seek_engine.h` | 不变（include 路径不变） |
| `samples/common/app_framework.cpp` | 不变（调用方式不变） |
| `engine/CMakeLists.txt` | 微调：确保 cgltf.h 在 include path |

### 不涉及

- 引擎内部类型（SceneComponent、MeshComponent、MaterialResource 等）
- 渲染管线（RHI/D3D11）
- 加载器的 3 个 map 访问器接口

## 设计

### 1. Loader 基类简化

移除两个无用虚函数和 rapidjson 依赖：

```cpp
// loader.h — 改后
#pragma once
#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

class Loader
{
public:
    virtual SResult LoadSceneFromFile(std::string const& filePath,
                                      SceneComponentPtr& scene,
                                      std::vector<AnimationComponentPtr>& anim) = 0;

    virtual std::map<uint32_t, MeshComponentPtr>&     GetMeshComponentMap() = 0;
    virtual std::map<uint32_t, SceneComponentPtr>&    GetSceneComponentMap() = 0;
    virtual std::map<std::string, SceneComponentPtr>& GetSceneComponentMapByName() = 0;
};

SEEK_NAMESPACE_END
```

- `GetModelDoc()` 和 `TickSceneFromJson()` 确认无外部调用方，直接移除
- `#include "rapidjson/document.h"` 移除

### 2. cgltf 集成，移除中间解析类型

#### 引入 cgltf

- 将 `cgltf.h` 放入 `engine/third_party/cgltf/` 目录（单头文件）
- CMakeLists.txt 添加该目录到 include path
- `gltf2_loader.cpp` 头部 `#define CGLTF_IMPLEMENTATION` 后 `#include "cgltf.h"`

#### 可移除的中间类型（被 cgltf 结构替代）

| 当前类型 | cgltf 替代 |
|----------|-----------|
| `LoaderBuffer` | `cgltf_buffer` (数据直接用 `.data` 指针) |
| `LoaderBufferView` | `cgltf_buffer_view` |
| `LoaderAccessor` | `cgltf_accessor` |
| `LoaderAnimSampler` | `cgltf_animation_sampler` + `channel` |
| `GLBInfo / GLBHeader / GLBChunk` | cgltf 内部处理 |

#### 可移除的方法

`LoadBuffer`、`LoadBufferView`、`LoadAccessor`、`LoadAnimationSampler`、`LoadUriFile`、`IsURIData`、`DecodeDataFromURI`、`LoadExtensionsUsed`

GLB 解析、压缩文件解压、data URI base64 解码、URI 文件加载全部由 `cgltf_parse_file` + `cgltf_load_buffers` 完成。

#### 保留的中间类型

- `LoaderTexture` — 轻量包装
- `LoaderImage` — bitmap 解码结果封装
- `LoaderSkin` — 引擎 joint/skeleton 引用集合

#### LoadSceneFromFile 改造

```cpp
// 之前: ~100行 GLB检测+解压+JSON解析+buffer加载
// 之后:
cgltf_options options = {};
cgltf_data* data = nullptr;
cgltf_result result = cgltf_parse_file(&options, filePath.c_str(), &data);
if (result != cgltf_result_success) return ERR_INVALID_ARG;
result = cgltf_load_buffers(&options, data, filePath.c_str());
if (result != cgltf_result_success) { cgltf_free(data); return ERR_INVALID_ARG; }
result = cgltf_validate(data);
// ... 转换 data → 引擎类型 ...
cgltf_free(data);
```

### 3. 转换层（保留 + 改写）

核心原则：方法签名从 `rapidjson::Value&` 改为 `cgltf_*` 对应类型，转换逻辑参数来源从 JSON key 查找改为读 cgltf 结构字段。

#### LoadNode(cgltf_node*)

- `matrix`/`rotation`/`scale`/`translation` 从 `cgltf_node` 的 float 数组字段直接读
- children 遍历：`cgltf_node*` 指针链
- mesh 检测：`node->mesh` 非空
- skin 检测：`node->skin` 非空

#### LoadPrimitive(cgltf_primitive*)

- attributes 遍历：`primitive->attributes[i].name` + `.type` 枚举（cgltf 已解析为 `cgltf_attribute_type`）
- 顶点数据指针：`cgltf_buffer_view_accessor_read_float/element` 或直接从 `cgltf_accessor` 的 `buffer_view->buffer->data + offset` 读取
- `gltf::ConvertToVertexFormat` 等转换函数保留，参数从 JSON 枚举值改为 cgltf 的 `cgltf_component_type` / `cgltf_type`
- 注意：cgltf 的枚举值与 glTF 规范值相同，`gltf2.h` 中的 `_component_type_map` 方向可简化
- joint index 转换（uint8/uint16 → uint32）逻辑保留

#### LoadMaterial(cgltf_material*)

- PBR 参数直接从 `material->pbr_metallic_roughness` 的字段读取
- 纹理引用通过 `material->pbr_metallic_roughness.base_color_texture.texture` 等获取
- 扩展（clearcoat、sheen、ior）通过 `material->has_clearcoat` 等标志和对应字段读取

#### LoadTexture / LoadImage / LoadSkin / LoadAnimation

- 输入改为 cgltf 对应类型指针
- Image 的 buffer 数据：cgltf 已处理 bufferView/URI，直接拿 `image->buffer_view->buffer->data` 喂给 `ImageDecode`
- Skin：`cgltf_skin` 提供 `inverse_bind_matrices` accessor
- Animation：遍历 `cgltf_animation` 的 channels + samplers

### 4. 枚举映射 (gltf2.h)

`gltf2.h` 中的转换函数保留，修改参数类型：

- `ConvertToTopologyType`: `int mode` → `cgltf_primitive_type`
- `ConvertToIndexBufferType`: `int componentType` → `cgltf_component_type`
- `ConvertToElementType`: 保留（从 cgltf_type 转）
- `ConvertToInterpolationType`: 保留（从 cgltf_interpolation_type 转）
- `ConvertVertexAttribute`: 保留，但增加 `cgltf_attribute_type` 重载
- `ConvertToVertexFormat`: 参数改为 `cgltf_type` + `cgltf_component_type`

不再需要的宏（如 `GLTF_MODE_*`、`GLTF_COMPONENT_TYPE_*` 等）可移除，直接使用 cgltf 枚举。

### 5. 图片加载

当前 `LoadImage` 接收 `rapidjson::Value`，内部处理 bufferView 和 URI 两种来源。cgltf 的 `cgltf_image` 已封装这些：

- `image->buffer_view` 非空 → buffer 嵌入数据，用 `cgltf_buffer_view` 的 data 指针
- `image->uri` 非空 → URI 外部文件，cgltf_load_buffers 已自动加载，同样用 buffer_view 访问
- PNG/JPEG 解码继续使用现有的 `ImageDecode()`（turbojpeg + libpng）

## 风险与注意事项

1. **cgltf 内存管理**：`cgltf_data` 由 cgltf 内部分配，转换完成后需要 `cgltf_free()`。转换过程中创建的引擎对象（SceneComponent 等）有独立生命周期，必须在 `cgltf_free` 之前完成数据拷贝。

2. **性能**：cgltf 以性能著称（内存映射、零拷贝），理论上不应引入回归。`cgltf_load_buffers` 中的文件 I/O 行为与当前实现相似。

3. **扩展兼容性**：cgltf 支持的扩展比当前 loader 多（Draco、KHR_texture_basisu、KHR_mesh_quantization 等），这些扩展在引擎不支持时会返回错误或忽略，不会影响现有功能。

4. **rapidjson**：CMakeLists.txt 中的 rapidjson 依赖暂时保留——如果项目其他地方使用了 rapidjson，不能直接删除。后续可单独清理。

5. **cgltf.h 版本管理**：建议固定 cgltf 版本（commit hash），避免随 master 变动引入意外行为。
