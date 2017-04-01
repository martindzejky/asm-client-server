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
#include "constants.h"
#include "server.h"
#include "prompt.h"
#include "helpers.h"


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
    address.sin_port = htons(socketPort); // TODO: Allow specifying the port
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


Result ForkForAccepting(int *childPID) {
    // fork a new process for accepting incoming connections
    int pid = fork();

    if (pid < 0) {
        RETURN_STANDARD_CRASH;
    }

    if (pid > 0) {
        // this is the parent, return and continue the execution
        *childPID = pid;
        Result result;
        result.type = OK;
        return result;
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
            continue;
        }

        // TODO: Fork a new worker
    }
#pragma clang diagnostic pop
}


Result RunServer() {
    int acceptPID;

    Result result;
    CALL_AND_HANDLE_RESULT(MakeServerSocket());
    CALL_AND_HANDLE_RESULT(ForkForAccepting(&acceptPID));

    // the socket is now ready and accepting connections
    printf("Server started and accepting connections on port %d\n", socketPort);

    // make a buffer for the commands
    char *buffer = malloc(sizeof(char) * commandBufferSize);

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

        // free the buffer for command
        free(command);
    }

    // release the buffer
    free(buffer);

    // kill the accept child process
    kill(acceptPID, SIGKILL);

    CALL_AND_HANDLE_RESULT(FreeServerSocket());

    return result;
}
