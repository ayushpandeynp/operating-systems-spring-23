#ifndef LINKED_LIST
#define LINKED_LIST

#include "dataNode.h"

typedef struct NDLinkedList NDLinkedList;

struct NDLinkedList
{
    int size;           // number of nodes in the linked list
    int dimension;      // dimension of the linked list
    DataNode *head;     // head of the linked list
    NDLinkedList *next; // next linked list (will be NULL for 1D linked list)
};

// initialize a linked list
NDLinkedList *nDLinkedListInit(int dimension)
{
    NDLinkedList *linkedList = malloc(sizeof(NDLinkedList));
    linkedList->size = 0;
    linkedList->head = NULL;
    linkedList->next = NULL;
    linkedList->dimension = dimension;

    return linkedList;
}

// insert a node into a linked list
bool nDLinkedListInsert(NDLinkedList *linkedList, DataNode *dataNode)
{
    bool overflow = false;

    if (linkedList->head == NULL)
    {
        linkedList->head = dataNode;
        linkedList->size++;
    }
    else
    {
        while (linkedList->next != NULL)
        {
            linkedList = linkedList->next;
        }

        // if the linked list reached its dimension, add a new linked list to the chain
        if (linkedList->dimension > 1 && linkedList->size >= linkedList->dimension)
        {
            NDLinkedList *newLinkedList = nDLinkedListInit(2);
            newLinkedList->head = dataNode;
            linkedList->next = newLinkedList;

            linkedList = linkedList->next;
            linkedList->size++;
            
            overflow = true;
        }
        else
        {
            DataNode *node = linkedList->head;
            while (node->next != NULL)
            {
                node = node->next;
            }
            node->next = dataNode;
            linkedList->size++;
        }

        
    }
    return overflow;
}

// print a linked list
void nDLinkedListPrint(NDLinkedList *linkedList, char *separator)
{
    if (linkedList->head == NULL)
    {
        printf("Empty list!\n");
    }
    else
    {
        NDLinkedList *list = linkedList;
        while (list != NULL)
        {
            DataNode *node = list->head;
            while (node != NULL)
            {
                printf("%d", node->pin);
                printf("%s", separator);
                node = node->next;
            }
            list = list->next;
        }
    }
}

// to make a copy of the linked list (we need copy of only 1 dimension)
NDLinkedList *copy1DLinkedList(NDLinkedList *linkedList)
{
    NDLinkedList *newLinkedList = nDLinkedListInit(linkedList->dimension);
    if (linkedList->head == NULL)
    {
        return newLinkedList;
    }
    else
    {
        NDLinkedList *list = linkedList;
        while (list != NULL)
        {
            DataNode *node = list->head;
            while (node != NULL)
            {
                nDLinkedListInsert(newLinkedList, dataNodeInit(node->pin, node->zip, node->fname, node->lname, node->voted));
                node = node->next;
            }
            list = list->next;
        }
    }
    return newLinkedList;
}

// search for a node in a linked list
DataNode *nDLinkedListSearch(NDLinkedList *linkedList, int pin)
{
    if (linkedList->head == NULL)
    {
        return NULL;
    }
    else
    {
        NDLinkedList *list = linkedList;
        while (list != NULL)
        {
            DataNode *node = list->head;
            while (node != NULL)
            {
                if (node->pin == pin)
                {
                    return node;
                }
                node = node->next;
            }
            list = list->next;
        }
    }
    return NULL;
}

// clear all memory occupied by a linked list
void destroyNDLinkedList(NDLinkedList *linkedList)
{
    if (linkedList != NULL)
    {
        NDLinkedList *list = linkedList;
        while (list != NULL)
        {
            DataNode *node = list->head;
            while (node != NULL)
            {
                DataNode *tmp = node;
                node = node->next;
                destroyDataNode(tmp);
            }

            list = list->next;
        }

        free(linkedList);
    }
}

#endif