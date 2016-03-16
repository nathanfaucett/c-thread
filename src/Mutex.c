#ifndef __THREAD_MUTEX_C__
#define __THREAD_MUTEX_C__


int32 Mutex_init(Mutex* mutex, int32 type) {
    #if defined(__IS_WINDOWS__)
        mutex->mAlreadyLocked = FALSE;
        mutex->mRecursive = type & mutex_recursive;
        mutex->mTimed = type & Muteximed;
        if (!mutex->mTimed) {
            InitializeCriticalSection(&(mutex->mHandle.cs));
        }
        else {
            mutex->mHandle.mut = CreateMutex(NULL, FALSE, NULL);
            if (mutex->mHandle.mut == NULL) {
              return thread_error;
            }
        }
        return thread_success;
    #else
        int32 ret;
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        if (type & mutex_recursive) {
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        }
        ret = pthread_mutex_init(mutex, &attr);
        pthread_mutexattr_destroy(&attr);
        return ret == 0 ? thread_success : thread_error;
    #endif
}

void Mutex_destroy(Mutex* mutex) {
    #if defined(__IS_WINDOWS__)
        if (!mutex->mTimed)
            DeleteCriticalSection(&(mutex->mHandle.cs));
        } else {
            CloseHandle(mutex->mHandle.mut);
        }
    #else
        pthread_mutex_destroy(mutex);
    #endif
}

int32 Mutex_lock(Mutex* mutex) {
    #if defined(__IS_WINDOWS__)
        if (!mutex->mTimed) {
            EnterCriticalSection(&(mutex->mHandle.cs));
        } else {
            switch (WaitForSingleObject(mutex->mHandle.mut, INFINITE)) {
                case WAIT_OBJECT_0:
                    break;
                case WAIT_ABANDONED:
                default:
                    return thread_error;
            }
        }

        if (!mutex->mRecursive) {
            while(mutex->mAlreadyLocked) Sleep(1);
            mutex->mAlreadyLocked = TRUE;
        }
        return thread_success;
    #else
        return pthread_mutex_lock(mutex) == 0 ? thread_success : thread_error;
    #endif
}

int32 Mutex_timedlock(Mutex* mutex, const struct timespec * ts) {
    #if defined(__IS_WINDOWS__)
        struct timespec current_ts;
        DWORD timeoutMs;

        if (!mutex->mTimed) {
            return thread_error;
        }
        timespec_get(&current_ts, TIME_UTC);

        if ((current_ts.tv_sec > ts->tv_sec) || ((current_ts.tv_sec == ts->tv_sec) && (current_ts.tv_nsec >= ts->tv_nsec))) {
            timeoutMs = 0;
        } else {
            timeoutMs  = (DWORD) (ts->tv_sec  - current_ts.tv_sec)  * 1000;
            timeoutMs += (ts->tv_nsec - current_ts.tv_nsec) / 1000000;
            timeoutMs += 1;
        }

        switch (WaitForSingleObject(mutex->mHandle.mut, timeoutMs)) {
            case WAIT_OBJECT_0:
                break;
            case WAIT_TIMEOUT:
                return thread_timedout;
            case WAIT_ABANDONED:
            default:
                return thread_error;
        }

        if (!mutex->mRecursive) {
            while(mutex->mAlreadyLocked) Sleep(1);
            mutex->mAlreadyLocked = TRUE;
        }

        return thread_success;
    #elif defined(_POSIX_TIMEOUTS) && (_POSIX_TIMEOUTS >= 200112L) && defined(_POSIX_THREADS) && (_POSIX_THREADS >= 200112L)
        switch (pthread_mutex_timedlock(mutex, ts)) {
            case 0:
                return thread_success;
            case ETIMEDOUT:
                return thread_timedout;
            default:
                return thread_error;
        }
    #else
        int32 rc;
        struct timespec cur, dur;

        while ((rc = pthread_mutex_trylock (mutex)) == EBUSY) {
            timespec_get(&cur, TIME_UTC);

            if ((cur.tv_sec > ts->tv_sec) || ((cur.tv_sec == ts->tv_sec) && (cur.tv_nsec >= ts->tv_nsec))) {
                break;
            }

            dur.tv_sec = ts->tv_sec - cur.tv_sec;
            dur.tv_nsec = ts->tv_nsec - cur.tv_nsec;
            if (dur.tv_nsec < 0) {
                dur.tv_sec--;
                dur.tv_nsec += 1000000000;
            }

            if ((dur.tv_sec != 0) || (dur.tv_nsec > 5000000)) {
                dur.tv_sec = 0;
                dur.tv_nsec = 5000000;
            }

            nanosleep(&dur, NULL);
        }

        switch (rc) {
            case 0:
                return thread_success;
            case ETIMEDOUT:
            case EBUSY:
                return thread_timedout;
            default:
                return thread_error;
        }
    #endif
}

int32 Mutex_trylock(Mutex* mutex) {
    #if defined(__IS_WINDOWS__)
        int32 ret;

        if (!mutex->mTimed) {
            ret = TryEnterCriticalSection(&(mutex->mHandle.cs)) ? thread_success : thread_busy;
        } else {
            ret = (WaitForSingleObject(mutex->mHandle.mut, 0) == WAIT_OBJECT_0) ? thread_success : thread_busy;
        }

        if ((!mutex->mRecursive) && (ret == thread_success)) {
            if (mutex->mAlreadyLocked) {
                LeaveCriticalSection(&(mutex->mHandle.cs));
                ret = thread_busy;
            } else {
                mutex->mAlreadyLocked = TRUE;
            }
        }
        return ret;
    #else
        return (pthread_mutex_trylock(mutex) == 0) ? thread_success : thread_busy;
    #endif
}

int32 Mutex_unlock(Mutex* mutex) {
    #if defined(__IS_WINDOWS__)
        mutex->mAlreadyLocked = FALSE;
        if (!mutex->mTimed) {
            LeaveCriticalSection(&(mutex->mHandle.cs));
        } else {
            if (!ReleaseMutex(mutex->mHandle.mut)) {
                return thread_error;
            }
        }
        return thread_success;
    #else
        return pthread_mutex_unlock(mutex) == 0 ? thread_success : thread_error;;
    #endif
}


#endif
