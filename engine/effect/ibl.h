#pragma once

#include "effect/postprocess.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * EquirectangularMapPostProcess : Equirectangular to Cube
 ******************************************************************************/
class EquirectangularToCubeMapPostProcess : public PostProcessChain
{
public:
    EquirectangularToCubeMapPostProcess(Context* context);

    void SetSrcTexture(RHITexturePtr const& tex_2d);
    void SetDstTexture(RHITexturePtr const& tex_cube);

private:
	RHIGpuBufferPtr  m_pMVPCBuffer = nullptr;
};
CLASS_DECLARE(EquirectangularToCubeMapPostProcess);

/******************************************************************************
 * IrradianceConvolution : IBL for Diffuse
 ******************************************************************************/
class IrradianceConvolutionPostProcess : public PostProcessChain
{
public:
    IrradianceConvolutionPostProcess(Context* context);

    void SetSrcTexture(RHITexturePtr const& tex_cube_env);

    RHITexturePtr const& GetIrradianceConvolutionTex() { return m_pIrradianceConvolutionTex; }

private:
    RHITexturePtr m_pIrradianceConvolutionTex = nullptr;
    RHIGpuBufferPtr  m_pMVPCBuffer = nullptr;
};
CLASS_DECLARE(IrradianceConvolutionPostProcess);


/******************************************************************************
 * PrefilterEnv : IBL for Specular
 ******************************************************************************/
class PrefilterEnvPostProcess : public PostProcessChain
{
public:
    PrefilterEnvPostProcess(Context* context);

    void SetSrcEnvTexture(RHITexturePtr const& tex_cube_env);
    void SetDstPrefilterTexture(RHITexturePtr const& tex_cube_prefilter);
    virtual SResult Run() override;

    static const uint32_t MAX_MIP_LEVELS = 5;
private:
    RHIRenderTargetViewPtr m_vRenderViews[(uint32_t)CubeFaceType::Num][MAX_MIP_LEVELS] = { nullptr };
    RHIGpuBufferPtr  m_pMVPCBuffer = nullptr;
    RHIGpuBufferPtr  m_pRoughnessCBuffer[MAX_MIP_LEVELS] = { nullptr };
};
CLASS_DECLARE(PrefilterEnvPostProcess);


/******************************************************************************
 * GenBRDF2DLutPostProcess : IBL for Specular
 ******************************************************************************/
class GenBRDF2DLutPostProcess
{
public:
    static RHITexturePtr const& GetBRDF2DLutTex(Context* pContext);

private:
    static RHITexturePtr m_pTexBRDF2DLut;
};

/******************************************************************************
 * SplitSumApproximation : IBL for Specular
 ******************************************************************************/
class SplitSumApproximationPostProcess : public PostProcessChain
{
public:
    SplitSumApproximationPostProcess(Context* context);
    void SetSrcEnvTex(RHITexturePtr const& tex_cube_env);

    RHITexturePtr const& GetPreFilterTexture() { return m_pTexPrefilter; }
    RHITexturePtr const& GetBRDF2DLutTexture() { return GenBRDF2DLutPostProcess::GetBRDF2DLutTex(m_pContext); }

private:
    PrefilterEnvPostProcessPtr m_pPPPrefilter = nullptr;
    PostProcessPtr m_pPPGenBRDF2DLut = nullptr;

    RHITexturePtr m_pTexPrefilter = nullptr;
    RHITexturePtr m_pTexBRDF2DLut = nullptr;
};

CLASS_DECLARE(SplitSumApproximationPostProcess);
SEEK_NAMESPACE_END
