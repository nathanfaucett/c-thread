#ifndef __THREAD_C__
#define __THREAD_C__


#include "./lib.h"


#if defined(__THREAD_EMULATE_TIMESPEC_GET__)
int32 _thread_timespec_get(struct timespec* ts, int32 base) {
    #if defined(__IS_WINDOWS__)
        struct _timeb tb;
    #elif !defined(CLOCK_REALTIME)
        struct timeval tv;
    #endif

    if (base != TIME_UTC) {
        return 0;
    } else {
        #if defined(__IS_WINDOWS__)
            _ftime_s(&tb);
            ts->tv_sec = (time_t) tb.time;
            ts->tv_nsec = 1000000L * (long) tb.millitm;
        #elif defined(CLOCK_REALTIME)
            base = (clock_gettime(CLOCK_REALTIME, ts) == 0) ? base : 0;
        #else
            gettimeofday(&tv, NULL);
            ts->tv_sec = (time_t)tv.tv_sec;
            ts->tv_nsec = 1000L * (long) tv.tv_usec;
        #endif

        return base;
    }
}
#endif


#include "./Mutex.c"
#include "./Condition.c"
#include "./Thread.c"
#include "./TSS.c"


#endif
