#pragma once

#include "kernel/kernel.h"
#include "effect/parameter.h"
#include "resource/resource_mgr.h"
#include "rhi/base/rhi_shader.h"
#include "rhi/base/rhi_render_state.h"
#include <unordered_map>
#include <limits>
#include <array>

#define INVALID_BINDING_POINT  std::numeric_limits<uint32_t>::max()

SEEK_NAMESPACE_BEGIN

//struct TechniqueDesc
//{
//    // Prop
//    std::string                 name;
//    RenderStateDesc             renderStateDesc;
//    std::string                 shaderName[to_underlying(ShaderType::Num)];
//
//    // Load from shader .meta
//    MetaShaderResourcePtr                       metaShaderResources[to_underlying(ShaderType::Num)];
//    std::vector<shadercompiler::MetaPredefine>  metaPredefines;
//};

/*
VirtualTechnique like a template function, which contains two parameter type
  - predefine, like template parameter
  - parameter, like formal parameter
After specify the predefines, VirtualTechnique becomes Technique, so Technique contain one parameter type
  - parameter, like formal parameter
After specify parameters, Technique becomes TechniqueInstance, and all parameters are specified
*/

class Technique;
//class TechniqueInstance;

class VirtualTechnique
{
public:
    VirtualTechnique(Context* context) : m_pContext(context)
    { }
    ~VirtualTechnique() = default;

    void SetName(const std::string& name)
    {
        m_techName = name;
    }

    const std::string& GetName() const
    {
        return m_techName;
    }

    void SetDefaultRenderState(const RenderStateDesc& renderState)
    {
        m_defaultRenderState = renderState;
    }
    
    const RenderStateDesc& GetDefaultRenderState()
    {
        return m_defaultRenderState;
    }

    void SetShaderName(ShaderType shaderType, const std::string& shaderName)
    {
        m_shaderNames[to_underlying(shaderType)] = shaderName;
    }

    const std::vector<shadercompiler::MetaPredefine>& GetMetaPredefines() const
    {
        return m_predefines;
    }

    bool IsGraphicsPipeline() const
    {
        return m_metaShaderResources[(size_t)ShaderType::Vertex] != nullptr;
    }
    
    bool IsComputePipeline() const
    {
        return m_metaShaderResources[(size_t)ShaderType::Compute] != nullptr;
    }
    
    SResult Build();

    // concrete VituralTechnique with predefine set & render state, if the same predefine set has been concreted
    // return the previous concreted Technique
    // VirtualTechnique owns the Technique
    Technique* Concrete(const std::vector<EffectPredefine>& predefines, const RenderStateDesc& renderStateDesc);
    Technique* Concrete(const std::vector<EffectPredefine>& predefines);
    Technique* Concrete();

private:
    Context* m_pContext = nullptr;
    std::string m_techName;
    std::vector<shadercompiler::MetaPredefine> m_predefines;
    RenderStateDesc m_defaultRenderState{};
    std::array<std::string, SHADER_STAGE_COUNT> m_shaderNames; // shader names which is not actived
    std::array<MetaShaderResourcePtr, SHADER_STAGE_COUNT> m_metaShaderResources;
    
    std::unordered_map<std::string, TechniquePtrUnique> m_concreteTechs;
};

class Technique
{
public:
    //static const uint32_t INVALID_BINDING_POINT = std::numeric_limits<uint32_t>::max();
    struct Param
    {
        using BindingArray = std::array<uint32_t, SHADER_STAGE_COUNT>;

        std::string name;
        std::string fallbackName;
        EffectDataType dataType = EffectDataType::Unknown;
        uint32_t arraySize = 1;
        BindingArray bindings; // uniform param can be shared between shaders, and can have different bindings

        // when dataType is SampledTexture, this is the original separate texture&sampler name
        std::string textureParamName;
        std::string samplerParamName;

        EffectVariablePtr variable;

        Param()
        {
            std::fill(bindings.begin(), bindings.end(), INVALID_BINDING_POINT);
        }
    };
    using ParamMap = std::unordered_map<std::string, Param>;

    // no predefine
    // fixed-shader
    // params
    // vertex attributes
    // render state
    // pipeline state
    Technique(Context* context, VirtualTechnique* virtualTechnique)
        : m_pContext(context)
        , m_pVirtualTechnique(virtualTechnique)
    { }

    ~Technique() {}
    
    VirtualTechnique* GetVirtualTechnique() const
    {
        return m_pVirtualTechnique;
    }

    void SetName(const std::string& name)
    {
        m_techName = name;
    }

    const std::string& GetName() const
    {
        return m_techName;
    }

    void SetShaderResource(ShaderType shaderType, const ShaderResourcePtr& shaderRes)
    {
        // TODO: check shaderType is valid
        m_shaderRes[to_underlying(shaderType)] = shaderRes;
    }

    const ShaderResourcePtr& GetShaderResource(ShaderType shaderType) const
    {
        // TODO: check shaderType is valid
        return m_shaderRes[to_underlying(shaderType)];
    }

    void SetRenderStateDesc(const RenderStateDesc& renderStateDesc)
    {
        m_RenderStateDesc = renderStateDesc;
    }

    RenderStateDesc& GetRenderStateDesc()
    {
        return m_RenderStateDesc;
    }

    RHIRenderStatePtr& GetRenderState();

    const ParamMap& GetFormalParams() const
    {
        return m_params;
    }

    RHIProgram* GetProgram()
    {
        return m_pProgram.get();
    }

    template<typename T>
    void SetParam(const std::string& name, const T& value)
    {
        auto paramIt = m_params.find(name);
        if (paramIt == m_params.end())
            return;

        *(paramIt->second.variable) = value;
    }

    bool HasParam(const std::string& name)
    {
        return m_params.find(name) != m_params.end();
    }

    SResult Render(const RHIMeshPtr& mesh);
    void Dispatch(uint32_t x, uint32_t y, uint32_t z);
    void DispatchIndirect(RHIGpuBufferPtr indirectBuf);
    void DrawIndirect(RHIGpuBufferPtr indirectBuf, MeshTopologyType type);
    void DrawInstanced(MeshTopologyType type, uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation);

    SResult Commit();
    void Uncommit();
    
private:
    SResult Build();
    void CreateEffectVariable(Param& param);

private:
    Context* m_pContext = nullptr;
    VirtualTechnique* m_pVirtualTechnique;
    std::string m_techName;
    RenderStateDesc m_RenderStateDesc;
    RHIRenderStatePtr m_RenderState;
    RHIRenderStatePtr m_RenderStateForTransparent;

    ParamMap m_params; // all params of this technique that user can set
    std::array<std::vector<size_t>, SHADER_STAGE_COUNT> m_shaderParamsIndex;

    std::array<ShaderResourcePtr, SHADER_STAGE_COUNT> m_shaderRes; // shader names which is actived
    RHIProgramPtr m_pProgram{ nullptr };

    bool m_bOpenGLAlreadyRemapBinding = false;
    
private:
    friend class VirtualTechnique;
};

SEEK_NAMESPACE_END
