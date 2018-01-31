/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Trace.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include "Time.h"
#include "Common/Macros.h"
#include "Thread/Thread.h"

using std::free;
using std::vprintf;
using std::printf;
using std::putc;
using std::isprint;

static void HandleCrashSignal(int signalNo, siginfo_t* pInfo, void* context)
{
    const char* sigText  = NULL;
    const void* pAddress = NULL;

    switch (pInfo->si_signo) {
    case SIGILL:
        switch (pInfo->si_code) {
        case ILL_ILLOPC:
            sigText = "Illegal opcode";
            break;
        case ILL_ILLOPN:
            sigText = "Illegal operand";
            break;
        case ILL_ILLADR:
            sigText = "Illegal addressing mode";
            break;
        case ILL_ILLTRP:
            sigText = "Illegal trap";
            break;
        case ILL_PRVOPC:
            sigText = "Privileged opcode";
            break;
        case ILL_PRVREG:
            sigText = "Privileged register";
            break;
        case ILL_COPROC:
            sigText = "Coprocessor error";
            break;
        case ILL_BADSTK:
            sigText = "Internal stack error";
            break;
        default:
            ASSERT(false, "Unknown signal information code: %d\n", pInfo->si_code);
        }
        pAddress = pInfo->si_addr;
        break;
    case SIGFPE:
        switch (pInfo->si_code) {
        case FPE_INTDIV:
            sigText = "Integer divide by zero";
            break;
        case FPE_INTOVF:
            sigText = "Integer overflow";
            break;
        case FPE_FLTDIV:
            sigText = "Floating point divide by zero";
            break;
        case FPE_FLTOVF:
            sigText = "Floating point overflow";
            break;
        case FPE_FLTUND:
            sigText = "Floating point underflow";
            break;
        case FPE_FLTRES:
            sigText = "Floating point inexact result";
            break;
        case FPE_FLTINV:
            sigText = "Floating point invalid operation";
            break;
        case FPE_FLTSUB:
            sigText = "Subscript out of range";
            break;
        default:
            ASSERT(false, "Unknown signal information code: %d\n", pInfo->si_code);
        }
        pAddress = pInfo->si_addr;
        break;
    case SIGSEGV:
        switch (pInfo->si_code) {
        case SEGV_MAPERR:
            sigText = "Address not mapped to object";
            break;
        case SEGV_ACCERR:
            sigText = "Invalid permissions for mapped object";
            break;
        default:
            ASSERT(false, "Unknown signal information code: %d\n", pInfo->si_code);
        }
        pAddress = pInfo->si_addr;
        break;
    case SIGBUS:
        switch (pInfo->si_code) {
        case BUS_ADRALN:
            sigText = "Invalid address alignment";
            break;
        case BUS_ADRERR:
            sigText = "Non-existant physical address";
            break;
        case BUS_OBJERR:
            sigText = "Object specific hardware error";
            break;
        case BUS_MCEERR_AR:
            sigText = "Hardware memory error: action required";
            break;
        case BUS_MCEERR_AO:
            sigText = "Hardware memory error: action optional";
            break;
        default:
            ASSERT(false, "Unknown signal information code: %d\n", pInfo->si_code);
        }
        pAddress = pInfo->si_addr;
        break;
    default:
        // Do nothing.
        break;
    }

    if (sigText) {
        OutputTrace(
            "Signal Captured: [%d] %s\n%s [ %p ]\n",
            signalNo,
            strsignal(signalNo),
            sigText,
            pAddress);
    }

    if (signalNo != SIGINT) {
        OutputTrace("Version: %s\n", __SW_VERSION__);
        OutputCallStack();

#ifdef __ARM_ARCH__
        if (context) {
            ucontext_t* pContext = reinterpret_cast<ucontext_t*>(context);
            OutputTrace(
                "\n=================================================================\n"
                "Fault Address: %p \nRegisters dump:\n",
                pContext->uc_mcontext.fault_address);

            OutputTrace(
                "%s: %p \t%s: %p \t%s: %p \t%s: %p \t%s: %p \n",
                "PC", pContext->uc_mcontext.arm_pc,
                "LR", pContext->uc_mcontext.arm_lr,
                "SP", pContext->uc_mcontext.arm_sp,
                "IP", pContext->uc_mcontext.arm_ip,
                "FP", pContext->uc_mcontext.arm_fp);
            OutputTrace(
                "%s: %p \t%s: %p \t%s: %p \t%s: %p \t%s: %p \n",
                "R0", pContext->uc_mcontext.arm_r0,
                "R1", pContext->uc_mcontext.arm_r1,
                "R2", pContext->uc_mcontext.arm_r2,
                "R3", pContext->uc_mcontext.arm_r3,
                "R4", pContext->uc_mcontext.arm_r4);
            OutputTrace(
                "%s: %p \t%s: %p \t%s: %p \t%s: %p \t%s: %p \n",
                "R5", pContext->uc_mcontext.arm_r5,
                "R6", pContext->uc_mcontext.arm_r6,
                "R7", pContext->uc_mcontext.arm_r7,
                "R8", pContext->uc_mcontext.arm_r8,
                "R9", pContext->uc_mcontext.arm_r9);
            OutputTrace(
                "%s: %p \t%s: %p \n",
                "R10",  pContext->uc_mcontext.arm_r10,
                "CPSR", pContext->uc_mcontext.arm_cpsr);

            OutputTrace(
                "\n=================================================================\nStack dump:\n");
            const size_t *pSP = reinterpret_cast<size_t*>(pContext->uc_mcontext.arm_sp) - 8;
            for (size_t i = 0; i < 32; i++) {
                OutputTrace(
                    "%p: %p %p %p %p %p %p %p %p\n", pSP,
                    pSP[0], pSP[1], pSP[2], pSP[3], pSP[4], pSP[5], pSP[6], pSP[7]);
                pSP += 8;
            }
        }
#endif
    }
    _exit(-1);
}

