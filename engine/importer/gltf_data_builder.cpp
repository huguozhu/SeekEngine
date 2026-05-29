// cgltf type declarations must come before builder header
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

// cgltf single-header implementation
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

using namespace std;
using namespace seek_engine;

#define SEEK_MACRO_FILE_UID 82

SEEK_NAMESPACE_BEGIN

// Bring cgltf types into seek_engine namespace
using ::cgltf_data;
using ::cgltf_primitive;
using ::cgltf_image;
using ::cgltf_animation_channel;
using ::cgltf_animation_sampler;
using ::cgltf_accessor;

// === Helper methods ===

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

// === Main entry point ===

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

    // Extract in dependency order
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

    // GltfData is now self-contained, cgltf can be freed
    cgltf_free(data);
    return S_Success;
}

// === 1. Buffers ===

void GltfDataBuilder::ExtractBuffers(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->buffers_count; i++)
    {
        GltfBuffer buf;
        buf.data = static_cast<uint8_t*>(data->buffers[i].data);
        buf.size = data->buffers[i].size;
        // Take ownership of buffer data with shared_ptr
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

    if (img->buffer_view)
    {
        cgltf_buffer_view* bv = img->buffer_view;
        uint8_t* data_ptr = static_cast<uint8_t*>(bv->buffer->data) + bv->offset;
        outImg.bitmapData = ImageDecode(data_ptr, (uint32_t)bv->size, imageType);
    }
    else if (img->uri)
    {
        outImg.uriPath = img->uri;
        // Load external file
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
        s.minFilter = data->samplers[i].min_filter;
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
        out.textures.push_back(tex);
    }
}

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
                m.albedoTextureIndex = (uint32_t)(pbr->base_color_texture.texture - data->textures);
            if (pbr->metallic_roughness_texture.texture)
                m.metallicRoughnessTextureIndex = (uint32_t)(pbr->metallic_roughness_texture.texture - data->textures);
        }

        // Normal texture
        if (mat->normal_texture.texture)
        {
            m.normalTextureIndex = (uint32_t)(mat->normal_texture.texture - data->textures);
            m.normalScale = mat->normal_texture.scale;
        }

        // Occlusion texture
        if (mat->occlusion_texture.texture)
            m.occlusionTextureIndex = (uint32_t)(mat->occlusion_texture.texture - data->textures);

        // Emissive texture
        if (mat->emissive_texture.texture)
            m.emissiveTextureIndex = (uint32_t)(mat->emissive_texture.texture - data->textures);
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

        // KHR_materials_clearcoat
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

        // KHR_materials_sheen
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

// === 6. Meshes + Primitives ===

