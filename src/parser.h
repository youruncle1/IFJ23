/*
IFJ PROJEKT 2023/2024
file: "parser.h"

Top-down recursive parser - header file

authors: xpolia05

*/

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "error.h"
#include "scanner.h"  
#include "symtable.h"
#include "generator.h"

/**
 * @brief Structure representing a parser.
 *
 * @param scanner Pointer to the scanner used by the parser.
 * @param current_token The current token being processed.
 * @param local_frame Stack of symbol tables for local scope management.
 * @param global_frame Pointer to the symbol table for global scope.
 * @param current_func Pointer to the symbol table node for the current function.
 * @param current_func_call Pointer to the symbol table node for the current function call.
 * @param hasReturn Flag indicating if the current function has a return statement.
 * @param inFunction Flag indicating if the parser is currently inside a function.
 * @param isReturn Flag indicating if the current statement is a return statement.
 * @param scopeDepth Depth of the current scope.
 * @param TKAIndex Index used for some internal management.
 * @param parsedParamCount Count of parameters parsed in the current function.
 */
typedef struct {
    scanner_t *scanner;
    token_t current_token;
    SymbolTableStack *local_frame;
    Node* global_frame;
    Node* current_func;
    Node* current_func_call;
    bool hasReturn;
    bool inFunction;
    bool isReturn;
    unsigned int scopeDepth;
    unsigned int TKAIndex;
    unsigned int parsedParamCount;
    
} parser_t;

/**
 * @brief Structure representing an array of tokens.
 *
 * @param tokens Pointer to an array of tokens.
 * @param size Current number of tokens in the array.
 * @param capacity Total allocated capacity of the array.
 */
typedef struct {
    token_t *tokens;
    size_t size;
    size_t capacity;
} TokenArray;

/**
 * @brief Initializes a new parser.
 *
 * @param scanner Pointer to the scanner to be used by the parser.
 * @return An initialized parser_t structure.
 */
parser_t initParser(scanner_t *scanner);

/**
 * @brief Initializes a new TokenArray.
 *
 * @return Pointer to the newly created TokenArray.
 */
TokenArray *initTokenArray();

/**
 * @brief Retrieves the next token and updates the current token in the parser.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray to store the token.
 */
void parser_get_next_token(parser_t *parser, TokenArray *tokenArray);

/**
 * @brief Retrieves the previous token and updates the current token in the parser.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray to store the token.
 */
void parser_get_previous_token(parser_t *parser, TokenArray *tokenArray);

/**
 * @brief Looks ahead at the next token without changing the current token in the parser.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @return The next token.
 */
token_t token_lookahead(parser_t *parser, TokenArray *tokenArray);

/**
 * @brief Looks back at the previous token without changing the current token in the parser.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @return The previous token.
 */
token_t token_lookback(parser_t *parser, TokenArray *tokenArray);

/**
 * @brief Checks if the next token matches the expected type and exits the program if not.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param expectedType The expected token type.
 */
void check_next_token(parser_t *parser, TokenArray *tokenArray, tk_type_t expectedType);

/**
 * @brief Checks if the current token matches the expected type.
 *
 * @param parser Pointer to the parser.
 * @param expectedType The expected token type.
 * @return True if the current token type matches, false otherwise.
 */
bool check_token_type(parser_t *parser, tk_type_t expectedType);

/**
 * @brief Determines if a token type represents a datatype.
 *
 * @param token Token type to check.
 * @return True if the token is a datatype, false otherwise.
 */
bool is_token_datatype(tk_type_t token);

/**
 * @brief Determines if a token type represents a literal.
 *
 * @param token Token type to check.
 * @return True if the token is a literal, false otherwise.
 */
bool is_token_literal(tk_type_t token);


/**
 * @brief Determines if a token type is part of an expression.
 *
 * @param tokenType Token type to check.
 * @return True if the token is part of an expression, false otherwise.
 */
bool isPartOfExpression(tk_type_t tokenType);

/**
 * @brief Determines if a token type signifies the start of an expression.
 *
 * @param tokenType Token type to check.
 * @return True if the token is the start of an expression, false otherwise.
 */
bool isStartOfExpression(tk_type_t tokenType);

/**
 * @brief Finds the data type of a variable in the symbol tables.
 *
 * @param parser Pointer to the parser.
 * @return The data type of the found variable.
 */
tk_type_t find_varType(parser_t *parser);

/**
 * @brief Parses the entire program based on the recursive grammar rule.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param gen Pointer to the generator.
 */
void parseProgram(parser_t *parser, TokenArray *tokenArray, generator_t* gen);

/**
 * @brief Checks the type of a variable and ensures it matches the expected type.
 *        Throws an error if the type is incompatible.
 *
 * @param parser Pointer to the parser.
 * @param token Token representing the variable.
 * @param type Expected data type.
 * @param hasType Flag indicating if the variable has a specified type.
 */
void check_VarType(parser_t *parser, token_t token, tk_type_t type, bool hasType);

