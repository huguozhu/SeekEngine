# cgltf 替换自研 glTF 加载器 — 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 用 cgltf 替换 engine/importer/gltf2_loader.cpp 中的手写 rapidjson 解析逻辑

**Architecture:** cgltf 负责 glTF/GLB 解析和 buffer 加载，glTF2_Loader 负责将 cgltf 的 C 结构体转换为引擎的 SceneComponent/MeshComponent/MaterialResource 等类型

**Tech Stack:** C++20, CMake, cgltf (single-header C library), D3D11

---

### Task 1: 下载 cgltf.h 并集成到构建系统

**Files:**
- Create: `engine/third_party/cgltf/cgltf.h`

- [ ] **Step 1: 下载 cgltf.h**

```powershell
$url = "https://raw.githubusercontent.com/jkuhlmann/cgltf/master/cgltf.h"
$dest = "engine/third_party/cgltf/cgltf.h"
if (-not (Test-Path "engine/third_party/cgltf")) { New-Item -ItemType Directory -Force -Path "engine/third_party/cgltf" }
Invoke-WebRequest -Uri $url -OutFile $dest
```

- [ ] **Step 2: 更新 engine/CMakeLists.txt，添加 cgltf include path**

修改 `engine/CMakeLists.txt:257`，在 `target_include_directories` 行后追加 cgltf 目录：

```cmake
target_include_directories(${SEEK_STATICLIB_NAME} PUBLIC
    ${SEEK_ENGINE_SOURCE_DIR}
    ${SEEK_ENGINE_SOURCE_DIR}/third_party/cgltf
)
```

现有代码位置（`engine/CMakeLists.txt:257`）：
```cmake
target_include_directories(${SEEK_STATICLIB_NAME} PUBLIC ${SEEK_ENGINE_SOURCE_DIR})
```

替换为：
```cmake
target_include_directories(${SEEK_STATICLIB_NAME} PUBLIC
    ${SEEK_ENGINE_SOURCE_DIR}
    ${SEEK_ENGINE_SOURCE_DIR}/third_party/cgltf
)
```

- [ ] **Step 3: 确认 rapidjson 在 engine 的外观**

用 `git grep "rapidjson" -- engine/` 确认 engine 目录下使用 rapidjson 的文件仅限于 `engine/importer/loader.h`、`engine/importer/gltf2_loader.h`、`engine/importer/gltf2_loader.cpp`。若如此，后续步骤中可将 `rapidjson` 从 engine 的 `target_link_libraries` 中移除。

---

### Task 2: 简化 loader.h

**Files:**
- Modify: `engine/importer/loader.h`

- [ ] **Step 1: 用以下内容替换 loader.h**

```cpp
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

CLASS_DECLARE(Loader);

SEEK_NAMESPACE_END
```

变化：
- 移除 `#include "rapidjson/document.h"`
- 移除 `virtual SResult TickSceneFromJson(std::string const& jsonStr) = 0;`
- 移除 `virtual rapidjson::Document& GetModelDoc() = 0;`

---

### Task 3: 重写 gltf2_loader.h

**Files:**
- Modify: `engine/importer/gltf2_loader.h`

- [ ] **Step 1: 用以下内容替换 gltf2_loader.h**

```cpp
#pragma once

#include "seek_engine.h"
#include "resource/resource_mgr.h"
#include "importer/loader.h"
#include "components/animation_component.h"

SEEK_NAMESPACE_BEGIN

struct BufferResource;

// 保留的轻量中间类型（引擎特有）
struct LoaderTexture
{
    int32_t imageIndex = -1;
    int32_t samplerIndex = -1;
    int32_t texCoord = 0;
    float scale = 1.0f;
};
using LoaderTexturePtr = std::shared_ptr<LoaderTexture>;

struct LoaderImage
{
    std::string mimeType;
    std::string uriPath; // 绝对路径，非 URI 时为空
    BitmapBufferPtr bitmapData = nullptr;
};
using LoaderImagePtr = std::shared_ptr<LoaderImage>;

struct LoaderSkin
{
    std::vector<Matrix4> inverseBindMatrices;
    std::vector<SceneComponentPtr> joints;
    SceneComponentPtr skeleton;
};
using LoaderSkinPtr = std::shared_ptr<LoaderSkin>;

// cgltf 前置声明
struct cgltf_data;

class glTF2_Loader : public Loader
{
public:
    glTF2_Loader(Context* ctx);
    virtual ~glTF2_Loader();

    virtual SResult LoadSceneFromFile(std::string const& filePath,
                                      SceneComponentPtr& scene,
                                      std::vector<AnimationComponentPtr>& anim) override;

    virtual std::map<uint32_t, MeshComponentPtr>&           GetMeshComponentMap() override;
    virtual std::map<uint32_t, SceneComponentPtr>&          GetSceneComponentMap() override;
    virtual std::map<std::string, SceneComponentPtr>&       GetSceneComponentMapByName() override;

private:
    // 入口
    SceneComponentPtr LoadDefaultScene(cgltf_data* data);
    SceneComponentPtr LoadScene(cgltf_data* data, uint32_t index);

    // 转换方法（从 cgltf 到引擎类型）
    SceneComponentPtr LoadNode(cgltf_data* data, uint32_t index);
    MeshComponentPtr  LoadMesh(cgltf_data* data, uint32_t mesh_index,
                               bool has_skin, uint32_t skin_index);
    LightComponentPtr LoadLight(cgltf_data* data, uint32_t light_index);
    RHIMeshPtr        LoadPrimitive(cgltf_data* data, struct cgltf_primitive* primitive);
    std::shared_ptr<MaterialResource> LoadMaterial(cgltf_data* data,
                                                   struct cgltf_material* material,
                                                   bool hasTangent);
    LoaderTexturePtr  LoadTexture(cgltf_data* data, uint32_t tex_index,
                                  struct cgltf_texture_view* tex_view);
    LoaderImagePtr    LoadImage(cgltf_data* data, uint32_t image_index, bool bColor);
    LoaderSkinPtr     LoadSkin(cgltf_data* data, uint32_t skin_index);
    void              LoadAnimation(cgltf_data* data,
                                    std::vector<AnimationComponentPtr>& animations);

    // 材质扩展
    bool LoadMetallicRoughness(struct cgltf_pbr_metallic_roughness* pbr,
                               MaterialResource& matRes);
    bool LoadClearcoat(struct cgltf_clearcoat* cc, MaterialResource& matRes);
    bool LoadSheen(struct cgltf_sheen* sheen, MaterialResource& matRes);

    // Morph target
    void ConverterToMorphStreamUnitFromTargets(RHIMeshPtr& mesh,
                                               struct cgltf_primitive* primitive,
                                               AABBox& targets_box);

    // 辅助：读取 accessor 数据指针和步长
    static const uint8_t* AccessorData(const struct cgltf_accessor* accessor);
    static uint32_t AccessorStride(const struct cgltf_accessor* accessor);

private:
    Context* m_pContext;

    std::map<uint32_t, SceneComponentPtr>                  m_Nodes;
    std::map<std::string, SceneComponentPtr>               m_NodesMapByName;
    std::map<uint32_t, MeshComponentPtr>                   m_Meshes;
    std::map<uint32_t, LightComponentPtr>                  m_Lights;
    std::map<uint32_t, std::shared_ptr<MaterialResource>>  m_Materials;
    std::map<uint32_t, LoaderTexturePtr>                   m_Textures;
    std::map<uint32_t, LoaderImagePtr>                     m_Images;
    std::map<uint32_t, LoaderSkinPtr>                      m_Skins;

    bool m_isLoadSucceed;
    std::string m_filePath;

    // 保持 backend buffer 存活（cgltf buffer data 的生命周期管理）
    std::vector<std::shared_ptr<BufferResource>> m_BuffersResources;
};

SEEK_NAMESPACE_END
```

