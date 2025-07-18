#include "d3d11_fence.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"
SEEK_NAMESPACE_BEGIN

uint64_t D3D11RHIFence::Signal()
{
	D3D11RHIContext* pD3D11RHIContext = static_cast<D3D11RHIContext*>(&m_pContext->RHIContextInstance());
	ID3D11Device* d3d_device = pD3D11RHIContext->GetD3D11Device();
	ID3D11DeviceContext* d3d_imm_ctx = pD3D11RHIContext->GetD3D11DeviceContext();

	D3D11_QUERY_DESC desc;
	desc.Query = D3D11_QUERY_EVENT;
	desc.MiscFlags = 0;

	ID3D11QueryPtr query;
	d3d_device->CreateQuery(&desc, query.GetAddressOf());

	uint64_t const value = m_iFenceVal;
	++m_iFenceVal;

	d3d_imm_ctx->End(query.Get());

	m_mFences[value] = std::move(query);

	return value;
}
void D3D11RHIFence::Wait(uint64_t value)
{
	auto iter = m_mFences.find(value);
	if (iter != m_mFences.end())
	{
		D3D11RHIContext* pD3D11RHIContext = static_cast<D3D11RHIContext*>(&m_pContext->RHIContextInstance());
		ID3D11DeviceContext* d3d_imm_ctx = pD3D11RHIContext->GetD3D11DeviceContext();

		uint32_t ret;
		while (S_OK != d3d_imm_ctx->GetData(iter->second.Get(), &ret, sizeof(ret), 0));
		m_mFences.erase(iter);
	}
}
bool D3D11RHIFence::IsCompleted(uint64_t value)
{
	auto iter = m_mFences.find(value);
	if (iter == m_mFences.end())
	{
		return true;
	}
	else
	{
		D3D11RHIContext* pD3D11RHIContext = static_cast<D3D11RHIContext*>(&m_pContext->RHIContextInstance());
		ID3D11DeviceContext* d3d_imm_ctx = pD3D11RHIContext->GetD3D11DeviceContext();

		uint32_t ret;
		HRESULT hr = d3d_imm_ctx->GetData(iter->second.Get(), &ret, sizeof(ret), 0);
		return (S_OK == hr);
	}
}
SEEK_NAMESPACE_END
