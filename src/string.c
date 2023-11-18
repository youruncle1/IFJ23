/*
IFJ PROJEKT 2023/2024
file: "src/string.c"

String helper functions

authors: xpolia05
         xbencs00
         xrusna08
         xhonze01
*/

#include "string.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

void init_buffer(buffer_t *buffer, size_t initial_capacity) {
    buffer->data = (char *)malloc(initial_capacity);
    if (!buffer->data) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
    }
    buffer->size = 0;
    buffer->capacity = initial_capacity;
}

void append_to_buffer(buffer_t *buffer, char ch) {
    if (buffer->size >= buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2; 
        char *new_data = (char *)realloc(buffer->data, new_capacity);
        if (!new_data) {
            free(buffer->data);
            handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
        }
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }
    buffer->data[buffer->size++] = ch;
}

void append_string_to_buffer(buffer_t *buffer, const char *str, size_t length) {
    for (size_t i = 0; i < length; i++) {
        append_to_buffer(buffer, str[i]);
    }
}

char* buffer_to_string(buffer_t *buffer) {
    char *result = (char *)malloc(buffer->size + 1);  // +1 for the null terminator
    if (!result) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
    }

    memcpy(result, buffer->data, buffer->size);
    result[buffer->size] = '\0'; 
    return result;
}

void free_buffer(buffer_t *buffer) {
    free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
}

char* trim_multiline_string_indentation(const char* raw_str_val, int closing_delimiter_indentation) {
    buffer_t trimmed_buffer;
    init_buffer(&trimmed_buffer, 4);

    const char* line_start = raw_str_val;
    while (*line_start) {
        // Find the end of the current line
        const char* line_end = strchr(line_start, '\n');
        if (!line_end) {
            line_end = line_start + strlen(line_start); // last line
        }

        // find indentation
        int line_indentation = 0;
        while (line_start + line_indentation < line_end && line_start[line_indentation] == ' ') {
            line_indentation++;
        }

        // Find out how much to trim from the current line
        int trim_amount = (line_indentation < closing_delimiter_indentation) ? line_indentation : closing_delimiter_indentation;

        append_string_to_buffer(&trimmed_buffer, line_start + trim_amount, line_end - (line_start + trim_amount));

        if (*line_end) {
            append_to_buffer(&trimmed_buffer, '\n');
            line_start = line_end + 1;
        } else {
            line_start = line_end;
        }
    }

    char* trimmed_string = buffer_to_string(&trimmed_buffer);
    size_t length = strlen(trimmed_string);
    if (length > 0 && trimmed_string[length - 1] == '\n') {
        trimmed_string[length - 1] = '\0';
    }
    free(trimmed_buffer.data);
    return trimmed_string;
}