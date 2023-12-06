/*
IFJ PROJEKT 2023/2024
file: "generator.c"

Code generator

authors: xrusna08 

*/

//TODO (a<=b) vo while

#include <stdlib.h>
#include <string.h>
#include "generator.h"
#include "expr.h"

void freeGenerator(generator_t *gen) {
    // Free the elements of localFrame
    for (int i = 0; i < gen->local_frame_size; ++i) {
        free(gen->localFrame[i]);
    }

    // Free the array itself
    free(gen->localFrame);

    gen->localFrame = NULL;
    // Reset local_frame_size to zero
    gen->local_frame_size = 0;
}

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
    gen.paramCount = 0;
    gen.isReturn = false;
    gen.exprResult = init_Tape();
    gen.localFrame = NULL;
    gen.local_frame_size = 0;
    gen.isWhile = false;
    return gen;
}

void gen_Header( generator_t* gen ) {
    add_Instruction(&gen->header, ".IFJcode23\n"                    //every IFJcode23 needs to start with .IFJcode23 header
                                  "DEFVAR GF@&bool\n"               //
                                  "DEFVAR GF@&boolEQ\n"
                                  "DEFVAR GF@&tmp1\n"               //Global variables
                                  "DEFVAR GF@&tmp2\n"               //
                                  "DEFVAR GF@&tmp3\n"               //
                                  );

}

void gen_inbuild( generator_t* gen ) {
    gen_buildin_readInt(gen);
    gen_buildin_readString(gen);
    gen_buildin_readDouble(gen);
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

void gen_VarDefinition( generator_t* gen, char* name, bool inFunc, int scope ) {
    add_Instruction( &gen->varName, name );
    //Is token in global or local frame
    if( inFunc ) {
        addToLocalFrame(name,scope + 1,gen);
        add_Instruction(&gen->functionBody, "DEFVAR LF@");
        add_Instruction(&gen->functionBody, name);            //add the variable name to definition
        add_Instruction(&gen->functionBody, "$");
        add_Int(&gen->functionBody, scope + 1);
        add_newLine( &gen->functionBody );                             //append new line
    } else {
        if(scope != 0){
            addToLocalFrame(name,scope + 1,gen);
            add_Instruction(&gen->mainBody, "DEFVAR LF@");
            add_Instruction(&gen->mainBody, name);             //add the variable name to definition
            add_Instruction(&gen->mainBody, "$");
            add_Int(&gen->mainBody, scope + 1);
            add_newLine(&gen->mainBody); //append new line
        }else {
            add_Instruction(&gen->mainBody, "DEFVAR GF@");
            add_Instruction(&gen->mainBody, name);             //add the variable name to definition
            add_newLine(&gen->mainBody);                                //append new line
        }
    }
}

void gen_AssignVal( generator_t* gen, char* varName, char* val, bool inFunc, char* type, int scope) {
    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "MOVE LF@" );
        add_Instruction( &gen->functionBody, getActualVariable(varName, scope, gen) );
        if(strlen(type) != 0) {
            add_Instruction(&gen->functionBody, type);
            add_Instruction( &gen->functionBody, val );
        }else{
            add_Instruction(&gen->functionBody, " LF@");
            add_Instruction( &gen->functionBody, getActualVariable(val, scope, gen));
        }
        add_newLine( &gen->functionBody);
    } else {
        if(scope != 0){
            add_Instruction( &gen->mainBody, "MOVE LF@" );
            add_Instruction( &gen->mainBody, getActualVariable(varName, scope, gen));
            if(strlen(type) != 0) {
                add_Instruction(&gen->mainBody, type);
                add_Instruction( &gen->mainBody, val );
            }else{
                add_Instruction(&gen->mainBody, " LF@");
                add_Instruction( &gen->mainBody, getActualVariable(val, scope, gen));
            }
            add_newLine( &gen->mainBody );
        }else{
            add_Instruction( &gen->mainBody, "MOVE GF@" );
            add_Instruction( &gen->mainBody, varName );
            if(strlen(type) != 0) {
                add_Instruction(&gen->mainBody, type);
            }else{
                add_Instruction(&gen->mainBody, " GF@");
            }
            add_Instruction( &gen->mainBody, val);
            add_newLine( &gen->mainBody );
        }
    }
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


void gen_FunctionHeader( generator_t* gen, char* funcName, Node* function ,Node* globalFrame, int scope) {
    clear_Tape(&gen->functionHead);
    add_Instruction( &gen->functionName, funcName );

    add_Instruction( &gen->functionHead, "JUMP _skip_" );
    add_Instruction( &gen->functionHead, funcName );  //name of function
    add_newLine( &gen->functionHead );
    add_Instruction( &gen->functionHead, "LABEL ");
    add_Instruction( &gen->functionHead, funcName );  //name of function
    add_newLine( &gen->functionHead );
    add_Instruction( &gen->functionHead, "PUSHFRAME\n");
    addNonFunctionSymbolsFromGlobal(globalFrame,gen,1,true);
    //Preklad predavanych parametru na parametry z hlavicky
    for (int i = 0; i < function->symbol.parametersCount; i++) {
        addToLocalFrame(function->symbol.parameters[i].id ,scope + 1,gen);
        add_Instruction( &gen->functionHead, "DEFVAR LF@" );
        add_Instruction( &gen->functionHead, function->symbol.parameters[i].id );
        add_Instruction( &gen->functionHead, "$");
        add_Int(&gen->functionHead,scope + 1);
        add_newLine( &gen->functionHead );
        add_Instruction( &gen->functionHead, "MOVE LF@" );
        add_Instruction( &gen->functionHead, function->symbol.parameters[i].id );
        add_Instruction( &gen->functionHead, "$");
        add_Int(&gen->functionHead,scope + 1);
        add_Instruction( &gen->functionHead, " LF@%" );
        add_Int(&gen->functionHead,i+1);
        add_newLine( &gen->functionHead );
    }

    //Pomocna promenna pro return
    add_Instruction( &gen->functionHead, "DEFVAR LF@%retval\n");
    clear_Tape(&gen->varName);

}

