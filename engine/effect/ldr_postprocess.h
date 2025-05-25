#pragma once

#include "effect/postprocess.h"

SEEK_NAMESPACE_BEGIN

class LDRPostProcess : public PostProcess
{
public:
	LDRPostProcess(Context* context);
 
	SResult Init();
	
	void SetLDRTexture(RHITexturePtr const& tex2d);
	virtual SResult SetOutput(uint32_t index, RHITexturePtr const& tex, CubeFaceType type = CubeFaceType::Positive_X) override;
	virtual SResult SetOutput(uint32_t index, RHIRenderViewPtr const& target);
	void SetTaaSceneVelocityTexture(RHITexturePtr const& tex2d);

	virtual SResult		Run() override;

private:
	PostProcessPtr			m_pCopyPostProcess = nullptr;
	PostProcessPtr			m_pFxaaPostProcess = nullptr;
	TaaPostProcessPtr		m_pTaaPostProcess = nullptr;

    //TexturePtr              m_pCopyTextureSrc = nullptr;
};

SEEK_NAMESPACE_END
