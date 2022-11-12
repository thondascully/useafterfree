#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int *notecard;
int *pw;

void check() {
    if (pw == NULL) {
        pw = malloc(sizeof(int));
        *pw = rand() % 1336;
    }

    if (1337 == *pw) {
        printf("0ski decides to let you through! Congragulations!\n");
        exit(0);
    }
    printf("0ski does not want to let you through!\n");
}

void write_notecard() {
    int user_input;

    if (notecard == NULL) {
        printf("There's no notecard to write to!\n");
        return;
    }
    printf("Write your number to the notecard: ");
    scanf("%d", &user_input);
    *notecard = user_input;

    printf("\n");
}

void destroy_notecard() {
    if (notecard == NULL) {
        printf("There's no notecard to destroy!\n");
        return;
    }
    free(notecard);
    printf("Destroyed notecard!\n");
}

void create_notecard() {
    if (notecard != NULL) {
        printf("You already have an notecard!\n");
        return;
    }
    notecard = malloc(sizeof(int));
    printf("Created an notecard!\n");
}

int main() {
    int inp;
    time_t t;
    srand((unsigned) time(&t));

    notecard = NULL;
    pw = NULL;

    printf("There's a large 0ski guarding the door. He might be in the mood to let you through.\n");

    while (1) {

        printf("\n\n1: Create notecard\n");
        printf("2: Destroy notecard\n");
        printf("3: Write to notecard\n");
        printf("4: Check if 0ski feels like letting you through\n\n");

        scanf("%d", &inp);

        if (inp == 1)
            create_notecard();
        else if (inp == 2)
            destroy_notecard();
        else if (inp == 3)
            write_notecard();
        else if (inp == 4)
            check();
    }
    
    return 1;
}