变化：
- 移除 `#include "rapidjson/document.h"` 和 `#include "importer/gltf2.h"`
- 移除 `LoaderBuffer`、`LoaderBufferView`、`LoaderAccessor`、`LoaderAnimSampler` 四个中间类型
- 移除所有和这几个类型相关的 map 成员（m_Buffers, m_BufferViews, m_Accessors, m_AnimSamplers）
- 移除 `GLBInfo`、`m_isGLBFormat`、`m_Doc`、`m_ExtensionsUsed`、`m_gltfResource`、`m_fileRes`、`m_uriFileResources`
- 移除 `LoadBuffer`、`LoadBufferView`、`LoadAccessor`、`LoadAnimationSampler`、`LoadUriFile`、`GetUriAbsolutePath`、`LoadExtensionsUsed`、`GetModelDoc`、`TickSceneFromJson`
- 所有转换方法的输入参数从 `rapidjson::Value&` 改为 cgltf 类型
- 新增 `AccessorData` / `AccessorStride` 辅助方法
- `LoadTexture` 增加 `cgltf_texture_view*` 参数，cgltf 的纹理引用中提取 texCoord/scale
- `LoadPrimitive` 改为接收 `cgltf_primitive*` 而非 `Value&`；`LoadSceneFromFile` 用 `cgltf_data` 统一传递

---

### Task 4: 重写 gltf2_loader.cpp — 基础结构

**Files:**
- Modify: `engine/importer/gltf2_loader.cpp`

- [ ] **Step 1: 替换文件顶部 includes 和全局宏**

将行 1-16：
```cpp
#include "gltf2_loader.h"
#include "utils/image_decode.h"
#include "utils/log.h"
#include "utils/zbase64.h"
#include "utils/timer.h"
#include "utils/error.h"
#include "math/math_utility.h"
#include <fstream>
#include "zlib.h"

using namespace std;
using namespace seek_engine;
using namespace rapidjson;

#define SEEK_MACRO_FILE_UID 81     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#define RETURN_IF_FOUND(map, key) \
    { \
        auto i = (map).find(key); \
        if (i != (map).end()) { \
            return i->second; \
        } \
    }

#define INIT_AND_RETURN_IF_FOUND(vec, objname, index)       \
    if (vec.empty())                                        \
    {                                                       \
        if (m_Doc.HasMember(objname))                       \
        {                                                   \
            Value& v = m_Doc[objname];                      \
            if (v.IsArray())                                \
                vec.resize(v.Size());                       \
        }                                                   \
    }                                                       \
    if (index >= vec.size())                                \
        return nullptr;                                     \
    if (vec[index])                                         \
        return vec[index];
```

替换为：
```cpp
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "gltf2_loader.h"
#include "utils/image_decode.h"
#include "utils/log.h"
#include "utils/timer.h"
#include "utils/error.h"
#include "math/math_utility.h"

using namespace std;
using namespace seek_engine;

#define SEEK_MACRO_FILE_UID 81     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#define RETURN_IF_FOUND(map, key) \
    { \
        auto i = (map).find(key); \
        if (i != (map).end()) { \
            return i->second; \
        } \
    }
```

变化：
- 移除 `zbase64.h`、`zlib.h`、`<fstream>`（cgltf 内部处理 base64/deflate/文件读取）
- 移除 `using namespace rapidjson;`、`INIT_AND_RETURN_IF_FOUND` 宏
- 添加 `#define CGLTF_IMPLEMENTATION` + `#include "cgltf.h"`

- [ ] **Step 2: 替换构造函数和析构函数（行 42-52）**

保持不变，只需确保构造函数匹配 header：
```cpp
glTF2_Loader::glTF2_Loader(Context* ctx)
    : m_pContext(ctx)
{
    m_isLoadSucceed = true;
}

glTF2_Loader::~glTF2_Loader()
{
}
```

---

### Task 5: 重写 gltf2_loader.cpp — LoadSceneFromFile + AccessorData/Stride

**Files:**
- Modify: `engine/importer/gltf2_loader.cpp`

- [ ] **Step 1: 重写 LoadSceneFromFile（替换行 54-149）**

```cpp
const uint8_t* glTF2_Loader::AccessorData(const cgltf_accessor* accessor)
{
    if (!accessor || !accessor->buffer_view)
        return nullptr;
    cgltf_buffer_view* bv = accessor->buffer_view;
    return static_cast<const uint8_t*>(bv->buffer->data)
           + accessor->offset + bv->offset;
}

uint32_t glTF2_Loader::AccessorStride(const cgltf_accessor* accessor)
{
    if (!accessor)
        return 0;
    if (accessor->stride > 0)
        return accessor->stride;
    return gltf::ElementSize((int)accessor->component_type, (int)accessor->type);
}

SResult glTF2_Loader::LoadSceneFromFile(std::string const& filePath,
                                         SceneComponentPtr& scene,
                                         vector<AnimationComponentPtr>& animations)
{
    SResult ret = S_Success;
    LOG_INFO("glTF2_Loader: gltf file path: %s", filePath.c_str());

    m_filePath = filePath;

    TIMER_BEG(t0);
    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result cres = cgltf_parse_file(&options, filePath.c_str(), &data);
    if (cres != cgltf_result_success)
    {
        LOG_ERROR("glTF2_Loader: cgltf_parse_file fail");
        return ERR_INVALID_ARG;
    }
    TIMER_END(t0, "glTF2_Loader: parse file");

    TIMER_BEG(t1);
    cres = cgltf_load_buffers(&options, data, filePath.c_str());
    if (cres != cgltf_result_success)
    {
        LOG_ERROR("glTF2_Loader: cgltf_load_buffers fail");
        cgltf_free(data);
        return ERR_INVALID_ARG;
    }
    TIMER_END(t1, "glTF2_Loader: load buffers");

    TIMER_BEG(t2);
    cres = cgltf_validate(data);
    if (cres != cgltf_result_success)
        LOG_WARNING("glTF2_Loader: cgltf_validate warning");
    TIMER_END(t2, "glTF2_Loader: validate");

    // 为 buffer 创建生命周期管理对象（确保 cgltf_free 前 buffer 数据不被释放）
    m_BuffersResources.clear();
    for (size_t i = 0; i < data->buffers_count; i++)
    {
        auto bufRes = MakeSharedPtr<BufferResource>();
        bufRes->_data = static_cast<uint8_t*>(data->buffers[i].data);
        bufRes->_size = data->buffers[i].size;
        m_BuffersResources.push_back(std::move(bufRes));
    }

    TIMER_BEG(t3);
    SceneComponentPtr rootSceneComponent = LoadDefaultScene(data);
    if (rootSceneComponent)
    {
        rootSceneComponent->SetName(filePath);
        scene = rootSceneComponent;

        animations.clear();
        LoadAnimation(data, animations);
    }
    TIMER_END(t3, "glTF2_Loader: load scene");

    cgltf_free(data);
    return S_Success;
}
```

---

### Task 6: 重写 gltf2_loader.cpp — LoadDefaultScene / LoadScene / LoadNode

**Files:**
- Modify: `engine/importer/gltf2_loader.cpp`

- [ ] **Step 1: 重写 LoadDefaultScene 和 LoadScene（替换行 151-215）**

```cpp
SceneComponentPtr glTF2_Loader::LoadDefaultScene(cgltf_data* data)
{
    SceneComponentPtr rootSC = nullptr;
    if (data->scene)
    {
        uint32_t index = (uint32_t)(data->scene - data->scenes);
        rootSC = LoadScene(data, index);
    }
    return rootSC;
}

SceneComponentPtr glTF2_Loader::LoadScene(cgltf_data* data, uint32_t index)
{
    SceneComponentPtr sc = nullptr;
    if (index < data->scenes_count)
    {
        cgltf_scene* scene = &data->scenes[index];
        sc = MakeSharedPtr<SceneComponent>(m_pContext);

        for (size_t i = 0; i < scene->nodes_count; i++)
        {
            cgltf_node* node = scene->nodes[i];
            uint32_t node_idx = (uint32_t)(node - data->nodes);
            SceneComponentPtr child = this->LoadNode(data, node_idx);
            if (child)
            {
                sc->AddChild(child);
                child->SetParent(sc.get());
            }
        }

        if (scene->name && sc)
            sc->SetName(scene->name);
    }
    return sc;
}
```

- [ ] **Step 2: 重写 LoadNode（替换行 220-333）**