/**
 * @brief Parses blockcontents grammar rule.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param gen Pointer to the generator.
 */
void parseBlockContents(parser_t* parser, TokenArray *tokenArray, generator_t* gen);

/**
 * @brief Parses a block content grammar rule.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param gen Pointer to the generator.
 */
void parseBlockContent(parser_t *parser, TokenArray *tokenArray, generator_t* gen);

/**
 * @brief Parses a function definition.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param gen Pointer to the generator.
 */
void parseFunctionDefinition(parser_t *parser, TokenArray *tokenArray, generator_t* gen);

/**
 * @brief Sets up the local symtable scope when parsing function.
 *
 * @param parser Pointer to the parser.
 * @param functionName Name of the function.
 */
void setupFunctionScope(parser_t *parser, const char *functionName);

/**
 * @brief Parses a variable definition.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param gen Pointer to the generator.
 */
void parseVarDefinition(parser_t *parser, TokenArray *tokenArray, generator_t* gen);

/**
 * @brief Inserts a variable into the appropriate symbol table.
 *
 * @param parser Pointer to the parser.
 * @param tmpToken Token representing the variable.
 * @param isLet Flag indicating if the variable is a 'let' variable.
 */
void parser_insertVar2symtable(parser_t *parser, token_t tmpToken, bool isLet);

/**
 * @brief Updates the initialization flag (isInit) in the symbol table for a variable.
 *
 * @param parser Pointer to the parser.
 * @param token Token representing the variable to update.
 */
void var_updateInit(parser_t *parser, token_t token);

/**
 * @brief Parses a control structure (if, while).
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param gen Pointer to the generator.
 */
void parseControlStructure(parser_t *parser, TokenArray *tokenArray, generator_t* gen);

/**
 * @brief Parses an assignment.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param gen Pointer to the generator.
 */
void parseAssignment(parser_t *parser, TokenArray *tokenArray, generator_t* gen);

/**
 * @brief Parses a function call.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param gen Pointer to the generator.
 */
void parseFunctionCall(parser_t *parser, TokenArray *tokenArray, generator_t* gen);

/**
 * @brief Parses the parameters in a function call.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param parsedParameters Pointer to an array of parsed parameters.
 * @param gen Pointer to the generator.
 */
void parseFunctionCallParams(parser_t *parser, TokenArray *tokenArray, Parameter **parsedParameters, generator_t* gen);

/**
 * @brief Parses a single parameter in a function call.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param parsedParameters Pointer to an array of parsed parameters.
 * @param gen Pointer to the generator.
 */
void parseCallParameter(parser_t *parser, TokenArray *tokenArray, Parameter **parsedParameters, generator_t* gen);

/**
 * @brief Adds a token to the TokenArray.
 *
 * @param array Pointer to the TokenArray.
 * @param token The token to add.
 */
void addToken(TokenArray *array, token_t token);

/**
 * @brief Starts the first pass of the two-pass parser.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 */
void firstParserPass(parser_t *parser, TokenArray *tokenArray);

/**
 * @brief Parses a return statement.
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 * @param gen Pointer to the generator.
 */
void parseReturn(parser_t *parser, TokenArray *TokenArray, generator_t* gen);

/**
 * @brief Collects function definitions and adds them to the global symbol table 
 * This is a first pass function
 *
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 */
void CollectFunctionDefinition(parser_t *parser, TokenArray *tokenArray);

/**
 * @brief Parses function parameters during the first pass of function definition.
 * This is a first pass function
 * 
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 */
void parseFunctionParameters(parser_t *parser, TokenArray *tokenArray);

/**
 * @brief Parses a single parameter during the first pass of function definition.
 * This is a first pass function
 * 
 * @param parser Pointer to the parser.
 * @param tokenArray Pointer to the TokenArray.
 */
void parseParameter(parser_t *parser, TokenArray *tokenArray);

/**
 * @brief Checks the next token, adds it to the TokenArray, and handles errors if it's not of the expected type.
 * This is a first pass function
 *
 * @param tokenArray Pointer to the TokenArray.
 * @param parser Pointer to the parser.
 * @param expectedType The expected type of the next token.
 */
void check_next_token_and_add(TokenArray *tokenArray, parser_t *parser, tk_type_t expectedType);

/**
 * @brief Retrieves the next token from the scanner and saves it to the current token in the parser.
 * This is a first pass function
 * 
 * @param parser Pointer to the parser.
 */
void get_next_token(parser_t *parser);

/**
 * @brief Searches for a variable in the local symbol table stack and then in the global table. 
 *        Throws an error if the variable is undefined.
 *
 * @param parser Pointer to the parser.
 * @return Pointer to the Node containing the variable, or NULL if not found.
 */
Node *searchFramesVar(parser_t *parser);

/**
 * @brief Converts a literal token type to its corresponding data type.
 *
 * @param tokenType The token type representing a literal.
 * @return The data type corresponding to the literal token type.
 */
tk_type_t convert_literal_to_datatype(tk_type_t tokenType);


#endif