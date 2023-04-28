#ifndef SEMAPHOREITEM
#define SEMAPHOREITEM

#include <semaphore.h>

typedef struct SemaphoreItem SemaphoreItem;

struct SemaphoreItem
{
    int id;
    sem_t sem_read;
    sem_t sem_write;
    sem_t turnstile;
    
    int readersCount;
};

SemaphoreItem createSemaphoreItem(int id)
{
    SemaphoreItem semaphoreItem;
    
    semaphoreItem.id = id;
    semaphoreItem.readersCount = 0;

    sem_init(&semaphoreItem.sem_read, 1, 1);
    sem_init(&semaphoreItem.sem_write, 1, 1);
    sem_init(&semaphoreItem.turnstile, 1, 1);

    return semaphoreItem;
}

#endif