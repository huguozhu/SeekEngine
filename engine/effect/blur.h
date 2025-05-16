#pragma once

#include "kernel/kernel.h"
#include "effect/postprocess.h"

SEEK_NAMESPACE_BEGIN

class GaussianBlur : public PostProcessChain
{
public:
    GaussianBlur(Context* context);

    void SetSrcTexture(RHITexturePtr const& tex2d);
    void SetDstTexture(RHITexturePtr const& tex2d);

public:
    RHITexturePtr           m_pTemp = nullptr;
	RHIRenderBufferPtr      m_pBlurXCBuffer = nullptr;
    RHIRenderBufferPtr      m_pBlurYCBuffer = nullptr;
    
};

SEEK_NAMESPACE_END
