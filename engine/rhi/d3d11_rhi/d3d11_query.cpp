#include "d3d11_query.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"
SEEK_NAMESPACE_BEGIN

D3D11TimerRHIQuery::D3D11TimerRHIQuery(Context* context)
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

D3D11TimerRHIQuery::~D3D11TimerRHIQuery()
{

}

void D3D11TimerRHIQueryExecutor::Begin(TimerRHIQueryPtr& timerRHIQuery)
{
    D3D11TimerRHIQuery* d3d11TimerRHIQuery = static_cast<D3D11TimerRHIQuery*>(timerRHIQuery.get());
    if (!d3d11TimerRHIQuery->Valid())
        return;

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    pDeviceContext->Begin(d3d11TimerRHIQuery->m_disjoint.Get());
    pDeviceContext->End(d3d11TimerRHIQuery->m_begin.Get());
}

void D3D11TimerRHIQueryExecutor::End(TimerRHIQueryPtr& timerRHIQuery)
{
    D3D11TimerRHIQuery* d3d11TimerRHIQuery = static_cast<D3D11TimerRHIQuery*>(timerRHIQuery.get());
    if (!d3d11TimerRHIQuery->Valid())
        return;

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    pDeviceContext->End(d3d11TimerRHIQuery->m_end.Get());
    pDeviceContext->End(d3d11TimerRHIQuery->m_disjoint.Get());

    std::weak_ptr<TimerRHIQuery> weakTimerRHIQuery = timerRHIQuery;
    //m_pContext->PendTimerRHIQuery([=]() {
    //    TimerRHIQueryPtr timerRHIQuery = weakTimerRHIQuery.lock();
    //    if (!timerRHIQuery)
    //        return;

    //    D3D11TimerRHIQuery* d3d11TimerRHIQuery = static_cast<D3D11TimerRHIQuery*>(timerRHIQuery.get());
    //    if (!d3d11TimerRHIQuery->Valid())
    //        return;

    //    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    //    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    //    uint64_t begin, end;
    //    while (S_OK != pDeviceContext->GetData(d3d11TimerRHIQuery->m_begin.Get(), &begin, sizeof(begin), 0));
    //    while (S_OK != pDeviceContext->GetData(d3d11TimerRHIQuery->m_end.Get(), &end, sizeof(end), 0));

    //    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint;
    //    HRESULT hr;
    //    while ((hr = pDeviceContext->GetData(d3d11TimerRHIQuery->m_disjoint.Get(), &disjoint, sizeof(disjoint), 0)) != S_OK);
    //    if (!disjoint.Disjoint)
    //    {
    //        d3d11TimerRHIQuery->m_bAvailable = true;
    //        d3d11TimerRHIQuery->m_TimeElapsedInMs = static_cast<double>((end - begin) * 1000) / disjoint.Frequency;
    //    }
    //    else
    //    {
    //        d3d11TimerRHIQuery->m_bAvailable = false;
    //        d3d11TimerRHIQuery->m_TimeElapsedInMs = 0.0;
    //    }
    //});
}

SEEK_NAMESPACE_END
