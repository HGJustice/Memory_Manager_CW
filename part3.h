#ifndef PART3_H
#define PART3_H

#define __VERBOSE_LOGGING__ 1

// management algos
enum {firstFit, nextFit, bestFit, worstFit, numAlgos};

// management functions
// called by reset allocator on each test iteration
void initialise(void *memory , int size, char *algorithm);

void *allocate(int  size);
void deallocate(void *memory);

// utility function for testing
void resetAllocator();

#endif
