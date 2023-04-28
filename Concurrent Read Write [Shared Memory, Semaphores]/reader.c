#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "sharedMemory.h"
#include "semaphoreItem.h"
#include "record.h"
#include "logger.h"

#define BUFFER_SIZE 100
#define SIG_REALTIME SIGRTMIN

int main(int argc, char *argv[])
{
    /* Parsing the commands */
    /* Usage: ./reader -f filename -l recid[,recid] -d time -s shmid -p pid */
    int opt;
    char *filename = NULL;

    char *recid_str = NULL;
    int recid_arr[BUFFER_SIZE];
    int count = 0;

    int delayTime = 0;
    long shmid = 0;
    pid_t mainPID = 0;

    while ((opt = getopt(argc, argv, "f:l:d:s:p:")) != -1)
    {
        switch (opt)
        {
        case 'f':
            filename = optarg;
            break;
        case 'l':
            recid_str = optarg;

            char *token = strtok(recid_str, ",");
            while (token != NULL)
            {
                recid_arr[count] = atoi(token);
                token = strtok(NULL, ",");
                count++;

                if (count == BUFFER_SIZE)
                {
                    fprintf(stderr, "Error: recid array is full\n");
                    exit(EXIT_FAILURE);
                }
            }

            break;
        case 'd':
            delayTime = atoi(optarg);
            break;
        case 's':
            shmid = atoi(optarg);
            break;
        case 'p':
            mainPID = (pid_t)atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s -f filename -l recid[,recid] -d time -s shmid -p pid\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < count; i++)
    {
        // fork for each recid and call reader
        int recid = recid_arr[i];
        pid_t pid = fork();
        if (pid == 0)
        {
            /* Attach the Shared Memory Segment */
            SharedMemory *shared_memory = shmat(shmid, NULL, 0);
            if (shared_memory == (void *)-1)
            {
                perror("[ERROR]: Shared memory attach");
                exit(EXIT_FAILURE);
            }

            char msg[BUFFER_SIZE];
            sprintf(msg, "New read request for id: %d\n", recid);
            logger(msg, shared_memory);

            /* Check if the recid exists in semaphoreList */
            int i;
            int semaphoreItemFound = 0;
            for (i = 0; i < shared_memory->listSize; i++)
            {
                if (shared_memory->semaphoreList[i].id == recid)
                {
                    semaphoreItemFound = 1;
                    break;
                }
            }

            if (!semaphoreItemFound)
            {
                /* create the semaphore item */
                SemaphoreItem semaphoreItem = createSemaphoreItem(recid);
                shared_memory->semaphoreList[shared_memory->listSize] = semaphoreItem;
                shared_memory->listSize++;
                i = shared_memory->listSize - 1;
            }

            SemaphoreItem *semaphoreItem = &shared_memory->semaphoreList[i];

            // get current time before wait
            time_t t1 = time(NULL);

            sem_wait(&semaphoreItem->turnstile); // Lock the turnstile to ensure FCFS
            sem_post(&semaphoreItem->turnstile); // Release the turnstile to allow other readers to enter
            sem_wait(&semaphoreItem->sem_read);  // Lock the read mutex to protect the readersCount variable
            semaphoreItem->readersCount++;

            if (semaphoreItem->readersCount == 1)
            {
                sem_wait(&semaphoreItem->sem_write); // First reader blocks writers
            }

            sem_post(&semaphoreItem->sem_read); // unlock the read mutex

            // get current time after wait
            time_t t2 = time(NULL);
            double waitDuration = difftime(t2, t1);

            /* stats: update max wait duration */
            sem_wait(&shared_memory->statsMutex); // wait for statsMutex to protect the stats variables
            if (waitDuration > shared_memory->maxWaitingTime)
            {
                shared_memory->maxWaitingTime = waitDuration;
            }
            sem_post(&shared_memory->statsMutex); // release the statsMutex

            // print & log waitDuration in seconds
            sprintf(msg, "Reader for rec id %d waited for %f seconds\n", recid, waitDuration);
            logger(msg, shared_memory);

            /* Read the record by traversing file */
            printf("Reading record %d from file %s\n", recid, filename);
            FILE *fp = fopen(filename, "rb");
            if (fp == NULL)
            {
                perror("[ERROR]: File open");
                exit(EXIT_FAILURE);
            }

            int recordFound = 0;
            Record record;
            while (fread(&record, sizeof(Record), 1, fp))
            {
                if (record.studentID == recid)
                {
                    sleep(delayTime);

                    // total process duration
                    double totalProcessDuration = difftime(time(NULL), t1);

                    sem_wait(&shared_memory->statsMutex); // wait for statsMutex to protect the stats variables
                    shared_memory->totalReadersDuration += totalProcessDuration;
                    sem_post(&shared_memory->statsMutex); // release the statsMutex

                    sprintf(msg, "Reader for rec id %d found record in %f seconds\n", recid, totalProcessDuration);
                    logger(msg, shared_memory);

                    // output
                    printf("Student ID: %ld, Name: %s %s, GPA: %f\n", record.studentID, record.firstname, record.lastname, record.gpa);
                    recordFound = 1;
                    break;
                }
            }

            fclose(fp);

            if (!recordFound)
            {
                sleep(delayTime);
                double totalProcessDuration = difftime(time(NULL), t1);

                sem_wait(&shared_memory->statsMutex); // wait for statsMutex to protect the stats variables
                shared_memory->totalReadersDuration += totalProcessDuration;
                sem_post(&shared_memory->statsMutex); // release the statsMutex

                sprintf(msg, "Reader for rec id %d did not find record in %f seconds\n", recid, totalProcessDuration);
                logger(msg, shared_memory);
            }

            sem_wait(&semaphoreItem->sem_read); // lock the read mutex to protect the readersCount variable
            semaphoreItem->readersCount--;

            if (semaphoreItem->readersCount == 0)
            {
                sem_post(&semaphoreItem->sem_write); // last reader unblocks writers
            }
            sem_post(&semaphoreItem->sem_read); // unlock the read mutex

            /* stats: update total readers processed count */
            sem_wait(&shared_memory->statsMutex); // wait for statsMutex to protect the stats variables
            shared_memory->totalReadersCount++;
            sem_post(&shared_memory->statsMutex); // release the statsMutex

            /* send realtime signal to main */
            union sigval sigval;
            sigval.sival_int = 1;
            if (sigqueue(mainPID, SIG_REALTIME, sigval) == -1)
            {
                perror("sigqueue");
                exit(1);
            }

            // exit
            exit(EXIT_SUCCESS);
        }
    }

    // wait for all children to finish
    for (int i = 0; i < count; i++)
    {
        wait(NULL);
    }

    return 0;
}