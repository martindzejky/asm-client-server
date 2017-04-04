#include <stdio.h>
#include <stdbool.h>
#include "prompt.h"


bool firstTime = true;


void PrintPrompt() {
    printf("\n");

    // print first-time help
    if (firstTime) {
        firstTime = false;
        printf("Enter commands and their parameters, use 'help' to see all available commands\n");
    }

    // print the prompt
    printf("Enter command: ");
    fflush(stdout);
}


void GetCommand(char *buffer, int size) {
    getline(&buffer, (size_t *) &size, stdin);
}
