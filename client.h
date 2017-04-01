#pragma once

#include "result.h"


/**
 * Start a client. Run until something forces the client to quit, like the server
 * or the user.
 *
 * @return Result of the function
 */
Result RunClient();
