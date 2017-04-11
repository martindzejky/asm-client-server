#pragma once

#include "result.h"


/**
 * Interpret the run command with file contents already read in the buffer.
 *
 * @param fileBuffer Buffer with file contents
 * @param outputBuffer Output buffer
 * @return Result of the operation
 */
Result RunCommandWithBuffer(char *fileBuffer, char *outputBuffer);


/**
 * Interpret a command from the user.
 *
 * @param command The command to interpret
 * @param params Command parameters
 * @param commandBuffer Buffer with the whole command and params
 * @return Result of the operation
 */
Result InterpretCommand(char *command, char *params, char *commandBuffer, char *outputBuffer);
