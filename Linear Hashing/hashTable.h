#ifndef HASH_TABLE
#define HASH_TABLE

#include <math.h>
#include "nDLinkedList.h"
#include "dataNode.h"

typedef struct HashTable HashTable;

struct HashTable
{
    int p;                      // index of the bucket to be split
    int m;                      // number of initial buckets
    int iteration;              // number of rounds of splitting

    int bar;                    // number of buckets to be split in the current round
    int maxBucketSize;          // maximum number of items in a bucket
    int bucketSize;             // current number of buckets
    int itemCount;              // current number of items in the hash table

    NDLinkedList **table;       // array of buckets
};

// instantiate a hash table
HashTable *hashTableInit(int maxBucketSize)
{
    HashTable *hashTable = (HashTable *)malloc(sizeof(HashTable));
    hashTable->iteration = 0;
    hashTable->p = 0;
    hashTable->m = 4;
    hashTable->bar = hashTable->m;
    hashTable->maxBucketSize = maxBucketSize;
    hashTable->bucketSize = 0;

    hashTable->table = (NDLinkedList **)malloc(sizeof(NDLinkedList *) * hashTable->m);

    for (int i = 0; i < hashTable->m; i++)
    {
        hashTable->table[i] = nDLinkedListInit(2);
        hashTable->bucketSize++;
    }

    return hashTable;
}

// h_i(k) = k mod (2^i * m)
int hashFunction(HashTable *hashTable, int k, int i)
{
    return fmod(k, pow(2, i) * hashTable->m);
}

// to split the bucket at index p
void splitBucket(HashTable *hashTable)
{
    NDLinkedList *listAtP = hashTable->table[hashTable->p];
    NDLinkedList *newListAtP = nDLinkedListInit(2);

    // since it's a N-dimensional Linked List, we need to iterate through each list in the bucket
    while (listAtP != NULL)
    {
        DataNode *node = listAtP->head;
        while (node != NULL)
        {
            int newHash = hashFunction(hashTable, node->pin, hashTable->iteration + 1);
            if (newHash == hashTable->p)
            {
                nDLinkedListInsert(newListAtP, dataNodeInit(node->pin, node->zip, node->fname, node->lname, node->voted));
            }
            else
            {
                nDLinkedListInsert(hashTable->table[hashTable->bucketSize - 1], dataNodeInit(node->pin, node->zip, node->fname, node->lname, node->voted));
            }
            node = node->next;
        }

        listAtP = listAtP->next;
    }

    hashTable->table[hashTable->p] = newListAtP;

    // destroy the old linked list
    destroyNDLinkedList(listAtP);

    hashTable->p++;

    // if we've split all the buckets in the current round, start a new round
    if (hashTable->p == hashTable->bar)
    {
        hashTable->p = 0;
        hashTable->iteration++;
        hashTable->bar = 2 * hashTable->bar;
    }
}

// to extend the hash table
void extendHashTable(HashTable *hashTable)
{
    hashTable->table = (NDLinkedList **)realloc(hashTable->table, sizeof(NDLinkedList *) * hashTable->bucketSize + 1);
    hashTable->table[hashTable->bucketSize] = nDLinkedListInit(2);
    hashTable->bucketSize++;

    splitBucket(hashTable);
}

// to search for a node in the hash table
DataNode *hashTableSearch(HashTable *hashTable, int pin)
{
    int hash = hashFunction(hashTable, pin, hashTable->iteration);

    if (hash >= hashTable->p)
    {
        return nDLinkedListSearch(hashTable->table[hash], pin);
    }
    else
    {
        hash = hashFunction(hashTable, pin, hashTable->iteration + 1);
        return nDLinkedListSearch(hashTable->table[hash], pin);
    }
}

// to print the hash table
void hashTablePrint(HashTable *hashTable)
{
    int j;
    for (j = 0; j < hashTable->bucketSize; j++)
    {
        printf("Bucket %d: ", j);
        nDLinkedListPrint(hashTable->table[j], " ");
        printf("\n");
    }
}

// to insert a node into the hash table
void hashTableInsert(HashTable *hashTable, DataNode *dataNode)
{

    // check if the hash table has reached its maximum capacity
    if (hashTable->bucketSize == hashTable->maxBucketSize)
    {
        printf("The database is full. No further entries are allowed.\n");
        return;
    }

    // check if the node already exists in the hash table
    DataNode *node = hashTableSearch(hashTable, dataNode->pin);
    if (node != NULL)
    {
        printf("PIN %d already exists in the database.\n", node->pin);
        return;
    }

    // decide which bucket to add the node to
    int hash = hashFunction(hashTable, dataNode->pin, hashTable->iteration);
    if (hash < hashTable->p)
    {
        hash = hashFunction(hashTable, dataNode->pin, hashTable->iteration + 1);
    }

    // insert the node into the bucket and check if the bucket has overflowed
    bool overflow = nDLinkedListInsert(hashTable->table[hash], dataNode);
    if (overflow)
    {
        extendHashTable(hashTable);
    }

    hashTable->itemCount++;
}

// to free everything from the hash table
void destroyHashTable(HashTable *hashTable)
{
    if (hashTable != NULL)
    {
        for (int i = 0; i < hashTable->bucketSize; i++)
        {
            destroyNDLinkedList(hashTable->table[i]);
        }

        free(hashTable->table);
        free(hashTable);
    }
}

#endif