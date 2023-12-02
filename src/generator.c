/*
IFJ PROJEKT 2023/2024
file: "generator.c"

Code generator

authors: xrusna08 

*/

#include <stdlib.h>
#include <string.h>
#include "generator.h"
#include "expr.h"

generator_t gen_Init(){
    generator_t gen;
    gen.functions = init_Tape();
    gen.mainBody = init_Tape();
    gen.header = init_Tape();
    gen.functionHead = init_Tape();
    gen.functionBody = init_Tape();
    gen.functionFoot = init_Tape();
    gen.functionName = init_Tape();
    gen.varName = init_Tape();
    gen.stringParam = init_Tape();
    gen.selectCount = 0;
    gen.iterCount = 0;
    return gen;
}

void gen_Header( generator_t* gen ) {
    add_Instruction(&gen->header, ".IFJcode23\n"                    //every IFJcode23 needs to start with .IFJcode23 header
                                  "DEFVAR GF@&bool\n"               //
                                  "DEFVAR GF@&tmp1\n"               //Global variables
                                  "DEFVAR GF@&tmp2\n"               //
                                  "DEFVAR GF@&tmp3\n"               //
                                  );

}

void gen_inbuild( generator_t* gen ) {
    gen_buildin_readInt(gen);
    gen_buildin_readString(gen);
    gen_buildin_readDouble(gen);
    gen_buildin_write(gen);
    gen_buildin_Double2Int(gen);
    gen_buildin_Int2Double(gen);
    gen_buildin_substring(gen);
    gen_buildin_chr(gen);
    gen_buildin_ord(gen);
    gen_buildin_length(gen);
    gen_COALESCE(gen);
    gen_LE(gen);
    gen_GE(gen);
}

void gen_VarDefinition( generator_t* gen, char* name, bool inFunc ) {
    //clear_Tape(&gen->varName);
    add_Instruction( &gen->varName, name );
    //Is token in global or local frame
    if( inFunc ) {
        add_Instruction( &gen->functionBody, "DEFVAR LF@" );
        add_Instruction( &gen->functionBody, name );      //add the variable name to definition
        add_newLine( &gen->functionBody );                                             //append new line
    } else {
        add_Instruction( &gen->mainBody, "DEFVAR GF@" );
        add_Instruction( &gen->mainBody, name );       //add the variable name to definition
        add_newLine( &gen->mainBody ); //append new line
    }
}

void gen_AssignVal( generator_t* gen, char* varName, char* val, bool inFunc, char* type ) {
    //clear_Tape(&gen->varName);
    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "MOVE LF@" );
        add_Instruction( &gen->functionBody, varName );
        add_Instruction( &gen->functionBody, type );
        add_Instruction( &gen->functionBody, val);
        add_newLine( &gen->functionBody);
    } else {
        add_Instruction( &gen->mainBody, "MOVE GF@" );
        add_Instruction( &gen->mainBody, varName );
        add_Instruction( &gen->mainBody, type );
        add_Instruction( &gen->mainBody, val);
        add_newLine( &gen->mainBody );
    }
    //clear_Tape( &gen->varName );
}

/*
JUMP _skip_functionName
LABEL _functionName
CREATEFRAME
PUSHFRAME

function body

POPFRAME
RETURN
LABEL _skip_functionName
*/


void gen_FunctionHeader( generator_t* gen, char* funcName,Node* function) {

    add_Instruction( &gen->functionName, funcName );

    add_Instruction( &gen->functionHead, "JUMP _skip_" );
    add_Instruction( &gen->functionHead, funcName );  //name of function
    add_newLine( &gen->functionHead );
    add_Instruction( &gen->functionHead, "LABEL ");
    add_Instruction( &gen->functionHead, funcName );  //name of function
    add_newLine( &gen->functionHead );
    add_Instruction( &gen->functionHead, "PUSHFRAME\n");

    //Preklad predavanych parametru na parametry z hlavicky
    for (int i = 0; i < function->symbol.parametersCount; i++) {
        add_Instruction( &gen->functionHead, "DEFVAR LF@" );
        add_Instruction( &gen->functionHead, function->symbol.parameters[i].id );
        add_newLine( &gen->functionHead );
        add_Instruction( &gen->functionHead, "MOVE LF@" );
        add_Instruction( &gen->functionHead, function->symbol.parameters[i].id );
        add_Instruction( &gen->functionHead, " LF@%" );
        add_Int(&gen->functionHead,i+1);
        add_newLine( &gen->functionHead );
    }

    //Pomocna promenna pro return
    add_Instruction( &gen->functionHead, "DEFVAR LF@%retval\n");
    clear_Tape(&gen->varName);

}

void gen_IdentifierReturn(generator_t* gen,token_t token){
    add_Instruction( &gen->functionFoot, "MOVE LF@%retval LF@");
    add_Instruction(&gen->functionFoot,token.data.String);
    add_newLine(&gen->functionFoot);
}

