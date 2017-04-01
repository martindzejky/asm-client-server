#include <stdlib.h>
#include <getopt.h>
#include "options.h"


char *permittedOptions = "hsc";


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
