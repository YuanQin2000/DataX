/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __TRACKER_TRACE_H__
#define __TRACKER_TRACE_H__

#include "Common/Typedefs.h"
#include <stdio.h>
#include "Time.h"

class __CFunctionTracker__;

#ifndef __SW_VERSION__
#define __SW_VERSION__ "Unknown SW"
#endif

#if defined(__DEBUG__)

#define ASSERT(expr, ...) \
    ((expr) ? (void(0)) : Assert(#expr, __FILE__, __LINE__, ##__VA_ARGS__, ""))
#define ASSERT_IF(if_expr, expr, ...) \
    ((if_expr) ? ((expr) ? (void(0)) : Assert(#expr, __FILE__, __LINE__, ##__VA_ARGS__, "")) : (void(0)))

#define TRACK_FUNCTION_LIFE_CYCLE __CFunctionTracker__ __tracker__(__PRETTY_FUNCTION__)
#define OUTPUT_DEBUG_TRACE(format, ...) do { \
        timespec __now__; \
        GetProcessElapseTime(&__now__); \
        OutputTrace("[%ld:%7ld] [DEBUG] [%s:%d]  " format, \
                    __now__.tv_sec, __now__.tv_nsec, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)
    
#else   // __DEBUG__
#define ASSERT(expr, ...)
#define ASSERT_IF(if_expr, expr, ...)
#define TRACK_FUNCTION_LIFE_CYCLE
#define OUTPUT_DEBUG_TRACE(format, ...)
#endif  // end __DEBUG__

#define OUTPUT_NOTICE_TRACE(format, ...) do { \
        timespec __now__; \
        GetProcessElapseTime(&__now__); \
        OutputTrace("[%ld:%7ld] [NOTICE] [%s:%d]  " format, \
                    __now__.tv_sec, __now__.tv_nsec, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define OUTPUT_WARNING_TRACE(format, ...) do { \
        timespec __now__; \
        GetProcessElapseTime(&__now__); \
        OutputTrace("[%ld:%7ld] [WARNING] [%s:%d]  " format, \
                    __now__.tv_sec, __now__.tv_nsec, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define OUTPUT_ERROR_TRACE(format, ...) do { \
        timespec __now__; \
        GetProcessElapseTime(&__now__); \
        OutputTrace("[%ld:%7ld] [ERROR] [%s:%d]  " format, \
                    __now__.tv_sec, __now__.tv_nsec, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define OUTPUT_RAW_TRACE(format, ...) OutputTrace(format, ##__VA_ARGS__)

void InitializeDebug();
void Assert(const char* pExpression, const char* pFilename, int lineno, ...);
void OutputTrace(const char* format, ...);
void OutputStringTrace(const char* pStr, size_t len = 0);
void OutputCallStack();

void DumpBytes(void* pBytes, size_t len);
void DumpString(char* pStr, size_t len);

class __CFunctionTracker__
{
public:
    __CFunctionTracker__(const char* pFuncName) : m_pFuncName(pFuncName)
    {
        timespec now;
        GetProcessElapseTime(&now);
        OutputTrace("[%ld:%7ld] [FUNC] %s <ENTER>\n", now.tv_sec, now.tv_nsec, pFuncName);
    }
    ~__CFunctionTracker__()
    {
        timespec now;
        GetProcessElapseTime(&now);
        OutputTrace("[%ld:%7ld] [FUNC] %s <EXIT>\n", now.tv_sec, now.tv_nsec, m_pFuncName);
    }

private:
    const char* m_pFuncName;
};

#endif
