#pragma once

#include "kernel/context.h"
#include "effect/scene_renderer.h"


SEEK_NAMESPACE_BEGIN


/******************************************************************************
 * GlobalIllumination
 ******************************************************************************/
class GlobalIllumination
{
public:
    GlobalIllumination(Context* context);

    virtual SResult         Init();
    virtual SResult         OnBegin() { return S_Success; }

    RHITexturePtr           GetIndirectIlluminationTex() { return m_pIndirectIlluminationTex; }

protected:
    Context*                m_pContext      = nullptr;
    GlobalIlluminationMode  m_eMode         = GlobalIlluminationMode::None;
    RHITexturePtr           m_pIndirectIlluminationTex = nullptr;
    RHIFrameBufferPtr       m_pIndirectIlluminationFb = nullptr;
};


/******************************************************************************
 * RSM : Reflective Shadow Map
 ******************************************************************************/
class RSM : public GlobalIllumination
{
public:
    RSM(Context* context);
    virtual SResult         OnBegin() override;
    virtual SResult         Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth);
    
    RendererReturnValue         GenerateReflectiveShadowMapJob(uint32_t light_index);
    virtual RendererReturnValue PostProcessReflectiveShadowMapJob(uint32_t light_index);

    void                    SetSampleRadius(float v) { m_fSampleRadius = v; }
    float                   GetSampleRadius() { return m_fSampleRadius; }

protected:
    SResult                 InitGenRsm();

protected:
    // for RSM & LPV & VXGI
    RHITexturePtr           m_pRsmTexs[3] = { nullptr };    // 0:Normal     1:Position    2:Flux
    RHITexturePtr           m_pRsmDepthTex = nullptr;
    RHIFrameBufferPtr       m_pGenRsmFb = nullptr;
   
private:
    PostProcessPtr          m_pGiRsmPp = nullptr;
	RHIRenderBufferPtr      m_pRsmParamCBuffer = nullptr;
    RHIRenderBufferPtr      m_pRsmInvProjCBuffer = nullptr;
    RHIRenderBufferPtr      m_pRsmCameraInfoCBuffer = nullptr;
    RHIRenderBufferPtr      m_pVplCoordAndWeightsCBuffer = nullptr;
    float                   m_fSampleRadius = 200;

private:
    std::vector<float4>     m_vVplCoordAndWeights;
};

/******************************************************************************
 * LPV : Light Propagation Volumes
 ******************************************************************************/
class LPV : public RSM
{
public:
    LPV(Context* context);
    virtual ~LPV();

    virtual SResult             OnBegin() override;
    virtual SResult             Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth);
    virtual RendererReturnValue PostProcessReflectiveShadowMapJob(uint32_t light_index) override;

private:
    SResult LPVInject();
    SResult LPVPropagation();
    SResult LPVCalcIndirect(uint32_t light_index);
    
    RHITexturePtr           m_pTexRedSh     = nullptr;
    RHITexturePtr           m_pTexGreenSh   = nullptr;
    RHITexturePtr           m_pTexBlueSh    = nullptr;
    std::vector<RHIFrameBufferPtr>       m_pFbInject = {};
    RHIRenderBufferPtr      m_pRTIndexCBuffer = nullptr;

    RHITexturePtr           m_pTexAccuRedSh = nullptr;
    RHITexturePtr           m_pTexAccuGreenSh = nullptr;
    RHITexturePtr           m_pTexAccuBlueSh = nullptr;
    RHITexturePtr           m_pTexRedSh_Bak = nullptr;
    RHITexturePtr           m_pTexGreenSh_Bak = nullptr;
    RHITexturePtr           m_pTexBlueSh_Bak = nullptr;
    std::vector<RHIFrameBufferPtr>       m_pFbPropagation = {};

    RenderStateDesc         m_PropagationRsDesc;
    RHIRenderBufferPtr      m_pInjectVerticsCBuffer = nullptr;    

    PostProcessPtr          m_pGiLpvPp = nullptr;
    RHIRenderBufferPtr      m_pLpvInvProjCBuffer = nullptr;
    RHIRenderBufferPtr      m_pLpvParamCBuffer = nullptr;
    RHIRenderBufferPtr      m_pLpvCameraInfoCBuffer = nullptr;

    uint32_t                m_iPropagationSteps = 25;
    float                   m_fLpvAttenuation = 1.0;
    float                   m_fLpvPower = 1.0;
    float                   m_fLpvCutoff = 0.2;

    static const std::string TechName_LPVInject;
    static const std::string TechName_LPVPropagation;
};


/******************************************************************************
 * VXGI : Voxel Cone Tracing 
 ******************************************************************************/
class VXGI : public GlobalIllumination
{
public:
    VXGI(Context* context);

    virtual SResult OnBegin() override;
    virtual SResult Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth);


private:
    RHITexturePtr   m_pTexVoxel3D = nullptr;
    RHITexturePtr   m_pTexVoxel3D_Copy = nullptr;



};

SEEK_NAMESPACE_END
