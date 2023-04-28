#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <signal.h>

#include "sharedMemory.h"
#include "record.h"
#include "logger.h"

#define SHM_KEY 1221
#define BUFFER_SIZE 100
#define SIG_REALTIME SIGRTMIN

SharedMemory *shared_memory;
int shmid;
pid_t pid;

void printStats() {
    sem_wait(&shared_memory->statsMutex);
    system("clear");

    printf("PID: %d\n", pid);
    printf("SHMID: %d\n", shmid);

    printf("\n\nSTATS -------------------------------\n");
    printf("Total Readers Processed: %d\n", shared_memory->totalReadersCount);
    printf("Total Writers Processed: %d\n", shared_memory->totalWritersCount);

    double avgReadersDuration = shared_memory->totalReadersDuration / (double) (shared_memory->totalReadersCount ? shared_memory->totalReadersCount : 1);
    printf("Avg Readers Duration: %f\n", avgReadersDuration);

    double avgWritersDuration = shared_memory->totalWritersDuration / (double) (shared_memory->totalWritersCount ? shared_memory->totalWritersCount : 1);
    printf("Avg Writers Duration: %f\n", avgWritersDuration);

    printf("Max Waiting Time: %f\n", shared_memory->maxWaitingTime);

    printf("Sum of records accessed or modified: %d\n", shared_memory->listSize);

    sem_post(&shared_memory->statsMutex);

    printf("\n\nPress RETURN to exit...");
}

void handler(int sig)
{
    if (sig == SIG_REALTIME)
    {
        printStats();
    }
}

int main(int argc, char *argv[])
{
    pid = getpid();

    /* Get a Shared Memory Segment */
    shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0777 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("[ERROR]: Shared memory get");
        exit(EXIT_FAILURE);
    }

    /* Attach the Shared Memory Segment */
    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (void *)-1)
    {
        perror("[ERROR]: Shared memory attach");
        exit(EXIT_FAILURE);
    }

    /* initialize signals handler for stats */
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIG_REALTIME, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    /* Initialize the shared memory variables */
    shared_memory->listSize = 0;
    sem_init(&shared_memory->statsMutex, 1, 1);
    sem_init(&shared_memory->loggerMutex, 1, 1);
    shared_memory->totalReadersCount = 0;
    shared_memory->totalWritersCount = 0;
    shared_memory->totalReadersDuration = 0.0;
    shared_memory->totalWritersDuration = 0.0;
    shared_memory->maxWaitingTime = 0.0;

    char msg[BUFFER_SIZE];
    sprintf(msg, "Program has started on PID: %d\n", pid);
    logger(msg, shared_memory);

    printf("Shared memory attached at id: %d\n", shmid);
    printf("\nListening for R/W requests...\nPress return to exit...");

    while(getchar() != '\n');

    /* Detach the Shared Memory Segment */
    if (shmdt(shared_memory) == -1)
    {
        logger("[ERROR]: Shared memory detach\n", NULL);
        perror("Shared memory detach");
        exit(EXIT_FAILURE);
    }
    else
    {
        logger("Shared memory detached.\n", NULL);
    }

    /* Destroy the Shared Memory Segment */
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        logger("[ERROR]: Shared memory destroy\n", NULL);
        perror("Shared memory destroy");
        exit(EXIT_FAILURE);
    }
    else
    {
        logger("Shared memory destroyed.\n", NULL);
    }

    return 0;
}
