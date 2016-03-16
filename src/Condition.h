#ifndef __THREAD_CONDITION_H__
#define __THREAD_CONDITION_H__


#if defined(__IS_WINDOWS__)
    #define _CONDITION_EVENT_ONE 0
    #define _CONDITION_EVENT_ALL 1
#endif


#if defined(__IS_WINDOWS__)
    typedef struct Condition {
        HANDLE mEvents[2];                  /* Signal and broadcast event HANDLEs. */
        uint32 mWaitersCount;               /* Count of the number of waiters. */
        CRITICAL_SECTION mWaitersCountLock; /* Serialize access to mWaitersCount. */
    } Condition;
#else
    typedef pthread_cond_t Condition;
#endif


int32 Condition_init(Condition *condition);
void Condition_destroy(Condition *condition);
int32 Condition_signal(Condition *condition);
int32 Condition_broadcast(Condition *condition);
int32 Condition_wait(Condition *condition, Mutex *mutex);
int32 Condition_timedwait(Condition *condition, Mutex *mutex, const struct timespec* ts);

#if defined(__IS_WINDOWS__)
    static int32 _thread_Condition_timedwait_win32(Condition *condition, Mutex *mutex, uint32 timeout);
#endif


#endif
