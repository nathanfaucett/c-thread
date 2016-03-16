#ifndef __THREAD_THREAD_H__
#define __THREAD_THREAD_H__


#if defined(__IS_WINDOWS__)
    typedef HANDLE Thread;
#else
    typedef pthread_t Thread;
#endif


typedef int32 (*Thread_start)(void* arg);


#if defined(__IS_WINDOWS__)
    struct ThreadTSSData {
        void* value;
        tss_t key;
        struct ThreadTSSData* next;
    };

    static tss_dtor_t _thread_tss_dtors[1088] = { NULL, };

    static _Thread_local struct ThreadTSSData* _thread_tss_head = NULL;
    static _Thread_local struct ThreadTSSData* _thread_tss_tail = NULL;

    static void _thread_tss_cleanup(void);
    static void NTAPI _thread_tss_callback(PVOID h, DWORD dwReason, PVOID pv);

    #if defined(_MSC_VER)
        #ifdef _M_X64
            #pragma const_seg(".CRT$XLB")
        #else
            #pragma data_seg(".CRT$XLB")
        #endif
            PIMAGE_TLS_CALLBACK p_thread_callback = _thread_tss_callback;
        #ifdef _M_X64
            #pragma data_seg()
        #else
            #pragma const_seg()
        #endif
    #else
        PIMAGE_TLS_CALLBACK p_thread_callback __attribute__((section(".CRT$XLB"))) = _thread_tss_callback;
    #endif
#endif

typedef struct _ThreadStartInfo {
    Thread_start mFunction;
    void* mArg;
} _ThreadStartInfo;


#if defined(__IS_WINDOWS__)
    static DWORD WINAPI _thread_wrapper_function(LPVOID aArg);
#elif defined(__IS_POSIX__)
    static void* _thread_wrapper_function(void * aArg);
#endif



int32 Thread_create(Thread *thread, Thread_start func, void *arg);
Thread Thread_current(void);
int32 Thread_detach(Thread thread);
int32 Thread_equal(Thread a, Thread b);
void Thread_exit(int32 res);
int32 Thread_join(Thread thread, int32 *res);
int32 Thread_sleep(const struct timespec* duration, struct timespec* remaining);
void Thread_yield(void);


#endif
