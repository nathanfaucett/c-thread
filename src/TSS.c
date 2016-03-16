#ifndef __THREAD_TSS_C__
#define __THREAD_TSS_C__


int32 TSS_create(TSS *key, TSS_destructor dtor) {
    #if defined(__IS_WINDOWS__)
        *key = TlsAlloc();
        if (*key == TLS_OUT_OF_INDEXES) {
            return thread_error;
        }
        _thread_TSS_dtors[*key] = dtor;
    #else
        if (pthread_key_create(key, dtor) != 0) {
            return thread_error;
        }
    #endif

    return thread_success;
}

void TSS_delete(TSS key) {
    #if defined(__IS_WINDOWS__)
        struct ThreadTSSData* data = (struct ThreadTSSData*) TlsGetValue (key);
        struct ThreadTSSData* prev = NULL;

        if (data != NULL) {
            if (data == _thread_TSS_head) {
                _thread_TSS_head = data->next;
            } else {
                prev = _thread_TSS_head;
                if (prev != NULL) {
                    while (prev->next != data) {
                        prev = prev->next;
                    }
                }
            }

            if (data == _thread_TSS_tail) {
                _thread_TSS_tail = prev;
            }

            free(data);
        }
        _thread_TSS_dtors[key] = NULL;
        TlsFree(key);
    #else
        pthread_key_delete(key);
    #endif
}

void *TSS_get(TSS key) {
    #if defined(__IS_WINDOWS__)
        struct ThreadTSSData* data = (struct ThreadTSSData*) TlsGetValue(key);
        if (data == NULL) {
            return NULL;
        } else {
            return data->value;
        }
    #else
        return pthread_getspecific(key);
    #endif
}

int32 TSS_set(TSS key, void *val) {
    #if defined(__IS_WINDOWS__)
        struct ThreadTSSData* data = (struct ThreadTSSData*)TlsGetValue(key);
        if (data == NULL) {
            data = (struct ThreadTSSData*) malloc(sizeof(struct ThreadTSSData));

            if (data == NULL) {
                return thread_error;
            } else {
                data->value = NULL;
                data->key = key;
                data->next = NULL;

                if (_thread_TSS_tail != NULL) {
                    _thread_TSS_tail->next = data;
                } else {
                    _thread_TSS_tail = data;
                }

                if (_thread_TSS_head == NULL) {
                    _thread_TSS_head = data;
                }

                if (!TlsSetValue(key, data)) {
                    free (data);
                    return thread_error;
                }
            }
        }
        data->value = val;
    #else
        if (pthread_setspecific(key, val) != 0) {
            return thread_error;
        }
    #endif
    return thread_success;
}

#if defined(__THREAD_EMULATE_TIMESPEC_GET__)
    int32 _tthread_timespec_get(struct timespec *ts, int32 base) {
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
                ts->tv_sec = (time_t)tb.time;
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

#if defined(__IS_WINDOWS__)
    void call_once(once_flag *flag, void (*func) (void)) {
        while (flag->status < 3) {
            switch (flag->status) {
                case 0:
                    if (InterlockedCompareExchange (&(flag->status), 1, 0) == 0) {
                        InitializeCriticalSection(&(flag->lock));
                        EnterCriticalSection(&(flag->lock));
                        flag->status = 2;
                        func();
                        flag->status = 3;
                        LeaveCriticalSection(&(flag->lock));
                        return;
                    }
                    break;
                case 1:
                    break;
                case 2:
                    EnterCriticalSection(&(flag->lock));
                    LeaveCriticalSection(&(flag->lock));
                    break;
            }
        }
    }
#endif


#endif
