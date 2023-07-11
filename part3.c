#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "part3.h"

// our node struct for
// managing our blocks
// of memory
typedef struct _Node
{
    int allocated;      // available?
    int size;           // how much memory does this node have?
    void *data;         // actual memory that can be used
    struct _Node* next; // next node in our list
    struct _Node* prev; // previous node in our list
} Node;

// keep track of which algorithm
// we are using for managing our
// memeory
typedef int fitalgo_t;
fitalgo_t algo = firstFit;

// get us the pointer to our node
// from a pointer to the node's data
#define MEMNODE(a) (a ? (Node *)((char *)a - sizeof(Node)) : NULL)

// golbal variable for managing
// the state of our memory manaager
pthread_mutex_t memMutex;       // thread safety
Node* headNode = NULL;          // our list
Node* nextFitCurrent = NULL;    // Use for the next fit algo
int pvt_memSize = 2000;         // default size for or memory manager
void *pvt_p = NULL;             // pointer to the initial base of our memory
void *pvt_memory = NULL;        // pointer to the memory we are allocated by sbrk

// statistics
int memAvailable = 0;           // how much memory out of all our memory is available?
int numNodes = 0;               // how many nodes do we have at the moment?
int allocations = 0;            // the number of allocation that have been made

// utility functions
void printNode(void *mem, char *prefix, char *suffix);
void printList();
#ifdef __VERBOSE_LOGGING__
#define log(a)              printf(a);
#define logList()           printList()
#else
#define log(a)
#define logList()
#endif

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

// find a free block of memory
// in our list of nodes. if it's
// bigger than our needs we
// can split it into two nodes,
// save the other node for another
// allocation, and return the final
// just right sized node to the
// caller.
void *allocate(int  size)
{
    // nothing to do here
    if (size <= 0)
        return NULL;

    // thread safety
    pthread_mutex_lock(&memMutex);

    log("Allocating...");
    printf(" %d\n", ++allocations);

    Node *foundNode = NULL;

    // get the next available node that
    // fits our needs.
    switch (algo) {
        // start looking from the
        // last place we stopped.
        case nextFit: {
            if (!nextFitCurrent)
                nextFitCurrent = headNode;

            Node *start = nextFitCurrent;
            Node *tmp = nextFitCurrent;
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
        // look for the node that most closely
        // fits our needs. not too big or small.
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
        // look for the biggest node in the list
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
        // look for the first node that is
        // big enough for our needs
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

    // if we've founds a suitable node
    // split it if we can, mark it allocated
    // and reduce it's size from the amount
    // of memory we have available to allocate.
    // otherwise print out a message saying
    // we've had miss.
    if (foundNode) {
        printf("\n\n ********* HIT! *************\n\n");
        foundNode = split(foundNode, size);
        foundNode->allocated = 1;
        memAvailable -= size;

        logList();
    }
    else
        printf("\n\n ********* MISS! %d *************\n\n", size);


    // thread safety
    pthread_mutex_unlock(&memMutex);

    // if we've got a node at this point,
    // return a pointer to it's data
    return foundNode ? foundNode->data : NULL;
}

// mark a node availabe for use again
// and merge it with adjacent nodes if
// possible
void deallocate(void *memory)
{
    pthread_mutex_lock(&memMutex);

    printNode(memory, "Deallocatirg... ", NULL);

    if (!memory) {
        pthread_mutex_unlock(&memMutex);
        return;
    }

    Node *node = (Node *)((char *)memory - sizeof(Node));
    node->allocated = 0;
    memAvailable += node->size;
    merge(node);

    logList();

    pthread_mutex_unlock(&memMutex);
}

// initialize memory manager.
// - create the thread mutex
// - if we memory is NULL we'll request
//   memory to manage with sbrk
// - initialize the head node
// - set the alorithm that we'll use
//   for managing our memory
void initialise(void *memory , int size, char *algorithm)
{
    pthread_mutex_init(&memMutex, NULL);
    pthread_mutex_lock(&memMutex);

    if (!memory) {
        pvt_p = sbrk(0);
        pvt_memory = sbrk(pvt_memSize);
        memory = pvt_memory;
    }

    headNode = (Node *)memory;
    headNode->allocated = 0;
    headNode->size = size - sizeof(Node);
    headNode->data = (char *)headNode + sizeof(Node);
    headNode->prev = NULL;
    headNode->next = NULL;

    pvt_memSize = size;
    memAvailable = size - sizeof(Node);
    numNodes = 1;

    if (algorithm) {
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

    pthread_mutex_unlock(&memMutex);
}

// print a single node - expects a pointer to the memory
// for a node, not the node itself. if mem is NULL, will
// print the head node.
void printNode(void *mem, char *prefix, char *suffix) {
    Node *node = NULL;

    if (!mem) {
        node = headNode;
    }
    else
        node = MEMNODE(mem);

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

// print all the nodes our memory is
// currently slit into
void printList() {
    printf("\nCurrent list:\n");

    Node *tmp = headNode;
    while (tmp) {
        printNode(tmp->data, NULL, NULL);
        tmp = tmp->next;
    }

    printf("\n");
}

// reset the state of the allocator
// number of allocator set to 0
// memory will not be reacquired if we
// already have some, but the head
// will be set to the entire block
void resetAllocator(char *algo) {
    allocations = 0;
    initialise(pvt_memory, pvt_memSize, algo);
    printNode(NULL, "Main Node Initialized: ", "\n\n");
}
