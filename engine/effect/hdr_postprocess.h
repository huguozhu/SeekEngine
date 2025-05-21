#pragma once

#include "kernel/kernel.h"
#include "effect/postprocess.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * HDRPostProcess
 ******************************************************************************/
class HDRPostProcess : public PostProcess
{
public:
    HDRPostProcess(Context* context);

    void SetSrcTexture(RHITexturePtr const& tex2d);
	virtual SResult SetOutput(uint32_t index, RHITexturePtr const& tex, CubeFaceType type = CubeFaceType::Positive_X) override
	{
		return m_pToneMapping->SetOutput(index, tex, type);
	}
    virtual SResult Run() override;

private:
    PostProcessPtr      m_pToneMapping  = nullptr;
};

SEEK_NAMESPACE_END