```cpp
#define JsonValue2Float4(v) float4(v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat(), v[3].GetFloat())
#define JsonValue2Float3(v) float3(v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat())

SceneComponentPtr glTF2_Loader::LoadNode(cgltf_data* data, uint32_t index)
{
    RETURN_IF_FOUND(m_Nodes, index);
    SceneComponentPtr sc = nullptr;

    if (index >= data->nodes_count)
        return sc;

    cgltf_node* node = &data->nodes[index];

    if (node->mesh)
    {
        uint32_t mesh_index = (uint32_t)(node->mesh - data->meshes);
        if (node->skin)
        {
            uint32_t skin_index = (uint32_t)(node->skin - data->skins);
            sc = this->LoadMesh(data, mesh_index, true, skin_index);
        }
        else
        {
            sc = this->LoadMesh(data, mesh_index, false, 0);
        }
    }
    else
        sc = MakeSharedPtr<SceneComponent>(m_pContext);

    m_Nodes[index] = sc;

    // 矩阵
    if (node->has_matrix)
    {
        Matrix4 mat4 = Matrix4(node->matrix);
        sc->SetLocalTransform(mat4);
    }

    // TRS
    if (node->has_rotation)
    {
        Quaternion qua = Quaternion(node->rotation[0], node->rotation[1],
                                    node->rotation[2], node->rotation[3]);
        sc->SetLocalRotation(qua);
    }
    if (node->has_scale)
    {
        float3 scale = float3{node->scale[0], node->scale[1], node->scale[2]};
        sc->SetLocalScale(scale);
    }
    if (node->has_translation)
    {
        float3 pos = float3{node->translation[0], node->translation[1],
                            node->translation[2]};
        sc->SetLocalTranslation(pos);
    }

    // 名称
    if (node->name)
        sc->SetName(node->name);

    // 子节点
    for (size_t i = 0; i < node->children_count; i++)
    {
        uint32_t child_index = (uint32_t)(node->children[i] - data->nodes);
        SceneComponentPtr childNode = this->LoadNode(data, child_index);
        if (childNode)
        {
            sc->AddChild(childNode);
            childNode->SetParent(sc.get());
        }
    }

    if (sc->GetName() != "")
        m_NodesMapByName[sc->GetName()] = sc;

    return sc;
}
```

保留 `JsonValue2Float4` / `JsonValue2Float3` 宏，后续任务中确认若已无引用则删除。

---

### Task 7: 重写 gltf2_loader.cpp — LoadMesh + LoadPrimitive

**Files:**
- Modify: `engine/importer/gltf2_loader.cpp`

- [ ] **Step 1: 重写 LoadMesh（替换行 335-432）**

```cpp
MeshComponentPtr glTF2_Loader::LoadMesh(cgltf_data* data, uint32_t mesh_index,
                                         bool has_skin, uint32_t skin_index)
{
    RETURN_IF_FOUND(m_Meshes, mesh_index);
    MeshComponentPtr meshComponent = nullptr;

    if (mesh_index >= data->meshes_count)
        return meshComponent;

    cgltf_mesh* mesh = &data->meshes[mesh_index];

    LoaderSkinPtr skin = nullptr;
    if (has_skin)
    {
        skin = LoadSkin(data, skin_index);
        SkeletalMeshComponentPtr sklMeshComponent =
            MakeSharedPtr<SkeletalMeshComponent>(m_pContext);
        sklMeshComponent->SetInverseBindMatrix(skin->inverseBindMatrices);
        sklMeshComponent->SetJoint(skin->joints);
        sklMeshComponent->SetSkeletonRoot(skin->skeleton);
        meshComponent = sklMeshComponent;
    }
    else
        meshComponent = MakeSharedPtr<MeshComponent>(m_pContext);

    m_Meshes[mesh_index] = meshComponent;

    // 名称
    if (mesh->name)
        meshComponent->SetName(mesh->name);

    // morph weights
    std::vector<float> weights;
    if (mesh->weights_count > 0)
    {
        weights.resize(mesh->weights_count);
        for (size_t i = 0; i < mesh->weights_count; i++)
            weights[i] = mesh->weights[i];
    }

    // morph target names (存储在 extras 中，cgltf 通过 target_names 暴露)
    std::vector<std::string> targetNames;
    if (mesh->target_names_count > 0)
    {
        targetNames.resize(mesh->target_names_count);
        for (size_t i = 0; i < mesh->target_names_count; i++)
        {
            if (mesh->target_names[i])
                targetNames[i] = mesh->target_names[i];
        }
    }

    // 遍历 primitives
    AABBox meshes_box;
    meshes_box.Min(float3(0.0f));
    meshes_box.Max(float3(0.0f));
    for (size_t i = 0; i < mesh->primitives_count; i++)
    {
        RHIMeshPtr rhiMesh = LoadPrimitive(data, &mesh->primitives[i]);
        if (rhiMesh)
        {
            meshComponent->AddMesh(rhiMesh);
            MorphInfo& morphTarget = rhiMesh->GetMorphTargetResource()._morphInfo;
            if (morphTarget.morph_target_names.size() == 0 && targetNames.size() != 0)
                morphTarget.morph_target_names = targetNames;
            if (weights.size() != 0)
                morphTarget.morph_target_weights = weights;

            AABBox mesh_box = rhiMesh->GetAABBox();
            if (i == 0)
                meshes_box = mesh_box;
            else
                meshes_box |= mesh_box;
        }
    }
    meshComponent->SetAABBox(meshes_box);

    return meshComponent;
}
```

- [ ] **Step 2: 重写 LoadPrimitive（替换行 491-689）**

