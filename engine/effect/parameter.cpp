#include "effect/parameter.h"
#include "kernel/context.h"
#include "rhi/base/render_state.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/render_buffer.h"
#include "utils/log.h"
#include "math/hash.h"

#define SEEK_MACRO_FILE_UID 79     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

std::unique_ptr<EffectVariable> CreateEffectVariable(EffectDataType data_type)
{
    switch (data_type)
    {
        case EffectDataType::ConstantBuffer:
        case EffectDataType::Buffer:
        case EffectDataType::RWBuffer:
            return MakeUniquePtr<EffectVariableRHIRenderBuffer>();
        case EffectDataType::Texture:
        case EffectDataType::RWTexture:
            return MakeUniquePtr<EffectVariableRHITexture>();
        case EffectDataType::Sampler:
            return MakeUniquePtr<EffectVariableSampler>();
        case EffectDataType::SampledTexture:
            return nullptr; // it's just a placehold type, has no instance
        default:
            LOG_ERROR("invalid EffectDataType %d", data_type);
            return nullptr;
    }
}

std::unique_ptr<EffectVariable> EffectParam::ReadRenderVariable(EffectDataType data_type)
{
    std::unique_ptr<EffectVariable> var = CreateEffectVariable(data_type);
    switch (data_type)
    {
        case EffectDataType::ConstantBuffer:
        case EffectDataType::Buffer:
        case EffectDataType::RWBuffer:
        {
            *var = RHIRenderBufferPtr();
            break;
        }
        case EffectDataType::Texture:
        case EffectDataType::RWTexture:
        {
            *var = RHITexturePtr();
            break;
        }
        case EffectDataType::Sampler:
        {
            *var = SamplerPtr();
            break;
        }
        case EffectDataType::SampledTexture:
        {    
            // do nothing
            break;
        }
        default:
            break;
    }
    return var;
}

/******************************************************************************
 * EffectParam
 ******************************************************************************/
std::string GenerateSeedString(std::vector<EffectPredefine>& predefines)
{
    std::string seedStr;
    if (predefines.size() > 0)
    {
        // TODO: optimize
        std::map<std::string, EffectPredefine> _pds;
        for (auto& pd : predefines)
        {
            _pds[pd.name] = pd;
        }

        size_t seed = 0;

        //LOG_INFO("GenerateSeedString:");
        for (auto& pd : _pds)
        {
            //LOG_INFO("  %s\t\t%s", pd.second.name.c_str(), pd.second.value.c_str());
            HashRange(seed, pd.second.value.begin(), pd.second.value.end());
        }

        char seedC[128];
        sprintf_s(seedC, sizeof(seedC), "_%zx", seed);
        seedStr = seedC;
    }
    return seedStr;
}

SResult EffectParam::UpdateConstantBuffer(const void* data, size_t size)
{
    if (m_eDataType != EffectDataType::ConstantBuffer)
    {
        LOG_ERROR("UpdateConstantBuffer for param with type EffectDataType::ConstantBuffer, %s type is %u", m_szName.c_str(), m_eDataType);
        return ERR_INVALID_ARG;
    }

    RHIRenderBufferPtr rb;
    GetValue(rb);
    if (!rb)
    {
        LOG_ERROR("ConstantBuffer %s is not created, it SHOULDN'T happen", m_szName.c_str());
        return ERR_UNKNOWN;
    }

    SResult ret = rb->Update(data, (uint32_t)size);
    if (SEEK_CHECKFAILED(ret))
        LOG_ERROR("update ConstantBuffer %s fail, ret=%x", m_szName.c_str(), ret);
    return ret;
}

SEEK_NAMESPACE_END


#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
