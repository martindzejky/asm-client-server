#include <printf.h>
#include "options.h"
#include "server.h"
#include "client.h"


/**
 * The program entry function. Parses the command line arguments and
 * starts either a server or a client.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 * @return Exit code
 */
int main(int argc, char *argv[]) {

    ParseOptions(argc, argv);
    Options *options = GetOptions();

    // display help
    if (options->displayHelp) {
        printf("\nUsage: %s [-h] [-s] [-c]\n", argv[0]);
        printf("Martin Jakubik ~ ASM project\n\nOptions:\n");
        printf("-h Display this help\n");
        printf("-s Start a server\n");
        printf("-c Start a client\n");

        return 1;
    }

    Result result;

    // start a server or a client
    if (options->startServer) {
        result = RunServer();
    } else if (options->startClient) {
        result = RunClient();
    }

    ResetOptions();

    // check the result
    if (result.type != OK) {
        printf("\n%s: %s\n", result.type == ERROR ? "Error" : "Crash", result.description);
    }

    return 0;

}
