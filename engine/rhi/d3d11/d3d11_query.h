#pragma once
#include "rhi/base/rhi_query.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/d3d11/d3d11_predeclare.h"


SEEK_NAMESPACE_BEGIN

class D3D11RHITimeQuery : public RHITimeQuery
{
  
public:
    D3D11RHITimeQuery(Context* context);
    virtual ~D3D11RHITimeQuery() override;

    bool Valid() const
    {
        return m_disjoint && m_begin && m_end;
    }
    virtual void Begin();
    virtual void End();
    virtual double TimeElapsedInMS();
    
private:
    void Create();

private:
    Context* m_pContext = nullptr;
    ID3D11QueryPtr m_disjoint;
    ID3D11QueryPtr m_begin;
    ID3D11QueryPtr m_end;
};



SEEK_NAMESPACE_END
