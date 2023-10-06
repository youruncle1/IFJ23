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
#include <stdbool.h>

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
    
    // if it's not keyword then it's identifier
    return (token_t){.type = TK_IDENTIFIER, .line = line, .data.String = identifier};
}

token_t create_token(tk_type_t type, unsigned int line) {
    return (token_t){.type = type, .line = line};
}

token_t get_token(scanner_t *scanner) {
    token_t token;
    char symb;
    int idx = 0;
    int nested_comment_count = 0;

    init_buffer(&scanner->buffer, 2);

    // FSM
    while (1) {
        
        if (symb == '\n') {
            scanner->line++;
        }
        symb = fgetc(scanner->input);
        
        switch (scanner->state) {
            
            case START: {
                if (isspace(symb)) {
                    if (symb == '\n') {
                        scanner->line++;
                    }
                    break;
                }
                
                switch(symb) {
                    /* SINGULAR SYMBOL LEXEMS */
                    case 'EOF':
                        return create_token(TK_EOF, scanner->line);
                    case '(':
                        return create_token(TK_LPAR, scanner->line);
                    case ')':
                        return create_token(TK_RPAR, scanner->line);
                    case '{':
                        return create_token(TK_RBRACE, scanner->line);
                    case '}':
                        return create_token(TK_LBRACE, scanner->line);
                    case ':':
                        return create_token(TK_COLON, scanner->line);
                    case ';':
                        return create_token(TK_SEMICOLON, scanner->line);
                    case ',':
                        return create_token(TK_COMMA, scanner->line);
                    case '+':
                        return create_token(TK_PLUS, scanner->line);
                    case '*':
                        return create_token(TK_MUL, scanner->line);
                    /* MULTI SYMBOL LEXEMS */
                    case '/':
                        scanner->state = DIV;
                        continue;
                    case '"':
                        scanner->state = STRING_S;
                        continue;
                    case '-':
                        scanner->state = MINUS;
                        continue;
                    case '=':
                        scanner->state = ASSIGN;
                        continue;
                    case '<':
                        scanner->state = LT;
                        continue;
                    case '>':
                        scanner->state = GT;
                        continue;
                    case '!':
                        scanner->state = UNWRAP;
                        continue;
                    case '?':
                        scanner->state = COALESCE;
                        continue;
                }
                if (isdigit(symb)) {
                    scanner->state = DIGIT;
                    append_to_buffer(&scanner->buffer, symb);
                    break;
                }

                if (isalpha(symb) || symb == '_') {
                    scanner->state = IDENTIFIER;
                    append_to_buffer(&scanner->buffer, symb);
                    break;
                }
                /*
                TODO
                ERROR 
                HANDLING (ERROR_EXIT)
                */
                exit(1);
                break;
            }

            case IDENTIFIER: {
                if (isalpha(symb) || isdigit(symb) || symb == '_') {
                    append_to_buffer(&scanner->buffer, symb);
                } else if (symb == '?') {
                    scanner->state = IDENTIFIER_TYPE;
                } else {
                    char *identifier = buffer_to_string(&scanner->buffer);

                    if (strcmp(identifier, "_") == 0) {
                        free(identifier); // Free the allocated memory for identifier
                        return create_token(TK_UNDERSCORE, scanner->line);
                    }

                    token_t token = get_identifier(identifier, scanner->line);
                    
                    if (token.type != TK_IDENTIFIER) {
                        free(identifier); // If it's a keyword, we don't need the string anymore
                    }
                    // Note: If it's a TK_IDENTIFIER, the string will be associated with the token. 
                    // Ensure you free this string later when done using the token.

                    //scanner->state = START; // Reset state
                    return token; // Return the identified token
                }
                break;
            }

            case IDENTIFIER_TYPE: {
                ungetc(symb, scanner->input);  // Already got ?, next symbol is different token
                char* identifier_str = buffer_to_string(&scanner->buffer);
                if (strcmp(identifier_str, "double?") == 0) {
                    create_token(TK_KW_DOUBLE_OPT, scanner->line);
                } else if (strcmp(identifier_str, "int?") == 0) {
                    create_token(TK_KW_INT_OPT, scanner->line);
                } else if (strcmp(identifier_str, "string?") == 0) {
                    create_token(TK_KW_STRING_OPT, scanner->line);
                } else {
                    /*
                    TODO
                    ERROR HANDLING
                    */
                    exit(-1);
                }
                free(identifier_str);
                
                // Reset state
                scanner->state = START;
                free_buffer(&scanner->buffer);
                return token;
                break;
            }

            case DIV: {
                if (symb == '/') {  
                    scanner->state = OLCOMMENT;
                } else if (symb == '*') { 
                    scanner->state = BLCOMMENT;
                    nested_comment_count = 1;
                } else {  // Just a division operator
                    ungetc(symb, scanner->input);  // Revert the last char read
                    return create_token(TK_DIV, scanner->line);
                }
                break;
            }

            case OLCOMMENT: {
                do {
                    symb = fgetc(scanner->input);
                    if (symb == '\n') {
                        scanner->line++;
                        scanner->state = START;
                        break;
                    }
                } while (symb != EOF);

                if (symb == EOF) {
                    ungetc(symb, scanner->input);  // Revert the EOF character
                    scanner->state = START;
                }
                break;
            }

            case BLCOMMENT: {
                if (symb == '*') {
                    scanner->state = BLCOMMENT_E1;
                } else if (symb == '/') {
                    char peek_symb = fgetc(scanner->input);
                    if (peek_symb == '*') {
                        nested_comment_count++;  // New nested comment found
                    } else {
                        ungetc(peek_symb, scanner->input);  // Revert the last char read
                    }
                } else if (symb == EOF) {
                    /*
                    TODO
                    ERROR: EOF inside a comment
                    */
                    exit(-1);
                } else if (symb == '\n') {
                    scanner->line++;
                }
                break;
            }

            case BLCOMMENT_E1: {
                if (symb == '/') {
                    nested_comment_count--;  // Close one nested comment
                    if (nested_comment_count == 0) {  // All nested comments are closed
                        scanner->state = START;
                    } else {  
                        scanner->state = BLCOMMENT;
                    }
                } else if (symb == '*') {  
                    // If we get another *, stay in BLCOMMENT_E1 and wait for /
                } else {
                    scanner->state = BLCOMMENT;  
                }
                break;
            }

            case LT: {
                if (symb == '=') {  
                    return create_token(TK_LE, scanner->line);  // Less or equal
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_LT, scanner->line);  // Just less than
                }
            }

            case GT: {
                if (symb == '=') {  
                    return create_token(TK_GE, scanner->line);  // Greater or equal
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_GT, scanner->line);  // Just greater than
                }
            }

            case ASSIGN: {
                if (symb == '=') {  
                    return create_token(TK_EQ, scanner->line);  // Equal
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_ASSIGN, scanner->line);  // Just assignment
                }
            }

            case UNWRAP: {
                if (symb == '=') {  
                    return create_token(TK_NEQ, scanner->line);  // Not equal
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_UNWRAP, scanner->line); // Just unwrap
                }
            }

            case COALESCE: {
                if (symb == '?') {  
                    return create_token(TK_COALESCE, scanner->line);
                } else {
                    /*
                    TODO
                    ERROR HANDLING (ERROR_EXIT): Unexpected character after ?
                    */
                    exit(-1);
                }
            }
        }
    }
}

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

char* buffer_to_string(buffer_t *buffer) {
    char *result = (char *)malloc(buffer->size + 1);  // +1 for the null terminator
    if (!result) {
        /* 
        TODO
        ERROR HANDLE
        */
        exit(-1);
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
