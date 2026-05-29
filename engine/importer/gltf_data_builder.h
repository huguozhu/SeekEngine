#pragma once

#include "importer/gltf_data.h"

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
    SResult Build(const std::string& filePath, GltfData& outData);

private:
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

    GltfPrimitive ExtractPrimitive(::cgltf_data* data, ::cgltf_primitive* prim);
    GltfImage     ExtractImage(::cgltf_data* data, ::cgltf_image* img, bool sRGB);
    GltfAnimationChannel ExtractChannel(::cgltf_data* data,
                                        ::cgltf_animation_channel* ch,
                                        ::cgltf_animation_sampler* sampler);

    static const uint8_t* AccessorData(const ::cgltf_accessor* accessor);
    static uint32_t       AccessorStride(const ::cgltf_accessor* accessor);

    std::string m_filePath;
};

SEEK_NAMESPACE_END
