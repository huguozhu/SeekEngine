# glTF 加载器两阶段架构 — 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 glTF 加载器重构为两阶段：Phase 1 解析 glTF 构建自包含 IR (GltfData)，Phase 2 从 IR 组装引擎 Component

**Architecture:** GltfDataBuilder（依赖 cgltf，完成即 free）→ GltfData（纯数据 IR）→ GltfSceneAssembler（不依赖 cgltf）→ 引擎 Component。glTF2_Loader 退化为薄壳组合两者。

**Tech Stack:** C++20, CMake, cgltf, D3D11

---

### Task 1: 创建 gltf_data.h — IR 数据结构

**Files:**
- Create: `engine/importer/gltf_data.h`

- [ ] **Step 1: 创建 gltf_data.h**

```cpp
#pragma once

#include "kernel/kernel.h"
#include "components/mesh_component.h"
#include "components/light_component.h"
#include "components/animation_component.h"
#include "resource/resource_mgr.h"
#include "math/matrix.h"
#include "math/aabbox.h"
#include "rhi/base/rhi_mesh.h"

SEEK_NAMESPACE_BEGIN

struct BufferResource;

// 原始二进制数据，shared_ptr 管理生命周期
struct GltfBuffer
{
    std::shared_ptr<BufferResource> resource;
    uint8_t* data = nullptr;
    size_t   size = 0;
};

// 图片（Phase 1 已完成解码）
struct GltfImage
{
    std::string    mimeType;
    std::string    uriPath;
    BitmapBufferPtr bitmapData;
};

// 采样器
struct GltfSampler
{
    int32_t magFilter = -1;
    int32_t minFilter = -1;
    int32_t wrapS = -1;
    int32_t wrapT = -1;
};

// 纹理引用
struct GltfTexture
{
    uint32_t imageIndex   = UINT32_MAX;
    uint32_t samplerIndex = UINT32_MAX;
    int32_t  texCoord     = 0;
    float    scale        = 1.0f;
};

// 材质
struct GltfMaterial
{
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
    float    clearcoatRoughnessFactor = 0.0f;
    uint32_t clearcoatTextureIndex          = UINT32_MAX;
    uint32_t clearcoatRoughnessTextureIndex = UINT32_MAX;

    // KHR_materials_sheen
    bool     hasSheen = false;
    float3   sheenColorFactor     = float3(0.0f);
    float    sheenRoughnessFactor = 0.0f;
    uint32_t sheenColorTextureIndex     = UINT32_MAX;
    uint32_t sheenRoughnessTextureIndex = UINT32_MAX;
};

// 图元（顶点/索引/morph 数据已提取完毕）
struct GltfPrimitive
{
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

// 网格
struct GltfMesh
{
    std::string                   name;
    std::vector<GltfPrimitive>    primitives;
    std::vector<float>            weights;       // morph weights
    std::vector<std::string>      targetNames;   // morph target names
};

// 蒙皮
struct GltfSkin
{
    std::vector<Matrix4>    inverseBindMatrices;
    std::vector<uint32_t>   jointNodeIndices;
    uint32_t                skeletonNodeIndex = UINT32_MAX;
};

// 场景节点
struct GltfNode
{
    std::string name;

    // Transform
    bool     hasMatrix      = false;
    Matrix4  matrix         = Matrix4::Identity();
    bool     hasTranslation = false;
    float3   translation    = float3(0.0f);
    bool     hasRotation    = false;
    Quaternion rotation     = Quaternion::Identity();
    bool     hasScale       = false;
    float3   scale          = float3(1.0f);

    // 数据引用（索引）
    uint32_t meshIndex  = UINT32_MAX;
    uint32_t skinIndex  = UINT32_MAX;
    uint32_t lightIndex = UINT32_MAX;

    // 层级关系
    std::vector<uint32_t> childIndices;
};

// 动画 channel（关键帧数据已展开）
struct GltfAnimationChannel
{
    uint32_t            targetNodeIndex;
    TransformType       transformType;    // Translate/Rotate/Scale
    bool                isMorphTarget = false; // morph target weights channel
    InterpolationType   interpolation;
    std::vector<float>  inputTimes;       // 关键帧时间戳
    std::vector<float>  outputData;       // 关键帧输出数据（分量展开）
    uint32_t            outputStride;     // 每帧输出数据分量数
    uint32_t            outputCount;      // 输出数据总帧数（即 inputTimes.size()）
};

// 动画
struct GltfAnimation
{
    std::string                        name;
    std::vector<GltfAnimationChannel>  channels;
};

// 灯光（KHR_lights_punctual）
struct GltfLight
{
    std::string name;
    LightType   type;
    Color       color;
    float       intensity;
};

// 场景
struct GltfScene
{
    std::string                     name;
    std::vector<uint32_t>           rootNodeIndices;
};

// 顶层 IR 容器
struct GltfData
{
    std::vector<GltfImage>      images;
    std::vector<GltfSampler>    samplers;
    std::vector<GltfTexture>    textures;
    std::vector<GltfMaterial>   materials;
    std::vector<GltfMesh>       meshes;
    std::vector<GltfSkin>       skins;
    std::vector<GltfNode>       nodes;
    std::vector<GltfAnimation>  animations;
    std::vector<GltfLight>      lights;
    std::vector<GltfScene>      scenes;

    uint32_t                    defaultSceneIndex = UINT32_MAX;
    std::vector<GltfBuffer>     buffers;            // 保证原始 buffer 数据生命周期
};

SEEK_NAMESPACE_END
```

- [ ] **Step 2: 验证编译通过**

```powershell
cmake --build build --config Debug --target seek_engine-static 2>&1 | Select-Object -First 20
```

预期：头文件无编译错误，但 linker 可能找不到尚未创建的 Builder/Assembler 符号（正常，后续任务补充）。

---

### Task 2: 创建 gltf_data_builder.h + .cpp — Phase 1 实现

**Files:**
- Create: `engine/importer/gltf_data_builder.h`
- Create: `engine/importer/gltf_data_builder.cpp`

- [ ] **Step 1: 创建 gltf_data_builder.h**

```cpp
#pragma once

#include "importer/gltf_data.h"

// 前置声明（避免头文件泄漏 cgltf）
struct cgltf_data;
struct cgltf_primitive;
struct cgltf_image;
struct cgltf_animation_channel;
struct cgltf_animation_sampler;
struct cgltf_accessor;

SEEK_NAMESPACE_BEGIN

class GltfDataBuilder
{
public:
    // 入口：从文件路径构建完整 GltfData
    // 内部调用 cgltf_parse_file → cgltf_load_buffers → Extract* 系列方法 → cgltf_free
    SResult Build(const std::string& filePath, GltfData& outData);

private:
    // 提取方法（按依赖顺序调用）
    void ExtractBuffers(::cgltf_data* data, GltfData& out);
    void ExtractImages(::cgltf_data* data, GltfData& out);
    void ExtractSamplers(::cgltf_data* data, GltfData& out);
    void ExtractTextures(::cgltf_data* data, GltfData& out);
    void ExtractMaterials(::cgltf_data* data, GltfData& out);
    void ExtractMeshes(::cgltf_data* data, GltfData& out);
    void ExtractSkins(::cgltf_data* data, GltfData& out);
    void ExtractNodes(::cgltf_data* data, GltfData& out);
    void ExtractAnimations(::cgltf_data* data, GltfData& out);
    void ExtractLights(::cgltf_data* data, GltfData& out);
    void ExtractScenes(::cgltf_data* data, GltfData& out);

    // 粒度方法
    GltfPrimitive ExtractPrimitive(::cgltf_data* data, ::cgltf_primitive* prim);
    GltfImage     ExtractImage(::cgltf_data* data, ::cgltf_image* img, bool sRGB);
    GltfAnimationChannel ExtractChannel(::cgltf_data* data,
                                        ::cgltf_animation_channel* ch,
                                        ::cgltf_animation_sampler* sampler);
    // 辅助方法
    static const uint8_t* AccessorData(const ::cgltf_accessor* accessor);
    static uint32_t       AccessorStride(const ::cgltf_accessor* accessor);

    std::string m_filePath;
};

SEEK_NAMESPACE_END
```