void gen_LiteralReturn(generator_t* gen,token_t token){
    if(token.type == TK_INT){
        add_Instruction( &gen->functionFoot, "MOVE LF@%retval int@");
        add_Int(&gen->functionFoot,token.data.Int);
        add_newLine(&gen->functionFoot);
    }
    if(token.type == TK_DOUBLE){
        add_Instruction( &gen->functionFoot, "MOVE LF@%retval float@");
        add_Double(&gen->functionFoot,token.data.Double);
        add_newLine(&gen->functionFoot);
    }
    if(token.type == TK_STRING){
        add_Instruction( &gen->functionFoot, "MOVE LF@%retval string@");
        add_Instruction(&gen->functionFoot,token.data.String);
        add_newLine(&gen->functionFoot);
    }
}

void gen_AssignReturnToVariable(generator_t* gen,token_t tokenToAssign,bool inFunc){
    if(inFunc){
        add_Instruction( &gen->functionBody, "MOVE LF@%");
        add_Instruction( &gen->functionBody, tokenToAssign.data.String);
        add_Instruction( &gen->functionBody, " TF@%retval");
        add_newLine(&gen->functionBody);
    }else{
        add_Instruction( &gen->mainBody, "MOVE GF@");
        add_Instruction( &gen->mainBody, tokenToAssign.data.String);
        add_Instruction( &gen->mainBody, " TF@%retval");
        add_newLine(&gen->mainBody);
    }
}
void gen_FunctionFooter( generator_t* gen) {

    add_Instruction( &gen->functionFoot, "POPFRAME\n"
                                         "RETURN\n"
                                         );

    add_Instruction( &gen->functionFoot, "LABEL _skip_" );
    add_Instruction( &gen->functionFoot, gen->functionName.data );  //name of function
    add_newLine( &gen->functionFoot );
    gen_Function(gen);
    clear_Tape(&gen->functionHead);
    clear_Tape(&gen->functionBody);
    clear_Tape(&gen->functionFoot);
    clear_Tape(&gen->functionName);
}

void gen_FunctionCall( generator_t* gen, char* funcName, bool inFunc ) {
    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "CALL ");
        add_Instruction( &gen->functionBody, funcName );  //name of called function
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "CALL ");
        add_Instruction( &gen->mainBody, funcName );
        add_newLine( &gen->mainBody );
    }
}

void gen_FunctionParam( generator_t* gen, char* param, bool inFunc ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "PUSHS LF@ " );
        add_Instruction( &gen->functionBody, param );
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "PUSHS GF@ " );
        add_Instruction( &gen->mainBody, param );
        add_newLine( &gen->mainBody );
    }
}

void gen_CreateFrame( generator_t* gen, bool inFunc ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "CREATEFRAME" );
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "CREATEFRAME" );
        add_newLine( &gen->mainBody );
    }
}

void gen_FunctionParamInt( generator_t* gen, long val, bool inFunc, int paramCount) {

    /*if ( inFunc ) {
        add_Instruction( &gen->functionBody, "PUSHS int@" );
        add_Int( &gen->functionBody, val );
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "PUSHS int@" );
        add_Int( &gen->mainBody, val );
        add_newLine( &gen->mainBody );
    }*/
    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "DEFVAR TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_newLine( &gen->functionBody );
        add_Instruction( &gen->functionBody, "MOVE TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_Instruction( &gen->functionBody, " int@" );
        add_Int( &gen->functionBody, val );
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "DEFVAR TF@%" );
        add_Int( &gen->mainBody, paramCount );
        add_newLine( &gen->mainBody );
        add_Instruction( &gen->mainBody, "MOVE TF@%" );
        add_Int( &gen->mainBody, paramCount );
        add_Instruction( &gen->mainBody, " int@" );
        add_Int( &gen->mainBody, val );
        add_newLine( &gen->mainBody );
    }
    //printf("func %s\n",gen->functionName.data);

}

void gen_FunctionParamDouble( generator_t* gen, double val, bool inFunc, int paramCount) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "DEFVAR TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_newLine( &gen->functionBody );
        add_Instruction( &gen->functionBody, "MOVE TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_Instruction( &gen->functionBody, " float@" );
        add_Double( &gen->functionBody, val );
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "DEFVAR TF@%" );
        add_Int( &gen->mainBody, paramCount );
        add_newLine( &gen->mainBody );
        add_Instruction( &gen->mainBody, "MOVE TF@%" );
        add_Int( &gen->mainBody, paramCount );
        add_Instruction( &gen->mainBody, " float@" );
        add_Double( &gen->mainBody, val );
        add_newLine( &gen->mainBody );
    }
}

