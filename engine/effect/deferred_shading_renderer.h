#include "effect/scene_renderer.h"
#include "components/light_component.h"
#include "components/mesh_component.h"
//#include "effect/gi.h"

SEEK_NAMESPACE_BEGIN

class DeferredShadingRenderer : public SceneRenderer
{
public:
    virtual ~DeferredShadingRenderer() {}

    virtual SResult             Init() override;
    virtual SResult             BuildRenderJobList() override;
            void                AppendShadowMapJobs(uint32_t light_index);
    virtual SResult             GetEffectTechniqueToRender(MeshPtr mesh, Effect** effect, Technique** tech) override;
    virtual void                AppendGIJobs(uint32_t light_index) override;

    virtual bool                IsNeedShaderInvariant(RenderStage stage) override;
    Effect*                     GetDeferredEffect(uint32_t morph_count);
    RHIMeshPtr                  GetLightVolumeMesh(LightType type);
    Technique*                  GetLightingTechByLightType(LightType type);

    RendererReturnValue         RenderPrepareJob();
    RendererReturnValue         RenderPreZJob();
    RendererReturnValue         GenerateGBufferJob();
    RendererReturnValue         SSAOJob();
    RendererReturnValue         LightingTileCullingJob();
    RendererReturnValue         LightingJob();
    RendererReturnValue         PrintTimeQueryJob();

    //GlobalIlluminationPtr const&    GetGI() { return m_pGI; }

protected:
    friend class Context;
    DeferredShadingRenderer(Context* context);

    RHIMeshPtr                      m_pQuadMesh = nullptr;

    // 3D Scene FrameBuffer
    RHITexturePtr                   m_pSceneColor = nullptr;
    RHITexturePtr                   m_pSceneVelocity = nullptr; // For TAA
    RHITexturePtr                   m_pSceneDepthStencil = nullptr;
    RHIFrameBufferPtr               m_pSceneFb = nullptr;

    RHITexturePtr                   m_pLDRColor = nullptr;
    RHIFrameBufferPtr               m_pLDRFb = nullptr;

    RHITexturePtr                   m_pHDRColor = nullptr;
    RHIFrameBufferPtr               m_pHDRFb = nullptr;

    // HDR
    HDRPostProcessPtr               m_pHDRPostProcess = nullptr;
    LDRPostProcessPtr               m_pLDRPostProcess = nullptr;


    EffectParam*                    m_pParamCameraInfo = nullptr;
    EffectParam*                    m_pParamGBuffer0 = nullptr;
    EffectParam*                    m_pParamGBuffer1 = nullptr;
    EffectParam*                    m_pParamDepthTex = nullptr;
    EffectParam*                    m_pParamShadowingTex = nullptr;
    EffectParam*                    m_pParamDeferredInfo = nullptr;
    EffectParam*                    m_pParamLightInfos = nullptr;

    // For Tile Culling Lighting
    EffectParam*                    m_pParamViewMatrix = nullptr;
    EffectParam*                    m_pParamProjMatrix = nullptr;
    EffectParam*                    m_pParamFrameSize = nullptr;
    EffectParam*                    m_pParamTileInfoRW = nullptr;
    EffectParam*                    m_pParamTileInfo = nullptr;

    // FrameBuffer / Texture
    RHITexturePtr                   m_pGBufferColor0 = nullptr;
    RHITexturePtr                   m_pGBufferColor1 = nullptr;
    RHITexturePtr                   m_pGBufferColor2 = nullptr;

    RHITexturePtr                   m_pPreZColor = nullptr;
    RHITexturePtr                   m_pSsaoColor = nullptr;
    
    RHIFrameBufferPtr               m_pGBufferFb = nullptr;
    RHIFrameBufferPtr               m_pPreZFb = nullptr;
    RHIFrameBufferPtr               m_pLightingFb = nullptr;
    RHIFrameBufferPtr               m_pSsaoFb = nullptr;

    std::map<uint32_t, EffectPtr>   m_Effects; // key=morph_count
    Technique*                      m_pLightingTech_HasShadow =  nullptr;
    Technique*                      m_pLightingTech_NoShadow = nullptr;

    // Shadowing
    RHITexturePtr                      m_pShadowTex = nullptr;
    RHIFrameBufferPtr                  m_pShadowingFb = nullptr;

    // SSAO
    RHITexturePtr                      m_pSsaoNoise = nullptr;
    EffectPtr                       m_pSsaoEffect = nullptr;
    Technique*                      m_pSsaoTech = nullptr;
    //GaussianBlurPtr                 m_pGaussianBlur = nullptr;
    EffectParam*                    m_pParamSsaoCameraInfo = nullptr;
    EffectParam*                    m_pParamSsaoViewMatrix = nullptr;
    EffectParam*                    m_pParamSsaoProjMatrix = nullptr;
    EffectParam*                    m_pParamSsaoInvProjMatrix = nullptr;

    
    //GlobalIlluminationPtr           m_pGI = nullptr;

    // Query
    TimeQueryPtr                    m_pTimeQueryGenShadowMap = nullptr;
    TimeQueryPtr                    m_pTimeQueryGenGBuffer = nullptr;
    TimeQueryPtr                    m_pTimeQuerySSAO = nullptr;
    TimeQueryPtr                    m_pTimeQueryLihgtCulling = nullptr;
    TimeQueryPtr                    m_pTimeQueryLighting = nullptr;
    TimeQueryPtr                    m_pTimeQueryGI = nullptr;
    TimeQueryPtr                    m_pTimeQuerySkybox = nullptr;
    TimeQueryPtr                    m_pTimeQueryHDR = nullptr;
    TimeQueryPtr                    m_pTimeQueryLDR = nullptr;

    // Tile Culling
    RHIRenderBufferPtr                 m_pLightInfoBuffer = nullptr;
    RHIRenderBufferPtr                 m_pTileInfoBuffer = nullptr;
    Technique*                      m_pTileCullingTech = nullptr;
    Technique*                      m_pLightingTech_Shadow_TileCulling = nullptr;


    static LightInfo                s_LightInfos[MAX_DEFERRED_LIGHTS_NUM];
};



SEEK_NAMESPACE_END
