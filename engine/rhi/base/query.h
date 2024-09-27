#pragma once

#include "kernel/kernel.h"
#include "rhi/base/shader.h"

SEEK_NAMESPACE_BEGIN

class RHIQuery
{
public:
	virtual ~RHIQuery() {}

	virtual void Begin() = 0;
	virtual void End() = 0;
};

struct TimerRHIQuery
{
	virtual ~TimerRHIQuery() {}
	
	virtual bool Available() = 0;
	virtual double GetTimeElapsedInMs() = 0;
};

//struct CPUTimerRHIQuery : public TimerRHIQuery
//{
//	bool Available()
//	{
//		return available;
//	}
//
//	double GetTimeElapsedInMs()
//	{
//		return timeElapsed;
//	}
//
//	double beginTime = 0.0;
//	double endTime = 0.0;
//	double timeElapsed = 0.0;
//	bool available = false;
//};
//
//struct CPUTimerRHIQueryExecutor
//{
//	void Begin(TimerRHIQueryPtr& timerRHIQuery)
//	{
//		CPUTimerRHIQuery* cpuTQ = static_cast<CPUTimerRHIQuery*>(timerRHIQuery.get());
//		cpuTQ->beginTime = Timer::CurrentTimeSinceEpoch_S();
//	}
//
//	void End(TimerRHIQueryPtr& timerRHIQuery)
//	{
//		CPUTimerRHIQuery* cpuTQ = static_cast<CPUTimerRHIQuery*>(timerRHIQuery.get());
//		cpuTQ->endTime = Timer::CurrentTimeSinceEpoch_S();
//		cpuTQ->timeElapsed = cpuTQ->endTime - cpuTQ->beginTime;
//		cpuTQ->available = true;
//	}
//};

SEEK_NAMESPACE_END