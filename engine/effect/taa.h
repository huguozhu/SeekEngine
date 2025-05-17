#pragma once

#include "effect/postprocess.h"

SEEK_NAMESPACE_BEGIN

#include "shader/shared/Taa.h"

class TaaPostProcess : public PostProcess
{
public:
	TaaPostProcess(Context* context)
		: PostProcess(context, "TAA")
	{ }

	SResult Init();
	virtual SResult Run() override;
	void SetTaaParams(TAAGlobalParams& taaparams);

private:
	RHITexturePtr m_pHistoryTex = nullptr;

	RHIRenderBufferPtr m_globalParamRB = nullptr;
	bool m_isFirstFrame = true;
};

SEEK_NAMESPACE_END
