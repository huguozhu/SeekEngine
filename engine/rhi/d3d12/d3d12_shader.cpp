#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_shader.h"
#include "rhi/d3d12/d3d12_context.h"
#include "kernel/context.h"
#include "math/hash.h"
#include "utils/log.h"
#include "utils/error.h"

#pragma comment(lib, "dxcompiler.lib")

SEEK_NAMESPACE_BEGIN

D3D12Shader::~D3D12Shader()
{
    m_pShaderBlob.Reset();
}

SResult D3D12Shader::Active()
{
    if (!m_bCompileReady)
    {
        SEEK_RETIF_FAIL(this->OnCompile());
        m_bCompileReady = true;
    }
    return S_Success;
}

void D3D12Shader::Deactive()
{
}

const char* D3D12Shader::GetCompileTarget(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:   return "vs_6_0";
    case ShaderType::Pixel:    return "ps_6_0";
    case ShaderType::Geometry: return "gs_6_0";
    case ShaderType::Hull:     return "hs_6_0";
    case ShaderType::Domain:   return "ds_6_0";
    case ShaderType::Compute:  return "cs_6_0";
    default:                   return nullptr;
    }
}

SResult D3D12Shader::OnCompile()
{
    std::string macroStr;
    for (auto& m : m_vMacros)
        macroStr += m.first + ":" + m.second + ",";
    LOG_INFO("D3D12Shader::OnCompile() %s Macro(%s)", m_szEntryFuncName.c_str(), macroStr.c_str());

    IDxcCompilerPtr pCompiler;
    HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.GetAddressOf()));
    if (FAILED(hr))
    {
        LOG_ERROR("DxcCreateInstance(CLSID_DxcCompiler) failed, hr=%x", hr);
        return ERR_INVALID_SHADER;
    }

    IDxcLibraryPtr pLibrary;
    hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(pLibrary.GetAddressOf()));
    if (FAILED(hr))
    {
        LOG_ERROR("DxcCreateInstance(CLSID_DxcLibrary) failed, hr=%x", hr);
        return ERR_INVALID_SHADER;
    }

    IDxcBlobEncodingPtr pSource;
    hr = pLibrary->CreateBlobWithEncodingOnHeapCopy(
        m_szCode.data(), (UINT32)m_szCode.size(), CP_UTF8, pSource.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("CreateBlobWithEncodingOnHeapCopy failed, hr=%x", hr);
        return ERR_INVALID_SHADER;
    }

    std::vector<LPCWSTR> args;
    args.push_back(L"-E");
    std::wstring entryW(m_szEntryFuncName.begin(), m_szEntryFuncName.end());
    args.push_back(entryW.c_str());

    args.push_back(L"-T");
    const char* target = GetCompileTarget(m_eShaderType);
    if (!target) return ERR_INVALID_ARG;
    std::string targetStr(target);
    std::wstring targetW(targetStr.begin(), targetStr.end());
    args.push_back(targetW.c_str());

    if (m_pContext->EnableDebug())
    {
        args.push_back(L"-Zi");     // debug info
        args.push_back(L"-Od");     // disable optimizations
    }
    else
    {
        args.push_back(L"-O3");     // optimization level 3
    }

    // Add macros as -D defines
    std::vector<std::wstring> macroWStrs;
    std::vector<DxcDefine> defines;
    for (auto& m : m_vMacros)
    {
        DxcDefine def;
        macroWStrs.push_back(std::wstring(m.first.begin(), m.first.end()));
        def.Name = macroWStrs.back().c_str();
        macroWStrs.push_back(std::wstring(m.second.begin(), m.second.end()));
        def.Value = macroWStrs.back().c_str();
        defines.push_back(def);
    }
    for (auto& p : m_vPredefines)
    {
        DxcDefine def;
        macroWStrs.push_back(std::wstring(p.name.begin(), p.name.end()));
        def.Name = macroWStrs.back().c_str();
        macroWStrs.push_back(std::wstring(p.value.begin(), p.value.end()));
        def.Value = macroWStrs.back().c_str();
        defines.push_back(def);
    }

    // Add PLATFORM_HLSL for preprocessor branching
    {
        DxcDefine def;
        def.Name = L"PLATFORM_HLSL";
        def.Value = L"1";
        defines.push_back(def);
    }

    IDxcOperationResultPtr pResult;
    hr = pCompiler->Compile(
        pSource.Get(),
        nullptr,                    // source name
        entryW.c_str(),
        targetW.c_str(),
        args.data(), (UINT32)args.size(),
        defines.data(), (UINT32)defines.size(),
        nullptr,                    // include handler
        pResult.GetAddressOf());

    if (FAILED(hr))
    {
        LOG_ERROR("IDxcCompiler::Compile failed, hr=%x", hr);
        return ERR_INVALID_SHADER;
    }

    HRESULT compileStatus;
    pResult->GetStatus(&compileStatus);
    if (FAILED(compileStatus))
    {
        IDxcBlobEncodingPtr pErrors;
        pResult->GetErrorBuffer(pErrors.GetAddressOf());
        if (pErrors && pErrors->GetBufferSize() > 0)
        {
            LOG_ERROR("D3D12 Shader Compile Error:\r\n%s", (char*)pErrors->GetBufferPointer());
        }
        return ERR_INVALID_SHADER;
    }

    hr = pResult->GetResult(m_pShaderBlob.ReleaseAndGetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("GetResult failed, hr=%x", hr);
        return ERR_INVALID_SHADER;
    }

    m_ShaderByteCode.pShaderBytecode = m_pShaderBlob->GetBufferPointer();
    m_ShaderByteCode.BytecodeLength = m_pShaderBlob->GetBufferSize();

    m_iPsoHashValue = 0;
    HashRange(m_iPsoHashValue,
        (char const*)m_ShaderByteCode.pShaderBytecode,
        (char const*)m_ShaderByteCode.pShaderBytecode + m_ShaderByteCode.BytecodeLength);

    return S_Success;
}

size_t D3D12Shader::PsoHashValue()
{
    if (!m_bCompileReady)
        return 0;
    return m_iPsoHashValue;
}

void D3D12Shader::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const
{
    if (!m_bCompileReady) return;

    switch (m_eShaderType)
    {
    case ShaderType::Vertex:   pso_desc.VS = m_ShaderByteCode; break;
    case ShaderType::Pixel:    pso_desc.PS = m_ShaderByteCode; break;
    case ShaderType::Geometry: pso_desc.GS = m_ShaderByteCode; break;
    case ShaderType::Hull:     pso_desc.HS = m_ShaderByteCode; break;
    case ShaderType::Domain:   pso_desc.DS = m_ShaderByteCode; break;
    default: break;
    }
}

void D3D12Shader::UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc) const
{
    if (!m_bCompileReady) return;
    if (m_eShaderType == ShaderType::Compute)
        pso_desc.CS = m_ShaderByteCode;
}

SEEK_NAMESPACE_END
