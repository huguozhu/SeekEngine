#pragma once

#include "seek_engine.h"
#include "rapidjson/document.h"
#include "resource/resource_mgr.h"
#include "importer/gltf2.h"
#include "loader.h"
#include "components/animation_component.h"
SEEK_NAMESPACE_BEGIN

struct BufferResource;

struct LoaderBuffer
{
    int32_t byteLength = GLTF_INVALID_INTEGER;
    uint8_t* binaryData = nullptr;
    std::string uriPath; // absolute path, null when uri is not a path
    
    std::shared_ptr<BufferResource> backendBuffer;
};
using LoaderBufferPtr = std::shared_ptr<LoaderBuffer>;

struct LoaderBufferView
{
    int32_t bufferIndex = GLTF_INVALID_ARRAY_INDEX;
    int32_t byteOffset = GLTF_DEFAULT_BYTE_OFFSET;
    int32_t byteLength = GLTF_INVALID_INTEGER;
    int32_t byteStride = GLTF_INVALID_INTEGER;
    int32_t target = GLTF_INVALID_ENUM;
    
    uint8_t* binaryData = nullptr;
    std::shared_ptr<BufferResource> backendBuffer;
};
using LoaderBufferViewPtr = std::shared_ptr<LoaderBufferView>;

//template <typename T>
struct LoaderAccessor
{
    int32_t bufferViewIndex = GLTF_INVALID_ARRAY_INDEX;
    int32_t byteOffset = GLTF_DEFAULT_BYTE_OFFSET;
    int32_t componentType = GLTF_INVALID_ENUM;
    bool normalized = GLTF_DEFAULT_NORMALIZED;
    int32_t count = GLTF_INVALID_INTEGER;
    int32_t elementType= GLTF_INVALID_ENUM;
    std::vector<float> minValues; // FIXME: template type should be componentType, not float
    std::vector<float> maxValues;
    // [unsupport] sparse
    std::string name;
};
using LoaderAccessorPtr = std::shared_ptr<LoaderAccessor>;

struct LoaderTexture
{
    int32_t imageIndex = GLTF_INVALID_ARRAY_INDEX;
    int32_t samplerIndex = GLTF_INVALID_ARRAY_INDEX;
    int32_t texCoord = GLTF_DEFAULT_TEXCOORD_INDEX;
    float scale = 1.0f;
};
using LoaderTexturePtr = std::shared_ptr<LoaderTexture>;

struct LoaderImage
{
    std::string mimeType;
    int32_t bufferViewIndex = GLTF_INVALID_ARRAY_INDEX;
    std::string uriPath; // absolute path, null when uri is not a path
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

struct LoaderAnimSampler
{
    float* inputData;
    int32_t inputDataCount;
    InterpolationType ipType;
    char* outputData;;
    int32_t outputDataStride;
    int32_t outputDataCount;
};
using LoaderAnimSamplerPtr = std::shared_ptr<LoaderAnimSampler>;

class glTF2_Loader : public Loader
{
public:
    glTF2_Loader(Context* ctx);
    virtual ~glTF2_Loader();

    virtual SResult LoadSceneFromFile(std::string const& filePath, SceneComponentPtr& scene, std::vector<AnimationComponentPtr>& anim) override;
    virtual SResult TickSceneFromJson(std::string const& jsonStr) override { return S_Success; }
    virtual rapidjson::Document& GetModelDoc() override;
    virtual std::map<uint32_t, MeshComponentPtr>& GetMeshComponentMap() override;
    virtual std::map<uint32_t, SceneComponentPtr>& GetSceneComponentMap() override;
    virtual std::map<std::string, SceneComponentPtr>& GetSceneComponentMapByName() override;

private:
    void LoadExtensionsUsed();
    SceneComponentPtr LoadDefaultScene();
    SceneComponentPtr LoadScene(uint32_t index);
    SceneComponentPtr LoadNode(uint32_t index);
    MeshComponentPtr LoadMesh(uint32_t mesh_index, bool has_skin, uint32_t skin_index);
    LightComponentPtr LoadLight(uint32_t light_index);
    RHIMeshPtr LoadPrimitive(rapidjson::Value& v_primitive);
    std::shared_ptr<MaterialResource> LoadMaterial(uint32_t material_index, bool hasTangent);
    LoaderTexturePtr LoadTexture(uint32_t tex_index, rapidjson::Value& v_mat);
    LoaderImagePtr LoadImage(uint32_t image_index, bool bColor);
    LoaderSkinPtr LoadSkin(uint32_t skin_index);
    void LoadAnimation(std::vector<AnimationComponentPtr>& animations);
    LoaderAnimSamplerPtr LoadAnimationSampler(uint32_t sampler_index, rapidjson::Value& v_anim);

    LoaderAccessorPtr LoadAccessor(uint32_t accessor_index);
    LoaderBufferPtr& LoadBuffer(uint32_t buffer_index);
    LoaderBufferViewPtr LoadBufferView(uint32_t buffer_view_index);

    void ConverterToMorphStreamUnitFromTargets(RHIMeshPtr& m_pMesh, rapidjson::Value& v, AABBox& targets_box);

    bool LoadMetallicRoughness(rapidjson::Value& v, MaterialResource& pMaterial);
    bool LoadClearcoat(rapidjson::Value& v, MaterialResource& pMaterial);
    bool LoadSheen(rapidjson::Value& v, MaterialResource& pMaterial);

    std::shared_ptr<FileResource>& LoadUriFile(const std::string& uriFilePath);
    inline std::string GetUriAbsolutePath(const std::string& uriPath);

private:
    Context* m_pContext;
    rapidjson::Document m_Doc;
    std::vector<std::string> m_ExtensionsUsed;

    std::map<uint32_t, SceneComponentPtr> m_Nodes;
    std::map<std::string, SceneComponentPtr> m_NodesMapByName;
    std::map<uint32_t, MeshComponentPtr> m_Meshes;
    std::map<uint32_t, LightComponentPtr> m_Lights;
    std::map<uint32_t, std::shared_ptr<MaterialResource>> m_Materials;
    std::map<uint32_t, LoaderTexturePtr> m_Textures;
    std::map<uint32_t, LoaderImagePtr> m_Images;
    std::map<uint32_t, LoaderSkinPtr> m_Skins;
    //std::map<LoaderSkinPtr, SkeletalMeshComponentPtr> m_SkletalMeshes;
    std::map<uint32_t, LoaderAnimSamplerPtr> m_AnimSamplers;

    std::map<uint32_t, LoaderBufferPtr> m_Buffers;
    std::map<uint32_t, LoaderBufferViewPtr> m_BufferViews;
    std::map<uint32_t, LoaderAccessorPtr> m_Accessors;

    bool m_isGLBFormat;
    bool m_isLoadSucceed;

    std::shared_ptr<FileResource> m_fileRes;
    std::string m_filePath;
    std::shared_ptr<BufferResource> m_gltfResource;
    std::vector<std::shared_ptr<BufferResource>> m_BuffersResources;
    std::map<std::string, std::shared_ptr<FileResource>> m_uriFileResources;
    std::unique_ptr<gltf::GLBInfo> m_GLBInfo;
};

SEEK_NAMESPACE_END
