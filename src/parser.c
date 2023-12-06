/*
IFJ PROJEKT 2023/2024
file: "parser.c"

Top-down recursive parser

authors: xpolia05

*/

#include "scanner.h"
#include "error.h"
#include "parser.h"
#include "expr.h"

TokenArray *initTokenArray() {
    TokenArray *array = malloc(sizeof(TokenArray));
    array->size = 0;
    array->capacity = 10; 
    array->tokens = malloc(array->capacity * sizeof(token_t));
    return array;
}

parser_t initParser(scanner_t *scanner) {
    parser_t parser = {.scanner = scanner,
            .local_frame = initStack(),
            .global_frame = NULL,
            .inFunction = false,
            .hasReturn = false,
            .isReturn = false,
            .TKAIndex = 0,
            .scopeDepth = 0,
            .parsedParamCount = 0
    };

    return parser;
}

Node *searchFramesVar(parser_t *parser){
    Node *node;

    if ((node = stackSearch(parser->local_frame, parser->current_token.data.String)) != NULL) {
        if (node->symbol.isFunction) {
            handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Undefined variable");
        }
        return node;

    } else if ((node = search(parser->global_frame, parser->current_token.data.String)) != NULL) {
        if (node->symbol.isFunction) {
            handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Undefined variable");
        }
        return node;
    } else {
        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Undefined variable");
    }
    return NULL;
}

void parser_get_next_token(parser_t *parser, TokenArray *tokenArray) {

    if (parser->TKAIndex < tokenArray->size) {
        parser->current_token = tokenArray->tokens[parser->TKAIndex];
        parser->TKAIndex++;
    } else {
        
        handle_error(INTERNAL_COMPILER_ERROR, 0, "OUT OF TOKENARRAY BOUNDS");
    }
}

void parser_get_previous_token(parser_t *parser, TokenArray *tokenArray) {

    if (parser->TKAIndex > 0) {
        parser->current_token = tokenArray->tokens[parser->TKAIndex - 2];
        parser->TKAIndex--;
    } else {
       
        handle_error(INTERNAL_COMPILER_ERROR, 0, "OUT OF TOKENARRAY BOUNDS");
    }
}

token_t token_lookahead(parser_t *parser, TokenArray *tokenArray) {
    if (parser->TKAIndex < tokenArray->size) {
        unsigned int next_idx = parser->TKAIndex;
        return tokenArray->tokens[next_idx];
    } else {
       
        handle_error(INTERNAL_COMPILER_ERROR, 0, "OUT OF TOKENARRAY BOUNDS");
        return tokenArray->tokens[parser->TKAIndex];;
    }
    
}
token_t token_lookback(parser_t *parser, TokenArray *tokenArray) {
    if (parser->TKAIndex > 0) {
        unsigned int prev_idx = parser->TKAIndex - 1;
        return tokenArray->tokens[prev_idx];
    } else {
        
        handle_error(INTERNAL_COMPILER_ERROR, 0, "OUT OF TOKENARRAY BOUNDS");
        return tokenArray->tokens[parser->TKAIndex];
    }
}
void check_next_token(parser_t *parser, TokenArray *tokenArray, tk_type_t expectedType) {
    parser_get_next_token(parser, tokenArray);

    if (parser->current_token.type != expectedType){
        handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR UNKNOWN TOKEN");
    }
}

bool check_token_type(parser_t *parser, tk_type_t expectedType) {
    return parser->current_token.type == expectedType;
}

bool is_token_datatype(tk_type_t token) {
    switch (token) {
        case TK_KW_INT:         // Int
        case TK_KW_DOUBLE:      // Double
        case TK_KW_STRING:      // String
        case TK_KW_INT_OPT:     // Int?
        case TK_KW_DOUBLE_OPT:  // Double?
        case TK_KW_STRING_OPT:  // String?
        case TK_KW_NIL:         // for unintialized variables
            return true;
        default:
            return false;
    }
}

bool is_token_literal(tk_type_t token) {
    switch (token) {
        case TK_INT:         // Int
        case TK_DOUBLE:      // Double
        case TK_STRING:      // String
        case TK_MLSTRING:    // Multiline String
            return true;
        default:
            return false;
    }
}

