#pragma once

#include <cstdint>
#include "seek_engine.h"
#include "resource/resource_mgr.h"
#include "importer/loader.h"
#include "components/animation_component.h"

struct cgltf_data;
struct cgltf_primitive;
struct cgltf_material;
struct cgltf_texture_view;
struct cgltf_accessor;
struct cgltf_pbr_metallic_roughness;
struct cgltf_clearcoat;
struct cgltf_sheen;

#undef LoadImage

SEEK_NAMESPACE_BEGIN

struct BufferResource;

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
    std::string uriPath;
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
    SceneComponentPtr LoadDefaultScene(::cgltf_data* data);
    SceneComponentPtr LoadScene(::cgltf_data* data, uint32_t index);
    SceneComponentPtr LoadNode(::cgltf_data* data, uint32_t index);
    MeshComponentPtr  LoadMesh(::cgltf_data* data, uint32_t mesh_index, bool has_skin, uint32_t skin_index);
    LightComponentPtr LoadLight(::cgltf_data* data, uint32_t light_index);
    RHIMeshPtr        LoadPrimitive(::cgltf_data* data, ::cgltf_primitive* primitive);
    std::shared_ptr<MaterialResource> LoadMaterial(::cgltf_data* data, ::cgltf_material* material, bool hasTangent);
    LoaderTexturePtr  LoadTexture(::cgltf_data* data, uint32_t tex_index, ::cgltf_texture_view* tex_view);
    LoaderImagePtr    LoadImage(::cgltf_data* data, uint32_t image_index, bool bColor);
    LoaderSkinPtr     LoadSkin(::cgltf_data* data, uint32_t skin_index);
    void              LoadAnimation(::cgltf_data* data, std::vector<AnimationComponentPtr>& animations);
    bool LoadMetallicRoughness(::cgltf_data* data, ::cgltf_pbr_metallic_roughness* pbr, MaterialResource& matRes);
    bool LoadClearcoat(::cgltf_data* data, ::cgltf_clearcoat* cc, MaterialResource& matRes);
    bool LoadSheen(::cgltf_data* data, ::cgltf_sheen* sheen, MaterialResource& matRes);
    void ConverterToMorphStreamUnitFromTargets(RHIMeshPtr& mesh, ::cgltf_primitive* primitive, AABBox& targets_box);
    static const uint8_t* AccessorData(const ::cgltf_accessor* accessor);
    static uint32_t AccessorStride(const ::cgltf_accessor* accessor);
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
    std::vector<std::shared_ptr<BufferResource>> m_BuffersResources;
};

SEEK_NAMESPACE_END
