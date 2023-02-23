#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "mvote.h"
#include "dataNode.h"
#include "nDLinkedList.h"
#include "hashTable.h"
#include "postalLinkedList.h"
#include "postalNode.h"
#include <time.h>

#define BUFFER_SIZE 256

// to print the commands available
void printCommands()
{
    printf("l <pin>\t\t\t\t\tlookup voter info with PIN <pin>\n");
    printf("i <pin> <lname> <fname> <zip>\t\tinsert voter info\n");
    printf("r <pin>\t\t\t\t\tregister a vote for the voter with PIN <pin>\n");
    printf("bv <fileofkeys>\t\t\t\tbulk register votes from file <fileofkeys>\n");
    printf("v\t\t\t\t\tprint no. of people who've voted so far\n");
    printf("perc\t\t\t\t\tprint the percentage of people who've voted so far\n");
    printf("z <zipcode>\t\t\t\tprint the no. of people & their PINs who've voted from <zip>\n");
    printf("o\t\t\t\t\tprint no. of voters in descending order grouped by zip code\n");
    printf("exit\t\t\t\t\tquit mvote\n");
}

// commands handler
void performCommand(char *buffer, HashTable *hashTable, PostalLinkedList *postalLinkedList)
{
    char *command = strtok(buffer, " ");

    // lookup command
    if (strcmp(command, "l") == 0)
    {
        char *pin = strtok(NULL, " ");
        if (pin == NULL)
        {
            printf("Command is invalid. Please try again!\n");
        }
        else
        {
            // search for the voter in the hash table and print details
            DataNode *result = hashTableSearch(hashTable, atoi(pin));

            if (result == NULL)
            {
                printf("Voter not found.\n");
            }
            else
            {
                printf("Voter Details:\nPIN: %d, Last Name: %s, First Name: %s, Zip: %d, Voted: %s\n", result->pin, result->lname, result->fname, result->zip, result->voted ? "Y" : "N");
            }
        }
    }
    // insert command
    else if (strcmp(command, "i") == 0)
    {
        // reading user input
        char *pin = strtok(NULL, " ");
        char *lname = strtok(NULL, " ");
        char *fname = strtok(NULL, " ");
        char *zip = strtok(NULL, " ");

        if (pin == NULL || lname == NULL || fname == NULL || zip == NULL)
        {
            printf("Command is invalid. Please try again!\n");
        }
        else
        {
            // insert voter in the hash table
            hashTableInsert(hashTable, dataNodeInit(atoi(pin), atoi(zip), fname, lname, false));
            printf("Data inserted successfully!\n");
        }
    }
    // register vote command
    else if (strcmp(command, "r") == 0)
    {
        char *pin = strtok(NULL, " ");
        if (pin == NULL)
        {
            printf("Command is invalid. Please try again!\n");
        }
        else
        {
            vote(hashTable, postalLinkedList, atoi(pin));
        }
    }
    // bulk vote command
    else if (strcmp(command, "bv") == 0)
    {
        char *fileName = strtok(NULL, " ");
        if (fileName == NULL)
        {
            printf("Command is invalid. Please try again!\n");
        }
        else
        {
            bulkVote(fileName, hashTable, postalLinkedList);
        }
    }
    // total vote count command
    else if (strcmp(command, "v") == 0)
    {
        int count = totalVoteCount(postalLinkedList);
        printf("Total number of voters who have voted: %d\n", count);
    }
    // total vote percentage command
    else if (strcmp(command, "perc") == 0)
    {
        int perc = totalVotePercentage(hashTable, postalLinkedList);
        printf("Percentage of voters who have voted: %d%%\n", perc);
    }
    // print voters from zip command
    else if (strcmp(command, "z") == 0)
    {
        char *zip = strtok(NULL, " ");
        if (zip == NULL)
        {
            printf("Command is invalid. Please try again!\n");
        }
        else
        {
            printVotersFromZip(postalLinkedList, atoi(zip));
        }
    }
    // print voters in descending order grouped by zip command
    else if (strcmp(command, "o") == 0)
    {
        printVoteCountForAllZip(postalLinkedList);
    }
    // exit command
    else if (strcmp(command, "exit") == 0)
    {
        quit(hashTable, postalLinkedList);
    }
    // print command
    else if (strcmp(command, "print") == 0)
    {
        hashTablePrint(hashTable);
    }
    // in case user input command is invalid
    else
    {
        printf("Command is invalid. Please try again!\n");
    }
}

int main(int argc, char *argv[])
{
    // read command line arguments in the following format:
    // ./mvote -f registeredvoters -b maxBucketSize
    char *fileName = NULL;
    int maxBucketSize = 0;

    if (argc != 5)
    {
        printf("Invalid number of arguments. Please try again!\n");
        exit(1);
    }

    if (strcmp(argv[1], "-f") == 0)
    {
        fileName = argv[2];
    }
    
    if (strcmp(argv[3], "-b") == 0)
    {
        maxBucketSize = atoi(argv[4]);
    }

    if (fileName == NULL || maxBucketSize == 0) {
        printf("Invalid arguments. Please try again!\n");
        exit(1);
    }

    // 2 data structures to store the data -> hash table with linear hashing and
    // linked list with each item having postal code and voters list
    HashTable *hashTable = hashTableInit(maxBucketSize);
    PostalLinkedList *postalLinkedList = postalLinkedListInit();

    // read data from file and insert into hash table
    FILE *file = fopen(fileName, "r");
    if (file == NULL)
    {
        printf("Data file not found. Please try again!\n");
        exit(1);
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file))
    {
        // remove trailing new line char from line
        line[strcspn(line, "\n")] = 0;

        // split line by space
        char *pin = strtok(line, " ");
        char *lname = strtok(NULL, " ");
        char *fname = strtok(NULL, " ");
        char *zip = strtok(NULL, " ");

        DataNode *dataNode = dataNodeInit(atoi(pin), atoi(zip), fname, lname, false);
        hashTableInsert(hashTable, dataNode);
    }

    char buffer[BUFFER_SIZE];
    printCommands();

    while (true)
    {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // remove trailing new line char from buffer
        buffer[strcspn(buffer, "\n")] = 0;

        // perform given command
        performCommand(buffer, hashTable, postalLinkedList);
    }

    return 0;
}