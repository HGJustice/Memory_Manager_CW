#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "part2.h"

Node* headNode = NULL;
Node* nextFitCurrent = NULL;
int memSize = 0;
int numNodes = 0;
fitalgo_t algo = firstFit;
int memAvailable = 0;

// if we can, let's split this node so that
// it more closely matches the needed size and
// add a new node to our list with the left
// over memory for use by another allocation.
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

    if (node->next)
        node->next->prev = newNode;

    node->next = newNode;
    node->size = size;

    return node;
}

// if we can, merge the node with its
// adjacent nodes.
void merge(Node *node) {
    // next node isn't allocated,
    // merge it with us
    if (node->next && !node->next->allocated) {
        if (nextFitCurrent == node->next) {
            nextFitCurrent = node->next->next;
        }

        node->size += sizeof(Node) + node->next->size;
        memAvailable += sizeof(Node);
        node->next = node->next->next;
        if (node->next)
            node->next->prev = node;

        numNodes--;
    }

    // previous node isn't allocated,
    // let's merge us with it
    if (node->prev && !node->prev->allocated) {
        if (nextFitCurrent == node) {
            nextFitCurrent = node->prev;
        }

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

    switch (algo) {
        case nextFit: {
            if (!nextFitCurrent) {
                nextFitCurrent = headNode;
            }

            Node *start = nextFitCurrent;
            if (!start)
                return NULL;

            Node *tmp = nextFitCurrent;;
            do {
                if (tmp->size >= size && !tmp->allocated) {
                    foundNode = tmp;
                    nextFitCurrent = foundNode->next;
                    break;
                }

                tmp = tmp->next;
                if (!tmp) {
                    tmp = headNode;
                }
            } while(tmp != start);
            break;
        }
        case bestFit: {
            Node *tmp = headNode;
            while (tmp) {
                if (tmp->size >= size && !tmp->allocated) {
                    if (!foundNode || tmp->size < foundNode->size) {
                        foundNode = tmp;
                    }
                }

                tmp = tmp->next;
            }

            break;
        }
        case worstFit: {
            Node *tmp = headNode;
            while (tmp) {
                if (tmp->size >= size && !tmp->allocated) {
                    if (!foundNode || tmp->size > foundNode->size) {
                        foundNode = tmp;
                    }
                }

                tmp = tmp->next;
            }

            break;
        }
        default:
        case firstFit: {
            Node *tmp = headNode;
            while (tmp) {
                if (tmp->size >= size && !tmp->allocated) {
                    foundNode = tmp;
                    break;
                }

                tmp = tmp->next;
            }
            break;
        }
    }

    if (foundNode) {
        if (algo != worstFit) {
            foundNode = split(foundNode, size);
        }
        foundNode->allocated = 1;
        memAvailable -= size;
    }

    return foundNode ? foundNode->data : NULL;
}

void deallocate(void *memory)
{
    if (!memory)
        return;

    Node *node = MEMNODE(memory);
    node->allocated = 0;
    memAvailable += node->size;
    merge(node);
}

void initialise(void *memory , int size, char *algorithm)
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

    if (!algorithm)
        return;

    if (!strcmp(algorithm, "FirstFit")) {
        algo = firstFit;
    }
    else if (!strcmp(algorithm, "NextFit")) {
        algo = nextFit;
    }
    else if (!strcmp(algorithm, "BestFit")) {
        algo = bestFit;
    }
    else if (!strcmp(algorithm, "WorstFit")) {
        algo = worstFit;
    }
}

void printNode(Node *node, char *prefix, char *suffix) {
    if (!node)
        return;

    if (!prefix)
        prefix = "";

    if (!suffix)
        suffix = "\n";

    printf("%snode address: %p, allocated: %d, size: %d, data: %p, prev: %p, next %p%s",
           prefix,
           node,
           node->allocated,
           node->size,
           node->data,
           node->prev,
           node->next,
           suffix);
}

void printList() {
    printf("\n");

    Node *tmp = headNode;
    while (tmp) {
        printNode(tmp, NULL, NULL);
        tmp = tmp->next;
    }

    printf("\n");
}

int getAvailableMem() {
    return memAvailable;
}

Node *getHead() {
    return headNode;
}

int getNumNodes() {
    return numNodes;
}