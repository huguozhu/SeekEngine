/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : d3d11_framebuffer.h
**
**      Brief                  : implement d3d11 framebuffer
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-05-28  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "kernel/kernel.h"
#include "rendering/framebuffer.h"

DVF_NAMESPACE_BEGIN

class D3D11FrameBuffer : public FrameBuffer
{
public:
    D3D11FrameBuffer(Context* context);
    virtual ~D3D11FrameBuffer() override;

    DVFResult OnBind() override;
    DVFResult OnUnbind() override;
    DVFResult Resolve();

    ID3D11RenderTargetView* GetRenderTargetView() const { return m_vD3dRednerTargets[0]; }

protected:
    std::vector<ID3D11RenderTargetView*> m_vD3dRednerTargets;
    ID3D11DepthStencilView* m_pD3dDepthStencilView = nullptr;
    D3D11_VIEWPORT m_stD3dViewport;
};

DVF_NAMESPACE_END
