#ifndef LOGGER
#define LOGGER

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

#include "sharedMemory.h"

/* Logger that logs to an appendable only file - logs.txt */
void logger(char *message, SharedMemory *shared_memory)
{
    if (shared_memory != NULL)
    {
        // wait for loggerMutex to be available
        sem_wait(&shared_memory->loggerMutex);
    }

    FILE *file = fopen("logs.txt", "a");

    // get current time and date, and print to file
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(file, "[%d-%02d-%02d %02d:%02d:%02d] ", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    // print message to file
    fprintf(file, "%s", message);
    printf("%s", message);

    // close file
    fclose(file);

    if (shared_memory != NULL)
    {
        // release loggerMutex
        sem_post(&shared_memory->loggerMutex);
    }
}

#endif