```cpp
RHIMeshPtr glTF2_Loader::LoadPrimitive(cgltf_data* data,
                                        cgltf_primitive* primitive)
{
    RHIMeshPtr mesh = nullptr;
    if (primitive->attributes_count == 0)
        return mesh;

    RHIContext* rc = &m_pContext->RHIContextInstance();
    mesh = rc->CreateMesh();
    if (!mesh)
        return mesh;

    // 拓扑类型
    mesh->SetTopologyType(gltf::ConvertToTopologyType(primitive->type));

    // 遍历顶点属性
    VertexAttributeResource vertexAttributeRes;
    bool hasPosition = false;
    bool hasNormal = false;
    bool hasTexcoord = false;
    bool hasTangent = false;
    SkinningJointBindSize joint_bind_size = SkinningJointBindSize::None;

    std::map<uint32_t, VertexStream> vertexStreams;
    std::map<uint32_t, std::shared_ptr<BufferResource>> blendIndexBufferRes;

    for (size_t i = 0; i < primitive->attributes_count; i++)
    {
        cgltf_attribute* attr = &primitive->attributes[i];
        cgltf_accessor* accessor = attr->data;
        if (!accessor || !accessor->buffer_view)
            continue;

        cgltf_buffer_view* bv = accessor->buffer_view;
        auto vertexAttribute = gltf::ConvertVertexAttribute(attr->index, attr->type);
        uint32_t bvIndex = (uint32_t)(bv - data->buffer_views);

        VertexStream* vertexStream;
        {
            vertexStreams.emplace(bvIndex, VertexStream{});
            vertexStream = &vertexStreams[bvIndex];
            if (accessor->stride > 0)
                vertexStream->stride = accessor->stride;
            else
                vertexStream->stride = (uint32_t)gltf::ElementSize(
                    (int)accessor->component_type, (int)accessor->type);
            vertexStream->offset = 0;
        }

        VertexStreamLayout layout;
        layout.buffer_offset = accessor->offset;
        layout.format = gltf::ConvertToVertexFormat(
            (int)accessor->type, (int)accessor->component_type);
        layout.usage = vertexAttribute.first;
        layout.usage_index = vertexAttribute.second;

        // Joint index 特殊处理：uint8/uint16 → uint32
        if (vertexAttribute.first == VertexElementUsage::BlendIndex)
        {
            layout.format = VertexFormat::UInt4;
            uint32_t jointsDataSize = (uint32_t)accessor->count * 4;
            std::shared_ptr<uint32_t> _backJointData{
                new uint32_t[jointsDataSize], default_array_deleter<uint32_t>()
            };
            std::shared_ptr<BufferResource> _backJointBuf = MakeSharedPtr<BufferResource>();
            _backJointBuf->_uninitializer = [_backJointData](IResource*) mutable {
                _backJointData.reset();
            };
            _backJointBuf->_data = (uint8_t*)_backJointData.get();
            _backJointBuf->_size = jointsDataSize * sizeof(uint32_t);

            const uint8_t* src = AccessorData(accessor);
            if (accessor->component_type == cgltf_component_type_r_8u)
            {
                for (uint32_t j = 0; j < jointsDataSize; j++)
                    _backJointData.get()[j] = (uint32_t)src[j];
            }
            else if (accessor->component_type == cgltf_component_type_r_16u)
            {
                const uint16_t* src16 = (const uint16_t*)src;
                for (uint32_t j = 0; j < jointsDataSize; j++)
                    _backJointData.get()[j] = src16[j];
            }
            vertexStream->stride = 16;
            blendIndexBufferRes.emplace(bvIndex, std::move(_backJointBuf));
        }
        vertexStream->layouts.push_back(layout);

        if (vertexAttribute.first == VertexElementUsage::Position)
        {
            hasPosition = true;
            if (accessor->has_min && accessor->has_max)
            {
                AABBox mesh_box;
                mesh_box.Max(float3{accessor->max[0], accessor->max[1], accessor->max[2]});
                mesh_box.Min(float3{accessor->min[0], accessor->min[1], accessor->min[2]});
                mesh->SetAABBox(mesh_box);
            }
        }
        else if (vertexAttribute.first == VertexElementUsage::Normal)
            hasNormal = true;
        else if (vertexAttribute.first == VertexElementUsage::TexCoord)
            hasTexcoord = true;
        else if (vertexAttribute.first == VertexElementUsage::BlendWeight)
            joint_bind_size = (SkinningJointBindSize)((vertexAttribute.second + 1) * 4);
        else if (vertexAttribute.first == VertexElementUsage::Tangent)
            hasTangent = true;
    }

    // 组装 vertex streams
    for (auto& pair : vertexStreams)
    {
        vertexAttributeRes._vertexStreams.push_back(pair.second);
        if (blendIndexBufferRes.find(pair.first) != blendIndexBufferRes.end())
            vertexAttributeRes._vertexBuffers.push_back(blendIndexBufferRes[pair.first]);
        else
        {
            // 使用 cgltf buffer data 创建 BufferResource
            cgltf_buffer_view* bv = &data->buffer_views[pair.first];
            auto bufRes = MakeSharedPtr<BufferResource>();
            bufRes->_data = static_cast<uint8_t*>(bv->buffer->data) + bv->offset;
            bufRes->_size = bv->size;
            vertexAttributeRes._vertexBuffers.push_back(std::move(bufRes));
        }
    }
    mesh->SetVertexAttributeResource(vertexAttributeRes);
    mesh->SetSkinningJointBindSize(joint_bind_size);

    if (!hasNormal)
        LOG_ERROR("Lack normal in the mesh\n");
    if (!hasTexcoord)
        LOG_ERROR("Lack texcoord in the mesh\n");

    // 索引
    if (primitive->indices)
    {
        cgltf_accessor* indices_acc = primitive->indices;
        const uint8_t* indexData = AccessorData(indices_acc);
        std::shared_ptr<VertexIndicesResource> indicesResource =
            MakeSharedPtr<VertexIndicesResource>();
        indicesResource->_indexBufferType = gltf::ConvertToIndexBufferType(
            (int)indices_acc->component_type);
        indicesResource->_indexCount = (uint32_t)indices_acc->count;
        indicesResource->_data = indexData;
        indicesResource->_size = (size_t)indices_acc->count *
            (size_t)gltf::ComponentByteSize((int)indices_acc->component_type);
        mesh->SetIndexBufferResource(indicesResource);
    }

    // Morph targets
    if (primitive->targets_count > 0)
    {
        AABBox targets_box;
        targets_box.Min(float3(0.0f));
        targets_box.Max(float3(0.0f));
        this->ConverterToMorphStreamUnitFromTargets(mesh, primitive, targets_box);
        AABBox mesh_box = mesh->GetAABBox();
        mesh->SetAABBox(mesh_box + targets_box);
    }

    // 材质
    if (primitive->material)
    {
        uint32_t material_index = (uint32_t)(primitive->material - data->materials);
        std::shared_ptr<MaterialResource> material =
            LoadMaterial(data, primitive->material, hasTangent);
        if (material)
            mesh->SetMaterialResource(material);
        else
        {
            std::shared_ptr<MaterialResource> defaultMat =
                MakeSharedPtr<MaterialResource>();
            mesh->SetMaterialResource(defaultMat);
        }
    }
    else
    {
        MaterialPtr material = MakeSharedPtr<Material>();
        mesh->SetMaterial(material);
    }

    return mesh;
}
```

注意：`gltf::ComponentByteSize` 和 `gltf::ElementSize` 现在需要接受 cgltf 枚举值（与 glTF 规范值相同）。`gltf::ComponentByteSize` 当前签名
`int ComponentByteSize(int componentType)` 取 `componentType - GLTF_COMPONENT_TYPE_BYTE` 做索引，cgltf 的枚举值也以 5120 为起点，兼容。但调用时需显式 cast。

在后续 Task 11（更新 gltf2.h）中将函数参数类型从 `int` 更新为 cgltf 枚举。

---

### Task 8: 重写 gltf2_loader.cpp — LoadMaterial + PBR helpers + LoadLight

**Files:**
- Modify: `engine/importer/gltf2_loader.cpp`

- [ ] **Step 1: 重写 LoadMaterial（替换行 691-808）**

```cpp
std::shared_ptr<MaterialResource> glTF2_Loader::LoadMaterial(
    cgltf_data* data, cgltf_material* mat, bool hasTangent)
{
    // 使用 material index 做缓存
    uint32_t mat_index = (uint32_t)(mat - data->materials);
    RETURN_IF_FOUND(m_Materials, mat_index);

    std::shared_ptr<MaterialResource> material = MakeSharedPtr<MaterialResource>();

    if (mat->name)
        material->_name = mat->name;

    // PBR metallic-roughness
    if (mat->has_pbr_metallic_roughness)
        this->LoadMetallicRoughness(data, &mat->pbr_metallic_roughness, *material.get());

    // normal texture
    if (mat->normal_texture.texture)
    {
        if (hasTangent)
        {
            LoaderTexturePtr tex = this->LoadTexture(
                data, (uint32_t)(mat->normal_texture.texture - data->textures),
                &mat->normal_texture);
            LoaderImagePtr img = this->LoadImage(data, tex->imageIndex, false);
            material->_normalImage = img->bitmapData;
            material->_normalScale = mat->normal_texture.scale;
        }
        else
            LOG_WARNING("Has normal texture, but lack tangent data in the mesh\n");
    }

    // occlusion texture
    if (mat->occlusion_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data, (uint32_t)(mat->occlusion_texture.texture - data->textures),
            &mat->occlusion_texture);
        LoaderImagePtr img = this->LoadImage(data, tex->imageIndex, false);
        material->_occlusionImage = img->bitmapData;
    }

    // emissive texture
    if (mat->emissive_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data, (uint32_t)(mat->emissive_texture.texture - data->textures),
            &mat->emissive_texture);
        LoaderImagePtr img = this->LoadImage(data, tex->imageIndex, false);
        material->_emmissiveImage = img->bitmapData;
    }

    // emissive factor
    material->_emissiveFactor = float3{mat->emissive_factor[0],
                                       mat->emissive_factor[1],
                                       mat->emissive_factor[2]};

    // alpha mode
    switch (mat->alpha_mode)
    {
    case cgltf_alpha_mode_mask:
        material->_alphaMode = AlphaMode::Mask;
        break;
    case cgltf_alpha_mode_blend:
        material->_alphaMode = AlphaMode::Blend;
        break;
    default:
        material->_alphaMode = AlphaMode::Opaque;
        break;
    }
    material->_alphaCutoff = mat->alpha_cutoff;
    material->_doubleSided = mat->double_sided;

    // extensions
    if (mat->has_clearcoat)
        this->LoadClearcoat(data, &mat->clearcoat, *material.get());
    if (mat->has_sheen)
        this->LoadSheen(data, &mat->sheen, *material.get());
    if (mat->has_ior)
        material->_IORFactor = mat->ior.ior;

    m_Materials[mat_index] = material;
    return material;
}
```

