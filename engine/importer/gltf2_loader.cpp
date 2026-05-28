// 先引入 cgltf 的类型声明（不带 CGLTF_IMPLEMENTATION）
// 确保头文件处理时 ::cgltf_data 等类型已完整定义
#include "cgltf.h"

#include "gltf2_loader.h"
#include "gltf2.h"                // gltf 命名空间辅助函数（ConvertToTopologyType 等）
#include "utils/image_decode.h"
#include "utils/log.h"
#include "utils/timer.h"
#include "utils/error.h"
#include "math/math_utility.h"

// Windows.h 通过 d3d11 RHI 头文件间接包含，LoadImage 是 Windows API 宏，会与我们的方法名冲突
#ifdef LoadImage
#undef LoadImage
#endif

// 引入 cgltf 的函数实现（定义 CGLTF_IMPLEMENTATION 后再次 include）
// 实现部分在 cgltf.h 的 include guard 之外，第二次 include 仍会处理
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

using namespace std;
using namespace seek_engine;

#define SEEK_MACRO_FILE_UID 81     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

// 将全局命名空间的 cgltf 类型引入 seek_engine
using ::cgltf_data;
using ::cgltf_primitive;
using ::cgltf_material;
using ::cgltf_texture_view;
using ::cgltf_accessor;
using ::cgltf_pbr_metallic_roughness;
using ::cgltf_clearcoat;
using ::cgltf_sheen;

#define RETURN_IF_FOUND(map, key) \
    { \
        auto i = (map).find(key); \
        if (i != (map).end()) { \
            return i->second; \
        } \
    }

// ===== 辅助方法 =====

const uint8_t* glTF2_Loader::AccessorData(const ::cgltf_accessor* accessor)
{
    if (!accessor || !accessor->buffer_view)
        return nullptr;
    cgltf_buffer_view* bv = accessor->buffer_view;
    return static_cast<const uint8_t*>(bv->buffer->data)
           + accessor->offset + bv->offset;
}

uint32_t glTF2_Loader::AccessorStride(const ::cgltf_accessor* accessor)
{
    if (!accessor)
        return 0;
    if (accessor->stride > 0)
        return (uint32_t)accessor->stride;
    // cgltf_component_type 枚举值（1-6）与 gltf2.h 中 GLTF_COMPONENT_TYPE_* 常量（5120-5126）不同，需转换
    return (uint32_t)gltf::ElementSize(
        gltf::CgltfToGltfComponentType((int)accessor->component_type),
        (int)accessor->type);
}

// ===== 构造 / 析构 =====

glTF2_Loader::glTF2_Loader(Context* ctx)
    : m_pContext(ctx)
{
    m_isLoadSucceed = true;
}

glTF2_Loader::~glTF2_Loader()
{
}

// ===== 主入口 =====

SResult glTF2_Loader::LoadSceneFromFile(std::string const& filePath,
                                         SceneComponentPtr& scene,
                                         vector<AnimationComponentPtr>& animations)
{
    SResult ret = S_Success;
    LOG_INFO("glTF2_Loader: gltf file path: %s", filePath.c_str());

    m_filePath = filePath;

    TIMER_BEG(t0);
    cgltf_options options = {};
    ::cgltf_data* data = nullptr;
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

    // 为每个 buffer 创建生命周期管理对象，确保 cgltf_free 后数据指针仍有效
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

// ===== 场景 / 节点 =====

SceneComponentPtr glTF2_Loader::LoadDefaultScene(::cgltf_data* data)
{
    SceneComponentPtr rootSC = nullptr;
    if (data->scene)
    {
        // 指针算术获取默认场景在数组中的索引
        uint32_t index = (uint32_t)(data->scene - data->scenes);
        rootSC = LoadScene(data, index);
    }
    return rootSC;
}

SceneComponentPtr glTF2_Loader::LoadScene(::cgltf_data* data, uint32_t index)
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

SceneComponentPtr glTF2_Loader::LoadNode(::cgltf_data* data, uint32_t index)
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

    // 矩阵变换（matrix 优先于 TRS）
    if (node->has_matrix)
    {
        Matrix4 mat4 = Matrix4(node->matrix);
        sc->SetLocalTransform(mat4);
    }

    // TRS 变换
    if (node->has_rotation)
    {
        const float* r = node->rotation;
        Quaternion qua = Quaternion(r[0], r[1], r[2], r[3]);
        sc->SetLocalRotation(qua);
    }
    if (node->has_scale)
    {
        const float* s = node->scale;
        float3 scale = float3{s[0], s[1], s[2]};
        sc->SetLocalScale(scale);
    }
    if (node->has_translation)
    {
        const float* t = node->translation;
        float3 pos = float3{t[0], t[1], t[2]};
        sc->SetLocalTranslation(pos);
    }

    // 节点名称
    if (node->name)
        sc->SetName(node->name);

    // 子节点递归加载
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

    // 按名称索引，供后续查找
    if (sc->GetName() != "")
        m_NodesMapByName[sc->GetName()] = sc;

    return sc;
}

