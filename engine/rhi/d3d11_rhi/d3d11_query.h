#pragma once
#include "rhi/base/rhi_definition.h"
#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/base/rhi_query.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

class D3D11RHITimerQuery : public RHITimerQuery
{
    Context* m_pContext = nullptr;
public:
    D3D11RHITimerQuery(Context* context);
    virtual ~D3D11RHITimerQuery() override;

    bool Valid() const
    {
        return m_disjoint && m_begin && m_end;
    }

    virtual bool Available() override
    {
        return m_bAvailable;
    }

    virtual double GetTimeElapsedInMs() override
    {
        return m_TimeElapsedInMs;
    }
    
    ID3D11QueryPtr m_disjoint;
    ID3D11QueryPtr m_begin;
    ID3D11QueryPtr m_end;

    bool m_bAvailable = false;
    double m_TimeElapsedInMs = 0.0;
};

class D3D11RHITimerQueryExecutor
{
public:
    D3D11RHITimerQueryExecutor(Context* context)
        : m_pContext(context)
    { }

    void Begin(RHITimerQueryPtr& timerRHIQuery);
    void End(RHITimerQueryPtr& timerRHIQuery);

private:
    Context* m_pContext = nullptr;
};

SEEK_NAMESPACE_END
