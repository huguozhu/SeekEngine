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
    
    RendererReturnValue     GenerateReflectiveShadowMapJob(uint32_t light_index);
    RendererReturnValue     PostProcessReflectiveShadowMapJob(uint32_t light_index);

    void                    SetSampleRadius(float v) { m_fSampleRadius = v; }
    float                   GetSampleRadius() { return m_fSampleRadius; }

protected:
    SResult                 InitGenRsm();

protected:
    // for RSM & LPV
    RHITexturePtr           m_pRsmTexs[3] = { nullptr };    // 0:Normal     1:Position    2:Flux
    RHITexturePtr           m_pRsmDepthTex = nullptr;
    RHIFrameBufferPtr       m_pGenRsmFb = nullptr;
   

private:
    PostProcessPtr          m_pGiRsmPp = nullptr;
	RHIRenderBufferPtr      m_pRsmParamCBuffer = nullptr;
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

    virtual SResult         OnBegin() override;
    virtual SResult         Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth);

    SResult LPVInject();
    SResult LPVPropagation();
    
    RHITexturePtr           m_pTexRedSh;
    RHITexturePtr           m_pTexGreenSh;
    RHITexturePtr           m_pTexBlueSh;
    RHIFrameBufferPtr       m_pFbSH;

private:

};


/******************************************************************************
 * VXGI : Voxel Cone Tracing
 ******************************************************************************/
class VXGI : public GlobalIllumination
{
public:
    VXGI(Context* context);

private:

};

SEEK_NAMESPACE_END
