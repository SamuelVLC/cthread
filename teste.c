#include "cthreads.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

FILE *file; 

void ping()
{
    int x = 0;
    int ret = 200;
    cthread_t id;

    while(x < 5)
    {
        printf("Thread %d: PING!\n", cthread_self());
        x++;
        sleep(1);
        printf("Thread %d returned from previous stop!\n", cthread_self());
    }
    Cthread_create(&id, NULL, ping, NULL);
    //printf("oi saindo\n");
    cthread_exit(&ret);
}

void pong()
{
    int *ptr[2];    
    while(1)
    {
        printf("Thread %d: PONG!\n", cthread_self());
        sleep(1);
        printf("Thread %d returned from previous stop!\n", cthread_self());
        cthread_join(1,(void**)(ptr+1));
        
        if(*(ptr+1) != NULL)
            printf("Retorno da outra task é %d\n",*ptr[1]);
            
    }
}

int main()
{
    cthread_t id;
    int x = 0;
    //file = fopen("testeOutput.txt", "w");
    
    while (id < 2)
    {
        x++;
        Cthread_create(&id, NULL, ping, NULL);
        Cthread_create(&id, NULL, pong, NULL);
    }
    
    cthreads_run();

    return 0;
}
