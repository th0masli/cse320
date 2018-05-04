#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

#include "thread_counter.h"
#include "csapp.h"


typedef struct thread_counter {
    unsigned int num;
    sem_t mutex;
} THREAD_COUNTER;

//global var
//THREAD_COUNTER thread_counter;

//Initialize a new thread counter.
THREAD_COUNTER *tcnt_init() {
    THREAD_COUNTER *tc = Malloc(sizeof(THREAD_COUNTER));
    Sem_init(&(tc->mutex), 0, 1);
    tc->num = 0;

    return tc;
}


//Finalize a thread counter.
void tcnt_fini(THREAD_COUNTER *tc) {
    free(tc);
}


//Increment a thread counter.
void tcnt_incr(THREAD_COUNTER *tc) {
    P(&(tc->mutex));
    tc->num++;
    V(&(tc->mutex));
}


//Decrement a thread counter, alerting anybody waiting
//if the thread count has dropped to zero.
void tcnt_decr(THREAD_COUNTER *tc) {
    unsigned int *thread_num = &(tc->num);
    if (thread_num > 0) {
        P(&(tc->mutex));
        tc->num--;
        V(&(tc->mutex));
    }
    //alert every body waiting
    /*
    while (!thread_num)
        ;
    V(&(tc->mutex));
    */
}


//A thread calling this function will block in the call until
//the thread count has reached zero, at which point the
//function will return.
void tcnt_wait_for_zero(THREAD_COUNTER *tc) {
    unsigned int *thread_num;
    thread_num = &(tc->num);
    while (thread_num) {
        if (thread_num == 0)
            break;
    }

}

