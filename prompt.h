#pragma once


/**
 * Print the prompt for the user.
 */
void PrintPrompt();


/**
 * Get a command from the user.
 *
 * @param buffer Where to save the command
 * @param size Size of the buffer
 */
void GetCommand(char *buffer, int size);
