/*
IFJ PROJEKT 2023/2024
file: "parser.c"

Top-down recursive parser

authors: xpolia05
         xbencs00 - testing

*/

#include "scanner.h"
#include "error.h"
#include "parser.h"
#include "expr.h"

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
            .hasReturn = false,
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
        // should never happen...
        handle_error(INTERNAL_COMPILER_ERROR, 0, "OUT OF TOKENARRAY BOUNDS");
    }
}

void parser_get_previous_token(parser_t *parser, TokenArray *tokenArray) {

    if (parser->TKAIndex > 0) {
        parser->current_token = tokenArray->tokens[parser->TKAIndex - 2];
        parser->TKAIndex--;
    } else {
        // should never happen...
        handle_error(INTERNAL_COMPILER_ERROR, 0, "OUT OF TOKENARRAY BOUNDS");
    }
}

token_t token_lookahead(parser_t *parser, TokenArray *tokenArray) {
    if (parser->TKAIndex < tokenArray->size) {
        unsigned int next_idx = parser->TKAIndex;
        return tokenArray->tokens[next_idx];
    } else {
        // should never happen...
        handle_error(INTERNAL_COMPILER_ERROR, 0, "OUT OF TOKENARRAY BOUNDS");
        return tokenArray->tokens[parser->TKAIndex];;
    }
    //return tokenArray->tokens[parser->TKAIndex];
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


    parser_get_next_token(parser, tokenArray); // get the first token


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
            parseFunctionDefinition(parser, tokenArray, gen);
            break;
        case TK_KW_VAR:                                             // -> var Identifier <type_opt> <def_var>
        case TK_KW_LET:                                             // -> let Identifier <type_opt> <def_var>
            parseVarDefinition(parser, tokenArray, gen);
            break;
        case TK_KW_IF:                                              // -> <control_structure> - if
        case TK_KW_WHILE:                                           // -> <control_structure> - while
            parseControlStructure(parser, tokenArray, gen);
            break;
        case TK_KW_RETURN:
            if (!parser->inFunction) {
                handle_error(SYNTAX_ERROR, parser->current_token.line, "return keyword found outside of function");
            }
            parseReturn(parser, tokenArray, gen);
            break;
        case TK_IDENTIFIER:
            // TU_SEMANTIKA
            // zistit, ci moze byt start term iba nejaky expression (a + b bez priradenia, proste expr ktory nic neurobi)
            if (token_lookahead(parser, tokenArray).type == TK_LPAR) {          // <function_call>
                parseFunctionCall(parser, tokenArray, gen);
            } else if (token_lookahead(parser, tokenArray).type == TK_ASSIGN) { // <assignment>
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
    // func Identifier ( <parameter_list> ) <return_type_opt> { <function_body> }
    parser->inFunction = true;
    parser->scopeDepth++;

    parser_get_next_token(parser, tokenArray);                        // Identifier

    gen_FunctionHeader( gen, parser->current_token.data.String );

    parser->current_func = search(parser->global_frame, parser->current_token.data.String);

    setupFunctionScope(parser, parser->current_token.data.String);    // <parameter_list> Local frame setup

    while (parser->current_token.type != TK_LBRACE) {                 // skip until '{'
        parser_get_next_token(parser, tokenArray);
    }
    parser_get_next_token(parser, tokenArray);

    while(parser->current_token.type != TK_RBRACE) {
        parseBlockContent(parser, tokenArray, gen);
        parser_get_next_token(parser,tokenArray);
    }

    // CHECK CI EXISTOVAL RETURN KED SA PRESLO TELO FUNCKIE


    parser->inFunction = false;
    parser->hasReturn = false;
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
    // Check if tokenType is one of the operators that be in a complex expression after the starting tokenType
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

    check_next_token(parser, tokenArray, TK_IDENTIFIER); // Identifier
    token_t tmpToken = parser->current_token;            // ulozeny nazov premmenej
    tk_type_t foundDataType = TK_KW_NIL;                 // ulozeny typ (ak je v definicii typ)
    tk_type_t foundType = TK_KW_NIL;                     // ulozeny najdeny typ vyrazu

    gen_VarDefinition( gen, parser->current_token.data.String, parser->inFunction );

    // check ci je uz definovana
    if (search((parser->scopeDepth > 0) ? parser->local_frame->top->symbolTable : parser->global_frame, parser->current_token.data.String) != NULL){
        handle_error(SEMANTIC_UNDEFINED_FUNCTION, parser->current_token.line, "Variable redefinition in same scope");
    }

    bool hasType = false, hasInitialization = false;

    token_t lookAheadToken = token_lookahead(parser, tokenArray);

    // Check for type
    if (lookAheadToken.type == TK_COLON) {
        parser_get_next_token(parser, tokenArray); // Consume ':' token
        parser_get_next_token(parser, tokenArray); // Consume datatype token
        if (!is_token_datatype(parser->current_token.type)) {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected data type");
        }
        // Update type in symbol table
        //tmpType = parser->current_token.type;
        hasType = true;
        foundDataType = parser->current_token.type;
        lookAheadToken = token_lookahead(parser, tokenArray); // Update lookahead token for possible '='
    }

    // Check for initialization
    if (lookAheadToken.type == TK_ASSIGN) {
        parser_get_next_token(parser, tokenArray); // Consume '=' token
        parser_get_next_token(parser, tokenArray); // Consume token after '='
        hasInitialization = true;

        token_t nextToken = token_lookahead(parser, tokenArray); // Look ahead to distinguish between function call and expression

        if (parser->current_token.type == TK_IDENTIFIER && nextToken.type == TK_LPAR) {
            // Function call
            parseFunctionCall(parser, tokenArray, gen);
            foundType = parser->current_func_call->symbol.type;
            if (foundType == TK_KW_NIL){
                handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Cannot assign a void function");
            }

        } else if ((isStartOfExpression(parser->current_token.type) && isPartOfExpression(nextToken.type))
                   || (parser->current_token.type == TK_LPAR && isStartOfExpression(nextToken.type))) {
            // Expressions
            //parser_get_next_token(parser, tokenArray);
            foundType = rule_expression(parser, tokenArray, gen);
            foundType = convert_literal_to_datatype(foundType); // me no like

        } else if (isStartOfExpression(parser->current_token.type) && !isPartOfExpression(nextToken.type)) {
            // semantika,
            // treba si vytvorit pomocne funkncie ktore konvertuju literaly na typy
            // ak by to bol identifier, treba vytiahnut ten typ zo symtable, zistit ci ma prave definovana priradeny typ(hastype), ak ne tak priradit podla typu idf
            // kedze expr call vrati token po mal by sa aj tu
            // co ked nil?
            // let a = a
            if (parser->current_token.type == TK_IDENTIFIER) {
                // nieco este s INIT
                Node *node = searchFramesVar(parser);

                foundType = node->symbol.type;

                if (!node->symbol.isInit){
                    handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Assigning an uninitialized variable");
                }
            } else {
                // literal
                foundType = convert_literal_to_datatype(parser->current_token.type);
                // check ci sedi foundtype s datovym typom prave definovanej premennej alebo prirad ak nema
            }
        } else {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR IN VARIABLE DEFINITION");
        }
        /*
            TU_SEMANTIKA
            nejak zistit aky vysiel typ z 'vyraz', ci uz expression alebo fnc call
            porovnat ci sedi s lavou stranou ak bol pri lavej strane typ (hasType)
                ak nebol definovany typ -> vlozit typ do variabilnej symtable pomocou insertType()
            a pouzit updateInit(), zmeni flag ze bola premenna inicializovana
        */
    }

    // OK, mozme ulozit
    // Let a : Int
    // Let a = 10
    // Let a : Int = 10
    parser_insertVar2symtable(parser, tmpToken, isLet); // Insert var into symbol table with redefinition check
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

        char buffer[30];
        switch (parser->current_token.type){
            case(TK_INT):
                sprintf(buffer, "%lld", parser->current_token.data.Int);
                gen_AssignVal( gen, buffer, parser->inFunction, " int@" );
                break;
            case(TK_DOUBLE):
                sprintf(buffer, "%f", parser->current_token.data.Double);
                gen_AssignVal( gen, buffer, parser->inFunction, " float@" );
                break;
            case(TK_STRING):
                gen_AssignVal( gen, parser->current_token.data.String, parser->inFunction, " string@" );
                break;
            case(TK_BOOLEAN):
                gen_AssignVal( gen, parser->current_token.data.String, parser->inFunction, " bool@" );
                break;
            default:
                break; 
        }
    }
    // parser didn't get any of ':', '='
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
    //token_t lookAheadToken;

    if (parser->current_token.type == TK_KW_IF) {

        parser_get_next_token(parser, tokenArray); // Get token after 'if'

        if (parser->current_token.type == TK_KW_LET) {
            // TU_SEMANTIKA
            // dokoncit semantiku pre let id variaciu
            // rip nil 2023-2023

            // check ci je init
            check_next_token(parser, tokenArray, TK_IDENTIFIER);
            letId = searchFramesVar(parser);
            if (!letId->symbol.isLet){
                handle_error(OTHER_SEMANTIC_ERROR, parser->current_token.line, "Let id, id is not an immutable variable");
            }

        } else {
            // lookAheadToken = token_lookahead(parser, tokenArray);
            // if (!isStartOfExpression(parser->current_token.type) || !isPartOfExpression(lookAheadToken.type)){
            //     handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected expression or Let id in if statement");
            // }
            foundType = rule_expression(parser, tokenArray, gen); // Parse the conditional expression
            if (foundType != TK_BOOLEAN){
                handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Expected boolean in if statement");
            }
        }

        gen_IfThenElse( gen, parser->inFunction );

        check_next_token(parser, tokenArray, TK_LBRACE); // Check for '{'

        Node* newScope = NULL;
        push(parser->local_frame, newScope); // Push new empty scope to local frame

        parser_get_next_token(parser,tokenArray);
        while (parser->current_token.type != TK_RBRACE){
            parseBlockContent(parser, tokenArray, gen);
            parser_get_next_token(parser,tokenArray);
        }

        gen_IfDone(gen, parser->inFunction);

        //check_next_token(parser, tokenArray, TK_RBRACE); // Check for '}'

        check_next_token(parser, tokenArray, TK_KW_ELSE);

        gen_IfThenElse_End( gen, parser->inFunction );

        check_next_token(parser, tokenArray, TK_LBRACE);
        parser_get_next_token(parser,tokenArray);
        while (parser->current_token.type != TK_RBRACE){
            parseBlockContent(parser, tokenArray, gen);
            parser_get_next_token(parser,tokenArray);
        }

        //check_next_token(parser, tokenArray, TK_LBRACE); // Check for '{'

        gen_IfDone_End( gen, parser->inFunction );

        //check_next_token(parser, tokenArray, TK_RBRACE); // Check for '}'

    } else if (parser->current_token.type == TK_KW_WHILE) {
        parser_get_next_token(parser, tokenArray); // Get token after 'while'

        gen_While( gen, parser->inFunction );

        // start start ( 10
        // start part
        // if (isStartOfExpression(parser->current_token.type) || !isPartOfExpression(lookAheadToken.type)){
        //     handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected expression while statement");
        // }
        foundType = rule_expression(parser, tokenArray, gen); // Parse the conditional expression
        if (foundType != TK_BOOLEAN){
            handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Expected boolean in while statement");
        }

        gen_WhileCond( gen, parser->inFunction );

        check_next_token(parser, tokenArray, TK_LBRACE); // Check for '{'

        Node* newScope = NULL;
        push(parser->local_frame, newScope); // Push new empty scope to local frame

        parser_get_next_token(parser,tokenArray);
        while (parser->current_token.type != TK_RBRACE){
            parseBlockContent(parser, tokenArray, gen);
            parser_get_next_token(parser,tokenArray);
        }

        gen_WhileEnd( gen, parser->inFunction );
        //check_next_token(parser, tokenArray, TK_RBRACE); // Check for '}'

    } else {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected 'if' or 'while'"); // This should not happen
    }
    pop(parser->local_frame);
    parser->scopeDepth--;
}


