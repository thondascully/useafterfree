/**
 * USE AFTER FREE() -> Heap Exploitation ... Why you should keep track of your pointers.
 * A flaw in glibc's heap management.
 * Teo Honda-Scully | 2022. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("%s", "Hello World");
}