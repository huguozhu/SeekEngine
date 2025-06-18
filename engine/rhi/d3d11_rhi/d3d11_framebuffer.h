#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_framebuffer.h"

SEEK_NAMESPACE_BEGIN

class D3D11RHIFrameBuffer : public RHIFrameBuffer
{
public:
    D3D11RHIFrameBuffer(Context* context);
    virtual ~D3D11RHIFrameBuffer() override;

    SResult OnBind() override;
    SResult OnUnbind() override;
    SResult Resolve();

    virtual void Clear(uint32_t flags = CBM_ALL, float4 const& clr = float4(0.0, 0.0, 0.0, 0.0), float depth = 1.0, int32_t stencil = 0) override;

    ID3D11RenderTargetView* GetRenderTargetView() const { return m_vD3dRednerTargets[0]; }

protected:
    std::vector<ID3D11RenderTargetView*> m_vD3dRednerTargets;
    ID3D11DepthStencilView* m_pD3dDepthStencilView = nullptr;
    D3D11_VIEWPORT m_stD3dViewport;
};

SEEK_NAMESPACE_END
