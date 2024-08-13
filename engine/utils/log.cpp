#include "utils/log.h"
#include <stdio.h>
#include <cstdarg>

SEEK_NAMESPACE_BEGIN

static bool bsetvbuf = false;
static char log_buf[LOG_MAX_LEN];

static void TRACECALLBACK(TRACE_LEVEL level, const char* szLog)
{
#ifndef DEBUG
    if (!bsetvbuf) {
        setvbuf(stdout, log_buf, _IOFBF, sizeof(log_buf));
        bsetvbuf = true;
    }
#endif

    switch (level)
    {
        case LevelInfo:
            fprintf(stdout, "[SEEK][INFO] %s\n", szLog);
            break;
        case LevelWarning:
            fprintf(stdout, "[SEEK][WARNING] %s\n", szLog);
            break;
        case LevelError:
            fprintf(stdout, "[SEEK][ERROR] %s\n", szLog);
            break;
        default:
            fprintf(stdout, "[SEEK][UNKNOW] %s\n", szLog);
            break;
    }
#if defined(SEEK_PLATFORM_LINUX)
    fflush(stdout);
#endif
}

static FPTRACECALLBACK g_pTraceObserver = TRACECALLBACK;
void dvfTrace::Config(FPTRACECALLBACK cb)
{
    g_pTraceObserver = cb;
}

void dvfTrace::TraceLog(TRACE_LEVEL level, const char* fmt, ...)
{
    if (!g_pTraceObserver) return;

    char buffer[LOG_MAX_LEN] = {0};

    va_list args;
    va_start(args, fmt);
    vsprintf_s(buffer, LOG_MAX_LEN, fmt, args);
    va_end(args);

    g_pTraceObserver(level, buffer);
}

void SetTraceCallBack(FPTRACECALLBACK cb)
{
    dvfTrace::Config(cb);
}
SEEK_NAMESPACE_END


