#include "rendering_d3d11/d3d11_predeclare.h"
#include "rendering_d3d11/d3d11_render_state.h"
#include "rendering_d3d11/d3d11_render_context.h"
#include "rendering_d3d11/d3d11_translate.h"

#include "kernel/context.h"

#define DVF_MACRO_FILE_UID 9     // this code is auto generated, don't touch it!!!

DVF_NAMESPACE_BEGIN

D3D11RenderState::D3D11RenderState(Context* context, RasterizerStateDesc const& rs_desc,
    DepthStencilStateDesc const& ds_desc, BlendStateDesc const& bs_desc)
    : RenderState(context, rs_desc, ds_desc, bs_desc)
{
    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();

    // Step1 : RasterizerState
    D3D11_RASTERIZER_DESC d3d_rs_desc;
    zm_memset_s(&d3d_rs_desc, sizeof(d3d_rs_desc), 0, sizeof(d3d_rs_desc));
    d3d_rs_desc.FillMode = D3D11Translate::TranslateFillMode(rs_desc.eFillMode);
    d3d_rs_desc.CullMode = D3D11Translate::TranslateCullMode(rs_desc.eCullMode);
    d3d_rs_desc.FrontCounterClockwise = rs_desc.bFrontFaceCCW;
    d3d_rs_desc.DepthBias = 0;
    d3d_rs_desc.DepthBiasClamp = 0;
    d3d_rs_desc.SlopeScaledDepthBias = 0;
    d3d_rs_desc.DepthClipEnable = rs_desc.bDepthClip;
    d3d_rs_desc.ScissorEnable = rs_desc.bScissorEnable;
    d3d_rs_desc.MultisampleEnable = m_pContext->GetNumSamples() > 1 ? TRUE : FALSE;
    d3d_rs_desc.AntialiasedLineEnable = false;
    HRESULT hr = pDevice->CreateRasterizerState(&d3d_rs_desc, m_pD3D11RasterizerState.GetAddressOf());
    if (FAILED(hr))
        LOG_ERROR("Create D3D11 Rasterizer State Failed");


    // Step2 : DepthStencil State
    D3D11_DEPTH_STENCIL_DESC d3d_ds_desc;
    zm_memset_s(&d3d_ds_desc, sizeof(d3d_ds_desc), 0, sizeof(d3d_ds_desc));
    d3d_ds_desc.DepthEnable = ds_desc.bDepthEnable;
    d3d_ds_desc.DepthWriteMask = ds_desc.bDepthWriteMask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    d3d_ds_desc.DepthFunc = D3D11Translate::TranslateCompareFunction(ds_desc.eDepthFunc);

    d3d_ds_desc.StencilEnable = ds_desc.bFrontStencilEnable || ds_desc.bBackStencilEnable;
    d3d_ds_desc.StencilReadMask = static_cast<UINT>(ds_desc.iFrontStencilReadMask);
    d3d_ds_desc.StencilWriteMask = static_cast<UINT>(ds_desc.iFrontStencilWriteMask);
    d3d_ds_desc.FrontFace.StencilFailOp = D3D11Translate::TranslateStencilOp(ds_desc.eFrontStencilFail);
    d3d_ds_desc.FrontFace.StencilDepthFailOp = D3D11Translate::TranslateStencilOp(ds_desc.eFrontStencilDepthFail);
    d3d_ds_desc.FrontFace.StencilPassOp = D3D11Translate::TranslateStencilOp(ds_desc.eFrontStencilPass);
    d3d_ds_desc.FrontFace.StencilFunc = D3D11Translate::TranslateCompareFunction(ds_desc.eFrontStencilFunction);

    if (ds_desc.bBackStencilEnable)
    {
        d3d_ds_desc.BackFace.StencilFailOp = D3D11Translate::TranslateStencilOp(ds_desc.eBackStencilFail);
        d3d_ds_desc.BackFace.StencilDepthFailOp = D3D11Translate::TranslateStencilOp(ds_desc.eBackStencilDepthFail);
        d3d_ds_desc.BackFace.StencilPassOp = D3D11Translate::TranslateStencilOp(ds_desc.eBackStencilPass);
        d3d_ds_desc.BackFace.StencilFunc = D3D11Translate::TranslateCompareFunction(ds_desc.eBackStencilFunction);
    }
    else
    {
        d3d_ds_desc.BackFace = d3d_ds_desc.FrontFace;
    }
    hr = pDevice->CreateDepthStencilState(&d3d_ds_desc, m_pD3D11DepthStencilState.GetAddressOf());
    if (FAILED(hr))
        LOG_ERROR("Create D3D11 Depth-Stencil State Failed");

    // Step3 : Blend State
    D3D11_BLEND_DESC d3d_blend_desc;
    zm_memset_s(&d3d_blend_desc, sizeof(d3d_blend_desc), 0, sizeof(d3d_blend_desc));
    d3d_blend_desc.AlphaToCoverageEnable = bs_desc.bAlphaToCoverageEnable;
    d3d_blend_desc.IndependentBlendEnable = bs_desc.bIndependentBlendEnable;

    for (uint32_t i = 0; i < 8; i++)
    {
        BlendStateDesc::TargetBlendDesc const& src = bs_desc.stTargetBlend[i];
        D3D11_RENDER_TARGET_BLEND_DESC& dst = d3d_blend_desc.RenderTarget[i];
        dst.BlendEnable = src.bBlendEnable;
            //src.eBlendOpColor != BlendOperation::Add || src.eSrcBlendColor != BlendFactor::One || src.eDstBlendColor != BlendFactor::Zero ||
            //src.eBlendOpAlpha != BlendOperation::Add || src.eSrcBlendAlpha != BlendFactor::One || src.eDstBlendAlpha != BlendFactor::Zero;
        dst.BlendOp = D3D11Translate::TranslateBlendOp(src.eBlendOpColor);
        dst.SrcBlend = D3D11Translate::TranslateBlendFactor(src.eSrcBlendColor);
        dst.DestBlend = D3D11Translate::TranslateBlendFactor(src.eDstBlendColor);

        dst.BlendOpAlpha = D3D11Translate::TranslateBlendOp(src.eBlendOpAlpha);
        dst.SrcBlendAlpha = D3D11Translate::TranslateBlendFactor(src.eSrcBlendAlpha);
        dst.DestBlendAlpha = D3D11Translate::TranslateBlendFactor(src.eDstBlendAlpha);

        dst.RenderTargetWriteMask =
              ((src.bColorWriteMask & CWM_Red)      ? D3D11_COLOR_WRITE_ENABLE_RED  : 0)
            | ((src.bColorWriteMask & CWM_Green)    ? D3D11_COLOR_WRITE_ENABLE_GREEN: 0)
            | ((src.bColorWriteMask & CWM_Blue)     ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0)
            | ((src.bColorWriteMask & CWM_Alpha)    ? D3D11_COLOR_WRITE_ENABLE_ALPHA: 0);
    }

    hr = pDevice->CreateBlendState(&d3d_blend_desc, m_pD3D11BlendState.GetAddressOf());
    if (FAILED(hr))
        LOG_ERROR("Create D3D11 Blend State Failed");
}

