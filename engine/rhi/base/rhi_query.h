#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_shader.h"

SEEK_NAMESPACE_BEGIN

class RHIQuery
{
public:
	virtual ~RHIQuery() {}

	virtual void Begin() = 0;
	virtual void End() = 0;
};

struct RHITimerQuery
{
	virtual ~RHITimerQuery() {}
	
	virtual bool Available() = 0;
	virtual double GetTimeElapsedInMs() = 0;
};


SEEK_NAMESPACE_END