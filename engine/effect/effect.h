#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_shader.h"
#include "effect/parameter.h"
#include "effect/technique.h"

SEEK_NAMESPACE_BEGIN

// manager all Technique resources and the related Shader resources
class Effect
{
public:
    Effect(Context* context)
        : m_pContext(context)
    { }

    ~Effect() = default;
    
    SResult Initialize();
    RHIShader* CreateShader(ShaderType stage, const ShaderResourcePtr& shaderRes);
    VirtualTechnique* GetVirtualTechnique(const std::string& name);
    Technique* GetTechnique(const std::string& name, const std::vector<EffectPredefine>& user_predefines);
    Technique* GetTechnique(const std::string& name);

private:
    void LoadDefaultVirtualTechniques();

private:
    Context* m_pContext = nullptr;
    std::unordered_map<std::string, RHIShaderPtr> m_Shaders;
    std::unordered_map<std::string, VirtualTechniquePtrUnique> m_VirtualTechniques;
};

SEEK_NAMESPACE_END
