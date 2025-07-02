#include "effect/ibl.h"
#include "effect/effect.h"
#include "effect/parameter.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_framebuffer.h"
#include "math/math_utility.h"
#include "utils/buffer.h"

#define DVF_MACRO_FILE_UID 68     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#define IRRADIANCE_CONVOLUTION_MAP_SIZE 32
#define PREFILTER_MAP_SIZE              128
#define BRDF_LUT_MAP_SIZE               512

static Matrix4 p = Math::PerspectiveLH(90.0 * Math::DEG2RAD, 1.0, 0.1, 10.0);
static std::vector<Matrix4> mvp = {
    Math::LookAtLH(float3(0.0f, 0.0f, 0.0f), float3(1.0f,   0.0f,  0.0f),   float3(0.0f,  1.0f,  0.0)) * p,
    Math::LookAtLH(float3(0.0f, 0.0f, 0.0f), float3(-1.0f,  0.0f,  0.0f),   float3(0.0f,  1.0f,  0.0)) * p,
    Math::LookAtLH(float3(0.0f, 0.0f, 0.0f), float3(0.0f,   1.0f,  0.0f),   float3(0.0f,  0.0f,  1.0)) * p,
    Math::LookAtLH(float3(0.0f, 0.0f, 0.0f), float3(0.0f,  -1.0f,  0.0f),   float3(0.0f,  0.0f,  1.0)) * p,
    Math::LookAtLH(float3(0.0f, 0.0f, 0.0f), float3(0.0f,   0.0f,  1.0f),   float3(0.0f,  1.0f,  0.0)) * p,
    Math::LookAtLH(float3(0.0f, 0.0f, 0.0f), float3(0.0f,   0.0f, -1.0f),   float3(0.0f,  1.0f,  0.0)) * p,
};

/******************************************************************************
 * EquirectangularToCubeMapPostProcess
 ******************************************************************************/
EquirectangularToCubeMapPostProcess::EquirectangularToCubeMapPostProcess(Context* context)
    :PostProcessChain(context, "EquirectangularToCubeMap")
{
    static std::vector<std::string> names = {
        "EquirectangularToCubeMap0_PositiveX",
        "EquirectangularToCubeMap1_NegativeX",
        "EquirectangularToCubeMap2_PositiveY",
        "EquirectangularToCubeMap3_NegativeY",
        "EquirectangularToCubeMap4_PositiveZ",
        "EquirectangularToCubeMap5_NegativeZ",
    };
	static const std::string tech_name = "EquirectangularToCubeMap";
    Effect& effect = m_pContext->EffectInstance();
    effect.LoadTechnique(tech_name, &RenderStateDesc::PostProcess(), "CubeMapVS", "EquirectangularToCubeMapPS", nullptr);
	m_pMVPCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(float4x4), RESOURCE_FLAG_CPU_WRITE);

    for (uint32_t face = 0; face < (uint32_t)CubeFaceType::Num; face++)
    {
        PostProcessPtr pp = MakeSharedPtr<PostProcess>(context, names[face], PostProcessRenderType::Render_Cube);
        pp->Init(tech_name, NULL_PREDEFINES);
        //pp->SetParam("mvp", mvp[face].Transpose());
		float4x4 m = mvp[face].Transpose();
		m_pMVPCBuffer->Update(&m, sizeof(float4x4));
		pp->SetParam("mvp", m_pMVPCBuffer);
        this->AddPostProcess(pp);
    }

}

void EquirectangularToCubeMapPostProcess::SetSrcTexture(RHITexturePtr const& tex_2d)
{
    for (PostProcessPtr pp : m_vPPChain)
    {
        pp->SetParam("tex_equirectangular", tex_2d);
    }
}
void EquirectangularToCubeMapPostProcess::SetDstTexture(RHITexturePtr const& tex_cube)
{
    if (tex_cube->Type() != TextureType::Cube)
    {
        LOG_ERROR("EquirectangularMapPostProcess::SetDstTexture invalid arg");
        return;
    }
    RHIContext& rc = m_pContext->RHIContextInstance();
    for (uint32_t face = 0; face < (uint32_t)CubeFaceType::Num; face++)
    {
        RHIFrameBufferPtr fb = m_vPPChain[face]->GetFrameBuffer();
        RHIRenderViewPtr rv = rc.CreateRenderTargetView(tex_cube, (CubeFaceType)face);
        fb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rv);
    }
}


/******************************************************************************
 * IrradianceConvolution
 ******************************************************************************/