void gen_IdentifierReturn(generator_t* gen,token_t token, int scope){
    add_Instruction( &gen->functionFoot, "MOVE LF@%retval LF@");
    add_Instruction(&gen->functionFoot, getActualVariable(token.data.String,scope,gen));
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

void gen_AssignReturnToVariable(generator_t* gen, token_t tokenToAssign, bool inFunc){
    if(inFunc){
        add_Instruction( &gen->functionBody, "MOVE LF@");
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

void gen_FunctionParam( generator_t* gen, char* param, bool inFunc, int paramCount, int scope) {

    if ( inFunc ) {
        if ( gen->isWrite == 0) {
            add_Instruction( &gen->functionBody, "WRITE LF@" );
            char *found = getActualVariable(param,scope,gen);
            add_Instruction( &gen->functionBody, getActualVariable(param,scope,gen) );
            add_newLine( &gen->functionBody );    
        } else {
            add_Instruction( &gen->functionBody, "DEFVAR TF@%" );
            add_Int( &gen->functionBody, paramCount );
            add_newLine( &gen->functionBody );
            add_Instruction( &gen->functionBody, "MOVE TF@%" );
            add_Int( &gen->functionBody, paramCount );
            add_Instruction( &gen->functionBody, " LF@" );
            add_Instruction(&gen->functionBody,param);
            add_newLine( &gen->functionBody );
        }
    } else {
        if ( gen->isWrite == 0) {
            if(scope != 0) {
                add_Instruction(&gen->mainBody, "WRITE GF@");
                char *found = getActualVariable(param,scope,gen);
                add_Instruction(&gen->mainBody, param);
                add_newLine(&gen->mainBody);
            }else{
                add_Instruction(&gen->mainBody, "WRITE GF@");
                add_Instruction(&gen->mainBody, param);
                add_newLine(&gen->mainBody);
            }
        } else {
            add_Instruction( &gen->mainBody, "DEFVAR TF@%" );
            add_Int( &gen->mainBody, paramCount );
            add_newLine( &gen->mainBody );
            add_Instruction( &gen->mainBody, "MOVE TF@%" );
            add_Int( &gen->mainBody, paramCount );
            add_Instruction( &gen->mainBody, " GF@" );
            add_Instruction(&gen->mainBody,param);
            add_newLine( &gen->mainBody );
        }
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
    if ( inFunc ) {
      if (gen->isWrite == 0){
            add_Instruction( &gen->functionBody, "WRITE int@" );
            add_Int( &gen->functionBody, val );
            add_newLine( &gen->functionBody );
      } else {
          add_Instruction( &gen->functionBody, "DEFVAR TF@%" );
          add_Int( &gen->functionBody, paramCount );
          add_newLine( &gen->functionBody );
          add_Instruction( &gen->functionBody, "MOVE TF@%" );
          add_Int( &gen->functionBody, paramCount );
          add_Instruction( &gen->functionBody, " int@" );
          add_Int( &gen->functionBody, val );
          add_newLine( &gen->functionBody );
      }
    } else {
      if ( gen->isWrite == 0 ){
        add_Instruction( &gen->mainBody, "WRITE int@" );
        add_Int( &gen->mainBody, val );
        add_newLine( &gen->mainBody );    
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
    }
}

void gen_FunctionParamDouble( generator_t* gen, double val, bool inFunc, int paramCount) {

    if ( inFunc ) {
      if ( gen->isWrite == 0) {
        add_Instruction( &gen->functionBody, "WRITE float@" );
        add_Double( &gen->functionBody, val );
        add_newLine( &gen->functionBody );
      } else {
        add_Instruction( &gen->functionBody, "DEFVAR TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_newLine( &gen->functionBody );
        add_Instruction( &gen->functionBody, "MOVE TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_Instruction( &gen->functionBody, " float@" );
        add_Double( &gen->functionBody, val );
        add_newLine( &gen->functionBody );
      }
    } else {
       if (gen->isWrite == 0 ) {
        add_Instruction( &gen->mainBody, "WRITE float@" );
        add_Double( &gen->mainBody, val );
        add_newLine( &gen->mainBody );    
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
}

char* gen_convertString( char* string ) {

    instructionTape_t tmpTape = init_Tape();
    unsigned int length = strlen(string);

    for ( unsigned int i = 0; i < length; i++ ) {
        switch(string[i]) {
            case 0:
                add_Instruction( &tmpTape, "\\000" );
                break;
            case 1:
                add_Instruction( &tmpTape, "\\001" );
                break;
            case 2:
                add_Instruction( &tmpTape, "\\002" );
                break;
            case 3:
                add_Instruction( &tmpTape, "\\003" );
                break;
            case 4:
                add_Instruction( &tmpTape, "\\004" );
                break;
            case 5:
                add_Instruction( &tmpTape, "\\005" );
                break;
            case 6:
                add_Instruction( &tmpTape, "\\006" );
                break;
            case 7:
                add_Instruction( &tmpTape, "\\007" );
                break;
            case 8:
                add_Instruction( &tmpTape, "\\008" );
                break;
            case 9:
                add_Instruction( &tmpTape, "\\009" );
                break;
            case 10:
                add_Instruction( &tmpTape, "\\010" );
                break;
            case 11:
                add_Instruction( &tmpTape, "\\011" );
                break;
            case 12:
                add_Instruction( &tmpTape, "\\012" );
                break;
            case 13:
                add_Instruction( &tmpTape, "\\013" );
                break;
            case 14:
                add_Instruction( &tmpTape, "\\014" );
                break;
            case 15:
                add_Instruction( &tmpTape, "\\015" );
                break;
            case 16:
                add_Instruction( &tmpTape, "\\016" );
                break;
            case 17:
                add_Instruction( &tmpTape, "\\017" );
                break;
            case 18:
                add_Instruction( &tmpTape, "\\018" );
                break;
            case 19:
                add_Instruction( &tmpTape, "\\019" );
                break;
            case 20:
                add_Instruction( &tmpTape, "\\020" );
                break;
            case 21:
                add_Instruction( &tmpTape, "\\021" );
                break;
            case 22:
                add_Instruction( &tmpTape, "\\022" );
                break;
            case 23:
                add_Instruction( &tmpTape, "\\023" );
                break;
            case 24:
                add_Instruction( &tmpTape, "\\024" );
                break;
            case 25:
                add_Instruction( &tmpTape, "\\025" );
                break;
            case 26:
                add_Instruction( &tmpTape, "\\026" );
                break;
            case 27:
                add_Instruction( &tmpTape, "\\027" );
                break;
            case 28:
                add_Instruction( &tmpTape, "\\028" );
                break;
            case 29:
                add_Instruction( &tmpTape, "\\029" );
                break;
            case 30:
                add_Instruction( &tmpTape, "\\030" );
                break;
            case 31:
                add_Instruction( &tmpTape, "\\031" );
                break;
            case 32:
                add_Instruction( &tmpTape, "\\032" );
                break;
            case 35:
                add_Instruction( &tmpTape, "\\035" );
                break;
            case 92:
                add_Instruction( &tmpTape, "\\092" );
                break;
            default:
                add_Char( &tmpTape, string[i] );
                break;
        }
    }
    return tmpTape.data;
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
      if ( gen->isWrite == 0 ) {
        add_Instruction( &gen->functionBody, "WRITE string@" );
        add_Instruction( &gen->functionBody, gen->stringParam.data );
        add_newLine( &gen->functionBody );    
      } else {
        add_Instruction( &gen->functionBody, "DEFVAR TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_newLine( &gen->functionBody );
        add_Instruction( &gen->functionBody, "MOVE TF@%" );
        add_Int( &gen->functionBody, paramCount );
        add_Instruction( &gen->functionBody, " string@" );
        add_Instruction(&gen->functionBody,gen->stringParam.data);
        add_newLine( &gen->functionBody );
      }
    } else {
       if ( gen->isWrite == 0 ) {
          add_Instruction( &gen->mainBody, "WRITE string@" );
          add_Instruction( &gen->mainBody, gen->stringParam.data );
          add_newLine( &gen->mainBody );
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
    }
    clear_Tape( &gen->stringParam );
}

void gen_FunctionParamNil( generator_t* gen, bool inFunc ) {

    if ( inFunc ) {
        if ( gen->isWrite == 0 ) {
            add_Instruction( &gen->functionBody, "WRITE nil@nil\n" );
        } else {
            add_Instruction( &gen->functionBody, "PUSHS nil@nil\n" );
        }
    } else {
        if ( gen->isWrite == 0) {
            add_Instruction( &gen->mainBody, "WRITE nil@nil\n" );    
        } else {
            add_Instruction( &gen->mainBody, "PUSHS nil@nil\n" );
        }
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
void addNonFunctionSymbolsFromGlobal(Node *root, generator_t* gen, int scope, bool isFunc) {
    if (root != NULL) {
        // In-order traversal
        addNonFunctionSymbolsFromGlobal(root->left, gen, scope, isFunc);

        // Check if the symbol is not a function
        if (!root->symbol.isFunction) {
            gen->localFrame = realloc(gen->localFrame, (gen->local_frame_size + 1) * sizeof(char*));

            // Vytvoření nového řetězce s přidaným znakem procenta a hodnotou scope
            char* var = malloc(strlen(root->symbol.key) + 20);  // Nastavte podle vašich potřeb
            sprintf(var, "%s$%d", root->symbol.key, scope);

            gen->localFrame[gen->local_frame_size] = var;
            gen->local_frame_size++;
            if(isFunc){
                add_Instruction(&gen->functionBody, "DEFVAR LF@");
                add_Instruction(&gen->functionBody, root->symbol.key);
                add_Instruction(&gen->functionBody, "$");
                add_Int(&gen->functionBody, scope);
                add_newLine(&gen->functionBody);
                add_Instruction(&gen->functionBody, "MOVE LF@");
                add_Instruction(&gen->functionBody, root->symbol.key);
                add_Instruction(&gen->functionBody, "$");
                add_Int(&gen->functionBody, scope);
                add_Instruction(&gen->functionBody, " GF@");
                add_Instruction(&gen->functionBody, root->symbol.key);
                add_newLine(&gen->functionBody);
            }else {
                add_Instruction(&gen->mainBody, "DEFVAR TF@");
                add_Instruction(&gen->mainBody, root->symbol.key);
                add_Instruction(&gen->mainBody, "$");
                add_Int(&gen->mainBody, scope);
                add_newLine(&gen->mainBody);
                add_Instruction(&gen->mainBody, "MOVE TF@");
                add_Instruction(&gen->mainBody, root->symbol.key);
                add_Instruction(&gen->mainBody, "$");
                add_Int(&gen->mainBody, scope);
                add_Instruction(&gen->mainBody, " GF@");
                add_Instruction(&gen->mainBody, root->symbol.key);
                add_newLine(&gen->mainBody);
            }
        }
        addNonFunctionSymbolsFromGlobal(root->right, gen, scope, isFunc);
    }
}

char* getActualVariable(char* key,int scope,generator_t* gen){
    /*for (int i = 0; i < gen->local_frame_size; ++i) {
        printf("%s\n",gen->localFrame[i]);
    }*/
    scope++;
    char searchKey[256]; // Předpokládáme, že klíč nebude delší než 255 znaků

    // Sestavení hledacího klíče ve tvaru "key$scope"


    for (int i = scope; i > 0; --i) {
        snprintf(searchKey, sizeof(searchKey), "%s$%d", key, scope);
        for (int j = 0; j < gen->local_frame_size; ++j) {
            char *currentVariable = gen->localFrame[j];
            if (strcmp(currentVariable, searchKey) == 0) {
                return gen->localFrame[j];
            }
        }
        scope--;
    }

    // Shoda nenalezena
    return NULL;
}

void copyVariables(generator_t* gen, bool inFunc){
    for (int i = 0; i < gen->local_frame_size; ++i) {
        if(inFunc){
            add_Instruction(&gen->functionBody,"DEFVAR TF@");
            add_Instruction(&gen->functionBody,gen->localFrame[i]);
            add_newLine(&gen->functionBody);
            add_Instruction(&gen->functionBody,"MOVE TF@");
            add_Instruction(&gen->functionBody,gen->localFrame[i]);
            add_Instruction(&gen->functionBody," LF@");
            add_Instruction(&gen->functionBody,gen->localFrame[i]);
            add_newLine(&gen->functionBody);
        }else{
            add_Instruction(&gen->mainBody,"DEFVAR TF@");
            add_Instruction(&gen->mainBody,gen->localFrame[i]);
            add_newLine(&gen->mainBody);
            add_Instruction(&gen->mainBody,"MOVE TF@");
            add_Instruction(&gen->mainBody,gen->localFrame[i]);
            add_Instruction(&gen->mainBody," LF@");
            add_Instruction(&gen->mainBody,gen->localFrame[i]);
            add_newLine(&gen->mainBody);
        }
    }
}
void addToLocalFrame(char* key,int scope, generator_t* gen){
    gen->localFrame = realloc(gen->localFrame, (gen->local_frame_size + 1) * sizeof(char*));

    // Vytvoření nového řetězce s přidaným znakem procenta a hodnotou scope
    char* var = malloc(strlen(key) + 20);  // Nastavte podle vašich potřeb
    sprintf(var, "%s$%d", key, scope);
    gen->localFrame[gen->local_frame_size] = var;
    gen->local_frame_size++;
}

void gen_IfThenElse( generator_t* gen, unsigned int scopeDepth, bool inFunc, Node* globalFrame) {
    //Increment the count of selections to differenriete between other flow control statements
    if ( scopeDepth <= 1) {gen->selectCount++;}

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "JUMPIFEQ _else_" );
        add_Int( &gen->functionBody, gen->selectCount );
        add_Int( &gen->functionBody, scopeDepth );
        add_Instruction( &gen->functionBody, " GF@&bool bool@false\n" );
        add_Instruction( &gen->functionBody, "CREATEFRAME\n" );
        if(scopeDepth > 1){
            copyVariables(gen,inFunc);
        }
        add_Instruction( &gen->functionBody, "PUSHFRAME\n" );
    } else {
        add_Instruction( &gen->mainBody, "JUMPIFEQ _else_" );
        add_Int( &gen->mainBody, gen->selectCount );
        add_Int( &gen->mainBody, scopeDepth );
        add_Instruction( &gen->mainBody, " GF@&bool bool@false\n" );
        add_Instruction( &gen->mainBody, "CREATEFRAME\n" );
        if(scopeDepth == 1) {
            addNonFunctionSymbolsFromGlobal(globalFrame, gen, scopeDepth,inFunc);
        }
        if(scopeDepth > 1){
            copyVariables(gen,inFunc);
        }
        add_Instruction( &gen->mainBody, "PUSHFRAME\n" );
    }
}

void gen_IfDone( generator_t* gen, unsigned int scopeDepth, bool inFunc ) {

    if ( inFunc ) {
        //freeGenerator(gen);
        add_Instruction( &gen->functionBody, "POPFRAME\n" );
        add_Instruction( &gen->functionBody, "JUMP _if_done" );
        add_Int( &gen->functionBody, gen->selectCount );
        add_Int( &gen->functionBody, scopeDepth );
        add_newLine( &gen->functionBody );
    } else {
        //freeGenerator(gen);
        add_Instruction( &gen->mainBody, "POPFRAME\n" );
        add_Instruction( &gen->mainBody, "JUMP _if_done" );
        add_Int( &gen->mainBody, gen->selectCount );
        add_Int( &gen->mainBody, scopeDepth );
        add_newLine( &gen->mainBody );
    }
}

void gen_IfDone_End( generator_t* gen, unsigned int scopeDepth, bool inFunc ) {

    if ( inFunc ) {
        //freeGenerator(gen);
        add_Instruction( &gen->functionBody, "POPFRAME\n" );
        add_Instruction( &gen->functionBody, "LABEL _if_done" );
        add_Int( &gen->functionBody, gen->selectCount );
        add_Int( &gen->functionBody, scopeDepth );
        add_newLine( &gen->functionBody );
    } else {
        freeGenerator(gen);
        add_Instruction( &gen->mainBody, "POPFRAME\n" );
        add_Instruction( &gen->mainBody, "LABEL _if_done" );
        add_Int( &gen->mainBody, gen->selectCount );
        add_Int( &gen->mainBody, scopeDepth );
        add_newLine( &gen->mainBody );
    }
}

void gen_IfThenElse_End( generator_t* gen, unsigned int scopeDepth, bool inFunc, Node* globalFrame ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "LABEL _else_" );
        add_Int( &gen->functionBody, gen->selectCount );
        add_Int( &gen->functionBody, scopeDepth );
        add_newLine( &gen->functionBody );
        add_Instruction( &gen->functionBody, "CREATEFRAME\n" );
        add_Instruction( &gen->functionBody, "PUSHFRAME\n" );
    } else {
        add_Instruction( &gen->mainBody, "LABEL _else_" );
        add_Int( &gen->mainBody, gen->selectCount );
        add_Int( &gen->mainBody, scopeDepth );
        add_newLine( &gen->mainBody );
        add_Instruction( &gen->mainBody, "CREATEFRAME\n" );
        if(scopeDepth == 1) {
            addNonFunctionSymbolsFromGlobal(globalFrame, gen, scopeDepth,inFunc);
        }
        if(scopeDepth > 1){
            copyVariables(gen,inFunc);
        }
        add_Instruction( &gen->mainBody, "PUSHFRAME\n" );
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
void gen_While( generator_t* gen, unsigned int scopeDepth, bool inFunc, Node* globalFrame) {

    if ( scopeDepth <= 1 ) {gen->iterCount++;}
    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "LABEL _while_" );
        add_Int( &gen->functionBody, gen->iterCount );
        add_Int( &gen->functionBody, scopeDepth );
        add_newLine( &gen->functionBody );

        //eval expression somehow
    } else {
        add_Instruction( &gen->mainBody, "CREATEFRAME\n");
        if(scopeDepth == 1){
            addNonFunctionSymbolsFromGlobal(globalFrame,gen,scopeDepth,inFunc);
        }
        add_Instruction( &gen->mainBody, "PUSHFRAME\n");
        add_Instruction( &gen->mainBody, "LABEL _while_" );
        add_Int( &gen->mainBody, gen->iterCount );
        add_Int( &gen->mainBody, scopeDepth );
        add_newLine( &gen->mainBody );


        //eval expression somehow
    }
}

void gen_WhileCond( generator_t* gen, unsigned int scopeDepth, bool inFunc ) {

    if ( inFunc ) {
        add_Instruction( &gen->functionBody, "JUMPIFEQ _while_end_" );
        add_Int( &gen->functionBody, gen->iterCount );
        add_Int( &gen->functionBody, scopeDepth );
        add_Instruction( &gen->functionBody, " GF@&bool bool@true\n");
    } else {
        add_Instruction( &gen->mainBody, "JUMPIFEQ _while_end_" );
        add_Int( &gen->mainBody, gen->iterCount );
        add_Int( &gen->mainBody, scopeDepth );
        add_Instruction( &gen->mainBody, " GF@&bool bool@true\n");
    }
}

void gen_WhileEnd( generator_t* gen, unsigned int scopeDepth, bool inFunc ) {

    if ( inFunc ) {

        //add jump instruction
        add_Instruction( &gen->functionBody, "JUMP _while_" );
        add_Int( &gen->functionBody, gen->iterCount );
        add_Int( &gen->functionBody, scopeDepth );
        add_newLine( &gen->functionBody );

        //add label for while end
        add_Instruction( &gen->functionBody, "LABEL _while_end_" );
        add_Int( &gen->functionBody, gen->iterCount );
        add_Int( &gen->functionBody, scopeDepth );
        add_newLine( &gen->functionBody );
    } else {

        //add jump instruction
        add_Instruction( &gen->mainBody, "JUMP _while_" );
        add_Int( &gen->mainBody, gen->iterCount );
        add_Int( &gen->mainBody, scopeDepth );
        add_newLine( &gen->mainBody );

        //add label for while end
        add_Instruction( &gen->mainBody, "LABEL _while_end_" );
        add_Int( &gen->mainBody, gen->iterCount );
        add_Int( &gen->mainBody, scopeDepth );
        add_newLine( &gen->mainBody );
    }
}

void gen_SaveExprResult( generator_t* gen, char* name ) {
    add_Instruction( &gen->exprResult, name );
}

void gen_ClearExprResult( generator_t* gen, bool inFunc ) {

    clear_Tape( &gen->exprResult );

    if (inFunc) {
        add_Instruction( &gen->functionBody, "CLEARS\n" );
    } else {
        add_Instruction( &gen->mainBody, "CLEARS\n" );
    }
}

//TODO ked je expression viac ako jedna operacia
void gen_Expr( generator_t* gen, ASTNode* node, bool inFunc, int scope) {

    if ( node == NULL ) return;

    char buf[255];
    switch( node->token.type ) {
        case TK_PLUS:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);

            //check if concatenation
            if ( node->resultType == TK_STRING ) {
                if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "POPS GF@&tmp2\nPOPS GF@&tmp1\n" );
                    if(gen->isReturn){
                        add_Instruction( &gen->functionBody, "CONCAT LF@%retval GF@&tmp1 GF@&tmp2" );
                        add_newLine( &gen->functionBody );
                        //go back into the stack you!
                        add_Instruction( &gen->functionBody, "PUSHS LF@%retval");
                        add_newLine( &gen->functionBody );
                    }else{
                        add_Instruction( &gen->functionBody, "CONCAT LF@" );
                        add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                        add_Instruction( &gen->functionBody, " GF@&tmp1 GF@&tmp2\n" );
                        add_Instruction(&gen->functionBody, "PUSHS LF@");
                        add_Instruction(&gen->functionBody, getActualVariable(gen->exprResult.data, scope, gen));
                        add_newLine( &gen->functionBody );
                    }
                } else {
                    if(scope > 0 || gen->isWhile) {
                        add_Instruction(&gen->mainBody, "POPS GF@&tmp2\nPOPS GF@&tmp1\n");
                        add_Instruction(&gen->mainBody, "CONCAT LF@");
                        add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                        add_Instruction(&gen->mainBody, " GF@&tmp1 GF@&tmp2\n");
                        add_Instruction(&gen->mainBody, "PUSHS LF@");
                        add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                        add_newLine(&gen->mainBody);
                    }else{
                        add_Instruction(&gen->mainBody, "POPS GF@&tmp2\nPOPS GF@&tmp1\n");
                        add_Instruction(&gen->mainBody, "CONCAT GF@");
                        add_Instruction(&gen->mainBody, gen->exprResult.data);
                        add_Instruction(&gen->mainBody, " GF@&tmp1 GF@&tmp2\n");
                        add_Instruction(&gen->mainBody, "PUSHS GF@");
                        add_Instruction(&gen->mainBody, gen->exprResult.data);
                        add_newLine(&gen->mainBody);
                    }
                }
                //addition
            } else {
                if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "ADDS\n" );
                    add_Instruction( &gen->functionBody, "POPS LF@" );
                    if(gen->isReturn){
                        add_Instruction( &gen->functionBody, "%retval\n" );
                        //go back into the stack you!
                        add_Instruction( &gen->functionBody, "PUSHS LF@%retval\n");
                    }else{
                        add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                        add_newLine( &gen->functionBody );
                        //maybe we will need you again
                        add_Instruction( &gen->functionBody, "PUSHS LF@" );
                        add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                        add_newLine( &gen->functionBody );
                    }
                } else {
                    if(scope > 1  || gen->isWhile) {
                        add_Instruction(&gen->mainBody, "ADDS\n");
                        add_Instruction(&gen->mainBody, "POPS LF@");
                        add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                        add_newLine(&gen->mainBody);
                        //back on the stack you go!
                        add_Instruction(&gen->mainBody, "PUSHS LF@");
                        add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                        add_newLine(&gen->mainBody);
                    }else{
                        add_Instruction(&gen->mainBody, "ADDS\n");
                        add_Instruction(&gen->mainBody, "POPS GF@");
                        add_Instruction(&gen->mainBody, gen->exprResult.data);
                        add_newLine(&gen->mainBody);
                        //back on the stack you go!
                        add_Instruction(&gen->mainBody, "PUSHS GF@");
                        add_Instruction(&gen->mainBody, gen->exprResult.data);
                        add_newLine(&gen->mainBody);
                    }
                }
            }
            // clear_Tape( &gen->exprResult );
            break;

        case TK_MINUS:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "SUBS\n" );
                add_Instruction( &gen->functionBody, "POPS LF@" );
                if(gen->isReturn){
                    add_Instruction( &gen->functionBody, "%retval\n" );
                    add_Instruction( &gen->functionBody, "PUSHS LF@%retval\n");
                }else{
                    add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                    add_newLine( &gen->functionBody );
                    add_Instruction( &gen->functionBody, "PUSHS LF@" );
                    add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                    add_newLine( &gen->functionBody );
                }
            } else {
                if(scope >0 || gen->isWhile) {
                    add_Instruction(&gen->mainBody, "SUBS\n");
                    add_Instruction(&gen->mainBody, "POPS LF@");
                    add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                    add_newLine(&gen->mainBody);
                    add_Instruction(&gen->mainBody, "PUSHS LF@");
                    add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                    add_newLine(&gen->mainBody);
                }else{
                    add_Instruction(&gen->mainBody, "SUBS\n");
                    add_Instruction(&gen->mainBody, "POPS GF@");
                    add_Instruction(&gen->mainBody, gen->exprResult.data);
                    add_newLine(&gen->mainBody);
                    add_Instruction(&gen->mainBody, "PUSHS GF@");
                    add_Instruction(&gen->mainBody, gen->exprResult.data);
                    add_newLine(&gen->mainBody);
                }
            }
            // clear_Tape( &gen->exprResult );
            break;

        case TK_MUL:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "MULS\n" );
                add_Instruction( &gen->functionBody, "POPS LF@" );
                if(gen->isReturn){
                    add_Instruction( &gen->functionBody, "%retval\n" );
                    add_Instruction( &gen->functionBody, "PUSHS LF@%retval\n");
                }else{
                    add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                    add_newLine( &gen->functionBody );
                    add_Instruction( &gen->functionBody, "PUSHS LF@" );
                    add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                    add_newLine( &gen->functionBody );
                }
            } else {
                if(scope >0 || gen->isWhile) {
                    add_Instruction(&gen->mainBody, "MULS\n");
                    add_Instruction(&gen->mainBody, "POPS LF@");
                    add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                    add_newLine(&gen->mainBody);
                    add_Instruction(&gen->mainBody, "PUSHS LF@");
                    add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                    add_newLine(&gen->mainBody);
                }else{
                    add_Instruction(&gen->mainBody, "MULS\n");
                    add_Instruction(&gen->mainBody, "POPS GF@");
                    add_Instruction(&gen->mainBody, gen->exprResult.data);
                    add_newLine(&gen->mainBody);
                    add_Instruction(&gen->mainBody, "PUSHS GF@");
                    add_Instruction(&gen->mainBody, gen->exprResult.data);
                    add_newLine(&gen->mainBody);
                }
            }
            // clear_Tape( &gen->exprResult );
            break;

        case TK_DIV:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);

            if ( node->resultType== TK_INT ) {  //  aky je rozfiel medzy node->left.token.type a node->resultType??  (node->left.token.type je type tokenu co obsahuje napr priamo konstantu node->resultType je podla semantiky co bi malo byt vysledkom tej operacii
            //Operandy su double
                if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "IDIVS\n" );
                    add_Instruction( &gen->functionBody, "POPS LF@" );
                    if(gen->isReturn){
                        add_Instruction( &gen->functionBody, "%retval\n" );
                        add_Instruction( &gen->functionBody, "PUSHS LF@%retval\n");
                    }else{
                        add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                        add_newLine( &gen->functionBody );
                        add_Instruction( &gen->functionBody, "PUSHS LF@" );
                        add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                        add_newLine( &gen->functionBody );
                    }
                } else {
                    if(scope > 0 || gen->isWhile) {
                        add_Instruction(&gen->mainBody, "IDIVS\n");
                        add_Instruction(&gen->mainBody, "POPS LF@");
                        add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                        add_newLine(&gen->mainBody);
                        add_Instruction(&gen->mainBody, "PUSHS LF@");
                        add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                        add_newLine(&gen->mainBody);
                    }else{
                        add_Instruction(&gen->mainBody, "IDIVS\n");
                        add_Instruction(&gen->mainBody, "POPS GF@");
                        add_Instruction(&gen->mainBody, gen->exprResult.data);
                        add_newLine(&gen->mainBody);
                        add_Instruction(&gen->mainBody, "PUSHS GF@");
                        add_Instruction(&gen->mainBody, gen->exprResult.data);
                        add_newLine(&gen->mainBody);
                    }
                }
                // clear_Tape( &gen->exprResult );
            } else {
            //Operandy su int
                if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "DIVS\n" );
                    add_Instruction( &gen->functionBody, "POPS LF@" );
                    if(gen->isReturn){
                        add_Instruction( &gen->functionBody, "%retval\n" );
                        add_Instruction( &gen->functionBody, "PUSHS LF@%retval\n");
                    }else{
                        add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                        add_newLine( &gen->functionBody );
                        add_Instruction( &gen->functionBody, "PUSHS LF@" );
                        add_Instruction( &gen->functionBody, getActualVariable(gen->exprResult.data,scope,gen));
                        add_newLine( &gen->functionBody );
                    }
                } else {
                    if(scope > 0 || gen->isWhile) {
                        add_Instruction(&gen->mainBody, "DIVS\n");
                        add_Instruction(&gen->mainBody, "POPS LF@");
                        add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                        add_newLine(&gen->mainBody);
                        add_Instruction(&gen->mainBody, "PUSHS LF@");
                        add_Instruction(&gen->mainBody, getActualVariable(gen->exprResult.data, scope, gen));
                        add_newLine(&gen->mainBody);
                    }else{
                        add_Instruction(&gen->mainBody, "DIVS\n");
                        add_Instruction(&gen->mainBody, "POPS GF@");
                        add_Instruction(&gen->mainBody, gen->exprResult.data);
                        add_newLine(&gen->mainBody);
                        add_Instruction(&gen->mainBody, "PUSHS GF@");
                        add_Instruction(&gen->mainBody, gen->exprResult.data);
                        add_newLine(&gen->mainBody);
                    }
                }
                // clear_Tape( &gen->exprResult );
            }
            break;

        case TK_EQ:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "EQS\n" );
                add_Instruction( &gen->functionBody, "POPS GF@&bool\n");            // pop the result in GF@&bool to use in comparisons
            } else {
                add_Instruction( &gen->mainBody, "EQS\n" );
                add_Instruction( &gen->mainBody, "POPS GF@&bool\n" );               // pop the result in GF@&bool to use in comparisons
            }
            break;

        case TK_NEQ:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);

            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "EQS\n" );
                add_Instruction( &gen->functionBody, "POPS GF@&tmp3\n" );
                add_Instruction( &gen->functionBody, "NOT GF@&bool GF@&tmp3\n" );
            } else {
                add_Instruction( &gen->mainBody, "EQS\n" );
                add_Instruction( &gen->mainBody, "POPS GF@&tmp3\n" );
                add_Instruction( &gen->mainBody, "NOT GF@&bool GF@&tmp3\n" );
            }
            break;

        case TK_LT:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);

            if ( gen->isWhile ){
                if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "POPS GF@&tmp1\n" );
                    add_Instruction( &gen->functionBody, "POPS GF@&tmp2\n" );
                    add_Instruction( &gen->functionBody, "LT GF@&bool GF@&tmp1 GF@&tmp2\n" );
                    add_Instruction( &gen->functionBody, "EQ GF@&boolEQ GF@&tmp1 GF@&tmp2\n");
                    add_Instruction( &gen->functionBody, "OR GF@&bool GF@&boolEQ GF@&bool\n");
                } else {
                    add_Instruction( &gen->mainBody, "POPS GF@&tmp1\n" );
                    add_Instruction( &gen->mainBody, "POPS GF@&tmp2\n" );
                    add_Instruction( &gen->mainBody, "LT GF@&bool GF@&tmp1 GF@&tmp2\n" );
                    add_Instruction( &gen->mainBody, "EQ GF@&boolEQ GF@&tmp1 GF@&tmp2\n");
                    add_Instruction( &gen->mainBody, "OR GF@&bool GF@&boolEQ GF@&bool\n");
                }
            } else {
                if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "POPS GF@&tmp2\n" );
                    add_Instruction( &gen->functionBody, "POPS GF@&tmp1\n" );
                    add_Instruction( &gen->functionBody, "LT GF@&bool GF@&tmp1 GF@&tmp2\n" );
                } else {
                    add_Instruction( &gen->mainBody, "POPS GF@&tmp2\n" );
                    add_Instruction( &gen->mainBody, "POPS GF@&tmp1\n" );
                    add_Instruction( &gen->mainBody, "LT GF@&bool GF@&tmp1 GF@&tmp2\n" );   
                }
            }
            break;

        case TK_GT:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);

            if ( gen->isWhile ){
                if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "POPS GF@&tmp1\n" );
                    add_Instruction( &gen->functionBody, "POPS GF@&tmp2\n" );
                    add_Instruction( &gen->functionBody, "GT GF@&bool GF@&tmp1 GF@&tmp2\n" );
                    add_Instruction( &gen->functionBody, "EQ GF@&boolEQ GF@&tmp1 GF@&tmp2\n");
                    add_Instruction( &gen->functionBody, "OR GF@&bool GF@&boolEQ GF@&bool\n");
                } else {
                    add_Instruction( &gen->mainBody, "POPS GF@&tmp1\n" );
                    add_Instruction( &gen->mainBody, "POPS GF@&tmp2\n" );
                    add_Instruction( &gen->mainBody, "GT GF@&bool GF@&tmp1 GF@&tmp2\n" );
                    add_Instruction( &gen->mainBody, "EQ GF@&boolEQ GF@&tmp1 GF@&tmp2\n");
                    add_Instruction( &gen->mainBody, "OR GF@&bool GF@&boolEQ GF@&bool\n");
                }
            } else {
                    if ( inFunc ) {
                    add_Instruction( &gen->functionBody, "POPS GF@&tmp2\n" );
                    add_Instruction( &gen->functionBody, "POPS GF@&tmp1\n" );
                    add_Instruction( &gen->functionBody, "GT GF@&bool GF@&tmp1 GF@&tmp2\n" );
                } else {
                    add_Instruction( &gen->mainBody, "POPS GF@&tmp2\n" );
                    add_Instruction( &gen->mainBody, "POPS GF@&tmp1\n" );
                    add_Instruction( &gen->mainBody, "GT GF@&bool GF@&tmp1 GF@&tmp2\n" );  
                }
            }
            break;

        case TK_LE:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);


            if ( inFunc ) {
                if ( gen->isWhile ) {
                    add_Instruction( &gen->functionBody, "CALL _LE\n" );
                    add_Instruction( &gen->functionBody, "NOT GF@&bool GF@&bool\n");    
                } else {
                    add_Instruction( &gen->functionBody, "CALL _LE\n" );
                }
            } else {
                if ( gen->isWhile ) {
                    add_Instruction( &gen->mainBody, "CALL _LE\n" );
                    add_Instruction( &gen->mainBody, "NOT GF@&bool GF@&bool\n");
                } else {
                    add_Instruction( &gen->mainBody, "CALL _LE\n" );
                }
            }
            break;

        case TK_GE:
            gen_Expr( gen, node->left, inFunc, scope);
            gen_Expr( gen, node->right, inFunc, scope);

            if ( inFunc ) {
                if ( gen->isWhile ) {
                    add_Instruction( &gen->functionBody, "CALL _GE\n" );
                    add_Instruction( &gen->functionBody, "NOT GF@&bool GF@&bool\n");    
                } else {
                    add_Instruction( &gen->functionBody, "CALL _GE\n" );
                }
            } else {
                if ( gen->isWhile ) {
                    add_Instruction( &gen->mainBody, "CALL _GE\n" );
                    add_Instruction( &gen->mainBody, "NOT GF@&bool GF@&bool\n");
                } else {
                    add_Instruction( &gen->mainBody, "CALL _GE\n" );
                }
            }
            break;

        case TK_IDENTIFIER:
            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "PUSHS LF@" );
                char* found = getActualVariable(node->token.data.String,scope,gen);
                add_Instruction( &gen->functionBody, getActualVariable(node->token.data.String,scope,gen));    //PUSHS LF@name\n
                add_newLine( &gen->functionBody );
            } else {
                if(scope > 1 || gen->isWhile){
                    add_Instruction(&gen->mainBody, "PUSHS LF@");
                    char *found = getActualVariable(node->token.data.String,scope,gen);
                    add_Instruction(&gen->mainBody, getActualVariable(node->token.data.String,scope,gen));    //PUSHS LF@name\n
                    add_newLine(&gen->mainBody);
                }else {
                    add_Instruction(&gen->mainBody, "PUSHS GF@");
                    add_Instruction(&gen->mainBody, node->token.data.String);    //PUSHS LF@name\n
                    add_newLine(&gen->mainBody);
                }
            }
            break;

        case TK_DOUBLE:        // double literal

            snprintf(buf, sizeof(buf), "%a", node->token.data.Double );
            if ( inFunc ) {
                if ( node->convertToType == TK_INT ) {
                    add_Instruction( &gen->functionBody, "FLOAT2INT GF@&tmp3 float@");
                    add_Instruction( &gen->functionBody, buf );
                    add_newLine( &gen->functionBody );
                    add_Instruction( &gen->functionBody, "PUSHS GF@&tmp3" );
                    add_newLine( &gen->mainBody );
                } else {
                    add_Instruction( &gen->functionBody, "PUSHS float@" );
                    add_Instruction( &gen->functionBody, buf );
                    add_newLine( &gen->functionBody );
                }
            } else {
                if ( node->convertToType == TK_INT ) {
                    add_Instruction( &gen->mainBody, "FLOAT2INT GF@&tmp3 float@");
                    add_Instruction( &gen->mainBody, buf );
                    add_newLine( &gen->mainBody );
                    add_Instruction( &gen->mainBody, "PUSHS GF@&tmp3" );
                    add_newLine( &gen->mainBody );
                } else {
                    add_Instruction( &gen->mainBody, "PUSHS float@" );
                    add_Instruction( &gen->mainBody, buf );
                    add_newLine( &gen->mainBody );
                }
            }
            break;

        case TK_INT:           // integer literal

            snprintf(buf, sizeof(buf), "%lld", node->token.data.Int );
            if ( inFunc ) {
                if ( node->convertToType == TK_DOUBLE ) {
                    add_Instruction( &gen->functionBody, "INT2FLOAT GF@&tmp3 int@");
                    add_Instruction( &gen->functionBody, buf );
                    add_newLine( &gen->functionBody );
                    add_Instruction( &gen->functionBody, "PUSHS GF@&tmp3" );
                    add_newLine( &gen->mainBody );
                } else {
                    add_Instruction( &gen->functionBody, "PUSHS int@" );
                    add_Instruction( &gen->functionBody, buf );
                    add_newLine( &gen->functionBody );
                }
            } else {
                if ( node->convertToType == TK_DOUBLE )
                {
                    add_Instruction( &gen->mainBody, "INT2FLOAT GF@&tmp3 int@");
                    add_Instruction( &gen->mainBody, buf );
                    add_newLine( &gen->mainBody );
                    add_Instruction( &gen->mainBody, "PUSHS GF@&tmp3" );
                    add_newLine( &gen->mainBody );
                } else {
                    add_Instruction( &gen->mainBody, "PUSHS int@" );
                    add_Instruction( &gen->mainBody, buf );
                    add_newLine( &gen->mainBody );
                }
            }
            break;

        case TK_STRING:        // string literal
        case TK_MLSTRING:      // multi-line string literal
            if ( inFunc ) {
                add_Instruction( &gen->functionBody, "PUSHS string@" );
                add_Instruction( &gen->functionBody, gen_convertString( node->token.data.String ) );
                add_newLine( &gen->functionBody );
            } else {
                add_Instruction( &gen->mainBody, "PUSHS string@" );
                add_Instruction( &gen->mainBody, gen_convertString( node->token.data.String ) );
                add_newLine( &gen->mainBody );
            }
            break;

        case TK_COALESCE:
            gen_Expr( gen, node->right, inFunc, scope);
            gen_Expr( gen, node->left, inFunc, scope);

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
                                      "CREATEFRAME\n"
                                      "PUSHFRAME\n"
                                      "DEFVAR LF@right\n"
                                      "DEFVAR LF@left\n"
                                      "DEFVAR LF@result\n"
                                      "POPS LF@right\n"
                                      "POPS LF@left\n"
                                      "LT LF@result LF@left LF@right\n"
                                      "JUMPIFEQ ?is_true1 LF@result bool@true\n"
                                      "EQ LF@result LF@left LF@right\n"
                                      "JUMPIFEQ ?is_true1 LF@result bool@true\n"
                                      "MOVE GF@&bool bool@false\n"
                                      "JUMP ?end_LE\n"
                                      "label ?is_true1\n"
                                      "MOVE GF@&bool bool@true\n"
                                      "LABEL ?end_LE\n"
                                      "POPFRAME\n"
                                      "RETURN\n"
                                      "LABEL _skip_LE\n"
    );
}

