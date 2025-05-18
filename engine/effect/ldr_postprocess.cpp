#include "effect/ldr_postprocess.h"
#include "effect/taa.h"
#include "effect/effect.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/base/rhi_render_view.h"

SEEK_NAMESPACE_BEGIN
static const std::string tech_name_none = "Copy";
static const std::string tech_name_taa = "TAA";
static const std::string tech_name_fxaa = "FXAA";

LDRPostProcess::LDRPostProcess(Context* context)
    :PostProcess(context, "LDR_PostProcess")
{
    this->Init();
}

SResult LDRPostProcess::Init()
{
    SResult ret = S_Success;
    AntiAliasingMode aa_mode = m_pContext->GetAntiAliasingMode();
    Effect& effect = m_pContext->EffectInstance();
    if (aa_mode == AntiAliasingMode::None)
    {
        effect.LoadTechnique(tech_name_none, &RenderStateDesc::PostProcess(), "PostProcessVS", "CopyPS", nullptr);
        m_pCopyPostProcess = MakeSharedPtr<PostProcess>(m_pContext, tech_name_none);
        m_pCopyPostProcess->Init(tech_name_none, NULL_PREDEFINES);
    }
    else if (aa_mode == AntiAliasingMode::TAA)
    {
        m_pTaaPostProcess = MakeSharedPtr<TaaPostProcess>(m_pContext);
        ret = m_pTaaPostProcess->Init();
        if (SEEK_CHECKFAILED(ret))
        {
            LOG_ERROR_PRIERR(ret, "TaaPostProcess::Init fail.");
            m_pTaaPostProcess.reset();
            return ret;
        }
    }
    else if (aa_mode == AntiAliasingMode::FXAA)
    {
        m_pContext->EffectInstance().LoadTechnique(tech_name_fxaa, &RenderStateDesc::PostProcess(), "PostProcessVS", "FxaaPS", nullptr);
        m_pFxaaPostProcess = MakeSharedPtr<PostProcess>(m_pContext, tech_name_fxaa);
        ret = m_pFxaaPostProcess->Init(tech_name_fxaa, NULL_PREDEFINES);
        if (SEEK_CHECKFAILED(ret))
        {
            LOG_ERROR_PRIERR(ret, "FxaaPostProcess::Init fail.");
            m_pFxaaPostProcess.reset();
            return ret;
        }
    }
    return S_Success;
}

void LDRPostProcess::SetLDRTexture(RHITexturePtr const& tex2d)
{
    AntiAliasingMode aa_mode = m_pContext->GetAntiAliasingMode();
    if (aa_mode == AntiAliasingMode::None)
        m_pCopyPostProcess->SetParam("src_tex", tex2d);
    else if (aa_mode == AntiAliasingMode::TAA)
        m_pTaaPostProcess->SetParam("currentTex", tex2d);
    else if (aa_mode == AntiAliasingMode::FXAA)
        m_pFxaaPostProcess->SetParam("currentTex", tex2d);
}
void LDRPostProcess::SetTaaSceneVelocityTexture(RHITexturePtr const& tex2d)
{
	if (m_pContext->GetAntiAliasingMode() == AntiAliasingMode::TAA)
		m_pTaaPostProcess->SetParam("velocityTex", tex2d);
}

SResult LDRPostProcess::Run()
{
    AntiAliasingMode aa_mode = m_pContext->GetAntiAliasingMode();
    if (aa_mode == AntiAliasingMode::None)
    {
        m_pCopyPostProcess->Run();
        //m_pContext->RenderContextInstance().CopyTexture(m_pCopyTextureSrc, m_pContext->RenderContextInstance().GetCurFrameBuffer()->GetRenderTarget(FrameBuffer::Color0)->Texture());
    }
    else if (aa_mode == AntiAliasingMode::TAA)
        m_pTaaPostProcess->Run();
    else if (aa_mode == AntiAliasingMode::FXAA)
        m_pFxaaPostProcess->Run();
    return S_Success;
}

SEEK_NAMESPACE_END