void parseReturn(parser_t *parser, TokenArray *tokenArray, generator_t* gen) {

    parser->hasReturn = true;

    token_t lookAheadToken = token_lookahead(parser, tokenArray);

    if (parser->current_func->symbol.type == TK_KW_NIL){
        if (isStartOfExpression(lookAheadToken.type) && !lookAheadToken.eol_before){
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Return in void function cannot have expression");
        }
    } else {
        parser_get_next_token(parser, tokenArray); // Consume the lookAhead token (we must have some kind of expression)

        token_t nextToken = token_lookahead(parser, tokenArray);

        tk_type_t foundType;

        if (parser->current_token.type == TK_IDENTIFIER && nextToken.type == TK_LPAR) {
            // function call
            handle_error(OTHER_SEMANTIC_ERROR, parser->current_token.line, "Return expression cannot be function call!");

        } else if ((isStartOfExpression(parser->current_token.type) && isPartOfExpression(nextToken.type))
                   || (parser->current_token.type == TK_LPAR && isStartOfExpression(nextToken.type))) {
            // Expressions
            //parser_get_next_token(parser, tokenArray);
            //Poznamka od Duriho: V rule_expression je gen_Expr, ktora vysledok automaticky ulozi na stack, cize sa nemusi pisat prikaz PUSHS "nejaka premenna"
            foundType = rule_expression(parser, tokenArray, gen);
            foundType = convert_literal_to_datatype(foundType); // me no like

        } else if (isStartOfExpression(parser->current_token.type) && !isPartOfExpression(nextToken.type)) {
            if (parser->current_token.type == TK_IDENTIFIER) {
                // nieco este s INIT
                Node *node = searchFramesVar(parser);

                foundType = node->symbol.type;

                if (!node->symbol.isInit){
                    handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Returning an uninitialized variable");
                }

            } else {
                // literal
                foundType = convert_literal_to_datatype(parser->current_token.type);
                // check ci sedi foundtype s datovym typom prave definovanej premennej alebo prirad ak nema
            }
        } else {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected expression after return statement in non-void function");
        }

        if (parser->current_func->symbol.type != foundType){
            handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Incompatible expression type of return with type of function");
        }

    }
}

