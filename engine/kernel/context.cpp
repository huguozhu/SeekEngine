#include "kernel/context.h"

#include "thread/thread.h"
#include "thread/thread_manager.h"

#include "rhi/base/rhi_context.h"

#include "effect/effect.h"
#include "effect/scene_renderer.h"
#include "effect/forward_shading_renderer.h"
#include "effect/deferred_shading_renderer.h"

#include "utils/log.h"
#include "utils/timer.h"
#include "utils/error.h"
#include "math/color.h"

#define SEEK_MACRO_FILE_UID 28     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

extern "C" 
{
    void MakeD3D11RHIContext(Context* context, RHIContextPtrUnique& out);
    void MakeD3D12RHIContext(Context* context, RHIContextPtrUnique& out);
    void MakeVulkanRHIContext(Context* context, RHIContextPtrUnique& out);
}

#if defined(SEEK_PLATFORM_WINDOWS)
extern void OutputD3DCommonDebugInfo();
#endif

Context::Context()
{
    m_InitInfo.rhi_type = RHIType::D3D11;
    m_InitInfo.enable_debug = true;
    m_InitInfo.HDR = true;
    m_InitInfo.renderer_type = RendererType::Deferred;
    m_fClearColor = float4(1, 1, 0, 1);
}
Context::~Context()
{
#if defined(SEEK_PLATFORM_WINDOWS)
    OutputD3DCommonDebugInfo();
#endif
}

SResult Context::Init(const RenderInitInfo& init_info)
{    
    SResult ret = S_Success;
    do
    {
        m_InitInfo.renderer_type = init_info.renderer_type;
        m_InitInfo.gi_mode = init_info.gi_mode;
        m_InitInfo.anti_aliasing_mode = init_info.anti_aliasing_mode;

        // RHIContext
        {
            ret = this->InitRHIContext();
            if (SEEK_CHECKFAILED(ret))
                break;
            if (init_info.native_wnd)
            {
                RHIContext& rc = this->RHIContextInstance();
                SEEK_RETIF_FAIL(rc.AttachNativeWindow("", init_info.native_wnd));
                rc.SetFinalRHIFrameBuffer(rc.GetScreenRHIFrameBuffer());
                this->SetViewport(rc.GetScreenRHIFrameBuffer()->GetViewport());
            }
        }

        if (!m_pResourceManager)
        {
            m_pResourceManager = MakeUniquePtrMacro(ResourceManager, this);
        }
        if (!m_pSceneManager)
        {
            m_pSceneManager = MakeUniquePtrMacro(SceneManager, this);
        }
        
        if (!m_pEffect)
        {
            m_pEffect = MakeUniquePtr<Effect>(this);
            m_pEffect->Initialize();
        }
        
        if (!m_pSceneRenderer)
        {
            RendererType type = this->GetRendererType();
            if (type == RendererType::Forward)
                m_pSceneRenderer = MakeUniquePtrMacro(ForwardShadingRenderer, this);
            else if ( type == RendererType::Deferred)
                m_pSceneRenderer = MakeUniquePtrMacro(DeferredShadingRenderer, this);
            else
            {
                LOG_ERROR("Invalid Scene Renderer Type: %d", type); 
                break;
            }
            ret = m_pSceneRenderer->Init();
            if (SEEK_CHECKFAILED(ret))
                break;
        }
        this->SetFpsLimitType(init_info.fps_limit_type);
        return S_Success;
    } while (0);

    this->Uninit();
    return ret;
}
void Context::Uninit()
{
    m_pRHIContext.reset();
    m_pSceneManager.reset();
    m_pSceneRenderer.reset();
    m_pResourceManager.reset();
    m_pEffect.reset();
}
void Context::SetViewport(Viewport vp)
{
    if (m_sViewport != vp)
    {
        LOG_INFO("viewport is changed, (%d, %d, %u, %u)->(%d, %d, %u, %u)",
            m_sViewport.left, m_sViewport.top, m_sViewport.width, m_sViewport.height,
            vp.left, vp.top, vp.width, vp.height);
        m_sViewport = vp;
        m_bViewportChanged = true;
    }
}
SResult Context::Tick()
{
    double last_time = m_dCurTime;
    float cur_time = m_pTimer->CurrentTimeSinceEpoch_S();
    float time_interval = cur_time - last_time;
    float  limit_time = 0.0;
    if (time_interval < m_fMinFrameTime)
        return S_Success;

    m_dCurTime = cur_time;
    m_dDeltaTime = m_dCurTime - last_time;

    SEEK_RETIF_FAIL(SceneManagerInstance().Tick((float)m_dDeltaTime));
    return S_Success;
}
SResult Context::Update()
{
    double last_time = m_dCurTime;
    float cur_time = m_pTimer->CurrentTimeSinceEpoch_S();
    float time_interval = cur_time - last_time;
    float  limit_time = 0.0;
    if (time_interval < m_fMinFrameTime)
        return S_Success;

    m_dCurTime = cur_time;
    m_dDeltaTime = m_dCurTime - last_time;

    SEEK_RETIF_FAIL(SceneManagerInstance().Tick((float)m_dDeltaTime));
    SEEK_RETIF_FAIL(this->BeginRender());
    SEEK_RETIF_FAIL(this->RenderFrame());
    SEEK_RETIF_FAIL(this->EndRender());
    return S_Success;
}

