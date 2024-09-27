#pragma once

#include "kernel/kernel.h"
#include "rhi/base/framebuffer.h"

SEEK_NAMESPACE_BEGIN

class D3D11FrameBuffer : public FrameBuffer
{
public:
    D3D11FrameBuffer(Context* context);
    virtual ~D3D11FrameBuffer() override;

    SResult OnBind() override;
    SResult OnUnbind() override;
    SResult Resolve();

    ID3D11RenderTargetView* GetRenderTargetView() const { return m_vD3dRednerTargets[0]; }

protected:
    std::vector<ID3D11RenderTargetView*> m_vD3dRednerTargets;
    ID3D11DepthStencilView* m_pD3dDepthStencilView = nullptr;
    D3D11_VIEWPORT m_stD3dViewport;
};

SEEK_NAMESPACE_END