IrradianceConvolutionPostProcess::IrradianceConvolutionPostProcess(Context* context)
    :PostProcessChain(context, "IrradianceConvolution")
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    static std::vector<std::string> names = {
        "IrradianceConvolution0_PositiveX",
        "IrradianceConvolution1_NegativeX",
        "IrradianceConvolution2_PositiveY",
        "IrradianceConvolution3_NegativeY",
        "IrradianceConvolution4_PositiveZ",
        "IrradianceConvolution5_NegativeZ",
    };

    if (!m_pIrradianceConvolutionTex)
    {
        RHITexture::Desc desc;
        desc.type = TextureType::Cube;
        desc.width = IRRADIANCE_CONVOLUTION_MAP_SIZE;
        desc.height = IRRADIANCE_CONVOLUTION_MAP_SIZE;
        desc.depth = 1;
        desc.num_mips = 1;
        desc.num_samples = 1;
        desc.format = PixelFormat::R8G8B8A8_UNORM;
        desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE;
        m_pIrradianceConvolutionTex = rc.CreateTextureCube(desc);
    }
    static const std::string tech_name = "IrradianceConvolutionTech";
    Effect& effect = m_pContext->EffectInstance();
    effect.LoadTechnique(tech_name, &RenderStateDesc::PostProcess(), "CubeMapVS", "IrradianceConvolutionPS", nullptr);
    m_pMVPCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(float4x4), RESOURCE_FLAG_CPU_WRITE);
    for (uint32_t face = 0; face < (uint32_t)CubeFaceType::Num; face++)
    {
        PostProcessPtr pp = MakeSharedPtr<PostProcess>(context, names[face], PostProcessRenderType::Render_Cube);
        pp->Init(tech_name, NULL_PREDEFINES);
        //pp->SetParam("mvp", mvp[face].Transpose());
        float4x4 m = mvp[face].Transpose();
        m_pMVPCBuffer->Update(&m, sizeof(float4x4));
        pp->SetParam("mvp", m_pMVPCBuffer);

        RHIRenderViewPtr rv = rc.CreateRenderTargetView(m_pIrradianceConvolutionTex, (CubeFaceType)face);
        pp->GetFrameBuffer()->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rv);

        this->AddPostProcess(pp);
    }
}

void IrradianceConvolutionPostProcess::SetSrcTexture(RHITexturePtr const& tex_cube_env)
{
    for (PostProcessPtr pp : m_vPPChain)
    {
        pp->SetParam("tex_env", tex_cube_env);
    }
}

/******************************************************************************
 * PrefilterEnv : IBL for Specular
 ******************************************************************************/
PrefilterEnvPostProcess::PrefilterEnvPostProcess(Context* context)
    :PostProcessChain(context, "PrefilterEnv")
{
    static std::vector<std::string> names = {
        "PrefilterEnv0_PositiveX",
        "PrefilterEnv1_NegativeX",
        "PrefilterEnv2_PositiveY",
        "PrefilterEnv3_NegativeY",
        "PrefilterEnv4_PositiveZ",
        "PrefilterEnv5_NegativeZ",
    };
    static std::vector<std::string> param_names = {
        "mvp",
        "roughness",
        "tex_env",
    };

    static const std::string tech_name = "PrefilterEnvTech";
    Effect& effect = m_pContext->EffectInstance();
    effect.LoadTechnique(tech_name, &RenderStateDesc::PostProcess(), "CubeMapVS", "PrefilterEnvPS", nullptr);
    m_pMVPCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(float4x4), RESOURCE_FLAG_CPU_WRITE);
    for (uint32_t face = 0; face < (uint32_t)CubeFaceType::Num; face++)
    {
        PostProcessPtr pp = MakeSharedPtr<PostProcess>(context, names[face], PostProcessRenderType::Render_Cube);
        pp->Init(tech_name, NULL_PREDEFINES);
        float4x4 m = mvp[face].Transpose();
        m_pMVPCBuffer->Update(&m, sizeof(float4x4));
        pp->SetParam("mvp", m_pMVPCBuffer);
        this->AddPostProcess(pp);
    }
    for (uint32_t i = 0; i< MAX_MIP_LEVELS; i++)
	{
		m_pRoughnessCBuffer[i] = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(float), RESOURCE_FLAG_CPU_WRITE);
		m_vPPChain[0]->SetParam(param_names[1], m_pRoughnessCBuffer[i]);
	}
}

