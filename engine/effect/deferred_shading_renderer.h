#include "effect/scene_renderer.h"
#include "components/light_component.h"
#include "components/mesh_component.h"
#include "effect/gi.h"

SEEK_NAMESPACE_BEGIN

class DeferredShadingRenderer : public SceneRenderer
{
public:
    virtual ~DeferredShadingRenderer() {}

    virtual SResult             Init() override;
    virtual SResult             BuildRenderJobList() override;
            void                AppendShadowMapJobs(uint32_t light_index);
    virtual SResult             GetEffectTechniqueToRender(RHIMeshPtr mesh, Technique** tech) override;
            void                AppendGIJobs(uint32_t light_index);

    virtual bool                IsNeedShaderInvariant(RenderStage stage) override;
    RHIMeshPtr                  GetLightVolumeMesh(LightType type);

    RendererReturnValue         RenderPrepareJob();
    RendererReturnValue         RenderPreZJob();
    RendererReturnValue         GenerateGBufferJob();
    RendererReturnValue         SSAOJob();
    RendererReturnValue         LightingTileCullingJob();
    RendererReturnValue         LightingJob();
    RendererReturnValue         RenderSkyBoxJob();
    RendererReturnValue         RenderParticlesJob();
    RendererReturnValue         HDRJob();
    RendererReturnValue         LDRJob();
    RendererReturnValue         PrintTimeQueryJob();

    GlobalIlluminationPtr const&    GetGI() { return m_pGI; }

protected:
    friend class Context;
    DeferredShadingRenderer(Context* context);

    // RHIMesh & RHIRenderBuffers
    RHIMeshPtr              m_pQuadMesh = nullptr;
    RHIRenderBufferPtr      m_pCameraInfoCBuffer = nullptr;
    RHIRenderBufferPtr      m_pSsaoSampleKernelCBuffer = nullptr;
    RHIRenderBufferPtr      m_pSsaoParamCBuffer = nullptr;
    RHIRenderBufferPtr      m_pLightInfoCBuffer = nullptr;
    RHIRenderBufferPtr      m_pTileInfoBuffer = nullptr;
	RHIRenderBufferPtr      m_pLightCullingInfoCBuffer = nullptr;
	RHIRenderBufferPtr      m_pDeferredLightingInfoCBuffer = nullptr;


    // Textures
    RHITexturePtr           m_pSceneDepthStencil = nullptr;
    RHITexturePtr           m_pSceneDepthCopy = nullptr;
    RHITexturePtr           m_pSceneVelocity = nullptr; // For TAA
    RHITexturePtr           m_pSceneColor = nullptr;
    RHITexturePtr           m_pLDRColor = nullptr;
    RHITexturePtr           m_pHDRColor = nullptr;

    RHITexturePtr           m_pPreZColor = nullptr;
    RHITexturePtr           m_pGBufferColor0 = nullptr;
    RHITexturePtr           m_pGBufferColor1 = nullptr;
    RHITexturePtr           m_pGBufferColor2 = nullptr;

    RHITexturePtr           m_pShadowTex = nullptr;
    RHITexturePtr           m_pSsaoColor = nullptr;
    RHITexturePtr           m_pSsaoNoise = nullptr;

    
	// RHIFrameBuffers
    RHIFrameBufferPtr       m_pPreZFb = nullptr;
    RHIFrameBufferPtr       m_pGBufferFb = nullptr;
    RHIFrameBufferPtr       m_pLightingFb = nullptr;
    RHIFrameBufferPtr       m_pLDRFb = nullptr;
    RHIFrameBufferPtr       m_pHDRFb = nullptr;
    RHIFrameBufferPtr       m_pSceneFb = nullptr;
    RHIFrameBufferPtr       m_pShadowingFb = nullptr;
    RHIFrameBufferPtr       m_pSsaoFb = nullptr;

    // Techniques
    Technique*              m_pLightingTech_HasShadow = nullptr;
    Technique*              m_pLightingTech_NoShadow = nullptr;
    Technique*              m_pSsaoTech = nullptr;
    Technique*              m_pTileCullingTech = nullptr;
    Technique*              m_pLightingTech_HasShadow_TileCulling = nullptr;

    // PostProcess
    HDRPostProcessPtr       m_pHDRPostProcess = nullptr;
    LDRPostProcessPtr       m_pLDRPostProcess = nullptr;

    GaussianBlurPtr         m_pGaussianBlur = nullptr;
    GlobalIlluminationPtr   m_pGI = nullptr;

    // Query
    RHITimeQueryPtr         m_pTimeQueryGenShadowMap = nullptr;
    RHITimeQueryPtr         m_pTimeQueryGenGBuffer = nullptr;
    RHITimeQueryPtr         m_pTimeQuerySSAO = nullptr;
    RHITimeQueryPtr         m_pTimeQueryLihgtCulling = nullptr;
    RHITimeQueryPtr         m_pTimeQueryLighting = nullptr;
    RHITimeQueryPtr         m_pTimeQueryGI = nullptr;
    RHITimeQueryPtr         m_pTimeQuerySkybox = nullptr;
    RHITimeQueryPtr         m_pTimeQueryHDR = nullptr;
    RHITimeQueryPtr         m_pTimeQueryLDR = nullptr;

    static LightInfo        s_LightInfos[MAX_DEFERRED_LIGHTS_NUM];
    uint32_t                m_iRenderWidth = 0;
	uint32_t                m_iRenderHeight = 0;
};



SEEK_NAMESPACE_END
