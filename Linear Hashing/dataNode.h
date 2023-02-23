#ifndef DATA_NODE
#define DATA_NODE

#define BUFFER_SIZE 256

typedef struct DataNode DataNode;

struct DataNode
{
    int pin;
    int zip;
    char *fname;
    char *lname;
    bool voted;
    DataNode *next;
};

// instantiate a new DataNode
DataNode *dataNodeInit(int pin, int zip, char *fname, char *lname, bool voted)
{
    DataNode *node = (DataNode *)malloc(sizeof(DataNode));
    node->pin = pin;
    node->zip = zip;

    // copying strings to avoid pointers pointing to the
    // same memory locations from buffer
    char *f = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    char *l = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    strncpy(f, fname, BUFFER_SIZE);
    strncpy(l, lname, BUFFER_SIZE);

    node->fname = f;
    node->lname = l;

    node->voted = voted;
    node->next = NULL;

    return node;
}

// clear memory of a DataNode
void destroyDataNode(DataNode *node)
{
    if (node != NULL)
    {
        free(node->fname);
        free(node->lname);
        free(node);
    }
}

#endif