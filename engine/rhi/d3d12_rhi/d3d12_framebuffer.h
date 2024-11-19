#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_framebuffer.h"

SEEK_NAMESPACE_BEGIN

class D3D12RHIFrameBuffer : public RHIFrameBuffer
{
public:
    D3D12RHIFrameBuffer(Context* context);
    virtual ~D3D12RHIFrameBuffer() override;

    SResult OnBind() override;
    SResult OnUnbind() override;
    SResult Resolve();

    //ID3D12RenderTargetView* GetRenderTargetView() const { return m_vD3dRednerTargets[0]; }

protected:
    //std::vector<D3D12Resource*> m_vD3dRednerTargets;
    //ID3D11DepthStencilView* m_pD3dDepthStencilView = nullptr;
    //D3D11_VIEWPORT m_stD3dViewport;
};

SEEK_NAMESPACE_END
