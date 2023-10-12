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
#include "error.h"
#include <string.h>
#include <stdbool.h>

/*  
        TODO 

    1. create working multiline string literal parse
    2. comment the code (?doxygen?)
    3. (?) move string logic into different files; string.c/.h  

*/

token_t get_identifier(char *identifier, unsigned int line) {
    if (strcmp(identifier, "Double") == 0)
        return (token_t){.type = TK_KW_DOUBLE, .line = line};
    else if (strcmp(identifier, "else") == 0)
        return (token_t){.type = TK_KW_ELSE, .line = line};
    else if (strcmp(identifier, "func") == 0)
        return (token_t){.type = TK_KW_FUNC, .line = line};
    else if (strcmp(identifier, "if") == 0)
        return (token_t){.type = TK_KW_IF, .line = line};
    else if (strcmp(identifier, "Int") == 0)
        return (token_t){.type = TK_KW_INT, .line = line};
    else if (strcmp(identifier, "let") == 0)
        return (token_t){.type = TK_KW_LET, .line = line};
    else if (strcmp(identifier, "nil") == 0)
        return (token_t){.type = TK_KW_NIL, .line = line};
    else if (strcmp(identifier, "return") == 0)
        return (token_t){.type = TK_KW_RETURN, .line = line};
    else if (strcmp(identifier, "String") == 0)
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
    scanner->state = START;
    token_t token;
    char symb;
    int nested_comment_count = 0;
    int unicode_val, hex_digits_count;

    // FSM
    while (1) {
        
        symb = fgetc(scanner->input);
        
        switch (scanner->state) {
            
            case START: {
                if (isspace(symb)) {
                    if (symb == '\n') 
                        scanner->line++;
                    
                    break;
                }

                switch(symb) {
                    /* SINGULAR SYMBOL LEXEMS */
                    case EOF:
                        return create_token(TK_EOF, scanner->line);
                    case '(':
                        return create_token(TK_LPAR, scanner->line);
                    case ')':
                        return create_token(TK_RPAR, scanner->line);
                    case '{':
                        return create_token(TK_LBRACE, scanner->line);
                    case '}':
                        return create_token(TK_RBRACE, scanner->line);
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
                        init_buffer(&scanner->buffer, 5);
                        scanner->state = STRING;
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
                    init_buffer(&scanner->buffer, 2);
                    append_to_buffer(&scanner->buffer, symb);
                    break;
                }

                if (isalpha(symb) || symb == '_') {
                    scanner->state = IDENTIFIER;
                    init_buffer(&scanner->buffer, 2);
                    append_to_buffer(&scanner->buffer, symb);
                    break;
                }
                handle_error(LEXICAL_ERROR, scanner->line, "Unsupported symbol found");
            }

            case STRING: {
                if (symb == '\\') { 
                    scanner->state = STRING_ESCAPE;
                } else if (symb == '"') {
                    char* str_val = buffer_to_string(&scanner->buffer);
                    free_buffer(&scanner->buffer);
                    token = create_token(TK_STRING, scanner->line);
                    token.data.String = str_val;
                    return token;
                } else if (symb == '\n' || symb == EOF) {
                   if (symb == '\n'){
                    handle_error(LEXICAL_ERROR, scanner->line, "Unexpected new-line character in string literal");
                   } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Unexpected end-of-file in string literal");
                   }
                    
                } else if (symb > 31 && symb != '"') {
                    append_to_buffer(&scanner->buffer, symb);
                } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Unsupported character value in string literal");
                }
                break;
            }

            case STRING_ESCAPE: {
                switch (symb) {
                    case '"': 
                        append_to_buffer(&scanner->buffer, '"');
                        scanner->state = STRING;
                        break;
                    case 'n': 
                        append_to_buffer(&scanner->buffer, '\n');
                        scanner->state = STRING;
                        break;
                    case 'r':
                        append_to_buffer(&scanner->buffer, '\r');
                        scanner->state = STRING;
                        break;
                    case 't': 
                        append_to_buffer(&scanner->buffer, '\t');
                        scanner->state = STRING;
                        break;
                    case '\\': 
                        append_to_buffer(&scanner->buffer, '\\');
                        scanner->state = STRING;
                        break;
                    case 'u':
                        scanner->state = STRING_ESCAPE_U;
                        break;
                    default:
                        handle_error(LEXICAL_ERROR, scanner->line, "Unsupported escape sequence in string literal");
                }
                break;
            }

            case STRING_ESCAPE_U: {
                if (symb == '{') {
                    unicode_val = 0;
                    hex_digits_count = 0;
                    scanner->state = STRING_ESCAPE_U_VALUE;
                } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Expected '{' after \\u in string literal");
                }
                break;
            }

            case STRING_ESCAPE_U_VALUE: {
                if (isxdigit(symb)) {
                    hex_digits_count++;
                    if (hex_digits_count > 8) {
                        handle_error(LEXICAL_ERROR, scanner->line, "Too many digits in \\u{dd} escape sequence");
                    }
                    unicode_val = unicode_val * 16 + (isdigit(symb) ? (symb - '0') : (toupper(symb) - 'A' + 10));
                } else if (symb == '}') {
                    if (unicode_val > 255) {
                        handle_error(LEXICAL_ERROR, scanner->line, "Value out of range in \\u{dd} escape sequence");
                    }
                    char actual_char = (char)unicode_val;
                    append_to_buffer(&scanner->buffer, actual_char);
                    scanner->state = STRING; // Succesful \u{dd} parse
                } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Expected '}' or hex digit in \\u{dd} escape sequence");
                }
                break;
            }
            
            case DIGIT: {
                if (isdigit(symb)) {
                    append_to_buffer(&scanner->buffer, symb);
                } else if (symb == '.') {
                    append_to_buffer(&scanner->buffer, symb);
                    scanner->state = DECIMAL;
                } else if (symb == 'e' || symb == 'E') {
                    append_to_buffer(&scanner->buffer, symb);
                    scanner->state = EXP;
                } else {
                    ungetc(symb, scanner->input);

                    char *num_str = buffer_to_string(&scanner->buffer);
                    unsigned long long int_val = strtoull(num_str, NULL, 10);
                    free(num_str);
                    free_buffer(&scanner->buffer);
                    token = create_token(TK_INT, scanner->line);
                    token.data.Int = int_val;
                    return token;
                }
                break;
            }

            case DECIMAL: {
                if (isdigit(symb)) {
                    append_to_buffer(&scanner->buffer, symb);
                } else if (symb == 'e' || symb == 'E') {
                    append_to_buffer(&scanner->buffer, symb);
                    scanner->state = EXP;
                } else {
                    ungetc(symb, scanner->input); 
                    char *num_str = buffer_to_string(&scanner->buffer);
                    double double_val = strtod(num_str, NULL);
                    free(num_str);
                    free_buffer(&scanner->buffer);
                    token = create_token(TK_DOUBLE, scanner->line);
                    token.data.Double = double_val;
                    return token;
                }
                break;
            }

            case EXP: {
                if (symb == '+' || symb == '-') {
                    append_to_buffer(&scanner->buffer, symb);
                    scanner->state = EXP_SIGN;
                } else if (isdigit(symb)) {
                    append_to_buffer(&scanner->buffer, symb);
                    scanner->state = EXP_NUMBER;
                } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Unexpected character in double literal exponent");
                }
                break;
            }

            case EXP_SIGN: {
                if (isdigit(symb)) {
                    append_to_buffer(&scanner->buffer, symb);
                    scanner->state = EXP_NUMBER;
                } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Unexpected character after double literal exponent");
                }
                break;
            }

            case EXP_NUMBER: {
                if (isdigit(symb)) {
                    append_to_buffer(&scanner->buffer, symb);
                } else {
                    ungetc(symb, scanner->input);
                    char *num_str = buffer_to_string(&scanner->buffer);
                    double double_val = strtod(num_str, NULL);
                    free(num_str);
                    free_buffer(&scanner->buffer);
                    token = create_token(TK_DOUBLE, scanner->line);
                    token.data.Double = double_val;
                    return token;
                }
                break;
            }

            case IDENTIFIER: {
                if (isalpha(symb) || isdigit(symb) || symb == '_') {
                    append_to_buffer(&scanner->buffer, symb);
                } else if (symb == '?') {
                    append_to_buffer(&scanner->buffer, symb);
                    scanner->state = IDENTIFIER_TYPE;
                } else {
                    char *identifier = buffer_to_string(&scanner->buffer);

                    if (strcmp(identifier, "_") == 0) {
                        free(identifier); 
                        free_buffer(&scanner->buffer);
                        return create_token(TK_UNDERSCORE, scanner->line);
                    }

                    token_t token = get_identifier(identifier, scanner->line);
                    
                    if (token.type != TK_IDENTIFIER) {
                        free(identifier); // If its a keyword, free the string mem
                    }

                    free_buffer(&scanner->buffer);
                    
                    ungetc(symb, scanner->input);  // will iterate once more before returning, stealing next token's symbol
                    return token;
                }
                break;
            }

            case IDENTIFIER_TYPE: {
                ungetc(symb, scanner->input);  // Already got ?, current symbol is from different token
                char* identifier_str = buffer_to_string(&scanner->buffer);
                if (strcmp(identifier_str, "Double?") == 0) {
                    token_t token = create_token(TK_KW_DOUBLE_OPT, scanner->line);
                } else if (strcmp(identifier_str, "Int?") == 0) {
                    token_t token = create_token(TK_KW_INT_OPT, scanner->line);
                } else if (strcmp(identifier_str, "String?") == 0) {
                    token_t token = create_token(TK_KW_STRING_OPT, scanner->line);
                } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Unknown optional type, did you mean: Double? / String? / Int?");
                }

                free(identifier_str);
                free_buffer(&scanner->buffer);
            
                return token;
            }

            case DIV: {
                if (symb == '/') {  
                    scanner->state = OLCOMMENT;
                } else if (symb == '*') { 
                    scanner->state = BLCOMMENT;
                    nested_comment_count = 1;
                } else {                        // Just a division operator '/'
                    ungetc(symb, scanner->input);  
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
                    ungetc(symb, scanner->input);  // Revert the EOF character (check if needed?)
                    scanner->state = START;
                }
                break;
            }

            case BLCOMMENT: {
                if (symb == '*') {
                    scanner->state = BLCOMMENT_E;
                } else if (symb == '/') {
                    char peek_symb = fgetc(scanner->input);
                    if (peek_symb == '*') {
                        nested_comment_count++; 
                    } else {
                        ungetc(peek_symb, scanner->input);
                    }
                } else if (symb == EOF) {
                    handle_error(LEXICAL_ERROR, scanner->line, "End-of-File found inside block comment");
                } else if (symb == '\n') {
                    scanner->line++;
                }
                break;
            }

            case BLCOMMENT_E: {
                if (symb == '/') {
                    nested_comment_count--;  // One less nested comment
                    if (nested_comment_count == 0) {  // All nested comments are closed
                        scanner->state = START;
                    } else {  
                        scanner->state = BLCOMMENT;
                    }
                } else if (symb == '*') {  
                    // If we get another *, stay in BLCOMMENT_E and wait for '/'
                } else {
                    scanner->state = BLCOMMENT;  
                }
                break;
            }
            
            case MINUS: {
                
                if (symb == '>') {
                    return create_token(TK_ARROW, scanner->line);  // Arrow ->
                } else {
                    ungetc(symb, scanner->input);
                    return create_token(TK_MINUS, scanner->line);  // Just minus -
                }
                break;
            }

            case LT: {
                if (symb == '=') {  
                    return create_token(TK_LE, scanner->line);  // Less or equal <=
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_LT, scanner->line);  // Just less than <
                }
            }

            case GT: {
                if (symb == '=') {  
                    return create_token(TK_GE, scanner->line);  // Greater or equal >=
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_GT, scanner->line);  // Just greater than >
                }
            }

            case ASSIGN: {
                if (symb == '=') {  
                    return create_token(TK_EQ, scanner->line);  // Equal == 
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_ASSIGN, scanner->line);  // Just assignment =
                }
            }

            case UNWRAP: {
                if (symb == '=') {  
                    return create_token(TK_NEQ, scanner->line);  // Not equal != 
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_UNWRAP, scanner->line); // Just unwrap !
                }
            }

            case COALESCE: {
                if (symb == '?') {  
                    return create_token(TK_COALESCE, scanner->line);
                } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Unexpected character after '?', expected '?'?'");
                }
            }
        }
    }
}

void init_buffer(buffer_t *buffer, size_t initial_capacity) {
    buffer->data = (char *)malloc(initial_capacity);
    if (!buffer->data) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "Memory allocation error!");
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
            handle_error(INTERNAL_COMPILER_ERROR, 0, "Memory allocation error!");
        }
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }
    buffer->data[buffer->size++] = ch;
}

char* buffer_to_string(buffer_t *buffer) {
    char *result = (char *)malloc(buffer->size + 1);  // +1 for the null terminator
    if (!result) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "Memory allocation error!");
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