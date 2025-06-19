#include "effect/effect.h"
#include "kernel/context.h"
#include "rhi/base/rhi_render_state.h"
#include "rhi/base/rhi_context.h"
#include "math/hash.h"

#define SEEK_MACRO_FILE_UID 27     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN


SResult Effect::Initialize()
{
    LoadDefaultVirtualTechniques();
    return S_Success;
}

void Effect::LoadDefaultVirtualTechniques()
{
    LoadTechnique("ForwardRenderingCommon", &RenderStateDesc::Default3D(), "MeshRenderingVS", "ForwardRenderingCommonPS", nullptr);
    LoadTechnique("ToneMapping", &RenderStateDesc::PostProcess(), "PostProcessVS", "ToneMappingPS", nullptr);

    LoadTechnique("GenerateShadowMap", &RenderStateDesc::Default3D(), "PreZMeshRenderingVS", "EmptyPS", nullptr);
    LoadTechnique("GenerateCubeShadowMap", &RenderStateDesc::Default3D(), "PreZMeshRenderingVS", "GenerateCubeShadowMapPS", nullptr);
    LoadTechnique("GenerateCascadedShadowMap", &RenderStateDesc::Default3D(), "PreZMeshRenderingVS", "GenerateCascadedShadowMapPS", nullptr);

}

RHIShader* Effect::CreateShader(ShaderType stage, const ShaderResourcePtr& shaderRes)
{
    auto shaderIt = m_Shaders.find(shaderRes->_name);
    if (shaderIt != m_Shaders.end())
        return shaderIt->second.get();

    RHIShaderPtr shader = m_pContext->RHIContextInstance().CreateShader(stage, shaderRes->_name, shaderRes->reflectInfo.entry_point, "");
    if (!shader)
    {
        LOG_ERROR("create shader %s fail", shaderRes->_name.c_str());
        return nullptr;
    }

    if (shader->Type() == ShaderType::Compute)
    {
        shader->SetCsThreadsPerGroup(shaderRes->reflectInfo.block_size.x, 
            shaderRes->reflectInfo.block_size.y, 
            shaderRes->reflectInfo.block_size.z);
    }

    if (shaderRes->reflectInfo.code_type == CodeType::ByteCode)
    {
        shader->SetCodePrecompiled(true);
    }

    shader->SetShaderCode(shaderRes->sourceCode, shaderRes->sourceCodeSize);
    RHIShader* shader_ = shader.get();
    m_Shaders[shaderRes->_name] = std::move(shader);
    return shader_;
}

VirtualTechnique* Effect::GetVirtualTechnique(const std::string& name)
{
    auto it = m_VirtualTechniques.find(name);
    if (it == m_VirtualTechniques.end())
        return nullptr;
    return it->second.get();
}

Technique* Effect::GetTechnique(const std::string& name, const std::vector<EffectPredefine>& user_predefines)
{
    auto virtualTech = GetVirtualTechnique(name);
    if (!virtualTech)
        return nullptr;
    else
        return virtualTech->Concrete(user_predefines);
}

Technique* Effect::GetTechnique(const std::string& name)
{
    return GetTechnique(name, NULL_PREDEFINES);
}

SResult Effect::LoadTechnique(const std::string& name, const RenderStateDesc* pDefaultRenderStateDesc,
    const char* vertexShaderName, const char* pixelShaderName, const char* computeShaderName)
{
    if (this->GetTechnique(name))
        return S_Success;

    VirtualTechniquePtrUnique virtualTech = MakeUniquePtr<VirtualTechnique>(m_pContext);
    virtualTech->SetName(name);
    if (pDefaultRenderStateDesc)
        virtualTech->SetDefaultRenderState(*pDefaultRenderStateDesc);
    if (vertexShaderName)
        virtualTech->SetShaderName(ShaderType::Vertex, vertexShaderName);
    if (pixelShaderName)
        virtualTech->SetShaderName(ShaderType::Pixel, pixelShaderName);
    if (computeShaderName)
        virtualTech->SetShaderName(ShaderType::Compute, computeShaderName);
    SResult ret = virtualTech->Build();
    if (SEEK_CHECKFAILED(ret))
    {
        LOG_ERROR("load default VirtualTechnique %s fail", name);
        return ret;
    }

    m_VirtualTechniques[name] = std::move(virtualTech);
    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
