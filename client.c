#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include "constants.h"
#include "client.h"
#include "prompt.h"
#include "helpers.h"
#include "options.h"


int clientSocket;


Result MakeClientSocket() {
    // create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        RETURN_STANDARD_CRASH;
    }

    // get information about the host
    struct hostent *server = gethostbyname("localhost"); // TODO: Allow specifying the host
    if (server == NULL) {
        RETURN_CRASH("No hostname localhost found, WTF");
    }

    // make an address to connect to
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(GetOptions()->port);
    address.sin_len = 0;
    bcopy(server->h_addr, &address.sin_addr.s_addr, (size_t) server->h_length);
    bzero(address.sin_zero, 8);

    // connect to the server
    if (connect(clientSocket, (const struct sockaddr *) &address, sizeof(address)) < 0) {
        RETURN_STANDARD_CRASH;
    }

    RETURN_OK;
}


Result FreeClientSocket() {
    // close the socket
    if (close(clientSocket) < 0) {
        RETURN_STANDARD_CRASH;
    }

    RETURN_OK;
}


Result ReadFile(char *params, char *fileBuffer) {
    // check if file name was passed in
    if (strlen(params) == 0) {
        RETURN_ERROR("Usage: run filename");
    }

    // open the file
    FILE *file;
    if ((file = fopen(params, "r")) == NULL) {
        // handle file not found
        if (errno == ENOENT) {
            RETURN_ERROR("File not found");
        } else {
            RETURN_STANDARD_CRASH;
        }
    }

    // read file contents into a buffer
    size_t charsRead = fread(fileBuffer, sizeof(char), (size_t) fileBufferSize, file);
    fileBuffer[charsRead] = 0;

    fclose(file);
    RETURN_OK;
}


Result RunClient() {
    Result result;

    printf("Connecting to a server at localhost:%d\n", GetOptions()->port); // TODO: Allow changing hostname
    CALL_AND_HANDLE_RESULT(MakeClientSocket());
    printf("Connected to the server\n");

    // make buffers
    char *buffer = malloc(sizeof(char) * commandBufferSize);
    char *outputBuffer = malloc(sizeof(char) * outputBufferSize);

    // loop
    while (true) {
        PrintPrompt();
        GetCommand(buffer, commandBufferSize);

        // find the size of the command
        int commandSize;
        for (commandSize = 0; !isspace(buffer[commandSize]) && buffer[commandSize] != 0; commandSize++);

        // split the command
        char *command = malloc(sizeof(char) * (commandSize + 1));
        for (int i = 0; i < commandSize; i++) {
            command[i] = buffer[i];
        }
        command[commandSize] = 0;

        // check if quit or exit was entered
        if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
            break;
        }

        // check run, it requires that we send the file contents
        if (strcmp(command, "run") == 0) {
            // read the file
            char *fileBuffer = malloc(sizeof(char) * fileBufferSize);
            buffer[strlen(buffer) - 1] = 0;
            Result readFileResult = ReadFile(buffer + commandSize + 1, fileBuffer);

            // check if file exists
            if (readFileResult.type == ERROR) {
                printf("Error: %s\n", readFileResult.description);
                continue;
            }

            // send the command
            if (write(clientSocket, "runc", 5) < 0) {
                RETURN_STANDARD_CRASH;
            }

            // send the file contents
            if (write(clientSocket, fileBuffer, strlen(fileBuffer)) < 0) {
                RETURN_STANDARD_CRASH;
            }

            free(fileBuffer);
        } else {
            // send the command and params normally
            if (write(clientSocket, buffer, strlen(buffer)) < 0) {
                RETURN_STANDARD_CRASH;
            }
        }

        // read the response
        bzero(outputBuffer, (size_t) outputBufferSize);
        ssize_t charsRead;
        if ((charsRead = read(clientSocket, outputBuffer, (size_t) outputBufferSize)) < 0) {
            RETURN_STANDARD_CRASH;
        }

        // if it is 0, the connection has been closed
        if (charsRead == 0) {
            RETURN_ERROR("Connection with server lost");
        }

        // print the response
        printf("%s\n", outputBuffer);

        // free the buffers
        free(command);
    }

    // release the buffers
    free(outputBuffer);
    free(buffer);

    CALL_AND_HANDLE_RESULT(FreeClientSocket());

    return result;
}
