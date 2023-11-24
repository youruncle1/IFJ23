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
    bool hasReturn;
    bool inFunction;
    unsigned int scopeDepth;
    unsigned int TKAIndex;
    unsigned int parsedParamCount;
    
} parser_t;

typedef struct {
    token_t *tokens;
    size_t size;
    size_t capacity;
} TokenArray;

parser_t initParser(scanner_t *scanner);
TokenArray *initTokenArray();

parser_t initParser(scanner_t *scanner);
TokenArray *initTokenArray();
void parser_get_next_token(parser_t *parser, TokenArray *tokenArray);
void parser_get_previous_token(parser_t *parser, TokenArray *tokenArray);
token_t token_lookahead(parser_t *parser, TokenArray *tokenArray);
void check_next_token(parser_t *parser, TokenArray *tokenArray, tk_type_t expectedType);
bool check_token_type(parser_t *parser, tk_type_t expectedType);
bool is_token_datatype(tk_type_t token);
bool is_token_literal(tk_type_t token);
bool isPartOfExpression(tk_type_t tokenType);
bool isStartOfExpression(tk_type_t tokenType);
tk_type_t find_varType(parser_t *parser);
void parseProgram(parser_t *parser, TokenArray *tokenArray);
void check_VarType(parser_t *parser, token_t token, tk_type_t type, bool hasType);
void parseBlockContents(parser_t* parser, TokenArray *tokenArray);
void parseBlockContent(parser_t *parser, TokenArray *tokenArray);
void parseFunctionDefinition(parser_t *parser, TokenArray *tokenArray);
void setupFunctionScope(parser_t *parser, const char *functionName);
void parseVarDefinition(parser_t *parser, TokenArray *tokenArray);
void parser_insertVar2symtable(parser_t *parser, token_t tmpToken, bool isLet);
void var_updateInit(parser_t *parser, token_t token);
void parseControlStructure(parser_t *parser, TokenArray *tokenArray);
void parseAssignment(parser_t *parser, TokenArray *tokenArray);
void parseFunctionCall(parser_t *parser, TokenArray *tokenArray);
void parseFunctionCallParams(parser_t *parser, TokenArray *tokenArray, token_t funcToken, Parameter **parsedParameters);
void parseCallParameter(parser_t *parser, TokenArray *tokenArray, token_t funcToken, Parameter **parsedParameters);
void addToken(TokenArray *array, token_t token);
void firstParserPass(parser_t *parser, TokenArray *tokenArray);
void parseReturn(parser_t *parser, TokenArray *TokenArray);
void CollectFunctionDefinition(parser_t *parser, TokenArray *tokenArray);
void parseFunctionParameters(parser_t *parser, TokenArray *tokenArray);
void parseParameter(parser_t *parser, TokenArray *tokenArray);
void check_next_token_and_add(TokenArray *tokenArray, parser_t *parser, tk_type_t expectedType);
void get_next_token(parser_t *parser);
Node *searchFramesVar(parser_t *parser, const char *varName);

// toto by bolo najlepsie nepouzivat a prerobit spravne exprparser...
tk_type_t convert_literal_to_datatype(tk_type_t tokenType);


#endif