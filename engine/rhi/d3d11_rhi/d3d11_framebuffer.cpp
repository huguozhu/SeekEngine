#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/d3d11_rhi/d3d11_framebuffer.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"
#include "rhi/d3d11_rhi/d3d11_render_view.h"
#include "rhi/d3d11_rhi/d3d11_texture.h"
#include "kernel/context.h"

#define SEEK_MACRO_FILE_UID 3     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

D3D11RHIFrameBuffer::D3D11RHIFrameBuffer(Context* context)
    : RHIFrameBuffer(context)
{
    seek_memset_s(&m_stD3dViewport, sizeof(m_stD3dViewport), 0, sizeof(m_stD3dViewport));
}

D3D11RHIFrameBuffer::~D3D11RHIFrameBuffer()
{
    m_vD3dRednerTargets.clear();
}

SResult D3D11RHIFrameBuffer::OnBind()
{
    SResult res = S_Success;
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    if (m_bViewDirty)
    {
        m_vD3dRednerTargets.resize(m_vRenderTargets.size(), nullptr);
        m_pD3dDepthStencilView = nullptr;
        
        for (uint32_t i = 0; i < MAX_COLOR_ATTACHMENTS; i++)
        {
            if (!m_vRenderTargets[i])
                continue;
            if (m_sampleNum == 1)
            {
                D3D11RenderTargetView& target = static_cast<D3D11RenderTargetView&>(*m_vRenderTargets[i]);
                m_vD3dRednerTargets[i] = target.GetD3DRtv();
            }
            else
            {
                D3D11Texture* d3dTex = static_cast<D3D11Texture*>(m_msaaColorTex[i].get());
                m_vD3dRednerTargets[i] = d3dTex->GetD3DRtv();

                if (m_colorStoreOptions[i].storeAction == StoreAction::Store)
                    m_resolveFlag |= RequireResolve((Attachment)i);
            }

            if (m_colorLoadOptions[i].loadAction == LoadAction::Clear)
                pDeviceContext->ClearRenderTargetView(m_vD3dRednerTargets[i], m_colorLoadOptions[i].clearColor.data());
        }

        if (m_pDepthStencilView)
        {
            if (m_sampleNum == 1)
            {
                D3D11DepthStencilView& ds = static_cast<D3D11DepthStencilView&>(*m_pDepthStencilView);
                m_pD3dDepthStencilView = ds.GetD3DDsv();
            }
            else
            {
                D3D11Texture* d3dTex = static_cast<D3D11Texture*>(m_msaaDepthStencilTex.get());
                m_pD3dDepthStencilView = d3dTex->GetD3DDsv();

                if (m_depthStoreOption.storeAction == StoreAction::Store)
                    m_resolveFlag |= RequireResolve(Attachment::Depth);
            }

            if (m_depthLoadOption.loadAction == LoadAction::Clear)
                pDeviceContext->ClearDepthStencilView(m_pD3dDepthStencilView, D3D11_CLEAR_DEPTH, m_depthLoadOption.clearDepth, 0);
        }
        m_bViewDirty = false;
    }

    pDeviceContext->OMSetRenderTargets(m_vD3dRednerTargets.size(), m_vD3dRednerTargets.data(), m_pD3dDepthStencilView);
    return res;
}

SResult D3D11RHIFrameBuffer::OnUnbind()
{
    SResult res = S_Success;
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    Resolve();
    std::vector< ID3D11RenderTargetView*> zero(m_vD3dRednerTargets.size(), nullptr);
    pDeviceContext->OMSetRenderTargets((UINT)1, zero.data(), nullptr);
    return res;
}

SResult D3D11RHIFrameBuffer::Resolve()
{
    if (m_resolveFlag == RESOLVE_NONE)
        return S_Success;

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    for (uint8_t idx = 0; idx != MAX_COLOR_ATTACHMENTS; idx++)
    {
        if (NeedResolve(m_resolveFlag, (Attachment)idx))
        {
            D3D11Texture* d3dTex = static_cast<D3D11Texture*>(m_msaaColorTex[idx].get());
            D3D11Texture* d3dResolveTex = static_cast<D3D11Texture*>(m_vRenderTargets[idx]->Texture().get());

            rc.GetD3D11DeviceContext()->ResolveSubresource(d3dResolveTex->GetD3DTexture(), 0, d3dTex->GetD3DTexture(), 0, d3dTex->GetD3DFormat());
        }
    }

    if (NeedResolve(m_resolveFlag, Attachment::Depth))
    {
        D3D11Texture* d3dTex = static_cast<D3D11Texture*>(m_msaaDepthStencilTex.get());
        D3D11Texture* d3dResolveTex = static_cast<D3D11Texture*>(m_pDepthStencilView->Texture().get());
        rc.GetD3D11DeviceContext()->ResolveSubresource(d3dResolveTex->GetD3DTexture(), 0, d3dTex->GetD3DTexture(), 0, d3dTex->GetD3DFormat());
    }

    m_resolveFlag = RESOLVE_NONE;
    return S_Success;
}
void D3D11RHIFrameBuffer::Clear(uint32_t flags, float4 const& clr, float depth, int32_t stencil)
{
    if (flags & CBM_Color)
    {
        for (uint32_t i = 0; i < m_vRenderTargets.size(); i++)
        {
            if (m_vRenderTargets[i])
            {
                D3D11RenderTargetView& view = static_cast<D3D11RenderTargetView&>(*m_vRenderTargets[i]);
                view.ClearColor(clr);
            }
        }
    }

    if (m_pDepthStencilView)
    {
        D3D11DepthStencilView& view = static_cast<D3D11DepthStencilView&>(*m_pDepthStencilView);
        if ((flags & CBM_Depth) && (flags & CBM_Stencil))
        {
            view.ClearDepthStencil(depth, stencil);
        }
        else
        {
            if (flags & CBM_Depth)
                view.ClearDepth(depth);
            if (flags & CBM_Stencil)
                view.ClearStencil(stencil);
        }
    }
}
void D3D11RHIFrameBuffer::ClearRenderTarget(Attachment att, float4 const& clr)
{
    if (att < Attachment::Color7)
    {
        uint32_t i = (uint32_t)Attachment::Color7 - (uint32_t)att;
        if (m_vRenderTargets[i])
        {
            D3D11RenderTargetView& view = static_cast<D3D11RenderTargetView&>(*m_vRenderTargets[i]);
            view.ClearColor(clr);
        }
    }
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