void gen_GE( generator_t* gen ) {
    add_Instruction( &gen->functions, "JUMP _skip_GE\n"
                                      "LABEL _GE\n"
                                      "CREATEFRAME\n"
                                      "PUSHFRAME\n"
                                      "DEFVAR LF@right\n"
                                      "DEFVAR LF@left\n"
                                      "DEFVAR LF@result\n"
                                      "POPS LF@right\n"
                                      "POPS LF@left\n"
                                      "GT LF@result LF@left LF@right\n"
                                      "JUMPIFEQ ?is_false2 LF@result bool@true\n"
                                      "EQ LF@result LF@left LF@right\n"
                                      "JUMPIFEQ ?is_false2 LF@result bool@true\n"
                                      "MOVE GF@&bool bool@false\n"
                                      "JUMP ?end_GE\n"
                                      "label ?is_false2\n"
                                      "MOVE GF@&bool bool@true\n"
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
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@%retval\n"
                                    "READ LF@tmp1 string\n"
                                    "MOVE LF@%retval LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_readString\n"
                                    );
}

void gen_buildin_readInt(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_readInt\n"
                                    "LABEL readInt\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@type\n"
                                    "DEFVAR LF@%retval\n"
                                    "READ LF@tmp1 int\n"
                                    "TYPE LF@type LF@tmp1\n"                           //check the type of input
                                    "JUMPIFNEQ _retnil1 LF@type string@int\n"           //If the input is not int, return nil
                                    "MOVE LF@%retval LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retnil1\n"
                                    "MOVE LF@%retval nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_readInt\n"
                                    );
}

