#ifndef LIB_CTHREADS_H
#define LIB_CTHREADS_H

#include <ucontext.h>

//#include "ctlist.h"
#include <stdlib.h>

/**
 * @brief define thread id
*/
typedef unsigned cthread_t;

typedef struct c_thread_attr {
    ucontext_t context; /* Thread context(i.e.: registers, stack pointer, program counter, etc) */    
    unsigned int state;
    unsigned int priority;
}c_thread_attr;

typedef struct c_thread {
    cthread_t id; /*Thread ID*/
    c_thread_attr *attr; /*Thread attributes*/
    void(*task)(void); /*Thread task*/
    void *arg; /*Thread task arguments*/
    //struct c_thread_list *thread; /*Thread element of the List*/
}c_thread;

extern int Cthread_create(cthread_t *, const c_thread_attr *, void(*)(void), void *);

extern cthread_t cthread_self();
extern void cthreads_run();
extern void cthread_yield();
extern void print_list();
extern int cthread_join(cthread_t, void **);
extern void cthread_exit(void *);

#endif