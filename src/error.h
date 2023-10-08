/*
IFJ PROJEKT 2023/2024
file: "src/error.h"

Error handling - header file

authors: xpolia05
         xbencs00
         xrusna08
         xhonze01
*/

#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdio.h>

typedef enum {
    LEXICAL_ERROR = 1,
    SYNTAX_ERROR = 2,
    SEMANTIC_UNDEFINED_FUNCTION = 3,
    SEMANTIC_FUNCTION_ARGUMENTS = 4,
    SEMANTIC_UNDEFINED_VARIABLE = 5,
    SEMANTIC_RETURN_VALUE = 6,
    SEMANTIC_TYPE_COMPATIBILITY = 7,
    SEMANTIC_TYPE_DERIVATION = 8,
    OTHER_SEMANTIC_ERROR = 9,
    INTERNAL_COMPILER_ERROR = 99
} ErrorCode;

void handle_error(ErrorCode error_code, unsigned line, const char *message);

#endif // ERROR_H
