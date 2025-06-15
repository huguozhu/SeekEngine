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

protected:
    Context*                m_pContext      = nullptr;
    GlobalIlluminationMode  m_eMode         = GlobalIlluminationMode::None;
    RHITexturePtr           m_pGITexture    = nullptr;
};


/******************************************************************************
 * RSM : Reflective Shadow Map
 ******************************************************************************/
class RSM : public GlobalIllumination
{
public:
    RSM(Context* context);
    virtual SResult         Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth);
    virtual SResult         OnBegin() override;
    
    RendererReturnValue     GenerateReflectiveShadowMapJob(uint32_t light_index);
    RendererReturnValue     PostProcessReflectiveShadowMapJob(uint32_t light_index);

    void                    SetSampleRadius(float v) { m_fSampleRadius = v; }
    float                   GetSampleRadius() { return m_fSampleRadius; }

    void                    FillVPLSampleCooordsAndWeights();
    
protected:
    // RSM & GI
    RHITexturePtr           m_pRSMTexs[3] = { nullptr };    // 0:Normal     1:Position    2:Flux
    RHITexturePtr           m_pRSMDepthTex = nullptr;
    RHIFrameBufferPtr       m_pRSMFb = nullptr;

    RHITexturePtr           m_pIndirecIlluminationTex = nullptr;
    RHIFrameBufferPtr       m_pIndirecIlluminationFb = nullptr;

    PostProcessPtr          m_pGiRsmPp = nullptr;
	RHIRenderBufferPtr      m_pRsmParamCBuffer = nullptr;
    RHIRenderBufferPtr      m_pRsmCameraInfoCBuffer = nullptr;
    RHIRenderBufferPtr      m_pVplCoordAndWeightsCBuffer = nullptr;

    float                   m_fSampleRadius = 25;

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
