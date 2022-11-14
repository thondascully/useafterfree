/**
 * USE AFTER FREE() -> Heap Exploitation ... Why you should keep track of your pointers.
 * A flaw in glibc's heap management.
 * Teo Honda-Scully | 2022. 
 */

#include <stdio.h>
#include <stdlib.h>

// x and y exist as pointers to nothing at the moment.
int *x;
int *y;

int main() {

    /**
     * x is now a pointer to an arbitrarily located 16 byte of memory.
     * because memory is allocated dynamically (malloc), the location of this chunk is on the heap.
    */
    x = malloc(0x10);

    /**
     * the chunk that x is pointing to is now stored in tcache in the byte 0..24 bin.
    */
    free(x);

    /**
     * to optimize for speed, the heap manager will check to see if a chunk of memory of the same
     * size exists in the tcache. it does. therefore, that 'available for usage' tcache chunk
     * is the new chunk that y is now pointing to.
     * 
     * note: the x pointer is still pointing to the address of the memory chunk in question..!
    */
    y = malloc(0x10);

    /**
     * assign a value of 50 to the memory chunk in question.
    */
    *y = 50;

    printf("y: %d\n", y);
    
    return 1;
}