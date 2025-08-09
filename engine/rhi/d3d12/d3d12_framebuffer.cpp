#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_framebuffer.h"
#include "rhi/d3d12/d3d12_context.h"
#include "rhi/d3d12/d3d12_resource.h"
#include "rhi/d3d12/d3d12_render_view.h"
#include "rhi/d3d_common/d3d_common_translate.h"
#include "rhi/base/viewport.h"
#include "math/hash.h"

SEEK_NAMESPACE_BEGIN

D3D12FrameBuffer::D3D12FrameBuffer(Context* context)
    :RHIFrameBuffer(context)
{
    m_stD3dViewport.MinDepth = 0.0f;
    m_stD3dViewport.MaxDepth = 1.0;
}
SResult D3D12FrameBuffer::OnBind()
{
	return S_Success;
}
SResult D3D12FrameBuffer::OnUnbind()
{
    return S_Success;
}
SResult D3D12FrameBuffer::Resolve()
{
    return S_Success;
}
void D3D12FrameBuffer::Clear(uint32_t flags, float4 const& clr, float depth, int32_t stencil)
{

}
void D3D12FrameBuffer::ClearRenderTarget(Attachment att, float4 const& clr)
{

}

void D3D12FrameBuffer::BindBarrier(ID3D12GraphicsCommandList* cmd_list)
{
    if (m_bViewDirty)
    {
        this->UpdateHashValue();
        m_bViewDirty = false;
    }

    for (uint32_t i = 0; i < m_vD3dRtvResources.size(); ++i)
    {
        D3D12Resource* pRes     = std::get<0>(m_vD3dRtvResources[i]);
        uint32_t first_subres   = std::get<1>(m_vD3dRtvResources[i]);
        uint32_t num_subres     = std::get<2>(m_vD3dRtvResources[i]);
        for (uint32_t j = 0; j < num_subres; ++j)
            pRes->UpdateResourceBarrier(cmd_list, first_subres + j, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
    if (std::get<0>(m_DsvResource))
    {
        D3D12Resource* pRes     = std::get<0>(m_DsvResource);
        uint32_t first_subres   = std::get<1>(m_DsvResource);
        uint32_t num_subres     = std::get<2>(m_DsvResource);
        for (uint32_t j = 0; j < num_subres; ++j)
            pRes->UpdateResourceBarrier(cmd_list, first_subres + j, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }


}
void D3D12FrameBuffer::SetRenderTargets(ID3D12GraphicsCommandList* cmd_list)
{
	if (m_bViewDirty)
	{
		this->UpdateHashValue();
		m_bViewDirty = false;
	}

    m_vD3dRtvResources.clear();
    m_vD3dRtvCpuHandles.resize(m_vRenderTargets.size());
}
size_t D3D12FrameBuffer::GetPsoHashValue()
{
	if (m_bViewDirty)
	{
		this->UpdateHashValue();
		m_bViewDirty = false;
	}
	return m_iPsoHashValue;
}
void D3D12FrameBuffer::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc)
{
	if (m_bViewDirty)
	{
		this->UpdateHashValue();
		m_bViewDirty = false;
	}

	pso_desc.NumRenderTargets = m_iNumRtvs;
	for (uint32_t i = 0; i < MAX_COLOR_ATTACHMENTS; ++i)
	{
		pso_desc.RTVFormats[i] = m_vRtvFormats[i];
	}
	pso_desc.DSVFormat = m_eDsvFormat;
	pso_desc.SampleDesc.Count = m_iNumSamples;
	pso_desc.SampleDesc.Quality = 0;
}
void D3D12FrameBuffer::UpdateHashValue()
{
	m_vD3dRtvResources.clear();
	m_vD3dRtvCpuHandles.clear();
	m_vD3dRtvCpuHandles.resize(m_vRenderTargets.size());
	for (uint32_t i = 0; i < m_vRenderTargets.size(); ++i)
	{
		if (m_vRenderTargets[i])
		{
			D3D12RenderTargetView& v = (D3D12RenderTargetView&)(*m_vRenderTargets[i]);
			m_vD3dRtvResources.push_back(std::make_tuple<D3D12Resource*, uint32_t, uint32_t>(v.GetResource().get(), v.GetFirstSubRes(), v.GetNumSubRes()));
			m_vD3dRtvCpuHandles[i] = v.GetD3DRtv()->Handle();
		}
		else
		{
			m_vD3dRtvCpuHandles[i].ptr = (SIZE_T)0;
		}
	}

	if (m_pDepthStencilView)
	{
		D3D12DepthStencilView& v = (D3D12DepthStencilView&)(*m_pDepthStencilView);
		
		m_DsvResource = std::make_tuple<D3D12Resource*, uint32_t, uint32_t>(v.GetResource().get(), v.GetFirstSubRes(), v.GetNumSubRes());

		m_D3dSdvHandle = v.GetD3DDsv()->Handle();
		m_D3dSdvHandlePtr = &m_D3dSdvHandle;
	}
	else
	{
		m_DsvResource = std::make_tuple<D3D12Resource*, uint32_t, uint32_t>(nullptr, 0, 0);
		m_D3dSdvHandlePtr = nullptr;
	}

	m_stD3dViewport.TopLeftX = static_cast<float>(m_stViewport.Left());
	m_stD3dViewport.TopLeftY = static_cast<float>(m_stViewport.Top());
	m_stD3dViewport.Width = static_cast<float>(m_stViewport.Width());
	m_stD3dViewport.Height = static_cast<float>(m_stViewport.Height());

	m_iPsoHashValue = 0;
	m_iNumRtvs = 0;
	m_vRtvFormats.fill(DXGI_FORMAT_UNKNOWN);
	m_iNumSamples = 0;
	for (size_t i = 0; i < m_vRenderTargets.size(); ++i)
	{
		if (m_vRenderTargets[i])
		{
			RHIRenderTargetView* view = m_vRenderTargets[i].get();
			PixelFormat fmt = view->Format();
			HashCombine(m_iPsoHashValue, fmt);
			m_vRtvFormats[i] = D3DCommonTranslate::TranslateToPlatformFormat(fmt);
			m_iNumRtvs = static_cast<uint32_t>(i + 1);
			m_iNumSamples = view->NumSamples();
		}
	}
	{
		RHIDepthStencilView* view = m_pDepthStencilView.get();
		if (view)
		{
			auto fmt = view->Format();
			HashCombine(m_iPsoHashValue, fmt);
			m_eDsvFormat = D3DCommonTranslate::TranslateToPlatformFormat(fmt);
		}
		else
		{
			m_eDsvFormat = DXGI_FORMAT_UNKNOWN;
		}
	}

	HashCombine(m_iPsoHashValue, m_iNumSamples);
}
SEEK_NAMESPACE_END