// ===== 网格 =====

MeshComponentPtr glTF2_Loader::LoadMesh(::cgltf_data* data, uint32_t mesh_index,
                                         bool has_skin, uint32_t skin_index)
{
    RETURN_IF_FOUND(m_Meshes, mesh_index);
    MeshComponentPtr meshComponent = nullptr;

    if (mesh_index >= data->meshes_count)
        return meshComponent;

    cgltf_mesh* mesh = &data->meshes[mesh_index];

    // 蒙皮骨骼网格
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

    // 网格名称
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

    // morph target 名称
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

    // 遍历所有 primitives
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

// ===== 图元 / 顶点数据 =====

RHIMeshPtr glTF2_Loader::LoadPrimitive(::cgltf_data* data,
                                        ::cgltf_primitive* primitive)
{
    RHIMeshPtr mesh = nullptr;
    if (primitive->attributes_count == 0)
        return mesh;

    RHIContext* rc = &m_pContext->RHIContextInstance();
    mesh = rc->CreateMesh();
    if (!mesh)
        return mesh;

    // 拓扑类型
    mesh->SetTopologyType(gltf::ConvertToTopologyType((int)primitive->type - 1));

    // 顶点属性解析
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
        ::cgltf_accessor* accessor = attr->data;
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
                vertexStream->stride = (uint32_t)accessor->stride;
            else
                vertexStream->stride = (uint32_t)gltf::ElementSize(
                    gltf::CgltfToGltfComponentType((int)accessor->component_type),
                    (int)accessor->type);
            vertexStream->offset = 0;
        }

        VertexStreamLayout layout;
        layout.buffer_offset = accessor->offset;
        layout.format = gltf::ConvertToVertexFormat(
            (int)accessor->type,
            gltf::CgltfToGltfComponentType((int)accessor->component_type));
        layout.usage = vertexAttribute.first;
        layout.usage_index = vertexAttribute.second;

        // Joint index 特殊处理
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
                    _backJointData.get()[j] = (uint32_t)src16[j];
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

    for (auto& pair : vertexStreams)
    {
        vertexAttributeRes._vertexStreams.push_back(pair.second);
        if (blendIndexBufferRes.find(pair.first) != blendIndexBufferRes.end())
            vertexAttributeRes._vertexBuffers.push_back(blendIndexBufferRes[pair.first]);
        else
        {
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

    // 索引缓冲区
    if (primitive->indices)
    {
        ::cgltf_accessor* indices_acc = primitive->indices;
        const uint8_t* indexData = AccessorData(indices_acc);
        std::shared_ptr<VertexIndicesResource> indicesResource =
            MakeSharedPtr<VertexIndicesResource>();
        indicesResource->_indexBufferType = gltf::ConvertToIndexBufferType(
            gltf::CgltfToGltfComponentType((int)indices_acc->component_type));
        indicesResource->_indexCount = (uint32_t)indices_acc->count;
        indicesResource->_data = const_cast<uint8_t*>(indexData);
        indicesResource->_size = (size_t)indices_acc->count *
            (size_t)gltf::ComponentByteSize(
                gltf::CgltfToGltfComponentType((int)indices_acc->component_type));
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

// ===== 材质 =====

std::shared_ptr<MaterialResource> glTF2_Loader::LoadMaterial(
    ::cgltf_data* data, ::cgltf_material* mat, bool hasTangent)
{
    uint32_t mat_index = (uint32_t)(mat - data->materials);
    RETURN_IF_FOUND(m_Materials, mat_index);

    std::shared_ptr<MaterialResource> material = MakeSharedPtr<MaterialResource>();

    if (mat->name)
        material->_name = mat->name;

    if (mat->has_pbr_metallic_roughness)
        this->LoadMetallicRoughness(data, &mat->pbr_metallic_roughness, *material.get());

    // 法线贴图
    if (mat->normal_texture.texture)
    {
        if (hasTangent)
        {
            LoaderTexturePtr tex = this->LoadTexture(
                data, (uint32_t)(mat->normal_texture.texture - data->textures),
                &mat->normal_texture);
            if (tex && tex->imageIndex >= 0)
            {
                LoaderImagePtr img = this->LoadImage(data, (uint32_t)tex->imageIndex, false);
                if (img)
                {
                    material->_normalImage = img->bitmapData;
                    material->_normalScale = mat->normal_texture.scale;
                }
            }
        }
        else
            LOG_WARNING("Has normal texture, but lack tangent data in the mesh\n");
    }

    // 遮挡贴图
    if (mat->occlusion_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data, (uint32_t)(mat->occlusion_texture.texture - data->textures),
            &mat->occlusion_texture);
        if (tex && tex->imageIndex >= 0)
        {
            LoaderImagePtr img = this->LoadImage(data, (uint32_t)tex->imageIndex, false);
            if (img)
                material->_occlusionImage = img->bitmapData;
        }
    }

    // 自发光贴图
    if (mat->emissive_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data, (uint32_t)(mat->emissive_texture.texture - data->textures),
            &mat->emissive_texture);
        if (tex && tex->imageIndex >= 0)
        {
            LoaderImagePtr img = this->LoadImage(data, (uint32_t)tex->imageIndex, false);
            if (img)
                material->_emmissiveImage = img->bitmapData;
        }
    }

    material->_emissiveFactor = float3{mat->emissive_factor[0],
                                       mat->emissive_factor[1],
                                       mat->emissive_factor[2]};

    switch (mat->alpha_mode)
    {
    case cgltf_alpha_mode_mask:  material->_alphaMode = AlphaMode::Mask;  break;
    case cgltf_alpha_mode_blend: material->_alphaMode = AlphaMode::Blend; break;
    default:                     material->_alphaMode = AlphaMode::Opaque; break;
    }
    material->_alphaCutoff = mat->alpha_cutoff;
    material->_doubleSided = mat->double_sided;

    if (mat->has_clearcoat)
        this->LoadClearcoat(data, &mat->clearcoat, *material.get());
    if (mat->has_sheen)
        this->LoadSheen(data, &mat->sheen, *material.get());
    if (mat->has_ior)
        material->_IORFactor = mat->ior.ior;

    m_Materials[mat_index] = material;
    return material;
}

// ===== PBR 扩展 =====

bool glTF2_Loader::LoadMetallicRoughness(
    ::cgltf_data* data, ::cgltf_pbr_metallic_roughness* pbr, MaterialResource& pMaterial)
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
        if (tex && tex->imageIndex >= 0)
        {
            LoaderImagePtr img = this->LoadImage(data, (uint32_t)tex->imageIndex, true);
            if (img)
                pMaterial._albedoImage = img->bitmapData;
        }
    }

    if (pbr->metallic_roughness_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(pbr->metallic_roughness_texture.texture - data->textures),
            &pbr->metallic_roughness_texture);
        if (tex && tex->imageIndex >= 0)
        {
            LoaderImagePtr img = this->LoadImage(data, (uint32_t)tex->imageIndex, false);
            if (img)
                pMaterial._metallicRoughnessImage = img->bitmapData;
        }
    }

    return true;
}

