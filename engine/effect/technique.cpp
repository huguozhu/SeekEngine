#include "effect/technique.h"
#include "kernel/context.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_program.h"
#include "rhi/base/rhi_mesh.h"
#include "effect/effect.h"
#include "effect/variable.h"

#define SEEK_MACRO_FILE_UID 29     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////////////////////////
// VirtualTechnique
SResult VirtualTechnique::Build()
{
    std::fill(m_metaShaderResources.begin(), m_metaShaderResources.end(), nullptr);
    m_predefines.clear();

    // collect all predefines in shaders
    for (uint32_t shader_type = 0; shader_type < to_underlying(ShaderType::Num); shader_type++)
    {
        std::string shader_name = m_shaderNames[shader_type];
        if (shader_name.empty())
            continue;

        MetaShaderResourcePtr metaShaderRes = m_pContext->ResourceManagerInstance().LoadMetaShaderResource(shader_name);
        if (!metaShaderRes)
        {
            LOG_ERROR("load %s meta shader fail", shader_name.c_str());
            return ERR_INVALID_ARG;
        }
        m_metaShaderResources[shader_type] = metaShaderRes;

        for (auto& predefine : metaShaderRes->metaInfo.predefines)
        {
            bool found = false;
            for (auto& metaPredefine : m_predefines)
            {
                if (metaPredefine.name == predefine.name)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                m_predefines.push_back(predefine);
            }
        }
    }

    return S_Success;
}

static bool GetActivePredefineValue(const std::vector<EffectPredefine>& predefines, const std::string& activeName, std::string& activeValue)
{
    for (auto& _predefine : predefines)
    {
        if (_predefine.name == activeName)
        {
            activeValue = _predefine.value;
            return true;
        }
    }
    return false;
}

static bool ActivePredefine(
    std::vector<shadercompiler::MetaPredefine>  const& metaPredefines,
    std::vector<EffectPredefine>                const& providedPredefines,
    std::vector<EffectPredefine>                     & activedPredefines)
{
    activedPredefines.resize(metaPredefines.size());
    for (size_t i = 0; i < activedPredefines.size(); i++)
    {
        activedPredefines[i].name = metaPredefines[i].name;
        if (!GetActivePredefineValue(providedPredefines, activedPredefines[i].name, activedPredefines[i].value))
        {
            activedPredefines.clear();
            return false;
        }
    }
    return true;
}

Technique* VirtualTechnique::Concrete(const std::vector<EffectPredefine>& predefines, const RenderStateDesc& renderStateDesc)
{
    std::vector<EffectPredefine> activedTechPredefines;
    bool actived = ActivePredefine(m_predefines, predefines, activedTechPredefines);
    if (!actived)
    {
        LOG_ERROR("concrete VirtualTechnique fail, the provided predefines are invalid");
        return nullptr;
    }
    
    std::string activedTechName = m_techName + GenerateSeedString(activedTechPredefines) + "_" + std::to_string(renderStateDesc.Hash());
    auto techIt = m_concreteTechs.find(activedTechName);
    if (techIt != m_concreteTechs.end())
        return techIt->second.get();

    TechniquePtrUnique tech = MakeUniquePtr<Technique>(m_pContext, this);
    tech->SetName(activedTechName);

    for (size_t stage = 0; stage < SHADER_STAGE_COUNT; stage++)
    { 
        if (!m_metaShaderResources[stage])
            continue;
        
        std::vector<EffectPredefine> activedShaderPredefines;
        bool active = ActivePredefine(m_metaShaderResources[stage]->metaInfo.predefines, activedTechPredefines, activedShaderPredefines);
        if (!active)
        {
            LOG_ERROR("active shader predefine fail, it SHOULDN'T happen");
            return nullptr;
        }

        std::string activedShaderName = m_metaShaderResources[stage]->_name + GenerateSeedString(activedShaderPredefines);
        ShaderResourcePtr shaderRes = m_pContext->ResourceManagerInstance().LoadShaderResource(activedShaderName);
        if (!shaderRes)
        {
            LOG_ERROR("no shader resource for %s", activedShaderName.c_str());
            return nullptr;
        }
        
        tech->SetShaderResource(static_cast<ShaderType>(stage), shaderRes);
    }

    tech->SetRenderStateDesc(renderStateDesc);

    SResult ret = tech->Build();
    if (SEEK_CHECKFAILED(ret))
    {
        LOG_ERROR("Technique setup fail, VirtualTechnique: %s, with predefines", m_techName.c_str());
        for (auto idx = 0; idx != predefines.size(); idx++)
        {
            LOG_ERROR("  %s -> %s", predefines[idx].name.c_str(), predefines[idx].value.c_str());
        }
        return nullptr;
    }

    Technique* tech_ = tech.get();
    m_concreteTechs[activedTechName] = std::move(tech);
    return tech_;
}

