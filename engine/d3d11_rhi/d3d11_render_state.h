/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : d3d11_render_state.h
**
**      Brief                  : d3d11 rendering state and d3d11 sampler
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-05-28  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "rendering_d3d11/d3d11_predeclare.h"
#include "rendering/render_state.h"

DVF_NAMESPACE_BEGIN

class D3D11RenderState : public RenderState
{
public:
    D3D11RenderState(Context* context, RasterizerStateDesc const& rs_desc,
        DepthStencilStateDesc const& ds_desc, BlendStateDesc const& bs_desc);
    D3D11RenderState(Context* context, RenderStateDesc const& desc)
        : D3D11RenderState(context, desc.rasterizer, desc.depthStencil, desc.blend)
    {}
    virtual ~D3D11RenderState() override;
    DVFResult               Active();

    ID3D11RasterizerState* GetD3D11RasterizerState() const { return m_pD3D11RasterizerState.Get(); }
    ID3D11DepthStencilState* GetD3D11DepthStencilState() const { return m_pD3D11DepthStencilState.Get(); }
    ID3D11BlendState* GetD3D11BlendState() const { return m_pD3D11BlendState.Get(); }

protected:
    ID3D11RasterizerStatePtr m_pD3D11RasterizerState = nullptr;
    ID3D11DepthStencilStatePtr m_pD3D11DepthStencilState = nullptr;
    ID3D11BlendStatePtr m_pD3D11BlendState = nullptr;
};

class D3D11Sampler : public Sampler
{
public:
    D3D11Sampler(Context* context, SamplerDesc const& desc);
    virtual ~D3D11Sampler() override;
    ID3D11SamplerState* GetD3D11SamplerState() const { return m_pD3D11SamplerState.Get(); }

private:
    ID3D11SamplerStatePtr m_pD3D11SamplerState = nullptr;
};


DVF_NAMESPACE_END
