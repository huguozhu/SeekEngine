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
    void MakeVulkanRHIContext(Context* context, RHIContextPtrUnique& out);
}

Context::Context()
{

}
Context::~Context()
{

}

SResult Context::Init(const RenderInitInfo& init_info)
{
    if (!m_pThreadManager)
    {
        m_pThreadManager = MakeUniquePtrMacro(ThreadManager, this);
        m_pThreadManager->Init();
    }
    if (!m_pRHIContext)
    {
        MakeD3D11RHIContext(this, m_pRHIContext); 
    }
    if (!m_pSceneManager)
    {
        m_pSceneManager = MakeUniquePtrMacro(SceneManager, this);
    }
    if (!m_pResourceManager)
    {
        m_pResourceManager = MakeUniquePtrMacro(ResourceManager, this);
    }
    if (!m_pRendererCommandManager)
    {
        m_pRendererCommandManager = MakeUniquePtrMacro(RendererCommandManager, this);
        m_pRendererCommandManager->InitRendererInit(init_info.native_wnd);
    }
    if (!m_pSceneRenderer)
    {
        {
            m_pSceneRenderer = MakeUniquePtrMacro(ForwardShadingRenderer, this);
        }
    }

    if (!m_pEffect)
    {
        m_pEffect = MakeUniquePtr<Effect>(this);
        SResult ret = m_pEffect->Initialize();
        if (SEEK_CHECKFAILED(ret))
        {
            m_pSceneRenderer.reset();
            return ret;
        }
    }

    m_MainThreadSemaphore.Post();

    return S_Success;
}
void Context::Uninit()
{
    m_pThreadManager.reset();
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
    m_dCurTime = m_pTimer->CurrentTimeSinceEpoch_S();
    m_dDeltaTime = m_dCurTime - last_time;

    return SceneManagerInstance().Tick((float)m_dDeltaTime);
}

SResult Context::BeginRender()
{
    LOG_RECORD_FUNCTION();
    this->RendererCommandManagerInstance().FinishSubmitCommandBuffer();
    this->RendererCommandManagerInstance().SwapCommandBuffer();

    if (m_bViewportChanged)
    {
        m_pSceneRenderer->SetViewport(m_sViewport);
        m_bViewportChanged = false;
    }
    this->RenderThreadSemPost();
    
    if (1)
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

    // TODO: merge this two renderer
    if (sr_scene.HasRenderJob())// && sr_sprite.HasRenderJob())
    {
        LOG_ERROR("SceneRenderer and Sprite2DRenderer is mutually exclusive");
        return ERR_INVALID_INVOKE_FLOW;
    }

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
    LOG_RECORD_FUNCTION();
    this->MainThreadSemWait();
    RHIFrameBufferPtr final_fb = this->RHIContextInstance().GetFinalRHIFrameBuffer();
    if (final_fb)
    {
        SEEK_RETIF_FAIL(final_fb->SwapBuffers());
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