GltfPrimitive GltfDataBuilder::ExtractPrimitive(::cgltf_data* data,
                                                 ::cgltf_primitive* prim)
{
    GltfPrimitive gp;
    if (prim->attributes_count == 0)
        return gp;

    // Topology: cgltf_primitive_type is offset by 1 from gltf2.h enum
    gp.topology = gltf::ConvertToTopologyType((int)prim->type - 1);

    // Vertex attribute extraction
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

        // Joint index conversion: uint8/uint16 -> uint32
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

    // Assemble vertex streams and buffer references
    for (auto& pair : vertexStreams)
    {
        gp.vertexStreams.push_back(pair.second);
        auto blendIt = blendIndexBufferRes.find(pair.first);
        if (blendIt != blendIndexBufferRes.end())
            gp.vertexBuffers.push_back(blendIt->second);
        else
        {
            cgltf_buffer_view* bv = &data->buffer_views[pair.first];
            // 深拷贝顶点数据到 IR 自有内存，确保 cgltf_free 后仍然有效
            // （GPU 缓冲区在渲染时才惰性创建，此时 cgltf 已被释放）
            std::shared_ptr<uint8_t> ownedData{
                new uint8_t[bv->size], default_array_deleter<uint8_t>()
            };
            memcpy(ownedData.get(),
                   static_cast<uint8_t*>(bv->buffer->data) + bv->offset,
                   bv->size);
            auto bufRes = MakeSharedPtr<BufferResource>();
            bufRes->_uninitializer = [ownedData](IResource*) mutable {
                ownedData.reset();
            };
            bufRes->_data = ownedData.get();
            bufRes->_size = bv->size;
            gp.vertexBuffers.push_back(std::move(bufRes));
        }
    }

    // Index buffer
    if (prim->indices)
    {
        ::cgltf_accessor* indices_acc = prim->indices;
        const uint8_t* indexData = AccessorData(indices_acc);
        auto indicesResource = MakeSharedPtr<VertexIndicesResource>();
        indicesResource->_indexBufferType = gltf::ConvertToIndexBufferType(
            gltf::CgltfToGltfComponentType((int)indices_acc->component_type));
        indicesResource->_indexCount = (uint32_t)indices_acc->count;
        size_t indexSize = (size_t)indices_acc->count *
            (size_t)gltf::ComponentByteSize(
                gltf::CgltfToGltfComponentType((int)indices_acc->component_type));
        // 深拷贝索引数据到 IR 自有内存，确保 cgltf_free 后仍然有效
        std::shared_ptr<uint8_t> ownedIndexData{
            new uint8_t[indexSize], default_array_deleter<uint8_t>()
        };
        memcpy(ownedIndexData.get(), indexData, indexSize);
        indicesResource->_uninitializer = [ownedIndexData](IResource*) mutable {
            ownedIndexData.reset();
        };
        indicesResource->_data = ownedIndexData.get();
        indicesResource->_size = indexSize;
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
            AABBox targets_box;
            targets_box.Min(float3(0.0f));
            targets_box.Max(float3(0.0f));
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
                        if (acc->has_min && acc->has_max)
                        {
                            AABBox tmp_box(float3{acc->min[0], acc->min[1], acc->min[2]},
                                           float3{acc->max[0], acc->max[1], acc->max[2]});
                            targets_box |= tmp_box;
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
            gp.boundingBox = gp.boundingBox + targets_box;
            morphRes->_morphInfo.morph_target_names = std::vector<std::string>();
            morphRes->_morphInfo.morph_target_type = MorphTargetType::Position;
            morphRes->_morphInfo.morph_target_weights.resize(targetSize, 0.0f);
            morphRes->_data = (uint8_t*)allTargetData.get();
            morphRes->_size = allTargetDataSize * sizeof(float);
            gp.morphTargets = std::move(morphRes);
        }
    }

    // Material index
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

// === 7. Skins ===

void GltfDataBuilder::ExtractSkins(::cgltf_data* data, GltfData& out)
{
    for (size_t i = 0; i < data->skins_count; i++)
    {
        cgltf_skin* cskin = &data->skins[i];
        GltfSkin skin;

        // Inverse bind matrices
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

        // Joint node indices
        for (size_t j = 0; j < cskin->joints_count; j++)
        {
            uint32_t joint_idx = (uint32_t)(cskin->joints[j] - data->nodes);
            skin.jointNodeIndices.push_back(joint_idx);
        }

        // Skeleton root node index
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

        // Data references (indices)
        if (node->mesh)
            gn.meshIndex = (uint32_t)(node->mesh - data->meshes);
        if (node->skin)
            gn.skinIndex = (uint32_t)(node->skin - data->skins);

        // Child node indices
        for (size_t c = 0; c < node->children_count; c++)
        {
            gn.childIndices.push_back((uint32_t)(node->children[c] - data->nodes));
        }

        out.nodes.push_back(std::move(gn));
    }
}

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

    // Extract input/output accessor data
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

            // Copy input times
            channel.inputTimes.assign(inputData, inputData + inputCount);

            // Copy output data
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

#undef SEEK_MACRO_FILE_UID