D3D11RenderState::~D3D11RenderState()
{
    m_pD3D11RasterizerState.Reset();
    m_pD3D11DepthStencilState.Reset();
    m_pD3D11BlendState.Reset();
}

DVFResult D3D11RenderState::Active()
{
    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
    if (m_pD3D11RasterizerState && m_pD3D11DepthStencilState && m_pD3D11BlendState)
    {
        rc.SetD3DRasterizerState(m_pD3D11RasterizerState.Get());
        rc.SetD3DDepthStencilState(m_pD3D11DepthStencilState.Get(), m_stRenderStateDesc.depthStencil.iFrontStencilRef);
        rc.SetD3DBlendState(m_pD3D11BlendState.Get(), m_stRenderStateDesc.blend.fBlendFactor, m_stRenderStateDesc.blend.iSampleMask);
        return DVF_Success;
    }
    else
    {
        LOG_ERROR("invalid render state, rasterize state:%p, depth stencil state:%p, blend state:%p",
            m_pD3D11RasterizerState.Get(), m_pD3D11DepthStencilState.Get(), m_pD3D11BlendState.Get());
        return ERR_INVALID_ARG;
    }
}

D3D11Sampler::D3D11Sampler(Context* context, SamplerDesc const& desc)
    : Sampler(context, desc)
{
    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_SAMPLER_DESC d3d_sampler_desc;
    zm_memset_s(&d3d_sampler_desc, sizeof(d3d_sampler_desc), 0, sizeof(d3d_sampler_desc));

    d3d_sampler_desc.Filter = D3D11Translate::TranslateTexFilterOp(desc.eFilterOp);
    d3d_sampler_desc.AddressU = D3D11Translate::TranslateAddressMode(desc.eAddrModeU);
    d3d_sampler_desc.AddressV = D3D11Translate::TranslateAddressMode(desc.eAddrModeV);
    d3d_sampler_desc.AddressW = D3D11Translate::TranslateAddressMode(desc.eAddrModeW);
    d3d_sampler_desc.MipLODBias = desc.iMipMapLodBias;
    d3d_sampler_desc.MaxAnisotropy = 4;
    d3d_sampler_desc.ComparisonFunc = D3D11Translate::TranslateCompareFunction(desc.eCompareFun);
    float4 border_color = m_stSamplerDesc.cBoarderColor.ToFloat4();
    d3d_sampler_desc.BorderColor[0] = border_color[0];
    d3d_sampler_desc.BorderColor[1] = border_color[1];
    d3d_sampler_desc.BorderColor[2] = border_color[2];
    d3d_sampler_desc.BorderColor[3] = border_color[3];
    d3d_sampler_desc.MinLOD = desc.iMinLod;
    d3d_sampler_desc.MaxLOD = desc.iMaxLod;

    HRESULT hr = pDevice->CreateSamplerState(&d3d_sampler_desc, m_pD3D11SamplerState.GetAddressOf());
    if (FAILED(hr))
        LOG_ERROR("Create D3D11 Sampler State Failed");
}
D3D11Sampler::~D3D11Sampler()
{
    m_pD3D11SamplerState.Reset();
}






DVF_NAMESPACE_END


#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