tk_type_t convert_literal_to_datatype(tk_type_t tokenType) {
    switch (tokenType) {
        case TK_INT:
            return TK_KW_INT;
        case TK_MLSTRING:
        case TK_STRING:
            return TK_KW_STRING;
        case TK_DOUBLE:
            return TK_KW_DOUBLE;
        default:
            return tokenType;
    }
}

void parseProgram(parser_t *parser, TokenArray *tokenArray, generator_t* gen){


    parser_get_next_token(parser, tokenArray);


    parseBlockContents(parser, tokenArray, gen);
}

void parseBlockContents(parser_t* parser, TokenArray *tokenArray, generator_t* gen) {
    switch (parser->current_token.type) {
        case TK_EOF:
            return;
        default:
            parseBlockContent(parser, tokenArray, gen);
            break;
    }
    parser_get_next_token(parser, tokenArray);
    parseBlockContents(parser, tokenArray, gen);
}

void parseBlockContent(parser_t *parser, TokenArray *tokenArray, generator_t* gen){

    switch (parser->current_token.type) {
        case TK_KW_FUNC:
            if (parser->scopeDepth > 0) {
                handle_error(SYNTAX_ERROR, parser->current_token.line,
                             "Functions cannot be defined outside global scope");
            }
            parseFunctionDefinition(parser, tokenArray, gen);
            break;
        case TK_KW_VAR:                                             
        case TK_KW_LET:                                             
            parseVarDefinition(parser, tokenArray, gen);
            break;
        case TK_KW_IF:                                              
        case TK_KW_WHILE:                                           
            parseControlStructure(parser, tokenArray, gen);
            break;
        case TK_KW_RETURN:
            if (!parser->inFunction) {
                handle_error(SYNTAX_ERROR, parser->current_token.line, "return keyword found outside of function");
            }
            parseReturn(parser, tokenArray, gen);
            break;
        case TK_IDENTIFIER:
            
            
            if (token_lookahead(parser, tokenArray).type == TK_LPAR) {          
                parseFunctionCall(parser, tokenArray, gen);
            } else if (token_lookahead(parser, tokenArray).type == TK_ASSIGN) {
                parseAssignment(parser, tokenArray, gen);
            } else {
                handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR");
            }
            break;
        default:
            handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR");
    }
}

void parseFunctionDefinition(parser_t *parser, TokenArray *tokenArray, generator_t* gen){
    
    parser->inFunction = true;
    parser->scopeDepth++;

    parser_get_next_token(parser, tokenArray);                       

    Node *node = search(parser->global_frame, parser->current_token.data.String);
    gen_FunctionHeader( gen, parser->current_token.data.String ,node,parser->global_frame,parser->scopeDepth);

    parser->current_func = search(parser->global_frame, parser->current_token.data.String);

    setupFunctionScope(parser, parser->current_token.data.String);    // local frame setup

    while (parser->current_token.type != TK_LBRACE) {                 
        parser_get_next_token(parser, tokenArray);

    }
    parser_get_next_token(parser, tokenArray);
    while(parser->current_token.type != TK_RBRACE) {
        parseBlockContent(parser, tokenArray, gen);
        parser_get_next_token(parser,tokenArray);
    }

    gen_FunctionFooter(gen);
    parser->inFunction = false;
    parser->hasReturn = false;
    parser->scopeDepth--;                                             
    pop(parser->local_frame);                                         

}

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

bool isStartOfExpression(tk_type_t tokenType) {
    // Check if tokenType is one of the operators that can start a complex expression
    switch (tokenType) {
        case TK_IDENTIFIER: // a, varB, result ...
        case TK_INT:        // 10, 5, 2 ...
        case TK_DOUBLE:     // 2.3, 33414.3, 0.1 ...
        case TK_STRING:     // "hello", "", ...
        case TK_MLSTRING:
        case TK_LPAR:       // (...
            return true;
        default:
            return false;
    }
}

bool isPartOfExpression(tk_type_t tokenType) {
    switch (tokenType) {
        case TK_MUL:
        case TK_DIV:
        case TK_PLUS:
        case TK_MINUS:
        case TK_LT:
        case TK_LE:
        case TK_GT:
        case TK_GE:
        case TK_EQ:
        case TK_NEQ:
        case TK_COALESCE:
        case TK_UNWRAP:
            return true;
        default:
            return false;
    }
}