SResult Context::BeginRender()
{
    if (m_bViewportChanged)
    {
        m_pSceneRenderer->SetViewport(m_sViewport);
        m_bViewportChanged = false;
    }
    return S_Success;
}
/********** Called by Rendering Thread ***************/
SResult Context::RenderFrame()
{
    SResult ret = S_Success;
    RendererReturnValue rrv;
    SceneRenderer& sr_scene = this->SceneRendererInstance();

    sr_scene.BuildRenderJobList();

    if (sr_scene.HasRenderJob())
    {
        while (1)
        {
            rrv = sr_scene.DoRenderJob();
            if (rrv & RRV_Finish)
                break;
        }
    }

    m_FrameCount++;
    return S_Success;
}
SResult Context::EndRender()
{
    RHIFrameBufferPtr final_fb = this->RHIContextInstance().GetFinalRHIFrameBuffer();
    if (final_fb)
    {
        SEEK_RETIF_FAIL(final_fb->SwapBuffers());
    }
    return S_Success;
}
void Context::SetFpsLimitType(FPSLimitType b)
{
    m_InitInfo.fps_limit_type = b;
    switch (b)
    {
    case FPSLimitType::FPS_30:  m_fMinFrameTime = 0.0333f; break;
    case FPSLimitType::FPS_60:  m_fMinFrameTime = 0.0167f; break;
    case FPSLimitType::FPS_120: m_fMinFrameTime = 0.0083f; break;
    case FPSLimitType::NoLImit:
    default: m_fMinFrameTime = 0.0f;
    }
}

SResult Context::InitRHIContext()
{
    if (m_InitInfo.rhi_type == RHIType::D3D11)
        MakeD3D11RHIContext(this, m_pRHIContext);
    else if (m_InitInfo.rhi_type == RHIType::D3D12)
        MakeD3D12RHIContext(this, m_pRHIContext);
    else  if (m_InitInfo.rhi_type == RHIType::Vulkan)
        MakeVulkanRHIContext(this, m_pRHIContext);
    SResult ret = m_pRHIContext->Init();
    if (SEEK_CHECKFAILED(ret))
    {
        LOG_ERROR("m_pRHIContext->Init() error, ret:0X%X", ret);
        m_pRHIContext.reset();
        return ret;
    }

    CapabilitySet const& cap = m_pRHIContext->GetCapabilitySet();
    /// Set sampleCount
    uint32_t num_samples = Math::Min<uint32_t>(m_InitInfo.num_samples, CAP_MAX_TEXTURE_SAMPLE_COUNT);
    if (!cap.TextureSampleCountSupport[num_samples])
    {
        for (; num_samples > 0; num_samples--)
        {
            if (cap.TextureSampleCountSupport[num_samples]) break;
        }
        if (num_samples < 1) num_samples = 1;
        LOG_INFO("Not support TextureSampleCount %d, set to %d", m_InitInfo.num_samples, num_samples);
        m_InitInfo.num_samples = num_samples;
    }
    return S_Success;
}
SEEK_NAMESPACE_END