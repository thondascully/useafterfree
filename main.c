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
     * exactly as expected, the address of both x and y are the same!
     * this means that they are pointing to the same chunk! 
     * therefore, if we change the value of the chunk x points to, 
     * we are also changing the value of the chunk that y pulls from.
     *
     * this is a way of manipulating a variable without directly touching it.
    */
    printf("\naddress of x: %p\n", x);
    printf("address of y: %p\n", y);

    /**
     * assign a value of 50 to the memory chunk in question.
    */
    *y = 50;

    /**
     * verify that the value of the chunk y is pointing to is 50.
    */    
    printf("\ny: %d\n", *y);

    /**
     * since x and y are pointing to the same chunk, assigning a value to x's chunk
     * will also change the value of y's chunk.
    */
    *x = 20515;
    printf("\n*x is now set to %d\nkeep in mind that x and y are pointing to the same chunk...\n", *x);
    
    /**
     * cool part :)
    */
    printf("\nsince x and y are pointing to the same chunk, changing the chunk's value through *x will change *y's pull value. therefore, printing *y will output *x's value, as they are pulling from the same source of information (despite no direct value assignment to *y)\n");
    printf("\ny: %d\n", *y);
 
    return 1;
}