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

/*Global variable Current Token and Scanner*/
token_t CurrentToken;
scanner_t scanner;


bool Check_Token_Type ( tk_type_t type )
{
        return CurrentToken.type == type;
}

bool Check_Next_Token_Type ( tk_type_t type, char* errMsg )
{
    Next_Token();
    // return CurrentToken.type == type;
    if ( CurrentToken.type == type )
    {
        return true;
    }
    else
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, errMsg);
    }
}

void Next_Token()
{
    CurrentToken = get_token(&scanner);
}

int Program()
{
    scanner.input = stdin;
    scanner.line = 1;
    Block_Contents();
}

int Block_Contents()
{
    if ( Check_Token_Type( TK_KW_VAR ) || Check_Token_Type( TK_KW_LET ) || Check_Token_Type( TK_KW_FUNC ) )//CurrentToken.type == TK_KW_VAR || CurrentToken.type == TK_KW_LET || CurrentToken.type == TK_KW_FUNC )
    {
        Block_Content();

        Block_Contents();
    }
    else if ( Check_Token_Type( TK_EOF ) )
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
            Next_Token();
            if ( Check_Token_Type( TK_IDENTIFIER ) )
            {
                Type_Opt();
                Def_Var();
            }
            else
            {
                handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected identifier");
                return ERROR_OCCURED;
            }
            break;
        case TK_KW_FUNC:
            Next_Token();
            Function_Definition();
            break;
        default:
    }
}

int Def_Var()
{
    Next_Token();
    if ( Check_Token_Type ( TK_ASSIGN ) )
    {
        Next_Token();
        Def_Value();
        return ERROR_FREE;
    }
}

int Function_Definition()
{
    Check_Next_Token_Type( TK_IDENTIFIER , "Expected identifier" );
    Check_Next_Token_Type( TK_LPAR , "Expected '('" );
    Next_Token();
    Parameter_List();
    Check_Next_Token_Type( TK_RPAR, "Expected ')'" );
    Next_Token();
    if ( Check_Token_Type( TK_ARROW ) )
    {
        Next_Token();
        Return_Type();
    }
    Function_Body();
    return ERROR_FREE;
}

int Type_Opt()
{
    Next_Token();
    if ( Check_token_Type( TK_COLON ) )
    {
        Next_Token();
        Type();
    }
    return ERROR_FREE;
}

int Type()
{
    switch(CurrentToken.type)
    {
        case TK_KW_INT:
        case TK_KW_DOUBLE:
        case TK_KW_STRING:
        case TK_KW_INT_OPT:
        case TK_KW_DOUBLE_OPT:
        case TK_KW_STRING_OPT:
            break;
        default:
            handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected type");
            return ERROR_OCCURED;
    }
    return ERROR_FREE;
}

int Return_Type()
{
    Next_Token();
    Type();
}

int Parameter_List()
{
    Next_Token();
    Parameter_Definition();
    Next_Token();
    Parameter_List_Tail();
}

int Parameter_List_Tail()
{   
    if ( Check_Token_Type( TK_COMMA ) )
    {
        Next_Token();
        Parameter_Definition();
        Next_Token();
        Parameter_List_Tail();
    }
    return ERROR_FREE;
}

int Parameter_Definition()
{
    if ( Check_Token_Type ( TK_IDENTIFIER ) || Check_Token_Type ( TK_UNDERSCORE ) )
    {
        Check_Next_Token_Type( TK_COLON, "Expected ':'" );
        Next_Token();
        Type();
        return ERROR_FREE;
    } 
    else
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected identifier or '_'");
        return ERROR_OCCURED;
    }

}

int Def_Value()
{
    /*TODO: Zistit ci mam zavolat expression alebo function call*/
    Next_Token();
    Function_Call();
}

int Function_Body()
{
    if ( Check_Token_Type( TK_LBRACE ) )
    {
        Next_Token();
        Statement_List();
        Check_Next_Token_Type( TK_RBRACE, "Expected '}'" );
        return ERROR_FREE;
    }
    else
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected '{'");
        return ERROR_OCCURED;
    }
}

int Statement_List()
{
    Statement();
    Next_Token();
    Statement_List();
}

int Statement()
{
    switch( CurrentToken.type )
    {
        case TK_KW_IF:
            Next_Token();
            if ( Check_Token_Type( TK_KW_LET ) )
            {
                Check_Next_Token_Type( TK_IDENTIFIER, "Expected identifier" );
                Check_Next_Token_Type( TK_LBRACE, "Expected '{'" );
                Next_Token();
                Statement_List();
                Check_Next_Token_Type( TK_RBRACE, "Expected '}'" );
                Check_Next_Token_Type( TK_KW_ELSE, "Expected 'else'" );
                Check_Next_Token_Type( TK_LBRACE, "Expected '{'" );
                Next_Token();
                Statement_List();
                Check_Next_Token_Type( TK_RBRACE, "Expected '}'" );
            }
            else
            {
                //TODO: evaluate expression
                Check_Next_Token_Type( TK_LBRACE, "Expected '{'" );
                Next_Token();
                Statement_List();
                Check_Next_Token_Type( TK_RBRACE, "Expected '}'" );
                Check_Next_Token_Type( TK_KW_ELSE, "Expected 'else'" );
                Check_Next_Token_Type( TK_LBRACE, "Expected '{'" );
                Next_Token();
                Statement_List();
                Check_Next_Token_Type( TK_RBRACE, "Expected '}'" );
            }
            break;
        case TK_KW_WHILE:
            //TODO: evaluate expression
            Check_Next_Token_Type( TK_LBRACE, "Expected '{'" );
            Next_Token();
            Statement_List();
            Check_Next_Token_Type( TK_RBRACE, "Expected '}'" );
            break;
        case TK_IDENTIFIER:
            Check_Next_Token_Type( TK_ASSIGN, "Expected '='" );
            Next_Token();
            Function_Call();
            break;
        case TK_KW_VAR:
        case TK_KW_LET:
            Check_Next_Token_Type( TK_IDENTIFIER, "Expected identifier" );
            Next_Token();
            Type_Opt();
            Next_Token();
            Def_Var();
            break;
        case TK_KW_RETURN:
            //TODO: Dorobit gramatiku na return (Funckia moze byt void a prikaz return tam vobec nemusi byt)
            break;
    }
}

int Function_Call()
{
    if ( Check_Token_Type( TK_IDENTIFIER ) )
    {
        Check_Next_Token_Type( TK_LPAR, "Expected '('");
        Next_Token();
        Input_Parameters();
        Check_Next_Token_Type( TK_RPAR, "Expected ')'");
    }
    else
    {
        handle_error(SYNTAX_ERROR, CurrentToken.line, "Expected the name of defined function");
        return ERROR_OCCURED;
    }
    return ERROR_FREE;
}

int Input_Parameters()
{
    Next_Token();
    Term();
    Next_Token();
    Input_Parameters_Tail();
}

int Input_Parameter_Tail()
{
    if ( Check_Token_Type( TK_COMMA ) )
    {
        Next_Token();
        Term();
        Next_Token();
        Input_Parameters_Tail();
    }
    return ERROR_FREE;
}

int Term()
{
    //TODO: zistit, ci je token ID alebo ci to je expression
}

