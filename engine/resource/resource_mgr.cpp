#include "resource_mgr.h"
#include "utils/log.h"
#include "kernel/context.h"
#include <fstream>
#include <atomic>
#if defined(SEEK_PLATFORM_IOS)
#include <CoreFoundation/CoreFoundation.h>
#endif
#include "shader/shader_content.h"

#define SEEK_MACRO_FILE_UID 84     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

static inline ShaderContent RHIQueryGeneratedShaderMetaContent(const std::string& shaderName)
{
    return QueryShaderMetaContent(shaderName);
}

static inline ShaderContent RHIQueryGeneratedShaderCodeContent(const std::string& shaderLanguage, const std::string& shaderName, bool isDebug)
{
    if (isDebug)
    {
#if defined(SEEK_SHADER_GENERATE_DEBUG)
        return RHIQueryShaderCodeContent_debug(shaderLanguage, shaderName);
#else
        LOG_ERROR("debug shader is not generated, please reconfigure with SEEK_SHADER_GENERATE_DEBUG enabled");
        return ShaderContent{ nullptr, 0 };
#endif
    }
    else
    {
        return QueryShaderCodeContent(shaderLanguage, shaderName);
    }
}

static inline ShaderContent RHIQueryGeneratedShaderReflectContent(const std::string& shaderLanguage, const std::string& shaderName, bool isDebug)
{
    if (isDebug)
    {
#if defined(SEEK_SHADER_GENERATE_DEBUG)
        return RHIQueryShaderReflectContent_debug(shaderLanguage, shaderName);
#else
        LOG_ERROR("debug shader is not generated, please reconfigure with SEEK_SHADER_GENERATE_DEBUG enabled");
        return ShaderContent{ nullptr, 0 };
#endif
    }
    else
    {
        return QueryShaderReflectContent(shaderLanguage, shaderName);
    }
}

ResourceID GenerateResourceID()
{
    static std::atomic<ResourceID> __global_resource_id{ 0 };
    return ++__global_resource_id;
}

std::string ResourceManager::ResourceRootPath()
{
    static std::string root_path;
    
    if (root_path.empty())
    {
#if defined(SEEK_PLATFORM_IOS)
        CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
        char resourcePath[PATH_MAX];
        if (CFURLGetFileSystemRepresentation(resourceURL, true, (UInt8 *)resourcePath, PATH_MAX))
        {
            if (resourceURL != NULL)
            {
                CFRelease(resourceURL);
            }
        }
        root_path = std::string(resourcePath);
#endif
    }
    return root_path;
}

std::string ResourceManager::GetShaderMetaPath(const std::string& shaderName)
{
#if defined(SEEK_PLATFORM_IOS)
    return ResourceRootPath() + "/.meta/" + shaderName + SHADER_META_FILE_SUFFIX;
#else
    return std::string{ SEEK_GENERATED_META_DIR "/" } + shaderName + SHADER_META_FILE_SUFFIX;
#endif
}

std::string ResourceManager::GetShaderCodePath(const std::string& shaderName)
{
    RHIType rhi_type = m_pContext->GetRHIType();
    std::string debug_dir_suffix = "";
    switch (rhi_type)
    {
        case RHIType::D3D11:
            return std::string{ SEEK_GENERATED_SHADER_DIR } + "/hlsl" + debug_dir_suffix + "/" + shaderName + ".hlsl";
        case RHIType::Metal:
#if defined(SEEK_PLATFORM_MAC)
            return std::string{ SEEK_GENERATED_SHADER_DIR } + "/msl_macos" + debug_dir_suffix + "/" + shaderName + ".msl";
#elif defined(SEEK_PLATFORM_IOS)
            return ResourceRootPath() + "/msl_ios" + debug_dir_suffix + "/" + shaderName + ".msl";
#else
            LOG_ERROR("please use Metal on Apple's OS");
            return "";
#endif
        case RHIType::GLES:
            return std::string{ SEEK_GENERATED_SHADER_DIR } + "/essl" + debug_dir_suffix + "/" + shaderName + ".glsl";
        default:
            LOG_ERROR("unsupported RHIType %d", rhi_type);
            return "";
    }
}

std::string ResourceManager::GetShaderReflectPath(const std::string& shaderName)
{
    RHIType render_api = m_pContext->GetRHIType();
    std::string debug_dir_suffix = "";
    //if (m_pContext->IsDebug())
    //    debug_dir_suffix = SHADER_DEBUG_DIR_SUFFIX;
    switch (render_api)
    {
    case RHIType::D3D11:
        return std::string{ SEEK_GENERATED_SHADER_DIR } + "/hlsl" + debug_dir_suffix + "/" + shaderName + SHADER_REFLECT_FILE_SUFFIX;
    case RHIType::Metal:
#if defined(SEEK_PLATFORM_MAC)
        return std::string{ SEEK_GENERATED_SHADER_DIR } + "/msl_macos" + debug_dir_suffix + "/" + shaderName + SHADER_REFLECT_FILE_SUFFIX;
#elif defined(SEEK_PLATFORM_IOS)
        return std::string{ SEEK_GENERATED_SHADER_DIR } + "/msl_ios" + debug_dir_suffix + "/" + shaderName + SHADER_REFLECT_FILE_SUFFIX;
#else
        LOG_ERROR("please use Metal on Apple's OS");
        return "";
#endif
    case RHIType::GLES:
        return std::string{ SEEK_GENERATED_SHADER_DIR } + "/essl" + debug_dir_suffix + "/" + shaderName + SHADER_REFLECT_FILE_SUFFIX;
    default:
        LOG_ERROR("unsupported RHIType %d", render_api);
        return "";
    }
}

