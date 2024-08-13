#pragma once

#include "util.h"
#include <map>
#include <array>

SEEK_NAMESPACE_BEGIN

class Timer final
{
public:
    Timer();
    void             Restart();
    static double    CurrentTimeSinceEpoch_S(); // microsecond

private:
    double m_dStartTime;
};

#if defined(__DISABLE_ALL_LOG__)

#define TIMER_BEG(var) double var = 0.0;
#define TIMER_END(var, name)

#define TIMER_FRAME_BEG(name)
#define TIMER_FRAME_END(name)
#define TIMER_FRAME_PRINT()

#else

#define TIMER_BEG(var)          double var = Timer::CurrentTimeSinceEpoch_S();
#define TIMER_END(var, name)    LOG_INFO("%s cost %.2lf ms", name, (Timer::CurrentTimeSinceEpoch_S() - var) * 1000);

void    TIMER_FRAME_BEG(std::string name);
void    TIMER_FRAME_END(std::string name);
void    TIMER_FRAME_PRINT();

#endif // defined(__DISABLE_ALL_LOG__)

SEEK_NAMESPACE_END