- [ ] **Step 2: 创建 gltf_data_builder.cpp — 头文件 + Build() + 辅助方法**

```cpp
// 先引入 cgltf 类型声明
#include "cgltf.h"

#include "importer/gltf_data_builder.h"
#include "importer/gltf2.h"
#include "utils/image_decode.h"
#include "utils/log.h"
#include "utils/timer.h"
#include "utils/error.h"
#include "math/math_utility.h"
#include "resource/resource_mgr.h"
#include <fstream>

#ifdef LoadImage
#undef LoadImage
#endif

// 引入 cgltf 实现
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

using namespace std;
using namespace seek_engine;

SEEK_NAMESPACE_BEGIN

// 将 cgltf 类型引入 seek_engine 命名空间
using ::cgltf_data;
using ::cgltf_primitive;
using ::cgltf_image;
using ::cgltf_animation_channel;
using ::cgltf_animation_sampler;
using ::cgltf_accessor;

// 辅助方法
const uint8_t* GltfDataBuilder::AccessorData(const ::cgltf_accessor* accessor)
{
    if (!accessor || !accessor->buffer_view)
        return nullptr;
    cgltf_buffer_view* bv = accessor->buffer_view;
    return static_cast<const uint8_t*>(bv->buffer->data)
           + accessor->offset + bv->offset;
}

uint32_t GltfDataBuilder::AccessorStride(const ::cgltf_accessor* accessor)
{
    if (!accessor)
        return 0;
    if (accessor->stride > 0)
        return (uint32_t)accessor->stride;
    return (uint32_t)gltf::ElementSize(
        gltf::CgltfToGltfComponentType((int)accessor->component_type),
        (int)accessor->type);
}

// 主入口
SResult GltfDataBuilder::Build(const std::string& filePath, GltfData& outData)
{
    m_filePath = filePath;
    LOG_INFO("GltfDataBuilder: gltf file path: %s", filePath.c_str());

    TIMER_BEG(t0);
    cgltf_options options = {};
    ::cgltf_data* data = nullptr;
    cgltf_result cres = cgltf_parse_file(&options, filePath.c_str(), &data);
    if (cres != cgltf_result_success)
    {
        LOG_ERROR("GltfDataBuilder: cgltf_parse_file fail");
        return ERR_INVALID_ARG;
    }
    TIMER_END(t0, "GltfDataBuilder: parse file");

    TIMER_BEG(t1);
    cres = cgltf_load_buffers(&options, data, filePath.c_str());
    if (cres != cgltf_result_success)
    {
        LOG_ERROR("GltfDataBuilder: cgltf_load_buffers fail");
        cgltf_free(data);
        return ERR_INVALID_ARG;
    }
    TIMER_END(t1, "GltfDataBuilder: load buffers");

    TIMER_BEG(t2);
    cres = cgltf_validate(data);
    if (cres != cgltf_result_success)
        LOG_WARNING("GltfDataBuilder: cgltf_validate warning");
    TIMER_END(t2, "GltfDataBuilder: validate");

    // 按依赖顺序提取
    ExtractBuffers(data, outData);
    ExtractImages(data, outData);
    ExtractSamplers(data, outData);
    ExtractTextures(data, outData);
    ExtractMaterials(data, outData);
    ExtractMeshes(data, outData);
    ExtractSkins(data, outData);
    ExtractNodes(data, outData);
    ExtractAnimations(data, outData);
    ExtractLights(data, outData);
    ExtractScenes(data, outData);

    // GltfData 已自包含，cgltf 可以释放
    cgltf_free(data);
    return S_Success;
}
```

- [ ] **Step 3: 实现 ExtractBuffers + ExtractImages + ExtractSamplers + ExtractTextures**

```cpp
// === 1. Buffers ===
void GltfDataBuilder::ExtractBuffers(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->buffers_count; i++)
    {
        GltfBuffer buf;
        buf.data = static_cast<uint8_t*>(data->buffers[i].data);
        buf.size = data->buffers[i].size;
        // 用 shared_ptr 接管 buffer 生命周期
        buf.resource = std::make_shared<BufferResource>();
        buf.resource->_data = buf.data;
        buf.resource->_size = buf.size;
        out.buffers.push_back(std::move(buf));
    }
}

// === 2. Images ===
GltfImage GltfDataBuilder::ExtractImage(::cgltf_data* data, ::cgltf_image* img, bool sRGB)
{
    GltfImage outImg;
    if (img->mime_type)
        outImg.mimeType = img->mime_type;

    ImageType imageType = ImageType::UNKNOWN;
    if (img->mime_type)
    {
        if (strcmp(img->mime_type, "image/jpeg") == 0)
            imageType = ImageType::JPEG;
        else if (strcmp(img->mime_type, "image/png") == 0)
            imageType = ImageType::PNG;
    }
    // 即使无 mime_type 也尝试解码（某些 glTF 文件省略 mime_type）

    if (img->buffer_view)
    {
        cgltf_buffer_view* bv = img->buffer_view;
        uint8_t* data_ptr = static_cast<uint8_t*>(bv->buffer->data) + bv->offset;
        outImg.bitmapData = ImageDecode(data_ptr, (uint32_t)bv->size, imageType);
    }
    else if (img->uri)
    {
        outImg.uriPath = img->uri;
        // 外部文件加载
        std::string absPath = m_filePath.substr(0, m_filePath.find_last_of("/\\"))
                              + "/" + img->uri;
        std::shared_ptr<FileResource> uriFileRes = MakeSharedPtr<FileResource>(absPath);
        if (uriFileRes->IsAvailable())
        {
            uint8_t* data_ptr = (uint8_t*)(uriFileRes->_data);
            outImg.bitmapData = ImageDecode(data_ptr, (uint32_t)uriFileRes->_size, imageType);
        }
        else
            LOG_ERROR("GltfDataBuilder: load uri file %s fail", absPath.c_str());
    }

    return outImg;
}

void GltfDataBuilder::ExtractImages(::cgltf_data* data, GltfData& out)
{
    // 先遍历 textures 收集哪些 image 需要 sRGB 处理
    // 简便做法：对所有 image 都处理，sRGB 标志由后续材质阶段设置
    for (size_t i = 0; i < data->images_count; i++)
    {
        out.images.push_back(ExtractImage(data, &data->images[i], false));
    }
}

// === 3. Samplers ===
void GltfDataBuilder::ExtractSamplers(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->samplers_count; i++)
    {
        GltfSampler s;
        if (data->samplers[i].has_min_filter)
            s.minFilter = data->samplers[i].min_filter;
        if (data->samplers[i].has_mag_filter)
            s.magFilter = data->samplers[i].mag_filter;
        s.wrapS = data->samplers[i].wrap_s;
        s.wrapT = data->samplers[i].wrap_t;
        out.samplers.push_back(s);
    }
}

// === 4. Textures ===
void GltfDataBuilder::ExtractTextures(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->textures_count; i++)
    {
        cgltf_texture* ctex = &data->textures[i];
        GltfTexture tex;
        if (ctex->image)
            tex.imageIndex = (uint32_t)(ctex->image - data->images);
        if (ctex->sampler)
            tex.samplerIndex = (uint32_t)(ctex->sampler - data->samplers);
        // texCoord/scale 在材质纹理引用中设定，此处为默认值
        out.textures.push_back(tex);
    }
}
```

