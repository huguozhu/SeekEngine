#pragma once

#include "rhi/base/rhi_shader.h"
#include "kernel/kernel.h"



SEEK_NAMESPACE_BEGIN

class D3D12Shader : public RHIShader
{
public:
    D3D12Shader(Context* context, ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code)
        : RHIShader(context, type, name, entry_func_name, code)
    {
    }

    SResult                 Active();
    void                    Deactive();
    virtual SResult         OnCompile() override;

};

SEEK_NAMESPACE_END
