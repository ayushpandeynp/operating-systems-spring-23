#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
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
    /* Usage: ./writer -f filename -l recid -d time -s shmid -p pid */
    int opt;
    char *filename = NULL;
    int recid = 0;
    int timeDelay = 0;
    int shmid = 0;
    pid_t mainPID = 0;

    while ((opt = getopt(argc, argv, "f:l:d:s:p:")) != -1)
    {
        switch (opt)
        {
        case 'f':
            filename = optarg;
            break;
        case 'l':
            recid = atoi(optarg);
            break;
        case 'd':
            timeDelay = atoi(optarg);
            break;
        case 's':
            shmid = atoi(optarg);
            break;
        case 'p':
            mainPID = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s -f filename -l recid -d time -s shmid\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    /* Attach the Shared Memory Segment */
    SharedMemory *shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (void *)-1)
    {
        perror("[ERROR]: Shared memory attach");
        exit(EXIT_FAILURE);
    }

    char msg[BUFFER_SIZE];
    sprintf(msg, "New writer request for id: %d\n", recid);
    logger(msg, shared_memory);

    /* Get the Semaphore List */
    SemaphoreItem *semaphoreList = shared_memory->semaphoreList;

    /* Check if the recid exists in semaphoreList */
    int i;
    int found = 0;
    for (i = 0; i < shared_memory->listSize; i++)
    {
        if (semaphoreList[i].id == recid)
        {
            found = 1;
            break;
        }
    }

    /* If the recid does not exist in semaphoreList, create a new SemaphoreItem */
    if (!found)
    {
        /* Create a new SemaphoreItem */
        SemaphoreItem semaphoreItem = createSemaphoreItem(recid);
        shared_memory->semaphoreList[shared_memory->listSize] = semaphoreItem;
        shared_memory->listSize++;
        i = shared_memory->listSize - 1;
    }

    // get current time before wait
    time_t t1 = time(NULL);

    SemaphoreItem *semaphoreItem = &shared_memory->semaphoreList[i];
    sem_wait(&semaphoreItem->turnstile); // lock the turnstile to ensure FCFS
    sem_wait(&semaphoreItem->sem_write); // lock the write mutex to prevent any readers from entering the critical section
    sem_post(&semaphoreItem->turnstile); // release the turnstile to allow new readers to enter after the writer

    // get current time after wait
    time_t t2 = time(NULL);

    // calculate the waiting time
    double waitDuration = difftime(t2, t1);

    /* stats: update max wait duration */
    sem_wait(&shared_memory->statsMutex); // wait for statsMutex to protect the stats variables
    if (waitDuration > shared_memory->maxWaitingTime)
    {
        shared_memory->maxWaitingTime = waitDuration;
    }
    sem_post(&shared_memory->statsMutex); // release the statsMutex

    sprintf(msg, "Writer for rec id: %d waited for %f seconds\n", recid, waitDuration);
    logger(msg, shared_memory);

    // update the record in binary file
    FILE *fp = fopen(filename, "rb+");
    if (fp == NULL)
    {
        perror("[ERROR]: File open");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    found = 0;

    Record record;
    while (fread(&record, sizeof(Record), 1, fp))
    {
        if (record.studentID == recid)
        {
            found = 1;
            break;
        }

        index++;
    }

    if (found)
    {
        // record current timestamp to avoid user input delay in stats
        time_t userInputT1 = time(NULL);

        // show current mark for every subject and then ask if they want to update
        int j;
        for (j = 0; j < 8; j++)
        {
            printf("Previous mark for subject %d: %f\n", j + 1, record.marks[j]);
            printf("Do you want to update the mark? (y/n): ");
            char choice;
            scanf(" %c", &choice);

            if (choice == 'y')
            {
                printf("Enter new mark: ");
                scanf("%f", &record.marks[j]);

                // validity check
                while (record.marks[j] < 0 || record.marks[j] > 4.0)
                {
                    printf("Invalid mark, please enter again: ");
                    scanf("%f", &record.marks[j]);
                }

                printf("New mark set to: %f\n", record.marks[j]);
            }
        }

        time_t userInputT2 = time(NULL);

        double userInputDelay = difftime(userInputT2, userInputT1);

        // new gpa
        record.gpa = 0;
        for (j = 0; j < 8; j++)
        {
            record.gpa += record.marks[j];
        }

        record.gpa /= 8;
        printf("GPA: %f\n", record.gpa);

        fseek(fp, sizeof(Record) * index, SEEK_SET);
        fwrite(&record, sizeof(Record), 1, fp);

        sleep(timeDelay);

        // total process duration excluding user input delay
        double totalProcessDuration = difftime(time(NULL), t1) - userInputDelay;

        sem_wait(&shared_memory->statsMutex); // wait for statsMutex to protect the stats variables
        shared_memory->totalWritersDuration += totalProcessDuration;
        sem_post(&shared_memory->statsMutex); // release the statsMutex

        sprintf(msg, "Writer for rec id %d completed processing record in %f seconds\n", recid, totalProcessDuration);
        logger(msg, shared_memory);
    }
    else
    {

        sleep(timeDelay);

        // total process duration
        double totalProcessDuration = difftime(time(NULL), t1);
        sprintf(msg, "Writer for rec id %d did not find any record in %f seconds\n", recid, totalProcessDuration);
        logger(msg, shared_memory);

        perror("[ERROR]: Record not found");
    }

    fclose(fp);

    sem_post(&semaphoreItem->sem_write); // unlock the write mutex

    /* stats: update total writers processed count */
    sem_wait(&shared_memory->statsMutex); // wait for statsMutex to protect the stats variables
    shared_memory->totalWritersCount++;
    sem_post(&shared_memory->statsMutex); // release the statsMutex

    /* send realtime signal to main */
    union sigval sigval;
    sigval.sival_int = 1;
    if (sigqueue(mainPID, SIG_REALTIME, sigval) == -1)
    {
        perror("sigqueue");
        exit(1);
    }

    return 0;
}
