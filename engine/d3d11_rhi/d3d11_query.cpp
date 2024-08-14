#include "d3d11_query.h"
#include "d3d11_rhi/d3d11_rhi_context.h"
SEEK_NAMESPACE_BEGIN

D3D11TimerQuery::D3D11TimerQuery(Context* context)
    : m_pContext(context)
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

D3D11TimerQuery::~D3D11TimerQuery()
{

}

void D3D11TimerQueryExecutor::Begin(TimerQueryPtr& timerQuery)
{
    D3D11TimerQuery* d3d11TimerQuery = static_cast<D3D11TimerQuery*>(timerQuery.get());
    if (!d3d11TimerQuery->Valid())
        return;

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    pDeviceContext->Begin(d3d11TimerQuery->m_disjoint.Get());
    pDeviceContext->End(d3d11TimerQuery->m_begin.Get());
}

void D3D11TimerQueryExecutor::End(TimerQueryPtr& timerQuery)
{
    D3D11TimerQuery* d3d11TimerQuery = static_cast<D3D11TimerQuery*>(timerQuery.get());
    if (!d3d11TimerQuery->Valid())
        return;

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    pDeviceContext->End(d3d11TimerQuery->m_end.Get());
    pDeviceContext->End(d3d11TimerQuery->m_disjoint.Get());

    std::weak_ptr<TimerQuery> weakTimerQuery = timerQuery;
    //m_pContext->PendTimerQuery([=]() {
    //    TimerQueryPtr timerQuery = weakTimerQuery.lock();
    //    if (!timerQuery)
    //        return;

    //    D3D11TimerQuery* d3d11TimerQuery = static_cast<D3D11TimerQuery*>(timerQuery.get());
    //    if (!d3d11TimerQuery->Valid())
    //        return;

    //    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    //    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    //    uint64_t begin, end;
    //    while (S_OK != pDeviceContext->GetData(d3d11TimerQuery->m_begin.Get(), &begin, sizeof(begin), 0));
    //    while (S_OK != pDeviceContext->GetData(d3d11TimerQuery->m_end.Get(), &end, sizeof(end), 0));

    //    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint;
    //    HRESULT hr;
    //    while ((hr = pDeviceContext->GetData(d3d11TimerQuery->m_disjoint.Get(), &disjoint, sizeof(disjoint), 0)) != S_OK);
    //    if (!disjoint.Disjoint)
    //    {
    //        d3d11TimerQuery->m_bAvailable = true;
    //        d3d11TimerQuery->m_TimeElapsedInMs = static_cast<double>((end - begin) * 1000) / disjoint.Frequency;
    //    }
    //    else
    //    {
    //        d3d11TimerQuery->m_bAvailable = false;
    //        d3d11TimerQuery->m_TimeElapsedInMs = 0.0;
    //    }
    //});
}

SEEK_NAMESPACE_END
