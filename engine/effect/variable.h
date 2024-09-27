#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "utils/log.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
* RenderVariable
* ****************************************************************************/
class EffectVariable
{
public:
    virtual ~EffectVariable() {}
    virtual void* Data() const { LOG_ERROR("Can't be called."); return nullptr; }
    virtual uint32_t Length() const { LOG_ERROR("Can't be called."); return 0xffffffff; }
    virtual uint32_t Size() const { LOG_ERROR("Can't be called."); return 0xffffffff; }

#define EffectVariable_Declare(Tp) \
    virtual EffectVariable& operator=(Tp                  const& value) { LOG_ERROR("Can't be called."); return *this; } \
    virtual EffectVariable& operator=(std::vector<Tp>     const& value) { LOG_ERROR("Can't be called."); return *this; } \
    virtual void            Value(Tp                     & value) const { LOG_ERROR("Can't be called."); } \
    virtual void            Value(std::vector<Tp>        & value) const { LOG_ERROR("Can't be called."); }

    EffectVariable_Declare(RHITexturePtr)
    EffectVariable_Declare(SamplerPtr)
    EffectVariable_Declare(RHIRenderBufferPtr)
#undef EffectVariable_Declare

protected:
    EffectVariable() {}
};
using EffectVariablePtr = std::shared_ptr<EffectVariable>;

template <typename T>
class EffectVariableConcrete : public EffectVariable
{
public:
    EffectVariableConcrete()
    {
        new (m_Data.val)T{};
    }
    virtual ~EffectVariableConcrete() override
    {
        this->RetriveT().~T();
    }
    virtual EffectVariable& operator=(T const& value) override
    {
        this->RetriveT() = value;
        return *this;
    }
    virtual void Value(T& val) const override
    {
        val = this->RetriveT();
    }
    virtual void* Data() const override
    {
        return (void*)m_Data.val;
    }
    virtual uint32_t Size() const override
    {
        return 1;
    }
    virtual uint32_t Length() const override
    {
        return (uint32_t)sizeof(T);
    }

protected:
    T& RetriveT()
    {
        union Raw2T
        {
            uint8_t* raw;
            T* t;
        } r2t;
        r2t.raw = m_Data.val;
        return *r2t.t;
    }
    T const& RetriveT() const
    {
        union Raw2T
        {
            uint8_t const* raw;
            T const* t;
        } r2t;
        r2t.raw = m_Data.val;
        return *r2t.t;
    }

private:
    union VarData
    {
        uint8_t val[sizeof(T)];
    };
    VarData m_Data;
};

#define EFFECT_VARIABLE_CONCRETE(name, type) \
    using EffectVariable##name          = EffectVariableConcrete<type>;

EFFECT_VARIABLE_CONCRETE(Sampler,       SamplerPtr)
EFFECT_VARIABLE_CONCRETE(RHITexture,       RHITexturePtr)
EFFECT_VARIABLE_CONCRETE(RHIRenderBuffer,  RHIRenderBufferPtr)

#undef EFFECT_VARIABLE_CONCRETE

SEEK_NAMESPACE_END