void gen_buildin_readDouble(generator_t* gen){
    
    add_Instruction(&gen->functions, "JUMP _skip_readDouble\n"
                                    "LABEL readDouble\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@type\n"
                                    "DEFVAR LF@%retval\n"
                                    "READ LF@tmp1 float\n"
                                    "TYPE LF@type LF@tmp1\n"
                                    "JUMPIFNEQ _retnil2 LF@type string@float\n"
                                    "MOVE LF@%retval LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retnil2\n"
                                    "MOVE LF@%retval nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_readDouble\n"
                                    );
}

void gen_buildin_Int2Double(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_Int2Double\n"
                                    "LABEL Int2Double\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp2\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@%retval\n"
                                    "MOVE LF@tmp2 LF@%1\n"
                                    "INT2FLOAT LF@tmp1 LF@tmp2\n"
                                    "MOVE LF@%retval LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_Int2Double\n" 
                                    );
}

void gen_buildin_Double2Int(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_Double2Int\n"
                                    "LABEL Double2Int\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@tmp2\n"
                                    "DEFVAR LF@%retval\n"
                                    "MOVE LF@tmp2 LF@%1\n"
                                    "FLOAT2INT LF@tmp1 LF@tmp2\n"
                                    "MOVE LF@%retval LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_Double2Int\n"
                                    );
}