- [ ] **Step 4: 实现 ExtractMaterials**

```cpp
// === 5. Materials ===
void GltfDataBuilder::ExtractMaterials(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->materials_count; i++)
    {
        cgltf_material* mat = &data->materials[i];
        GltfMaterial m;

        if (mat->name)
            m.name = mat->name;

        // PBR metallic-roughness
        if (mat->has_pbr_metallic_roughness)
        {
            cgltf_pbr_metallic_roughness* pbr = &mat->pbr_metallic_roughness;
            m.albedoFactor = float4{pbr->base_color_factor[0],
                                    pbr->base_color_factor[1],
                                    pbr->base_color_factor[2],
                                    pbr->base_color_factor[3]};
            m.metallicFactor  = pbr->metallic_factor;
            m.roughnessFactor = pbr->roughness_factor;

            if (pbr->base_color_texture.texture)
            {
                m.albedoTextureIndex = (uint32_t)(pbr->base_color_texture.texture - data->textures);
            }
            if (pbr->metallic_roughness_texture.texture)
            {
                m.metallicRoughnessTextureIndex = (uint32_t)(pbr->metallic_roughness_texture.texture - data->textures);
            }
        }

        // 法线贴图
        if (mat->normal_texture.texture)
        {
            m.normalTextureIndex = (uint32_t)(mat->normal_texture.texture - data->textures);
            m.normalScale = mat->normal_texture.scale;
        }

        // 遮挡贴图
        if (mat->occlusion_texture.texture)
        {
            m.occlusionTextureIndex = (uint32_t)(mat->occlusion_texture.texture - data->textures);
        }

        // 自发光贴图
        if (mat->emissive_texture.texture)
        {
            m.emissiveTextureIndex = (uint32_t)(mat->emissive_texture.texture - data->textures);
        }
        m.emissiveFactor = float3{mat->emissive_factor[0],
                                  mat->emissive_factor[1],
                                  mat->emissive_factor[2]};

        // Alpha mode
        switch (mat->alpha_mode)
        {
        case cgltf_alpha_mode_mask:  m.alphaMode = AlphaMode::Mask;  break;
        case cgltf_alpha_mode_blend: m.alphaMode = AlphaMode::Blend; break;
        default:                     m.alphaMode = AlphaMode::Opaque; break;
        }
        m.alphaCutoff = mat->alpha_cutoff;
        m.doubleSided = mat->double_sided;

        // Extensions
        if (mat->has_clearcoat)
        {
            m.hasClearcoat = true;
            m.clearcoatFactor     = mat->clearcoat.clearcoat_factor;
            m.clearcoatRoughnessFactor = mat->clearcoat.clearcoat_roughness_factor;
            if (mat->clearcoat.clearcoat_texture.texture)
                m.clearcoatTextureIndex = (uint32_t)(mat->clearcoat.clearcoat_texture.texture - data->textures);
            if (mat->clearcoat.clearcoat_roughness_texture.texture)
                m.clearcoatRoughnessTextureIndex = (uint32_t)(mat->clearcoat.clearcoat_roughness_texture.texture - data->textures);
        }

        if (mat->has_sheen)
        {
            m.hasSheen = true;
            m.sheenColorFactor     = float3{mat->sheen.sheen_color_factor[0],
                                            mat->sheen.sheen_color_factor[1],
                                            mat->sheen.sheen_color_factor[2]};
            m.sheenRoughnessFactor = mat->sheen.sheen_roughness_factor;
            if (mat->sheen.sheen_color_texture.texture)
                m.sheenColorTextureIndex = (uint32_t)(mat->sheen.sheen_color_texture.texture - data->textures);
            if (mat->sheen.sheen_roughness_texture.texture)
                m.sheenRoughnessTextureIndex = (uint32_t)(mat->sheen.sheen_roughness_texture.texture - data->textures);
        }

        if (mat->has_ior)
            m.ior = mat->ior.ior;

        out.materials.push_back(std::move(m));
    }
}
```

- [ ] **Step 5: 实现 ExtractMeshes + ExtractPrimitive + MorphTarget**

