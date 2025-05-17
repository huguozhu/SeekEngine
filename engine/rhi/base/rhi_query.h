#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_shader.h"

SEEK_NAMESPACE_BEGIN

class RHIQuery
{
public:
	virtual void Begin() = 0;
	virtual void End() = 0;
};

class RHITimeQuery : public RHIQuery
{
public:
	RHITimeQuery(Context* context)
		:m_pContext(context)
	{
	}
	virtual ~RHITimeQuery() {}	
	virtual double TimeElapsedInMS() = 0;

protected:
	Context* m_pContext = nullptr;
};


SEEK_NAMESPACE_END