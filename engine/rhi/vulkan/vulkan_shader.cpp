#include "kernel/context.h"
#include "rhi/vulkan/vulkan_shader.h"
#include "rhi/vulkan/vulkan_context.h"
#include "rhi/vulkan/vulkan_translate.h"
#include "utils/log.h"
#include "utils/dll_loader.h"

#include <dxc/dxcapi.h>

#define SEEK_MACRO_FILE_UID 79     //

SEEK_NAMESPACE_BEGIN

// DXC DLL and function pointer
static DllLoader s_dxcompiler("dxcompiler.dll");
static DxcCreateInstanceProc s_DxcCreateInstance = nullptr;

// Get DXC shader model string for Vulkan/SPIR-V
static const wchar_t* GetDxcShaderModel(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:   return L"vs_6_0";
    case ShaderType::Pixel:    return L"ps_6_0";
    case ShaderType::Geometry: return L"gs_6_0";
    case ShaderType::Hull:     return L"hs_6_0";
    case ShaderType::Domain:   return L"ds_6_0";
    case ShaderType::Compute:  return L"cs_6_0";
    default:                   return L"vs_6_0";
    }
}

// Helper: convert narrow string to wide
static std::wstring ToWide(const std::string& s)
{
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring result(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), result.data(), len);
    return result;
}

VkShader::VkShader(Context* context, ShaderType type, std::string const& name,
    std::string const& entry_func_name, std::string const& code)
    : RHIShader(context, type, name, entry_func_name, code)
{
    m_vkStage = VkTranslate::ShaderTypeToVkStage(type);
}

VkShader::~VkShader()
{
    VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
    if (m_vkShaderModule != VK_NULL_HANDLE && vkCtx->GetVkDevice() != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(vkCtx->GetVkDevice(), m_vkShaderModule, nullptr);
        m_vkShaderModule = VK_NULL_HANDLE;
    }
}

