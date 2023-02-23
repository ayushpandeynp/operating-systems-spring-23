#ifndef MVOTE
#define MVOTE

#include <stdio.h>
#include "postalLinkedList.h"
#include "postalNode.h"
#include "dataNode.h"
#include "nDLinkedList.h"
#include "hashTable.h"
#include "mergeSort.h"

// to register a vote for a voter with a given PIN
void vote(HashTable *hashTable, PostalLinkedList *postalLinkedList, int pin)
{
    // searching for voter in hash table
    DataNode *voterData = hashTableSearch(hashTable, pin);
    if (voterData == NULL)
    {
        printf("Voter not found\n");
    }
    else
    {
        if (voterData->voted == 1)
        {
            printf("Voter has already voted.\n");
        }
        else
        {
            // if voter hasn't voted, change voted status to 1
            voterData->voted = 1;
            postalDataListInsert(postalLinkedList, voterData);
            printf("Vote has been registered!\n");
        }
    }
}

// to register votes for voter PINs from a file
void bulkVote(char *fileName, HashTable *hashTable, PostalLinkedList *postalLinkedList)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        printf("File not found\n");
    }
    else
    {
        int pin;
        while (fscanf(fp, "%d", &pin) != EOF)
        {
            vote(hashTable, postalLinkedList, pin);
        }
    }
}

// to print the total number of voters who have voted
int totalVoteCount(PostalLinkedList *postalLinkedList)
{
    int count = 0;
    PostalNode *node = postalLinkedList->head;
    while (node != NULL)
    {
        count += node->dataList->size;
        node = node->next;
    }
    return count;
}

// to print the percentage of voters who have voted
int totalVotePercentage(HashTable *hashTable, PostalLinkedList *postalLinkedList)
{
    int voteCount = totalVoteCount(postalLinkedList);
    int totalVoters = hashTable->itemCount;

    return (voteCount * 100) / totalVoters;
}

// to print the list of voters from a given ZIP code
void printVotersFromZip(PostalLinkedList *postalLinkedList, int zip)
{
    PostalNode *node = postalLinkedList->head;
    bool found = false;
    
    while (node != NULL)
    {
        if (node->zip == zip)
        {
            printf("In ZIP %d, total no. of voters are %d.\nList of voter IDs:\n", zip, node->dataList->size);
            nDLinkedListPrint(node->dataList, "\n");
            found = true;
            break;
        }
        node = node->next;
    }

    if (!found)
    {
        printf("No voters found in ZIP %d.\n", zip);
    }
}

// to print vote count for all ZIP codes
void printVoteCountForAllZip(PostalLinkedList *postalLinkedList)
{
    PostalLinkedList *unsortedList = copyPostalLinkedList(postalLinkedList);
    PostalNode *node = sort(unsortedList->head);
    
    printf("ZIP\t\tVOTERS\n");

    while (node != NULL)
    {
        printf("%d\t\t%d\n", node->zip, node->dataList->size);
        node = node->next;
    }
}

// to clear memory and quit the program
void quit(HashTable *hashTable, PostalLinkedList *postalLinkedList)
{
    // delete all used memory
    destroyHashTable(hashTable);
    destroyPostalLinkedList(postalLinkedList);
    exit(0);
}

#endif