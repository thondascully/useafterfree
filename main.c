/**
 * USE AFTER FREE() -> Heap Exploitation ... Why you should keep track of your pointers.
 * A flaw in glibc's heap management.
 * Teo Honda-Scully | 2022. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int *arbitrary_chunk;
int *value;

int main() {
    int inp;
    arbitrary_chunk = NULL;
    value = NULL;

    while (1) {
        printf("\n1: create chunk\n");
        printf("2: free chunk\n");
        printf("3: assign value to chunk\n");
        printf("4: check to see if correct value\n");

        scanf("%d", &inp);

        switch (inp) {
            case(1) : create_chunk();
            case(2) : free_chunk();
            case(3) : assign_to_chunk();
            case(4) : verify_chunk_value();
            default : continue;
        }
    }
    
    return 1;
}