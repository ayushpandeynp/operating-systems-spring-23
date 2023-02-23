#ifndef POSTAL_TABLE
#define POSTAL_TABLE

#include "postalNode.h"
#include "dataNode.h"
#include "nDLinkedList.h"

typedef struct PostalLinkedList PostalLinkedList;

struct PostalLinkedList
{
    PostalNode *head;       // head of the linked list
    int size;               // size of the linked list
};

// instantiate a new PostalLinkedList
PostalLinkedList *postalLinkedListInit()
{
    PostalLinkedList *postalLinkedList = malloc(sizeof(PostalLinkedList));
    postalLinkedList->head = NULL;
    postalLinkedList->size = 0;

    return postalLinkedList;
}

// insert a new PostalNode into the linked list
void postalLinkedListInsert(PostalLinkedList *postalLinkedList, PostalNode *postalNode)
{
    if (postalLinkedList->head == NULL)
    {
        postalLinkedList->head = postalNode;
    }
    else
    {
        PostalNode *node = postalLinkedList->head;
        while (node->next != NULL)
        {
            if (node->zip == postalNode->zip)
            {
                return;
            }
            node = node->next;
        }

        node->next = postalNode;
    }
    postalLinkedList->size++;
}

// insert a new DataNode into the dataList linked list of a Postal Node
void postalDataListInsert(PostalLinkedList *postalLinkedList, DataNode *voterData)
{
    DataNode *dataNode = dataNodeInit(voterData->pin, voterData->zip, voterData->fname, voterData->lname, voterData->voted);
    PostalNode *node = postalLinkedList->head;

    if (node == NULL)
    {
        PostalNode *postalNode = postalNodeInit(dataNode->zip);
        postalLinkedListInsert(postalLinkedList, postalNode);

        nDLinkedListInsert(postalNode->dataList, dataNodeInit(dataNode->pin, dataNode->zip, dataNode->fname, dataNode->lname, dataNode->voted));
    }
    else
    {
        while (node != NULL && node->zip != dataNode->zip)
        {
            node = node->next;
        }

        if (node == NULL)
        {

            PostalNode *postalNode = postalNodeInit(dataNode->zip);
            postalLinkedListInsert(postalLinkedList, postalNode);

            nDLinkedListInsert(postalNode->dataList, dataNodeInit(dataNode->pin, dataNode->zip, dataNode->fname, dataNode->lname, dataNode->voted));
        }
        else
        {
            nDLinkedListInsert(node->dataList, dataNodeInit(dataNode->pin, dataNode->zip, dataNode->fname, dataNode->lname, dataNode->voted));
        }
    }
}

// copy the contents of a PostalLinkedList
PostalLinkedList *copyPostalLinkedList(PostalLinkedList *postalLinkedList)
{
    PostalLinkedList *newPostalLinkedList = postalLinkedListInit();
    PostalNode *node = postalLinkedList->head;
    while (node != NULL)
    {
        PostalNode *newPostalNode = postalNodeInit(node->zip);
        newPostalNode->dataList = copy1DLinkedList(node->dataList);
        postalLinkedListInsert(newPostalLinkedList, newPostalNode);
        node = node->next;
    }

    return newPostalLinkedList;
}

// free the contents of a PostalLinkedList
void destroyPostalLinkedList(PostalLinkedList *postalLinkedList)
{
    if (postalLinkedList != NULL)
    {
        PostalNode *node = postalLinkedList->head;
        while (node != NULL)
        {
            PostalNode *tmp = node;
            node = node->next;
            destroyPostalNode(tmp);
        }

        free(postalLinkedList);
    }
}

#endif