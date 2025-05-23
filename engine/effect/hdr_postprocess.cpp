#include "effect/hdr_postprocess.h"
#include "effect/effect.h"
#include "rhi/base/rhi_framebuffer.h"

SEEK_NAMESPACE_BEGIN
/******************************************************************************
 * HDRPostProcess
 ******************************************************************************/
HDRPostProcess::HDRPostProcess(Context* context)
    :PostProcess(context, "HDRPostProcess")
{
    static const std::string tech_name = "ToneMapping";
    Effect& effect = m_pContext->EffectInstance();
    effect.LoadTechnique(tech_name, &RenderStateDesc::PostProcess(), "PostProcessVS", "ToneMappingPS", nullptr);
	
    m_pToneMapping = MakeSharedPtr<PostProcess>(context, tech_name);
    m_pToneMapping->Init(tech_name, NULL_PREDEFINES);
}

void HDRPostProcess::SetSrcTexture(RHITexturePtr const& tex2d)
{
    m_pToneMapping->SetParam("src_rgba", tex2d);
}

SResult HDRPostProcess::Run()
{
    return m_pToneMapping->Run();
}

SEEK_NAMESPACE_END
