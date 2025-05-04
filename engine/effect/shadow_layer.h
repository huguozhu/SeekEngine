#pragma once

#include "kernel/kernel.h"
#include "kernel/context.h"
#include "utils/error.h"
#include "components/light_component.h"
#include "effect/scene_renderer.h"


SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * ShadowLayer
 ******************************************************************************/
class ShadowLayer
{
public:
    ShadowLayer(Context* context);
    ~ShadowLayer() {}
    virtual SResult InitResource();

    SResult AnalyzeLightShadow();

    RendererReturnValue GenerateShadowMapJob(uint32_t light_index);
    RendererReturnValue GenerateCascadedShadowMapJob(uint32_t light_index);
    virtual RendererReturnValue PostProcessShadowMapJob(uint32_t light_index) = 0;

    RHITexturePtr              GetFilteredShadowMap() { return m_pFilteredSmTex; }
    RHITexturePtr              GetCubeShadowMap() { return m_pCubeSmTex; }
    std::vector<RHITexturePtr> GetCascadedShadowMap() { return m_vCsmTex; }
    int32_t                 GetShadowMapIndexByLightIndex(size_t light_index);

    Effect*                 GetShadowEffect(uint32_t morph_count);

    RHITexturePtr const&       GetSmDepthTex() { return m_pSmDepthTex; }

    void                    SetCascadedIndex(uint32_t i) { m_iCascadedIndex = i; }
    uint32_t                GetCascadedIndex() { return m_iCascadedIndex; }

    static const uint32_t               MAX_CASCADED_SHADOW_NUM = 4;
    static const uint32_t               MAX_SHADOW_LIGHT_NUM = 4;
    static const uint32_t               MAX_CUBE_LIGHT_NUM = 1;
    static const uint32_t               SM_SIZE = 512;
    
protected:
    Context* m_pContext = nullptr;

    // Shadow Map(SM)
    RHITexturePtr                          m_pSmTex = nullptr;
    RHITexturePtr                          m_pSmDepthTex = nullptr;
    RHIFrameBufferPtr                      m_pSmFb = nullptr;
    RHITexturePtr                          m_pFilteredSmTex;

    // Cascaded Shadow Map(CSM)
    std::vector<RHITexturePtr>             m_vCsmTex;
    std::vector<RHITexturePtr>             m_vCsmDepthTex;
    std::vector<RHIFrameBufferPtr>         m_vCsmFb;

    // Cube Shadow Map
    RHITexturePtr                          m_pCubeSmTex = nullptr;
    RHITexturePtr                          m_pCubeSmDepthTex = nullptr;
    RHIFrameBufferPtr                      m_pCubeSmFb[(uint32_t)CubeFaceType::Num] = { nullptr };

    // State
    std::vector<std::pair<int32_t, int32_t>>  m_vShadowIndex;           // including: shadow_index(0~3) + filtered_map_index
    uint32_t                            m_iCascadedIndex = 0;

    // Shadow Effect
    std::map<uint32_t, EffectPtr>       m_ShadowEffects; // key=morph_count
};


/******************************************************************************
 * ForwardShadowLayer
 ******************************************************************************/
class ForwardShadowLayer : public ShadowLayer
{
public:
    ForwardShadowLayer(Context* context);
    ~ForwardShadowLayer() {}

    virtual SResult InitResource();
    virtual RendererReturnValue PostProcessShadowMapJob(uint32_t light_index);

private:
    // Shadow Map PostProcess
    PostProcessPtr                      m_pShadowCopy[MAX_SHADOW_LIGHT_NUM] = { nullptr };
};


/******************************************************************************
 * DeferredShadowLayer
 ******************************************************************************/
class DeferredShadowLayer : public ShadowLayer
{
private:
    // Shadow Map PostProcess
    Effect*                 m_pShadow0Effect = nullptr;
    Technique*              m_pShadowingTechs[(uint32_t)LightType::Num][2] = { nullptr };
    RHIRenderStatePtr       m_pShadowingRenderStates[4] = { nullptr };

    RendererReturnValue     m_pShadowingTex = nullptr;
    RHIFrameBufferPtr       m_pShadowingFb = nullptr;
    RHIMeshPtr              m_pQuadMesh = nullptr;

    // Effect Params
    EffectParam*            m_pParamCameraInfo = nullptr;        
    EffectParam*            m_pParamLightInfo = nullptr;
    EffectParam*            m_pParamDeferredLightingInfo = nullptr;
    EffectParam*            m_pParamDepthTex = nullptr;
    EffectParam*            m_pParamShadowTex = nullptr;
    EffectParam*            m_pParamCubeShadowTex = nullptr;
    EffectParam*            m_pParamCsmTex[NUM_CSM_LEVELS] = { nullptr };
    EffectParam*            m_pParamCsmDistance = nullptr;
    EffectParam*            m_pParamCsmLightVPMatrices = nullptr;
    EffectParam*            m_pParamLightViewMatrices = nullptr;

public:
    DeferredShadowLayer(Context* context);
    ~DeferredShadowLayer() {}

    virtual SResult InitResource();
    virtual RendererReturnValue PostProcessShadowMapJob(uint32_t light_index);

    RHITexturePtr           GetShadowingTex() { return m_pShadowingTex; }
};

SEEK_NAMESPACE_END
