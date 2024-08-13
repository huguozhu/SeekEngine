#pragma once
#include "rendering/shader.h"
#include "rendering_d3d11/d3d11_predeclare.h"

DVF_NAMESPACE_BEGIN

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
    DVFResult               Active();
    void                    Deactive();
    virtual DVFResult       OnCompile() override;

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

DVF_NAMESPACE_END
