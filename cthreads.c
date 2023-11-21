#include "cthreads.h"
#include "ctlist.h"
#include <stdio.h>

/**
 * Global variable definitions
*/
cthread_t tid = 0; /*Static thread ID Definition*/
cthread_t tJoin = 0; /*Join handler global thread ID */
void **vJoin = NULL; /*Join handler global value_ptr*/
ucontext_t scheduler_context;
ucontext_t task_end; /*Defines context to end a task*/
c_thread *current_thread; /*Thread in Execution*/

/**
 * Signal definitions
*/
#define YSIG 1 /*Yield signal*/
#define JSIG 2 /*Join signal*/
#define ESIG 3 /*Exit signal*/
int signals;

/**
 * Flag definitions
*/
int YRFLAG; /*Scheduler flag to yield*/
int JRFLAG; /*Scheduler flag to join*/

/*Forward definitions*/
void update_status();
void scheduler_default();
void join_handler();
void reorder();
void yield_handler();
void end_task();
int cthread_scheduler();
/*teste*/

int Cthread_create(cthread_t *t, const struct c_thread_attr *attr, void(*start_task)(void), void *arg)
{
    if(attr == NULL)
    {
        tid = tid + 1;
        
        c_thread *thread  = malloc(sizeof(c_thread));

         thread->attr =  malloc(sizeof(c_thread_attr));
         thread->attr->state = 1;
         thread->attr->priority = 0;

         thread->task = start_task;
         thread->arg = arg;
         thread->id = tid;
         *t = thread->id;
         
         task_end.uc_stack.ss_sp = malloc(1024);
         task_end.uc_stack.ss_size = 1024;
         getcontext(&task_end);
         makecontext(&task_end, end_task, 0);

         getcontext(&(thread->attr->context));
         thread->attr->context.uc_stack.ss_sp = malloc(2048);
         thread->attr->context.uc_stack.ss_size = 2048;
         thread->attr->context.uc_link = &task_end; 
        
         makecontext(&(thread->attr->context),thread->task,0);
         
         if(t_node == NULL)
            c_thread_list_init();
        
         c_thread_list_insert(thread);
    }

    return 0;
}

int cthread_scheduler()
{
    getcontext(&scheduler_context);

    /*Update thread status*/
    update_status();

    switch (signals)
    {
    case 1:
        signals = -1;
        reorder();
        yield_handler();
        break;
    
    case 2:
        signals = -1;
        join_handler();
        break;

    case 3:
        signals = -1;
        end_task();

    default:
    scheduler_default();
        break;
    }
    
    return 0;
}

void update_status()
{
    struct c_thread_list *tmp = t_node_first;

    if(tmp->next == NULL)
        return;

    while(tmp->next != NULL)
    {
        tmp = tmp->next;

        /*Update thread status to ready if it is stopped/blocked*/
        if(tmp->thread->attr->state == 3)
            tmp->thread->attr->state = 1;

        /* Update thread status to ready if it is interrupted by join call */
        if(tmp->thread->attr->state == 5 && JRFLAG == 1)
            tmp->thread->attr->state = 1;
    }

}

void reorder()
{
    print_list();
    struct c_thread_list *tmp = t_node;
    struct c_thread_list *tmp2;

    while (tmp->previous != t_node_first && tmp->thread != current_thread)
        tmp = tmp->previous;
    

    if(t_node == t_node_first)
        return;

    if(tmp != t_node)
    {
        tmp->next->previous = tmp->previous;
        tmp->previous->next = tmp->next;
        tmp->previous = tmp->next;
        
        tmp2 = tmp->next->next;
        tmp->next->next = tmp;
        tmp->next = tmp2;
        
    }
    else
    {
        tmp->previous->next = tmp->next;
        
        t_node_first->next->previous = tmp;
        tmp->previous = t_node_first;
        tmp->next = t_node_first->next;
        t_node_first->next = tmp;
    }

    tmp = t_node_first;

    while(tmp->next != NULL)
        tmp = tmp->next;

    t_node = tmp;
}

/**
 * @brief yields the thread
*/
void cthread_yield()
{
    YRFLAG = 0;

    if(getcontext(&(current_thread->attr->context)) != 0)
    {
        return;
    }else if(YRFLAG == 0){
        signals = YSIG;
        setcontext(&scheduler_context);
        
    }
}

/**
 * @brief handle yield call
*/
void yield_handler()
{
    struct c_thread_list *curr = t_node_first->next;

    current_thread->attr->state = 3;
    while(curr->next != NULL && (curr->thread->attr->state != 1))
    {
        curr = curr->next;
    }

    if (curr->thread->attr->state == 1)
    {
        YRFLAG = 1; /* Enable task to return from yield call*/
        current_thread = curr->thread;

        setcontext(&(current_thread->attr->context));
    }
    else
    {
        YRFLAG = 1; /* Enable task to return from yield call*/
        current_thread->attr->state = 2;
        setcontext(&(current_thread->attr->context));
    }
    
}

int cthread_join(cthread_t thread, void **value_ptr)
{
    JRFLAG = 0;
    tJoin = thread; /*Prepare ID to be used by join handler*/

    if(getcontext(&(current_thread->attr->context)) != 0)
    {
        return 1;
    }else if(JRFLAG == 0){
        signals = JSIG;
        setcontext(&scheduler_context);       
    }

    current_thread->attr->state = 2;

    if(value_ptr == NULL)
        return 0;


    *value_ptr = *vJoin;
    tJoin = 0;
    
    return 0;
}

void join_handler()
{
    struct c_thread_list *curr = t_node_first->next;

    current_thread->attr->state = 5; /*Suspends calling thread*/

    while(curr->next != NULL && (curr->thread->id != tJoin))
    {
        curr = curr->next;
    }

    if (curr->thread->attr->state == 1)
    {
        YRFLAG = 1; /* Enable task to return from yield call*/
        current_thread = curr->thread;
        current_thread->attr->state = 2;

        setcontext(&(current_thread->attr->context));
    }
    else if(curr->thread->attr->state == 4)
    {
        JRFLAG = 1; /* Enable task to return from join call*/
        printf("Target Thread has already been terminated. Returning control to the calling thread...\n");
        current_thread->attr->state = 2;
        setcontext(&(current_thread->attr->context));
    }
    else
        setcontext(&scheduler_context);
    
}

void cthread_exit(void *value_ptr)
{
    vJoin = &value_ptr;
    signals = ESIG;
    setcontext(&scheduler_context);
}

void scheduler_default()
{
    struct c_thread_list *curr = t_node_first->next;

    while(curr->next != NULL && (curr->thread->attr->state != 1))
    {
        curr = curr->next;
    }
    
    current_thread = curr->thread;
    
    if(current_thread->attr->state == 4)
    {
        printf("All tasks have finished!\n");
        return;
    }

    YRFLAG = 1; /* Enable task to return from yield call*/
    setcontext(&(current_thread->attr->context));
}

void end_task()
{
    current_thread->attr->state = 4;

    if(tJoin > 0)
        JRFLAG = 1; /* Enable task to return from join call*/

    setcontext(&scheduler_context);
}

void cthreads_run()
{
    cthread_scheduler();
}

cthread_t cthread_self()
{
    return current_thread->id;
}
