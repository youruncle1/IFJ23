/*
IFJ PROJEKT 2023/2024
file: "string.h"

String helper functions - header files

authors: xpolia05

*/

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

/**
 * @brief Structure representing a dynamic string buffer.
 *
 * @param data Pointer to the character array.
 * @param size Current size of the string in the buffer.
 * @param capacity Total allocated capacity of the buffer.
 */
typedef struct {
    char *data;         
    size_t size;       
    size_t capacity;
} buffer_t;

/**
 * @brief Initializes a buffer with a specified initial capacity.
 *
 * @param buffer Pointer to the buffer to be initialized.
 * @param initial_capacity The initial allocated capacity of the buffer.
 */
void init_buffer(buffer_t *buffer, size_t initial_capacity);

/**
 * @brief Appends a single character to the buffer.
 *
 * @param buffer Pointer to the buffer.
 * @param ch The character to append.
 */
void append_to_buffer(buffer_t *buffer, char ch);

/**
 * @brief Appends a string to the buffer.
 *
 * @param buffer Pointer to the buffer.
 * @param str Pointer to the string to append.
 * @param length Length of the string to append.
 */
void append_string_to_buffer(buffer_t *buffer, const char *str, size_t length);

/**
 * @brief Converts the contents of the buffer into a new null-terminated string.
 *
 * @param buffer Pointer to the buffer.
 * @return Newly allocated string with the contents of the buffer.
 */
char* buffer_to_string(buffer_t *buffer);

/**
 * @brief Frees the memory allocated for the buffer.
 *
 * @param buffer Pointer to the buffer to be freed.
 */
void free_buffer(buffer_t *buffer);

/**
 * @brief Trims indentation from a multi-line string based on the indentation of the closing delimiter.
 *
 * @param raw_str_val The raw multi-line string to be trimmed.
 * @param closing_delimiter_indentation The indentation level of the closing delimiter.
 * @return Newly allocated string with trimmed indentation.
 */
char* trim_multiline_string_indentation(const char* raw_str_val, int closing_delimiter_indentation);

#endif // STRING_H