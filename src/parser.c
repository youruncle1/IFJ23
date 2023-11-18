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

void addToken(TokenArray *array, token_t token) {
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->tokens = realloc(array->tokens, array->capacity * sizeof(token_t));
        // TODO CHECK IF MEMORY REALLOC FAULT
    }
    array->tokens[array->size++] = token;
}

void firstParserPass(parser_t *parser, TokenArray *tokenArray) {
    
    token_t token = parser->current_token;
    while ((token = get_token(parser->scanner)).type != TK_EOF) {
        addToken(tokenArray, token); // Add token to array
        if (token.type == TK_KW_FUNC) {
            parseFunctionDefinition(parser, tokenArray);
        }
    }
    addToken(tokenArray, token); // add EOF token to the end of array
}

void parseFunctionDefinition(parser_t *parser, TokenArray *tokenArray) {
    // <function_definition> -> func Identifier ( <parameter_list> ) <return_type_opt> ||||| <function_body>
    
    check_next_token_and_add(tokenArray, parser, TK_IDENTIFIER);
    
    // pridat nazov do stromu
    
    check_next_token_and_add(tokenArray, parser, TK_LPAR);

    parseFunctionParameters(parser, tokenArray);

    check_next_token_and_add(tokenArray, parser, TK_RPAR);

    parser->current_token = get_token(parser->scanner);
    if(check_token_type(parser, TK_RBRACE)){
        // <return_type_opt> -> ε 
        //ulozit return type ako NULL
        addToken(tokenArray, parser->current_token);
    } else if (check_token_type(parser, TK_ARROW)){
        // <return_type_opt> -> -> <return_type>
        addToken(tokenArray, parser->current_token);
        // ziskat dalsi token porovnat ci je token datoveho typu
        parser->current_token = get_token(parser->scanner);
        is_token_datatype(parser->current_token.type);
        // ulozit return type

        
    } else {
        // token nepatri do gramatiky
        handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR");
    }    
    
}

void parseFunctionParameters(parser_t *parser, TokenArray *tokenArray){
    //             <parameter_list> -> <parameter_definition> <parameter_list_tail>
    // <parameter_list> -> ε
    // <parameter_list_tail> -> , <parameter_definition> <parameter_list_tail>
    // <parameter_list_tail> -> ε
    // <parameter_definition> -> Identifier : <type>
    // <parameter_definition> -> _ : <type>  

    // tu pridavat parametre do stromu
}

/* retrieves token from scanner, checks if it matches expected token type, adds it to TokenArray */
void check_next_token_and_add(TokenArray *tokenArray, parser_t *parser, tk_type_t expectedType){

    parser->current_token = get_token(parser->scanner);

    if (!check_token_type(parser, expectedType)){
        handle_error(SYNTAX_ERROR, parser->current_token.line, "SYNTAX ERROR IN FUNC DEFINITION");
    }
    addToken(tokenArray, parser->current_token);
}

parser_t initParser(scanner_t *scanner) {
    parser_t parser = {.scanner = scanner,
                       .local_frame = initStack(), 
                       .global_frame =  NULL,
                       .TKAIndex = 0         
                       };

    return parser;
}

void next_token(parser_t *parser) {
    parser->current_token = get_token(parser->scanner);
}

bool check_token_type(parser_t *parser, tk_type_t expected_type) {
    return parser->current_token.type == expected_type;
}

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


void Next_Token_From_Array( parser_t *parser, TokenArray *array )
{
    parser->current_token = array->tokens[parser->TKAIndex];
    parser->TKAIndex++;
}

bool Check_Token_Type_From_Array( tk_type_t expected_token_type )
{
    return parser->current_token.type == expected_token_type;
}

bool Check_Next_Token_From_Array( tk_type_t expected_token_type, char* err_msg )
{
    Next_Token_From_Array( parser_t *parser, char* err_msg );
    
    if ( parser->current_token.type == expected_token_type )
    {
        return true;
    }
 
    else
    
    {
        handle_error( SYNTAX_ERROR)
       }

void Program(parser_t *parser) {

}



void Block_Contents(parser_t *parser) {
    // <block_contents> -> <block_content> <block_contents>
    Block_Content(parser);
    Block_Contents(parser);
}

void Block_Content(parser_t *parser) {
    // <block_content> -> var Identifier <type_opt> <def_var>
    // <block_content> -> let Identifier <type_opt> <def_var>
    // <block_content> -> <function_definition>
    // <block_content> -> <function_call>
    switch (parser->current_token.type) {
        case TK_KW_VAR:
        case TK_KW_LET:
           
            break;
        case TK_KW_FUNC:
            Check_Next_Token_From_Array( parser, TK_IDENTIFIER, "EXPECTED AN IDENTIFIER" );
            Type_Opt( parser );
            Def_Var( parser ); 
            break;
        case TK_IDENTIFIER:
}
  Function_Definition ()pa()pa parser ;

            break;
        case TK_IDENTIFIER:
            Function_Call( parser );
void Def_Var( parser )
{

}

    void Function_Definition()
{   /*
    func Identifier ( <parameter_list> ) <return_type_opt> <function_body>
    Token: 'func' sa zjedol pred volanim funkcie 
    */

}

void Type_Opt()
{

}

void Type()
{

}

void Return_Type()
{

}

void Return_Type_Opt()
{

}

void Parameter_List()
{

}

void Parameter_List_Tail()
{   

}

void Parameter_Definition()
{


}

void Def_Value()
{

}

void Function_Body()
{

}

void Statement_List()
{

}

void Statement()
{

}

void Function_Call()
{

}

void Input_Parameters()
{

}

void Input_Parameter_Tail()
{

}

void Term()
{
    
}