void gen_FunctionParamString( generator_t* gen, char* str, bool inFunc, int paramCount ) {

    unsigned int length = strlen(str);

    for ( unsigned int i = 0; i < length; i++ ) {
        switch(str[i]) {
            case 0:
                add_Instruction( &gen->stringParam, "\\000" );
                break;
            case 1:
                add_Instruction( &gen->stringParam, "\\001" );
                break;
            case 2:
                add_Instruction( &gen->stringParam, "\\002" );
                break;
            case 3:
                add_Instruction( &gen->stringParam, "\\003" );
                break;
            case 4:
                add_Instruction( &gen->stringParam, "\\004" );
                break;
            case 5:
                add_Instruction( &gen->stringParam, "\\005" );
                break;
            case 6:
                add_Instruction( &gen->stringParam, "\\006" );
                break;
            case 7:
                add_Instruction( &gen->stringParam, "\\007" );
                break;
            case 8:
                add_Instruction( &gen->stringParam, "\\008" );
                break;
            case 9:
                add_Instruction( &gen->stringParam, "\\009" );
                break;
            case 10:
                add_Instruction( &gen->stringParam, "\\010" );
                break;
            case 11:
                add_Instruction( &gen->stringParam, "\\011" );
                break;
            case 12:
                add_Instruction( &gen->stringParam, "\\012" );
                break;
            case 13:
                add_Instruction( &gen->stringParam, "\\013" );
                break;
            case 14:
                add_Instruction( &gen->stringParam, "\\014" );
                break;
            case 15:
                add_Instruction( &gen->stringParam, "\\015" );
                break;
            case 16:
                add_Instruction( &gen->stringParam, "\\016" );
                break;
            case 17:
                add_Instruction( &gen->stringParam, "\\017" );
                break;
            case 18:
                add_Instruction( &gen->stringParam, "\\018" );
                break;
            case 19:
                add_Instruction( &gen->stringParam, "\\019" );
                break;
            case 20:
                add_Instruction( &gen->stringParam, "\\020" );
                break;
            case 21:
                add_Instruction( &gen->stringParam, "\\021" );
                break;
            case 22:
                add_Instruction( &gen->stringParam, "\\022" );
                break;
            case 23:
                add_Instruction( &gen->stringParam, "\\023" );
                break;
            case 24:
                add_Instruction( &gen->stringParam, "\\024" );
                break;
            case 25:
                add_Instruction( &gen->stringParam, "\\025" );
                break;
            case 26:
                add_Instruction( &gen->stringParam, "\\026" );
                break;
            case 27:
                add_Instruction( &gen->stringParam, "\\027" );
                break;
            case 28:
                add_Instruction( &gen->stringParam, "\\028" );
                break;
            case 29:
                add_Instruction( &gen->stringParam, "\\029" );
                break;
            case 30:
                add_Instruction( &gen->stringParam, "\\030" );
                break;
            case 31:
                add_Instruction( &gen->stringParam, "\\031" );
                break;
            case 32:
                add_Instruction( &gen->stringParam, "\\032" );
                break;
            case 35:
                add_Instruction( &gen->stringParam, "\\035" );
                break;
            case 92:
                add_Instruction( &gen->stringParam, "\\092" );
                break;
            default:
                add_Char( &gen->stringParam, str[i] );
                break;
        }
    }

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "DEFVAR TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_newLine( &gen->functionBody );
        add_Instruction( &gen->functionBody, "MOVE TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_Instruction( &gen->functionBody, " string@" );
        add_Instruction(&gen->functionBody,gen->stringParam.data);
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "DEFVAR TF@%" );
        add_Int( &gen->mainBody, paramCount );
        add_newLine( &gen->mainBody );
        add_Instruction( &gen->mainBody, "MOVE TF@%" );
        add_Int( &gen->mainBody, paramCount );
        add_Instruction( &gen->mainBody, " string@" );
        add_Instruction(&gen->mainBody,gen->stringParam.data);
        add_newLine( &gen->mainBody );
    }
    clear_Tape( &gen->stringParam );
}

void gen_FunctionParamNil( generator_t* gen, bool inFunc ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "PUSHS nil@nil\n" );
    } else {
        add_Instruction( &gen->mainBody, "PUSHS nil@nil\n" );
    }
}


void gen_Function( generator_t* gen ) {
    //connect the head, body and foot of the function
    add_Instruction(&gen->functions, gen->functionHead.data);
    add_Instruction(&gen->functions, gen->functionBody.data);
    add_Instruction(&gen->functions, gen->functionFoot.data);
    add_newLine(&gen->functions);
 }

/*
!!!eval experession!!!
JUMPIFEQ _else_[selectCount] GF@&bool bool@false
true_statements
JUMP _if_done_[selectCount]
LABEL _else_[selectCount]
fakse_statenebts
LABEL _if_done[selectCount]
*/
void gen_IfThenElse( generator_t* gen, bool inFunc) {

    //eval expression somehow
    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "JUMPIFEQ _else_" );
        add_Int( &gen->functionBody, gen->selectCount );
        add_Instruction( &gen->functionBody, " GF@&bool bool@false\n" );
    } else {
        add_Instruction( &gen->mainBody, "JUMPIFEQ _else_" );
        add_Int( &gen->mainBody, gen->selectCount );
        add_Instruction( &gen->mainBody, " GF@&bool bool@false\n" );
    }
 }

