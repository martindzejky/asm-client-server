#include <stdlib.h>
#include <getopt.h>
#include "options.h"
#include "constants.h"


char *permittedOptions = "hscp:t:";


Options *options = NULL;


void ParseOptions(int argc, char *argv[]) {
    // remove existing options
    if (options) {
        free(options);
    }

    // make new options
    options = malloc(sizeof(Options));

    // set defaults
    options->startServer = true;
    options->startClient = false;
    options->displayHelp = false;
    options->port = socketPort;
    options->timeout = -1;

    // parse command line arguments
    int flag;
    while ((flag = getopt(argc, argv, permittedOptions)) != -1) {
        switch (flag) {
            case 'h':
                options->displayHelp = true;
                break;

            case 's':
                options->startServer = true;
                options->startClient = false;
                break;

            case 'c':
                options->startServer = false;
                options->startClient = true;
                break;

            case 'p':
                options->port = atoi(optarg);
                break;

            case 't':
                options->timeout = atoi(optarg);
                break;

            default:
                options->displayHelp = true;
                break;
        }
    }
}


Options *GetOptions() {
    return options;
}


void ResetOptions() {
    if (options) {
        free(options);
        options = NULL;
    }
}
