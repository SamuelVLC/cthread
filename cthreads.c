#include "cthreads.h"
#include "ctlist.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

/**
 * Constant definitions
*/
#define TJOIN_SIZE 1000

/**
 * Global variable definitions
*/
cthread_t tid = 0; /*Static thread ID Definition*/
int tJoin[TJOIN_SIZE];
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
int JRFLAG[TJOIN_SIZE]; /*Scheduler flag to join*/

/*Forward definitions*/
void schedule();
void join_handler();
void reorder();
void yield_handler();
void end_task();
int cthread_scheduler();
c_thread *find_thread(cthread_t, struct c_thread_list *);
struct c_thread_list *IsBlocking(cthread_t );
void unblock_threads(struct c_thread_list *);
int thread_exist(cthread_t );

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
         makecontext(&task_end, (void*)cthread_exit, 0, NULL);

         getcontext(&(thread->attr->context));
         thread->attr->context.uc_stack.ss_sp = malloc(2048);
         thread->attr->context.uc_stack.ss_size = 2048;
         thread->attr->context.uc_link = &task_end; 
        
         makecontext(&(thread->attr->context),thread->task,0);
         
         if(ready_t_node == NULL)
            c_thread_list_init();
        
         c_thread_list_insert(thread, ready_t_node);
    }

    return 0;
}

int cthread_scheduler()
{
    getcontext(&scheduler_context);

    switch (signals)
    {
    case YSIG:
        yield_handler();
        signals = 0;
        schedule();
        break;

    case JSIG:
        join_handler();
        signals = 0;
        schedule();
        break;

    case ESIG:
        signals = 0;
        end_task();

    default:
        schedule();
        break;
    }

    return 0;
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

void yield_handler()
{
    /* Insert thread in the ready thread queue */
    c_thread_list_insert(current_thread, ready_t_node);

    /* Change the state to running */
    current_thread->attr->state = 1;

    /* Remove thread from the running thread queue */
    c_thread_list_remove(current_thread, running_t_node);

}

/**
 * @brief wait for other specified thread to finish execution
*/
int cthread_join(cthread_t thread, void **value_ptr)
{
    if(thread_exist(thread))
        return 1;
        
    JRFLAG[current_thread->id] = 0;
    tJoin[current_thread->id] = thread;

    if(getcontext(&(current_thread->attr->context)) != 0)
    {
        return 1;
    }else if(JRFLAG[current_thread->id] == 0){
        signals = JSIG;
        setcontext(&scheduler_context);       
    }

    JRFLAG[current_thread->id] = 0;

    if(value_ptr == NULL)
        return 0;
   
    *value_ptr = *vJoin;

    return 0;
}

void join_handler()
{
    /* Insert thread in the blocked thread queue */
    c_thread_list_insert(current_thread, blocked_t_node);

    /* Change the state to blocked */
    current_thread->attr->state = 3;

    /* Remove thread from the running thread queue */
    c_thread_list_remove(current_thread, running_t_node);
}

/**
 * @brief exit and finish thread execution
*/
void cthread_exit(void *value_ptr)
{
    vJoin = &value_ptr;
    signals = ESIG;
    setcontext(&scheduler_context);
}

void schedule()
{   
    struct c_thread_list *curr = ready_t_node->next;

    if(curr == NULL)
    {
        printf("All threads have finished!\n");
        return;
    }

    current_thread = curr->thread;

    YRFLAG = 1; /* Enable task to return from yield call */

    /* Change the state to running */
    current_thread->attr->state = 2;

    /* Insert thread in the running thread queue */
    c_thread_list_insert(current_thread, running_t_node);

    /* Remove thread from the ready thread queue */
    c_thread_list_remove(current_thread, ready_t_node);

    /* prepare preemption */
    signal(SIGALRM, cthread_yield);
    alarm(2);

    setcontext(&(current_thread->attr->context));
}

void end_task()
{
    struct c_thread_list *blockedThreads = IsBlocking(current_thread->id);
    
    if(blockedThreads->next != NULL)
    {
        unblock_threads(blockedThreads);
    }

    c_thread_list_remove(current_thread, running_t_node);

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

c_thread *find_thread(cthread_t id, struct c_thread_list *t_node)
{
    struct c_thread_list  *tmp = t_node->next;

    while(tmp != NULL && id != tmp->thread->id)
        tmp = tmp->next;

    if(tmp == NULL)
        return NULL;
    
    return tmp->thread;
}

struct c_thread_list *IsBlocking(cthread_t id)
{
    int x = 1;
    
    struct c_thread_list *b_threads = (struct c_thread_list*)malloc(sizeof(struct c_thread_list));
    b_threads->next = NULL;
    b_threads->previous = NULL;
    b_threads->thread = NULL;

    while(x < TJOIN_SIZE)
    {
        if(id == tJoin[x])
        {
            c_thread_list_insert(find_thread(x,blocked_t_node),b_threads);
        }   
        x++;
    }
    return b_threads;
}

void unblock_threads(struct c_thread_list *b_threads)
{
    struct c_thread_list *tmp = b_threads->next;

    while(tmp != NULL)
    {
        JRFLAG[tmp->thread->id] = 1; /* Enable task to return from join call*/

        /* Insert thread in the ready thread queue */
        c_thread_list_insert(tmp->thread, ready_t_node);
        
        /* Change the state to ready */
        tmp->thread->attr->state = 1;
        
        /* Remove thread from the blocked thread queue */
        c_thread_list_remove(tmp->thread, blocked_t_node);
        
        tmp = tmp->next;    
    } 
}

int thread_exist(cthread_t id)
{
    if(find_thread(id, ready_t_node) == NULL)
    {
        if(find_thread(id, running_t_node) == NULL)
        {
            if(find_thread(id, blocked_t_node) == NULL)
                return 1;
        }
        else
            return 0;
    }

    return 0;
}