- [ ] **Step 2: 重写 LoadMetallicRoughness（替换行 810-851）**

```cpp
bool glTF2_Loader::LoadMetallicRoughness(cgltf_pbr_metallic_roughness* pbr,
                                          MaterialResource& pMaterial)
{
    pMaterial._albedoFactor = float4{pbr->base_color_factor[0],
                                     pbr->base_color_factor[1],
                                     pbr->base_color_factor[2],
                                     pbr->base_color_factor[3]};

    pMaterial._metallicFactor = pbr->metallic_factor;
    pMaterial._roughnessFactor = pbr->roughness_factor;

    if (pbr->base_color_texture.texture)
    {
        // 无法从方法内访问 data->textures，通过纹理指针的传递
        // 此处通过 data 中的 textures 数组反向计算索引，需由调用方透传 data
        // 实际：调用此方法的 LoadMaterial 已将 data 作为成员，但这里没有 data 参数
        // 解决：将 data 作为参数传入或用成员变量 m_data
        LOG_WARNING("base color texture loading needs data access - handled in LoadMaterial");
    }

    if (pbr->metallic_roughness_texture.texture)
    {
        LOG_WARNING("metallic roughness texture loading needs data access - handled in LoadMaterial");
    }

    return true;
}
```

等等，这里有个问题。`LoadMetallicRoughness` 需要 `data` 来计算纹理索引，但当前签名没有 `data`。需要调整设计：要么给这些辅助方法加 `data` 参数，要么纹理加载全部移到 `LoadMaterial` 中处理。

**更新 Task 3 (gltf2_loader.h) 中的签名**，将以下方法都加上 `cgltf_data* data` 参数：

```cpp
    bool LoadMetallicRoughness(cgltf_data* data,
                               struct cgltf_pbr_metallic_roughness* pbr,
                               MaterialResource& matRes);
    bool LoadClearcoat(cgltf_data* data,
                       struct cgltf_clearcoat* cc, MaterialResource& matRes);
    bool LoadSheen(cgltf_data* data,
                   struct cgltf_sheen* sheen, MaterialResource& matRes);
```

**更正 Task 8 Step 2:**

```cpp
bool glTF2_Loader::LoadMetallicRoughness(
    cgltf_data* data, cgltf_pbr_metallic_roughness* pbr, MaterialResource& pMaterial)
{
    pMaterial._albedoFactor = float4{pbr->base_color_factor[0],
                                     pbr->base_color_factor[1],
                                     pbr->base_color_factor[2],
                                     pbr->base_color_factor[3]};
    pMaterial._metallicFactor = pbr->metallic_factor;
    pMaterial._roughnessFactor = pbr->roughness_factor;

    if (pbr->base_color_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(pbr->base_color_texture.texture - data->textures),
            &pbr->base_color_texture);
        LoaderImagePtr img = this->LoadImage(data, tex->imageIndex, true);
        pMaterial._albedoImage = img->bitmapData;
    }

    if (pbr->metallic_roughness_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(pbr->metallic_roughness_texture.texture - data->textures),
            &pbr->metallic_roughness_texture);
        LoaderImagePtr img = this->LoadImage(data, tex->imageIndex, false);
        pMaterial._metallicRoughnessImage = img->bitmapData;
    }

    return true;
}
```

- [ ] **Step 3: 重写 LoadClearcoat（替换行 853-888）**

```cpp
bool glTF2_Loader::LoadClearcoat(cgltf_data* data,
                                  cgltf_clearcoat* cc, MaterialResource& pMaterial)
{
    pMaterial._clearcoatFactor = cc->clearcoat_factor;
    pMaterial._clearcoatRoughnessFactor = cc->clearcoat_roughness_factor;

    if (cc->clearcoat_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(cc->clearcoat_texture.texture - data->textures),
            &cc->clearcoat_texture);
        LoaderImagePtr img = this->LoadImage(data, tex->imageIndex, false);
        pMaterial._clearcoatImage = img->bitmapData;
    }

    if (cc->clearcoat_roughness_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(cc->clearcoat_roughness_texture.texture - data->textures),
            &cc->clearcoat_roughness_texture);
        LoaderImagePtr img = this->LoadImage(data, tex->imageIndex, false);
        pMaterial._clearcoatRoughnessImage = img->bitmapData;
    }

    return true;
}
```

- [ ] **Step 4: 重写 LoadSheen（替换行 890-925）**

```cpp
bool glTF2_Loader::LoadSheen(cgltf_data* data,
                              cgltf_sheen* sheen, MaterialResource& pMaterial)
{
    pMaterial._sheenColorFactor = float3{sheen->sheen_color_factor[0],
                                         sheen->sheen_color_factor[1],
                                         sheen->sheen_color_factor[2]};
    pMaterial._sheenRoughnessFactor = sheen->sheen_roughness_factor;

    if (sheen->sheen_color_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(sheen->sheen_color_texture.texture - data->textures),
            &sheen->sheen_color_texture);
        LoaderImagePtr img = this->LoadImage(data, tex->imageIndex, false);
        pMaterial._sheenColorImage = img->bitmapData;
    }

    if (sheen->sheen_roughness_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(sheen->sheen_roughness_texture.texture - data->textures),
            &sheen->sheen_roughness_texture);
        LoaderImagePtr img = this->LoadImage(data, tex->imageIndex, false);
        pMaterial._sheenRoughnessImage = img->bitmapData;
    }

    return true;
}
```

- [ ] **Step 5: 保留 LoadLight（替换行 434-489）**

cgltf 通过 `cgltf_data::lights` 暴露 KHR_lights_punctual 扩展。注意 cgltf 将 lights 挂在 data 级别而非 node extensions。

```cpp
LightComponentPtr glTF2_Loader::LoadLight(cgltf_data* data, uint32_t light_index)
{
    RETURN_IF_FOUND(m_Lights, light_index);
    LightComponentPtr lightComponent = nullptr;

    if (data->lights_count > 0 && light_index < data->lights_count)
    {
        cgltf_light* light = &data->lights[light_index];

        switch (light->type)
        {
        case cgltf_light_type_directional:
            lightComponent = MakeSharedPtr<DirectionalLightComponent>(m_pContext);
            break;
        case cgltf_light_type_point:
            lightComponent = MakeSharedPtr<PointLightComponent>(m_pContext);
            break;
        case cgltf_light_type_spot:
            lightComponent = MakeSharedPtr<SpotLightComponent>(m_pContext);
            break;
        default:
            LOG_ERROR("Light type error\n");
            return lightComponent;
        }

        m_Lights[light_index] = lightComponent;

        if (light->name)
            lightComponent->SetName(light->name);
        lightComponent->SetColor(Color(uint8_t(light->color[0] * 255.0f),
                                       uint8_t(light->color[1] * 255.0f),
                                       uint8_t(light->color[2] * 255.0f)));
        lightComponent->SetIntensity(light->intensity);
    }

    return lightComponent;
}
```

注意：原代码中 LoadLight 在 LoadNode 中被调用（被注释掉了），但声明保留。新版 LoadNode 中可以从 node 的 extensions 或 data 的 lights 中提取 light 信息。cgltf 当前不直接支持 `KHR_lights_punctual` 在 node 级别的 light 引用，如需完整支持需手动解析扩展 JSON。当前保持与原代码相同的行为（注释掉）。

---

### Task 9: 重写 gltf2_loader.cpp — LoadTexture + LoadImage

**Files:**
- Modify: `engine/importer/gltf2_loader.cpp`

- [ ] **Step 1: 重写 LoadTexture（替换行 927-957）**

```cpp
LoaderTexturePtr glTF2_Loader::LoadTexture(cgltf_data* data, uint32_t tex_index,
                                            cgltf_texture_view* tex_view)
{
    RETURN_IF_FOUND(m_Textures, tex_index);
    LoaderTexturePtr tex = nullptr;

    if (tex_index < data->textures_count)
    {
        tex = MakeSharedPtr<LoaderTexture>();
        cgltf_texture* ctex = &data->textures[tex_index];

        if (ctex->image)
            tex->imageIndex = (int32_t)(ctex->image - data->images);
        else
            tex->imageIndex = -1;

        if (ctex->sampler)
            tex->samplerIndex = (int32_t)(ctex->sampler - data->samplers);
        else
            tex->samplerIndex = -1;

        // texCoord 和 scale 来自纹理引用（texture info / texture view）
        if (tex_view)
        {
            tex->texCoord = tex_view->texcoord;
            tex->scale = tex_view->scale;
        }

        m_Textures[tex_index] = tex;
    }
    return tex;
}
```

