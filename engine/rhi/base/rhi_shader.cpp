#include "rhi/base/rhi_shader.h"
#include "effect/parameter.h"

#define SEEK_MACRO_FILE_UID 44     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

void RHIShader::AddParam(EffectParam* param)
{
    m_vParams.push_back(param);
    m_bCompileReady = false;
}
void RHIShader::AddMacro(const std::string& name, const std::string& value)
{
    m_vMacros[name] = value;
    m_bCompileReady = false;
}

void RHIShader::AddPredefine(const EffectPredefine& predefine)
{
    m_vPredefines.push_back(predefine);
}

void RHIShader::SetCsThreadsPerGroup(uint32_t x, uint32_t y, uint32_t z)
{
    m_iCsThreadsPerGroup = uint3(x, y, z);
}
void RHIShader::GetCsThreadsPerGroup(uint32_t& x, uint32_t& y, uint32_t& z)
{
    x = m_iCsThreadsPerGroup[0];
    y = m_iCsThreadsPerGroup[1];
    z = m_iCsThreadsPerGroup[2];
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
