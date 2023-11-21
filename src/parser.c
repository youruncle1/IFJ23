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
                       .inFunction = false,
                       .TKAIndex = 0,
                       .scopeDepth = 0
                       };

    return parser;
}

void parser_get_next_token(parser_t *parser, TokenArray *tokenArray) {

    if (parser->TKAIndex < tokenArray->size) {
        parser->current_token = tokenArray->tokens[parser->TKAIndex];
        parser->TKAIndex++;
    } else {
        // should never happen...
        handle_error(INTERNAL_COMPILER_ERROR, 0, "OUT OF TOKENARRAY BOUNDS");
    }
}

token_t token_lookahead(parser_t *parser, TokenArray *tokenArray) {
    if (parser->TKAIndex < tokenArray->size) {
        unsigned int next_idx = parser->TKAIndex + 1;
        return tokenArray->tokens[next_idx];
    } else {
        // should never happen...
        handle_error(INTERNAL_COMPILER_ERROR, 0, "OUT OF TOKENARRAY BOUNDS");
    }
}

void check_next_token(parser_t *parser, TokenArray *tokenArray, tk_type_t expectedType) {
    parser_get_next_token(parser, tokenArray);

    if (parser->current_token.type != expectedType){
        handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR UNKNOWN TOKEN");
    }
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

void parseProgram(parser_t *parser, TokenArray *tokenArray){


    parser_get_next_token(parser, tokenArray); // get the first token


    parseBlockContents(parser, tokenArray);
}

void parseBlockContents(parser_t* parser, TokenArray *tokenArray) {
    switch (parser->current_token.type) {
        case TK_EOF:
            return;
        default:
            parseBlockContent(parser, tokenArray);
            break;
    }
    parser_get_next_token(parser, tokenArray);
    parseBlockContents(parser, tokenArray);
}

void parseBlockContent(parser_t *parser, TokenArray *tokenArray){
    // <block_content> -> var Identifier <type_opt> <def_var>
    // <block_content> -> let Identifier <type_opt> <def_var>
    // <block_content> -> <function_definition>
    // <block_content> -> <control_structure>
    // <block_content> -> <function_call>
    // <block_content> -> ε

    switch (parser->current_token.type) {
        case TK_KW_FUNC:
            if (parser->scopeDepth > 0) {
                handle_error(SYNTAX_ERROR, parser->current_token.line,
                             "Functions cannot be defined outside global scope");
            }
        parseFunctionDefinition(parser, tokenArray);
        break;
        case TK_KW_VAR:                                             // -> var Identifier <type_opt> <def_var>
        case TK_KW_LET:                                             // -> let Identifier <type_opt> <def_var>
            parseVarDefinition(parser, tokenArray);
        break;
        case TK_KW_IF:                                              // -> <control_structure> - if
        case TK_KW_WHILE:                                           // -> <control_structure> - while
            parseControlStructure(parser, tokenArray);
        break;
        case TK_KW_RETURN:
            if (!parser->inFunction) {
                handle_error(SYNTAX_ERROR, parser->current_token.line, "return keyword found outside of function");
            }
        //parseReturn(parser, tokenArray);
        break;
        case TK_IDENTIFIER:
            // TU_SEMANTIKA
            // zistit, ci moze byt start term iba nejaky expression (a + b bez priradenia, proste expr ktory nic neurobi)
            if (token_lookahead(parser, tokenArray).type == TK_LPAR) {          // <function_call>
                parseFunctionCall(parser, tokenArray);
            } else if (token_lookahead(parser, tokenArray).type == TK_ASSIGN) { // <assignment>
                //parseAssignment(parser, tokenArray);
            } else {
                handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR");
            }
        break;
        default:
            handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR");
    }
}

void parseFunctionDefinition(parser_t *parser, TokenArray *tokenArray){
    // func Identifier ( <parameter_list> ) <return_type_opt> { <function_body> }
    parser->inFunction = true;
    parser->scopeDepth++;

    parser_get_next_token(parser, tokenArray);                        // Identifier

    setupFunctionScope(parser, parser->current_token.data.String);    // <parameter_list> Local frame setup

    while (parser->current_token.type != TK_LBRACE) {                 // skip until '{'
         parser_get_next_token(parser, tokenArray);
    }
    parser_get_next_token(parser, tokenArray);

    while(parser->current_token.type != TK_RBRACE) {
        parseBlockContent(parser, tokenArray);
        parser_get_next_token(parser,tokenArray);
    }

    parser->inFunction = false;
    parser->scopeDepth--;                                             // scopeDepth should be 0 after this!!!
    pop(parser->local_frame);                                         // Local symtable should be clear!!!

}

/* retrieves function's parameters from global frame, creates new symtable with said parameters and pushes it on local frame*/
void setupFunctionScope(parser_t *parser, const char *functionName) {

    Node *funcNode = search(parser->global_frame, functionName);

    Node *funcParamTable = NULL;
    for (int i = 0; i < funcNode->symbol.parametersCount; ++i) {
        Parameter param = funcNode->symbol.parameters[i];
        Symbol *symbol = initSymbol(param.id, param.type, false, true, false);
        funcParamTable = insert(funcParamTable, *symbol);
    }

    // Push this new symbol table onto the local frame stack
    push(parser->local_frame, funcParamTable);
}

void parseVarDefinition(parser_t *parser, TokenArray *tokenArray){

    bool isLet = (parser->current_token.type == TK_KW_LET);
    check_next_token(parser, tokenArray, TK_IDENTIFIER);

    const char* varName = parser->current_token.data.String;
    parser_insertVar2symtable(parser, isLet);                 // should do redefinition check

    bool hasType = false, hasInitialization = false;
    token_t tmpToken = parser->current_token;                          // udrzi nazov identifikatoru

    token_t lookAheadToken = parser->current_token;
    parser_get_next_token(parser, tokenArray);

    // Check for type
    if (parser->current_token.type == TK_COLON) {
        hasType = true;
        parser_get_next_token(parser, tokenArray); // Datatype
        if (!is_token_datatype(parser->current_token.type)) {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected data type");
        }
        // Update type in symbol table
        InsertType((parser->scopeDepth > 0) ? parser->local_frame->top->symbolTable : parser->global_frame, varName, parser->current_token.type);
        //parser_get_next_token(parser, tokenArray);
        token_t lookAheadToken = token_lookahead(parser,tokenArray);
    }

    // Check for initialization
    if (lookAheadToken.type == TK_ASSIGN) {
        hasInitialization = true;
        token_t nextToken = token_lookahead(parser, tokenArray);
        if (nextToken.type == TK_IDENTIFIER && token_lookahead(parser, tokenArray).type == TK_RPAR) {
            // Function call
            parseFunctionCall(parser, tokenArray);
        } else {
            // Expression
            //parseExpression(parser, tokenArray);
        }
        /*
            TU_SEMANTIKA
            nejak zistit aky vysiel typ z 'vyraz', ci uz expression alebo fnc call
            porovnat ci sedi s lavou stranou ak bol pri lavej strane typ (hasType)
                ak nebol definovany typ -> vlozit typ do variabilnej symtable pomocou insertType()
            a pouzit updateInit(), zmeni flag ze bola premenna inicializovana
        */
        var_updateInit(parser, tmpToken);
    }

    // parser didn't get any of ':', '='
    if (!hasType && !hasInitialization) {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Variable declaration must include a type or initialization");
    }
}

void parser_insertVar2symtable(parser_t *parser, bool isLet){
    if (parser->scopeDepth > 0) {
        parser->local_frame->top->symbolTable = insertVar(parser->local_frame->top->symbolTable, parser->current_token, isLet, false);
    } else {
        parser->global_frame = insertVar(parser->local_frame->top->symbolTable, parser->current_token, isLet, false);
    }
}

void var_updateInit(parser_t *parser, token_t token){
    Node* root;

    if (parser->scopeDepth > 0) {
        root = search(parser->local_frame->top->symbolTable, token.data.String);
        root->symbol.isInit = true;
    } else {
        root = search(parser->global_frame, token.data.String);
        root->symbol.isInit = true;
    }
}

void parseControlStructure(parser_t *parser, TokenArray *tokenArray) {

    parser->scopeDepth++;

    if (parser->current_token.type == TK_KW_IF) {
        parser_get_next_token(parser, tokenArray); // Get token after 'if'

        if (parser->current_token.type == TK_KW_LET) {
                // TU_SEMANTIKA
                // dokoncit semantiku pre let id variaciu
        } else {
            //parseExpression(parser, tokenArray); // Parse the conditional expression
        }

        check_next_token(parser, tokenArray, TK_LBRACE); // Check for '{'

        Node* newScope = NULL;
        push(parser->local_frame, newScope); // Push new empty scope to local frame

        parseBlockContent(parser, tokenArray);

        check_next_token(parser, tokenArray, TK_RBRACE); // Check for '}'

        // Optional 'else' part
        if (token_lookahead(parser, tokenArray).type == TK_KW_ELSE) {
            parser_get_next_token(parser, tokenArray); // Consume 'else'

            check_next_token(parser, tokenArray, TK_LBRACE); // Check for '{'

            parseBlockContent(parser, tokenArray);

            check_next_token(parser, tokenArray, TK_RBRACE); // Check for '}'
        }

    } else if (parser->current_token.type == TK_KW_WHILE) {
        parser_get_next_token(parser, tokenArray); // Get token after 'while'

        //parseExpression(parser, tokenArray); // Parse the conditional expression

        check_next_token(parser, tokenArray, TK_LBRACE); // Check for '{'

        Node* newScope = NULL;
        push(parser->local_frame, newScope); // Push new empty scope to local frame

        parseBlockContent(parser, tokenArray);

        check_next_token(parser, tokenArray, TK_RBRACE); // Check for '}'

    } else {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected 'if' or 'while'"); // This should not happen
    }
    pop(parser->local_frame);
    parser->scopeDepth--;
}


/* ||||||||||||||||||||||||||||||||| FINSH PARSEFUNCTIONCALL ||||||||||||||||||||||||||||||||*/

void parseFunctionCall(parser_t *parser, TokenArray *tokenArray) {

    token_t tmpToken = parser->current_token;

    parser_get_next_token(parser, tokenArray); // consume '('

    Parameter *parsedParameters = NULL;
    unsigned int *parsedParamCount = 0;

    parseFunctionCallParams(parser, tokenArray, tmpToken, &parsedParameters, &parsedParamCount);

    //check_next_token(parser, tokenArray, TK_RPAR); uz by mal byt consumed z parseFunctionCallParams

    Node* functionNode = search(parser->global_frame, tmpToken.data.String);
    if (functionNode == NULL || !functionNode->symbol.isFunction) {
        handle_error(SEMANTIC_UNDEFINED_FUNCTION, parser->current_token.line, "Undefined function call");
        return;
    }

    // TU_SEMANTIKA
    // vsetko okolo funcCall
}

void parseFunctionCallParams(parser_t *parser, TokenArray *tokenArray, token_t funcToken, Parameter **parsedParameters, unsigned int *parsedParamCount){

    parser_get_next_token(parser, tokenArray);

    // Check for empty parameter list
    if (parser->current_token.type == TK_RPAR) {
        return;  // No parameters to process
    }

    while (true) {
        parseCallParameter(parser, tokenArray, funcToken, parsedParameters, parsedParamCount);

        if (check_token_type(parser, TK_COMMA)) {
              // do nothing
        } else {
            break;
        }
    }

    if (!check_token_type(parser, TK_RPAR)){
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected ')'");
    }
}

void parseCallParameter(parser_t *parser, TokenArray *tokenArray, token_t funcToken, Parameter **parsedParameters, unsigned int *parsedParamCount){

    if (parser->current_token.type == TK_IDENTIFIER) {

        *parsedParameters = realloc(*parsedParameters, (*parsedParamCount + 1) * sizeof(Parameter));
        if (*parsedParameters == NULL) {
            handle_error(INTERNAL_COMPILER_ERROR, parser->current_token.line, "Memory allocation error");
            return;
        }


        (*parsedParameters)[*parsedParamCount].name = strdup(parser->current_token.data.String);
        (*parsedParameters)[*parsedParamCount].type = TK_KW_NIL; // TU_SEMANTIKA dalsie checks....

        (*parsedParamCount)++;

        parser_get_next_token(parser, tokenArray);

    } else {
        // TU_SEMANTIKA
        // handle expression parameter
        //parseExpression(parser, tokenArray);
    }

    parser_get_next_token(parser, tokenArray);

    if (parser->current_token.type == TK_COMMA) {
        parser_get_next_token(parser, tokenArray); // Move past comma to the next parameter
    } else if (parser->current_token.type != TK_RPAR) {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected ',' or ')'");
    }
}




/* #######################################################################################################################*/
/* #######################################################################################################################*/
/* ############################################## functions for first pass ###############################################*/
/* #######################################################################################################################*/
/* #######################################################################################################################*/

/* retrieves next token from the scanner and saves it to parser */
void get_next_token(parser_t *parser) {
    parser->current_token = get_token(parser->scanner);
}

/* retrieves token from scanner, checks if it matches expected token type, adds it to TokenArray */
void check_next_token_and_add(TokenArray *tokenArray, parser_t *parser, tk_type_t expectedType){

    parser->current_token = get_token(parser->scanner);

    if (!check_token_type(parser, expectedType)){
        handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR IN FUNC DEFINITION");
    }
    addToken(tokenArray, parser->current_token);
}

void addToken(TokenArray *array, token_t token) {
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->tokens = realloc(array->tokens, array->capacity * sizeof(token_t));
        // TODO CHECK IF MEMORY REALLOC FAULT
    }
    array->tokens[array->size++] = token;
}

/* FirstPass: stores every encountered token into tokenArray and parses function definitions
   to store them into the global symtable */
void firstParserPass(parser_t *parser, TokenArray *tokenArray) {

    while ((parser->current_token = get_token(parser->scanner)).type != TK_EOF) {
        addToken(tokenArray, parser->current_token); // Add token to array
        if (parser->current_token.type == TK_KW_FUNC) {
            CollectFunctionDefinition(parser, tokenArray);
        }
    }
    addToken(tokenArray, parser->current_token); // add EOF token to the end of array
}

/* top-down recursive function definition parsing (up until '{') */
void CollectFunctionDefinition(parser_t *parser, TokenArray *tokenArray) {
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

        check_next_token_and_add(tokenArray, parser, TK_LBRACE); // pridaj LBRACE nech su cases symetricke(2. pass postaci skipnut po rbrace)


    } else {
        // token nepatri do gramatiky
        handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR");
    }
}

/* recursively parses function's parameters */
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

/* parses individual parameter and stores it to the function's details in symtable */
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

    if (strcmp(name, id) == 0){
        handle_error(OTHER_SEMANTIC_ERROR, parser->current_token.line, "PARAMETER ID AND NAME CANNOT BE SAME");
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
