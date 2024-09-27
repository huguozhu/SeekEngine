#pragma once
#include "rhi/base/render_definition.h"
#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/base/query.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

class D3D11TimerQuery : public TimerQuery
{
    Context* m_pContext = nullptr;
public:
    D3D11TimerQuery(Context* context);
    virtual ~D3D11TimerQuery() override;

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

class D3D11TimerQueryExecutor
{
public:
    D3D11TimerQueryExecutor(Context* context)
        : m_pContext(context)
    { }

    void Begin(TimerQueryPtr& timerQuery);
    void End(TimerQueryPtr& timerQuery);

private:
    Context* m_pContext = nullptr;
};

SEEK_NAMESPACE_END
