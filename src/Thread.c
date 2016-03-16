#ifndef __THREAD_THREAD_C__
#define __THREAD_THREAD_C__


#if defined(__IS_WINDOWS__)
    static void _thread_tss_cleanup(void) {
        struct TinyCThreadTSSData* data;
        int32 iteration;
        unsigned int32 again = 1;
        void* value;

        for (iteration = 0 ; iteration < TSS_DTOR_ITERATIONS && again > 0 ; iteration++) {
            again = 0;
            for (data = _thread_tss_head ; data != NULL ; data = data->next) {
                if (data->value != NULL) {
                    value = data->value;
                    data->value = NULL;

                    if (_thread_tss_dtors[data->key] != NULL) {
                        again = 1;
                        _thread_tss_dtors[data->key](value);
                    }
                }
            }
        }

        while (_thread_tss_head != NULL) {
            data = _thread_tss_head->next;
            free(_thread_tss_head);
            _thread_tss_head = data;
        }
        _thread_tss_head = NULL;
        _thread_tss_tail = NULL;
    }

    static void NTAPI _thread_tss_callback(PVOID h, DWORD dwReason, PVOID pv) {
        (void) h;
        (void) pv;

        if (_thread_tss_head != NULL && (dwReason == DLL_THREAD_DETACH || dwReason == DLL_PROCESS_DETACH)) {
            _thread_tss_cleanup();
        }
    }
#endif


#if defined(__IS_WINDOWS__)
    static DWORD WINAPI _thread_wrapper_function(LPVOID aArg) {
#elif defined(__IS_POSIX__)
    static void* _thread_wrapper_function(void* aArg) {
#endif
        Thread_start func;
        void *arg;
        int32 res;

        _ThreadStartInfo *ti = (_ThreadStartInfo*) aArg;
        func = ti->mFunction;
        arg = ti->mArg;

        free((void*) ti);
        res = func(arg);

        #if defined(__IS_WINDOWS__)
            if (_thread_tss_head != NULL) {
                _thread_tss_cleanup();
            }

            return (DWORD) res;
        #else
            return (void*) (intptr_t) res;
        #endif
    }


int32 Thread_create(Thread *thread, Thread_start func, void *arg) {
    _ThreadStartInfo* ti = (_ThreadStartInfo*)malloc(sizeof(_ThreadStartInfo));
    if (ti == NULL) {
        return thread_nomemory;
    }
    ti->mFunction = func;
    ti->mArg = arg;

    #if defined(__IS_WINDOWS__)
        *thread = CreateThread(NULL, 0, _thread_wrapper_function, (LPVOID) ti, 0, NULL);
    #elif defined(__IS_POSIX__)
        if(pthread_create(thread, NULL, _thread_wrapper_function, (void*) ti) != 0) {
            *thread = 0;
        }
    #endif

    if(!*thread) {
        free(ti);
        return thread_error;
    } else {
        return thread_success;
    }
}

Thread Thread_current(void) {
    #if defined(__IS_WINDOWS__)
        return GetCurrentThread();
    #else
        return pthread_self();
    #endif
}

int32 Thread_detach(Thread thread) {
    #if defined(__IS_WINDOWS__)
        return CloseHandle(thread) != 0 ? thread_success : thread_error;
    #else
        return pthread_detach(thread) == 0 ? thread_success : thread_error;
    #endif
}

int32 Thread_equal(Thread a, Thread b) {
    #if defined(__IS_WINDOWS__)
        return GetThreadId(a) == GetThreadId(b);
    #else
        return pthread_equal(a, b);
    #endif
}

void Thread_exit(int32 res) {
    #if defined(__IS_WINDOWS__)
        if (_thread_tss_head != NULL) {
            _thread_tss_cleanup();
        }

        ExitThread(res);
    #else
        pthread_exit((void*)(intptr_t)res);
    #endif
}

int32 Thread_join(Thread thread, int32 *res) {
    #if defined(__IS_WINDOWS__)
        DWORD dwRes;

        if (WaitForSingleObject(thread, INFINITE) == WAIT_FAILED) {
            return thread_error;
        }
        if (res != NULL) {
            if (GetExitCodeThread(thread, &dwRes) != 0) {
                *res = dwRes;
            } else {
                return thread_error;
            }
        }

        CloseHandle(thread);
    #elif defined(__IS_POSIX__)
        void *pres;

        if (pthread_join(thread, &pres) != 0) {
            return thread_error;
        }
        if (res != NULL) {
            *res = (int)(intptr_t)pres;
        }
    #endif

    return thread_success;
}

int32 Thread_sleep(const struct timespec* duration, struct timespec* remaining) {
    #if !defined(__IS_WINDOWS__)
        int32 res = nanosleep(duration, remaining);
        if (res == 0) {
            return 0;
        } else if (errno == EINTR) {
            return -1;
        } else {
            return -2;
        }
    #else
    struct timespec start;
    DWORD t;

    timespec_get(&start, TIME_UTC);

    t = SleepEx((DWORD)(duration->tv_sec * 1000 +
    duration->tv_nsec / 1000000 +
    (((duration->tv_nsec % 1000000) == 0) ? 0 : 1)),
    TRUE);

    if (t == 0) {
        return 0;
    } else {
        if (remaining != NULL) {
            timespec_get(remaining, TIME_UTC);
            remaining->tv_sec -= start.tv_sec;
            remaining->tv_nsec -= start.tv_nsec;
            if (remaining->tv_nsec < 0) {
                remaining->tv_nsec += 1000000000;
                remaining->tv_sec -= 1;
            }
        }

        return (t == WAIT_IO_COMPLETION) ? -1 : -2;
    }
    #endif
}

void Thread_yield(void) {
    #if defined(__IS_WINDOWS__)
        Sleep(0);
    #else
        sched_yield();
    #endif
}


#endif
