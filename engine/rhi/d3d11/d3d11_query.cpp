#include "d3d11_query.h"
#include "rhi/d3d11/d3d11_rhi_context.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

D3D11RHITimeQuery::D3D11RHITimeQuery(Context* context)
    : RHITimeQuery(context)
{

}

D3D11RHITimeQuery::~D3D11RHITimeQuery()
{

}

void D3D11RHITimeQuery::Begin()
{
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    Create();
    pDeviceContext->Begin(m_disjoint.Get());
    pDeviceContext->End(m_begin.Get());
}

void D3D11RHITimeQuery::End()
{
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    pDeviceContext->End(m_end.Get());
    pDeviceContext->End(m_disjoint.Get());
}
double D3D11RHITimeQuery::TimeElapsedInMS()
{
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    uint64_t begin, end;
    while (S_OK != pDeviceContext->GetData(m_begin.Get(), &begin, sizeof(begin), 0));
    while (S_OK != pDeviceContext->GetData(m_end.Get(), &end, sizeof(end), 0));

    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint;
    while (S_OK != pDeviceContext->GetData(m_disjoint.Get(), &disjoint, sizeof(disjoint), 0));

    return disjoint.Disjoint ? -1 : static_cast<double>((end - begin) * 1000) / disjoint.Frequency;
}
void D3D11RHITimeQuery::Create()
{
    if (!m_begin || !m_end || !m_disjoint)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        ID3D11Device* pDevice = rc.GetD3D11Device();

        D3D11_QUERY_DESC desc;
        desc.Query = D3D11_QUERY_TIMESTAMP;
        desc.MiscFlags = 0;
        pDevice->CreateQuery(&desc, m_begin.ReleaseAndGetAddressOf());
        pDevice->CreateQuery(&desc, m_end.ReleaseAndGetAddressOf());

        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        pDevice->CreateQuery(&desc, m_disjoint.ReleaseAndGetAddressOf());
    }
}

SEEK_NAMESPACE_END
