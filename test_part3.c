#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "part3.h"

#define NUM_THREADS 40

// test thread safety of mem manager
void *threadFunc() {
    // wait a random moment
    usleep((useconds_t)(rand() % 500000));

    // allocate
    void *chunk = allocate(rand() % 200);
    if (!chunk)
        return NULL;

    // wait another random moment
    usleep((useconds_t)(rand() % 500000));

    // deallocate
    deallocate(chunk);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    srand((unsigned int)time(NULL));

    // iterate over the mem allocator algorithms
    for (int algo = firstFit; algo < numAlgos; algo++) {
        char *algoName = NULL;
        switch (algo) {
            case nextFit:
                algoName = "NextFit";
                break;
            case bestFit:
                algoName = "BestFit";
                break;
            case worstFit:
                algoName = "WorstFit";
                break;
            case firstFit:
            default:
                algoName = "FirstFit";
                break;
        }


        printf("\n\n***************** START %s *****************\n\n", algoName);

        resetAllocator(algoName);


        // spawn NUM_THREADS threads to pseudo randomly make many allocations
        // and corresponding deallocations. all allocations and deallocations
        // are followed by the memory manager printing the current nodes of
        // our list of blocks of memory which we are managing.
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_create(&threads[i], NULL, threadFunc, NULL);
        }

        // wait for all the threads to finish
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        printf("\n***************** END %s *****************\n", algoName);

        // rinse and repeat
    }

    return 0;
}