- [ ] **Step 2: 重写 LoadImage（替换行 959-1077）**

```cpp
LoaderImagePtr glTF2_Loader::LoadImage(cgltf_data* data, uint32_t image_index,
                                        bool bColor)
{
    if (image_index >= data->images_count)
        return nullptr;

    RETURN_IF_FOUND(m_Images, image_index);
    LoaderImagePtr img = MakeSharedPtr<LoaderImage>();

    cgltf_image* cimg = &data->images[image_index];

    ImageType imageType = ImageType::UNKNOWN;
    if (cimg->mime_type)
    {
        img->mimeType = cimg->mime_type;
        if (strcmp(cimg->mime_type, "image/jpeg") == 0)
            imageType = ImageType::JPEG;
        else if (strcmp(cimg->mime_type, "image/png") == 0)
            imageType = ImageType::PNG;
        else
        {
            LOG_ERROR("Unsupported image mime type: %s\n", cimg->mime_type);
            return nullptr;
        }
    }

    if (cimg->buffer_view)
    {
        // 去重：如果已有同 buffer_view + mimeType 的 image，复用 bitmapData
        for (auto& it : m_Images)
        {
            if (it.second->mimeType == img->mimeType)
            {
                cgltf_image* prevImg = nullptr;
                uint32_t prevIndex = it.first;
                if (prevIndex < data->images_count)
                    prevImg = &data->images[prevIndex];
                if (prevImg && prevImg->buffer_view == cimg->buffer_view)
                {
                    img->bitmapData = it.second->bitmapData;
                    LOG_INFO("Reuse a texture2D");
                    break;
                }
            }
        }

        if (!img->bitmapData)
        {
            cgltf_buffer_view* bv = cimg->buffer_view;
            uint8_t* data_ptr = static_cast<uint8_t*>(bv->buffer->data) + bv->offset;
            BitmapBufferPtr bm = ImageDecode(data_ptr, (uint32_t)bv->size, imageType);
            img->bitmapData = bm;
            LOG_INFO("Create a texture2D");
        }
    }
    else if (cimg->uri)
    {
        img->uriPath = cimg->uri;

        // cgltf_load_buffers 已加载 URI 引用的文件
        // 如果是外部图片文件（非 data URI），cgltf 不会自动加载
        // 回退到手动文件加载
        if (cimg->buffer_view)
        {
            cgltf_buffer_view* bv = cimg->buffer_view;
            uint8_t* data_ptr = static_cast<uint8_t*>(bv->buffer->data) + bv->offset;
            BitmapBufferPtr bm = ImageDecode(data_ptr, (uint32_t)bv->size, imageType);
            img->bitmapData = bm;
        }
        else
        {
            std::string uriPath = m_filePath.substr(0, m_filePath.find_last_of('/'))
                                  + "/" + img->uriPath;
            std::shared_ptr<FileResource> uriFileRes = MakeSharedPtr<FileResource>(uriPath);
            if (!uriFileRes->IsAvailable())
            {
                LOG_ERROR("gltf_Loader: load uri file %s fail", uriPath.c_str());
                return nullptr;
            }
            uint8_t* data_ptr = (uint8_t*)(uriFileRes->_data);
            BitmapBufferPtr bm = ImageDecode(data_ptr,
                                             (uint32_t)uriFileRes->_size, imageType);
            img->bitmapData = bm;
        }
    }

    m_Images[image_index] = img;
    return img;
}
```

---

### Task 10: 重写 gltf2_loader.cpp — LoadSkin + LoadAnimation + MorphTarget

**Files:**
- Modify: `engine/importer/gltf2_loader.cpp`

- [ ] **Step 1: 重写 LoadSkin（替换行 1079-1142）**

```cpp
LoaderSkinPtr glTF2_Loader::LoadSkin(cgltf_data* data, uint32_t skin_index)
{
    RETURN_IF_FOUND(m_Skins, skin_index);
    LoaderSkinPtr skin = nullptr;

    if (skin_index >= data->skins_count)
        return skin;

    cgltf_skin* cskin = &data->skins[skin_index];
    skin = MakeSharedPtr<LoaderSkin>();

    // inverse bind matrices
    if (cskin->inverse_bind_matrices)
    {
        cgltf_accessor* accessor = cskin->inverse_bind_matrices;
        if (accessor->type == cgltf_type_mat4)
        {
            const uint8_t* src = AccessorData(accessor);
            for (size_t i = 0; i < accessor->count; i++)
            {
                skin->inverseBindMatrices.push_back(
                    Matrix4(reinterpret_cast<const float*>(src) + i * 16));
            }
        }
    }

    // joints
    for (size_t i = 0; i < cskin->joints_count; i++)
    {
        uint32_t joint_index = (uint32_t)(cskin->joints[i] - data->nodes);
        SceneComponentPtr sc = this->LoadNode(data, joint_index);
        if (sc)
            skin->joints.push_back(sc);
        else
            LOG_ERROR("skin joints index is not correct");
    }

    // skeleton root
    if (cskin->skeleton)
    {
        uint32_t skel_index = (uint32_t)(cskin->skeleton - data->nodes);
        SceneComponentPtr skeleton_sc = this->LoadNode(data, skel_index);
        if (skeleton_sc)
            skin->skeleton = skeleton_sc;
        else
            LOG_ERROR("skin skeleton index is not correct");
    }

    m_Skins[skin_index] = skin;
    return skin;
}
```

- [ ] **Step 2: 重写 LoadAnimation（替换行 1144-1295）**

