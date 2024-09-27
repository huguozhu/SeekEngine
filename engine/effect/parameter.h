#pragma once

#include "kernel/kernel.h"
#include "effect/variable.h"
#include "rhi/base/rhi_render_state.h"
#include "utils/error.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************
 * Independent Function
 *****************************************************************/
TexFilterOp         TexFilterOpFromName         (std::string const& name);
TexAddressMode      TexAddressingModeFromName   (std::string const& name);
CompareFunction     CompareFunctionFromName     (std::string const& name);
CullMode            CullModeFromName            (std::string const& name);
BlendOperation      BlendOperationFromName      (std::string const& name);
StencilOperation    StencilOperationFromName    (std::string const& name);
BlendFactor         BlendFactorFromName         (std::string const& name);
bool                BoolFromString              (std::string const& str);

/******************************************************************************
 * EffectDataType
 ******************************************************************************/
enum class EffectDataType : uint32_t
{
    Unknown = 0,
    ConstantBuffer,
    Buffer,
    RWBuffer,
    Texture,
    RWTexture,
    Sampler,
    SampledTexture, // for some shading language has no separate texture, only have combined texture, like glsl
};

/******************************************************************************
 * EffectParam
 ******************************************************************************/
struct EffectParam final
{
    template <class T>
    EffectParam& operator=(T const& value)
    {
        *m_pVariable = value;
        return *this;
    }

    template <class T>
    void GetValue(T& val) const 
    {
        m_pVariable->Value(val); 
    }

    SResult UpdateConstantBuffer(const void* data, size_t size);

    std::string     const&  GetName()               const { return m_szName; }
    EffectVariable  const&  GetVar()                const { return *m_pVariable; }
    EffectDataType          GetDataType()           const { return m_eDataType; }

    std::unique_ptr<EffectVariable> ReadRenderVariable(EffectDataType data_type);

    std::string                                             m_szName;
    std::string                                             m_fallbackName;
    EffectDataType                                          m_eDataType = EffectDataType::Unknown;
    std::unique_ptr<EffectVariable>                         m_pVariable;
    std::map<std::string, uint32_t>                         m_vBindings;

    // valid when m_eDataType is EffectDataType::SampledTexture
    EffectParam* m_pTextureParam = nullptr;
    EffectParam* m_pSamplerParam = nullptr;
};

struct EffectPredefine
{
    std::string name;
    std::string value;
};
static std::vector<EffectPredefine> NULL_PREDEFINES;
static std::vector<std::string>     NULL_PARAMS;

std::string GenerateSeedString(std::vector<EffectPredefine>& predefines);

SEEK_NAMESPACE_END
