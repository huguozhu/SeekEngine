#include "timer.h"
#include "log.h"
#include "math/math_utility.h"
#include <limits>
#include <chrono>
#include <sstream>

SEEK_NAMESPACE_BEGIN

Timer::Timer()
{
    this->Restart();
}

void Timer::Restart()
{
    m_dStartTime = this->CurrentTimeSinceEpoch_S();
}

double Timer::CurrentTimeSinceEpoch_S()
{
    std::chrono::high_resolution_clock::time_point const tp = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(tp.time_since_epoch()).count();
}


#if !defined(__DISABLE_ALL_LOG__)

struct TimerStatistics
{
private:
    struct TimerStamp
    {
#define TimerStampLen   300
        double                              tmp = 0;
        int32_t                             idx = -1;
        std::array<double, TimerStampLen>   stamps;

        double                              curr = 0;
        double                              total_sum = 0;
        double                              total_max = 0;
        double                              total_min = 9999999;
        int32_t                             total_cnt = 0;

        inline void                         Beg();
        inline void                         End();
        inline void                         Reset();
    };

    std::vector<std::pair<std::string, TimerStamp>>     times;
    uint64_t                                            frameid = 0;

public:
    void            Beg(std::string name);
    void            End(std::string name);
    void            Print();
};
static TimerStatistics g_TimerStatistics;

inline void TimerStatistics::TimerStamp::Beg()
{
    tmp = Timer::CurrentTimeSinceEpoch_S();
}
inline void TimerStatistics::TimerStamp::End()
{
    idx = (idx + 1) % TimerStampLen;
    stamps[idx] = curr = (Timer::CurrentTimeSinceEpoch_S() - tmp) * 1000;
    tmp = 0;

    total_sum += stamps[idx];
    total_max = Math::Max<double>(total_max, curr);
    total_min = Math::Min<double>(total_min, curr);
    total_cnt++;
}
inline void TimerStatistics::TimerStamp::Reset()
{
    tmp = curr = 0;
}
void TimerStatistics::Beg(std::string name)
{
    for (auto& n_v : times)
    {
        if (n_v.first == name)
        {
            n_v.second.Beg();
            return;
        }
    }
    times.push_back(std::make_pair(name, TimerStamp()));
    times[times.size() - 1].second.Beg();
}
void TimerStatistics::End(std::string name)
{
    for (auto& n_v : times)
    {
        if (n_v.first == name)
        {
            n_v.second.End();
            return;
        }
    }
}
void TimerStatistics::Print()
{
    if (frameid++ % 100 != 99)
        return;

    for (auto it = times.begin(); it != times.end(); it++)
    {
        TimerStamp& ts = it->second;
        int32_t total_cnt = ts.total_cnt == 0 ? 1 : ts.total_cnt;
        double short_sum = 0;
        double short_max = 0;
        double short_min = 99999999;
        int64_t short_cnt = Math::Min<int64_t>(total_cnt, TimerStampLen);
        for (int32_t i = 0; i < short_cnt; i++)
        {
            short_sum += ts.stamps[i];
            short_max = Math::Max<double>(short_max, ts.stamps[i]);
            short_min = Math::Min<double>(short_min, ts.stamps[i]);
        }

        std::ostringstream string_stream;
        string_stream.precision(2);
        string_stream << it->first << " {"
            << "cur:" << ts.curr << ", "
            << "past" << short_cnt
            << ":<avg:" << short_sum / short_cnt
            << ",max:" << short_max
            << ",min:" << short_min
            << ">, total:" << total_cnt
            << "<avg:" << ts.total_sum / total_cnt
            << ",max:" << ts.total_max
            << ",min:" << ts.total_min
            << ">}";

        LOG_INFO("SEEK Time: %s", string_stream.str().c_str());
    }

    for (auto it = times.begin(); it != times.end(); it++)
    {
        it->second.Reset();
    }
}

void    TIMER_FRAME_BEG(std::string name)   { g_TimerStatistics.Beg(name); }
void    TIMER_FRAME_END(std::string name)   { g_TimerStatistics.End(name); }
void    TIMER_FRAME_PRINT()                 { g_TimerStatistics.Print(); }

#endif // !defined(__DISABLE_ALL_LOG__)

SEEK_NAMESPACE_END