/* ||||||||||||||||||||||||||||||||| FINSH PARSEFUNCTIONCALL ||||||||||||||||||||||||||||||||*/

void parseFunctionCall(parser_t *parser, TokenArray *tokenArray, generator_t* gen) {

    token_t funcToken = parser->current_token; // udrzi nazov funkcie

    parser_get_next_token(parser, tokenArray); // consume '('

    Parameter *parsedParameters = NULL;
    //int *parsedParamCount = 0;
    //token_t *tmpToken; // udrzi nazov

    parseFunctionCallParams(parser, tokenArray, &parsedParameters, gen);

    int parsedParamCount = parser->parsedParamCount;
    //check_next_token(parser, tokenArray, TK_RPAR); uz by mal byt consumed z parseFunctionCallParams

    // checkni existenciu funkcie (nezabudni na isFunction)
    Node* functionNode = search(parser->global_frame, funcToken.data.String);
    if (functionNode == NULL || !functionNode->symbol.isFunction) {
        handle_error(SEMANTIC_UNDEFINED_FUNCTION, parser->current_token.line, "Undefined function call");
        return;
    }
    // checkni parameter count ( ak write tak necheckuj ) - chyba 4
    if (functionNode->symbol.parametersCount != parsedParamCount){
        if (strcmp(funcToken.data.String, "write") != 0){
            handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Wrong number of parameters in function call");
            return;
        }
    }
    // checkni parametre - ak zle chyba 4
    // checknut ci je v definovanej aj meno parametru, ak nie je '_', musi byt pritomny vo volani
    /*Při použití _ se jméno parametru při volání vynechává a zapisuje se pouze
        term parametru. Identifikátor parametru slouží jako identifikátor v těle funkce pro
        získání hodnoty tohoto parametru. Při použití _ jako identifikátor parametru se tento
        parametr v těle funkce nepoužívá*/

    // func foo(x: Int)
    // foo(with : 10)
    // func(10)
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
                }
            }

            if (parsedParameters[i].name) {
                // checknut ci je v definicii _, ked je tak error
                if (strcmp(functionNode->symbol.parameters[i].name, "_") == 0){
                    handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Called parameter with a name when it does not have a name");
                }
                // ked tam je nieco ine, porovnat pomocou strcmp ci sedia - ak nie tak error
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
            }
            if (parsedParameters[i].type) {
                // checknut typ s definiciou funkcie
                if (functionNode->symbol.parameters[i].type != parsedParameters[i].type){
                    handle_error(SEMANTIC_FUNCTION_ARGUMENTS, parser->current_token.line, "Wrong type in function call");
                }
            }
        }
    }


    // NASTAV PARSER->CURRENT_FUNC_CALL NA CHECKNUTIE TYPU PRI ASSIGNMENT
    parser->current_func_call = functionNode;
    parser->parsedParamCount = 0;
}

