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

/*Global variable Current Token a Scanner*/
token_t CurrentToken;
scanner_t scanner;


int Program()
{
    scanner.input = stdin;
    scanner.line = 1;
    Block_Contents();
}

int Block_Contents()
{
    if ( CurrentToken.type == TK_KW_VAR || CurrentToken.type == TK_KW_LET || CurrentToken.type == TK_KW_FUNC )
    {
        Block_Content();

        Block_Contents();
    }
    else if ( CurrentToken.type == TK_EOF )
    {
        return ERROR_FREE;
    }
    else
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected keywords: 'var', 'let' or 'func'");
        return ERROR_OCCURED;
    }
}

int Block_Content()
{
    switch(CurrentToken.type)
    {
        case TK_KW_VAR:
        case TK_KW_LET:
            Def_Var();
            break;
        case TK_KW_FUNC:
            Function_Definition();
            break;
        default:
    }
}

int Def_Var()
{
    CurrentToken = get_token(&scanner);
    if ( CurrentToken.type != TK_IDENTIFIER )
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected identifier");
        printf(stderr, "ERROR: Expected identifier, got %s on line %d\n", token_type_to_string(CurrentToken.type), CurrentToken.line);
        return ERROR_OCCURED;
    }

    CurrentToken = get_token(&scanner);
    
    Type_Opt();
    
    Def_Var();
}

int Function_Definition()
{
    CurrentToken = get_token(&scanner);

    if ( CurrentToken.type != TK_IDENTIFIER )
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected identifier");
        return ERROR_OCCURED;
    }

    CurrentToken = get_token(&scanner);
    if ( CurrentToken.type != TK_LPAR )
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected '('");
        return ERROR_OCCURED;
    }

    CurrentToken = get_token(&scanner);

    Parameter_List();

    if ( CurrentToken.type != TK_RPAR )
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected ')'");
        return ERROR_OCCURED;
    }

    CurrentToken = get_token(&scanner);
    if ( CurrentToken.type != TK_ARROW )
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected '->'");
        return ERROR_OCCURED;
    }

    CurrentToken = get_token(&scanner);
    
    Return_Type();

    Function_Body();

}

int Type_Opt()
{
    if ( CurrentToken.type == TK_COLON )
    {
        CurrentToken = get_token(&scanner);
        Type();
    }
    return ERROR_FREE;
}

/*TODO zistit ake typy tam su a opravit funkciu Type()!*/

int Type()
{
    switch(CurrentToken.type)
    {
        case TK_INT:
        case TK_DOUBLE:
        case TK_STRING:
            CurrentToken = get_token(&scanner);
            break;
        default:
            handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected type");
            return ERROR_OCCURED;
    }
}

int Return_Type()
{
    switch(CurrentToken.type)
    {
        case TK_INT:
        case TK_DOUBLE:
        case TK_STRING:
        case TK_KW_NIL:
            CurrentToken = get_token(&scanner);
            return ERROR_FREE;
        default:
            handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected type or 'NIL'");
            return ERROR_OCCURED;
    }

}

int Parameter_List()
{
    if ( CurrentToken.type == TK_IDENTIFIER )
    {
        Parameter_Definition();
        Parameter_List_Tail();
    }
    else if ( CurrentToken.type == TK_RPAR )
    {
        return ERROR_FREE;
    }
    else
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected identifier or ')'");
        return ERROR_OCCURED;
    }
}

int Parameter_List_Tail()
{
    if ( CurrentToken.type == TK_COMMA )
    {
        CurrentToken = get_token(&scanner);
        Parameter_Definition();
        Parameter_List_Tail();
    }
    else if ( CurrentToken.type == TK_RPAR )
    {
        return ERROR_FREE;
    }
    else
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected ',' or ')'");
        return ERROR_OCCURED;
    }
}

int Parameter_Definition()
{
    if ( CurrentToken.type == TK_IDENTIFIER || CurrentToken.type == TK_UNDERSCORE)
    {
        CurrentToken = get_token(&scanner);
        if ( CurrentToken.type == TK_COLON )
        {
            CurrentToken = get_token(&scanner);
            Type();
        }
        else
        {
            handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected ':'");
            return ERROR_OCCURED;
        }
    } 
    else
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected identifier or '_'");
        return ERROR_OCCURED;
    }
    return ERROR_FREE;
}

int Function_Body()
{

}