void parseVarDefinition(parser_t *parser, TokenArray *tokenArray, generator_t* gen){

    bool isLet = (parser->current_token.type == TK_KW_LET);

    check_next_token(parser, tokenArray, TK_IDENTIFIER); 
    token_t tmpToken = parser->current_token;            
    tk_type_t foundDataType = TK_KW_NIL;              
    tk_type_t foundType = TK_KW_NIL;                  

    gen_VarDefinition( gen, parser->current_token.data.String, parser->inFunction, parser->scopeDepth);

    if (search((parser->scopeDepth > 0) ? parser->local_frame->top->symbolTable : parser->global_frame, parser->current_token.data.String) != NULL){
        handle_error(SEMANTIC_UNDEFINED_FUNCTION, parser->current_token.line, "Variable redefinition in same scope");
    }

    bool hasType = false, hasInitialization = false;

    token_t lookAheadToken = token_lookahead(parser, tokenArray);

    
    token_t varToAssign = parser->current_token;
    if (lookAheadToken.type == TK_COLON) {

        parser_get_next_token(parser, tokenArray); 
        parser_get_next_token(parser, tokenArray);
        if (!is_token_datatype(parser->current_token.type)) {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected data type");
        }
        
       
        hasType = true;
        foundDataType = parser->current_token.type;
        lookAheadToken = token_lookahead(parser, tokenArray); 
    }

    // Check for initialization
    if (lookAheadToken.type == TK_ASSIGN) {
        parser_get_next_token(parser, tokenArray); 
        parser_get_next_token(parser, tokenArray);
        hasInitialization = true;

        token_t nextToken = token_lookahead(parser, tokenArray); // Look ahead to distinguish between function call and expression

        if (parser->current_token.type == TK_IDENTIFIER && nextToken.type == TK_LPAR) {
            
            parseFunctionCall(parser, tokenArray, gen);
            foundType = parser->current_func_call->symbol.type;
            if (foundType == TK_KW_NIL){
                handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Cannot assign a void function");
            }

            gen_AssignReturnToVariable(gen,varToAssign,parser->inFunction);

        } else if ((isStartOfExpression(parser->current_token.type) && isPartOfExpression(nextToken.type))
                   || (parser->current_token.type == TK_LPAR && isStartOfExpression(nextToken.type))) {
           
           
            gen_SaveExprResult( gen, tmpToken.data.String);
            foundType = rule_expression(parser, tokenArray, gen);
            gen_ClearExprResult( gen, parser->inFunction );
            foundType = convert_literal_to_datatype(foundType); // me no like

        } else if (isStartOfExpression(parser->current_token.type) && !isPartOfExpression(nextToken.type)) {
            
            if (parser->current_token.type == TK_IDENTIFIER) {
                
                Node *node = searchFramesVar(parser);

                foundType = node->symbol.type;

                if (!node->symbol.isInit){
                    handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Assigning an uninitialized variable");
                }
            } else {
               
                foundType = convert_literal_to_datatype(parser->current_token.type);
                
            }
            char buffer[255];
            switch (parser->current_token.type){
                case(TK_INT):
                    sprintf(buffer, "%lld", parser->current_token.data.Int);
                    gen_AssignVal( gen, tmpToken.data.String,buffer, parser->inFunction, " int@", parser->scopeDepth);
                    break;
                case(TK_DOUBLE):
                    sprintf(buffer, "%a", parser->current_token.data.Double);
                    gen_AssignVal( gen, tmpToken.data.String,buffer, parser->inFunction, " float@", parser->scopeDepth);
                    break;
                case(TK_MLSTRING):
                case(TK_STRING):
                    gen_AssignVal( gen, tmpToken.data.String, gen_convertString( parser->current_token.data.String ), parser->inFunction, " string@", parser->scopeDepth);
                    break;
                case(TK_IDENTIFIER):
                    gen_AssignVal( gen, tmpToken.data.String,parser->current_token.data.String, parser->inFunction, "", parser->scopeDepth);
                    break;
                default:

                    break;
            }
        } else {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR IN VARIABLE DEFINITION");
        }
    }

    parser_insertVar2symtable(parser, tmpToken, isLet);
    if (hasType && hasInitialization){
        if (foundDataType != foundType) {
            handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Type mismatch");
        }
    }

    if (hasType) {
        InsertType((parser->scopeDepth > 0) ? parser->local_frame->top->symbolTable : parser->global_frame, tmpToken.data.String, foundDataType);
    }
    if (hasInitialization){
        check_VarType(parser, tmpToken, foundType, hasType);
        var_updateInit(parser, tmpToken);


    }
    
    if (!hasType && !hasInitialization) {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Variable declaration must include a type or initialvarTypeization");
    }
}