bool glTF2_Loader::LoadClearcoat(::cgltf_data* data,
                                  ::cgltf_clearcoat* cc, MaterialResource& pMaterial)
{
    pMaterial._clearcoatFactor = cc->clearcoat_factor;
    pMaterial._clearcoatRoughnessFactor = cc->clearcoat_roughness_factor;

    if (cc->clearcoat_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(cc->clearcoat_texture.texture - data->textures),
            &cc->clearcoat_texture);
        if (tex && tex->imageIndex >= 0)
        {
            LoaderImagePtr img = this->LoadImage(data, (uint32_t)tex->imageIndex, false);
            if (img)
                pMaterial._clearcoatImage = img->bitmapData;
        }
    }

    if (cc->clearcoat_roughness_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(cc->clearcoat_roughness_texture.texture - data->textures),
            &cc->clearcoat_roughness_texture);
        if (tex && tex->imageIndex >= 0)
        {
            LoaderImagePtr img = this->LoadImage(data, (uint32_t)tex->imageIndex, false);
            if (img)
                pMaterial._clearcoatRoughnessImage = img->bitmapData;
        }
    }

    return true;
}

bool glTF2_Loader::LoadSheen(::cgltf_data* data,
                              ::cgltf_sheen* sheen, MaterialResource& pMaterial)
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
        if (tex && tex->imageIndex >= 0)
        {
            LoaderImagePtr img = this->LoadImage(data, (uint32_t)tex->imageIndex, false);
            if (img)
                pMaterial._sheenColorImage = img->bitmapData;
        }
    }

    if (sheen->sheen_roughness_texture.texture)
    {
        LoaderTexturePtr tex = this->LoadTexture(
            data,
            (uint32_t)(sheen->sheen_roughness_texture.texture - data->textures),
            &sheen->sheen_roughness_texture);
        if (tex && tex->imageIndex >= 0)
        {
            LoaderImagePtr img = this->LoadImage(data, (uint32_t)tex->imageIndex, false);
            if (img)
                pMaterial._sheenRoughnessImage = img->bitmapData;
        }
    }

    return true;
}

