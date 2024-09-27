#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "effect/parameter.h"
#include "utils/safe_c.h"

SEEK_NAMESPACE_BEGIN

enum class ShaderType : uint8_t
{
    Vertex = 0,
    Pixel,
    Geometry,
    Hull,
    Domain,
    Compute,
    Num
};
static constexpr std::underlying_type<ShaderType>::type SHADER_STAGE_COUNT = to_underlying(ShaderType::Num);

class RHIShader
{
public:
    void SetShaderCode(const void* byteCode, size_t byteCodeSize)
    {
        m_szCode.resize(byteCodeSize);
        memcpy_s((void*)m_szCode.data(), byteCodeSize, byteCode, byteCodeSize);
    }

    void            AddParam(EffectParam* param);
    void            AddMacro(const std::string& name, const std::string& value);
    void            AddPredefine(const EffectPredefine& predefine);
    ShaderType      Type() const { return m_eShaderType; }

    void            SetCodePrecompiled(bool bPrecompiled) { m_bCodePrecompiled = bPrecompiled; }

    void            SetCsThreadsPerGroup(uint32_t x, uint32_t y, uint32_t z);
    void            GetCsThreadsPerGroup(uint32_t& x, uint32_t& y, uint32_t& z);
    size_t          GetPredefineNum()
    {
        return m_vPredefines.size();
    }

    EffectPredefine& GetPredefineByIndex(size_t idx)
    {
        return m_vPredefines[idx];
    }

    std::vector<EffectPredefine>& GetPredefine()
    {
        return m_vPredefines;
    }


    std::string& GetCode()
    {
        return m_szCode;
    }

    std::string& GetEntryPoint()
    {
        return m_szEntryFuncName;
    }

    std::map<std::string, std::string>& GetMacros()
    {
        return m_vMacros;
    }

    const std::vector<EffectParam*>& GetParams()
    {
        return m_vParams;
    }

protected:
    RHIShader(Context* context)
        : m_pContext(context) {}
    RHIShader(Context* context, ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code)
        : m_pContext(context), m_eShaderType(type), m_szName(name), m_szEntryFuncName(entry_func_name), m_szCode(code) {}
    virtual ~RHIShader() {}

    virtual SResult   OnCompile() = 0;

protected:
    Context*                                m_pContext = nullptr;
    bool                                    m_bCompileReady = false;
    ShaderType                              m_eShaderType = ShaderType::Num;
    std::string                             m_szName;
    std::string                             m_szEntryFuncName;
    std::string                             m_szCode;
    std::vector<EffectParam*>               m_vParams;
    std::vector<EffectPredefine>            m_vPredefines;
    std::map<std::string, std::string>      m_vMacros;

    uint3                                   m_iCsThreadsPerGroup = uint3(0, 0, 0);
    bool                                    m_bCodePrecompiled = false;
};

SEEK_NAMESPACE_END
