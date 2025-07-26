#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_framebuffer.h"

SEEK_NAMESPACE_BEGIN

class D3D11FrameBuffer : public RHIFrameBuffer
{
public:
    D3D11FrameBuffer(Context* context);
    virtual ~D3D11FrameBuffer();

    SResult OnBind() override;
    SResult OnUnbind() override;
    SResult Resolve();

    void Clear(uint32_t flags = CBM_ALL, float4 const& clr = float4(0.0, 0.0, 0.0, 0.0), float depth = 1.0, int32_t stencil = 0);
    void ClearRenderTarget(Attachment att, float4 const& clr = float4(0.0, 0.0, 0.0, 0.0));

    ID3D11RenderTargetView* GetRenderTargetView(uint32_t rt_index = 0) const { return m_vD3dRednerTargets[rt_index]; }

protected:
    std::vector<ID3D11RenderTargetView*> m_vD3dRednerTargets;
    ID3D11DepthStencilView* m_pD3dDepthStencilView = nullptr;
    D3D11_VIEWPORT m_stD3dViewport;
};

SEEK_NAMESPACE_END
