# glTF 加载器两阶段架构设计

## 目标

将 glTF 加载器重构为两阶段架构：Phase 1 解析 glTF 文件构建自包含的中间表示（IR），Phase 2 从 IR 组装引擎 Component。实现 cgltf 依赖解耦、代码结构优化，并为异步加载等场景提供扩展基础。

## 动机

- 当前 loader 直接从 cgltf_data 构建引擎 Component，解析和组件构建交织在一起
- cgltf_data 必须存活至整个加载过程结束，解耦不彻底
- Loader 类承担过多职责（~800 行），方法间通过成员 map 隐式传递状态
- 无法在构建 Component 前检查和修改中间数据

## 架构概览

```
glTF 文件
    │
    ▼
┌──────────────────────┐
│  GltfDataBuilder     │  Phase 1: cgltf_parse → cgltf_load_buffers → Extract* → cgltf_free
│  (cgltf dependent)   │
└──────────┬───────────┘
           │ GltfData (IR, 自包含)
           ▼
┌──────────────────────┐
│  GltfSceneAssembler  │  Phase 2: IR → SceneComponent / MeshComponent / MaterialResource / ...
│  (cgltf independent) │
└──────────┬───────────┘
           │
           ▼
      引擎 Component 树
```

## 改动范围

| 文件 | 操作 |
|------|------|
| `engine/importer/gltf_data.h` | 新增 — IR 数据结构 |
| `engine/importer/gltf_data_builder.h` | 新增 — Phase 1 构建器声明 |
| `engine/importer/gltf_data_builder.cpp` | 新增 — Phase 1 实现 |
| `engine/importer/gltf_scene_assembler.h` | 新增 — Phase 2 组装器声明 |
| `engine/importer/gltf_scene_assembler.cpp` | 新增 — Phase 2 实现 |
| `engine/importer/gltf2_loader.h` | 重写 — 退化为薄壳 |
| `engine/importer/gltf2_loader.cpp` | 重写 — 去掉全部转换逻辑 |
| `engine/importer/loader.h` | 不变 |
| `engine/importer/gltf2.h` | 保留 — 枚举转换辅助函数 |
| `engine/seek_engine.h` | 不变 |

## 一、IR 数据结构 (gltf_data.h)

全部使用索引（非指针）引用其他对象，自包含、不依赖 cgltf。原始 buffer 数据通过 shared_ptr 管理生命周期。

