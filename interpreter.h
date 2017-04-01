#pragma once

#include "result.h"


/**
 * Interpret a command from the user.
 *
 * @param command The command to interpret
 * @param params Command parameters
 * @return Result of the operation
 */
Result InterpretCommand(char *command, char *params, char *outputBuffer);