void gen_IfDone( generator_t* gen, bool inFunc ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "JUMP _if_done" );
        add_Int( &gen->functionBody, gen->selectCount );
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "JUMP _if_done" );
        add_Int( &gen->mainBody, gen->selectCount );
        add_newLine( &gen->mainBody );
    }
 }

void gen_IfDone_End( generator_t* gen, bool inFunc ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "LABEL _if_done" );
        add_Int( &gen->functionBody, gen->selectCount );
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "LABEL _if_done" );
        add_Int( &gen->mainBody, gen->selectCount );
        add_newLine( &gen->mainBody );
    }
    //Increment the count of selections to differenriete between other flow control statements
    gen->selectCount++;
 }

void gen_IfThenElse_End( generator_t* gen, bool inFunc ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "LABEL _else_" );
        add_Int( &gen->functionBody, gen->selectCount );
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "LABEL _else_" );
        add_Int( &gen->mainBody, gen->selectCount );
        add_newLine( &gen->mainBody );
    }
}

/*
LABEL _while_[iterCount]                                //  \
!!!eval expression!!!                                   //   gen_While
JUMPIFEQ _while_end_[iterCount] GF@&bool bool@true      //  /
statements
JUMP _while_[iterCount]                                 //  \
LABEL _while_end_[iterCount]                            //   gen_WhileEnd
*/
void gen_While( generator_t* gen, bool inFunc ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "LABEL _while_" );
        add_Int( &gen->functionBody, gen->iterCount );
        add_newLine( &gen->functionBody );

        //eval expression somehow
    } else {
        add_Instruction( &gen->mainBody, "LABEL _while_" );
        add_Int( &gen->mainBody, gen->iterCount );
        add_newLine( &gen->mainBody );

        //eval expression somehow
    }
}

void gen_WhileCond( generator_t* gen, bool inFunc ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "JUMPIFEQ _while_end_" );
        add_Int( &gen->functionBody, gen->iterCount );
        add_Instruction( &gen->functionBody, " GF@&bool bool@true\n");
    } else {
        add_Instruction( &gen->mainBody, "JUMPIFEQ _while_end_" );
        add_Int( &gen->mainBody, gen->iterCount );
        add_Instruction( &gen->mainBody, " GF@&bool bool@true\n");
    }
}

void gen_WhileEnd( generator_t* gen, bool inFunc ) {

    if ( inFunc ) {

        //add jump instruction
        add_Instruction( &gen->functionBody, "JUMP _while_" );
        add_Int( &gen->functionBody, gen->iterCount );
        add_newLine( &gen->functionBody );

        //add label for while end
        add_Instruction( &gen->functionBody, "LABEL _while_end_" );
        add_Int( &gen->functionBody, gen->iterCount );
        add_newLine( &gen->functionBody );
    } else {

        //add jump instruction
        add_Instruction( &gen->mainBody, "JUMP _while_" );
        add_Int( &gen->mainBody, gen->iterCount );
        add_newLine( &gen->mainBody );

        //add label for while end
        add_Instruction( &gen->mainBody, "LABEL _while_end_" );
        add_Int( &gen->mainBody, gen->iterCount );
        add_newLine( &gen->mainBody );
    }
    gen->iterCount++;
}

// void convert_Type( generator_t* gen, ASTNode* node, ASTNode* childNode, bool inFunc ) {

//     if (  node->resultType == TK_DOUBLE ) {
//         if ( inFunc ) {
//             add_Instruction( &gen->functionBody, "CALL Int2Double\n" );
//         } else {
//             add_Instruction( &gen->mainBody, "CALL Int2Double\n" );
//         }
//     } else if ( node->resultType == TK_INT ) {
//         if ( inFunc ) {
//             add_Instruction( &gen->functionBody, "CALL Double2Int\n" );
//         } else {
//             add_Instruction( &gen->mainBody, "CALL Double2Int\n" );
//         }
//     }
// }

void boolian_convert_Type( generator_t* gen, ASTNode* node, bool inFunc ) {

    if ( node->left->token.type == node->right->token.type ) {
        return;
    } else if ( node->left->token.type == TK_DOUBLE ) {
        if ( inFunc ) {
            add_Instruction( &gen->functionBody, "CALL Int2Double\n" );
        } else {
            add_Instruction( &gen->mainBody, "CALL Int2Double\n" );
        }
    } else {
        if ( inFunc ) {
            add_Instruction( &gen->functionBody, "CALL Double2Int\n" );
        } else {
            add_Instruction( &gen->mainBody, "CALL Double2Int\n" );
        }
    }
}

