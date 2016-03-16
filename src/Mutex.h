#ifndef __THREAD_MUTEX_H__
#define __THREAD_MUTEX_H__


#if defined(__THREAD_WIN32__)
typedef struct Mutex {
    union {
        CRITICAL_SECTION cs;      /* Critical section handle (used for non-timed mutexes) */
        HANDLE mut;               /* Mutex handle (used for timed mutex) */
    } mHandle;                    /* Mutex handle */
    int32 mAlreadyLocked;         /* TRUE if the mutex is already locked */
    int32 mRecursive;             /* TRUE if the mutex is recursive */
    int32 mTimed;                 /* TRUE if the mutex is timed */
} Mutex;
#else
typedef pthread_mutex_t Mutex;
#endif


int32 Mutex_init(Mutex* mutex, int32 type);
void Mutex_destroy(Mutex* mutex);
int32 Mutex_lock(Mutex* mutex);
int32 Mutex_timedlock(Mutex* mutex, const struct timespec* ts);
int32 Mutex_trylock(Mutex* mutex);
int32 Mutex_unlock(Mutex* mutex);


#endif
