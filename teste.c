#include "cthreads.h"
#include <stdio.h>
#include <unistd.h>

void ping()
{
    int x = 0;
    int ret = 200;

    while(x < 5)
    {
        printf("Thread %d: PING!\n", cthread_self());
        sleep(2);
        x++;
        cthread_yield();
        printf("Thread %d returned from previous stop!\n", cthread_self());
    }

    cthread_exit(&ret);
}

void pong()
{
    int *ptr[0];

    while(1)
    {
        printf("Thread %d: PONG!\n", cthread_self());
        sleep(2);
        cthread_yield();
        printf("Thread %d returned from previous stop!\n", cthread_self());
        cthread_join(1, (void**)&ptr[0]);

        if(*ptr != NULL)
            printf("Retorno da outra task é %d\n",*ptr[0]);
    }
}

int main()
{
    cthread_t id;
    int x = 0;
    while (id < 2)
    {
        x++;
        Cthread_create(&id, NULL, ping, NULL);
        Cthread_create(&id, NULL, pong, NULL);
    }
    
    cthreads_run();

    return 0;
}
