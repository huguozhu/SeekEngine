#include "kernel/context.h"

#include "thread/thread.h"
#include "thread/thread_manager.h"

#include "rhi/base/rhi_context.h"

#include "effect/effect.h"
#include "effect/scene_renderer.h"
#include "effect/forward_shading_renderer.h"

#include "utils/log.h"
#include "utils/timer.h"
#include "utils/error.h"

#define SEEK_MACRO_FILE_UID 28     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

extern "C"
{
    void MakeD3D11RHIContext(Context* context, RHIContextPtrUnique& out);
    void MakeD3D12RHIContext(Context* context, RHIContextPtrUnique& out);
    void MakeVulkanRHIContext(Context* context, RHIContextPtrUnique& out);
}

Context::Context()
{
    m_InitInfo.multi_thread = false;
    m_InitInfo.rhi_type = RHIType::D3D12;
}
Context::~Context()
{
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
SResult Context::Init(const RenderInitInfo& init_info)
{    
    SResult ret = S_Success;
    do
    {
        if (!m_pThreadManager && m_InitInfo.multi_thread)
        {
            m_pThreadManager = MakeUniquePtrMacro(ThreadManager, this);
            ret = m_pThreadManager->Init();
            if (SEEK_CHECKFAILED(ret))
                break;
        }
        if (!m_pRendererCommandManager)
        {
            m_pRendererCommandManager = MakeUniquePtrMacro(RendererCommandManager, this);
            //m_pRendererCommandManager->InitRenderer(init_info.native_wnd);
        }

        ret = this->InitRHIContext();
		{
			// to delete after testing
	        RHIContext& rc = this->RHIContextInstance();
	        rc.Init();
	        if (init_info.native_wnd)
	        {
	            rc.AttachNativeWindow("", init_info.native_wnd);
	            rc.SetFinalRHIFrameBuffer(rc.GetScreenRHIFrameBuffer());
	        }
		}
        if (SEEK_CHECKFAILED(ret))
            break;
        if (!m_pResourceManager)
        {
            m_pResourceManager = MakeUniquePtrMacro(ResourceManager, this);
        }
        if (!m_pSceneManager)
        {
            m_pSceneManager = MakeUniquePtrMacro(SceneManager, this);
        }
        
        
        if (!m_pSceneRenderer)
        {
            m_pSceneRenderer = MakeUniquePtrMacro(ForwardShadingRenderer, this);
            ret = m_pSceneRenderer->Init();
            if (SEEK_CHECKFAILED(ret))
                break;
        }
        if (!m_pEffect)
        {
            m_pEffect = MakeUniquePtr<Effect>(this);
            m_pRendererCommandManager->InitEffect(m_pEffect.get());
        }
        return S_Success;
    } while (0);

    this->Uninit();
    return ret;
}
void Context::Uninit()
{
    m_pThreadManager.reset();
    m_pRHIContext.reset();
    m_pSceneManager.reset();
    m_pSceneRenderer.reset();
    m_pResourceManager.reset();
    m_pRendererCommandManager.reset();
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

SResult Context::Update()
{
    double last_time = m_dCurTime;
    float cur_time = m_pTimer->CurrentTimeSinceEpoch_S();
    float time_interval = cur_time - last_time;
    float  limit_time = 0.0;
    if (m_InitInfo.fps_limit_type == FPSLimit::FPS_30) limit_time = 0.033f;
    if (m_InitInfo.fps_limit_type == FPSLimit::FPS_60) limit_time = 0.0167f;
    if (time_interval < limit_time)
        return S_Success;

    m_dCurTime = cur_time;
    m_dDeltaTime = m_dCurTime - last_time;


    SEEK_RETIF_FAIL(this->BeginRender());
	SEEK_RETIF_FAIL(SceneManagerInstance().Tick((float)m_dDeltaTime));
    if (m_InitInfo.multi_thread == false)
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

    if (m_InitInfo.multi_thread)
    {
        this->RendererCommandManagerInstance().FinishSubmitCommandBuffer();
        this->RendererCommandManagerInstance().SwapCommandBuffer();
        this->RenderThreadSemPost();
    }
    if (0)
    {
        static uint32_t MainThread_Index = 0;
        double var = Timer::CurrentTimeSinceEpoch_S();
        LOG_INFO("MainThread Index:  %6d:  time = %20f", MainThread_Index++, var);
        ::_sleep(10);
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
    if (m_InitInfo.multi_thread)
        this->MainThreadSemWait();
    else
    {
        RHIFrameBufferPtr final_fb = this->RHIContextInstance().GetFinalRHIFrameBuffer();
        if (final_fb)
        {
            SEEK_RETIF_FAIL(final_fb->SwapBuffers());
        }
    }
    return S_Success;
}
void Context::MainThreadSemWait()
{
    if (m_InitInfo.multi_thread)
    {
        m_MainThreadSemaphore.Wait();
    }
}
void Context::MainThreadSemPost()
{
    if (m_InitInfo.multi_thread)
    {
        m_MainThreadSemaphore.Post();
    }
}
void Context::RenderThreadSemWait()
{
    if (m_InitInfo.multi_thread)
    {
        m_pThreadManager->GetRenderThread()->GetSemaphore().Wait();
    }
}
void Context::RenderThreadSemPost()
{
    if (m_InitInfo.multi_thread)
    {
        m_pThreadManager->GetRenderThread()->GetSemaphore().Post();
    }
}
SEEK_NAMESPACE_END