```cpp
void glTF2_Loader::LoadAnimation(cgltf_data* data,
                                  std::vector<AnimationComponentPtr>& animations)
{
    float start_time = 0.0;
    float end_time = 0.0;

    for (size_t anim_idx = 0; anim_idx < data->animations_count; anim_idx++)
    {
        cgltf_animation* canim = &data->animations[anim_idx];
        AnimationComponentPtr anim = MakeSharedPtr<AnimationComponent>(m_pContext);

        if (canim->name)
            anim->SetName(canim->name);

        size_t channel_count = canim->channels_count;
        for (size_t ch = 0; ch < channel_count; ch++)
        {
            cgltf_animation_channel* channel = &canim->channels[ch];
            cgltf_animation_sampler* sampler = channel->sampler;

            if (!channel->target_node || !sampler)
                continue;

            uint32_t target_node_index =
                (uint32_t)(channel->target_node - data->nodes);
            SceneComponentPtr sc = LoadNode(data, target_node_index);
            if (!sc)
            {
                LOG_ERROR("LoadAnimation Error: node not found");
                continue;
            }

            TransformType type = TransformType::None;
            switch (channel->target_path)
            {
            case cgltf_animation_path_type_translation: type = Translate; break;
            case cgltf_animation_path_type_rotation:    type = Rotate;    break;
            case cgltf_animation_path_type_scale:       type = Scale;     break;
            default: break;
            }

            AnimationTrackPtr animTrack = nullptr;
            TransformAnimationTrackPtr tAnimTrack = nullptr;
            MorphTargetAnimationTrackPtr mtAnimTrack = nullptr;

            if (type != TransformType::None)
            {
                tAnimTrack = anim->CreateTransformTrack();
                tAnimTrack->SetSceneComponent(sc);
                animTrack = tAnimTrack;
            }
            else if (channel->target_path == cgltf_animation_path_type_weights)
            {
                mtAnimTrack = anim->CreateMorphTargetTrack();
                mtAnimTrack->SetSceneComponent(sc);
                animTrack = mtAnimTrack;
            }
            else
            {
                LOG_ERROR("LoadAnimation Error: unknown path type");
                continue;
            }

            // interpolation type
            animTrack->SetInterpolationType(
                gltf::ConvertToInterpolationType(sampler->interpolation));

            // 读取输入/输出数据
            cgltf_accessor* input_acc = sampler->input;
            cgltf_accessor* output_acc = sampler->output;
            if (!input_acc || !output_acc)
            {
                LOG_ERROR("Animation sampler accessor error");
                continue;
            }

            const float* inputData =
                reinterpret_cast<const float*>(AccessorData(input_acc));
            uint32_t inputCount = (uint32_t)input_acc->count;

            const uint8_t* outputData = AccessorData(output_acc);
            uint32_t outputCount = (uint32_t)output_acc->count;
            uint32_t outputStride = AccessorStride(output_acc);

            if (inputCount != outputCount || !inputData)
                continue;

            uint32_t count = inputCount;
            for (uint32_t k = 0; k < count; k++)
            {
                if (tAnimTrack)
                {
                    TransformKeyFramePtr tKFPtr = tAnimTrack->CreateTransformKeyFrame();
                    float time = inputData[k];
                    tKFPtr->SetTime(time);
                    start_time = (k == 0) ? time : start_time;
                    end_time = time;

                    if (type == Translate && outputStride == 12)
                    {
                        const float* src = (const float*)outputData + 3 * k;
                        float3 translation = float3{src[0], src[1], src[2]};
                        tKFPtr->SetTranslate(translation);
                        tKFPtr->SetOffsetTranslate(
                            translation - sc->GetLocalTransform().GetTranslation());
                    }
                    else if (type == Rotate && outputStride == 16)
                    {
                        const float* src = (const float*)outputData + 4 * k;
                        Quaternion qua(src[0], src[1], src[2], src[3]);
                        tKFPtr->SetRotate(qua);
                        tKFPtr->SetOffsetRotate(
                            (sc->GetLocalTransform().GetRotation().Inverse()) * qua);
                    }
                    else if (type == Scale && outputStride == 12)
                    {
                        const float* src = (const float*)outputData + 3 * k;
                        float3 scale = float3{src[0], src[1], src[2]};
                        tKFPtr->SetScale(scale);
                        tKFPtr->SetOffsetScale(
                            scale / sc->GetLocalTransform().GetScale());
                    }
                    else
                        LOG_ERROR("Animation data format not supported");
                }
                else if (mtAnimTrack)
                {
                    MorphTargetKeyFramePtr mtKFPtr =
                        mtAnimTrack->CreateMorphTargetKeyFrame();
                    float time = inputData[k];
                    mtKFPtr->SetTime(time);
                    start_time = (k == 0) ? time : start_time;
                    end_time = time;

                    uint32_t weight_count = outputCount / count;
                    const float* weight_ptr = (const float*)outputData + k * weight_count;
                    std::vector<float> weights(weight_count);
                    for (uint32_t w = 0; w < weight_count; w++)
                        weights[w] = weight_ptr[w];
                    mtKFPtr->SetWeights(weights);
                }
            }
        }

        if (anim)
        {
            AnimInfo animInfo;
            animInfo.startTime = start_time;
            animInfo.preFrameTime = start_time;
            animInfo.endTime = end_time;
            animInfo.state = AnimationState::Stopped;
            animInfo.loop = true;
            anim->AddAnimSectionInfo(animInfo);
        }

        animations.push_back(anim);
    }
}
```

- [ ] **Step 3: 重写 ConverterToMorphStreamUnitFromTargets（替换行 1577-1633）**

```cpp
void glTF2_Loader::ConverterToMorphStreamUnitFromTargets(
    RHIMeshPtr& mesh, cgltf_primitive* primitive, AABBox& targets_box)
{
    size_t targetSize = primitive->targets_count;
    if (targetSize == 0)
        return;

    // 获取第一个 target 的 POSITION accessor 以获取 count
    cgltf_morph_target* firstTarget = &primitive->targets[0];
    cgltf_accessor* firstAcc = nullptr;
    for (size_t a = 0; a < firstTarget->attributes_count; a++)
    {
        if (firstTarget->attributes[a].type == cgltf_attribute_type_position)
        {
            firstAcc = firstTarget->attributes[a].data;
            break;
        }
    }
    if (!firstAcc)
        return;

    int32_t targetCount = (int32_t)firstAcc->count;
    int32_t allTargetDataSize = (int32_t)targetSize * targetCount * 4; // float4

    MorphTargetResource morphTargetRes;
    std::shared_ptr<float> allTargetData{
        new float[allTargetDataSize]{0.0f}, default_array_deleter<float>()
    };
    morphTargetRes._uninitializer = [allTargetData](IResource*) mutable {
        allTargetData.reset();
    };

    float* allTargetDataPtr = allTargetData.get();

    for (size_t i = 0; i < targetSize; i++)
    {
        cgltf_morph_target* target = &primitive->targets[i];
        for (size_t a = 0; a < target->attributes_count; a++)
        {
            if (target->attributes[a].type == cgltf_attribute_type_position)
            {
                cgltf_accessor* accessor = target->attributes[a].data;
                if (!accessor || accessor->count != (size_t)targetCount)
                    return;

                if (accessor->has_min && accessor->has_max)
                {
                    AABBox tmp_box(float3{accessor->min[0], accessor->min[1],
                                          accessor->min[2]},
                                   float3{accessor->max[0], accessor->max[1],
                                          accessor->max[2]});
                    targets_box |= tmp_box;
                }

                const float* targetData =
                    reinterpret_cast<const float*>(AccessorData(accessor));
                for (int32_t j = 0; j < targetCount; j++)
                {
                    int32_t addr = (j * (int32_t)targetSize + (int32_t)i) * 4;
                    allTargetDataPtr[addr] = targetData[0];
                    allTargetDataPtr[addr + 1] = targetData[1];
                    allTargetDataPtr[addr + 2] = targetData[2];
                    targetData += 3;
                }
            }
        }
    }

    morphTargetRes._morphInfo.morph_target_names = std::vector<std::string>();
    morphTargetRes._morphInfo.morph_target_type = MorphTargetType::Position;
    morphTargetRes._morphInfo.morph_target_weights.resize(targetSize, 0.0f);
    morphTargetRes._data = (uint8_t*)allTargetData.get();
    morphTargetRes._size = allTargetDataSize * sizeof(float);
    mesh->SetMorphTargetResource(morphTargetRes);
}
```

- [ ] **Step 4: 保留访问器方法（替换行 1649-1669）**

```cpp
std::map<uint32_t, MeshComponentPtr>& glTF2_Loader::GetMeshComponentMap()
{
    return m_Meshes;
}

std::map<uint32_t, SceneComponentPtr>& glTF2_Loader::GetSceneComponentMap()
{
    return m_Nodes;
}

std::map<std::string, SceneComponentPtr>& glTF2_Loader::GetSceneComponentMapByName()
{
    return m_NodesMapByName;
}
```

- [ ] **Step 5: 确认删除不再需要的方法**

以下方法应已从文件中删除（对应之前替换的行）：
- `LoadBuffer` (行 1499-1575)
- `LoadBufferView` (行 1377-1409)
- `LoadAccessor` (行 1346-1375)
- `LoadAnimationSampler` (行 1297-1334)
- `LoadUriFile` (行 1635-1647)
- `IsURIData` (行 1411-1470)
- `DecodeDataFromURI` (行 1472-1497)
- `LoadExtensionsUsed` (行 151-165)
- `GetModelDoc` (行 1664-1667)

- [ ] **Step 6: 删除不再需要的宏定义和静态函数**

确认以下内容已从文件中删除：
- `parse_json_array` 模板函数 (行 1336-1344)
- `JsonValue2Float4` / `JsonValue2Float3` 宏 (行 217-218) — 如果 LoadNode 中已移除 JSON 版本的代码

---

### Task 11: 更新 gltf2.h — 枚举映射函数签名

**Files:**
- Modify: `engine/importer/gltf2.h`

- [ ] **Step 1: 包含 cgltf.h 并更新函数签名**

在文件开头添加 `#include "cgltf.h"`，删除与 cgltf 重复的 `#define` 宏（`GLTF_MODE_*`、`GLTF_COMPONENT_TYPE_*`、`GLTF_ELEMENT_TYPE_*`、`GLB_*`）。

将函数签名改为接受 cgltf 枚举：

