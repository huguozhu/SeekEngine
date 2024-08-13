#include "rendering_d3d11/d3d11_predeclare.h"
#include "rendering_d3d11/d3d11_framebuffer.h"
#include "rendering_d3d11/d3d11_render_context.h"
#include "rendering_d3d11/d3d11_render_view.h"
#include "rendering_d3d11/d3d11_texture.h"
#include "kernel/context.h"

#define DVF_MACRO_FILE_UID 3     // this code is auto generated, don't touch it!!!

DVF_NAMESPACE_BEGIN

D3D11FrameBuffer::D3D11FrameBuffer(Context* context)
    : FrameBuffer(context)
{
    zm_memset_s(&m_stD3dViewport, sizeof(m_stD3dViewport), 0, sizeof(m_stD3dViewport));
}

D3D11FrameBuffer::~D3D11FrameBuffer()
{
    m_vD3dRednerTargets.clear();
}

DVFResult D3D11FrameBuffer::OnBind()
{
    DVFResult res = DVF_Success;
    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
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
                m_vD3dRednerTargets[i] = target.GetD3DRenderTargetView();
            }
            else
            {
                D3D11Texture* d3dTex = static_cast<D3D11Texture*>(m_msaaColorTex[i].get());
                m_vD3dRednerTargets[i] = d3dTex->GetD3DRenderTargetView();

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
                m_pD3dDepthStencilView = ds.GetD3DDepthStencilView();
            }
            else
            {
                D3D11Texture* d3dTex = static_cast<D3D11Texture*>(m_msaaDepthStencilTex.get());
                m_pD3dDepthStencilView = d3dTex->GetD3DDepthStencilView();

                if (m_depthStoreOption.storeAction == StoreAction::Store)
                    m_resolveFlag |= RequireResolve(Attachment::Depth);
            }

            if (m_depthLoadOption.loadAction == LoadAction::Clear)
                pDeviceContext->ClearDepthStencilView(m_pD3dDepthStencilView, D3D11_CLEAR_DEPTH, m_depthLoadOption.clearDepth, 0);
        }
        m_bViewDirty = false;
    }

    pDeviceContext->OMSetRenderTargets((UINT)1, m_vD3dRednerTargets.data(), m_pD3dDepthStencilView);
    return res;
}

DVFResult D3D11FrameBuffer::OnUnbind()
{
    DVFResult res = DVF_Success;
    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    Resolve();
    std::vector< ID3D11RenderTargetView*> zero(m_vD3dRednerTargets.size(), nullptr);
    pDeviceContext->OMSetRenderTargets((UINT)1, zero.data(), nullptr);
    return res;
}

DVFResult D3D11FrameBuffer::Resolve()
{
    if (m_resolveFlag == RESOLVE_NONE)
        return DVF_Success;

    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
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
    return DVF_Success;
}

DVF_NAMESPACE_END

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