```cpp
// === 6. Meshes + Primitives ===
GltfPrimitive GltfDataBuilder::ExtractPrimitive(::cgltf_data* data,
                                                 ::cgltf_primitive* prim)
{
    GltfPrimitive gp;
    if (prim->attributes_count == 0)
        return gp;

    // 拓扑类型（cgltf_primitive_type 与 gltf2.h 枚举差 1）
    gp.topology = gltf::ConvertToTopologyType((int)prim->type - 1);

    // 顶点属性解析
    std::map<uint32_t, VertexStream> vertexStreams;
    std::map<uint32_t, std::shared_ptr<BufferResource>> blendIndexBufferRes;

    for (size_t i = 0; i < prim->attributes_count; i++)
    {
        cgltf_attribute* attr = &prim->attributes[i];
        ::cgltf_accessor* accessor = attr->data;
        if (!accessor || !accessor->buffer_view)
            continue;

        cgltf_buffer_view* bv = accessor->buffer_view;
        auto vertexAttribute = gltf::ConvertVertexAttribute(attr->index, attr->type);
        uint32_t bvIndex = (uint32_t)(bv - data->buffer_views);

        VertexStream* vertexStream;
        {
            auto it = vertexStreams.find(bvIndex);
            if (it == vertexStreams.end())
            {
                vertexStreams.emplace(bvIndex, VertexStream{});
                vertexStream = &vertexStreams[bvIndex];
                if (accessor->stride > 0)
                    vertexStream->stride = (uint32_t)accessor->stride;
                else
                    vertexStream->stride = (uint32_t)gltf::ElementSize(
                        gltf::CgltfToGltfComponentType((int)accessor->component_type),
                        (int)accessor->type);
                vertexStream->offset = 0;
            }
            else
                vertexStream = &it->second;
        }

        VertexStreamLayout layout;
        layout.buffer_offset = accessor->offset;
        layout.format = gltf::ConvertToVertexFormat(
            (int)accessor->type,
            gltf::CgltfToGltfComponentType((int)accessor->component_type));
        layout.usage = vertexAttribute.first;
        layout.usage_index = vertexAttribute.second;

        // Joint index 转换：uint8/uint16 → uint32
        if (vertexAttribute.first == VertexElementUsage::BlendIndex)
        {
            layout.format = VertexFormat::UInt4;
            uint32_t jointsDataSize = (uint32_t)accessor->count * 4;
            std::shared_ptr<uint32_t> backJointData{
                new uint32_t[jointsDataSize], default_array_deleter<uint32_t>()
            };
            std::shared_ptr<BufferResource> backJointBuf = MakeSharedPtr<BufferResource>();
            backJointBuf->_uninitializer = [backJointData](IResource*) mutable {
                backJointData.reset();
            };
            backJointBuf->_data = (uint8_t*)backJointData.get();
            backJointBuf->_size = jointsDataSize * sizeof(uint32_t);

            const uint8_t* src = AccessorData(accessor);
            if (accessor->component_type == cgltf_component_type_r_8u)
            {
                for (uint32_t j = 0; j < jointsDataSize; j++)
                    backJointData.get()[j] = (uint32_t)src[j];
            }
            else if (accessor->component_type == cgltf_component_type_r_16u)
            {
                const uint16_t* src16 = (const uint16_t*)src;
                for (uint32_t j = 0; j < jointsDataSize; j++)
                    backJointData.get()[j] = (uint32_t)src16[j];
            }
            vertexStream->stride = 16;
            blendIndexBufferRes.emplace(bvIndex, std::move(backJointBuf));
        }
        vertexStream->layouts.push_back(layout);

        if (vertexAttribute.first == VertexElementUsage::Position)
        {
            if (accessor->has_min && accessor->has_max)
            {
                gp.boundingBox.Max(float3{accessor->max[0], accessor->max[1], accessor->max[2]});
                gp.boundingBox.Min(float3{accessor->min[0], accessor->min[1], accessor->min[2]});
            }
        }
        else if (vertexAttribute.first == VertexElementUsage::Normal)
            gp.hasNormal = true;
        else if (vertexAttribute.first == VertexElementUsage::TexCoord)
            gp.hasTexcoord = true;
        else if (vertexAttribute.first == VertexElementUsage::BlendWeight)
            gp.jointBindSize = (SkinningJointBindSize)((vertexAttribute.second + 1) * 4);
        else if (vertexAttribute.first == VertexElementUsage::Tangent)
            gp.hasTangent = true;
    }

    // 组装 vertex stream 和 buffer 引用
    for (auto& pair : vertexStreams)
    {
        gp.vertexStreams.push_back(pair.second);
        auto blendIt = blendIndexBufferRes.find(pair.first);
        if (blendIt != blendIndexBufferRes.end())
            gp.vertexBuffers.push_back(blendIt->second);
        else
        {
            cgltf_buffer_view* bv = &data->buffer_views[pair.first];
            auto bufRes = MakeSharedPtr<BufferResource>();
            bufRes->_data = static_cast<uint8_t*>(bv->buffer->data) + bv->offset;
            bufRes->_size = bv->size;
            gp.vertexBuffers.push_back(std::move(bufRes));
        }
    }

    // 索引缓冲区
    if (prim->indices)
    {
        ::cgltf_accessor* indices_acc = prim->indices;
        const uint8_t* indexData = AccessorData(indices_acc);
        auto indicesResource = MakeSharedPtr<VertexIndicesResource>();
        indicesResource->_indexBufferType = gltf::ConvertToIndexBufferType(
            gltf::CgltfToGltfComponentType((int)indices_acc->component_type));
        indicesResource->_indexCount = (uint32_t)indices_acc->count;
        indicesResource->_data = const_cast<uint8_t*>(indexData);
        indicesResource->_size = (size_t)indices_acc->count *
            (size_t)gltf::ComponentByteSize(
                gltf::CgltfToGltfComponentType((int)indices_acc->component_type));
        gp.indexResource = indicesResource;
    }

    // Morph targets
    if (prim->targets_count > 0)
    {
        auto morphRes = std::make_shared<MorphTargetResource>();
        size_t targetSize = prim->targets_count;
        cgltf_morph_target* firstTarget = &prim->targets[0];
        ::cgltf_accessor* firstAcc = nullptr;
        for (size_t a = 0; a < firstTarget->attributes_count; a++)
        {
            if (firstTarget->attributes[a].type == cgltf_attribute_type_position)
            {
                firstAcc = firstTarget->attributes[a].data;
                break;
            }
        }
        if (firstAcc)
        {
            int32_t targetCount = (int32_t)firstAcc->count;
            int32_t allTargetDataSize = (int32_t)targetSize * targetCount * 4;
            std::shared_ptr<float> allTargetData{
                new float[allTargetDataSize]{0.0f}, default_array_deleter<float>()
            };
            morphRes->_uninitializer = [allTargetData](IResource*) mutable {
                allTargetData.reset();
            };

            float* allTargetDataPtr = allTargetData.get();
            for (size_t ti = 0; ti < targetSize; ti++)
            {
                cgltf_morph_target* target = &prim->targets[ti];
                for (size_t a = 0; a < target->attributes_count; a++)
                {
                    if (target->attributes[a].type == cgltf_attribute_type_position)
                    {
                        ::cgltf_accessor* acc = target->attributes[a].data;
                        if (!acc || acc->count != (size_t)targetCount)
                            continue;
                        AABBox targets_box;
                        targets_box.Min(float3(0.0f));
                        targets_box.Max(float3(0.0f));
                        if (acc->has_min && acc->has_max)
                        {
                            AABBox tmp_box(float3{acc->min[0], acc->min[1], acc->min[2]},
                                           float3{acc->max[0], acc->max[1], acc->max[2]});
                            targets_box |= tmp_box;
                            gp.boundingBox = gp.boundingBox + targets_box;
                        }
                        const float* targetData =
                            reinterpret_cast<const float*>(AccessorData(acc));
                        for (int32_t j = 0; j < targetCount; j++)
                        {
                            int32_t addr = (j * (int32_t)targetSize + (int32_t)ti) * 4;
                            allTargetDataPtr[addr]     = targetData[0];
                            allTargetDataPtr[addr + 1] = targetData[1];
                            allTargetDataPtr[addr + 2] = targetData[2];
                            targetData += 3;
                        }
                    }
                }
            }
            morphRes->_morphInfo.morph_target_names = std::vector<std::string>();
            morphRes->_morphInfo.morph_target_type = MorphTargetType::Position;
            morphRes->_morphInfo.morph_target_weights.resize(targetSize, 0.0f);
            morphRes->_data = (uint8_t*)allTargetData.get();
            morphRes->_size = allTargetDataSize * sizeof(float);
            gp.morphTargets = std::move(morphRes);
        }
    }

    // 材质索引
    if (prim->material)
        gp.materialIndex = (uint32_t)(prim->material - data->materials);

    return gp;
}

void GltfDataBuilder::ExtractMeshes(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->meshes_count; i++)
    {
        cgltf_mesh* mesh = &data->meshes[i];
        GltfMesh gm;

        if (mesh->name)
            gm.name = mesh->name;

        // Morph weights
        if (mesh->weights_count > 0)
        {
            gm.weights.resize(mesh->weights_count);
            for (size_t w = 0; w < mesh->weights_count; w++)
                gm.weights[w] = mesh->weights[w];
        }

        // Morph target names
        if (mesh->target_names_count > 0)
        {
            gm.targetNames.resize(mesh->target_names_count);
            for (size_t n = 0; n < mesh->target_names_count; n++)
            {
                if (mesh->target_names[n])
                    gm.targetNames[n] = mesh->target_names[n];
            }
        }

        // Primitives
        for (size_t p = 0; p < mesh->primitives_count; p++)
        {
            gm.primitives.push_back(ExtractPrimitive(data, &mesh->primitives[p]));
        }

        out.meshes.push_back(std::move(gm));
    }
}
```

- [ ] **Step 6: 实现 ExtractSkins + ExtractNodes**