```cpp
struct GltfBuffer {
    std::shared_ptr<BufferResource> resource;
    uint8_t* data = nullptr;
    size_t   size = 0;
};

struct GltfImage {
    std::string    mimeType;
    std::string    uriPath;
    BitmapBufferPtr bitmapData;
};

struct GltfSampler {
    int32_t magFilter = -1;
    int32_t minFilter = -1;
    int32_t wrapS = -1;
    int32_t wrapT = -1;
};

struct GltfTexture {
    uint32_t imageIndex   = UINT32_MAX;
    uint32_t samplerIndex = UINT32_MAX;
    int32_t  texCoord     = 0;
    float    scale        = 1.0f;
};

struct GltfMaterial {
    std::string name;
    float4 albedoFactor       = float4(1.0f);
    float  metallicFactor     = 1.0f;
    float  roughnessFactor    = 1.0f;
    float3 emissiveFactor     = float3(0.0f);
    AlphaMode alphaMode       = AlphaMode::Opaque;
    float  alphaCutoff        = 0.5f;
    bool   doubleSided        = false;
    float  ior                = 1.5f;

    uint32_t albedoTextureIndex            = UINT32_MAX;
    uint32_t metallicRoughnessTextureIndex = UINT32_MAX;
    uint32_t normalTextureIndex            = UINT32_MAX;
    uint32_t occlusionTextureIndex         = UINT32_MAX;
    uint32_t emissiveTextureIndex          = UINT32_MAX;
    float    normalScale                   = 1.0f;

    // KHR_materials_clearcoat
    bool     hasClearcoat = false;
    float    clearcoatFactor = 0.0f;
    float    clearcoatRoughness = 0.0f;
    uint32_t clearcoatTextureIndex          = UINT32_MAX;
    uint32_t clearcoatRoughnessTextureIndex = UINT32_MAX;

    // KHR_materials_sheen
    bool     hasSheen = false;
    float3   sheenColorFactor     = float3(0.0f);
    float    sheenRoughnessFactor = 0.0f;
    uint32_t sheenColorTextureIndex     = UINT32_MAX;
    uint32_t sheenRoughnessTextureIndex = UINT32_MAX;
};

struct GltfPrimitive {
    MeshTopologyType topology = MeshTopologyType::Triangles;
    uint32_t         materialIndex = UINT32_MAX;

    std::vector<VertexStream>                     vertexStreams;
    std::vector<std::shared_ptr<BufferResource>>  vertexBuffers;
    std::shared_ptr<VertexIndicesResource>        indexResource;
    std::shared_ptr<MorphTargetResource>          morphTargets;

    SkinningJointBindSize jointBindSize = SkinningJointBindSize::None;
    AABBox boundingBox;
    bool hasNormal   = false;
    bool hasTexcoord = false;
    bool hasTangent  = false;
};

struct GltfMesh {
    std::string                   name;
    std::vector<GltfPrimitive>    primitives;
    std::vector<float>            weights;
    std::vector<std::string>      targetNames;
};

struct GltfSkin {
    std::vector<Matrix4>    inverseBindMatrices;
    std::vector<uint32_t>   jointNodeIndices;
    uint32_t                skeletonNodeIndex = UINT32_MAX;
};

struct GltfNode {
    std::string name;
    bool    hasMatrix      = false;
    Matrix4 matrix          = Matrix4::Identity();
    bool    hasTranslation = false;
    float3  translation    = float3(0.0f);
    bool    hasRotation    = false;
    Quaternion rotation    = Quaternion::Identity();
    bool    hasScale       = false;
    float3  scale          = float3(1.0f);

    uint32_t meshIndex  = UINT32_MAX;
    uint32_t skinIndex  = UINT32_MAX;
    uint32_t lightIndex = UINT32_MAX;

    std::vector<uint32_t> childIndices;
};

struct GltfAnimationChannel {
    uint32_t            targetNodeIndex;
    TransformType       transformType;
    InterpolationType   interpolation;
    std::vector<float>  inputTimes;
    std::vector<float>  outputData;
    uint32_t            outputStride;
};

struct GltfAnimation {
    std::string name;
    std::vector<GltfAnimationChannel> channels;
};

struct GltfLight {
    std::string name;
    LightType   type;
    Color       color;
    float       intensity;
};

struct GltfScene {
    std::string name;
    std::vector<uint32_t> rootNodeIndices;
};

struct GltfData {
    std::vector<GltfImage>    images;
    std::vector<GltfSampler>  samplers;
    std::vector<GltfTexture>  textures;
    std::vector<GltfMaterial> materials;
    std::vector<GltfMesh>     meshes;
    std::vector<GltfSkin>     skins;
    std::vector<GltfNode>     nodes;
    std::vector<GltfAnimation>animations;
    std::vector<GltfLight>    lights;
    std::vector<GltfScene>    scenes;

    uint32_t              defaultSceneIndex = UINT32_MAX;
    std::vector<GltfBuffer> buffers;
};
```

## 二、Phase 1 — GltfDataBuilder

### 职责

解析 glTF 文件，填充 GltfData。完成后立即 `cgltf_free`，GltfData 此后完全自治。

### 调用流程

```
Build(filePath, outData)
  ├── cgltf_parse_file → data
  ├── cgltf_load_buffers → data
  ├── cgltf_validate → data
  ├── ExtractBuffers(data, out)        // 接管 buffer 生命周期
  ├── ExtractImages(data, out)         // 解码所有图片
  ├── ExtractSamplers(data, out)       // 枚举值拷贝
  ├── ExtractTextures(data, out)       // 索引映射
  ├── ExtractMaterials(data, out)      // 材质参数 + 纹理索引
  ├── ExtractMeshes(data, out)         // primitives 顶点/索引/morph
  ├── ExtractSkins(data, out)          // 蒙皮数据（用 node 索引）
  ├── ExtractNodes(data, out)          // 节点层级 + 变换
  ├── ExtractAnimations(data, out)     // 关键帧数据展开
  ├── ExtractLights(data, out)         // 灯光参数
  ├── ExtractScenes(data, out)         // 根节点列表
  └── cgltf_free(data)
```