void check_VarType(parser_t *parser, token_t foundToken, tk_type_t type, bool hasType){
    if (!hasType) {
        InsertType((parser->scopeDepth > 0) ? parser->local_frame->top->symbolTable : parser->global_frame, foundToken.data.String, type);
    } else {
        if (search((parser->scopeDepth > 0) ? parser->local_frame->top->symbolTable : parser->global_frame, foundToken.data.String)->symbol.type != type) {
            handle_error(SEMANTIC_TYPE_COMPATIBILITY, foundToken.line, "Incompatible type of assigned variable with defined variable");
        }
    }
}

tk_type_t find_varType(parser_t *parser){
    Node *node;
    if ((node = stackSearch(parser->local_frame, parser->current_token.data.String)) != NULL) {

        if (node->symbol.isFunction){
            handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Left side of the assignment is an function name, not a variable");
        }
        return node->symbol.type;

    } else if ((node = search(parser->global_frame, parser->current_token.data.String)) != NULL) {

        if (node->symbol.isFunction){
            handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Left side of the assignment is an function name, not a variable");
        }
        return node->symbol.type;

    } else {
        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "");
    }
    return TK_KW_NIL;
}

void parser_insertVar2symtable(parser_t *parser, token_t tmpToken, bool isLet){
    if (parser->scopeDepth > 0) {
        parser->local_frame->top->symbolTable = insertVar(parser->local_frame->top->symbolTable, tmpToken, isLet, false);
    } else {
        parser->global_frame = insertVar(parser->global_frame, tmpToken, isLet, false);
    }
}

void var_updateInit(parser_t *parser, token_t token){
    Node *node;

    if ((node = stackSearch(parser->local_frame, token.data.String)) != NULL) {
        if (node->symbol.isFunction) {
            handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Undefined variable");
        }
        node->symbol.isInit = true;
    } else if ((node = search(parser->global_frame, token.data.String)) != NULL) {
        if (node->symbol.isFunction) {
            handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Undefined variable");
        }
        node->symbol.isInit = true;
    } else {
        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Undefined variable");
    }
}

void parseControlStructure(parser_t *parser, TokenArray *tokenArray, generator_t* gen) {

    parser->scopeDepth++;

    tk_type_t foundType;
    Node *letId;
    

    if (parser->current_token.type == TK_KW_IF) {

        parser_get_next_token(parser, tokenArray); 

        if (parser->current_token.type == TK_KW_LET) {
            
            check_next_token(parser, tokenArray, TK_IDENTIFIER);
            letId = searchFramesVar(parser);
            if (!letId->symbol.isLet){
                handle_error(OTHER_SEMANTIC_ERROR, parser->current_token.line, "Let id, id is not an immutable variable");
            }

        } else {
            
            foundType = rule_expression(parser, tokenArray, gen); // Parse the conditional expression
            if (foundType != TK_BOOLEAN){
                handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Expected boolean in if statement");
            }
        }

        if(parser->scopeDepth != 1) {
            gen_IfThenElse(gen, parser->scopeDepth, parser->inFunction,
                           parser->global_frame);
        }else{
            gen_IfThenElse(gen, parser->scopeDepth, parser->inFunction,
                           parser->global_frame);
        }
        

        check_next_token(parser, tokenArray, TK_LBRACE); 

        Node* newScope = NULL;
        push(parser->local_frame, newScope); 

        parser_get_next_token(parser,tokenArray);
        while (parser->current_token.type != TK_RBRACE){
            parseBlockContent(parser, tokenArray, gen);
            parser_get_next_token(parser,tokenArray);
        }

        pop(parser->local_frame); 

        push(parser->local_frame, newScope); 
        gen_IfDone(gen, parser->scopeDepth, parser->inFunction);

        check_next_token(parser, tokenArray, TK_KW_ELSE);

        gen_IfThenElse_End( gen, parser->scopeDepth, parser->inFunction, parser->global_frame);

        check_next_token(parser, tokenArray, TK_LBRACE);
        parser_get_next_token(parser,tokenArray);
        while (parser->current_token.type != TK_RBRACE){
            parseBlockContent(parser, tokenArray, gen);
            parser_get_next_token(parser,tokenArray);
        }

        pop(parser->local_frame); 

        gen_IfDone_End( gen, parser->scopeDepth, parser->inFunction );

       

    } else if (parser->current_token.type == TK_KW_WHILE) {
        parser_get_next_token(parser, tokenArray); 

        gen->isWhile = true;

        gen_While( gen, parser->scopeDepth, parser->inFunction, parser->global_frame);

        foundType = rule_expression(parser, tokenArray, gen); 
        if (foundType != TK_BOOLEAN){
            handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Expected boolean in while statement");
        }

        gen_WhileCond( gen, parser->scopeDepth, parser->inFunction );

        check_next_token(parser, tokenArray, TK_LBRACE); 

        Node* newScope = NULL;
        push(parser->local_frame, newScope);

        parser_get_next_token(parser,tokenArray);
        while (parser->current_token.type != TK_RBRACE){
            parseBlockContent(parser, tokenArray, gen);
            parser_get_next_token(parser,tokenArray);
        }
        pop(parser->local_frame); 
        gen_WhileEnd( gen, parser->scopeDepth, parser->inFunction );

        gen->isWhile = false;
        

    } else {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected 'if' or 'while'");
    }

    parser->scopeDepth--;
}


