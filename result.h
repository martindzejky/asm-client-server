#pragma once


/**
 * Type of the result. Whether something ended well or not.
 */
typedef enum ResultType {

    OK = 0,
    ERROR = -1, // expected, mostly due to the user
    CRASH = -2 // unexpected

} ResultType;


/**
 * Represents a result of a function.
 */
typedef struct Result {

    ResultType type;
    char *description;

} Result;
