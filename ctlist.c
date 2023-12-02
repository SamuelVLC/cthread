#include "ctlist.h"
#include <stdio.h>

struct c_thread_list *ready_t_node = NULL;
struct c_thread_list *blocked_t_node = NULL;
struct c_thread_list *running_t_node = NULL;

int c_thread_list_init(void)
{
    ready_t_node = (struct c_thread_list*)malloc(sizeof(struct c_thread_list));
    blocked_t_node = (struct c_thread_list*)malloc(sizeof(struct c_thread_list));
    running_t_node = (struct c_thread_list*)malloc(sizeof(struct c_thread_list));

    if(ready_t_node == NULL || blocked_t_node == NULL || running_t_node == NULL)
        return 0;
    
    /*Initializing ready queue*/
    ready_t_node->thread = NULL;
    ready_t_node->next = NULL;
    ready_t_node->previous = NULL;   

    /*Initializing ready queue*/
    blocked_t_node->thread = NULL;
    blocked_t_node->next = NULL;
    blocked_t_node->previous = NULL;   

    /*Initializing ready queue*/
    running_t_node->thread = NULL;
    running_t_node->next = NULL;
    running_t_node->previous = NULL; 

    return 1;
}

int c_thread_list_insert(c_thread *thread, struct c_thread_list *t_node)
{
    struct c_thread_list *new_thread;

    new_thread = (struct c_thread_list*)malloc(sizeof(struct c_thread_list));

    if(new_thread == NULL)
        return 1;

    struct c_thread_list *tmp = t_node;

    while (tmp->next != NULL)
        tmp = tmp->next;

    new_thread->next = NULL;
    new_thread->thread = thread;
    new_thread->previous = tmp;
    tmp->next = new_thread;

    return 0;
}

int c_thread_list_remove(c_thread *thread, struct c_thread_list *t_node)
{
    struct c_thread_list *curr = t_node->next;

    while (curr->thread != thread && curr->next != NULL)
    {
        curr = curr->next;
    }

    if (curr->thread == thread && curr->next != NULL)
    {
        curr->previous->next = curr->next;
        curr->next->previous = curr->previous;
        free(curr);
    }
    else if(curr->thread == thread && curr->next == NULL)
    {
        curr->previous->next = curr->next;
        free(curr);
    }
    else
        return 1;

    return 0;
}


void print_list(struct c_thread_list *t_node)
{
    struct c_thread_list *tmp = t_node;

    while(tmp->next != NULL)
    {
        tmp = tmp->next;
        printf("ID: %d | state: %d\n",tmp->thread->id, tmp->thread->attr->state);
    }
}