void parseReturn(parser_t *parser, TokenArray *tokenArray, generator_t* gen) {

    parser->hasReturn = true;
    gen->isReturn = true;

    token_t lookAheadToken = token_lookahead(parser, tokenArray);

    if(lookAheadToken.eol_before){
        if (parser->current_func->symbol.type != TK_KW_NIL){
            handle_error(SEMANTIC_RETURN_VALUE, parser->current_token.line, "Return without expression in non void function");
        }
    }
    if (parser->current_func->symbol.type == TK_KW_NIL){
        if (isStartOfExpression(lookAheadToken.type) && !lookAheadToken.eol_before){
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Return in void function cannot have expression");
        }
    } else {
        parser_get_next_token(parser, tokenArray); 

        token_t nextToken = token_lookahead(parser, tokenArray);

        tk_type_t foundType;

        if (parser->current_token.type == TK_IDENTIFIER && nextToken.type == TK_LPAR) {
           
            handle_error(OTHER_SEMANTIC_ERROR, parser->current_token.line, "Return expression cannot be function call!");

        } else if ((isStartOfExpression(parser->current_token.type) && isPartOfExpression(nextToken.type))
                   || (parser->current_token.type == TK_LPAR && isStartOfExpression(nextToken.type))) {
           
            if(parser->current_token.type == TK_IDENTIFIER) {
                if (searchFramesVar(parser)->symbol.type == TK_KW_STRING) {
                    gen_SaveExprResult(gen, lookAheadToken.data.String);
                }
            }
            if(parser->current_token.type == TK_STRING) {
                gen_SaveExprResult(gen, lookAheadToken.data.String);
            }
            foundType = rule_expression(parser, tokenArray, gen);
            gen_ClearExprResult(gen,parser->inFunction);
            foundType = convert_literal_to_datatype(foundType); 

        } else if (isStartOfExpression(parser->current_token.type) && !isPartOfExpression(nextToken.type)) {
            if (parser->current_token.type == TK_IDENTIFIER) {
               
                Node *node = searchFramesVar(parser);

                foundType = node->symbol.type;

                if (!node->symbol.isInit){
                    handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Returning an uninitialized variable");
                }
                gen_IdentifierReturn(gen,parser->current_token,parser->scopeDepth);

            } else {
                
                foundType = convert_literal_to_datatype(parser->current_token.type);
                

                gen_LiteralReturn(gen,parser->current_token);
            }
        } else {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected expression after return statement in non-void function");
        }

        if (parser->current_func->symbol.type != foundType){
            handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Incompatible expression type of return with type of function");
        }

    }
}

