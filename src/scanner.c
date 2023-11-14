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

    1. watch out for memleaks when error exiting in string literals
    2. comment the code (?doxygen?)

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

token_t create_token(tk_type_t type, unsigned int line, bool eol_before) {
    return (token_t){.type = type, .line = line, .eol_before = eol_before};
}

token_t get_token(scanner_t *scanner) {
    scanner->state = START;
    token_t token;
    
    char symb;
    int nested_comment_count = 0;
    int whtespce_count = 0;
    int closing_delimiter_indentation = 0;
    int unicode_val, hex_digits_count;
    bool mlstring = false;
    bool eol_before = false;

    // FSM
    while (1) {
        
        symb = fgetc(scanner->input);
        
        switch (scanner->state) {
            
            case START: {
                if (isspace(symb)) {
                    if (symb == '\n') {
                        scanner->line++;
                        eol_before = true;
                    }
                    continue;
                }

                switch(symb) {
                    /* SINGULAR SYMBOL LEXEMS */
                    case EOF:
                        return create_token(TK_EOF, scanner->line, eol_before);
                    case '(':
                        return create_token(TK_LPAR, scanner->line, eol_before);
                    case ')':
                        return create_token(TK_RPAR, scanner->line, eol_before);
                    case '{':
                        return create_token(TK_LBRACE, scanner->line, eol_before);
                    case '}':
                        return create_token(TK_RBRACE, scanner->line, eol_before);
                    case ':':
                        return create_token(TK_COLON, scanner->line, eol_before);
                    case ';':
                        return create_token(TK_SEMICOLON, scanner->line, eol_before);
                    case ',':
                        return create_token(TK_COMMA, scanner->line, eol_before);
                    case '+':
                        return create_token(TK_PLUS, scanner->line, eol_before);
                    case '*':
                        return create_token(TK_MUL, scanner->line, eol_before);
                    
                    /* MULTI SYMBOL LEXEMS */
                    case '/':
                        scanner->state = DIV;
                        continue;
                    case '"':
                        init_buffer(&scanner->buffer, 5);
                        scanner->state = STRING_TYPE;
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

            case STRING_TYPE: {
                if (symb == '"') { // second quote
                    char next_symb = fgetc(scanner->input);
                    if (next_symb == '"') { 
                        while ((symb = fgetc(scanner->input)) == ' ');
                        if (symb == '\n') {
                            scanner->line++;
                            mlstring = true;
                            scanner->state = MLSTRING;
                        } else {
                            handle_error(LEXICAL_ERROR, scanner->line, "unexpected character after start of multiline string literal");
                        }
                    } else {
                        
                        ungetc(next_symb, scanner->input); 
                        char* str_val = buffer_to_string(&scanner->buffer); 
                        free_buffer(&scanner->buffer);
                        token = create_token(TK_STRING, scanner->line, eol_before);
                        token.data.String = str_val;
                        return token;
                    }
                } else {
                    
                    ungetc(symb, scanner->input); 
                    scanner->state = STRING;
                }
                break;
            }

            case MLSTRING: {
                if (symb == '"') {
                    char next_symb = fgetc(scanner->input);
                    if (next_symb == '"') {
                        char third_symb = fgetc(scanner->input);
                        if (third_symb == '"') {
                            
                            char* raw_str_val = buffer_to_string(&scanner->buffer);
                            if (strlen(raw_str_val) == 0) { // Empty multiline string
                                
                                free(raw_str_val);
                                free_buffer(&scanner->buffer);

                                token = create_token(TK_STRING, scanner->line, eol_before);
                                token.data.String = strdup(""); 
                                return token;
                            }
                            // Calculate the indentation of the string
                            char* last_newline = strrchr(raw_str_val, '\n');
                            int last_line_start = 0;
                            if (last_newline) {
                                last_line_start = last_newline - raw_str_val + 1;
                            }

                            while (raw_str_val[last_line_start] == ' ') {
                                closing_delimiter_indentation++;
                                last_line_start++;
                            }
            
                            char* str_val = trim_multiline_string_indentation(raw_str_val, closing_delimiter_indentation);
                            free(raw_str_val);

                            free_buffer(&scanner->buffer);
                            token = create_token(TK_STRING, scanner->line, eol_before);
                            token.data.String = str_val;
                            return token;
                        } else {
                            // Only two quotes found
                            append_to_buffer(&scanner->buffer, symb);
                            append_to_buffer(&scanner->buffer, next_symb);
                            ungetc(third_symb, scanner->input);
                        }
                    } else {
                        // Only one quote found
                        append_to_buffer(&scanner->buffer, symb);
                        ungetc(next_symb, scanner->input);
                    }
                } else if (symb == '\\') {
                    scanner->state = STRING_ESCAPE;
                } else if (symb == EOF) {
                    handle_error(LEXICAL_ERROR, scanner->line, "Unexpected end-of-file in multiline string literal");
                } else if (symb == '\n') {
                    scanner->line++;
                    append_to_buffer(&scanner->buffer, symb);
                } else {
                    append_to_buffer(&scanner->buffer, symb);
                }
                break;
            }

            case STRING: {
                if (symb == '\\') { 
                    scanner->state = STRING_ESCAPE;
                } else if (symb == '"') {
                    char* str_val = buffer_to_string(&scanner->buffer);
                    free_buffer(&scanner->buffer);
                    token = create_token(TK_STRING, scanner->line, eol_before);
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
                        scanner->state = mlstring ? MLSTRING : STRING;
                        break;
                    case 'n': 
                        append_to_buffer(&scanner->buffer, '\n');
                        scanner->state = mlstring ? MLSTRING : STRING;
                        break;
                    case 'r':
                        append_to_buffer(&scanner->buffer, '\r');
                        scanner->state = mlstring ? MLSTRING : STRING;
                        break;
                    case 't': 
                        append_to_buffer(&scanner->buffer, '\t');
                        scanner->state = mlstring ? MLSTRING : STRING;
                        break;
                    case '\\': 
                        append_to_buffer(&scanner->buffer, '\\');
                        scanner->state = mlstring ? MLSTRING : STRING;
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
                    scanner->state = mlstring ? MLSTRING : STRING; // Succesful \u{dd} parse
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
                    token = create_token(TK_INT, scanner->line, eol_before);
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
                    token = create_token(TK_DOUBLE, scanner->line, eol_before);
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
                    token = create_token(TK_DOUBLE, scanner->line, eol_before);
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
                        return create_token(TK_UNDERSCORE, scanner->line, eol_before);
                    }

                    token_t token = get_identifier(identifier, scanner->line);
                    
                    if (token.type != TK_IDENTIFIER) {
                        free(identifier); // If its a keyword, free the string mem
                    }

                    free_buffer(&scanner->buffer);
                    
                    ungetc(symb, scanner->input);  // will iterate once more before returning, stealing next token's symbol
                    token.eol_before = eol_before;
                    return token;
                }
                break;
            }

            case IDENTIFIER_TYPE: {
                ungetc(symb, scanner->input);  // Already got ?, current symbol is from different token
                char* identifier_str = buffer_to_string(&scanner->buffer);
                if (strcmp(identifier_str, "Double?") == 0) {
                    token_t token = create_token(TK_KW_DOUBLE_OPT, scanner->line, eol_before);
                } else if (strcmp(identifier_str, "Int?") == 0) {
                    token_t token = create_token(TK_KW_INT_OPT, scanner->line, eol_before);
                } else if (strcmp(identifier_str, "String?") == 0) {
                    token_t token = create_token(TK_KW_STRING_OPT, scanner->line, eol_before);
                } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Unknown optional type, did you mean: Double? / String? / Int?");
                }

                free(identifier_str);
                free_buffer(&scanner->buffer);

                token.eol_before = eol_before;
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
                    return create_token(TK_DIV, scanner->line, eol_before);
                }
                break;
            }

            case OLCOMMENT: {
                do {
                    symb = fgetc(scanner->input);
                    if (symb == '\n') {
                        scanner->line++;
                        scanner->state = START;
                        eol_before = true;
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
                    return create_token(TK_ARROW, scanner->line, eol_before);  // Arrow ->
                } else {
                    ungetc(symb, scanner->input);
                    return create_token(TK_MINUS, scanner->line, eol_before);  // Just minus -
                }
                break;
            }

            case LT: {
                if (symb == '=') {  
                    return create_token(TK_LE, scanner->line, eol_before);  // Less or equal <=
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_LT, scanner->line, eol_before);  // Just less than <
                }
            }

            case GT: {
                if (symb == '=') {  
                    return create_token(TK_GE, scanner->line, eol_before);  // Greater or equal >=
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_GT, scanner->line, eol_before);  // Just greater than >
                }
            }

            case ASSIGN: {
                if (symb == '=') {  
                    return create_token(TK_EQ, scanner->line, eol_before);  // Equal == 
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_ASSIGN, scanner->line, eol_before);  // Just assignment =
                }
            }

            case UNWRAP: {
                if (symb == '=') {  
                    return create_token(TK_NEQ, scanner->line, eol_before);  // Not equal != 
                } else {
                    ungetc(symb, scanner->input);  
                    return create_token(TK_UNWRAP, scanner->line, eol_before); // Just unwrap !
                }
            }

            case COALESCE: {
                if (symb == '?') {  
                    return create_token(TK_COALESCE, scanner->line, eol_before);
                } else {
                    handle_error(LEXICAL_ERROR, scanner->line, "Unexpected character after '?', expected '?'?'");
                }
            }
        }
    }
}