SResult VkShader::OnCompile()
{
    VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
    VkDevice device = vkCtx->GetVkDevice();

    if (m_szCode.empty())
    {
        LOG_ERROR("Shader code is empty: %s", m_szName.c_str());
        return ERR_INVALID_ARG;
    }

    // Precompiled code: treat as raw SPIR-V binary
    if (m_bCodePrecompiled)
    {
        if (m_szCode.size() % 4 != 0)
        {
            LOG_ERROR("Precompiled shader code size not multiple of 4: %s", m_szName.c_str());
            return ERR_INVALID_SHADER;
        }
        m_SPIRV.resize(m_szCode.size() / 4);
        memcpy(m_SPIRV.data(), m_szCode.data(), m_szCode.size());
    }
    else
    {
        // Compile HLSL 鈫?SPIR-V using DXC
        if (!s_dxcompiler.IsLoaded())
        {
            if (!s_dxcompiler.Load())
            {
                LOG_ERROR("Failed to load dxcompiler.dll for shader: %s", m_szName.c_str());
                return ERR_SYSTEM_ERROR;
            }
            s_DxcCreateInstance = (DxcCreateInstanceProc)s_dxcompiler.FindSymbol("DxcCreateInstance");
            if (!s_DxcCreateInstance)
            {
                LOG_ERROR("Failed to find DxcCreateInstance in dxcompiler.dll");
                return ERR_SYSTEM_ERROR;
            }
        }

        // Create DXC utils
        IDxcUtils* pUtils = nullptr;
        HRESULT hr = s_DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
        if (FAILED(hr))
        {
            LOG_ERROR("DxcCreateInstance(Utils) failed: 0x%X for %s", hr, m_szName.c_str());
            return ERR_SYSTEM_ERROR;
        }

        // Create blob from HLSL source
        IDxcBlobEncoding* pSource = nullptr;
        pUtils->CreateBlob(m_szCode.data(), (UINT32)m_szCode.size(), CP_UTF8, &pSource);

        // Create compiler
        IDxcCompiler3* pCompiler = nullptr;
        hr = s_DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
        if (FAILED(hr))
        {
            LOG_ERROR("DxcCreateInstance(Compiler) failed: 0x%X for %s", hr, m_szName.c_str());
            pUtils->Release();
            return ERR_SYSTEM_ERROR;
        }

        // Build compiler arguments
        std::wstring entry = ToWide(m_szEntryFuncName);
        const wchar_t* target = GetDxcShaderModel(m_eShaderType);
        std::vector<const wchar_t*> args;
        args.push_back(L"-spirv");
        args.push_back(L"-fspv-target-env=vulkan1.3");
        args.push_back(L"-HV");  // Use HLSL 202x compiler
        args.push_back(L"2021");
        args.push_back(L"-E");
        args.push_back(entry.c_str());
        args.push_back(L"-T");
        args.push_back(target);

        // Add macros
        std::vector<std::wstring> macroStr;
        for (auto& m : m_vMacros)
        {
            std::string def = m.first + "=" + m.second;
            macroStr.push_back(ToWide(def));
            args.push_back(L"-D");
            args.push_back(macroStr.back().c_str());
        }
        // Add predefines
        for (auto& p : m_vPredefines)
        {
            std::string def = p.name + "=" + p.value;
            macroStr.push_back(ToWide(def));
            args.push_back(L"-D");
            args.push_back(macroStr.back().c_str());
        }

        if (m_pContext->EnableDebug())
        {
            args.push_back(L"-Zi");  // Debug info
            args.push_back(L"-O0");  // No optimization
        }

        DxcBuffer sourceBuffer = {};
        sourceBuffer.Ptr = pSource->GetBufferPointer();
        sourceBuffer.Size = pSource->GetBufferSize();
        sourceBuffer.Encoding = DXC_CP_UTF8;

        IDxcResult* pResult = nullptr;
        hr = pCompiler->Compile(&sourceBuffer, args.data(), (UINT32)args.size(),
            nullptr, IID_PPV_ARGS(&pResult));

        // Check result
        HRESULT compileStatus;
        pResult->GetStatus(&compileStatus);

        if (FAILED(compileStatus))
        {
            IDxcBlobUtf8* pErrors = nullptr;
            pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
            if (pErrors && pErrors->GetStringLength() > 0) {
                LOG_ERROR("DXC compile error for %s:\n%s", m_szName.c_str(), pErrors->GetStringPointer());
            } else {
                LOG_ERROR("DXC compile failed for %s (hr=0x%X)", m_szName.c_str(), compileStatus);
            }

            if (pErrors) pErrors->Release();
            pResult->Release();
            pCompiler->Release();
            pUtils->Release();
            return ERR_INVALID_SHADER;
        }

        // Get compiled SPIR-V object
        IDxcBlob* pSpirv = nullptr;
        pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pSpirv), nullptr);
        if (!pSpirv || pSpirv->GetBufferSize() == 0)
        {
            LOG_ERROR("DXC produced empty output for %s", m_szName.c_str());
            pResult->Release();
            pCompiler->Release();
            pUtils->Release();
            return ERR_INVALID_SHADER;
        }

        // Copy SPIR-V to member
        size_t spirvSize = pSpirv->GetBufferSize();
        m_SPIRV.resize(spirvSize / 4);
        memcpy(m_SPIRV.data(), pSpirv->GetBufferPointer(), spirvSize);

        // Release DXC objects
        pSpirv->Release();
        pResult->Release();
        pCompiler->Release();
        pUtils->Release();

        LOG_INFO("Shader compiled to SPIR-V: %s (%zu bytes)", m_szName.c_str(), spirvSize);
    }

    // Create VkShaderModule
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = m_SPIRV.size() * sizeof(uint32_t);
    createInfo.pCode = m_SPIRV.data();

    if (vkCreateShaderModule(device, &createInfo, nullptr, &m_vkShaderModule) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create VkShaderModule for %s", m_szName.c_str());
        return ERR_SYSTEM_ERROR;
    }

    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     //