void parseFunctionCall(parser_t *parser, TokenArray *tokenArray, generator_t* gen) {


    token_t funcToken = parser->current_token; 
    gen->isWrite = strcmp(funcToken.data.String, "write");

    if(gen->isWrite != 0 ){
        gen_CreateFrame(gen,parser->inFunction);
    }
    parser_get_next_token(parser, tokenArray);

    Parameter *parsedParameters = NULL;

    parseFunctionCallParams(parser, tokenArray, &parsedParameters, gen);

    int parsedParamCount = parser->parsedParamCount;

    Node* functionNode = search(parser->global_frame, funcToken.data.String);
    if (functionNode == NULL || !functionNode->symbol.isFunction) {
        handle_error(SEMANTIC_UNDEFINED_FUNCTION, parser->current_token.line, "Undefined function call");
        return;
    }

    if (functionNode->symbol.parametersCount != parsedParamCount){
        if (strcmp(funcToken.data.String, "write") != 0){
            handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Wrong number of parameters in function call");
            return;
        }
    }
   
    if (strcmp(functionNode->symbol.key, "write") == 0) {
        for (int i = 0; i < parsedParamCount; i++){
            if (parsedParameters[i].name){
                handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Write has no parameter with name");
            }

            if (parsedParameters[i].id) {
                Node *node;
                if ((node = stackSearch(parser->local_frame, parsedParameters[i].id)) != NULL) {
                    if (node->symbol.isFunction || !node->symbol.isInit){
                        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of undefined or uninitialized variable in one of the parameters");
                    }
                }

                if (!node) {
                    if ((node = search(parser->global_frame, parsedParameters[i].id)) != NULL) {
                        if (node->symbol.isFunction || !node->symbol.isInit) {
                            handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of undefined  or uninitialized variable in one of the parameters");
                        }
                    }
                }

                if (!node) {
                    handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of undefined variable");
                }
                gen_FunctionParam( gen,parsedParameters[i].id, parser->inFunction,parsedParamCount,parser->scopeDepth + 1);
            }

        }
    } else {

        for (int i = 0; i < parsedParamCount; i++){

            if (strcmp(functionNode->symbol.parameters[i].name, "_") != 0){
                if (!parsedParameters[i].name){
                    handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Missing parameter name in function call");
                }
                if (parsedParameters[i].id) {
                    Node *node;
                    if ((node = stackSearch(parser->local_frame, parsedParameters[i].id)) != NULL) {
                        if (node->symbol.isFunction || !node->symbol.isInit){
                            handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of undefined or uninitialized variable in one of the parameters");
                        }
                    }

                    if (!node) {
                        if ((node = search(parser->global_frame, parsedParameters[i].id)) != NULL) {
                            if (node->symbol.isFunction || !node->symbol.isInit) {
                                handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of undefined  or uninitialized variable in one of the parameters");
                            }
                        }
                    }

                    if (!node) {
                        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of undefined variable");
                    }
                    gen_FunctionParam( gen,parsedParameters[i].id, parser->inFunction,parsedParamCount,parser->scopeDepth + 1);
                }
            }

            if (parsedParameters[i].name) {
                
                if (strcmp(functionNode->symbol.parameters[i].name, "_") == 0){
                    handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Called parameter with a name when it does not have a name");
                }
                
                if (strcmp(functionNode->symbol.parameters[i].name, parsedParameters[i].name) != 0){
                    handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Wrong name in function call");
                }
            }
            if (parsedParameters[i].id) {
                // check ci existuje
                Node *node;
                if ((node = stackSearch(parser->local_frame, parsedParameters[i].id)) != NULL) {
                    if (node->symbol.isFunction){
                        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of undefined variable in one of the parameters");
                    }
                }

                if (!node) {
                    if ((node = search(parser->global_frame, parsedParameters[i].id)) != NULL) {
                        if (node->symbol.isFunction) {
                            handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of undefined variable in one of the parameters");
                        }
                    }
                }

                if (!node) {
                    handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of undefined variable");
                }

                // check ci je init
                if (!node->symbol.isInit) {
                    handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Usage of uninitialized variable in one of the parameters");
                }
                // check typ s definiciou funkcie
                if (functionNode->symbol.parameters[i].type != node->symbol.type){
                    handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Wrong name in function call");
                }
                gen_FunctionParam( gen,parsedParameters[i].id, parser->inFunction,parsedParamCount,parser->scopeDepth + 1);

            }
            if (parsedParameters[i].type != TK_KW_NIL) {
                // checknut typ s definiciou funkcie
                if (functionNode->symbol.parameters[i].type != parsedParameters[i].type){
                    handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Wrong type in function call");
                }
            }
        }
    }

   
    if ( gen->isWrite != 0) {
        gen_FunctionCall( gen, funcToken.data.String, parser->inFunction );
    }

    // for assignment check when a = funccall()
    parser->current_func_call = functionNode;
    parser->parsedParamCount = 0;
}

