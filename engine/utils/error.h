#pragma once

#include "log.h"

SEEK_NAMESPACE_BEGIN

#define SEEK_CHECKFAILED(ret)    ( (ret) != S_Success )

#define SEEK_RETIF_NULL(ptr) { \
    if (!ptr) { \
        LOG_ERROR("ERROR Ptr in null"); \
        return ERR_INVALID_ARG; \
    } \
}
#define SEEK_RETIF_FAIL(func) { \
    SResult __ret = func; \
    if (SEEK_CHECKFAILED(__ret)) { \
        char buf[1024]; \
        LOG_ERROR("ERROR %s", buf); \
        return __ret; \
    } \
}
#define SEEK_RET_NULL_IF_FAIL(func) { \
    SResult __ret = func; \
    if (SEEK_CHECKFAILED(__ret)) { \
        char buf[1024]; \
        LOG_ERROR("ERROR %s", buf); \
        return nullptr; \
    } \
}

/**
 * SResult contains three components:
 * [31, 24] error code
 * [23, 14] file uid
 * [13,  0] line number
 *
 * 31 ----------- 24 23 -------- 14 13 ----------- 0
 * |  error code   |||  file uid  |||  line number |
 * +---------------+++------------+++--------------+
 *
 */

#define ERR_ERRORCODE_BITS  8       // 0 ~ 255
#define ERR_FILEUID_BITS    10      // 0 ~ 1023
#define ERR_LINENUM_BITS    14      // 0 ~ 16383

#define ERR_ERRORCODE_SHIFT (ERR_FILEUID_BITS + ERR_LINENUM_BITS)
#define ERR_ERRORCODE_MASK  ((1 << ERR_ERRORCODE_BITS) - 1)
#define ERR_FILEUID_SHIFT   (ERR_LINENUM_BITS)
#define ERR_FILEUID_MASK    ((1 << ERR_FILEUID_BITS) - 1)
#define ERR_LINENUM_MASK    ((1 << ERR_LINENUM_BITS) - 1)
#define ERR_LINENUM_MAX     ERR_LINENUM_MASK

#define ERR_ERRORCODE(err)      (((err) >> ERR_ERRORCODE_SHIFT) & ERR_ERRORCODE_MASK)
#define ERR_FILEUID(err)        (((err) >> ERR_FILEUID_SHIFT) & ERR_FILEUID_MASK)
#define ERR_LINENUM(err)        ((err) & ERR_LINENUM_MASK)
#define ERR_EQUAL(err1, err2)   (ERR_ERRORCODE(err1) == ERR_ERRORCODE(err2))
#define ERR_GEN(errorcode)      (((errorcode) << ERR_ERRORCODE_SHIFT) | (SEEK_MACRO_FILE_UID << ERR_FILEUID_SHIFT) | __LINE__)

 /**
  Use follow ERR MACRO as a RETURN value
 */
#define ERR_UNKNOWN             ERR_GEN(SEEK_ERR_UNKNOWN)
#define ERR_INVALID_ARG         ERR_GEN(SEEK_ERR_INVALID_ARG)
#define ERR_INVALID_INIT        ERR_GEN(SEEK_ERR_INVALID_INIT)
#define ERR_INVALID_INVOKE_FLOW ERR_GEN(SEEK_ERR_INVALID_INVOKE_FLOW)
#define ERR_INVALID_SHADER      ERR_GEN(SEEK_ERR_INVALID_SHADER)
#define ERR_INVALID_MODEL_FILE  ERR_GEN(SEEK_ERR_INVALID_MODEL_FILE)
#define ERR_INVALID_DATA        ERR_GEN(SEEK_ERR_INVALID_DATA)
#define ERR_NOT_SUPPORT         ERR_GEN(SEEK_ERR_NOT_SUPPORT)
#define ERR_NOT_IMPLEMENTED     ERR_GEN(SEEK_ERR_NOT_IMPLEMENTED)
#define ERR_NO_MEM              ERR_GEN(SEEK_ERR_NO_MEM)
#define ERR_NO_DATA             ERR_GEN(SEEK_ERR_NO_DATA)
#define ERR_FILE_NOT_FOUND      ERR_GEN(SEEK_ERR_FILE_NOT_FOUND)
#define ERR_SYSTEM_ERROR        ERR_GEN(SEEK_ERR_SYSTEM_ERROR)


#define PRIERR_FORMAT           " [errcode:%d,fileuid:%d,linenum:%d]"
#define PRIERR_FORMAT_SIMPLE    "%d/%d/%d"
#define PRIERR_VALUE(err)       ERR_ERRORCODE(err),ERR_FILEUID(err),ERR_LINENUM(err)

SEEK_NAMESPACE_END
