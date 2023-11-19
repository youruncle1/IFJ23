/*
IFJ PROJEKT 2023/2024
file: "src/parser.c"

Parser

authors: xpolia05
         xbencs00
         xrusna08
         xhonze01
*/

#include "scanner.h"
#include "error.h"
#include "parser.h"

TokenArray *initTokenArray() {
    TokenArray *array = malloc(sizeof(TokenArray));
    array->size = 0;
    array->capacity = 10; // Initial capacity
    array->tokens = malloc(array->capacity * sizeof(token_t));
    return array;
}

parser_t initParser(scanner_t *scanner) {
    parser_t parser = {.scanner = scanner,
                       .local_frame = initStack(), 
                       .global_frame = NULL,
                       .TKAIndex = 0        
                       };

    return parser;
}

void addToken(TokenArray *array, token_t token) {
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->tokens = realloc(array->tokens, array->capacity * sizeof(token_t));
        // TODO CHECK IF MEMORY REALLOC FAULT
    }
    array->tokens[array->size++] = token;
}

void firstParserPass(parser_t *parser, TokenArray *tokenArray) {
    
    while ((parser->current_token = get_token(parser->scanner)).type != TK_EOF) {
        addToken(tokenArray, parser->current_token); // Add token to array
        if (parser->current_token.type == TK_KW_FUNC) {
            parseFunctionDefinition(parser, tokenArray);
        }
    }
    addToken(tokenArray, parser->current_token); // add EOF token to the end of array
}

void parseFunctionDefinition(parser_t *parser, TokenArray *tokenArray) {
    // <function_definition> -> func Identifier ( <parameter_list> ) <return_type_opt>
    
    check_next_token_and_add(tokenArray, parser, TK_IDENTIFIER);
    
    parser->global_frame = insertFunc(parser->global_frame, parser->current_token); // insert the function into tree and get updated root
    parser->current_func = search(parser->global_frame, parser->current_token.data.String); // set the new current function
    
    check_next_token_and_add(tokenArray, parser, TK_LPAR);

    parseFunctionParameters(parser, tokenArray);

    get_next_token(parser);
    
    if(check_token_type(parser, TK_LBRACE)){
        //ulozit return type ako NULL
        InsertType(parser->global_frame, parser->current_func->symbol.key, TK_KW_NIL);
        addToken(tokenArray, parser->current_token);

    } else if (check_token_type(parser, TK_ARROW)) {
        
        // ziskat dalsi token a porovnat ci je token datoveho typu
        addToken(tokenArray, parser->current_token);  
        get_next_token(parser);

        if (!is_token_datatype(parser->current_token.type)){
            handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR");
        }
        
        InsertType(parser->global_frame, parser->current_func->symbol.key, parser->current_token.type);

        addToken(tokenArray, parser->current_token);

        
    } else {
        // token nepatri do gramatiky
        handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR");
    }    
}

void parseFunctionParameters(parser_t *parser, TokenArray *tokenArray){

    get_next_token(parser);
    // Check for empty parameter list
    if (check_token_type(parser, TK_RPAR)) {
        addToken(tokenArray, parser->current_token);
        return;  // No parameters to process
    }

    while (true) {
        parseParameter(parser, tokenArray);

        if (check_token_type(parser, TK_COMMA)) {
            addToken(tokenArray, parser->current_token);  // Add comma to token array
            get_next_token(parser);  
        } else {
            break;
        }
    }
    
    if (!check_token_type(parser, TK_RPAR)){
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected ')'"); //exits the program
    }
    addToken(tokenArray, parser->current_token); // added ')' to tokenarray
}

void parseParameter(parser_t *parser, TokenArray *tokenArray) {

    if (parser->current_token.type != TK_IDENTIFIER && parser->current_token.type != TK_UNDERSCORE) {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected parameter name or underscore");
    }
    addToken(tokenArray, parser->current_token); 

    char *name;
    if(parser->current_token.type == TK_UNDERSCORE){
        name = "_";
    } else {
        name = parser->current_token.data.String;
    }
        
    get_next_token(parser);

    if (parser->current_token.type != TK_IDENTIFIER && parser->current_token.type != TK_UNDERSCORE) {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected parameter name or underscore");
    }
    addToken(tokenArray, parser->current_token);

    char *id;
    if(parser->current_token.type == TK_UNDERSCORE){
        id = "_";
    } else {
        id = parser->current_token.data.String;
    }
    

    check_next_token_and_add(tokenArray, parser, TK_COLON);

    get_next_token(parser);
    if (!is_token_datatype(parser->current_token.type)) {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected data type after colon in parameter definition");
    }
    addToken(tokenArray, parser->current_token);  

    tk_type_t type = parser->current_token.type;
    // ADD THIS PARAMETER WITH NAME, ID, AND TOKENTYPE TO THE SYMTABLE
    InsertParam(parser->global_frame, parser->current_func->symbol.key, name, id, type);
    // Prepare for next token (either a comma or right parenthesis)
    get_next_token(parser);  
}

/* retrieves token from scanner, checks if it matches expected token type, adds it to TokenArray */
void check_next_token_and_add(TokenArray *tokenArray, parser_t *parser, tk_type_t expectedType){

    parser->current_token = get_token(parser->scanner);

    if (!check_token_type(parser, expectedType)){
        handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR IN FUNC DEFINITION");
    }
    addToken(tokenArray, parser->current_token);
}

/* grabs next token from the scanner and saves it to parser */
void get_next_token(parser_t *parser) {
    parser->current_token = get_token(parser->scanner);
}

/* returns true if token is the expected type */
bool check_token_type(parser_t *parser, tk_type_t expectedType) {
    return parser->current_token.type == expectedType;
}

/* checks if token type is one of the language's datatype */
bool is_token_datatype(tk_type_t token) {
    switch (token) {
        case TK_KW_INT:
        case TK_KW_DOUBLE:
        case TK_KW_STRING:
        case TK_KW_INT_OPT:
        case TK_KW_DOUBLE_OPT:
        case TK_KW_STRING_OPT:
        case TK_KW_NIL:
            return true;
        default:
            return false;
    }
}