void gen_Expr( generator_t* gen, ASTNode* node, bool inFunc ) {

    if ( node == NULL ) return;

    char buf[30];
    switch( node->token.type ) {
        case TK_PLUS:
            gen_Expr( gen, node->left, inFunc );
            // convert_Type( gen, node, node->left, inFunc );

            gen_Expr( gen, node->right, inFunc );
            // convert_Type( gen, node, node->right, inFunc );

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "ADDS\n" );
                add_Instruction( &gen->functionBody, "POPS GF@&tmp1\n" );
                add_Instruction( &gen->functionBody, "MOVE " );
                add_Instruction( &gen->functionBody, "LF@" );
                add_Instruction( &gen->functionBody, gen->varName.data );
                add_Instruction( &gen->functionBody, " GF@&tmp1\n" );
            } else {
                add_Instruction( &gen->mainBody, "ADDS\n" );
                add_Instruction( &gen->mainBody, "POPS GF@&tmp1\n" );
                add_Instruction( &gen->mainBody, "MOVE " );
                add_Instruction( &gen->mainBody, "GF@" );
                add_Instruction( &gen->mainBody, gen->varName.data );
                add_Instruction( &gen->mainBody, " GF@&tmp1\n" );
            }
            break;

        case TK_MINUS:
            gen_Expr( gen, node->left, inFunc );
            // convert_Type( gen, node, node->left, inFunc );

            gen_Expr( gen, node->right, inFunc );
            // convert_Type( gen, node, node->right, inFunc );

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "SUBS\n" );
                add_Instruction( &gen->functionBody, "POPS GF@t&mp1\n" );
                add_Instruction( &gen->functionBody, "MOVE " );
                add_Instruction( &gen->functionBody, "LF@" );
                add_Instruction( &gen->functionBody, gen->varName.data );
                add_Instruction( &gen->functionBody, " GF@&tmp1\n" );
            } else {
                add_Instruction( &gen->mainBody, "SUBS\n" );
                add_Instruction( &gen->mainBody, "POPS GF@t&mp1\n" );
                add_Instruction( &gen->mainBody, "MOVE " );
                add_Instruction( &gen->mainBody, "GF@" );
                add_Instruction( &gen->mainBody, gen->varName.data );
                add_Instruction( &gen->mainBody, " GF@&tmp1\n" );
            }
            break;

        case TK_MUL:
            gen_Expr( gen, node->left, inFunc );
            // convert_Type( gen, node, node->left, inFunc );

            gen_Expr( gen, node->right, inFunc );
            // convert_Type( gen, node, node->right, inFunc );

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "MULS\n" );
                add_Instruction( &gen->functionBody, "POPS GF@&tmp1\n" );
                add_Instruction( &gen->functionBody, "MOVE " );
                add_Instruction( &gen->functionBody, "LF@" );
                add_Instruction( &gen->functionBody, gen->varName.data );
                add_Instruction( &gen->functionBody, " GF@&tmp1\n" );
            } else {
                add_Instruction( &gen->mainBody, "MULS\n" );
                add_Instruction( &gen->mainBody, "POPS GF@&tmp1\n" );
                add_Instruction( &gen->mainBody, "MOVE " );
                add_Instruction( &gen->mainBody, "GF@" );
                add_Instruction( &gen->mainBody, gen->varName.data );
                add_Instruction( &gen->mainBody, " GF@&tmp1\n" );
            }
            break;

        case TK_DIV:
            gen_Expr( gen, node->left, inFunc );
            // convert_Type( gen, node, node->left, inFunc );

            gen_Expr( gen, node->right, inFunc );
            // convert_Type( gen, node, node->right, inFunc );

            if ( node->left->token.type == TK_INT && node->left->token.type == TK_INT ) {  //  aky je rozfiel medzy node->left.token.type a node->resultType??  (node->left.token.type je type tokenu co obsahuje napr priamo konstantu node->resultType je podla semantiky co bi malo byt vysledkom tej operacii
            //Operandy su double
                if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "IDIVS\n" );
                    add_Instruction( &gen->functionBody, "POPS GF@t&mp1\n" );
                    add_Instruction( &gen->functionBody, "MOVE " );
                    add_Instruction( &gen->functionBody, "LF@" );
                    add_Instruction( &gen->functionBody, gen->varName.data );
                    add_Instruction( &gen->functionBody, " GF@&tmp1\n" );
                } else {
                    add_Instruction( &gen->mainBody, "IDIVS\n" );
                    add_Instruction( &gen->mainBody, "POPS GF@t&mp1\n" );
                    add_Instruction( &gen->mainBody, "MOVE " );
                    add_Instruction( &gen->mainBody, "GF@" );
                    add_Instruction( &gen->mainBody, gen->varName.data );
                    add_Instruction( &gen->mainBody, " GF@&tmp1\n" );
                }
            } else {
            //Operandy su int
                if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "DIVS\n" );
                    add_Instruction( &gen->functionBody, "POPS GF@t&mp1\n" );
                    add_Instruction( &gen->functionBody, "MOVE " );
                    add_Instruction( &gen->functionBody, "LF@" );
                    add_Instruction( &gen->functionBody, gen->varName.data );
                    add_Instruction( &gen->functionBody, " GF@&tmp1\n" );
                } else {
                    add_Instruction( &gen->mainBody, "DIVS\n" );
                    add_Instruction( &gen->mainBody, "POPS GF@t&mp1\n" );
                    add_Instruction( &gen->mainBody, "MOVE " );
                    add_Instruction( &gen->mainBody, "GF@" );
                    add_Instruction( &gen->mainBody, gen->varName.data );
                    add_Instruction( &gen->mainBody, " GF@&tmp1\n" );
                }
            }
            break;

        case TK_EQ:
            gen_Expr( gen, node->left, inFunc );
            gen_Expr( gen, node->right, inFunc );
            boolian_convert_Type( gen, node, inFunc );

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "EQS\n" );
                add_Instruction( &gen->functionBody, "POPS GF@&bool\n");            // pop the result in GF@&bool to use in comparisons
            } else {
                add_Instruction( &gen->mainBody, "EQS\n" );
                add_Instruction( &gen->mainBody, "POPS GF@&bool\n" );               // pop the result in GF@&bool to use in comparisons
            }
            break;

        case TK_NEQ:
            gen_Expr( gen, node->left, inFunc );
            gen_Expr( gen, node->right, inFunc );
            boolian_convert_Type( gen, node, inFunc );

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "EQS\n" );
                add_Instruction( &gen->functionBody, "POPS GF@&bool\n" );
                add_Instruction( &gen->functionBody, "NOT GF@&bool\n" );
            } else {
                add_Instruction( &gen->mainBody, "EQS\n" );
                add_Instruction( &gen->mainBody, "POPS GF@&bool\n" );
                add_Instruction( &gen->mainBody, "NOT GF@&bool\n" );
            }
            break;

        case TK_LT:
            gen_Expr( gen, node->left, inFunc );
            gen_Expr( gen, node->right, inFunc );
            boolian_convert_Type( gen, node, inFunc );

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "LTS\n" );
                add_Instruction( &gen->functionBody, "POPS GF@&bool\n" );
            } else {
                add_Instruction( &gen->mainBody, "LTS\n" );
                add_Instruction( &gen->mainBody, "POPS GF@&bool\n" );
            }
            break;

        case TK_GT:
            gen_Expr( gen, node->left, inFunc );
            gen_Expr( gen, node->right, inFunc );
            boolian_convert_Type( gen, node, inFunc );

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "GTS\n" );
                add_Instruction( &gen->functionBody, "POPS GF@&bool\n");            // pop the result in GF@&bool to use in comparisons
            } else {
                add_Instruction( &gen->mainBody, "GTS\n" );
                add_Instruction( &gen->mainBody, "POPS GF@&bool\n" );               // pop the result in GF@&bool to use in comparisons
            }
            break;

        case TK_LE:
            gen_Expr( gen, node->left, inFunc );
            gen_Expr( gen, node->right, inFunc );
            boolian_convert_Type( gen, node, inFunc );

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "CALL _LE\n" );
            } else {
                add_Instruction( &gen->mainBody, "CALL _LE\n" );
            }
            break;

        case TK_GE:
            gen_Expr( gen, node->left, inFunc );
            gen_Expr( gen, node->right, inFunc );
            boolian_convert_Type( gen, node, inFunc );

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "CALL _GE\n" );
            } else {
                add_Instruction( &gen->mainBody, "CALL _GE\n" );
            }
            break;

        case TK_IDENTIFIER:
            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "PUSHS LF@" );
                add_Instruction( &gen->functionBody, node->token.data.String );    //PUSHS LF@name\n
                add_newLine( &gen->functionBody );
            } else {
                add_Instruction( &gen->mainBody, "PUSHS GF@" );
                add_Instruction( &gen->mainBody, node->token.data.String );    //PUSHS LF@name\n
                add_newLine( &gen->mainBody );
            }
            break;

        case TK_DOUBLE:        // double literal

            snprintf(buf, sizeof(buf), "%f", node->token.data.Double );
            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "PUSHS float@" );
                add_Instruction( &gen->functionBody, buf);
                add_newLine( &gen->functionBody );
            } else {
                add_Instruction( &gen->mainBody, "PUSHS float@" );
                add_Instruction( &gen->mainBody, buf );
                add_newLine( &gen->mainBody );
            }
            break;

        case TK_INT:           // integer literal

            snprintf(buf, sizeof(buf), "%lld", node->token.data.Int );
            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "PUSHS int@" );
                add_Instruction( &gen->functionBody, buf );
                add_newLine( &gen->functionBody );
            } else {
                add_Instruction( &gen->mainBody, "PUSHS int@" );
                add_Instruction( &gen->mainBody, buf );
                add_newLine( &gen->mainBody );
            }
            break;

        case TK_STRING:        // string literal
        case TK_MLSTRING:      // multi-line string literal
            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "PUSHS string@" );
                add_Instruction( &gen->functionBody, node->token.data.String );
                add_newLine( &gen->functionBody );
            } else {
                add_Instruction( &gen->mainBody, "PUSHS string@" );
                add_Instruction( &gen->mainBody, node->token.data.String );
                add_newLine( &gen->mainBody );
            }
            break;

        case TK_COALESCE:
            gen_Expr( gen, node->right, inFunc );
            gen_Expr( gen, node->left, inFunc );

            if (inFunc ) {
                add_Instruction( &gen->functionBody, "CALL _COALESCE\n" );
            } else {
                add_Instruction( &gen->mainBody, "CALL _COALESCE\n" );
            }
            break;

        default:
            break;
    }
}

