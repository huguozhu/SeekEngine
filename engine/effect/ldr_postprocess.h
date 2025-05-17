#pragma once

#include "effect/postprocess.h"

SEEK_NAMESPACE_BEGIN

class LDRPostProcess : public PostProcess
{
public:
	LDRPostProcess(Context* context);
 
	SResult Init();
	
	void SetSrcTexture(RHITexturePtr const& tex2d);
	virtual SResult		Run() override;

private:
	PostProcessPtr			m_pCopyPostProcess = nullptr;
	PostProcessPtr			m_pFxaaPostProcess = nullptr;
	TaaPostProcessPtr		m_pTaaPostProcess = nullptr;

    //TexturePtr              m_pCopyTextureSrc = nullptr;
};

SEEK_NAMESPACE_END