void parseFunctionCallParams(parser_t *parser, TokenArray *tokenArray, Parameter **parsedParameters, generator_t* gen){

    parser_get_next_token(parser, tokenArray);

    // Check for empty parameter list
    if (parser->current_token.type == TK_RPAR) {
        return;  // No parameters to process
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
    // su len dve moznosti - bud sa najprv najde identifier alebo literal (tk_int, tk_double, tk_string, tk_mlstring) - inak syntax error
    // TOK 1:
    // najde sa identifier
    //1. moznost
    // pozri ci je dalsi token ':' pomocou lookahead - ak dalsi token nie je ':', ',' alebo ')', vyhod syntax error
    // ak je, uloz z token.data.String do parameters pod symbol.name
    // teraz ocakavas dva pripady - ked tam bude nieco ine ako tk_identifier alebo literal, tak syntax error
    // 1. pripad - pride TK_IDENTIFIER
    // musi byt premenna, uloz pod symbol.id
    // 2. pripad - pride literal
    // konvertuj pomocou convert_literal_to_datatype, uloz pod type
    //2. moznost
    // ak nie je za nim token ':', nachadza sa v nom nejaka premenna(neskor check ci je vobec valid)
    // ulozime pod symbol.id, aby sa potom mohla spatne backtracknut v semantike
    // TOK 2:
    // najde sa literal
    // pouzi func na konverziu podla token.type (tk_int -> tk_kw_int)
    // uloz do parametra len typ, bez name a id
    // pozri dalsi token lookaheadom, ak tam nie je bud ',' alebo ')', tak syntax error

    // Allocate or reallocate memory for storing parameters
    //printf("count %d\n",((*parsedParamCount)+1));
    int parsedParamCount = parser->parsedParamCount;
    token_t lookaheadToken;

    *parsedParameters = realloc((*parsedParameters), ((parsedParamCount) + 1) * sizeof(Parameter));

    if (*parsedParameters == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, parser->current_token.line, "Memory allocation error");
        return;
    }
    (*parsedParameters)[parsedParamCount].name = NULL;
    (*parsedParameters)[parsedParamCount].id = NULL;

    // Token 1: Identifier found
    if (parser->current_token.type == TK_IDENTIFIER) {
        lookaheadToken = token_lookahead(parser, tokenArray);

        // Check next token
        if (lookaheadToken.type == TK_COLON) {
            // Store identifier under symbol.name
            (*parsedParameters)[parsedParamCount].name = strdup(parser->current_token.data.String);

            parser_get_next_token(parser, tokenArray);  // Consume ':' and move to next token
            parser_get_next_token(parser, tokenArray);

            if (parser->current_token.type == TK_IDENTIFIER) {
                // Store next identifier under symbol.id
                (*parsedParameters)[parsedParamCount].id = strdup(parser->current_token.data.String);
                parser_get_next_token(parser,tokenArray);
            } else if (is_token_literal(parser->current_token.type)) {
                // Convert literal and store under type
                (*parsedParameters)[parsedParamCount].type = convert_literal_to_datatype(parser->current_token.type);
                parser_get_next_token(parser,tokenArray);
            } else {
                handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected a variablei identifier or literal after ':'");
            }
        } else if (lookaheadToken.type == TK_COMMA || lookaheadToken.type == TK_RPAR) {
            // Store identifier under symbol.id

            (*parsedParameters)[parsedParamCount].id = strdup(parser->current_token.data.String);
            parser_get_next_token(parser, tokenArray);
        } else {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected ':', ',' or ')' after identifier");
        }
    }
        // Token 2: Literal found
    else if (is_token_literal(parser->current_token.type)) {
        lookaheadToken = token_lookahead(parser, tokenArray);
        if (!(lookaheadToken.type == TK_COMMA || lookaheadToken.type == TK_RPAR)) {
            handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected ',' or ')' after literal");
        }
        // Convert literal to its corresponding datatype
        (*parsedParameters)[parsedParamCount].type = convert_literal_to_datatype(parser->current_token.type);

        parser_get_next_token(parser, tokenArray);  // consume comma or lpar

        // Check for ',' or ')' after literal
        // if (!(parser->current_token.type == TK_COMMA || parser->current_token.type == TK_RPAR)) {
        //     handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected ',' or ')' after literal");
        // }
    } else {
        handle_error(SYNTAX_ERROR, parser->current_token.line, "Expected identifier or literal");
    }

    // Increment the parameter count
    //printf("param 1 %s\n",parsedParameters[*parsedParamCount]->id);
    parser->parsedParamCount++;

    //parser_get_next_token(parser,tokenArray);

}