const std::string& ResourceManager::GetShaderLanguageStr()
{
    static const std::string d3d11_shaderlanguage       = "hlsl";
    static const std::string metal_mac_shaderlanguage   = "msl_macos";
    static const std::string metal_ios_shaderlanguage   = "msl_ios";
    static const std::string opengles_shaderlanguage    = "essl";
    static const std::string empty_str = "";
    RHIType render_api = m_pContext->GetRHIType();
    switch (render_api)
    {
        case RHIType::D3D11:
            return d3d11_shaderlanguage;
        case RHIType::Metal:
    #if defined(SEEK_PLATFORM_MAC)
            return metal_mac_shaderlanguage;
    #elif defined(SEEK_PLATFORM_IOS)
            return metal_ios_shaderlanguage;
    #else
            return empty_str;
    #endif
        case RHIType::GLES:
            return opengles_shaderlanguage;
        default:
            return empty_str;
    }
}

ResourceManager::ResourceManager(Context* context)
    : m_pContext(context)
{ }

FileResourcePtr ResourceManager::LoadFileResource(const std::string& filePath)
{
    auto resIt = m_fileRes.find(filePath);
    if (resIt == m_fileRes.end())
    {
        FileResourcePtr _res = MakeSharedPtr<FileResource>(filePath);
        if (!_res->IsAvailable())
        {
            LOG_ERROR("ResourceManager, load file resource %s fail", filePath.c_str());
            return nullptr;
        }

        resIt = m_fileRes.insert({ filePath, _res }).first;
    }
    return resIt->second;
}

MetaShaderResourcePtr ResourceManager::LoadMetaShaderResource(const std::string& shaderName)
{
    auto resIt = m_metaShaderRes.find(shaderName);
    if (resIt == m_metaShaderRes.end())
    {
        MetaShaderResourcePtr _res = MakeSharedPtr<MetaShaderResource>(this);
        SResult ret = _res->Load(shaderName);
        if (SEEK_CHECKFAILED(ret))
        {
            LOG_ERROR("ResourceManager, load meta shader resource %s fail", shaderName.c_str());
            return nullptr;
        }

        resIt = m_metaShaderRes.insert({ shaderName, _res }).first;
    }

    return resIt->second;
}

ShaderResourcePtr ResourceManager::LoadShaderResource(const std::string& shaderName)
{
    auto resIt = m_shaderRes.find(shaderName);
    if (resIt == m_shaderRes.end())
    {
        ShaderResourcePtr _res = MakeSharedPtr<ShaderResource>(this);
        SResult ret = _res->Load(shaderName);
        if (SEEK_CHECKFAILED(ret))
        {
            LOG_ERROR("ResourceManager, load shader resource %s fail", shaderName.c_str());
            return nullptr;
        }

        resIt = m_shaderRes.insert({ shaderName, _res }).first;
    }

    return resIt->second;
}

SResult MetaShaderResource::Load(const std::string& metaShaderName)
{
    const void* metaData = nullptr;
    size_t metaSize = 0;
//#if !defined(SEEK_SHADER_GENERATE_HEADER_FILE)
//    std::string metaShaderFilePath = resourceMgr->GetShaderMetaPath(metaShaderName);
//    if (metaShaderFilePath.empty())
//    {
//        LOG_ERROR("get shader %s meta file path fail", metaShaderName.c_str());
//        return ERR_FILE_NOT_FOUND;
//    }
//    FileResource _fileRes(metaShaderFilePath);
//    if (!_fileRes.IsAvailable())
//        return ERR_FILE_NOT_FOUND;
//    metaData = _fileRes._data;
//    metaSize = _fileRes._size;
//#else
    auto content = RHIQueryGeneratedShaderMetaContent(metaShaderName);
    if (!content.first)
    {
        LOG_ERROR("get shader %s meta content fail", metaShaderName.c_str());
        return ERR_FILE_NOT_FOUND;
    }
    metaData = content.first;
    metaSize = content.second;
//#endif

    _name = metaShaderName;
    shadercompiler::MetaJsonReader reader(reinterpret_cast<const char*>(metaData), metaSize);
    shadercompiler::MetaInfo _metaInfo;
    if (0 == reader.Read(_metaInfo))
    {
        metaInfo = _metaInfo;
        SetAvailable(true);
        return S_Success;
    }
    else
    {
        return ERR_INVALID_DATA;
    }
}

SResult ShaderResource::Load(const std::string& shaderName)
{
    const void* codeData;
    size_t codeSize;
    const void* reflectData;
    size_t reflectSize;
    auto codeContent = RHIQueryGeneratedShaderCodeContent(resourceMgr->GetShaderLanguageStr(), shaderName, false);
    if (!codeContent.first)
    {
        LOG_ERROR("get shader %s code content fail", shaderName.c_str());
        return ERR_FILE_NOT_FOUND;
    }

    auto reflectContent = RHIQueryGeneratedShaderReflectContent(resourceMgr->GetShaderLanguageStr(), shaderName, false);
    if (!reflectContent.first)
    {
        LOG_ERROR("get shader %s code content fail", shaderName.c_str());
        return ERR_FILE_NOT_FOUND;
    }

    codeData = codeContent.first;
    codeSize = codeContent.second;
    reflectData = reflectContent.first;
    reflectSize = reflectContent.second;

    _name = shaderName;
    shadercompiler::ReflectJsonReader reader(reinterpret_cast<const char*>(reflectData), reflectSize);
    if (0 == reader.Read(reflectInfo))
    {
        sourceCode = codeData;
        sourceCodeSize = codeSize;
        SetAvailable(true);
        return S_Success;
    }
    else
    {
        return ERR_INVALID_DATA;
    }
}


/******************************************************************************
* ResourceManager
******************************************************************************/


SEEK_NAMESPACE_END


#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
