#ifndef __THREAD_TSS_H__
#define __THREAD_TSS_H__


#if defined(__IS_WINDOWS__)
    typedef DWORD TSS;
#else
    typedef pthread_key_t TSS;
#endif


typedef void (*TSS_destructor)(void* val);


int32 TSS_create(TSS *key, TSS_destructor dtor);
void TSS_delete(TSS key);
void *TSS_get(TSS key);
int32 TSS_set(TSS key, void *val);

#if defined(__THREAD_EMULATE_TIMESPEC_GET__)
    int32 _tthread_timespec_get(struct timespec *ts, int32 base);
#endif

#if defined(__IS_WINDOWS__)
    void call_once(once_flag *flag, void (*func) (void));
#endif


#endif
