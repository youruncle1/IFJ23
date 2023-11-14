/*
IFJ PROJEKT 2023/2024
file: "src/scanner.h"

Lexical analysis - header file

authors: xpolia05
         xbencs00
         xrusna08
         xhonze01
*/

#ifndef SCANNER_H
#define SCANNER_H

#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum {
    /* Start state */
    START,

    /* Comment states */
    OLCOMMENT,
    BLCOMMENT,
    BLCOMMENT_E,

    /* Arithmetic and other operator states */
    DIV,
    MINUS,
    LT,
    GT,
    ASSIGN,
    UNWRAP,

    /* Literal states */
    DIGIT,
    DECIMAL,
    EXP,
    EXP_SIGN,
    EXP_NUMBER,
    IDENTIFIER,
    IDENTIFIER_TYPE,

    /* String states */
    STRING_TYPE,
    MLSTRING,
    STRING,
    STRING_ESCAPE,
    STRING_ESCAPE_U,
    STRING_ESCAPE_U_VALUE,

    /* Special character states */
    COALESCE,
} fsm_state_t;


typedef enum {
    /* Keywords */
    TK_KW_DOUBLE,
    TK_KW_DOUBLE_OPT,
    TK_KW_ELSE,
    TK_KW_FUNC,
    TK_KW_IF,
    TK_KW_INT,
    TK_KW_INT_OPT,
    TK_KW_LET,
    TK_KW_NIL,
    TK_KW_RETURN,
    TK_KW_STRING,
    TK_KW_STRING_OPT,
    TK_KW_VAR,
    TK_KW_WHILE,

    /* Literals */
    TK_IDENTIFIER,    // Identifier
    TK_DOUBLE,        // double literal
    TK_INT,           // integer literal
    TK_STRING,        // string literal
    TK_MLSTRING,      // multi-line string literal
    
    /* Operators */
    TK_PLUS,          // +
    TK_MINUS,         // -
    TK_MUL,           // *
    TK_DIV,           // /
    TK_COALESCE,      // ?? https://docs.swift.org/swift-book/documentation/the-swift-programming-language/basicoperators/#Nil-Coalescing-Operator
    TK_UNWRAP,        // !  force unwraps a value, asserting that it is NOT 'nil'(see ^ and assignment: 5.1 Aritmeticke, retezcove...)
    TK_ASSIGN,        // =
    TK_EQ,            // ==
    TK_NEQ,           // !=
    TK_LT,            // <
    TK_GT,            // >
    TK_LE,            // <=
    TK_GE,            // >=
    /* Punctation */
    TK_LPAR,          // (
    TK_RPAR,          // )
    TK_COMMA,         // ,
    TK_COLON,         // :
    TK_SEMICOLON,     // ;
    TK_LBRACE,        // {
    TK_RBRACE,        // }
    TK_ARROW,         // ->
    TK_UNDERSCORE,    // _
    
    TK_EOF,           // End of file
    
} tk_type_t;

typedef struct {
    union { 
        // https://learn.microsoft.com/cs-cz/cpp/cpp/data-type-ranges?view=msvc-170
        double Double;          //double literals
        unsigned long long Int; //integer literals
        char* String;           //identifiers, string literals, keywords, ?operators?
    } data;
    tk_type_t type;
    unsigned int line;
    bool eol_before;
} token_t;

typedef struct {
    FILE *input;
    fsm_state_t state;
    unsigned int line;
    buffer_t buffer;
} scanner_t;

token_t get_identifier(char *identifier, unsigned int line);
token_t create_token(tk_type_t type, unsigned int line, bool eol_before);
token_t get_token(scanner_t *scanner);

#endif