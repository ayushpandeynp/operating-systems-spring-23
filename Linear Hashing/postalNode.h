#ifndef POSTAL_NODE
#define POSTAL_NODE

#include "nDLinkedList.h"

typedef struct PostalNode PostalNode;

struct PostalNode
{
    int zip;                        // zip code
    NDLinkedList *dataList;         // linked list of data nodes (voters data)
    PostalNode *next;               // next node in the linked list
};

// initialize a postal node
PostalNode *postalNodeInit(int zip)
{
    PostalNode *node = (PostalNode *)malloc(sizeof(PostalNode));
    node->zip = zip;
    node->next = NULL;
    node->dataList = nDLinkedListInit(1);

    return node;
}

// clear memory of a postal node
void destroyPostalNode(PostalNode *node)
{
    if (node != NULL)
    {
        destroyNDLinkedList(node->dataList);
    }
}

#endif