void print_Code(generator_t* gen){
    print_Intructions(&gen->header);
    print_Intructions(&gen->functions);
    //print_Intructions(&gen->functionHead);

    //print_Intructions(&gen->functionFoot);
    //print_Intructions(&gen->functionBody);

    print_Intructions(&gen->mainBody);
}

void gen_COALESCE( generator_t* gen ) {
    add_Instruction( &gen->functions, "JUMP _skip_COALESCE\n"
                                      "LABEL _COALESCE\n"
                                      "CREATEFRAME\n"
                                      "PUSHFRAME\n"
                                      "DEFVAR LF@left\n"
                                      "DEFVAR LF@right\n"
                                      "POPS LF@left\n"
                                      "JUMPIFEQ ?is_nil LF@left nil@nil\n"
                                      "POPS LF@right\n"
                                      "PUSHS LF@left\n"  
                                      "LABEL ?is_nil\n"
                                      "POPFRAME\n"
                                      "RETURN\n"
                                      "LABEL _skip_COALESCE\n" 
                                        );
}

void gen_LE( generator_t* gen ) {
    add_Instruction( &gen->functions, "JUMP _skip_LE\n"
                                      "LABEL _LE\n" 
                                      "CREAFRAME\n"
                                      "PUSHFRAME\n"
                                      "DEFVAR LF@right\n"
                                      "DEFVAR LF@left\n"
                                      "DEFVAR LF@result\n"
                                      "POPS LF@right\n"
                                      "POPS LF@left\n"
                                      "LT LF@result LF@left LF@right\n"
                                      "JUMPIFEQ ?is_false LF@result bool@false\n"
                                      "EQ LF@result LF@left LF@right\n"
                                      "JUMPIFEQ ?is_false LF@result bool@false\n"
                                      "MOVE GF@&bool bool@true\n"
                                      "JUMP ?end_LE\n"
                                      "label ?is_false\n"
                                      "MOVE GF@&bool bool@false\n"
                                      "LABEL ?end_LE\n"
                                      "POPFRAME\n"
                                      "RETURN\n"
                                      "LABEL _skip_LE\n"
                                      );
}

