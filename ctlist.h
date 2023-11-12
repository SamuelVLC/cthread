#ifndef CT_LIST
#define CT_LIST

#include "cthreads.h"

extern struct c_thread_list *t_node;
extern struct c_thread_list *t_node_first;

struct c_thread_list {
    c_thread *thread;
    struct c_thread_list *next;
    struct c_thread_list *previous;
};

extern int c_thread_list_init(void);

extern int c_thread_list_insert(c_thread *);

extern int c_thread_list_remove(c_thread *);

#endif