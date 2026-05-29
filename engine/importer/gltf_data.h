#pragma once

#include "seek_engine.h"
#include "resource/resource_mgr.h"
#include "components/mesh_component.h"
#include "components/light_component.h"
#include "components/animation_component.h"
#include "math/matrix.h"
#include "math/aabbox.h"
#include "rhi/base/rhi_mesh.h"

SEEK_NAMESPACE_BEGIN

struct BufferResource;

struct GltfBuffer
{
    std::shared_ptr<BufferResource> resource;
    uint8_t* data = nullptr;
    size_t   size = 0;
};

struct GltfImage
{
    std::string    mimeType;
    std::string    uriPath;
    BitmapBufferPtr bitmapData;
};

struct GltfSampler
{
    int32_t magFilter = -1;
    int32_t minFilter = -1;
    int32_t wrapS = -1;
    int32_t wrapT = -1;
};

struct GltfTexture
{
    uint32_t imageIndex   = UINT32_MAX;
    uint32_t samplerIndex = UINT32_MAX;
    int32_t  texCoord     = 0;
    float    scale        = 1.0f;
};

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

struct GltfMesh
{
    std::string                   name;
    std::vector<GltfPrimitive>    primitives;
    std::vector<float>            weights;
    std::vector<std::string>      targetNames;
};

struct GltfSkin
{
    std::vector<Matrix4>    inverseBindMatrices;
    std::vector<uint32_t>   jointNodeIndices;
    uint32_t                skeletonNodeIndex = UINT32_MAX;
};

struct GltfNode
{
    std::string name;

    bool     hasMatrix      = false;
    Matrix4  matrix         = Matrix4::Identity();
    bool     hasTranslation = false;
    float3   translation    = float3(0.0f);
    bool     hasRotation    = false;
    Quaternion rotation     = Quaternion::Identity();
    bool     hasScale       = false;
    float3   scale          = float3(1.0f);

    uint32_t meshIndex  = UINT32_MAX;
    uint32_t skinIndex  = UINT32_MAX;
    uint32_t lightIndex = UINT32_MAX;

    std::vector<uint32_t> childIndices;
};

struct GltfAnimationChannel
{
    uint32_t            targetNodeIndex;
    TransformType       transformType;
    bool                isMorphTarget = false;
    InterpolationType   interpolation;
    std::vector<float>  inputTimes;
    std::vector<float>  outputData;
    uint32_t            outputStride;
    uint32_t            outputCount;
};

struct GltfAnimation
{
    std::string                        name;
    std::vector<GltfAnimationChannel>  channels;
};

struct GltfLight
{
    std::string name;
    LightType   type;
    Color       color;
    float       intensity;
};

struct GltfScene
{
    std::string                     name;
    std::vector<uint32_t>           rootNodeIndices;
};

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
    std::vector<GltfBuffer>     buffers;
};

SEEK_NAMESPACE_END