```cpp
// === 7. Skins ===
void GltfDataBuilder::ExtractSkins(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->skins_count; i++)
    {
        cgltf_skin* cskin = &data->skins[i];
        GltfSkin skin;

        // Inverse bind matrices（提取数据指针并拷贝）
        if (cskin->inverse_bind_matrices)
        {
            ::cgltf_accessor* accessor = cskin->inverse_bind_matrices;
            if (accessor->type == cgltf_type_mat4)
            {
                const uint8_t* src = AccessorData(accessor);
                for (size_t j = 0; j < accessor->count; j++)
                {
                    skin.inverseBindMatrices.push_back(
                        Matrix4(reinterpret_cast<const float*>(src) + j * 16));
                }
            }
        }

        // Joint node 索引
        for (size_t j = 0; j < cskin->joints_count; j++)
        {
            uint32_t joint_idx = (uint32_t)(cskin->joints[j] - data->nodes);
            skin.jointNodeIndices.push_back(joint_idx);
        }

        // Skeleton root node 索引
        if (cskin->skeleton)
            skin.skeletonNodeIndex = (uint32_t)(cskin->skeleton - data->nodes);

        out.skins.push_back(std::move(skin));
    }
}

// === 8. Nodes ===
void GltfDataBuilder::ExtractNodes(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->nodes_count; i++)
    {
        cgltf_node* node = &data->nodes[i];
        GltfNode gn;

        if (node->name)
            gn.name = node->name;

        // Transform
        if (node->has_matrix)
        {
            gn.hasMatrix = true;
            gn.matrix = Matrix4(node->matrix);
        }
        if (node->has_rotation)
        {
            gn.hasRotation = true;
            gn.rotation = Quaternion(node->rotation[0], node->rotation[1],
                                     node->rotation[2], node->rotation[3]);
        }
        if (node->has_scale)
        {
            gn.hasScale = true;
            gn.scale = float3{node->scale[0], node->scale[1], node->scale[2]};
        }
        if (node->has_translation)
        {
            gn.hasTranslation = true;
            gn.translation = float3{node->translation[0], node->translation[1],
                                    node->translation[2]};
        }

        // 数据引用索引
        if (node->mesh)
            gn.meshIndex = (uint32_t)(node->mesh - data->meshes);
        if (node->skin)
            gn.skinIndex = (uint32_t)(node->skin - data->skins);
        // Light 暂时跳过（KHR_lights_punctual 在 cgltf 中通过 data->lights 访问，
        // 但 node 级别的 light 引用需要手动解析扩展 JSON，当前保持与原来一致不处理）

        // 子节点索引
        for (size_t c = 0; c < node->children_count; c++)
        {
            gn.childIndices.push_back((uint32_t)(node->children[c] - data->nodes));
        }

        out.nodes.push_back(std::move(gn));
    }
}
```

- [ ] **Step 7: 实现 ExtractAnimations + ExtractLights + ExtractScenes**

```cpp
// === 9. Animations ===
GltfAnimationChannel GltfDataBuilder::ExtractChannel(
    ::cgltf_data* data,
    ::cgltf_animation_channel* ch,
    ::cgltf_animation_sampler* sampler)
{
    GltfAnimationChannel channel;
    channel.targetNodeIndex = (uint32_t)(ch->target_node - data->nodes);

    // Transform type
    switch (ch->target_path)
    {
    case cgltf_animation_path_type_translation: channel.transformType = Translate; break;
    case cgltf_animation_path_type_rotation:    channel.transformType = Rotate;    break;
    case cgltf_animation_path_type_scale:       channel.transformType = Scale;     break;
    case cgltf_animation_path_type_weights:     channel.isMorphTarget  = true;     break;
    default: break;
    }

    // Interpolation
    channel.interpolation = gltf::ConvertToInterpolationType((int)sampler->interpolation);

    // 展开 input/output accessor 数据
    ::cgltf_accessor* input_acc  = sampler->input;
    ::cgltf_accessor* output_acc = sampler->output;
    if (input_acc && output_acc)
    {
        const float* inputData = reinterpret_cast<const float*>(AccessorData(input_acc));
        uint32_t inputCount = (uint32_t)input_acc->count;

        const uint8_t* outputData  = AccessorData(output_acc);
        uint32_t outputCount = (uint32_t)output_acc->count;
        uint32_t outputStride = AccessorStride(output_acc);

        if (inputData)
        {
            channel.outputCount  = outputCount;
            channel.outputStride = outputStride;

            // 拷贝 input times
            channel.inputTimes.assign(inputData, inputData + inputCount);

            // 拷贝 output data（outputCount 在 morph target 场景下远大于 inputCount）
            uint32_t outputSize = outputCount * (outputStride / sizeof(float));
            const float* outFloats = reinterpret_cast<const float*>(outputData);
            channel.outputData.assign(outFloats, outFloats + outputSize);
        }
    }
    return channel;
}

void GltfDataBuilder::ExtractAnimations(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->animations_count; i++)
    {
        cgltf_animation* canim = &data->animations[i];
        GltfAnimation anim;

        if (canim->name)
            anim.name = canim->name;

        for (size_t ch = 0; ch < canim->channels_count; ch++)
        {
            cgltf_animation_channel* channel = &canim->channels[ch];
            cgltf_animation_sampler* sampler = channel->sampler;
            if (!channel->target_node || !sampler)
                continue;

            anim.channels.push_back(ExtractChannel(data, channel, sampler));
        }

        out.animations.push_back(std::move(anim));
    }
}

// === 10. Lights ===
void GltfDataBuilder::ExtractLights(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->lights_count; i++)
    {
        cgltf_light* light = &data->lights[i];
        GltfLight gl;

        if (light->name)
            gl.name = light->name;

        switch (light->type)
        {
        case cgltf_light_type_directional: gl.type = LightType::Directional; break;
        case cgltf_light_type_point:       gl.type = LightType::Point;       break;
        case cgltf_light_type_spot:        gl.type = LightType::Spot;        break;
        default: continue;
        }

        gl.color = Color(uint8_t(light->color[0] * 255.0f),
                         uint8_t(light->color[1] * 255.0f),
                         uint8_t(light->color[2] * 255.0f));
        gl.intensity = light->intensity;

        out.lights.push_back(std::move(gl));
    }
}

// === 11. Scenes ===
void GltfDataBuilder::ExtractScenes(::cgltf_data* data, GltfData& out)
{
    if (data->scene)
        out.defaultSceneIndex = (uint32_t)(data->scene - data->scenes);

    for (size_t i = 0; i < data->scenes_count; i++)
    {
        cgltf_scene* scene = &data->scenes[i];
        GltfScene gs;

        if (scene->name)
            gs.name = scene->name;

        for (size_t n = 0; n < scene->nodes_count; n++)
        {
            gs.rootNodeIndices.push_back((uint32_t)(scene->nodes[n] - data->nodes));
        }

        out.scenes.push_back(std::move(gs));
    }
}

SEEK_NAMESPACE_END
```

- [ ] **Step 8: 验证 Builder 编译通过**

```powershell
cmake --build build --config Debug --target seek_engine-static 2>&1 | Select-Object -Last 30
```

预期：Builder 编译通过，可能 Assembler 相关符号未定义（Linker error，下一任务解决）。

---

### Task 3: 创建 gltf_scene_assembler.h + .cpp — Phase 2 实现

**Files:**
- Create: `engine/importer/gltf_scene_assembler.h`
- Create: `engine/importer/gltf_scene_assembler.cpp`

- [ ] **Step 1: 创建 gltf_scene_assembler.h**

