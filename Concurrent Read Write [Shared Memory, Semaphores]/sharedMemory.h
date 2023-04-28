#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include "semaphoreItem.h"

typedef struct SharedMemory SharedMemory;

struct SharedMemory
{
    SemaphoreItem semaphoreList[100000];
    int listSize;

    sem_t loggerMutex;
    
    sem_t statsMutex;
    int totalReadersCount;
    int totalWritersCount;

    double totalReadersDuration;
    double totalWritersDuration;

    double maxWaitingTime;
};

#endif