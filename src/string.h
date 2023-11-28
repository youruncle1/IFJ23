/*
IFJ PROJEKT 2023/2024
file: "string.h"

String helper functions - header files

authors: xpolia05

*/

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

typedef struct {
    char *data;         
    size_t size;       
    size_t capacity;
} buffer_t;


void init_buffer(buffer_t *buffer, size_t initial_capacity);
void append_to_buffer(buffer_t *buffer, char ch);
void append_string_to_buffer(buffer_t *buffer, const char *str, size_t length);
char* buffer_to_string(buffer_t *buffer);
void free_buffer(buffer_t *buffer);
char* trim_multiline_string_indentation(const char* raw_str_val, int closing_delimiter_indentation);

#endif // STRING_H