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
    virtual SResult Run() override;

private:
    PostProcessPtr      m_pToneMapping  = nullptr;
};

SEEK_NAMESPACE_END
