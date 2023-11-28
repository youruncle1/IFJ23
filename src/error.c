/*
IFJ PROJEKT 2023/2024
file: "error.c"

Error handling

authors: xpolia05

*/

#include "error.h"

void handle_error(ErrorCode error_code, unsigned line, const char *message) {
    
    fprintf(stderr, "Error on line %u: ", line);

    switch (error_code) {
        case LEXICAL_ERROR:
            fprintf(stderr, "Lexical error");
            break;
        case SYNTAX_ERROR:
            fprintf(stderr, "Syntax error");
            break;
        case SEMANTIC_UNDEFINED_FUNCTION:
            fprintf(stderr, "Semantic error: Undefined function");
            break;
        case SEMANTIC_FUNCTION_ARGUMENTS:
            fprintf(stderr, "Semantic error: Invalid function arguments");
            break;
        case SEMANTIC_UNDEFINED_VARIABLE:
            fprintf(stderr, "Semantic error: Undefined variable");
            break;
        case SEMANTIC_RETURN_VALUE:
            fprintf(stderr, "Semantic error: Invalid return value");
            break;
        case SEMANTIC_TYPE_COMPATIBILITY:
            fprintf(stderr, "Semantic error: Type compatibility issue");
            break;
        case SEMANTIC_TYPE_DERIVATION:
            fprintf(stderr, "Semantic error: Unable to derive type");
            break;
        case OTHER_SEMANTIC_ERROR:
            fprintf(stderr, "Other semantic error");
            break;
        case INTERNAL_COMPILER_ERROR:
            fprintf(stderr, "Internal compiler error");
            break;
        default:
            fprintf(stderr, "Unknown error occured");
            break;
    }

    if (message) {
        fprintf(stderr, ": %s", message);
    }

    fprintf(stderr, "\n");

    exit(error_code);
}