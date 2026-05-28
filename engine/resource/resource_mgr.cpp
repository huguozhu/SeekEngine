#include "resource_mgr.h"
#include "utils/log.h"
#include "kernel/context.h"
#include <fstream>
#include <atomic>
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
    return root_path;
}

std::string ResourceManager::GetShaderMetaPath(const std::string& shaderName)
{
    return std::string{ SEEK_GENERATED_META_DIR "/" } + shaderName + SHADER_META_FILE_SUFFIX;
}

std::string ResourceManager::GetShaderCodePath(const std::string& shaderName)
{
    std::string debug_dir_suffix = "";
    return std::string{ SEEK_GENERATED_SHADER_DIR } + "/hlsl" + debug_dir_suffix + "/" + shaderName + ".hlsl";
}

std::string ResourceManager::GetShaderReflectPath(const std::string& shaderName)
{
    std::string debug_dir_suffix = "";
    return std::string{ SEEK_GENERATED_SHADER_DIR } + "/hlsl" + debug_dir_suffix + "/" + shaderName + SHADER_REFLECT_FILE_SUFFIX;
}

const std::string& ResourceManager::GetShaderLanguageStr()
{
    static const std::string d3d11_shaderlanguage = "hlsl";
    return d3d11_shaderlanguage;
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
