#include "rhi/d3d12/d3d12_render_state.h"
#include "rhi/d3d12/d3d12_context.h"
#include "rhi/d3d12/d3d12_translate.h"
#include "kernel/context.h"
#include "math/color.h"

SEEK_NAMESPACE_BEGIN

D3D12RenderState::D3D12RenderState(Context* context, RasterizerStateDesc const& rs_desc,
	DepthStencilStateDesc const& ds_desc, BlendStateDesc const& bs_desc)
	: RHIRenderState(context, rs_desc, ds_desc, bs_desc)
{
	// Step1 : RasterizerState
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc = std::get<D3D12_GRAPHICS_PIPELINE_STATE_DESC>(m_vPipelineStateDesc);
	desc.RasterizerState.FillMode = D3D12Translate::TranslateFillMode(rs_desc.eFillMode);
	desc.RasterizerState.CullMode = D3D12Translate::TranslateCullMode(rs_desc.eCullMode);
	desc.RasterizerState.FrontCounterClockwise = rs_desc.bFrontFaceCCW;
	desc.RasterizerState.DepthBias = 0;
	desc.RasterizerState.DepthBiasClamp = 0;
	desc.RasterizerState.SlopeScaledDepthBias = 0;
	desc.RasterizerState.DepthClipEnable = rs_desc.bDepthClip;
	desc.RasterizerState.MultisampleEnable = m_pContext->GetNumSamples() > 1 ? TRUE : FALSE;
	desc.RasterizerState.AntialiasedLineEnable = false;
	desc.RasterizerState.ForcedSampleCount = 0;
	desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


	// Step2 : DepthStencil State
	desc.DepthStencilState.DepthEnable = ds_desc.bDepthEnable;
	desc.DepthStencilState.DepthWriteMask = ds_desc.bDepthWriteMask ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthStencilState.DepthFunc = D3D12Translate::TranslateCompareFunction(ds_desc.eDepthFunc);
	desc.DepthStencilState.StencilEnable = ds_desc.bFrontStencilEnable;
	desc.DepthStencilState.StencilReadMask = (uint8_t)(ds_desc.iFrontStencilReadMask);
	desc.DepthStencilState.StencilWriteMask = (uint8_t)(ds_desc.iFrontStencilWriteMask);

	desc.DepthStencilState.FrontFace.StencilFailOp = D3D12Translate::TranslateStencilOp(ds_desc.eFrontStencilFail);
	desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12Translate::TranslateStencilOp(ds_desc.eFrontStencilDepthFail);
	desc.DepthStencilState.FrontFace.StencilPassOp = D3D12Translate::TranslateStencilOp(ds_desc.eFrontStencilPass);
	desc.DepthStencilState.FrontFace.StencilFunc = D3D12Translate::TranslateCompareFunction(ds_desc.eFrontStencilFunction);

	desc.DepthStencilState.BackFace.StencilFailOp = D3D12Translate::TranslateStencilOp(ds_desc.eBackStencilFail);
	desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12Translate::TranslateStencilOp(ds_desc.eBackStencilDepthFail);
	desc.DepthStencilState.BackFace.StencilPassOp = D3D12Translate::TranslateStencilOp(ds_desc.eBackStencilPass);
	desc.DepthStencilState.BackFace.StencilFunc = D3D12Translate::TranslateCompareFunction(ds_desc.eBackStencilFunction);


	// Step3 : Blend State
	desc.BlendState.AlphaToCoverageEnable = bs_desc.bAlphaToCoverageEnable;
	desc.BlendState.IndependentBlendEnable = bs_desc.bIndependentBlendEnable;
	desc.SampleMask = bs_desc.iSampleMask;
	for (int i = 0; i < 8; ++i)
	{
		BlendStateDesc::TargetBlendDesc const& src = bs_desc.stTargetBlend[i];
		D3D12_RENDER_TARGET_BLEND_DESC& dst = desc.BlendState.RenderTarget[i];
		dst.BlendEnable = src.bBlendEnable;
		dst.LogicOpEnable = src.bLogicOpEnable;
		dst.SrcBlend = D3D12Translate::TranslateBlendFactor(src.eSrcBlendColor);
		dst.DestBlend = D3D12Translate::TranslateBlendFactor(src.eDstBlendColor);
		dst.BlendOp = D3D12Translate::TranslateBlendOp(src.eBlendOpColor);
		dst.SrcBlendAlpha = D3D12Translate::TranslateBlendFactor(src.eSrcBlendAlpha);
		dst.DestBlendAlpha = D3D12Translate::TranslateBlendFactor(src.eDstBlendAlpha);
		dst.BlendOpAlpha = D3D12Translate::TranslateBlendOp(src.eBlendOpAlpha);
		dst.LogicOp = D3D12_LOGIC_OP_NOOP;
		dst.RenderTargetWriteMask =
			((src.bColorWriteMask & CWM_Red) ? D3D12_COLOR_WRITE_ENABLE_RED : 0)
			| ((src.bColorWriteMask & CWM_Green) ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0)
			| ((src.bColorWriteMask & CWM_Blue) ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0)
			| ((src.bColorWriteMask & CWM_Alpha) ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0);
	}	

	desc.NodeMask = 0;
	desc.CachedPSO.pCachedBlob = nullptr;
	desc.CachedPSO.CachedBlobSizeInBytes = 0;
	desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
}
SResult D3D12RenderState::Active()
{
	D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
	ID3D12GraphicsCommandList* cmd_list =rc.D3DRenderCmdList();
	cmd_list->OMSetStencilRef(m_stRenderStateDesc.depthStencil.iFrontStencilRef);
	cmd_list->OMSetBlendFactor(&m_stRenderStateDesc.blend.fBlendFactor.x());	
	return S_Success;
}


D3D12Sampler::D3D12Sampler(Context* context, SamplerDesc const& desc)
    :RHISampler(context, desc)
{
	m_SampleDesc.Filter = D3D12Translate::TranslateTexFilterOp(desc.eFilterOp);
	m_SampleDesc.AddressU = D3D12Translate::TranslateAddressMode(desc.eAddrModeU);;
	m_SampleDesc.AddressV = D3D12Translate::TranslateAddressMode(desc.eAddrModeV);;
	m_SampleDesc.AddressW = D3D12Translate::TranslateAddressMode(desc.eAddrModeW);;
	m_SampleDesc.MipLODBias = desc.iMipMapLodBias;
	m_SampleDesc.MaxAnisotropy = 4;
	m_SampleDesc.ComparisonFunc = D3D12Translate::TranslateCompareFunction(desc.eCompareFun);

	float4 border_color = desc.cBoarderColor.ToFloat4();
	m_SampleDesc.BorderColor[0] = border_color[0];
	m_SampleDesc.BorderColor[1] = border_color[1];
	m_SampleDesc.BorderColor[2] = border_color[2];
	m_SampleDesc.BorderColor[3] = border_color[3];
	m_SampleDesc.MinLOD = desc.iMinLod;
	m_SampleDesc.MaxLOD = desc.iMaxLod;
}

SEEK_NAMESPACE_END
