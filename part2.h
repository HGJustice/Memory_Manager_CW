#ifndef PART2_H
#define PART2_H

#include <unistd.h>

typedef struct _Node
{
    int allocated;
    int size;
    void *data;
    struct _Node* next;
    struct _Node* prev;
} Node;

enum {firstFit, nextFit, bestFit, worstFit, numAlgos};
typedef int fitalgo_t ;

#define MEMNODE(a) (a ? (Node *)((char *)a - sizeof(Node)) : NULL)

void *allocate(int  size);
void deallocate(void *memory);
void initialise(void *memory , int size, char *algorithm);

void printNode(Node *node, char *prefix, char *suffix);
void printList();

int getAvailableMem();
Node *getHead();
int getNumNodes();

#endif

