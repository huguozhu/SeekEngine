#pragma once

#include "kernel/kernel.h"
#include "kernel/context.h"
#include "utils/error.h"
#include "effect/postprocess.h"

SEEK_NAMESPACE_BEGIN

#include "shader/shared/common.h"
#include "shader/shared/WaterMark.h"

/******************************************************************************
 * PostProcess
 ******************************************************************************/
class WaterMarkPostProcess : public PostProcess
{
public:
    WaterMarkPostProcess(Context* context);
    virtual ~WaterMarkPostProcess()
    {
    }
    
    SResult Init();
    SResult SetSrcTex(RHITexturePtr src);
    SResult SetWaterMarkDesc(WaterMarkDesc desc);

private:
    WaterMarkDesc m_sDesc = { 0 };
    RHIRenderBufferPtr  m_pWatermarkCBuffer = nullptr;

    RHITexturePtr       m_pSrcTex = nullptr;
    
};


SEEK_NAMESPACE_END