```cpp
#pragma once

#include "importer/gltf_data.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

class GltfSceneAssembler
{
public:
    explicit GltfSceneAssembler(Context* ctx)
        : m_pContext(ctx)
    {}

    // 入口：从 GltfData 构建引擎 Component
    SResult Assemble(const GltfData& data,
                     SceneComponentPtr& outScene,
                     std::vector<AnimationComponentPtr>& outAnimations);

    // 访问器（供 Loader 转发）
    std::map<uint32_t, MeshComponentPtr>&           GetMeshComponentMap()   { return m_meshes; }
    std::map<uint32_t, SceneComponentPtr>&          GetSceneComponentMap()  { return m_nodes; }
    std::map<std::string, SceneComponentPtr>&       GetSceneComponentMapByName() { return m_nodesByName; }

private:
    void BuildMaterials(const GltfData& data);
    void BuildMeshes(const GltfData& data);
    void BuildSkins(const GltfData& data);
    SceneComponentPtr BuildNode(const GltfData& data, uint32_t idx);
    void BuildAnimations(const GltfData& data,
                         std::vector<AnimationComponentPtr>& outAnimations);
    SceneComponentPtr BuildScene(const GltfData& data);

    // 粒度方法
    std::shared_ptr<MaterialResource> BuildMaterial(const GltfData& data, uint32_t idx);
    RHIMeshPtr BuildPrimitive(const GltfData& data, const GltfPrimitive& prim);

    Context* m_pContext;

    // 构建产物缓存（按索引）
    std::map<uint32_t, std::shared_ptr<MaterialResource>> m_materials;
    std::map<uint32_t, MeshComponentPtr>                  m_meshes;
    std::map<uint32_t, SceneComponentPtr>                 m_nodes;
    std::map<std::string, SceneComponentPtr>              m_nodesByName;
};

SEEK_NAMESPACE_END
```

- [ ] **Step 2: 创建 .cpp — includes + Assemble + BuildMaterials + BuildPrimitive**

```cpp
#include "importer/gltf_scene_assembler.h"
#include "importer/gltf2.h"
#include "utils/image_decode.h"
#include "utils/log.h"
#include "utils/timer.h"
#include "utils/error.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "components/scene_component.h"
#include "components/mesh_component.h"
#include "components/skeletal_mesh_component.h"
#include "components/light_component.h"
#include "components/camera_component.h"
#include "kernel/context.h"

using namespace std;
using namespace seek_engine;

SEEK_NAMESPACE_BEGIN

// 缓存查找宏
#define RETURN_IF_FOUND(map, key) \
    { \
        auto it = (map).find(key); \
        if (it != (map).end()) \
            return it->second; \
    }

SResult GltfSceneAssembler::Assemble(const GltfData& data,
                                      SceneComponentPtr& outScene,
                                      vector<AnimationComponentPtr>& outAnimations)
{
    // 清空缓存（支持多次调用）
    m_materials.clear();
    m_meshes.clear();
    m_nodes.clear();
    m_nodesByName.clear();

    TIMER_BEG(t0);
    BuildMaterials(data);       // 1. MaterialResource（无依赖）
    BuildMeshes(data);          // 2. MeshComponent + RHIMesh（依赖 materials）
    BuildSkins(data);           // 3. 蒙皮数据关联（记录关联信息）
    // BuildNodes 递归构建节点树（内部使用 BuildNode，依赖 meshes/skins）
    BuildAnimations(data, outAnimations); // 4. AnimationComponent（依赖 nodes）
    outScene = BuildScene(data);  // 5. 根节点组装
    TIMER_END(t0, "GltfSceneAssembler: assemble");

    return S_Success;
}

// === BuildMaterials ===
void GltfSceneAssembler::BuildMaterials(const GltfData& data)
{
    for (size_t i = 0; i < data.materials.size(); i++)
    {
        BuildMaterial(data, (uint32_t)i);
    }
}

std::shared_ptr<MaterialResource> GltfSceneAssembler::BuildMaterial(
    const GltfData& data, uint32_t idx)
{
    RETURN_IF_FOUND(m_materials, idx);
    if (idx >= data.materials.size())
        return nullptr;

    const GltfMaterial& m = data.materials[idx];
    auto mat = MakeSharedPtr<MaterialResource>();
    mat->_name = m.name;

    // 辅助 lambda：texture 索引 → image bitmap（二次查表）
    auto GetImage = [&data](uint32_t texIdx) -> BitmapBufferPtr {
        if (texIdx >= data.textures.size())
            return nullptr;
        uint32_t imgIdx = data.textures[texIdx].imageIndex;
        if (imgIdx >= data.images.size())
            return nullptr;
        return data.images[imgIdx].bitmapData;
    };

    // PBR 参数
    mat->_albedoFactor    = m.albedoFactor;
    mat->_metallicFactor  = m.metallicFactor;
    mat->_roughnessFactor = m.roughnessFactor;

    // 纹理引用（texture 索引 → GltfTexture → image 索引 → GltfImage → bitmap）
    mat->_albedoImage              = GetImage(m.albedoTextureIndex);
    mat->_metallicRoughnessImage   = GetImage(m.metallicRoughnessTextureIndex);
    mat->_normalImage              = GetImage(m.normalTextureIndex);
    mat->_normalScale              = m.normalScale;
    mat->_occlusionImage           = GetImage(m.occlusionTextureIndex);
    mat->_emmissiveImage           = GetImage(m.emissiveTextureIndex);
    mat->_emissiveFactor           = m.emissiveFactor;

    // Alpha mode
    mat->_alphaMode    = m.alphaMode;
    mat->_alphaCutoff  = m.alphaCutoff;
    mat->_doubleSided  = m.doubleSided;

    // Clearcoat
    if (m.hasClearcoat)
    {
        mat->_clearcoatFactor          = m.clearcoatFactor;
        mat->_clearcoatRoughnessFactor = m.clearcoatRoughnessFactor;
        mat->_clearcoatImage           = GetImage(m.clearcoatTextureIndex);
        mat->_clearcoatRoughnessImage  = GetImage(m.clearcoatRoughnessTextureIndex);
    }

    // Sheen
    if (m.hasSheen)
    {
        mat->_sheenColorFactor     = m.sheenColorFactor;
        mat->_sheenRoughnessFactor = m.sheenRoughnessFactor;
        mat->_sheenColorImage      = GetImage(m.sheenColorTextureIndex);
        mat->_sheenRoughnessImage  = GetImage(m.sheenRoughnessTextureIndex);
    }

    // IOR
    mat->_IORFactor = m.ior;

    m_materials[idx] = mat;
    return mat;
}

// === BuildMeshes + BuildPrimitive ===
void GltfSceneAssembler::BuildMeshes(const GltfData& data)
{
    for (size_t i = 0; i < data.meshes.size(); i++)
    {
        const GltfMesh& gm = data.meshes[i];
        MeshComponentPtr meshComp = MakeSharedPtr<MeshComponent>(m_pContext);

        if (!gm.name.empty())
            meshComp->SetName(gm.name);

        AABBox meshes_box;
        meshes_box.Min(float3(0.0f));
        meshes_box.Max(float3(0.0f));

        for (size_t p = 0; p < gm.primitives.size(); p++)
        {
            RHIMeshPtr rhiMesh = BuildPrimitive(data, gm.primitives[p]);
            if (rhiMesh)
            {
                meshComp->AddMesh(rhiMesh);

                // Morph target names/weights
                MorphInfo& morphInfo = rhiMesh->GetMorphTargetResource()._morphInfo;
                if (morphInfo.morph_target_names.empty() && !gm.targetNames.empty())
                    morphInfo.morph_target_names = gm.targetNames;
                if (!gm.weights.empty())
                    morphInfo.morph_target_weights = gm.weights;

                AABBox meshBox = rhiMesh->GetAABBox();
                if (p == 0)
                    meshes_box = meshBox;
                else
                    meshes_box |= meshBox;
            }
        }
        meshComp->SetAABBox(meshes_box);
        m_meshes[(uint32_t)i] = meshComp;
    }
}

RHIMeshPtr GltfSceneAssembler::BuildPrimitive(const GltfData& data,
                                               const GltfPrimitive& prim)
{
    RHIContext* rc = &m_pContext->RHIContextInstance();
    RHIMeshPtr mesh = rc->CreateMesh();
    if (!mesh)
        return mesh;

    mesh->SetTopologyType(prim.topology);
    mesh->SetSkinningJointBindSize(prim.jointBindSize);

    // Vertex attribute resource
    VertexAttributeResource vertexAttrRes;
    vertexAttrRes._vertexStreams   = prim.vertexStreams;
    vertexAttrRes._vertexBuffers   = prim.vertexBuffers;
    mesh->SetVertexAttributeResource(vertexAttrRes);

    // Index buffer
    if (prim.indexResource)
        mesh->SetIndexBufferResource(prim.indexResource);

    // Morph targets
    if (prim.morphTargets)
        mesh->SetMorphTargetResource(*prim.morphTargets);

    // AABB
    mesh->SetAABBox(prim.boundingBox);

    // 材质
    if (prim.materialIndex < data.materials.size())
    {
        auto mat = BuildMaterial(data, prim.materialIndex);
        if (mat)
            mesh->SetMaterialResource(mat);
        else
        {
            auto defaultMat = MakeSharedPtr<MaterialResource>();
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

- [ ] **Step 3: 实现 BuildSkins + BuildNode + BuildScene**

```cpp
// === BuildSkins ===
void GltfSceneAssembler::BuildSkins(const GltfData& data)
{
    // Skin 的 joint SceneComponent 引用需要在 BuildNode 中建立
    // 此处仅通过 BuildNode 的延迟构建来处理
    // BuildNode 中会检查 node 的 skinIndex，然后：
    // 1. 先构建所有 joint nodes（递归调用 BuildNode）
    // 2. 读取 GltfSkin 数据创建 SkeletalMeshComponent
    // 因此 BuildSkins 本身不需要单独逻辑
}

