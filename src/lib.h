#ifndef __THREAD_H__
#define __THREAD_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "types/lib.h"
#include "os/lib.h"


#if defined(__IS_POSIX__)
    #undef _FEATURES_H
    #if !defined(_GNU_SOURCE)
        #define _GNU_SOURCE
    #endif
    #if !defined(_POSIX_C_SOURCE) || ((_POSIX_C_SOURCE - 0) < 199309L)
        #undef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 199309L
    #endif
    #if !defined(_XOPEN_SOURCE) || ((_XOPEN_SOURCE - 0) < 500)
        #undef _XOPEN_SOURCE
        #define _XOPEN_SOURCE 500
    #endif
#endif


#if defined(__IS_POSIX__)
    #include <pthread.h>
#elif defined(__IS_WINDOWS__)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
        #define __UNDEF_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #ifdef __UNDEF_LEAN_AND_MEAN
        #undef WIN32_LEAN_AND_MEAN
        #undef __UNDEF_LEAN_AND_MEAN
    #endif
#endif


#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define THREAD_NORETURN _Noreturn
#elif defined(__GNUC__)
    #define THREAD_NORETURN __attribute__((__noreturn__))
#else
    #define THREAD_NORETURN
#endif


#ifndef TIME_UTC
    #define TIME_UTC 1
    #define __THREAD_EMULATE_TIMESPEC_GET__

    #if defined(__IS_WINDOWS__)
        struct _thread_timespec {
            time_t tv_sec;
            long   tv_nsec;
        };
        #define timespec _thread_timespec
    #endif

    int32 _thread_timespec_get(struct timespec * ts, int32 base);
    #define timespec_get _thread_timespec_get
#endif


#if !(defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201102L)) && !defined(_Thread_local)
    #if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
        #define _Thread_local __thread
    #else
        #define _Thread_local __declspec(thread)
    #endif
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) && (((__GNUC__ << 8) | __GNUC_MINOR__) < ((4 << 8) | 9))
    #define _Thread_local __thread
#endif


#if defined(__IS_WINDOWS__)
    #define TSS_DTOR_ITERATIONS (4)
#else
    #define TSS_DTOR_ITERATIONS PTHREAD_DESTRUCTOR_ITERATIONS
#endif


#define thread_error    0
#define thread_success  1
#define thread_timedout 2
#define thread_busy     3
#define thread_nomemory 4

#define mutex_plain     0
#define mutex_timed     1
#define mutex_recursive 2


#include <time.h>
#include <stdlib.h>


#if defined(__IS_POSIX__)
    #include <signal.h>
    #include <sched.h>
    #include <unistd.h>
    #include <sys/time.h>
    #include <errno.h>
    #include <stdint.h>
#elif defined(__IS_WINDOWS__)
    #include <process.h>
    #include <sys/timeb.h>
#endif


#include "./Mutex.h"
#include "./Condition.h"
#include "./Thread.h"
#include "./TSS.h"


#ifdef __cplusplus
}
#endif


#endif
