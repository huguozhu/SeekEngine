#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_shader.h"
#include "rhi/d3d12/d3d12_context.h"
#include "kernel/context.h"
#include "math/hash.h"
#include "utils/log.h"
#include "utils/timer.h"

#define SEEK_MACRO_FILE_UID 69     // this code is auto generated, don't touch it!!!

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
    case ShaderType::Vertex:   return "vs_5_0";
    case ShaderType::Pixel:    return "ps_5_0";
    case ShaderType::Geometry: return "gs_5_0";
    case ShaderType::Hull:     return "hs_5_0";
    case ShaderType::Domain:   return "ds_5_0";
    case ShaderType::Compute:  return "cs_5_0";
    default:                   return nullptr;
    }
}

SResult D3D12Shader::OnCompile()
{
    std::string macroStr;
    for (auto& m : m_vMacros)
        macroStr += m.first + ":" + m.second + ",";
    TIMER_BEG(t1);
    LOG_INFO("D3D12Shader::OnCompile() %s Macro(%s)", m_szEntryFuncName.c_str(), macroStr.c_str());

    ID3DBlob* pError = nullptr;
    ID3DBlobPtr pCode = nullptr;
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    if (m_pContext->EnableDebug())
        dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    LPCVOID code_data = nullptr;
    SIZE_T code_size = 0;
    HRESULT hr = S_OK;
    if (m_bCodePrecompiled)
    {
        code_data = m_szCode.data();
        code_size = m_szCode.size();
    }
    else
    {
        LPCVOID data = m_szCode.data();
        SIZE_T data_size = m_szCode.size();
        const char* entry_func = m_szEntryFuncName.c_str();
        const char* target = GetCompileTarget(m_eShaderType);
        D3D_SHADER_MACRO* pMacros = nullptr;
        size_t macro_size = m_vMacros.size() + 1 + m_vPredefines.size();
        if (macro_size > 0)
        {
            pMacros = (D3D_SHADER_MACRO*)malloc(sizeof(D3D_SHADER_MACRO) * (macro_size + 1));
            seek_memset_s(pMacros, sizeof(D3D_SHADER_MACRO) * (macro_size + 1), 0, sizeof(D3D_SHADER_MACRO) * (macro_size + 1));
            uint32_t index = 0;
            for (auto& m : m_vMacros)
            {
                pMacros[index].Name = m.first.c_str();
                pMacros[index].Definition = m.second.c_str();
                index++;
            }
            for (auto& p : m_vPredefines)
            {
                pMacros[index].Name = p.name.c_str();
                pMacros[index].Definition = p.value.c_str();
                index++;
            }
            pMacros[index].Name = "PLATFORM_HLSL";
            pMacros[index].Definition = "1";
        }
        hr = D3DCompile(data, data_size, nullptr, pMacros, nullptr, entry_func, target, dwShaderFlags, 0, pCode.GetAddressOf(), &pError);
        if (pMacros)
        {
            free(pMacros);
        }
        if (FAILED(hr))
        {
            LOG_ERROR("D3DCompile Error:\r\n %s", (char*)pError->GetBufferPointer());
            SAFE_RELEASE(pError);
            return ERR_INVALID_SHADER;
        }
        SAFE_RELEASE(pError);
    }

    m_pShaderBlob = pCode;
    m_ShaderByteCode.pShaderBytecode = m_pShaderBlob->GetBufferPointer();
    m_ShaderByteCode.BytecodeLength = m_pShaderBlob->GetBufferSize();

    m_iPsoHashValue = 0;
    HashRange(m_iPsoHashValue,
        (char const*)m_ShaderByteCode.pShaderBytecode,
        (char const*)m_ShaderByteCode.pShaderBytecode + m_ShaderByteCode.BytecodeLength);

    TIMER_END(t1, "D3D12Shader::OnCompile()");
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

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