void parseFunctionCallParams(parser_t *parser, TokenArray *tokenArray, Parameter **parsedParameters, generator_t* gen){

    parser_get_next_token(parser, tokenArray);

 
    if (parser->current_token.type == TK_RPAR) {
        return; 
    }

    while (true) {
        parseCallParameter(parser, tokenArray, parsedParameters, gen);

        if (check_token_type(parser, TK_COMMA)) {
            parser_get_next_token(parser,tokenArray);
        } else {
            break;
        }
    }

    if (!check_token_type(parser, TK_RPAR)){
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected ')'");
    }
}

void parseCallParameter(parser_t *parser, TokenArray *tokenArray, Parameter **parsedParameters, generator_t* gen){
    
    int parsedParamCount = parser->parsedParamCount;
    token_t lookaheadToken;

    *parsedParameters = realloc((*parsedParameters), ((parsedParamCount) + 1) * sizeof(Parameter));

    if (*parsedParameters == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, parser->current_token.line, "Memory allocation error");
        return;
    }
    (*parsedParameters)[parsedParamCount].name = NULL;
    (*parsedParameters)[parsedParamCount].id = NULL;
    (*parsedParameters)[parsedParamCount].type = TK_KW_NIL;
    // Token 1: Identifier found
    if (parser->current_token.type == TK_IDENTIFIER) {
        lookaheadToken = token_lookahead(parser, tokenArray);

        
        if (lookaheadToken.type == TK_COLON) {
            
            (*parsedParameters)[parsedParamCount].name = strdup(parser->current_token.data.String);

            parser_get_next_token(parser, tokenArray);  
            parser_get_next_token(parser, tokenArray);

            if (parser->current_token.type == TK_IDENTIFIER) {
                
                (*parsedParameters)[parsedParamCount].id = strdup(parser->current_token.data.String);
                parser_get_next_token(parser,tokenArray);
                gen_FunctionParam( gen, (*parsedParameters)[parsedParamCount].id, parser->inFunction,parsedParamCount, parser->scopeDepth + 1);
            } else if (is_token_literal(parser->current_token.type)) {
                
                (*parsedParameters)[parsedParamCount].type = convert_literal_to_datatype(parser->current_token.type);
                switch( (*parsedParameters)[parsedParamCount].type ) {
                    case TK_KW_INT:
                    case TK_KW_INT_OPT:
                        gen_FunctionParamInt( gen, parser->current_token.data.Int, parser->inFunction, parsedParamCount + 1);
                        break;
                    case TK_KW_DOUBLE:
                    case TK_KW_DOUBLE_OPT:
                        gen_FunctionParamDouble( gen, parser->current_token.data.Double, parser->inFunction, parsedParamCount + 1);
                        break;
                    case TK_KW_STRING:
                    case TK_KW_STRING_OPT:
                        gen_FunctionParamString( gen, parser->current_token.data.String, parser->inFunction, parsedParamCount + 1);
                        break;
                    case TK_KW_NIL:
                        gen_FunctionParamNil( gen, parser->inFunction );
                    default:
                        break;
                }
                parser_get_next_token(parser,tokenArray);
            } else {
                handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected a variablei identifier or literal after ':'");
            }
        } else if (lookaheadToken.type == TK_COMMA || lookaheadToken.type == TK_RPAR) {

            (*parsedParameters)[parsedParamCount].id = strdup(parser->current_token.data.String);
            parser_get_next_token(parser, tokenArray);
        } else {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected ':', ',' or ')' after identifier");
        }




    }
        
    else if (is_token_literal(parser->current_token.type)) {
        lookaheadToken = token_lookahead(parser, tokenArray);
        if (!(lookaheadToken.type == TK_COMMA || lookaheadToken.type == TK_RPAR)) {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected ',' or ')' after literal");
        }
        
        (*parsedParameters)[parsedParamCount].type = convert_literal_to_datatype(parser->current_token.type);
            
        switch( (*parsedParameters)[parsedParamCount].type ) {
            case TK_KW_INT:
            case TK_KW_INT_OPT:
                gen_FunctionParamInt( gen, parser->current_token.data.Int, parser->inFunction, parsedParamCount + 1);
                break;
            case TK_KW_DOUBLE:
            case TK_KW_DOUBLE_OPT:
                gen_FunctionParamDouble( gen, parser->current_token.data.Double, parser->inFunction, parsedParamCount + 1);
                break;
            case TK_KW_STRING:
            case TK_KW_STRING_OPT:
                gen_FunctionParamString( gen, parser->current_token.data.String, parser->inFunction, parsedParamCount + 1);
                break;
            default:
                break;

        }
        parser_get_next_token(parser, tokenArray);  

    } else {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected identifier or literal");
    }

    parser->parsedParamCount++;


}


