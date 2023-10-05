/*
IFJ PROJEKT 2023/2024
file: "src/scanner.c"

Lexical analysis

authors: xpolia05
         xbencs00
         xrusna08
         xhonze01
*/

#include "scanner.h"
#include <string.h>

token_t get_identifier(char *identifier, unsigned int line) {
    if (strcmp(identifier, "double") == 0)
        return (token_t){.type = TK_KW_DOUBLE, .line = line};
    else if (strcmp(identifier, "else") == 0)
        return (token_t){.type = TK_KW_ELSE, .line = line};
    else if (strcmp(identifier, "func") == 0)
        return (token_t){.type = TK_KW_FUNC, .line = line};
    else if (strcmp(identifier, "if") == 0)
        return (token_t){.type = TK_KW_IF, .line = line};
    else if (strcmp(identifier, "int") == 0)
        return (token_t){.type = TK_KW_INT, .line = line};
    else if (strcmp(identifier, "let") == 0)
        return (token_t){.type = TK_KW_LET, .line = line};
    else if (strcmp(identifier, "nil") == 0)
        return (token_t){.type = TK_KW_NIL, .line = line};
    else if (strcmp(identifier, "return") == 0)
        return (token_t){.type = TK_KW_RETURN, .line = line};
    else if (strcmp(identifier, "string") == 0)
        return (token_t){.type = TK_KW_STRING, .line = line};
    else if (strcmp(identifier, "var") == 0)
        return (token_t){.type = TK_KW_VAR, .line = line};
    else if (strcmp(identifier, "while") == 0)
        return (token_t){.type = TK_KW_WHILE, .line = line};
    
    // if not keyword then identifier
    return (token_t){.type = TK_IDENTIFIER, .line = line, .data.String = identifier};
}

// Initialize the buffer
void init_buffer(buffer_t *buffer, size_t initial_capacity) {
    buffer->data = (char *)malloc(initial_capacity);
    if (!buffer->data) {
        /* 
        TODO
        ERROR HANDLE
        */
        exit(-1);
    }
    buffer->size = 0;
    buffer->capacity = initial_capacity;
}

// Append a character to the buffer
void append_to_buffer(buffer_t *buffer, char ch) {
    if (buffer->size >= buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2; 
        char *new_data = (char *)realloc(buffer->data, new_capacity);
        if (!new_data) {
            /* 
            TODO
            ERROR HANDLE
            */
            free(buffer->data);
            exit(-1);
        }
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }
    buffer->data[buffer->size++] = ch;
}

void free_buffer(buffer_t *buffer) {
    free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
}