void PrefilterEnvPostProcess::SetSrcEnvTexture(RHITexturePtr const& tex_cube_env)
{
    for (PostProcessPtr pp : m_vPPChain)
    {
        pp->SetParam("tex_env", tex_cube_env);
    }
}
void PrefilterEnvPostProcess::SetDstPrefilterTexture(RHITexturePtr const& tex_cube_prefilter)
{
    if (tex_cube_prefilter->Type() != TextureType::Cube)
    {
        LOG_ERROR("PrefilterEnvPostProcess::SetDstTexture invalid arg");
        return;
    }
    RHIContext& rc = m_pContext->RHIContextInstance();

    for (uint32_t face = 0; face < (uint32_t)CubeFaceType::Num; face++)
    {
        for (uint32_t mip = 0; mip < MAX_MIP_LEVELS; ++mip)
        {         
            m_vRenderViews[face][mip] = rc.CreateRenderTargetView(tex_cube_prefilter, (CubeFaceType)face, mip);
        }
    }
}

SResult PrefilterEnvPostProcess::Run()
{
    // to optimize: update roughness one time
    for (uint32_t face = 0; face < (uint32_t)CubeFaceType::Num; face++)
    {
        RHIFrameBufferPtr fb = m_vPPChain[face]->GetFrameBuffer();
        for (uint32_t mip = 0; mip < MAX_MIP_LEVELS; ++mip)
        {
            float roughness = (float)mip / (float)(MAX_MIP_LEVELS - 1);
			m_pRoughnessCBuffer[mip]->Update(&roughness, sizeof(float));
			m_vPPChain[face]->SetParam("roughness", m_pRoughnessCBuffer[mip]);

            fb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, m_vRenderViews[face][mip]);
            SEEK_RETIF_FAIL(m_vPPChain[face]->Run());
        }
    }
    return S_Success;
}

/******************************************************************************
 * GenBRDF2DLut : IBL for Specular
 ******************************************************************************/
RHITexturePtr GenBRDF2DLutPostProcess::m_pTexBRDF2DLut = nullptr;
RHITexturePtr const& GenBRDF2DLutPostProcess::GetBRDF2DLutTex(Context* pContext)
{
    static const std::string tech_name = "GenBRDF2DLutTech";
    Effect& effect = pContext->EffectInstance();
    effect.LoadTechnique(tech_name, &RenderStateDesc::PostProcess(), "CubeMapVS", "GenBRDF2DLutPS", nullptr);
    if (!m_pTexBRDF2DLut)
    {
        PostProcessPtr pGenBRDF2DLut = MakeSharedPtr<PostProcess>(pContext, "GenBRDF2DLut");
        pGenBRDF2DLut->Init(tech_name, NULL_PREDEFINES);
        RHITexture::Desc brdf_lut_desc;
        brdf_lut_desc.type = TextureType::Tex2D;
        brdf_lut_desc.width = BRDF_LUT_MAP_SIZE;
        brdf_lut_desc.height = BRDF_LUT_MAP_SIZE;
        brdf_lut_desc.depth = 1;
        brdf_lut_desc.num_mips = 1;
        brdf_lut_desc.num_samples = 1;
        brdf_lut_desc.format = PixelFormat::R32G32F;
        brdf_lut_desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE;
        m_pTexBRDF2DLut = pContext->RHIContextInstance().CreateTexture2D(brdf_lut_desc, nullptr);
        pGenBRDF2DLut->SetOutput(0, m_pTexBRDF2DLut);
        pGenBRDF2DLut->Run();
    }
    return m_pTexBRDF2DLut;
}

/******************************************************************************
 * SplitSumApproximationPostProcess : IBL for Specular
 ******************************************************************************/
SplitSumApproximationPostProcess::SplitSumApproximationPostProcess(Context* context)
    :PostProcessChain(context, "SplitSumApproximation")
{
    RHIContext& rc = m_pContext->RHIContextInstance();

    m_pPPPrefilter = MakeSharedPtr<PrefilterEnvPostProcess>(m_pContext);
    this->AddPostProcess(m_pPPPrefilter);
    RHITexture::Desc prefilter_desc;
    prefilter_desc.type = TextureType::Cube;
    prefilter_desc.width = PREFILTER_MAP_SIZE;
    prefilter_desc.height = PREFILTER_MAP_SIZE;
    prefilter_desc.depth = 1;
    prefilter_desc.num_mips = PrefilterEnvPostProcess::MAX_MIP_LEVELS;
    prefilter_desc.num_samples = 1;
    prefilter_desc.format = PixelFormat::R16G16B16A16_FLOAT;
    prefilter_desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE | RESOURCE_FLAG_GENERATE_MIPS;
    m_pTexPrefilter = rc.CreateTextureCube(prefilter_desc, nullptr);
    m_pTexPrefilter->GenerateMipMap();
    m_pPPPrefilter->SetDstPrefilterTexture(m_pTexPrefilter);
}
void SplitSumApproximationPostProcess::SetSrcEnvTex(RHITexturePtr const& tex_cube_env)
{
    m_pPPPrefilter->SetSrcEnvTexture(tex_cube_env);
}


SEEK_NAMESPACE_END

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