// ===== 光源 =====

LightComponentPtr glTF2_Loader::LoadLight(::cgltf_data* data, uint32_t light_index)
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

// ===== 纹理 / 图片 =====

LoaderTexturePtr glTF2_Loader::LoadTexture(::cgltf_data* data, uint32_t tex_index,
                                            ::cgltf_texture_view* tex_view)
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

        if (tex_view)
        {
            tex->texCoord = tex_view->texcoord;
            tex->scale = tex_view->scale;
        }

        m_Textures[tex_index] = tex;
    }
    return tex;
}

LoaderImagePtr glTF2_Loader::LoadImage(::cgltf_data* data, uint32_t image_index,
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
        for (auto& it : m_Images)
        {
            if (it.second && it.second->mimeType == img->mimeType)
            {
                cgltf_image* prevImg = (it.first < data->images_count)
                    ? &data->images[it.first] : nullptr;
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

// ===== 骨骼 / 动画 =====

LoaderSkinPtr glTF2_Loader::LoadSkin(::cgltf_data* data, uint32_t skin_index)
{
    RETURN_IF_FOUND(m_Skins, skin_index);
    LoaderSkinPtr skin = nullptr;

    if (skin_index >= data->skins_count)
        return skin;

    cgltf_skin* cskin = &data->skins[skin_index];
    skin = MakeSharedPtr<LoaderSkin>();

    if (cskin->inverse_bind_matrices)
    {
        ::cgltf_accessor* accessor = cskin->inverse_bind_matrices;
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

    for (size_t i = 0; i < cskin->joints_count; i++)
    {
        uint32_t joint_index = (uint32_t)(cskin->joints[i] - data->nodes);
        SceneComponentPtr sc = this->LoadNode(data, joint_index);
        if (sc)
            skin->joints.push_back(sc);
        else
            LOG_ERROR("skin joints index is not correct");
    }

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

void glTF2_Loader::LoadAnimation(::cgltf_data* data,
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

            animTrack->SetInterpolationType(
                gltf::ConvertToInterpolationType((int)sampler->interpolation));

            ::cgltf_accessor* input_acc = sampler->input;
            ::cgltf_accessor* output_acc = sampler->output;
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

// ===== Morph Target =====

void glTF2_Loader::ConverterToMorphStreamUnitFromTargets(
    RHIMeshPtr& mesh, ::cgltf_primitive* primitive, AABBox& targets_box)
{
    size_t targetSize = primitive->targets_count;
    if (targetSize == 0)
        return;

    cgltf_morph_target* firstTarget = &primitive->targets[0];
    ::cgltf_accessor* firstAcc = nullptr;
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
    int32_t allTargetDataSize = (int32_t)targetSize * targetCount * 4;

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
                ::cgltf_accessor* accessor = target->attributes[a].data;
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

// ===== 访问器（公共接口） =====

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

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
