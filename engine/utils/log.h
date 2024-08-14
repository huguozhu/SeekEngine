#pragma once

#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

#define LOG_MAX_LEN 900 // > 1000 will cause crash in zoom

//#define SEEK_ANDROID_SAMPLE
#if defined(SEEK_PLATFORM_WINDOWS)
#define PATH_SLASH '\\'
#else
#define PATH_SLASH '/'
#endif

constexpr const char* str_end(const char* str) {
    return *str ? str_end(str + 1) : str;
}
constexpr bool str_slant(const char* str) {
    return *str == PATH_SLASH ? true : (*str ? str_slant(str + 1) : false);
}
constexpr const char* r_slant(const char* str) {
    return *str == PATH_SLASH ? (str + 1) : r_slant(str - 1);
}
constexpr const char* file_name(const char* str) {
    return str_slant(str) ? r_slant(str_end(str)) : str;
}

#define __TO_STR(x) #x
#define TO_STR(x) __TO_STR(x)
#define __LINE_STR__ TO_STR(__LINE__)

enum TRACE_LEVEL
{
    LevelError = 1,
    LevelWarning = 2,
    LevelInfo = 3,
    LevelDebug = 4,
};

typedef void(*FPTRACECALLBACK)(TRACE_LEVEL level, const char* szLog);
typedef void(*FPMONITORCALLBACK)(const char* log);

class seekTrace
{
public:
    static void Config(FPTRACECALLBACK cb);
    static void TraceLog(TRACE_LEVEL level, const char* fmt, ...);
};

#if defined __DISABLE_INFO__ || defined __DISABLE_ALL_LOG__

#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARNING(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_ERROR_RET(ret, fmt, ...)
#define LOG_ERROR_PRIERR(ret, fmt, ...)

#elif defined(SEEK_ANDROID_SAMPLE)

#include <android/log.h>
#define JNI_TAG_0 "dvf_engine"
#define LOG_DEBUG(fmt, ...)             { __android_log_print(ANDROID_LOG_DEBUG, JNI_TAG_0, fmt, ##__VA_ARGS__); }
#define LOG_INFO(fmt, ...)              { __android_log_print(ANDROID_LOG_INFO,  JNI_TAG_0, fmt, ##__VA_ARGS__); }
#define LOG_WARNING(fmt, ...)           { __android_log_print(ANDROID_LOG_WARN,  JNI_TAG_0, fmt, ##__VA_ARGS__); }
#define LOG_ERROR(fmt, ...)             { __android_log_print(ANDROID_LOG_ERROR, JNI_TAG_0, fmt, ##__VA_ARGS__); }
#define LOG_ERROR_PRIERR(ret, fmt, ...)    { __android_log_print(ANDROID_LOG_ERROR, JNI_TAG_0, fmt PRIERR_FORMAT, ##__VA_ARGS__, PRIERR_VALUE(ret)); }

#else

#define LOG_DEBUG(fmt, ...) \
{ \
    constexpr const char* base_name = file_name(__FILE__); \
    seekTrace::TraceLog(TRACE_LEVEL::LevelDebug, "[%s:" __LINE_STR__ " %s] " fmt, base_name, __func__, ##__VA_ARGS__); \
}

#define LOG_INFO(fmt, ...) \
{ \
    constexpr const char* base_name = file_name(__FILE__); \
    seekTrace::TraceLog(TRACE_LEVEL::LevelInfo, "[%s:" __LINE_STR__ " %s] " fmt, base_name, __func__, ##__VA_ARGS__); \
}

#define LOG_WARNING(fmt, ...) \
{ \
    constexpr const char* base_name = file_name(__FILE__); \
    seekTrace::TraceLog(TRACE_LEVEL::LevelWarning, "[%s:" __LINE_STR__ " %s] " fmt, base_name, __func__, ##__VA_ARGS__); \
}

#define LOG_ERROR(fmt, ...) \
{ \
    constexpr const char* base_name = file_name(__FILE__); \
    seekTrace::TraceLog(TRACE_LEVEL::LevelError, "[%s:" __LINE_STR__ " %s] " fmt, base_name, __func__, ##__VA_ARGS__); \
}

#define LOG_ERROR_PRIERR(ret, fmt, ...) \
{ \
    constexpr const char* base_name = file_name(__FILE__); \
    seekTrace::TraceLog(TRACE_LEVEL::LevelError, "[%s:" __LINE_STR__ "] [%s] " fmt PRIERR_FORMAT, base_name, __func__, ##__VA_ARGS__, PRIERR_VALUE(ret)); \
}

#endif

SEEK_NAMESPACE_END
