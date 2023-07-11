
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _Node
{
    int allocated;
    int size;
    void *data;
    struct _Node* next;
    struct _Node* prev;
} Node;

Node* headNode = NULL;
int memSize = 0;
int memAvailable = 0;
int numNodes = 0;

#define MEMNODE(a) (a ? (Node *)((char *)a - sizeof(Node)) : NULL)

Node *split(Node *node, int size) {
    int newNodeMemSize = node->size - size- sizeof(Node);
    if (newNodeMemSize <= 0)
        return node;

    Node *newNode = (Node *)((char *)node + sizeof(Node) + size);
    newNode->allocated = 0;
    newNode->size = newNodeMemSize;
    newNode->data = (char *)newNode + sizeof(Node);
    newNode->prev = node;
    newNode->next = node->next;
    memAvailable -= sizeof(Node);
    numNodes++;

    node->next = newNode;
    node->size = size;

    return node;
}

void merge(Node *node) {
    if (node->next && !node->next->allocated) {
        node->size += sizeof(Node) + node->next->size;
        memAvailable += sizeof(Node);
        node->next = node->next->next;
        if (node->next)
            node->next->prev = node;

        numNodes--;
    }

    if (node->prev && !node->prev->allocated) {
        node->prev->next = node->next;
        node->prev->size += node->size + sizeof(Node);
        memAvailable += sizeof(Node);
        if (node->next)
            node->next->prev = node->prev;

        numNodes--;
    }
}

void *allocate(int  size)
{
    if (size <= 0)
        return NULL;

    Node *foundNode = NULL;
    Node *tmp = headNode;
    while (tmp) {
        if (tmp->size >= size && !tmp->allocated) {
            foundNode = split(tmp, size);
            foundNode->allocated = 1;
            memAvailable -= size;
            break;
        }

        tmp = tmp->next;
    }

    return foundNode ? foundNode->data : NULL;
}

void deallocate(void *memory)
{
    if (!memory)
        return;

    Node *node = (Node *)((char *)memory - sizeof(Node));
    node->allocated = 0;
    memAvailable += node->size;
    merge(node);
}

void initialise(void *memory , int size)
{
    headNode = (Node *)memory;
    headNode->allocated = 0;
    headNode->size = size - sizeof(Node);
    headNode->data = (char *)headNode + sizeof(Node);
    headNode->prev = NULL;
    headNode->next = NULL;

    memSize = size;
    memAvailable = size - sizeof(Node);
    numNodes = 1;
}

void printNode(Node *node) {
    if (!node)
        return;

    printf("node address: %p, allocated: %d, size: %d, data: %p, prev: %p, next %p\n",
           node,
           node->allocated,
           node->size,
           node->data,
           node->prev,
           node->next);
}

void printList() {
    printf("\n");

    Node *tmp = headNode;
    while (tmp) {
        printNode(tmp);
        tmp = tmp->next;
    }

    printf("\n");
}

int main() {

    int size = 2000;

    printf("\n\nInitialising with size: %d. After initialization, the size\n", size);
    printf("will reduce by the size of the head node header which is %d. \n\n", (int)sizeof(Node));

    void *p = sbrk(0);
    void *memory = sbrk(size);; //malloc((size_t)size);
    initialise(memory, size);
    printNode(headNode);

    printf("\n\nAllocating...\n");
    printf("Each allocation will remove the size of the allocation from the available \n");
    printf("memory plus the size of a node header if a split is performed.\n");
    printf("We'll try to allocate 10 times. Each time we'll request an amount less\n");
    printf("or equal to the amount of available memory which will reduce after each\n");
    printf("allocation.\n\n");

    void *chunks[10];

    int allocations = 0;
    int totalAllocated = 0;

    for (int i = 0; i < 10; i++) {
        void *chunk = allocate(rand() % memAvailable);
        chunks[i] = chunk;
        if (chunk) {
            Node *node = MEMNODE(chunk);
            printNode(node);
            allocations++;
            totalAllocated += node->size;
        }
    }

    printf("\n\nNow we've successfully performed %d allocations for a total of %d bytes\n", allocations, totalAllocated);
    printf("of memory allocated. Also, we now have %d Nodes. That means we have %d * %d\n", numNodes, numNodes, (int)sizeof(Node));
    printf("node header space used or %d. Between the allocations and the amount of node\n", numNodes * (int)sizeof(Node));
    printf("header that has been used, we managed to consume %d bytes of the memory\n", totalAllocated + (numNodes * (int)sizeof(Node)));
    printf("we had available.\n");

    printf("\n\nDeallocating...\n");
    printf("As we deallocate the Node are merged with adjacent unallocated Nodes.\n");
    printf("After each deallocation we print the list of Nodes and you can see they\n");
    printf("are being merged until finally we wind up back at the start with only one.\n");

    for (int i = 0; i < 10; i++) {
        void *chunk = chunks[i];
        if (!chunk)
            continue;

        deallocate(chunk);
        printList();
    }


    return 0;
}
