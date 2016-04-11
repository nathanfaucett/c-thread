#include <stdio.h>
#include <stdlib.h>

#include "../src/lib.h"


int HelloThread(void* aArg) {
    (void) aArg;
    printf("Hello world!\n");
    return 0;
}


int main(void) {
    Thread thread;
    if (Thread_create(&thread, HelloThread, (void*) 0) == thread_success) {
        Thread_join(thread, NULL);
    }
    return 0;
}
