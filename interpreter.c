#include <string.h>
#include "interpreter.h"
#include "helpers.h"


Result InterpretCommand(char *command, char *params, char *outputBuffer) {
    // figure out what command was passed in,
    // this is like a huge switch

    if (strcmp(command, "ping") == 0) {
        strcat(outputBuffer, "pong");
        RETURN_OK;
    }

    if (strcmp(command, "cat") == 0 || strcmp(command, "echo") == 0) {
        strcat(outputBuffer, params);
        RETURN_OK;
    }

    if (strcmp(command, "help") == 0) {
        strcat(outputBuffer, "Available commands:\n");
        strcat(outputBuffer, "quit|exit - exit the program\n");
        strcat(outputBuffer, "ping - reply with pong, test connection\n");
        strcat(outputBuffer, "cat|echo text - echo the parameter text\n");
        strcat(outputBuffer, "help - print the list of commands\n");
        strcat(outputBuffer, "halt|close (server only) - close all connections\n");
        //strcat(outputBuffer, "\n");
        RETURN_OK;
    }

    RETURN_ERROR("Unknown command");
}
