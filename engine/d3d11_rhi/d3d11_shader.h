#pragma once
#include "rhi/shader.h"
#include "d3d11_rhi/d3d11_predeclare.h"

SEEK_NAMESPACE_BEGIN

class D3D11Shader : public Shader
{
public:
    D3D11Shader(Context* context, ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code)
        : Shader(context, type, name, entry_func_name, code)
    {}
    virtual ~D3D11Shader() override
    {
        m_pShader.Reset();
        m_pD3DVsCode.Reset();
    }
    SResult               Active();
    void                    Deactive();
    virtual SResult       OnCompile() override;

    //ID3DBlobPtr GetD3DVSCode() { return m_pD3DVsCode; }
    const void* GetBufferPointer()
    {
        if (m_bCodePrecompiled)
            return m_szCode.data();
        else
            return m_pD3DVsCode->GetBufferPointer();
    }
    
    uint32_t GetBufferSize()
    {
        if (m_bCodePrecompiled)
            return m_szCode.size();
        else
            return m_pD3DVsCode->GetBufferSize();
    }

private:
    ID3D11DeviceChildPtr    m_pShader = nullptr;
    ID3DBlobPtr             m_pD3DVsCode = nullptr;
};

SEEK_NAMESPACE_END
