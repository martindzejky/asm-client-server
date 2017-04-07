#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include "constants.h"
#include "server.h"
#include "prompt.h"
#include "helpers.h"
#include "interpreter.h"
#include "options.h"


int serverSocket;


void SplitCommandParams(char *buffer, char **command, char **params) {
    int commandSize;
    for (commandSize = 0; !isspace(buffer[commandSize]) && buffer[commandSize] != 0; commandSize++);

    // split and copy command
    *command = malloc(sizeof(char) * (commandSize + 1));
    for (int i = 0; i < commandSize; i++) {
        (*command)[i] = buffer[i];
    }
    (*command)[commandSize] = 0;

    // set pointer to params
    *params = buffer + commandSize + 1;

    // get rid of whitespace at the end of params
    size_t paramsLen = strlen(*params);
    (*params)[paramsLen - 1] = 0;
}


Result MakeServerSocket() {
    // create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        RETURN_STANDARD_CRASH;
    }

    // make an address to bind to
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(GetOptions()->port);
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_len = 0;
    bzero(address.sin_zero, 8);

    // bind the socket
    if (bind(serverSocket, (const struct sockaddr *) &address, sizeof(address)) < 0) {
        RETURN_STANDARD_CRASH;
    }

    // start listening
    if (listen(serverSocket, 5) < 0) {
        RETURN_STANDARD_CRASH;
    }

    RETURN_OK;
}


Result FreeServerSocket() {
    // close the socket
    if (close(serverSocket) < 0) {
        RETURN_STANDARD_CRASH;
    }

    RETURN_OK;
}


void ForkForClient(int clientSocket) {
    // save the parent id
    int parentPID = getpid();

    // fork a new process specially for the client
    int pid = fork();

    if (pid != 0) {
        // either something crashed or this is the parent
        return;
    }

    char *commandBuffer = malloc(sizeof(char) * commandBufferSize);
    char *outputBuffer = malloc(sizeof(char) * outputBufferSize);

    time_t lastTimeActive;
    time(&lastTimeActive);

    // run until the client closes the connection
    while (true) {
        // check if the server is still running
        if (getppid() != parentPID) {
            // nope, stop
            break;
        }

        // check for timeout
        Options *options = GetOptions();
        if (options->timeout > 0) {
            time_t currentTime;
            time(&currentTime);

            if (difftime(currentTime, lastTimeActive) > options->timeout) {
                // close the connection
                break;
            }
        }

        // poll for data
        struct pollfd fds[1];
        fds[0].fd = clientSocket;
        fds[0].events = POLLIN;

        if (poll(fds, 1, 1) <= 0) {
            // no data to read, loop to detect server shutdown
            continue;
        }

        // read command
        bzero(commandBuffer, (size_t) commandBufferSize);
        ssize_t charsRead;
        if ((charsRead = read(clientSocket, commandBuffer, (size_t) commandBufferSize)) < 0) {
            // something went horribly wrong
            break;
        }

        // if it is 0, the connection has been closed
        if (charsRead == 0) {
            break;
        }

        // reset timer
        time(&lastTimeActive);

        char *command;
        char *params;
        SplitCommandParams(commandBuffer, &command, &params);

        bzero(outputBuffer, (size_t) outputBufferSize);
        Result interpretResult = InterpretCommand(command, params, outputBuffer);

        if (interpretResult.type == OK) {
            // if everything went well, send the output
            if (write(clientSocket, outputBuffer, strlen(outputBuffer)) < 0) {
                // something went horribly wrong
                break;
            }
        } else if (interpretResult.type == ERROR) {
            // if there was an error, send that
            strcat(outputBuffer, "Error: ");
            strcat(outputBuffer, interpretResult.description);

            if (write(clientSocket, outputBuffer, strlen(outputBuffer)) < 0) {
                // something went horribly wrong
                break;
            }
        } else {
            // something went horribly wrong
            break;
        }

        // free the buffer for command
        free(command);
    }

    free(outputBuffer);
    free(commandBuffer);

    // close the socket with the client
    close(clientSocket);

    exit(EXIT_SUCCESS);
}


Result ForkForAccepting(int *childPID) {
    // fork a new process for accepting incoming connections
    int pid = fork();

    if (pid < 0) {
        RETURN_STANDARD_CRASH;
    }

    if (pid > 0) {
        // this is the parent, return and continue the execution
        *childPID = pid;
        RETURN_OK;
    }

    // as a child, run forever and accept connections
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);

        // accept a new socket connection
        int newSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressSize);
        if (newSocket < 0) {
            // there's no way to tell the parent process that there was an error
            // so just keep accepting connections
            continue;
        }

        ForkForClient(newSocket);

        // close the socket since we don't need it in this process
        close(newSocket);
    }
#pragma clang diagnostic pop
}


Result RunServer() {
    int acceptPID;

    Result result;
    CALL_AND_HANDLE_RESULT(MakeServerSocket());
    CALL_AND_HANDLE_RESULT(ForkForAccepting(&acceptPID));

    // the socket is now ready and accepting connections
    printf("Server started and accepting connections on port %d\n", GetOptions()->port);

    // make buffers
    char *buffer = malloc(sizeof(char) * commandBufferSize);
    char *outputBuffer = malloc(sizeof(char) * outputBufferSize);

    // loop
    while (true) {
        PrintPrompt();
        GetCommand(buffer, commandBufferSize);

        char *command;
        char *params;
        SplitCommandParams(buffer, &command, &params);

        // check if quit or exit was entered
        if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
            break;
        }

        // check for halt or close
        if (strcmp(command, "halt") == 0 || strcmp(command, "close") == 0) {
            // restart the process that waits for connections which should
            // also close all connections
            kill(acceptPID, SIGKILL);
            CALL_AND_HANDLE_RESULT(ForkForAccepting(&acceptPID));

            printf("Connections closed\n");

            free(command);
            continue;
        }

        bzero(outputBuffer, (size_t) outputBufferSize);
        Result interpretResult = InterpretCommand(command, params, outputBuffer);

        // print the output if everything went well
        if (interpretResult.type == OK) {
            printf("%s\n", outputBuffer);
        } else if (interpretResult.type == ERROR) {
            printf("Error: %s\n", interpretResult.description);
        } else {
            return interpretResult;
        }

        // free the buffer for command
        free(command);
    }

    // release the buffers
    free(outputBuffer);
    free(buffer);

    // kill the accept child process
    kill(acceptPID, SIGKILL);

    CALL_AND_HANDLE_RESULT(FreeServerSocket());

    return result;
}
