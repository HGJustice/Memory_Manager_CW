
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "part2.h"

int main() {
    int size = 2000;

    for (int algo = firstFit; algo < numAlgos; algo++) {

        char *algoName = NULL;
        switch (algo) {
            case firstFit:
                algoName = "FirstFit";
                break;
            case nextFit:
                algoName = "NextFit";
                break;
            case bestFit:
                algoName = "BestFit";
                break;
            case worstFit:
                algoName = "WorstFit";
                break;
        }

        printf("\n***************** START %s *****************\n", algoName);

        printf("\n\nInitialising with size: %d. After initialization, the size\n", size);
        printf("will reduce by the size of the head node header which is %d. \n\n", (int)sizeof(Node));

        void *p = sbrk(0);
        void *memory = sbrk(size); //malloc((size_t)size);
        initialise(memory, size, NULL);
        printNode(getHead(), NULL, NULL);

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
            void *chunk = allocate(rand() % getAvailableMem());
            chunks[i] = chunk;
            if (chunk) {
                Node *node = MEMNODE(chunk);
                printNode(node, NULL, NULL);
                allocations++;
                totalAllocated += node->size;
            }
        }

        int numNodes = getNumNodes();

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

        printf("\n****************** END %s******************\n", algoName);
    }

    return 0;
}