Technique* VirtualTechnique::Concrete(const std::vector<EffectPredefine>& predefines)
{
    return Concrete(predefines, m_defaultRenderState);
}

Technique* VirtualTechnique::Concrete()
{
    std::vector<EffectPredefine> null_predefines;
    return Concrete(null_predefines, m_defaultRenderState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Technique

RHIRenderStatePtr& Technique::GetRenderState()
{
    if (!m_RenderState)
    {
        m_RenderState = m_pContext->RHIContextInstance().GetRenderState(m_RenderStateDesc);
    }
    return m_RenderState;
}

static EffectDataType ConvertFromResourceType(shadercompiler::ResourceType type)
{
    switch (type)
    {
        case shadercompiler::ResourceType::ConstantBuffer:
            return EffectDataType::ConstantBuffer;
        case shadercompiler::ResourceType::Texture:
            return EffectDataType::Texture;
        case shadercompiler::ResourceType::RWTexture:
            return EffectDataType::RWTexture;
        case shadercompiler::ResourceType::Buffer:
            return EffectDataType::Buffer;
        case shadercompiler::ResourceType::RWBuffer:
            return EffectDataType::RWBuffer;
        case shadercompiler::ResourceType::Sampler:
            return EffectDataType::Sampler;
        case shadercompiler::ResourceType::SampledTexture:
            return EffectDataType::SampledTexture;
        default:
            return EffectDataType::Unknown;
    }
}

void Technique::CreateEffectVariable(Param& param)
{
    switch (param.dataType)
    {
        case EffectDataType::ConstantBuffer:
        case EffectDataType::Buffer:
        case EffectDataType::RWBuffer:
        {
            param.variable = MakeUniquePtr<EffectVariableRHIRenderBuffer>();
            break;
        }
        case EffectDataType::Texture:
        case EffectDataType::RWTexture:
        {
            param.variable = MakeUniquePtr<EffectVariableRHITexture>();
            break;
        }
        case EffectDataType::Sampler:
        {
            param.variable = MakeUniquePtr<EffectVariableRHISampler>();
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
}

SResult Technique::Build()
{
    // collect all params in shaders
    for (size_t stage = 0; stage != SHADER_STAGE_COUNT; stage++)
    {
        auto& shaderRes = m_shaderRes[stage];
        if (!shaderRes)
            continue;

        for (auto& resource : shaderRes->reflectInfo.resources)
        {
            auto paramIt = m_params.find(resource.name);
            if (paramIt != m_params.end())
            {
                paramIt->second.bindings[stage] = resource.binding;
                continue;
            }
            
            Param param;
            param.dataType = ConvertFromResourceType(resource.type);
            if (param.dataType == EffectDataType::Unknown)
            {
                LOG_ERROR("invalid resource.type %d", resource.type);
                return ERR_INVALID_ARG; // return? or keep going
            }
            param.name = resource.name;
            param.fallbackName = resource.fallback_name;
            param.bindings[stage] = resource.binding;
            if (param.dataType == EffectDataType::SampledTexture)
            {
                param.textureParamName = resource.texture_name;
                param.samplerParamName = resource.sampler_name;
            }
            CreateEffectVariable(param);

            m_params[param.name] = std::move(param);
        }

        // another pass to check if SampledTexture has related separate texture&sampler
        for (auto& resource : shaderRes->reflectInfo.resources)
        {
            if (resource.type == shadercompiler::ResourceType::SampledTexture)
            {
                auto& sampledTextureParam = m_params[resource.name]; // always find

                auto samplerIt = m_params.find(sampledTextureParam.samplerParamName);
                if (samplerIt == m_params.end())
                {
                    LOG_ERROR("has no sampler param %s in combined SampledTexture %s", resource.sampler_name.c_str(), resource.name.c_str());
                    // do something?
                }

                auto textureIt = m_params.find(sampledTextureParam.textureParamName);
                if (textureIt == m_params.end())
                {
                    LOG_ERROR("has no texture param %s in combined SampledTexture %s", resource.texture_name.c_str(), resource.name.c_str());
                    // do something?
                }
            }
        }
    }

    RHIProgramPtr program = m_pContext->RHIContextInstance().CreateRHIProgram();
    for (size_t stage = 0; stage != SHADER_STAGE_COUNT; stage++)
    {
        if (!m_shaderRes[stage])
            continue;

        RHIShader* shader = m_pContext->EffectInstance().CreateShader((ShaderType)stage, m_shaderRes[stage]);
        if (shader)
        {
            program->SetShader(shader);
        }
    }
    m_pProgram = std::move(program);
    return S_Success;
}

//std::shared_ptr<TechniqueInstance> Technique::CreateInstance()
//{
//    std::shared_ptr<TechniqueInstance> techInst = MakeSharedPtr<TechniqueInstance>(m_pContext, this);
//
//    SResult ret = techInst->Build();
//    if (SEEK_CHECKFAILED(ret))
//    {
//        LOG_ERROR("Build TechniqueInstance fail, ret:%x", ret);
//        return nullptr;
//    }
//
//    return techInst;
//}

SResult Technique::Render(RHIMeshPtr const& mesh)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    RHIRenderStatePtr state;
    if (m_pContext->GetRenderInitInfo().enable_transparent && mesh->GetMaterial() && mesh->GetMaterial()->alpha_mode != AlphaMode::Opaque)
    {
        if (!m_RenderStateForTransparent)
        {
            RenderStateDesc rs = m_RenderStateDesc;
            if (!mesh->GetMaterial()->double_sided && rs.rasterizer.eCullMode == CullMode::None)
                rs.rasterizer.eCullMode = CullMode::Back;
            rs.rasterizer.bFrontFaceCCW = true;
            rs.depthStencil.bDepthWriteMask = false;
            rs.blend.stTargetBlend[0].bBlendEnable = true;
            rs.blend.stTargetBlend[0].eSrcBlendColor = BlendFactor::SrcAlpha;
            rs.blend.stTargetBlend[0].eDstBlendColor = BlendFactor::InvSrcAlpha;
            rs.blend.stTargetBlend[0].eBlendOpColor = BlendOperation::Add;
            rs.blend.stTargetBlend[0].eSrcBlendAlpha = BlendFactor::One;
            rs.blend.stTargetBlend[0].eDstBlendAlpha = BlendFactor::One;
            rs.blend.stTargetBlend[0].eBlendOpAlpha = BlendOperation::Add;
            m_RenderStateForTransparent = rc.GetRenderState(rs);
        }
        state = m_RenderStateForTransparent;
    }
    else
    {
        if (!m_RenderState)
        {
            m_RenderState = rc.GetRenderState(m_RenderStateDesc);
        }
        state = m_RenderState;
    }

    if (!mesh->GetRenderState())
    {
        RenderStateDesc rsd = state->GetRenderStateDesc();
        if (mesh->GetMaterial() && !mesh->GetMaterial()->double_sided)
            rsd.rasterizer.eCullMode = CullMode::None;
        else
            rsd.rasterizer.eCullMode = CullMode::None;
        rsd.rasterizer.bFrontFaceCCW = mesh->m_bFrontFaceCCW;
        state = rc.GetRenderState(rsd);

        mesh->SetRenderState(state);
    }

    if (m_pContext->GetRHIType() == RHIType::GLES && !m_bOpenGLAlreadyRemapBinding)
    {
        // OpenGL need to remap the binding point, it need to be unique in the whole program
        uint32_t uboBinding = 0;
        //uint32_t ssboBinding = 0;
        uint32_t samplerBinding = 0;
        for (auto& paramPair : m_params)
        {
            auto& param = paramPair.second;
            for (size_t stage = 0; stage != SHADER_STAGE_COUNT; stage++)
            {
                if (param.bindings[stage] == INVALID_BINDING_POINT)
                    continue;
                switch (param.dataType)
                {
                case EffectDataType::ConstantBuffer:
                {
                    param.bindings[stage] = uboBinding++;
                    break;
                }
                //case EffectDataType::Buffer:
                //{
                //    param.bindings[stage] = ssboBinding++;
                //    break;
                //}
                //case EffectDataType::RWBuffer:
                //{
                //    param.bindings[stage] = ssboBinding++;
                //    break;
                //}
                case EffectDataType::SampledTexture:
                {
                    param.bindings[stage] = samplerBinding++;
                    break;
                }
                default:
                    break;
                }
            }
        }
        m_bOpenGLAlreadyRemapBinding = true;
    }

    rc.BindRHIProgram(m_pProgram.get());
    Commit();
    SResult ret = rc.Render(m_pProgram.get(), mesh);
    Uncommit();
    return ret;
}

void Technique::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    Commit();
    m_pContext->RHIContextInstance().Dispatch(m_pProgram.get(), x, y, z);
    Uncommit();
}
void Technique::DispatchIndirect(RHIRenderBufferPtr indirectBuf)
{
    Commit();
    m_pContext->RHIContextInstance().DispatchIndirect(m_pProgram.get(), indirectBuf);
    Uncommit();
}
void Technique::DrawIndirect(RHIRenderBufferPtr indirectBuf, MeshTopologyType type)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    RHIRenderStatePtr rs = this->GetRenderState();

    Commit();
    rc.DrawIndirect(m_pProgram.get(), rs, indirectBuf, type);
    Uncommit();
}

SResult Technique::Commit()
{
    RHIContext& rc = m_pContext->RHIContextInstance();

    for (auto& paramPair : m_params)
    {
        auto& param = paramPair.second;
        for (size_t stage = 0; stage != SHADER_STAGE_COUNT; stage++)
        {
            if (param.bindings[stage] == INVALID_BINDING_POINT)
                continue;

            switch (param.dataType)
            {
            case EffectDataType::ConstantBuffer:
            {
                RHIRenderBufferPtr rb;
                param.variable->Value(rb);
                if (rb)
                    rc.BindConstantBuffer((ShaderType)stage, param.bindings[stage], rb.get(), param.fallbackName.c_str());
                else
                    LOG_WARNING("param %s has no resource binding", param.name.c_str());
                break;
            }
            case EffectDataType::Buffer:
            {
                RHIRenderBufferPtr rb;
                param.variable->Value(rb);
                if (rb)
                    rc.BindRHIRenderBuffer((ShaderType)stage, param.bindings[stage], rb.get(), param.name.c_str());
                else
                    LOG_WARNING("param %s has no resource binding", param.name.c_str());
                break;
            }
            case EffectDataType::RWBuffer:
            {
                RHIRenderBufferPtr rb;
                param.variable->Value(rb);
                if (rb)
                    rc.BindRWRHIRenderBuffer((ShaderType)stage, param.bindings[stage], rb.get(), param.name.c_str());
                else
                    LOG_WARNING("param %s has no resource binding", param.name.c_str());
                break;
            }
            default:
            {
                if (m_pContext->GetRHIType() != RHIType::GLES)
                {
                    switch (param.dataType)
                    {
                    case EffectDataType::Texture:
                    {
                        RHITexturePtr tex;
                        param.variable->Value(tex);
                        if (tex)
                            rc.BindTexture((ShaderType)stage, param.bindings[stage], tex.get(), param.name.c_str());
                        else
                            LOG_WARNING("param %s has no resource binding", param.name.c_str());
                        break;
                    }
                    case EffectDataType::RWTexture:
                    {
                        RHITexturePtr tex;
                        param.variable->Value(tex);
                        if (tex)
                            rc.BindRWTexture((ShaderType)stage, param.bindings[stage], tex.get(), param.name.c_str());
                        else
                            LOG_WARNING("param %s has no resource binding", param.name.c_str());
                        break;
                    }
                    case EffectDataType::Sampler:
                    {
                        RHISamplerPtr sampler;
                        param.variable->Value(sampler);
                        if (!sampler)
                        {
                            sampler = rc.GetSampler(SamplerDesc::GetSamplerDescByName(param.name));
                            *(param.variable) = sampler;
                        }
                        rc.BindSampler((ShaderType)stage, param.bindings[stage], sampler.get(), param.name.c_str());
                        break;
                    }
                    default:
                        break;
                    }
                }
                else
                {
                    if (param.dataType == EffectDataType::SampledTexture)
                    {
                        auto texIt = m_params.find(param.textureParamName);
                        if (texIt != m_params.end())
                        {
                            RHITexturePtr tex = nullptr;
                            texIt->second.variable->Value(tex);
                            rc.BindTexture((ShaderType)stage, param.bindings[stage], tex.get(), param.name.c_str());
                        }
                    }
                }
            }
            }
        }
    }
    return S_Success;
}

void Technique::Uncommit()
{
    if (m_pContext->GetRHIType() != RHIType::D3D11)
        return;

    RHIContext& rc = m_pContext->RHIContextInstance();
    for (auto& paramPair : m_params)
    {
        auto& param = paramPair.second;
        for (size_t stage = 0; stage != SHADER_STAGE_COUNT; stage++)
        {
            if (param.bindings[stage] == INVALID_BINDING_POINT)
                continue;

            switch (param.dataType)
            {
            case EffectDataType::Texture:
                rc.BindTexture((ShaderType)stage, param.bindings[stage], nullptr, nullptr);
                break;
            case EffectDataType::RWTexture:
                rc.BindRWTexture((ShaderType)stage, param.bindings[stage], nullptr, nullptr);
                break;
            default:
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// TechniqueInstance
//SResult TechniqueInstance::Build()
//{
//    for (auto& formalParam : m_pTech->GetFormalParams())
//    {
//        ActualParam actualParam;
//        std::unique_ptr<EffectVariable> var;
//        switch (formalParam.second.dataType)
//        {
//            case EffectDataType::ConstantBuffer:
//            case EffectDataType::Buffer:
//            case EffectDataType::RWBuffer:
//            {
//                actualParam.variable = MakeUniquePtr<EffectVariableRenderBuffer>();
//                break;
//            }
//            case EffectDataType::Texture:
//            case EffectDataType::RWTexture:
//            {
//                actualParam.variable = MakeUniquePtr<EffectVariableTexture>();
//                break;
//            }
//            case EffectDataType::Sampler:
//            {
//                actualParam.variable = MakeUniquePtr<EffectVariableSampler>();
//                break;
//            }
//            case EffectDataType::SampledTexture:
//            {
//                // do nothing
//                break;
//            }
//            default:
//                break;
//        }
//    }
//
//    return S_Success;
//}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
