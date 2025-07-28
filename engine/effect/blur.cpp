#include "effect/blur.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "effect/effect.h"
#include "effect/postprocess.h"

#define SEEK_MACRO_FILE_UID 89     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#include "shader/shared/Blur.h"

/******************************************************************************
 * GaussianBlur
 ******************************************************************************/
GaussianBlur::GaussianBlur(Context* context)
    :PostProcessChain(context, "GaussianBlur")
{
	m_pBlurXCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(BlurGlobalParams), RESOURCE_FLAG_CPU_WRITE);
    m_pBlurYCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(BlurGlobalParams), RESOURCE_FLAG_CPU_WRITE);
    BlurGlobalParams global_params;
	global_params.is_horizontal = 1;
	m_pBlurXCBuffer->Update(&global_params, sizeof(global_params));
    global_params.is_horizontal = 0;
    m_pBlurYCBuffer->Update(&global_params, sizeof(global_params));

	static const std::string tech_name = "GaussianBlur";
    Effect& effect = m_pContext->EffectInstance();
    effect.LoadTechnique(tech_name, &RenderStateDesc::PostProcess(), "PostProcessVS", "BlurPS", nullptr);

    PostProcessPtr blur_x = MakeSharedPtr<PostProcess>(context, "Blur_X");
    blur_x->Init(tech_name, NULL_PREDEFINES);
    blur_x->SetParam("GlobalParams", m_pBlurXCBuffer);

    PostProcessPtr blur_y = MakeSharedPtr<PostProcess>(context, "Blur_Y");
    blur_y->Init(tech_name, NULL_PREDEFINES);
    blur_y->SetParam("GlobalParams", m_pBlurXCBuffer);

    this->AddPostProcess(blur_x);
    this->AddPostProcess(blur_y);
}
void GaussianBlur::SetSrcTexture(RHITexturePtr const& tex2d)
{
    m_vPPChain[0]->SetParam("src_tex", tex2d);
    if (!m_pTemp || 
        m_pTemp->Width()  != tex2d->Width()  ||
        m_pTemp->Height() != tex2d->Height() ||
        m_pTemp->Format() != tex2d->Format()  )
    {
        RHITexture::Desc desc = {};
        desc.type = TextureType::Tex2D;
        desc.width = tex2d->Width();
        desc.height = tex2d->Height();
        desc.format = tex2d->Format();
        desc.flags = RESOURCE_FLAG_GPU_WRITE | RESOURCE_FLAG_GPU_READ;
        m_pTemp = m_pContext->RHIContextInstance().CreateTexture2D(desc);
    }
    m_vPPChain[0]->SetOutput(0, m_pTemp);
    m_vPPChain[1]->SetParam("src_tex", m_pTemp);
}
void GaussianBlur::SetDstTexture(RHITexturePtr const& tex)
{
    m_vPPChain[1]->SetOutput(0, tex);
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