void parseAssignment(parser_t *parser, TokenArray *tokenArray, generator_t* gen){

    bool exprAssinged = false;  //help variable to chceck if assignent was done in expression

    token_t tmpToken = parser->current_token; // to hold name, for usage in updateInit

    Node *node = searchFramesVar(parser);

    if (node->symbol.isFunction){
        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Left side of the assignment is an function name, not a variable");
    }

    if (node->symbol.isLet){
        handle_error(OTHER_SEMANTIC_ERROR, parser->current_token.line, "Assigning into a Immutable variable");
    }
    if(parser->inFunction){
        for (int i = 0; i < parser->current_func->symbol.parametersCount; ++i) {
            if(strcmp(parser->current_token.data.String,parser->current_func->symbol.parameters[i].id) == 0){
                handle_error(OTHER_SEMANTIC_ERROR, parser->current_token.line, "Modifying function parameter");
            }
        }
    }
    tk_type_t leftType = node->symbol.type;
    tk_type_t foundType;
   
    parser_get_next_token(parser, tokenArray); 
    parser_get_next_token(parser, tokenArray); 

    

    token_t nextToken = token_lookahead(parser, tokenArray);

    if (parser->current_token.type == TK_IDENTIFIER && nextToken.type == TK_LPAR) {
        
        parseFunctionCall(parser, tokenArray, gen);
        foundType = parser->current_func_call->symbol.type;
        if (foundType == TK_KW_NIL){
            handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Cannot assign a void function");
        }
        gen_AssignReturnToVariable(gen,tmpToken,parser->inFunction);

    } else if ((isStartOfExpression(parser->current_token.type) && isPartOfExpression(nextToken.type))
               || (parser->current_token.type == TK_LPAR && isStartOfExpression(nextToken.type))) {
        
       
        gen_SaveExprResult( gen, tmpToken.data.String);
        foundType = rule_expression(parser, tokenArray, gen);
        gen_ClearExprResult( gen, parser->inFunction );
        if(foundType == TK_BOOLEAN){
            handle_error(SEMANTIC_TYPE_COMPATIBILITY,parser->current_token.line,"Cannot assign boolean to variable");
        }
        exprAssinged = true;
        foundType = convert_literal_to_datatype(foundType); 

    } else if (isStartOfExpression(parser->current_token.type) && !isPartOfExpression(nextToken.type)) {

        if (parser->current_token.type == TK_IDENTIFIER) {

            Node *node = searchFramesVar(parser);
            foundType = node->symbol.type; 

            if (!node->symbol.isInit){
                handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Assigning an uninitialized variable");
            }


        } else {
            
            foundType = convert_literal_to_datatype(parser->current_token.type);
        }
    } else {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR IN ASSIGNMENT STATEMENT");
    }

    if (leftType != foundType){
        handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Left and right side of assignment typ incompability");
    }

    var_updateInit(parser, tmpToken);

    if (!exprAssinged){
        char buffer[30];
        switch (parser->current_token.type){
            case(TK_INT):
                sprintf(buffer, "%lld", parser->current_token.data.Int);
                gen_AssignVal( gen, tmpToken.data.String,buffer, parser->inFunction, " int@", parser->scopeDepth);
                break;
            case(TK_DOUBLE):
                sprintf(buffer, "%a", parser->current_token.data.Double);
                gen_AssignVal( gen, tmpToken.data.String,buffer, parser->inFunction, " float@", parser->scopeDepth);
                break;
            case(TK_MLSTRING):
            case(TK_STRING):
                gen_AssignVal( gen, tmpToken.data.String, gen_convertString( parser->current_token.data.String ), parser->inFunction, " string@", parser->scopeDepth);
                break;
            case(TK_IDENTIFIER):
                    gen_AssignVal( gen, tmpToken.data.String,parser->current_token.data.String, parser->inFunction, "", parser->scopeDepth);
                break;
            default:
                break; 
        }
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