void parseAssignment(parser_t *parser, TokenArray *tokenArray, generator_t* gen){
    /*
    Sémantika příkazu je následující: Příkaz provádí vyhodnocení výrazu výraz (viz kapitola 5) a přiřazení jeho hodnoty do levého operandu id,
    který je modifikovatelnou proměnnou (tj. definovanou pomocí klíčového slova var).
    Pokud není hodnota výrazu typově kompatibilní s typem proměnné id, dojde k chybě 7.
    */

    // !!! vratit sa do vardef a return a spravit check na init
    // !!! pozor ci to nie je nazov funkcie

    // prvy check, existuje lava strana?
    // ak existuje, je to let? -> chyba asi 9?
    // dalsi check: porovnat lavu s pravou stranou
    // dalsi check: bacha na initialization pravej strany
    // spravit updateinit ak nebola lava strana init

    bool exprAssinged = false;  //help variable to chceck if assignent was done in expression

    token_t tmpToken = parser->current_token; // should hold name of variable, for usage in updateInit

    Node *node = searchFramesVar(parser);

    if (node->symbol.isFunction){
        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Left side of the assignment is an function name, not a variable");
    }

    if (node->symbol.isLet){
        handle_error(OTHER_SEMANTIC_ERROR, parser->current_token.line, "Assigning into a Immutable variable");
    }

    tk_type_t leftType = node->symbol.type;
    tk_type_t foundType;
    //bool isInit = node->symbol.isInit; // needed?

    // left hand check done
    // right hand check
    parser_get_next_token(parser, tokenArray); // consume '='
    parser_get_next_token(parser, tokenArray); // consume token after '='

    

    token_t nextToken = token_lookahead(parser, tokenArray);

    if (parser->current_token.type == TK_IDENTIFIER && nextToken.type == TK_LPAR) {
        // Function call
        parseFunctionCall(parser, tokenArray, gen);
        foundType = parser->current_func_call->symbol.type;
        if (foundType == TK_KW_NIL){
            handle_error(SEMANTIC_TYPE_COMPATIBILITY, parser->current_token.line, "Cannot assign a void function");
        }

    } else if ((isStartOfExpression(parser->current_token.type) && isPartOfExpression(nextToken.type))
               || (parser->current_token.type == TK_LPAR && isStartOfExpression(nextToken.type))) {
        // Expressions
        //parser_get_next_token(parser, tokenArray);
        foundType = rule_expression(parser, tokenArray, gen);
        exprAssinged = true;
        foundType = convert_literal_to_datatype(foundType); // me no like

    } else if (isStartOfExpression(parser->current_token.type) && !isPartOfExpression(nextToken.type)) {

        if (parser->current_token.type == TK_IDENTIFIER) {

            Node *node = searchFramesVar(parser);
            foundType = node->symbol.type; // should deal with if function

            if (!node->symbol.isInit){
                handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "Assigning an uninitialized variable");
            }


        } else {
            // literal
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
                gen_AssignVal( gen, buffer, parser->inFunction, " int@" );
                break;
            case(TK_DOUBLE):
                sprintf(buffer, "%f", parser->current_token.data.Double);
                gen_AssignVal( gen, buffer, parser->inFunction, " float@" );
                break;
            case(TK_STRING):
                gen_AssignVal( gen, parser->current_token.data.String, parser->inFunction, " string@" );
                break;
            case(TK_BOOLEAN):
                gen_AssignVal( gen, parser->current_token.data.String, parser->inFunction, " bool@" );
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