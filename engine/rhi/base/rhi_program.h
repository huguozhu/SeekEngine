#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_shader.h"

SEEK_NAMESPACE_BEGIN

class RHIProgram
{
public:
    RHIProgram(Context* context) : m_pContext(context) {}
    virtual ~RHIProgram() {}
    RHIShader*  GetShader(ShaderType type) const { return m_vShaders[to_underlying(type)]; }
    SResult     SetShader(RHIShader* shader);

    size_t      Hash() const { return m_Hash; }

protected:
    Context* m_pContext = nullptr;
    std::array<RHIShader*, SHADER_STAGE_COUNT> m_vShaders = {};
    size_t m_Hash = 0;
};

SEEK_NAMESPACE_END
