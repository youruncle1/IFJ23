#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "error.h"
#include "scanner.h"  
#include "symtable.h" 

typedef struct {
    scanner_t *scanner;
    token_t current_token;
    SymbolTableStack *local_frame;
    Node* global_frame;
    Node* current_func;
    Node* current_func_call;
    unsigned int TKAIndex;
} parser_t;

typedef struct {
    token_t *tokens;
    size_t size;
    size_t capacity;
} TokenArray;

parser_t initParser(scanner_t *scanner);
TokenArray *initTokenArray();

void addToken(TokenArray *array, token_t token);
void firstParserPass(parser_t *parser, TokenArray *tokenArray);
void parseFunctionDefinition(parser_t *parser, TokenArray *tokenArray);
void parseFunctionParameters(parser_t *parser, TokenArray *tokenArray);
void parseParameter(parser_t *parser, TokenArray *tokenArray);
void check_next_token_and_add(TokenArray *tokenArray, parser_t *parser, tk_type_t expectedType);
void get_next_token(parser_t *parser);
bool check_token_type(parser_t *parser, tk_type_t expectedType);
bool is_token_datatype(tk_type_t token);

#endif