void InitializeDebug()
{
    struct sigaction sig;
    sig.sa_flags = SA_SIGINFO;
    sigemptyset(&sig.sa_mask);
    sig.sa_sigaction = HandleCrashSignal;
    sigaction(SIGSEGV, &sig, NULL);
    sigaction(SIGABRT, &sig, NULL);
    sigaction(SIGFPE, &sig, NULL);
    sigaction(SIGILL, &sig, NULL);
    sigaction(SIGBUS, &sig, NULL);
    sigaction(SIGINT, &sig, NULL);
}

void Assert(const char* pExpression, const char* pFilename, int lineno, ...)
{
    timespec __now__;
    GetProcessElapseTime(&__now__);
    OutputTrace(
        "[%ld:%7ld] [ASSERTION FAILED] [%s] [%s:%d] : %s\n",
        __now__.tv_sec,
        __now__.tv_nsec,
        CThread::GetCurrentThreadName(),
        pFilename,
        lineno,
        pExpression);

    va_list args;
    va_start(args, lineno);
    const char* pFormat = va_arg(args, const char*);
    if (pFormat && *pFormat != '\0') {
        vprintf(pFormat, args);
    }
    va_end(args);
    OutputTrace("Version: %s\n", __SW_VERSION__);
    OutputCallStack();
    _exit(-1);
}

void OutputTrace(const char* format, ...)
{
    va_list ap;

    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    fflush(stdout);
}

void OutputStringTrace(const char* pStr, size_t len /* = 0 */)
{
    if (len) {
        for (size_t i = 0; i < len; ++i) {
            if (pStr[i] != '\0') {
                putc(pStr[i], stdout);
            }
        }
        putc('\n', stdout);
        fflush(stdout);
    } else {
        OutputTrace("%s\n", pStr);
    }
}

void OutputCallStack()
{
    void *callStacks[32];
    int depth = backtrace(callStacks, COUNT_OF_ARRAY(callStacks));
    OutputTrace("=================================================================\n"
                "Output call stacks:\n");

    for (int i = 1; i < depth; ++i) {
        // Inore ourself callstack.
        char buf[256];
        Dl_info info;
        if (dladdr(callStacks[i], &info)) {
            if (info.dli_sname) {
                const char *pName = info.dli_sname;
                char *pCXXName = abi::__cxa_demangle(info.dli_sname, NULL, 0, NULL);
                if (pCXXName) {
                    pName = pCXXName;
                }
                snprintf(
                    buf, COUNT_OF_ARRAY(buf), "[%p]: %s(%s%c0x%x)\n", callStacks[i], info.dli_fname, pName,
                    (callStacks[i] > info.dli_saddr ? '+' : '-'),
                    static_cast<unsigned int>((callStacks[i] > info.dli_saddr ?
                        reinterpret_cast<uint8_t*>(callStacks[i]) - reinterpret_cast<uint8_t*>(info.dli_saddr) :
                        reinterpret_cast<uint8_t*>(info.dli_saddr) - reinterpret_cast<uint8_t*>(callStacks[i]))));
                free(pCXXName);
            } else {
                snprintf(
                    buf, COUNT_OF_ARRAY(buf), "[%p]: %s(+0x%x)\n", callStacks[i], info.dli_fname,
                    static_cast<unsigned int>(reinterpret_cast<uint8_t*>(callStacks[i]) - reinterpret_cast<uint8_t*>(info.dli_fbase)));
            }
        } else {
            snprintf(buf, COUNT_OF_ARRAY(buf), "[%p]: No module\n", callStacks[i]);
        }
        OutputTrace(buf);
    }
}

void DumpBytes(void* pData, size_t len)
{
    if (len == 0) {
        return;
    }

    size_t colum = 0;
    uint8_t* pBytes = reinterpret_cast<uint8_t*>(pData);
    printf("\n----------------- Data length: %lu -----------------\n", len);
    for (size_t i = 0; i < len; ++i) {
        if (isprint(pBytes[i])) {
            putchar(pBytes[i]);
        } else {
            printf("[%02x]", pBytes[i]);
        }
        if (++colum == 80) {
            putchar('\n');
            colum = 0;
        }
    }
    printf("\n---------------------------------------------------\n");
    fflush(stdout);
}

void DumpString(char* pStr, size_t len)
{
    if (len == 0) {
        return;
    }
    printf("\n----------------- Data length: %lu -----------------\n", len);
    for (size_t i = 0; i < len; ++i) {
        char ch = pStr[i];
        if (ch == '\r') {
            putchar('\\');
            putchar('r');
        } else if (ch == '\n') {
            putchar('\\');
            putchar('n');
            putchar(ch);
        } else {
            putchar(ch);
        }
    }
    printf("\n----------------- Data length: %lu -----------------\n", len);
    fflush(stdout);
}
