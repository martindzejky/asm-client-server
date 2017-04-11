#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <sys/resource.h>
#include <mach/task_info.h>
#include <mach/task.h>
#include <mach/mach_init.h>
#include <unistd.h>
#include "interpreter.h"
#include "helpers.h"
#include "constants.h"


Result HelpCommand(char *outputBuffer) {
    strcat(outputBuffer, "Available commands:\n");
    strcat(outputBuffer, "quit|exit - exit the program\n");
    strcat(outputBuffer, "halt|close (server only) - close all connections\n");

    strcat(outputBuffer, "ping - reply with pong, test connection\n");
    strcat(outputBuffer, "cat|echo text - echo the parameter text\n");
    strcat(outputBuffer, "help - print the list of commands\n");

    strcat(outputBuffer,
           "info [date|time|cpu|ram] - display useful information, current date, time, cpu, and ram usage\n");

    RETURN_OK;
}


Result GetTime(struct timeval *timeSinceEpoch) {
    int errorCode;
    asm (
        "syscall"                                        // call kernel
        : "=a" (errorCode)                               // outputs - error code
        : "a" (0x2000074), "D" (timeSinceEpoch), "S" (0) // inputs - syscall number, struct timeval tp, void* tzp
        : "memory"                                       // changed registers
    );

    if (errorCode < 0) {
        RETURN_STANDARD_CRASH;
    }

    RETURN_OK;
}


Result GetCpuTime(float *cpuTime) {
    // get cpu time of process
    int errorCode;
    struct rusage cpuUsage;
    asm (
        "syscall"                                             // call kernel
        : "=a" (errorCode)                                    // outputs - error code
        : "a" (0x2000075), "D" (RUSAGE_SELF), "S" (&cpuUsage) // inputs - syscall number, int who, struct rusage*
        : "memory"                                            // changed registers
    );

    if (errorCode < 0) {
        RETURN_STANDARD_CRASH;
    }

    // get total time in milliseconds
    *cpuTime = cpuUsage.ru_utime.tv_usec + cpuUsage.ru_stime.tv_usec;
    *cpuTime /= 1000.f;

    RETURN_OK;
}


Result GetMemoryUsage(float *residentSize, float *virtualSize) {
    mach_msg_type_number_t outCount = MACH_TASK_BASIC_INFO_COUNT;
    mach_task_basic_info_data_t taskInfo;

    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t) &taskInfo, &outCount) != KERN_SUCCESS) {
        RETURN_CRASH("Failed to get task info");
    }

    // get used memory
    *residentSize = taskInfo.resident_size;
    *virtualSize = taskInfo.virtual_size;

    // multiply by page size
    float pageSize = getpagesize();
    *residentSize *= pageSize;
    *virtualSize *= pageSize;

    // convert to kilobytes
    *residentSize /= 1000.f;
    *virtualSize /= 1000.f;

    RETURN_OK;
}


Result InfoCommand(char *params, char *outputBuffer) {
    // compare the params to the 4 possible options, run the
    // appropriate function to fetch the data, or return an error

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

    if (strcmp(params, "cpu") == 0) {
        float cpuTime;
        {
            Result result;
            CALL_AND_HANDLE_RESULT(GetCpuTime(&cpuTime));
        }

        sprintf(outputBuffer, "Used processor time is %.3f ms\n", cpuTime);

        RETURN_OK;
    }

    if (strcmp(params, "ram") == 0) {
        float residentUsage;
        float virtualUsage;
        {
            Result result;
            CALL_AND_HANDLE_RESULT(GetMemoryUsage(&residentUsage, &virtualUsage));
        }

        sprintf(outputBuffer, "Used resident memory is %.3f kB\nUsed virtual memory is %.3f kB\n",
                residentUsage, virtualUsage);

        RETURN_OK;
    }

    // by default show usage
    RETURN_ERROR("Usage: info [date|time|cpu|ram]");
}


Result RunBashCommand(char *commandBuffer, char *outputBuffer) {
    // redirect stderr
    strcat(commandBuffer, " 2>&1");

    // open the file descriptor for the shell
    FILE *shell = popen(commandBuffer, "r");

    if (shell == NULL) {
        RETURN_STANDARD_CRASH;
    }

    // get the output
    size_t charsRead = fread(outputBuffer, sizeof(char), (size_t) outputBufferSize, shell);
    outputBuffer[charsRead] = 0;

    if (ferror(shell) != 0) {
        RETURN_STANDARD_CRASH;
    }

    pclose(shell);

    // check if command was found
    if (strstr(outputBuffer, "command not found") != NULL) {
        RETURN_ERROR("Command not found");
    }

    RETURN_OK;
}


Result InterpretCommand(char *command, char *params, char *commandBuffer, char *outputBuffer) {
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

    // try running the command in bash
    return RunBashCommand(commandBuffer, outputBuffer);
}
