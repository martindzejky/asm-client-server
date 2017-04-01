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

    RETURN_ERROR("Unknown command");
}