void gen_GE( generator_t* gen ) {
    add_Instruction( &gen->functions, "JUMP _skip_GE\n"
                                      "LABEL _GE\n" 
                                      "CREAFRAME\n"
                                      "PUSHFRAME\n"
                                      "DEFVAR LF@right\n"
                                      "DEFVAR LF@left\n"
                                      "DEFVAR LF@result\n"
                                      "POPS LF@right\n"
                                      "POPS LF@left\n"
                                      "GT LF@result LF@left LF@right\n"
                                      "JUMPIFEQ ?is_false LF@result bool@false\n"
                                      "EQ LF@result LF@left LF@right\n"
                                      "JUMPIFEQ ?is_false LF@result bool@false\n"
                                      "MOVE GF@&bool bool@true\n"
                                      "JUMP ?end_GE\n"
                                      "label ?is_false\n"
                                      "MOVE GF@&bool bool@false\n"
                                      "LABEL ?end_GE\n"
                                      "POPFRAME\n"
                                      "RETURN\n"
                                      "LABEL _skip_GE\n\n"
                                      );
}

/*
##########################################################################################################
##########################################################################################################
################################ functions to generate inbuild functions #################################
##########################################################################################################
##########################################################################################################
*/


void gen_buildin_readString(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_readString\n"
                                    "LABEL readString\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "READ LF@tmp1 string\n"
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_readString\n"
                                    );
}

void gen_buildin_readInt(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_readInt\n"
                                    "LABEL readInt\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@type\n"
                                    "READ LF@tmp1 int\n"
                                    "TYPE LF@type LF@tmp1\n"                           //check the type of input
                                    "JUMPIFNEQ _retnil LF@type string@int\n"           //If the input is not int, return nil
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retnil\n"
                                    "PUSHS nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_readInt\n"
                                    );
}

