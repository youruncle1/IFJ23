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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
    /* NFS - NOT finish state */
    
    /* Start state */
    START,

    /* Comment states */
    OLCOMMENT,        // //
    OLCOMMENT_E,      // end of one-line comment (EOF)
    BLCOMMENT,        // /*
    BLCOMMENT_E1,     // NFS: multi-line comment end 1 (got *)
    MLCOMMENT_E2,     // FS: end of multi-line comment (got */)

    /* Arithmetic and other operator states */
    DIV,              // /
    PLUS,             // +
    MUL,              // *  
    MINUS,            // -
    ARROW,            // ->
    LT,               // <
    LE,               // <=
    GT,               // >
    GE,               // >=
    ASSIGN,           // =
    EQ,               // ==
    UNWRAP,           // !
    NEQ,              // !=
    /* Punctuation states */
    LPAR,             // (
    RPAR,             // )
    LBRACE,           // {
    RBRACE,           // }
    COMMA,            // ,
    COLON,            // :
    SEMICOLON,        // ;

    /* Literal states */
    DIGIT,
    
    DECIMAL,          // NFS: got 0.??
           // NFS: got 0.1e/E?? or 2e/E??        !! exponent can be only integer
        // NFS: got 0.1e/E+/-?? or 2e/E+/-??
    EXP,              // NFS: 0.1E? or 2E?
    EXP_SIGN,          // FS: 0.1E-2 or 2E+4  
    EXP_NUMBER,
    IDENTIFIER,     
    IDENTIFIER_TYPE,  // double?, string?, int?

    /* String states TODO Int?*/
    DETECT_STRING_TYPE,
    STRING_S,         // "
    STRING_E,         // ""
    STRING_ESCAPE,    // got '\' in string literal
    STRING_ESCAPE_U,
    STRING_ESCAPE_U_VALUE,
    STRING_MULTI,      // ""
    MLSTRING_S,      // """\n
    MLSTRING_E1,      // """
    MLSTRING_E2,      // ""
    MLSTRING_E3,      // "\n

    /* Special character states */
    COALESCE,         // ??
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
    TK_IDENTIFIER_OPT,// Type identifier with optional "?" (double?, string?, int?)
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
} token_t;

typedef struct {
    char *data;         
    size_t size;       
    size_t capacity;
} buffer_t;

typedef struct {
    FILE *input;
    fsm_state_t state;
    unsigned int line;
    buffer_t buffer;
} scanner_t;

token_t get_identifier(char *identifier, unsigned int line);
token_t create_token(tk_type_t type, unsigned int line);
token_t get_token(scanner_t *scanner);

void init_buffer(buffer_t *buffer, size_t initial_capacity);
void append_to_buffer(buffer_t *buffer, char ch);
char* buffer_to_string(buffer_t *buffer);
void free_buffer(buffer_t *buffer);


#endif