#include "ctlist.h"
#include <stdio.h>

struct c_thread_list *t_node = NULL;
struct c_thread_list *t_node_first = NULL;

int c_thread_list_init(void)
{
    t_node = (struct c_thread_list*)malloc(sizeof(struct c_thread_list));

    if(t_node == NULL)
        return 0;
    
    t_node->thread = NULL;
    t_node->next = NULL;
    t_node->previous = NULL;
    t_node_first = t_node;    

    return 1;
}

int c_thread_list_insert(c_thread *thread)
{
    struct c_thread_list *new_thread;
    new_thread = (struct c_thread_list*)malloc(sizeof(struct c_thread_list));

    if(new_thread == NULL)
        return 1;

    new_thread->next = NULL;
    new_thread->previous = t_node;
    new_thread->thread = thread;
    
    if(t_node != NULL)
        t_node->next = new_thread;

    t_node = new_thread;

    return 0;
}

int c_thread_list_remove(c_thread *thread)
{
    struct c_thread_list *curr = t_node;
    
    while (curr->thread != thread && curr->previous != NULL)
    {
        curr = curr->previous;
    }

    if (curr->previous != NULL && curr->next == NULL)
    {
        curr->previous->next = curr->next;
        t_node = curr->previous;
        free(curr);
    }
    else if(curr->next != NULL)
    {
        curr->previous->next = curr->next;
        curr->next->previous = curr->previous;
        free(curr);
    }
    else
        return 1;

    return 0;
}


void print_list()
{
    struct c_thread_list *tmp = t_node_first;

    while(tmp->next != NULL)
    {
        tmp = tmp->next;
        printf("ID: %d | state: %d\n",tmp->thread->id, tmp->thread->attr->state);
    }
}