void gen_buildin_length(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_length\n"
                                    "LABEL length\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@len\n"
                                    "DEFVAR LF@str\n"
                                    "DEFVAR LF@%retval\n"
                                    "MOVE LF@str LF@%1\n"
                                    "STRLEN LF@len LF@str\n"
                                    "MOVE LF@%retval LF@len\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_length\n"
                                    );
}

/*########################## Doriesit j otazku #############################*/
void gen_buildin_substring(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_substring\n"
                                    "LABEL substring\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@j\n"
                                    "DEFVAR LF@i\n"
                                    "DEFVAR LF@str\n"
                                    "DEFVAR LF@len\n"                           //define var for length of string
                                    "DEFVAR LF@%retval\n"
                                    "MOVE LF@j LF@%3\n"                               //get j
                                    "MOVE LF@i LF@%2\n"                              //get i
                                    "MOVE LF@str LF@%1\n"                       //get string from stack
                                    "STRLEN LF@len LF@str\n"                    //get the length of string
                                    "DEFVAR LF@cmp\n"                           //help var to store result of comparison
                                    "#Guard functionality\n"
                                    "LT LF@cmp LF@i int@0\n"                    //Compare = i < 0
                                    "JUMPIFEQ _retnil3 LF@cmp bool@true\n"       //jump to retnil if i < 0
                                    "LT LF@cmp LF@j int@0\n"                    //compare = j < 0
                                    "JUMPIFEQ _retnil3 LF@cmp bool@true\n"       //jump to retnil if j < 0
                                    "GT LF@cmp LF@i LF@j\n"                     //compare = i > j
                                    "JUMPIFEQ _retnil3 LF@cmp bool@true\n"       //jump to retnil if j < j
                                    "EQ LF@cmp LF@i LF@len\n"                   //compare = i = len
                                    "JUMPIFEQ _retnil3 LF@cmp bool@true\n"       //jump to retnil if i = len
                                    "GT LF@cmp LF@i LF@len\n"                   //compare = i > len
                                    "JUMPIFEQ _retnil3 LF@cmp bool@true\n"       //jump to retnil if j > len
                                    "GT LF@cmp LF@j LF@len\n"                   //compare = j > len
                                    "JUMPIFEQ _retnil3 LF@cmp bool@true\n"       //jump to retnil if j > len
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
                                    "MOVE LF@%retval LF@resultString\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retnil3\n"
                                    "MOVE LF@%retval nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_substring\n"
                                    );
}                      

void gen_buildin_ord(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_ord\n"
                                    "LABEL ord\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@string\n"
                                    "DEFVAR LF@%retval\n"
                                    "MOVE LF@string LF@%1\n"
                                    "DEFVAR LF@c\n"                             //var to store first char of string
                                    "DEFVAR LF@len\n"                           //var to store length of string
                                    "STRLEN LF@len LF@string\n"                 //get the length of string
                                    "JUMPIFEQ _retzero LF@len int@0\n"          //if length of string is 0, return 0
                                    "STRI2INT LF@c LF@string int@0\n"           //get the char        
                                    "MOVE LF@%retval LF@c\n"                              //return char
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retzero\n"
                                    "MOVE LF@%retval int@0\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_ord\n"
                                    );
}

void gen_buildin_chr(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_chr\n"
                                    "LABEL chr\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@c\n"
                                    "DEFVAR LF@i\n"
                                    "DEFVAR LF@%retval\n"
                                    "MOVE LF@i LF@%1\n"
                                    "INT2CHAR LF@c LF@i\n"                      //covert int to ascii char
                                    "MOVE LF@%retval LF@c\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_chr\n"
                                    );
}

