#pragma once

#include "kernel/kernel.h"
#include "rhi/shader.h"

SEEK_NAMESPACE_BEGIN

class Query
{
public:
	virtual ~Query() {}

	virtual void Begin() = 0;
	virtual void End() = 0;
};

struct TimerQuery
{
	virtual ~TimerQuery() {}
	
	virtual bool Available() = 0;
	virtual double GetTimeElapsedInMs() = 0;
};

//struct CPUTimerQuery : public TimerQuery
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
//struct CPUTimerQueryExecutor
//{
//	void Begin(TimerQueryPtr& timerQuery)
//	{
//		CPUTimerQuery* cpuTQ = static_cast<CPUTimerQuery*>(timerQuery.get());
//		cpuTQ->beginTime = Timer::CurrentTimeSinceEpoch_S();
//	}
//
//	void End(TimerQueryPtr& timerQuery)
//	{
//		CPUTimerQuery* cpuTQ = static_cast<CPUTimerQuery*>(timerQuery.get());
//		cpuTQ->endTime = Timer::CurrentTimeSinceEpoch_S();
//		cpuTQ->timeElapsed = cpuTQ->endTime - cpuTQ->beginTime;
//		cpuTQ->available = true;
//	}
//};

SEEK_NAMESPACE_END