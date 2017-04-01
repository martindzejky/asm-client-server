#include <stdbool.h>
#include <stdlib.h>
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
        RETURN_CRASH("No hostname localhost found");
    }

    // make an address to connect to
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(socketPort); // TODO: Allow specifying the port
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


Result RunClient() {
    Result result;
    CALL_AND_HANDLE_RESULT(MakeClientSocket());

    // make a buffer for the commands
    char *buffer = malloc(sizeof(char) * commandBufferSize);

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

        // TODO: Send to server and display the result

        // free the buffer for command
        free(command);
    }

    // release the buffer
    free(buffer);

    CALL_AND_HANDLE_RESULT(FreeClientSocket());

    return result;
}
