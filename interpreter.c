#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include "interpreter.h"
#include "helpers.h"
#include "constants.h"


Result HelpCommand(char *outputBuffer) {
    strcat(outputBuffer, "Available commands:\n");
    strcat(outputBuffer, "quit|exit - exit the program\n");
    strcat(outputBuffer, "ping - reply with pong, test connection\n");
    strcat(outputBuffer, "cat|echo text - echo the parameter text\n");
    strcat(outputBuffer, "help - print the list of commands\n");
    strcat(outputBuffer, "halt|close (server only) - close all connections\n");
    strcat(outputBuffer,
           "info [date|time|cpu|ram] - display useful information, current date, time, cpu, and ram usage\n");
    RETURN_OK;
}


Result GetTime(struct timeval *timeSinceEpoch) {
    int errorCode;
    asm (
    "syscall"                                        // call kernel
    : "=a" (errorCode)                               // outputs - error code
    : "a" (0x2000074), "D" (timeSinceEpoch), "S" (0) // inputs - syscall number, struct timeval tp, void tzp
    : "memory"                                       // changed registers
    );

    if (errorCode < 0) {
        RETURN_STANDARD_CRASH;
    }

    RETURN_OK;
}


Result InfoCommand(char *params, char *outputBuffer) {

    if (strcmp(params, "date") == 0) {
        struct timeval timeSinceEpoch;
        {
            Result result;
            CALL_AND_HANDLE_RESULT(GetTime(&timeSinceEpoch));
        }

        strftime(outputBuffer, (size_t) outputBufferSize, "Current time is %d. %m. %Y",
                 localtime(&timeSinceEpoch.tv_sec));

        RETURN_OK;
    }

    if (strcmp(params, "time") == 0) {
        struct timeval timeSinceEpoch;
        {
            Result result;
            CALL_AND_HANDLE_RESULT(GetTime(&timeSinceEpoch));
        }

        strftime(outputBuffer, (size_t) outputBufferSize, "Current time is %H:%M:%S",
                 localtime(&timeSinceEpoch.tv_sec));

        RETURN_OK;
    }

    // by default show usage
    RETURN_ERROR("Usage: info [date|time|cpu|ram]");
}


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
        return HelpCommand(outputBuffer);
    }

    if (strcmp(command, "info") == 0) {
        return InfoCommand(params, outputBuffer);
    }

    RETURN_ERROR("Unknown command");
}
