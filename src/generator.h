#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdbool.h>
#include "instructions.h"



typedef struct{
    instructionTape_t mainBody;
    instructionTape_t header;
    instructionTape_t functions;
    instructionTape_t functionHead;
    instructionTape_t functionBody;
    instructionTape_t functionFoot;
    instructionTape_t functionName;
    instructionTape_t varName;
    int selectCount;

    int iterCount;


}generator_t;

generator_t gen_Init();

void gen_Header(generator_t* gen);

void gen_inbuild(generator_t* gen);

void gen_VarDefinition( generator_t* gen, char* name, bool inFunc );

void gen_FunctionHeader( generator_t* gen, char* funcName );

void gen_FunctionFooter( generator_t* gen);

void gen_FunctionCall( generator_t* gen, char* funcName, bool inFunc );

void gen_FunctionParam( generator_t* gen, char* param, bool inFunc );

void gen_Function( generator_t* gen );

void gen_AssignVal( generator_t* gen, char* val, bool inFunc, char* type );

void gen_IfThenElse( generator_t* gen, bool inFunc);

void gen_IfDone( generator_t* gen, bool inFunc );

void gen_IfDone_End( generator_t* gen, bool inFunc );

void gen_IfThenElse_End( generator_t* gen, bool inFunc );

void gen_While( generator_t* gen, bool inFunc );

void gen_WhileCond( generator_t* gen, bool inFunc );

void gen_WhileEnd( generator_t* gen, bool inFunc );

// void gen_Expr( generator_t* gen, ASTNode* node, bool inFunc );

void gen_LE( generator_t* gen );

void gen_COALESCE( generator_t* gen );

void gen_GE( generator_t* gen );

void gen_buildin_readString(generator_t* gen);

void gen_buildin_readInt(generator_t* gen);

void gen_buildin_readDouble(generator_t* gen);

void gen_buildin_write(generator_t* gen);

void gen_buildin_Int2Double(generator_t* gen);

void gen_buildin_Double2Int(generator_t* gen);

void gen_buildin_length(generator_t* gen);

void gen_buildin_substring(generator_t* gen);

void gen_buildin_ord(generator_t* gen);

void gen_buildin_chr(generator_t* gen);

void print_Code(generator_t* gen);

#endif //GENERATOR_H
