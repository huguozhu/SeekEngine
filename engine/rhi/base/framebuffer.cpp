#include "rhi/base/framebuffer.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/render_view.h"
#include "kernel/context.h"

#define SEEK_MACRO_FILE_UID 43     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

Viewport const & FrameBuffer::GetViewport()
{
    return m_stViewport;
}

void FrameBuffer::AttachTargetView(Attachment att, RenderViewPtr const& view)
{
    uint8_t index = to_underlying(att);
    if (index >= MAX_COLOR_ATTACHMENTS)
        return;

    m_vRenderTargets[att] = view;
    m_bViewDirty = true;
}

void FrameBuffer::AttachDepthStencilView(RenderViewPtr const& view)
{
    m_pDepthStencilView = view;
    m_bViewDirty = true;
}

RenderViewPtr FrameBuffer::GetRenderTarget(Attachment att) const
{
    uint32_t color_index = att - Attachment::Color0;
    if (color_index < m_vRenderTargets.size())
        return m_vRenderTargets[color_index];
    else
        return nullptr;
}

SResult FrameBuffer::Bind()
{
    // prepare the MSAA attachments
    if (m_sampleNum > 1)
    {
        for (size_t idx = 0; idx != MAX_COLOR_ATTACHMENTS; idx++)
        {
            if (!m_vRenderTargets[idx])
                continue;
            
            // check if the exist MSAA attachment is matched with the color attachment, now only check with/height/format
            if (m_msaaColorTex[idx])
            {
                if (m_msaaColorTex[idx]->Width() != m_vRenderTargets[idx]->Width() ||
                    m_msaaColorTex[idx]->Height() != m_vRenderTargets[idx]->Height() ||
                    m_msaaColorTex[idx]->Format() != m_vRenderTargets[idx]->Format())
                {
                    m_msaaColorTex[idx] = nullptr;
                    LOG_INFO("the exist MSAA color texture %zu is mismatched, recreate it",  idx)
                }
            }
            
            if (!m_msaaColorTex[idx])
            {
                Texture::Desc colorTexDesc = m_vRenderTargets[idx]->Texture()->Descriptor();
                colorTexDesc.num_samples = m_sampleNum;
                m_msaaColorTex[idx] = m_pContext->RHIContextInstance().CreateTexture2D(colorTexDesc);
            }
        }
        
        if (m_pDepthStencilView)
        {
            if (m_msaaDepthStencilTex)
            {
                if (m_msaaDepthStencilTex->Width() != m_pDepthStencilView->Width() ||
                    m_msaaDepthStencilTex->Height() != m_pDepthStencilView->Height() ||
                    m_msaaDepthStencilTex->Format() != m_pDepthStencilView->Format())
                {
                    m_msaaDepthStencilTex = nullptr;
                    LOG_INFO("the exist MSAA depth texture is mismatched, recreate it")
                }
            }
            
            if (!m_msaaDepthStencilTex)
            {
                Texture::Desc depthStencilTexDesc = m_pDepthStencilView->Texture()->Descriptor();
                depthStencilTexDesc.num_samples = m_sampleNum;
                m_msaaDepthStencilTex = m_pContext->RHIContextInstance().CreateTexture2D(depthStencilTexDesc);
            }
        }
    }
    
    return OnBind();
}

void FrameBuffer::Unbind()
{
    OnUnbind();
}

void FrameBuffer::SetColorLoadOption(Attachment attachment, LoadOption loadOption)
{
    uint8_t index = to_underlying(attachment);
    if (index >= MAX_COLOR_ATTACHMENTS)
        return;
    
    m_colorLoadOptions[index] = loadOption;
    m_bViewDirty = true;
}

void FrameBuffer::SetColorStoreOption(Attachment attachment, StoreOption storeOption)
{
    uint8_t index = to_underlying(attachment);
    if (index >= MAX_COLOR_ATTACHMENTS)
        return;
    
    m_colorStoreOptions[index] = storeOption;
    m_bViewDirty = true;
}

void FrameBuffer::SetDepthLoadOption(LoadOption loadOption)
{
    m_depthLoadOption = loadOption;
    m_bViewDirty = true;
}

void FrameBuffer::SetDepthStoreOption(StoreOption storeOption)
{
    m_depthStoreOption = storeOption;
    m_bViewDirty = true;
}

void FrameBuffer::Reset()
{
    std::fill(m_vRenderTargets.begin(), m_vRenderTargets.end(), nullptr);
    m_pDepthStencilView = nullptr;
    m_bViewDirty = true;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