// === BuildNode ===
SceneComponentPtr GltfSceneAssembler::BuildNode(const GltfData& data, uint32_t idx)
{
    RETURN_IF_FOUND(m_nodes, idx);
    SceneComponentPtr sc = nullptr;

    if (idx >= data.nodes.size())
        return sc;

    const GltfNode& gn = data.nodes[idx];

    // 有 mesh 时用 MeshComponent 作为节点
    if (gn.meshIndex < data.meshes.size())
    {
        bool hasSkin = gn.skinIndex < data.skins.size();
        if (hasSkin)
        {
            const GltfSkin& skin = data.skins[gn.skinIndex];
            SkeletalMeshComponentPtr sklMeshComp =
                MakeSharedPtr<SkeletalMeshComponent>(m_pContext);

            // 加载 joint nodes（递归）
            std::vector<SceneComponentPtr> joints;
            for (uint32_t ji : skin.jointNodeIndices)
            {
                SceneComponentPtr jointSC = BuildNode(data, ji);
                if (jointSC)
                    joints.push_back(jointSC);
            }

            sklMeshComp->SetInverseBindMatrix(skin.inverseBindMatrices);
            sklMeshComp->SetJoint(joints);

            // Skeleton root
            if (skin.skeletonNodeIndex < data.nodes.size())
            {
                SceneComponentPtr skeleton = BuildNode(data, skin.skeletonNodeIndex);
                if (skeleton)
                    sklMeshComp->SetSkeletonRoot(skeleton);
            }

            sc = sklMeshComp;
        }
        else
        {
            MeshComponentPtr meshComp = m_meshes[gn.meshIndex];
            sc = meshComp;
        }
    }
    else
        sc = MakeSharedPtr<SceneComponent>(m_pContext);

    m_nodes[idx] = sc;

    // Transform
    if (gn.hasMatrix)
        sc->SetLocalTransform(gn.matrix);
    if (gn.hasRotation)
        sc->SetLocalRotation(gn.rotation);
    if (gn.hasScale)
        sc->SetLocalScale(gn.scale);
    if (gn.hasTranslation)
        sc->SetLocalTranslation(gn.translation);

    // 名称
    if (!gn.name.empty())
        sc->SetName(gn.name);

    // 子节点递归
    for (uint32_t ci : gn.childIndices)
    {
        SceneComponentPtr child = BuildNode(data, ci);
        if (child)
        {
            sc->AddChild(child);
            child->SetParent(sc.get());
        }
    }

    // 名称索引
    if (!sc->GetName().empty())
        m_nodesByName[sc->GetName()] = sc;

    return sc;
}

// === BuildScene ===
SceneComponentPtr GltfSceneAssembler::BuildScene(const GltfData& data)
{
    SceneComponentPtr rootSC = nullptr;
    if (data.defaultSceneIndex < data.scenes.size())
    {
        const GltfScene& scene = data.scenes[data.defaultSceneIndex];
        rootSC = MakeSharedPtr<SceneComponent>(m_pContext);

        if (!scene.name.empty())
            rootSC->SetName(scene.name);

        for (uint32_t ni : scene.rootNodeIndices)
        {
            SceneComponentPtr child = BuildNode(data, ni);
            if (child)
            {
                rootSC->AddChild(child);
                child->SetParent(rootSC.get());
            }
        }
    }
    return rootSC;
}
```

- [ ] **Step 4: 实现 BuildAnimations**

```cpp
// === BuildAnimations ===
void GltfSceneAssembler::BuildAnimations(
    const GltfData& data,
    vector<AnimationComponentPtr>& outAnimations)
{
    for (size_t animIdx = 0; animIdx < data.animations.size(); animIdx++)
    {
        const GltfAnimation& ga = data.animations[animIdx];
        AnimationComponentPtr anim = MakeSharedPtr<AnimationComponent>(m_pContext);

        if (!ga.name.empty())
            anim->SetName(ga.name);

        float startTime = 0.0f;
        float endTime   = 0.0f;

        for (size_t chIdx = 0; chIdx < ga.channels.size(); chIdx++)
        {
            const GltfAnimationChannel& ch = ga.channels[chIdx];
            SceneComponentPtr sc = BuildNode(data, ch.targetNodeIndex);
            if (!sc)
            {
                LOG_ERROR("BuildAnimations: node not found for channel");
                continue;
            }

            TransformAnimationTrackPtr tAnimTrack = nullptr;
            MorphTargetAnimationTrackPtr mtAnimTrack = nullptr;
            AnimationTrackPtr animTrack = nullptr;

            if (!ch.isMorphTarget)
            {
                tAnimTrack = anim->CreateTransformTrack();
                tAnimTrack->SetSceneComponent(sc);
                animTrack = tAnimTrack;
            }
            else
            {
                mtAnimTrack = anim->CreateMorphTargetTrack();
                mtAnimTrack->SetSceneComponent(sc);
                animTrack = mtAnimTrack;
            }

            animTrack->SetInterpolationType(ch.interpolation);

            uint32_t count = (uint32_t)ch.inputTimes.size();
            for (uint32_t k = 0; k < count; k++)
            {
                if (tAnimTrack)
                {
                    TransformKeyFramePtr kf = tAnimTrack->CreateTransformKeyFrame();
                    float time = ch.inputTimes[k];
                    kf->SetTime(time);
                    if (k == 0)
                        startTime = time;
                    endTime = time;

                    if (ch.transformType == Translate && ch.outputStride == 12)
                    {
                        const float* src = &ch.outputData[3 * k];
                        float3 translation{src[0], src[1], src[2]};
                        kf->SetTranslate(translation);
                        kf->SetOffsetTranslate(
                            translation - sc->GetLocalTransform().GetTranslation());
                    }
                    else if (ch.transformType == Rotate && ch.outputStride == 16)
                    {
                        const float* src = &ch.outputData[4 * k];
                        Quaternion qua(src[0], src[1], src[2], src[3]);
                        kf->SetRotate(qua);
                        kf->SetOffsetRotate(
                            (sc->GetLocalTransform().GetRotation().Inverse()) * qua);
                    }
                    else if (ch.transformType == Scale && ch.outputStride == 12)
                    {
                        const float* src = &ch.outputData[3 * k];
                        float3 scale{src[0], src[1], src[2]};
                        kf->SetScale(scale);
                        kf->SetOffsetScale(
                            scale / sc->GetLocalTransform().GetScale());
                    }
                    else
                        LOG_ERROR("BuildAnimations: data format not supported");
                }
                else if (mtAnimTrack)
                {
                    MorphTargetKeyFramePtr kf = mtAnimTrack->CreateMorphTargetKeyFrame();
                    float time = ch.inputTimes[k];
                    kf->SetTime(time);
                    if (k == 0)
                        startTime = time;
                    endTime = time;

                    uint32_t weightCount = ch.outputCount / count;
                    const float* wp = &ch.outputData[k * weightCount];
                    vector<float> weights(wp, wp + weightCount);
                    kf->SetWeights(weights);
                }
            }
        }

        if (anim)
        {
            AnimInfo animInfo;
            animInfo.startTime    = startTime;
            animInfo.preFrameTime = startTime;
            animInfo.endTime      = endTime;
            animInfo.state        = AnimationState::Stopped;
            animInfo.loop         = true;
            anim->AddAnimSectionInfo(animInfo);
        }
        outAnimations.push_back(anim);
    }
}