void gen_buildin_readDouble(generator_t* gen){
    
    add_Instruction(&gen->functions, "JUMP _skip_readDouble\n"
                                    "LABEL readDouble\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@type\n"
                                    "READ LF@tmp1 float\n"
                                    "TYPE LF@type LF@tmp1\n"
                                    "JUMPIFNEQ _retnil LF@type string@float\n"
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retnil\n"
                                    "PUSHS nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_readDouble\n"
                                    );
}
/*#################### termy budu musiet byt napushovane odzadu ####################*/
void gen_buildin_write(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_write\n"
                                    "LABEL write\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@term\n"
                                    "DEFVAR LF@termCount\n"
                                    "POPS LF@termCount\n"
                                    "LABEL _writeTerms\n"
                                    "JUMPIFEQ _writeEnd LF@termCount int@0\n"
                                    "SUB LF@termCount LF@termCount int@1\n"
                                    "POPS LF@term\n"
                                    "WRITE LF@term\n"
                                    "JUMP _writeTerms\n"
                                    "LABEL _writeEnd\n"
                                    "PUSHS nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_write\n"
                                    );
}

void gen_buildin_Int2Double(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_Int2Double\n"
                                    "LABEL Int2Double\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp2\n"
                                    "DEFVAR LF@tmp1\n"
                                    "POPS LF@tmp2\n"
                                    "INT2FLOAT LF@tmp1 LF@tmp2\n"
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_Int2Double\n" 
                                    );
}

void gen_buildin_Double2Int(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_Double2Int\n"
                                    "LABEL Double2Int\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@tmp2\n"
                                    "POPS LF@tmp2\n"
                                    "FLOAT2INT LF@tmp1 LF@tmp2\n"
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_Double2Int\n"
                                    );
}

void gen_buildin_length(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_length\n"
                                    "LABEL length\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@len\n"
                                    "DEFVAR LF@str\n"
                                    "POPS LF@str\n"
                                    "STRLEN LF@len LF@str\n"
                                    "PUSHS LF@len\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_length\n"
                                    );
}

/*########################## Doriesit j otazku #############################*/
void gen_buildin_substring(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_substring\n"
                                    "LABEL substring\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@j\n"
                                    "DEFVAR LF@i\n"
                                    "DEFVAR LF@str\n"
                                    "DEFVAR LF@len\n"                           //define var for length of string
                                    "POPS LF@j\n"                               //get j
                                    "POPS LF@i\n"                               //get i
                                    "POPS LF@str\n"                             //get string from stack
                                    "STRLEN LF@len LF@str\n"                    //get the length of string
                                    "DEFVAR LF@cmp\n"                           //help var to store result of comparison
                                    "#Guard functionality\n"
                                    "LT LF@cmp LF@i int@0\n"                    //Compare = i < 0
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if i < 0
                                    "LT LF@cmp LF@j int@0\n"                    //compare = j < 0
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if j < 0
                                    "GT LF@cmp LF@i LF@j\n"                     //compare = i > j
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if j < j
                                    "EQ LF@cmp LF@i LF@len\n"                   //compare = i = len
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if i = len
                                    "GT LF@cmp LF@i LF@len\n"                   //compare = i > len
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if j > len
                                    "GT LF@cmp LF@j LF@len\n"                   //compare = j > len
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if j > len
                                    "#Making of substring\n"
                                    "DEFVAR LF@c\n"                             //here the chars of substring will be stored
                                    "DEFVAR LF@resultString\n"
                                    "MOVE LF@resultString string@\n"            //initiate resultString to ""
                                    "LABEL _writeSubstring\n"
                                    "JUMPIFEQ _finished LF@i LF@j\n"
                                    "GETCHAR LF@c LF@str LF@i\n"
                                    "CONCAT LF@resultString LF@resultString LF@c\n"
                                    "ADD LF@i LF@i int@1\n"                     //i++;
                                    "JUMP _writeSubstring\n"
                                    "LABEL _finished\n"
                                    "PUSHS LF@resultString\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retnil\n"
                                    "PUSHS nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_substring\n"
                                    );
}                      

void gen_buildin_ord(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_ord\n"
                                    "LABEL ord\n"  
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@string\n"
                                    "POPS LF@string\n"
                                    "DEFVAR LF@c\n"                             //var to store first char of string
                                    "DEFVAR LF@len\n"                           //var to store length of string
                                    "STRLEN LF@len LF@string\n"                 //get the length of string
                                    "JUMPIFEQ _retzero LF@len int@0\n"          //if length of string is 0, return 0
                                    "STRI2INT LF@c LF@string int@0\n"           //get the char        
                                    "PUSHS LF@c\n"                              //return char
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retzero\n"
                                    "PUSHS int@0\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_ord\n"
                                    );
}

void gen_buildin_chr(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_chr\n"
                                    "LABEL chr\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@c\n"
                                    "DEFVAR LF@i\n"
                                    "POPS LF@i\n"
                                    "INT2CHAR LF@c LF@i\n"                      //covert int to ascii char
                                    "PUSHS LF@c\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_chr\n"
                                    );
}

