#include "effect/postprocess.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "rhi/base/rhi_program.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_shader.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_render_view.h"
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/base/rhi_context.h"
#include "math/math_utility.h"
#include "math/quad_mesh_process.h"
#include "utils/log.h"


#define SEEK_MACRO_FILE_UID 53     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * PostProcess
 ******************************************************************************/
PostProcess::PostProcess(Context* context, std::string const& name, PostProcessRenderType type)
    : m_pContext(context), m_szName(name), m_eRenderType(type)
{
    std::fill(m_vOutputs.begin(), m_vOutputs.end(), nullptr);
}

SResult PostProcess::Init(const std::string& tech_name, const std::vector<EffectPredefine>& predefines)
{
    SResult ret = S_Success;

    auto& effect = m_pContext->EffectInstance();

    m_pTechnique = effect.GetTechnique(tech_name, predefines);
    if (!m_pTechnique)
    {
        LOG_ERROR("load Technique %s fail", tech_name.c_str());
        return ERR_INVALID_INIT;
    }
    
    m_bIsGraphicsPipeline = m_pTechnique->GetVirtualTechnique()->IsGraphicsPipeline();
    if (m_bIsGraphicsPipeline)
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
        m_pFrameBuffer = rc.CreateRHIFrameBuffer();
        
        // TODO: don't hardcode
        m_pFrameBuffer->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, { RHIFrameBuffer::LoadAction::DontCare });
        m_pFrameBuffer->SetColorStoreOption(RHIFrameBuffer::Attachment::Color0, { RHIFrameBuffer::StoreAction::Store });
        
        if (m_eRenderType == PostProcessRenderType::Render_2D)
            m_pPostProcessMesh = QuadMesh_GetMesh(rc);
        else
            m_pPostProcessMesh = rc.GetCubeMesh();
    }

    m_bInitSucceed = true;
    return S_Success;
}

SResult PostProcess::Init(const std::string& tech_name)
{
    return Init(tech_name, NULL_PREDEFINES);
}

SResult PostProcess::Run()
{
    if (!m_bInitSucceed)
        return ERR_INVALID_INIT;

    RHIContext& rc = m_pContext->RHIContextInstance();
    SResult ret;
    if (!m_bIsGraphicsPipeline)
    {
        RHIContext::ComputePassInfo info;
        info.name = m_szName;
        rc.BeginComputePass(info);
        ret = RunComputePipeline();
        rc.EndComputePass();
        return S_Success;
    }
    else
    {
        RHIContext::RenderPassInfo info;
        info.name = m_szName;
        info.fb = m_pFrameBuffer.get();
        RHIFrameBuffer::LoadAction la = m_bClear ? RHIFrameBuffer::LoadAction::Clear : RHIFrameBuffer::LoadAction::Load;
        for (int i = 0; i < RHIFrameBuffer::MAX_COLOR_ATTACHMENTS; i++)
            m_pFrameBuffer->SetColorLoadOption(RHIFrameBuffer::Attachment(i), la);
        rc.BeginRenderPass(info);
        ret = RunGraphicsPipeline();
        rc.EndRenderPass();
        return ret;
    }
}

SResult PostProcess::RunGraphicsPipeline()
{
    m_pPostProcessMesh->SetRenderState(m_pTechnique->GetRenderState());
    return m_pTechnique->Render(m_pPostProcessMesh);
}

SResult PostProcess::RunComputePipeline()
{
    m_pTechnique->Dispatch(m_iCsDispatchX, m_iCsDispatchY, m_iCsDispatchZ);
    return S_Success;
}

SResult PostProcess::SetOutput(uint32_t index, RHITexturePtr const& tex, CubeFaceType type)
{
    if (nullptr == tex)
        m_pFrameBuffer->AttachTargetView((RHIFrameBuffer::Attachment)(RHIFrameBuffer::Color0 + index), nullptr);
    else
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
        if (tex->Type() == TextureType::Cube)
        {
            if ((uint8_t)type >= (uint8_t)CubeFaceType::Num)
                return ERR_INVALID_ARG;
			m_pFrameBuffer->AttachTargetView((RHIFrameBuffer::Attachment)(RHIFrameBuffer::Color0 + index), rc.CreateRenderTargetView(tex, type));
        }
        else
            m_pFrameBuffer->AttachTargetView((RHIFrameBuffer::Attachment)(RHIFrameBuffer::Color0 + index), rc.CreateRenderTargetView(tex));

        if (0 == index)
            m_pFrameBuffer->SetViewport({ 0, 0, tex->Width(), tex->Height() });
    }
    return S_Success;
}

SResult PostProcess::SetOutput(uint32_t index, RHIRenderViewPtr const& target)
{
    if (!m_bInitSucceed)
        return ERR_INVALID_INIT;
    
    if (index >= m_vOutputs.size())
        return ERR_INVALID_ARG;
    
    m_pFrameBuffer->AttachTargetView((RHIFrameBuffer::Attachment)(RHIFrameBuffer::Color0 + index), target);
    if (0 == index)
    {
        m_pFrameBuffer->SetViewport({ 0, 0, target->Width(), target->Height() });
    }
    return S_Success;
}

SResult PostProcess::GetCsThreadsPerGroup(uint32_t& x, uint32_t& y, uint32_t& z)
{
    if (!m_bInitSucceed)
        return ERR_INVALID_INIT;
    
    if (m_bIsGraphicsPipeline)
        return ERR_NOT_SUPPORT;
    
    m_pTechnique->GetProgram()->GetShader(ShaderType::Compute)->GetCsThreadsPerGroup(x, y, z);
    return S_Success;
}

void PostProcess::SetCsDispatchSize(uint32_t x, uint32_t y, uint32_t z)
{
    m_iCsDispatchX = x;
    m_iCsDispatchY = y;
    m_iCsDispatchZ = z;
}
void PostProcess::SetPostProcessRenderStateDesc(RenderStateDesc desc)
{
    if (m_pTechnique)
    {
        m_pTechnique->SetRenderStateDesc(desc);
    }
}
/******************************************************************************
 * PostProcessChain
 ******************************************************************************/
PostProcessChain::PostProcessChain(Context* context, std::string const& name, PostProcessRenderType type)
    :PostProcess(context, name, type)
{
}

SResult PostProcessChain::Run()
{
    for (auto const& pp : m_vPPChain)
    {
        SEEK_RETIF_FAIL(pp->Run());
    }
    return S_Success;
}

SResult PostProcessChain::SetOutput(uint32_t index, RHITexturePtr const& tex, CubeFaceType type)
{
    return m_vPPChain.back()->SetOutput(index, tex, type);
}

void PostProcessChain::AddPostProcess(PostProcessPtr const& pp)
{
    m_vPPChain.push_back(pp);
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
