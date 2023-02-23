#ifndef MERGE_SORT
#define MERGE_SORT

#include "postalNode.h"
#include "postalLinkedList.h"

PostalNode *mid(PostalNode *head);
PostalNode *merge(PostalNode *linkedList1, PostalNode *linkedList2);
PostalNode *sort(PostalNode *head);

// find the middle node of a linked list
PostalNode *mid(PostalNode *head)
{
    PostalNode *prevM = NULL;
    while (head != NULL && head->next != NULL)
    {
        prevM = (prevM != NULL) ? prevM->next : head;
        head = head->next->next;
    }

    PostalNode *m = prevM->next;
    prevM->next = NULL;

    return m;
}

// merge two linked lists
PostalNode *merge(PostalNode *linkedList1, PostalNode *linkedList2)
{
    PostalNode *node = postalNodeInit(0);
    PostalNode *head = node;
    while (linkedList1 != NULL && linkedList2 != NULL)
    {
        if (linkedList1->dataList->size > linkedList2->dataList->size)
        {
            node->next = linkedList1;
            linkedList1 = linkedList1->next;
        }
        else
        {
            node->next = linkedList2;
            linkedList2 = linkedList2->next;
        }

        node = node->next;
    }

    if (linkedList1 != NULL)
    {
        node->next = linkedList1;
    }

    else
    {
        node->next = linkedList2;
    }

    return head->next;
}

// sort a linked list using merge sort algorithm
PostalNode *sort(PostalNode *head)
{
    if (head == NULL || head->next == NULL)
    {
        return head;
    }

    PostalNode *m = mid(head);
    PostalNode *l = sort(head);
    PostalNode *r = sort(m);

    return merge(l, r);
}

#endif