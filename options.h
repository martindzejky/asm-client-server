#pragma once

#include <stdbool.h>


/**
 * Encapsulates command line arguments.
 */
typedef struct Options {

    bool startServer;
    bool startClient;
    bool displayHelp;
    int port;

} Options;


/**
 * Parse the command line arguments.
 *
 * @param argc Number of arguments
 * @param argv Command line arguments
 */
void ParseOptions(int argc, char *argv[]);


/**
 * Get the options.
 *
 * @return Options
 */
Options *GetOptions();


/**
 * Reset options and free the memory.
 */
void ResetOptions();