```cpp
// gltf2.h — 更新后
#pragma once
#include <map>
#include <string>
#include "cgltf.h"
#include "components/animation_component.h"
#include "components/light_component.h"

SEEK_NAMESPACE_BEGIN

namespace gltf
{

// 保留的常量（引擎内部使用）
#define GLTF_INVALID_INTEGER        (-1)
#define GLTF_INVALID_ENUM           (-1)
#define GLTF_INVALID_ARRAY_INDEX    (-1)

// --- 类型转换函数 ---

inline MeshTopologyType ConvertToTopologyType(cgltf_primitive_type mode)
{
    switch (mode)
    {
    case cgltf_primitive_type_points:         return MeshTopologyType::Points;
    case cgltf_primitive_type_lines:          return MeshTopologyType::Lines;
    case cgltf_primitive_type_line_strip:     return MeshTopologyType::Line_Strip;
    case cgltf_primitive_type_triangles:      return MeshTopologyType::Triangles;
    case cgltf_primitive_type_triangle_strip: return MeshTopologyType::Triangle_Strip;
    case cgltf_primitive_type_triangle_fan:   return MeshTopologyType::Triangle_Strip; // 近似
    default:                                  return MeshTopologyType::Unknown;
    }
}

inline IndexBufferType ConvertToIndexBufferType(int componentType)
{
    switch (componentType)
    {
    case cgltf_component_type_r_16u:  return IndexBufferType::UInt16;
    case cgltf_component_type_r_32u:  return IndexBufferType::UInt32;
    default:                          return IndexBufferType::Unknown;
    }
}

// 保留：从 cgltf_type 获取元素分量数
inline int ConvertToElementType(cgltf_type type)
{
    switch (type)
    {
    case cgltf_type_scalar: return 1;
    case cgltf_type_vec2:   return 2;
    case cgltf_type_vec3:   return 3;
    case cgltf_type_vec4:   return 4;
    case cgltf_type_mat2:   return 4;
    case cgltf_type_mat3:   return 9;
    case cgltf_type_mat4:   return 16;
    default:                return 0;
    }
}

// 注意：以下 ComponentNum 和 ComponentByteSize 函数需要改为接收 cgltf 枚举
// cgltf_component_type 枚举值与 glTF 规范值相同（5120-5126），与原有计算兼容

inline int ComponentByteSize(int componentType)
{
    switch (componentType)
    {
    case cgltf_component_type_r_8:    return 1;
    case cgltf_component_type_r_8u:   return 1;
    case cgltf_component_type_r_16:   return 2;
    case cgltf_component_type_r_16u:  return 2;
    case cgltf_component_type_r_32u:  return 4;
    case cgltf_component_type_r_32f:  return 4;
    default:                          return 0;
    }
}

inline int ComponentNumOfElementType(int elementType)
{
    static const int component_num[] = {0, 1, 2, 3, 4, 4, 9, 16};
    if (elementType < 1 || elementType > 7)
        return 0;
    return component_num[elementType];
}

inline int ElementSize(int componentType, int elementType)
{
    return ComponentNumOfElementType(elementType) * ComponentByteSize(componentType);
}

// vertex attribute 转换
inline std::pair<VertexElementUsage, uint32_t> ConvertVertexAttribute(
    uint32_t attr_index, cgltf_attribute_type attrType)
{
    // 返回 Index 为 attr_index（TEXCOORD_0=0, TEXCOORD_1=1 等）
    switch (attrType)
    {
    case cgltf_attribute_type_position: return {VertexElementUsage::Position,   attr_index};
    case cgltf_attribute_type_normal:   return {VertexElementUsage::Normal,     attr_index};
    case cgltf_attribute_type_tangent:  return {VertexElementUsage::Tangent,    attr_index};
    case cgltf_attribute_type_texcoord: return {VertexElementUsage::TexCoord,   attr_index};
    case cgltf_attribute_type_color:    return {VertexElementUsage::Color,      attr_index};
    case cgltf_attribute_type_joints:   return {VertexElementUsage::BlendIndex, attr_index};
    case cgltf_attribute_type_weights:  return {VertexElementUsage::BlendWeight,attr_index};
    default:                            return {VertexElementUsage::Position,   0};
    }
}

// vertex format 映射表
static const VertexFormat _vertex_format_map[8][7] = {
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown,
      VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown,
      VertexFormat::Unknown },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown,
      VertexFormat::Unknown, VertexFormat::Int, VertexFormat::UInt,
      VertexFormat::Float },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Short2,
      VertexFormat::UShort2, VertexFormat::Int2, VertexFormat::UInt2,
      VertexFormat::Float2 },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Short3,
      VertexFormat::UShort3, VertexFormat::Int3, VertexFormat::UInt3,
      VertexFormat::Float3 },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Short4,
      VertexFormat::UShort4, VertexFormat::Int4, VertexFormat::UInt4,
      VertexFormat::Float4 },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown,
      VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown,
      VertexFormat::Unknown },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown,
      VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown,
      VertexFormat::Unknown },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown,
      VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown,
      VertexFormat::Unknown },
};

inline VertexFormat ConvertToVertexFormat(int elementType, int componentType)
{
    return _vertex_format_map[elementType][componentType - cgltf_component_type_r_8];
}

// interpolation type
inline InterpolationType ConvertToInterpolationType(
    cgltf_interpolation_type interp)
{
    switch (interp)
    {
    case cgltf_interpolation_type_linear:      return InterpolationType::Linear;
    case cgltf_interpolation_type_step:        return InterpolationType::Step;
    case cgltf_interpolation_type_cubic_spline:return InterpolationType::CubicSpline;
    default:                                   return InterpolationType::Linear;
    }
}

} // namespace gltf

SEEK_NAMESPACE_END
```

注意：
- `ConvertVertexAttribute` 函数从接收 string 改为接收 `cgltf_attribute_type` + index
- 调用处（LoadPrimitive）需同时传递 `attr->index` 和 `attr->type`
- 保留宏 `GLTF_INVALID_INTEGER/ENUM/ARRAY_INDEX`（引擎内部仍在使用）

---

### Task 12: 清理 CMakeLists.txt 中的 rapidjson 依赖

**Files:**
- Modify: `engine/CMakeLists.txt`

- [ ] **Step 1: 从 engine 的 target_link_libraries 中移除 rapidjson**

修改 `engine/CMakeLists.txt:249-256`：

```cmake
target_link_libraries(${SEEK_STATICLIB_NAME} PUBLIC
    turbojpeg
    libpng
    eigen
    zlib
    ${SEEK_STATICLIB_DEPENDENT_LIBS}
)
```

移除 `rapidjson` 行。rapidjson 库声明保留在 `third_party/CMakeLists.txt` 中（tools 和 samples 仍在使用）。

---

### Task 13: 构建验证

- [ ] **Step 1: 检查 seek_engine.h 是否需要调整**

`engine/seek_engine.h:25` 包含 `#include "importer/gltf2_loader.h"`，后者不再 include rapidjson，但接口不变。`seek_engine.h` 无需修改。

- [ ] **Step 2: CMake 配置 & 构建**

```powershell
cmake --build build --config Debug --target seek_engine-static
```

- [ ] **Step 3: 修复编译错误**

根据编译错误修复代码。常见可能问题：
1. `cgltf.h` 版本间字段名差异（如 `cgltf_clearcoat` vs `cgltf_material_clearcoat`）——按实际 cgltf 版本调整字段名
2. `primitive->targets_count` vs `primitive->targets` —— cgltf 中 morph targets 通过 `cgltf_primitive::targets` 和 `targets_count` 访问
3. 跨模块的 rapidjson 引用 —— 如果 samples 通过 `seek_engine.h` 间接触发了 rapidjson，需检查

---

### Task 14: 功能验证（运行时）

- [ ] **Step 1: 构建 samples**

```powershell
cmake --build build --config Debug --target 05.DeferredShading
```

- [ ] **Step 2: 运行 Sponza 场景**

```powershell
.\build\samples\05.DeferredShading\Debug\05.DeferredShading.exe
```

验证 Sponza.gltf 正常加载和渲染。观察 LOG 输出确认加载路径正常。

- [ ] **Step 3: 运行 cube/room 等其他测试场景**

运行其他已知使用 glTF 的 sample 确认兼容性。
