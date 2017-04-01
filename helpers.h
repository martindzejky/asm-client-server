#pragma once


/**
 * Run a function that returns a Result. If this result
 * is not OK, return it, otherwise continue execution.
 *
 * @note A variable Result result must be defined.
 * @param function The function call
 */
#define CALL_AND_HANDLE_RESULT(function) \
    result = function; \
    if (result.type != OK) { \
        return result; \
    }


/**
 * Return a result with type OK.
 *
 * @note Defines a variable Result result
 */
#define RETURN_OK \
    Result result; \
    result.type = OK; \
    return result;


/**
 * Return a result with type ERROR and
 * the specified error message.
 *
 * @note Defines a variable Result result
 */
#define RETURN_ERROR(message) \
    Result result; \
    result.type = ERROR; \
    result.description = message; \
    return result;


/**
 * Return a result with type CRASH and
 * the specified error message.
 *
 * @note Defines a variable Result result
 */
#define RETURN_CRASH(message) \
    Result result; \
    result.type = CRASH; \
    result.description = message; \
    return result;


/**
 * Return a result with type CRASH and
 * the standard error description.
 *
 * @note Defines a variable Result result
 */
#define RETURN_STANDARD_CRASH \
    Result result; \
    result.type = CRASH; \
    result.description = strerror(errno); \
    return result;
