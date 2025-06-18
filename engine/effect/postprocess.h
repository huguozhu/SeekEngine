#pragma once

#include "kernel/kernel.h"
#include "kernel/context.h"
#include "utils/error.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_framebuffer.h"
#include "effect/technique.h"

SEEK_NAMESPACE_BEGIN

enum class PostProcessRenderType
{
    Render_2D,
    Render_Cube,
};
/******************************************************************************
 * PostProcess
 ******************************************************************************/
class PostProcess
{
public:
    PostProcess(Context* context, std::string const& name, PostProcessRenderType type = PostProcessRenderType::Render_2D);
    virtual ~PostProcess()
    {
        std::fill(m_vOutputs.begin(), m_vOutputs.end(), nullptr);
    }

    virtual SResult     Run();
    virtual SResult     RunGraphicsPipeline();
    virtual SResult     RunComputePipeline();
    
    virtual SResult     SetOutput(uint32_t index, RHITexturePtr const& tex, CubeFaceType type = CubeFaceType::Positive_X);
    virtual SResult     SetOutput(uint32_t index, RHIRenderViewPtr const& target);

    template <typename T>
    void SetParam(const std::string& name, const T& val)
    {
        if (m_pTechnique)
            m_pTechnique->SetParam(name, val);
    }

    SResult             Init(const std::string& tech_name, const std::vector<EffectPredefine>& predefines);
    SResult             Init(const std::string& tech_name);

    SResult             GetCsThreadsPerGroup(uint32_t& x, uint32_t& y, uint32_t& z);
    void                SetCsDispatchSize(uint32_t x, uint32_t y, uint32_t z);

    RHIFrameBufferPtr&  GetFrameBuffer() { return m_pFrameBuffer; }
    void                SetClear(bool clear) { m_bClear = clear; }

    void                SetPostProcessRenderStateDesc(RenderStateDesc desc);

    // so ugly!!! maybe a constant buffer pool for the modern Graphics API(metal/d3d12/vulkan)?
    void                UpdateGlobalParams(const void* data, uint32_t size, bool bForceRecreate = false);

protected:
    Context*            m_pContext = nullptr;
    std::string         m_szName;
    
    Technique*          m_pTechnique = nullptr;
    bool                m_bIsGraphicsPipeline = false;
    bool                m_bInitSucceed = false;
    
    bool                m_bClear = false;
    PostProcessRenderType   m_eRenderType = PostProcessRenderType::Render_2D;

    RHIFrameBufferPtr   m_pFrameBuffer = nullptr;
    RHIMeshPtr          m_pPostProcessMesh = nullptr;

    std::array<RHIRenderViewPtr, RHIFrameBuffer::MAX_COLOR_ATTACHMENTS> m_vOutputs;

    // for Compute Shader PostProcess
    uint32_t            m_iCsDispatchX;
    uint32_t            m_iCsDispatchY;
    uint32_t            m_iCsDispatchZ;

    RHIRenderBufferPtr  m_GlobalParamsCBuffer;
};

/******************************************************************************
 * PostProcessChain
 ******************************************************************************/
class PostProcessChain : public PostProcess
{
public:
    PostProcessChain(Context* context, std::string const& name, PostProcessRenderType type = PostProcessRenderType::Render_2D);
    void AddPostProcess(PostProcessPtr const& pp);

    virtual SResult   Run() override;
    virtual SResult   SetOutput(uint32_t index, RHITexturePtr const& tex, CubeFaceType type = CubeFaceType::Positive_X) override;

    template <typename T>
    void SetParam(const std::string& name, const T& val)
    {
		if (m_vPPChain.size() > 0)
			m_vPPChain[0]->SetParam(name, val);
    }


protected:
    std::vector<PostProcessPtr>      m_vPPChain;
};

SEEK_NAMESPACE_END
