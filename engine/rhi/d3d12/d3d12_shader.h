#pragma once

#include "rhi/base/rhi_shader.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "kernel/kernel.h"



SEEK_NAMESPACE_BEGIN

class D3D12Shader : public RHIShader
{
public:
    D3D12Shader(Context* context, ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code)
        : RHIShader(context, type, name, entry_func_name, code)
    {
    }
    virtual ~D3D12Shader();

    SResult                 Active();
    void                    Deactive();
    virtual SResult         OnCompile() override;

    size_t PsoHashValue();
    const D3D12_SHADER_BYTECODE& GetShaderByteCode() const { return m_ShaderByteCode; }
    void UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc) const;
    void UpdatePsoDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc) const;

private:
    size_t m_iPsoHashValue = 0;
    ID3DBlobPtr m_pShaderBlob = nullptr;
    D3D12_SHADER_BYTECODE m_ShaderByteCode = {};

    static const char* GetCompileTarget(ShaderType type);
};

SEEK_NAMESPACE_END
