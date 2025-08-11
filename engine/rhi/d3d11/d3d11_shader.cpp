#include "rhi/d3d11/d3d11_shader.h"
#include "rhi/d3d11/d3d11_context.h"
#include "rhi/d3d11/d3d11_translate.h"
#include "rhi/d3d11/d3d11_texture.h"
#include "rhi/d3d11/d3d11_render_state.h"
#include "rhi/d3d11/d3d11_gpu_buffer.h"

#include "math/math_utility.h"
#include "kernel/context.h"
#include "utils/log.h"
#include "utils/timer.h"

#define SEEK_MACRO_FILE_UID 2     // this code is auto generated, don't touch it!!!

ID3D11UnorderedAccessView* null_uavs[128] = { nullptr };
ID3D11ShaderResourceView*  null_srvs[128] = { nullptr };

SEEK_NAMESPACE_BEGIN

LPCSTR GetCompileShaderModel(ShaderType type)
{
    static const char* v = "vs_5_0";
    static const char* p = "ps_5_0";
    static const char* g = "gs_5_0";
    static const char* h = "hs_5_0";
    static const char* d = "ds_5_0";
    static const char* c = "cs_5_0";
    switch (type)
    {
        case ShaderType::Vertex:     return v;
        case ShaderType::Pixel:      return p;
        case ShaderType::Geometry:   return g;
        case ShaderType::Hull:       return h;
        case ShaderType::Domain:     return d;
        case ShaderType::Compute:    return c;
    }
    return nullptr;
}

SResult D3D11Shader::Active()
{
    if (!m_bCompileReady)
    {
        SEEK_RETIF_FAIL(this->OnCompile());
        m_bCompileReady = true;
    }

    D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    rc.SetD3DShader(m_eShaderType, (ID3D11DeviceChild*)m_pShader.Get());

    return S_Success;
}

void D3D11Shader::Deactive()
{

}

SResult D3D11Shader::OnCompile()
{
    std::string macroStr;
    for (auto& m : m_vMacros)
        macroStr += m.first + ":" + m.second + ",";
    TIMER_BEG(t1);
    LOG_INFO("D3D11Shader::OnCompile() %s Macro(%s)", m_szEntryFuncName.c_str(), macroStr.c_str());

    D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();

    ID3DBlob* pError = nullptr;
    ID3DBlobPtr pCode = nullptr;
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    if (m_pContext->EnableDebug())
        dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    LPCVOID code_data = nullptr;
    SIZE_T code_size = 0;
    HRESULT hr = S_OK;
    m_bCodePrecompiled = true;
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
        const char* target = GetCompileShaderModel(m_eShaderType);
        D3D_SHADER_MACRO* pMacros = nullptr;
        size_t macro_size = m_vMacros.size() + 1 + m_vPredefines.size();
        if (macro_size > 0)
        {
            pMacros = (D3D_SHADER_MACRO*)malloc(sizeof(D3D_SHADER_MACRO) * (macro_size + 1));
            seek_memset_s(pMacros, sizeof(D3D_SHADER_MACRO) * (macro_size + 1), 0, sizeof(D3D_SHADER_MACRO) * (macro_size + 1));
            uint32_t index = 0;
            for (auto& m : m_vMacros)
            {
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!! TODO
                if (m.first == "MORPH_SIZE" && m.second == "0")
                    m.second = "1";

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

        code_data = pCode->GetBufferPointer();
        code_size = pCode->GetBufferSize();
    }

    switch (m_eShaderType)
    {
        case ShaderType::Vertex:
        {
            ID3D11VertexShaderPtr vertex_shader = nullptr;
            hr = pDevice->CreateVertexShader(code_data, code_size, nullptr, vertex_shader.GetAddressOf());
            if (FAILED(hr))
                return ERR_INVALID_ARG;
            m_pShader = vertex_shader;
            m_pD3DVsCode = pCode;
            break;
        }
        case ShaderType::Pixel:
        {
            ID3D11PixelShaderPtr pixel_shader = nullptr;
            hr = pDevice->CreatePixelShader(code_data, code_size, nullptr, pixel_shader.GetAddressOf());
            if (FAILED(hr))
                return ERR_INVALID_ARG;
            m_pShader = pixel_shader;
            break;
        }
        case ShaderType::Geometry:
        {
            ID3D11GeometryShaderPtr geometry_shader = nullptr;
            hr = pDevice->CreateGeometryShader(code_data, code_size, nullptr, geometry_shader.GetAddressOf());
            if (FAILED(hr))
                return ERR_INVALID_ARG;
            m_pShader = geometry_shader;
            break;
        }
        case ShaderType::Hull:
        {
            ID3D11HullShaderPtr hull_shader = nullptr;
            hr = pDevice->CreateHullShader(code_data, code_size, nullptr, hull_shader.GetAddressOf());
            if (FAILED(hr))
                return ERR_INVALID_ARG;
            m_pShader = hull_shader;
            break;
        }
        case ShaderType::Domain:
        {
            ID3D11DomainShaderPtr domain_shader = nullptr;
            hr = pDevice->CreateDomainShader(code_data, code_size, nullptr, domain_shader.GetAddressOf());
            if (FAILED(hr))
                return ERR_INVALID_ARG;
            m_pShader = domain_shader;
            break;
        }
        case ShaderType::Compute:
        {
            ID3D11ComputeShaderPtr compute_shader = nullptr;
            hr = pDevice->CreateComputeShader(code_data, code_size, nullptr, compute_shader.GetAddressOf());
            if (FAILED(hr))
                return ERR_INVALID_ARG;

            m_pShader = compute_shader;

            break;
        }
        default:
            return ERR_INVALID_ARG;
    }

    SAFE_RELEASE(pError);

    TIMER_END(t1, "D3D11Shader::OnCompile()");
    return S_Success;
 }

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
