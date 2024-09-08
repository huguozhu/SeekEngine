#pragma once

#include "kernel/kernel.h"
#include "utils/util.h"
#include "utils/file.h"
#include "math/vector.h"
#include "rhi/render_definition.h"
#include "rhi/format.h"
#include <functional>
// AGAIN!!! X11 define Bool as macro.
#undef Bool
#include "tools/ShaderCompiler/shader_helper.h"
using namespace shadercompiler;

SEEK_NAMESPACE_BEGIN

using ResourceID = uint64_t;
ResourceID GenerateResourceID();

#define SEEK_MACRO_FILE_UID 82     // this code is auto generated, don't touch it!!!

class ResourceManager;

struct IResource
{
    using Uninitializer = std::function<void(IResource*)>;

    IResource(ResourceManager* mgr = nullptr) 
        : resourceMgr(mgr) 
    { }
    virtual ~IResource()
    {
        if (_uninitializer)
            _uninitializer(this);
    }

    void RetainBackendResource(const std::shared_ptr<IResource>& backendRes)
    {
        _backendResources.push_back(backendRes);
    }

    void RetainBackendResource(std::shared_ptr<IResource>&& backendRes)
    {
        _backendResources.push_back(std::forward<std::shared_ptr<IResource>>(backendRes));
    }

    void ReleaseBackendResource()
    {
        _backendResources.clear();
    }

    virtual bool IsAvailable()
    {
        return _isAvailabled;
    }

    void SetAvailable(bool bAvailable)
    {
        _isAvailabled = bAvailable;
    }

    ResourceID _id = GenerateResourceID(); // OPT: generate only when resource is valid?
    std::string _name;
    //Initializer _initializer;
    Uninitializer _uninitializer;
    // current resource does not own memory, just a sub-resource of backendResources
    std::vector<std::shared_ptr<IResource>> _backendResources;
    bool _isAvailabled = false;
    ResourceManager* resourceMgr = nullptr;
};

struct FileResource : public IResource
{
    FileResource(char* pRes, int32_t resLength)
    {
        _backendBuffer.resize(resLength);
        memcpy(_backendBuffer.data(), pRes, resLength);
        _data = _backendBuffer.data();
        _size = _backendBuffer.size();
        SetAvailable(true);
    }

    FileResource(const std::string& filepath)
        : _filePath(filepath)
    {
        SResult ret = read_file_content(filepath.c_str(), "rb", _backendBuffer);
        if (!SEEK_CHECKFAILED(ret))
        {
            _data = _backendBuffer.data();
            _size = _backendBuffer.size();
            SetAvailable(true);
        }
    }
    
    std::vector<uint8_t> _backendBuffer;
    std::string _filePath;
    void* _data = nullptr;
    size_t _size = 0;
};
CLASS_DECLARE(FileResource);

struct BufferResource : public IResource
{
    void* _data = nullptr;
    size_t _size = 0;
};

struct VertexAttributeResource : public IResource
{
    std::vector<VertexStream> _vertexStreams;
    std::vector<std::shared_ptr<BufferResource>> _vertexBuffers;
};

struct VertexIndicesResource : public IResource
{
    IndexBufferType _indexBufferType = IndexBufferType::Unknown;
    uint32_t _indexCount = 0;
    void* _data = nullptr;
    size_t _size = 0;
};

struct MorphTargetResource : public IResource
{
    MorphInfo _morphInfo;
    void* _data = nullptr;
    size_t _size = 0;
    bool _refreshRenderBuffer = false;
};

struct MaterialResource : public IResource
{
    BitmapBufferPtr _albedoImage;
    BitmapBufferPtr _normalImage;
    BitmapBufferPtr _normalMaskImage;
    BitmapBufferPtr _occlusionImage;
    BitmapBufferPtr _metallicRoughnessImage;
    BitmapBufferPtr _emmissiveImage;
    BitmapBufferPtr _clearcoatImage;
    BitmapBufferPtr _clearcoatRoughnessImage;
    BitmapBufferPtr _sheenColorImage;
    BitmapBufferPtr _sheenRoughnessImage;
    
    float4      _albedoFactor               = float4{ 1.0, 1.0, 1.0, 1.0 };
    float       _normalScale                = 1.0;
    float4      _normalMaskWeights          = float4{ 0.0, 0.0, 0.0, 0.0 };
    float3      _emissiveFactor             = float3{ 0.0, 0.0, 0.0 };
    float       _metallicFactor             = 1.0;
    float       _roughnessFactor            = 1.0;
    float       _clearcoatFactor            = 1.0;
    float       _clearcoatRoughnessFactor   = 1.0;
    float3      _sheenColorFactor           = float3{ 1.0, 1.0, 1.0 };
    float       _sheenRoughnessFactor       = 1.0;
    
    AlphaMode   _alphaMode                  = AlphaMode::Opaque;
    float       _alphaCutoff                = 0.5;
    bool        _doubleSided                = false;
    float       _IORFactor;
};

struct MetaShaderResource : public IResource
{
    MetaShaderResource(ResourceManager* mgr)
        : IResource(mgr)
    { }

    SResult Load(const std::string& metaShaderName);

    shadercompiler::MetaInfo metaInfo;
};
CLASS_DECLARE(MetaShaderResource);

struct ShaderResource : public IResource
{
    ShaderResource(ResourceManager* mgr)
        : IResource(mgr)
    { }

    SResult Load(const std::string& shaderName);

    shadercompiler::ReflectInfo     reflectInfo;
    const void*                     sourceCode          = nullptr;
    size_t                          sourceCodeSize      = 0;
};
CLASS_DECLARE(ShaderResource);

#define SEEK_HANDLE(_name)                   \
    struct _name { uint16_t index = -1; };        \
    inline bool IsValid(_name _handle) { return -1 != _handle.index; }
#define SEEK_INVALID_HANDLE -1

SEEK_HANDLE(VertexStreamHandle)

#define SEEK_RESOURCE_FUNCTIONS(name)                   \
    private: std::vector<name>           m_v##name##s;   \
    public: name&       Alloc##name() {                  \
        m_v##name##s.emplace_back(name());              \
        name& v = m_v##name##s.back();                  \
        v.index = m_v##name##s.size() -1;               \
        return v;                                       \
    }

class ResourceManager
{
public:
    static std::string ResourceRootPath();
    
    ResourceManager(Context* context);
    ~ResourceManager() = default;

    FileResourcePtr             LoadFileResource        (const std::string& filePath);
    MetaShaderResourcePtr       LoadMetaShaderResource  (const std::string& shaderName);
    ShaderResourcePtr           LoadShaderResource      (const std::string& shaderName);

    std::string                 GetShaderMetaPath       (const std::string& shaderName);
    std::string                 GetShaderCodePath       (const std::string& shaderName);
    std::string                 GetShaderReflectPath    (const std::string& shaderName);
    
    const std::string&          GetShaderLanguageStr();

    SEEK_RESOURCE_FUNCTIONS(VertexStreamHandle)


private:
    Context*                                        m_pContext = nullptr;
    std::map<std::string, FileResourcePtr>          m_fileRes;
    std::map<std::string, MetaShaderResourcePtr>    m_metaShaderRes;
    std::map<std::string, ShaderResourcePtr>        m_shaderRes;


};
CLASS_DECLARE(ResourceManager)

SEEK_NAMESPACE_END


#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
