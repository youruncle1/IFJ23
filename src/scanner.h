/*
IFJ PROJEKT 2023/2024
file: "scanner.h"

Lexical analyser - header file

authors: xpolia05

*/

#ifndef SCANNER_H
#define SCANNER_H

#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @brief All possible states of the scanner
 * 
 */
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

/**
 * @brief Token types used throughout the program, some are used exclusively in expression parser
 * 
 */
typedef enum {

    TK_UNWRAP,        // !  force unwraps a value, asserting that it is NOT 'nil'(see ^ and assignment: 5.1 Aritmeticke, retezcove...)
    TK_MUL,           // *
    TK_DIV,           // /
    TK_PLUS,          // +
    TK_MINUS,         // -
    TK_LT,            // <
    TK_LE,            // <=
    TK_GT,            // >
    TK_GE,            // >=
    TK_EQ,            // ==
    TK_NEQ,           // !=
    TK_COALESCE,      // ?? https://docs.swift.org/swift-book/documentation/the-swift-programming-language/basicoperators/#Nil-Coalescing-Operator
    TK_LPAR,          // (
    TK_RPAR,          // )

    /* Literals */
    TK_IDENTIFIER,    // Identifier
    TK_DOUBLE,        // double literal
    TK_INT,           // integer literal
    TK_STRING,        // string literal
    TK_MLSTRING,      // multi-line string literal

    TK_DOLLAR,        // $ For expression 
    TK_BOOLEAN,       // Boolean

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

    
    
    TK_ASSIGN,        // =
    /* Punctation */
    TK_COMMA,         // ,
    TK_COLON,         // :
    TK_SEMICOLON,     // ;
    TK_LBRACE,        // {
    TK_RBRACE,        // }
    TK_ARROW,         // ->
    TK_UNDERSCORE,    // _

    /* Expression*/
    TK_LEFT,          // <
    TK_RIGHT,         // >

    TK_EOF,           // End of file

    
} tk_type_t;


/**
 * @brief Structure representing a token.
 *
 * @param data Union holding the data of the token.
 * @param type Type of the token as defined by tk_type_t.
 * @param line Line number where the token appears in the source code.
 * @param eol_before Flag indicating if there was an end-of-line before this token.
 */
typedef struct {
    union { 
        // https://learn.microsoft.com/cs-cz/cpp/cpp/data-type-ranges?view=msvc-170
        double Double;          //double literals
        unsigned long long Int; //integer literals
        char* String;           //identifiers, string literals, keywords
    } data;
    tk_type_t type;
    unsigned int line;
    bool eol_before;
} token_t;

/**
 * @brief Structure representing the state of a scanner.
 *
 * @param input Pointer to the input file.
 * @param state Current state of the finite state machine (FSM).
 * @param line Current line number being processed.
 * @param buffer Buffer used for storing temporary data.
 */
typedef struct {
    FILE *input;
    fsm_state_t state;
    unsigned int line;
    buffer_t buffer;
} scanner_t;

/**
 * @brief Checks if token is a keyword or identifier
 *
 * @param identifier Found identifier in scanner.
 * @param line Line number where the identifier appears so new token can be created with this line.
 * @return A token representing the identifier/keyword.
 */
token_t get_identifier(char *identifier, unsigned int line);

/**
 * @brief Creates a generic token.
 *
 * @param type Type of the token as defined by tk_type_t.
 * @param line Line number where the token appears.
 * @param eol_before Flag indicating if there was an end-of-line before this token.
 * @return A newly created token.
 */
token_t create_token(tk_type_t type, unsigned int line, bool eol_before);

/**
 * @brief Retrieves the next token from the scanner.
 *
 * @param scanner Pointer to the scanner structure.
 * @return The next token extracted by the scanner.
 */
token_t get_token(scanner_t *scanner);

#endif