SEEK_NAMESPACE_END
```

---

### Task 4: 重写 gltf2_loader.h + .cpp — 薄壳组合

**Files:**
- Modify: `engine/importer/gltf2_loader.h`
- Modify: `engine/importer/gltf2_loader.cpp`

- [ ] **Step 1: 重写 gltf2_loader.h** — 用以下内容替换整个文件

```cpp
#pragma once

#include "seek_engine.h"
#include "resource/resource_mgr.h"
#include "importer/loader.h"
#include "importer/gltf_data.h"
#include "importer/gltf_data_builder.h"
#include "importer/gltf_scene_assembler.h"
#include "components/animation_component.h"

#undef LoadImage

SEEK_NAMESPACE_BEGIN

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
    Context*             m_pContext;
    GltfDataBuilder      m_builder;
    GltfSceneAssembler   m_assembler;
    GltfData             m_data;
    std::string          m_filePath;
    bool                 m_isLoadSucceed;
};

SEEK_NAMESPACE_END
```

- [ ] **Step 2: 重写 gltf2_loader.cpp** — 用以下内容替换整个文件

```cpp
#include "importer/gltf2_loader.h"
#include "utils/log.h"
#include "utils/timer.h"
#include "utils/error.h"

using namespace std;
using namespace seek_engine;

#define SEEK_MACRO_FILE_UID 81

SEEK_NAMESPACE_BEGIN

glTF2_Loader::glTF2_Loader(Context* ctx)
    : m_pContext(ctx)
    , m_assembler(ctx)
{
    m_isLoadSucceed = true;
}

glTF2_Loader::~glTF2_Loader()
{
}

SResult glTF2_Loader::LoadSceneFromFile(std::string const& filePath,
                                         SceneComponentPtr& scene,
                                         vector<AnimationComponentPtr>& animations)
{
    LOG_INFO("glTF2_Loader: gltf file path: %s", filePath.c_str());
    m_filePath = filePath;

    // Phase 1: 解析 glTF 构建 IR
    TIMER_BEG(t0);
    SResult ret = m_builder.Build(filePath, m_data);
    TIMER_END(t0, "glTF2_Loader: Phase 1 - Build IR");
    if (SEEK_CHECKFAILED(ret))
        return ret;

    // Phase 2: IR → 引擎 Component
    TIMER_BEG(t1);
    ret = m_assembler.Assemble(m_data, scene, animations);
    TIMER_END(t1, "glTF2_Loader: Phase 2 - Assemble Components");
    if (SEEK_CHECKFAILED(ret))
        return ret;

    scene->SetName(filePath);
    return S_Success;
}

std::map<uint32_t, MeshComponentPtr>& glTF2_Loader::GetMeshComponentMap()
{
    return m_assembler.GetMeshComponentMap();
}

std::map<uint32_t, SceneComponentPtr>& glTF2_Loader::GetSceneComponentMap()
{
    return m_assembler.GetSceneComponentMap();
}

std::map<std::string, SceneComponentPtr>& glTF2_Loader::GetSceneComponentMapByName()
{
    return m_assembler.GetSceneComponentMapByName();
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID
```

---

### Task 5: 更新 CMakeLists.txt + 构建验证

**Files:**
- Modify: `engine/CMakeLists.txt`

- [ ] **Step 1: 在 CMakeLists.txt 中添加新文件**

在 `engine/CMakeLists.txt` 第 73-76 行（importer 区域），将原来的两行改为：

```cmake
    # importer
    ${SEEK_ENGINE_SOURCE_DIR}/importer/gltf2.h
    ${SEEK_ENGINE_SOURCE_DIR}/importer/gltf_data.h
    ${SEEK_ENGINE_SOURCE_DIR}/importer/gltf_data_builder.h
    ${SEEK_ENGINE_SOURCE_DIR}/importer/gltf_data_builder.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/importer/gltf_scene_assembler.h
    ${SEEK_ENGINE_SOURCE_DIR}/importer/gltf_scene_assembler.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/importer/gltf2_loader.h
    ${SEEK_ENGINE_SOURCE_DIR}/importer/gltf2_loader.cpp
    ${SEEK_ENGINE_SOURCE_DIR}/importer/loader.h
```

具体操作：在第 74 行 `gltf2_loader.h` 之前插入 gltf_data.h、gltf_data_builder.h/.cpp、gltf_scene_assembler.h/.cpp 四行。

- [ ] **Step 2: CMake 配置 + 编译**

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --target seek_engine-static
```

- [ ] **Step 3: 修复编译错误（如有）**

常见可能错误：
1. `GltfImage` 中 `ImageType` 未定义 — 确保 `#include "utils/image_decode.h"` 或等价声明
2. `BitmapBufferPtr` 未定义 — 确认 `resource/resource_mgr.h` 被包含
3. 动画 keyframe 中 `outputCount / count` 除零 — 加保护判断
4. `m_nodesByName` 在 BuildNode 中赋值时机问题 — 确认在 `SetName` 之后添加

- [ ] **Step 4: 构建 Sample 验证运行时**

```powershell
cmake --build build --config Debug --target 05.DeferredShading
.\build\samples\05.DeferredShading\Debug\05.DeferredShading.exe
```

验证 Sponza 场景正常加载和渲染。观察 LOG 输出确认两阶段计时正常：
```
glTF2_Loader: Phase 1 - Build IR: xxx ms
glTF2_Loader: Phase 2 - Assemble Components: xxx ms
```
