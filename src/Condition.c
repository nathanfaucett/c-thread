#ifndef __THREAD_CONDITION_C__
#define __THREAD_CONDITION_C__


int32 Condition_init(Condition *condition) {
    #if defined(__IS_WINDOWS__)
        condition->mWaitersCount = 0;

        InitializeCriticalSection(&condition->mWaitersCountLock);

        condition->mEvents[_CONDITION_EVENT_ONE] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (condition->mEvents[_CONDITION_EVENT_ONE] == NULL) {
            condition->mEvents[_CONDITION_EVENT_ALL] = NULL;
            return thread_error;
        }
        condition->mEvents[_CONDITION_EVENT_ALL] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (condition->mEvents[_CONDITION_EVENT_ALL] == NULL) {
            CloseHandle(condition->mEvents[_CONDITION_EVENT_ONE]);
            condition->mEvents[_CONDITION_EVENT_ONE] = NULL;
            return thread_error;
        }

        return thread_success;
    #else
        return pthread_cond_init(condition, NULL) == 0 ? thread_success : thread_error;
    #endif
}

void Condition_destroy(Condition *condition) {
    #if defined(__IS_WINDOWS__)
        if (condition->mEvents[_CONDITION_EVENT_ONE] != NULL) {
            CloseHandle(condition->mEvents[_CONDITION_EVENT_ONE]);
        }
        if (condition->mEvents[_CONDITION_EVENT_ALL] != NULL) {
            CloseHandle(condition->mEvents[_CONDITION_EVENT_ALL]);
        }
        DeleteCriticalSection(&condition->mWaitersCountLock);
    #else
        pthread_cond_destroy(condition);
    #endif
}

int32 Condition_signal(Condition *condition) {
    #if defined(__IS_WINDOWS__)
        int32 haveWaiters;

        EnterCriticalSection(&condition->mWaitersCountLock);
        haveWaiters = (condition->mWaitersCount > 0);
        LeaveCriticalSection(&condition->mWaitersCountLock);

        if(haveWaiters) {
            if (SetEvent(condition->mEvents[_CONDITION_EVENT_ONE]) == 0) {
                return thread_error;
            }
        }

        return thread_success;
    #else
        return pthread_cond_signal(condition) == 0 ? thread_success : thread_error;
    #endif
}

int32 Condition_broadcast(Condition *condition) {
    #if defined(__IS_WINDOWS__)
        int32 haveWaiters;

        EnterCriticalSection(&condition->mWaitersCountLock);
        haveWaiters = (condition->mWaitersCount > 0);
        LeaveCriticalSection(&condition->mWaitersCountLock);

        if(haveWaiters) {
            if (SetEvent(condition->mEvents[_CONDITION_EVENT_ALL]) == 0) {
                return thread_error;
            }
        }

        return thread_success;
    #else
        return pthread_cond_broadcast(condition) == 0 ? thread_success : thread_error;
    #endif
}

int32 Condition_wait(Condition *condition, Mutex *mutex) {
    #if defined(__IS_WINDOWS__)
        return _thread_Condition_timedwait_win32(condition, mutex, INFINITE);
    #else
        return pthread_cond_wait(condition, mutex) == 0 ? thread_success : thread_error;
    #endif
}

int32 Condition_timedwait(Condition *condition, Mutex *mutex, const struct timespec* ts) {
    #if defined(__IS_WINDOWS__)
        struct timespec now;
        if (timespec_get(&now, TIME_UTC) == TIME_UTC) {
            unsigned long long now_in_milliseconds = now.tv_sec * 1000 + now.tv_nsec / 1000000;
            unsigned long long ts_in_milliseconds  = ts->tv_sec * 1000 + ts->tv_nsec / 1000000;
            DWORD delta = (ts_in_milliseconds > now_in_milliseconds) ? (DWORD) (ts_in_milliseconds - now_in_milliseconds) : 0;
            return _thread_Condition_timedwait_win32(condition, mutex, delta);
        } else {
            return thread_error;
        }
    #else
        int32 ret;
        ret = pthread_cond_timedwait(condition, mutex, ts);

        if (ret == ETIMEDOUT) {
            return thread_timedout;
        } else {
            return ret == 0 ? thread_success : thread_error;
        }
    #endif
}

#if defined(__IS_WINDOWS__)
    static int32 _thread_Condition_timedwait_win32(Condition *condition, Mutex *mutex, DWORD timeout) {
        int32 result, lastWaiter;

        EnterCriticalSection(&condition->mWaitersCountLock);
        ++condition->mWaitersCount;
        LeaveCriticalSection(&condition->mWaitersCountLock);

        Mutex_unlock(mutex);

        result = WaitForMultipleObjects(2, condition->mEvents, FALSE, timeout);
        if (result == WAIT_TIMEOUT) {
            Mutex_lock(mutex);
            return thread_timedout;
        }
        else if (result == (int32) WAIT_FAILED) {
            Mutex_lock(mutex);
            return thread_error;
        }

        EnterCriticalSection(&condition->mWaitersCountLock);
        --condition->mWaitersCount;
        lastWaiter = (result == (WAIT_OBJECT_0 + _CONDITION_EVENT_ALL)) && (condition->mWaitersCount == 0);
        LeaveCriticalSection(&condition->mWaitersCountLock);

        if (lastWaiter) {
            if (ResetEvent(condition->mEvents[_CONDITION_EVENT_ALL]) == 0) {
                Mutex_lock(mutex);
                return thread_error;
            }
        }

        Mutex_lock(mutex);

        return thread_success;
    }
#endif


#endif
