/*
IFJ PROJEKT 2023/2024
file: "generator.h"

Code generator - header file

authors: xrusna08 

*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdbool.h>
#include "instructions.h"
#include "symtable.h"
#include "scanner.h"


typedef struct{
    instructionTape_t mainBody;
    instructionTape_t header;
    instructionTape_t functions;
    instructionTape_t functionHead;
    instructionTape_t functionBody;
    instructionTape_t functionFoot;
    instructionTape_t functionName;
    instructionTape_t varName;
    instructionTape_t stringParam;
    instructionTape_t exprResult;
    int selectCount;
    int iterCount;
    int isWrite;
    int paramCount;
    bool isReturn;

}generator_t;

generator_t gen_Init();

void gen_Header(generator_t* gen);

void gen_inbuild(generator_t* gen);

void gen_VarDefinition( generator_t* gen, char* name, bool inFunc );

void gen_FunctionHeader( generator_t* gen, char* funcName,Node* function);

void gen_LiteralReturn(generator_t* gen,token_t token);

void gen_AssignReturnToVariable(generator_t* gen,token_t tokenToAssign,bool inFunc);

void gen_IdentifierReturn(generator_t* gen,token_t token);

void gen_FunctionFooter( generator_t* gen);

void gen_FunctionCall( generator_t* gen, char* funcName, bool inFunc );

void gen_FunctionParam( generator_t* gen, char* param, bool inFunc, int paramCount);

void gen_CreateFrame( generator_t* gen, bool inFunc );

void gen_FunctionParamInt( generator_t* gen, long val, bool inFunc, int paramCount);

void gen_FunctionParamDouble( generator_t* gen, double val, bool inFunc, int paramCount);

char* gen_convertString( char* string );

void gen_FunctionParamString( generator_t* gen, char* str, bool inFunc, int paramCount);

void gen_FunctionParamNil( generator_t* gen, bool inFunc );

void gen_Function( generator_t* gen );

void gen_AssignVal( generator_t* gen, char* varName,  char* val, bool inFunc, char* type );

void gen_IfThenElse( generator_t* gen, unsigned int scopeDepth, bool inFunc);

void gen_IfDone( generator_t* gen, unsigned int scopeDepth, bool inFunc );

void gen_IfDone_End( generator_t* gen, unsigned int scopeDepth, bool inFunc );

void gen_IfThenElse_End( generator_t* gen, unsigned int scopeDepth, bool inFunc );

void gen_While( generator_t* gen, unsigned int scopeDepth, bool inFunc );

void gen_WhileCond( generator_t* gen, unsigned int scopeDepth, bool inFunc );

void gen_WhileEnd( generator_t* gen, unsigned int scopeDepth, bool inFunc );

// void gen_WriteParams( generator_t* gen, tk_type_t type, char* name, bool inFunc );

void gen_SaveExprResult( generator_t* gen, char* name );

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