### 关键实现细节

- **ExtractBuffers**：每个 cgltf_buffer 创建一个 GltfBuffer，shared_ptr 接管 data 指针。cgltf_free 后 buffer 数据仍存活
- **ExtractImages**：区分 buffer 嵌入和 URI 引用，统一用 ImageDecode 解码。URI 图片需拼接 m_filePath 目录路径
- **ExtractPrimitive**：遍历 primitive->attributes 组装 VertexStream。Joint index uint8/uint16→uint32 转换在此完成。所有数据指针指向 GltfData::buffers 中的 buffer
- **ExtractAnimations**：cgltf_animation_sampler 的 input/output accessor 数据展开为 vector<float>

## 三、Phase 2 — GltfSceneAssembler

### 职责

从 GltfData（只读）构建引擎 Component，不依赖 cgltf。

### 构建顺序

```
Assemble(data, outScene, outAnimations)
  ├── BuildMaterials(data)       // 1. MaterialResource（无依赖）
  ├── BuildMeshes(data)          // 2. MeshComponent + RHIMesh（依赖 materials）
  ├── BuildSkins(data)           // 3. 蒙皮数据关联
  ├── BuildNodes(data)           // 4. SceneComponent 树（依赖 meshes/skins）
  ├── BuildAnimations(data)      // 5. AnimationComponent（依赖 nodes）
  └── BuildScene(data)           // 6. 组装根节点
```

### 构建缓存

三个索引 map 供后续步骤查找引用：

- `m_materials: map<uint32_t, shared_ptr<MaterialResource>>`
- `m_meshes: map<uint32_t, MeshComponentPtr>`
- `m_nodes: map<uint32_t, SceneComponentPtr>`

### 与原方法对照

| 原方法 | Phase 1 | Phase 2 |
|--------|---------|---------|
| LoadImage / LoadTexture | ExtractImages / ExtractTextures | — |
| LoadMaterial | ExtractMaterials | BuildMaterial |
| LoadPrimitive | ExtractPrimitive（顶点/索引/morph） | BuildPrimitive（组装 RHIMesh） |
| LoadMesh | ExtractMeshes（primitives 列表） | BuildMesh（MeshComponent 组装） |
| LoadSkin | ExtractSkins（关节索引 + IBM） | BuildSkins（关联 joints 节点） |
| LoadNode | ExtractNodes（层级关系记录） | BuildNode（SceneComponent 树） |
| LoadAnimation | ExtractAnimations（keyframe 展开） | BuildAnimations（AnimationComponent） |

## 四、glTF2_Loader 改造

Loader 退化为薄壳，组合 Builder + Assembler：

```cpp
class glTF2_Loader : public Loader {
    SResult LoadSceneFromFile(...) override {
        m_builder.Build(filePath, m_data);      // Phase 1
        m_assembler.Assemble(m_data, ...);       // Phase 2
    }
private:
    GltfDataBuilder    m_builder;
    GltfSceneAssembler m_assembler;
    GltfData           m_data;
};
```

Loader 接口不变，外部调用方零改动。

## 风险与注意事项

1. **内存开销**：IR 将 accessor 数据展开为 vector<float>（动画），对大型动画文件有额外内存开销。可后续优化为惰性加载
2. **Image 解码时机**：Phase 1 中完成 ImageDecode，意味着所有纹理在组件构建前已解码完成，对启动时间有影响。当前行为与原来一致（原先也是在 LoadImage 中立即解码）
3. **GltfData 与 cgltf 数据一致性**：确保 ExtractBuffers 在 ExtractMeshes/ExtractImages 之前调用，所有 buffer 数据指针在 cgltf_free 前已被 shared_ptr 接管
4. **gltf2.h 清理**：Phase 1 Builder 内部仍会使用 gltf2.h 中的辅助函数（ConvertToVertexFormat 等），不做删除
5. **Loader 访问器转发**：`GetMeshComponentMap()`、`GetSceneComponentMap()`、`GetSceneComponentMapByName()` 由 `glTF2_Loader` 暴露，内部转发到同名 map（Assembler 构建完成后将缓存赋值给 Loader 成员，或 Loader 持有 Assembler 引用直接读取）
