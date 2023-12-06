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
    bool isWhile;
    char** localFrame;
    int local_frame_size;

}generator_t;

/**
 * @brief initializes the generator
 * @return initialized generator
*/
generator_t gen_Init();

/**
 * @brief generates the header of IFJcode23 program
 * @param generator
*/
void gen_Header(generator_t* gen);

/**
 * @brief generates all inbuild functions at the top of IFJcode23 program
 * @param generator
*/
void gen_inbuild(generator_t* gen);

/**
 * @brief handles var definition
 * @param generator
 * @param name of a new variable
 * @param inFunc to know where to generate the definition
*/
void gen_VarDefinition( generator_t* gen, char* name, bool inFunc, int scope);

/**
 * @brief assings value to a variable
 * @param generator
 * @param varName name of variable
 * @param val new value of variable
 * @param inFunc to know where to generate the definition
 * @param type type of the variablr
*/
void gen_AssignVal( generator_t* gen, char* varName,  char* val, bool inFunc, char* type, int scope);

/**
 * @brief generates the start of the function
 * @param generator
 * @param funcName name of a function
 * @param function
*/
void gen_FunctionHeader( generator_t* gen, char* funcName, Node* function ,Node* globalFrame, int scope);

/**
 * @brief decides what literal to generate with gen_FunctionParam
 * @param generator
 * @param token
*/
void gen_LiteralReturn(generator_t* gen, token_t token);

/**
 * @brief generates a code to move result into result variable
 * @param tokenToAssing
 * @param inFunc
*/
void gen_AssignReturnToVariable(generator_t* gen, token_t tokenToAssign, bool inFunc);

/**
 * @brief generates code when returning variablr
 * @param token
 * @param scope
*/
void gen_IdentifierReturn(generator_t* gen,token_t token, int scope);

/**
 * @brief generates the footer of functions
 * @param generator
*/
void gen_FunctionFooter( generator_t* gen);

/**
 * @brief generates function call
 * @param generator
 * @param funcName
 * @param inFunc
*/
void gen_FunctionCall( generator_t* gen, char* funcName, bool inFunc );

/**
 * @brief generates the parameters of the function, if the function is write, it simply uses IFJcode23 instruction WRITE to print the parameter
 * @param param parameter to generate
 * @param inFunc
 * @param paramCount
 * @param scope
*/
void gen_FunctionParam( generator_t* gen, char* param, bool inFunc, int paramCount, int scope);

/**
 * @brief creates a new frame
 * @param generator
 * @param inFunc
*/
void gen_CreateFrame( generator_t* gen, bool inFunc );

/**
 * @brief generates function parameter when is int
 * @param val value of the parameter
 * @param inFunc
 * @param paramCount
*/
void gen_FunctionParamInt( generator_t* gen, long val, bool inFunc, int paramCount);

/**
 * @brief generates function parameter when is double
 * @param val
 * @param inFunc
 * @param paramCount
*/
void gen_FunctionParamDouble( generator_t* gen, double val, bool inFunc, int paramCount);

/**
 * @brief convers a C string into IFJcode23 sting
 * @param string
 * @return new converted string
*/
char* gen_convertString( char* string );

/**
 * @brief generates function parameter when is string
 * @param str
 * @param inFunc
 * @param paramCount
*/
void gen_FunctionParamString( generator_t* gen, char* str, bool inFunc, int paramCount);

/**
 * @brief this function is not used
*/
void gen_FunctionParamNil( generator_t* gen, bool inFunc );

/**
 * @brief
 * @param generator
*/
void gen_Function( generator_t* gen );

void addNonFunctionSymbolsFromGlobal(Node *root, generator_t* gen, int scope, bool isFunc);

char* getActualVariable(char* key,int scope,generator_t* gen);

void copyVariables(generator_t* gen, bool inFunc);

void addToLocalFrame(char* key,int scope, generator_t* gen);

// void gen_IfThenElse( generator_t* gen, unsigned int scopeDepth, bool inFunc);
void gen_IfThenElse( generator_t* gen, unsigned int scopeDepth, bool inFunc, Node* globalFrame);

void gen_IfDone( generator_t* gen, unsigned int scopeDepth, bool inFunc );

void gen_IfDone_End( generator_t* gen, unsigned int scopeDepth, bool inFunc );

void gen_IfThenElse_End( generator_t* gen, unsigned int scopeDepth, bool inFunc, Node* globalFrame);

void gen_While( generator_t* gen, unsigned int scopeDepth, bool inFunc, Node* globalFrame);

void gen_WhileCond( generator_t* gen, unsigned int scopeDepth, bool inFunc );

void gen_WhileEnd( generator_t* gen, unsigned int scopeDepth, bool inFunc );

// void gen_WriteParams( generator_t* gen, tk_type_t type, char* name, bool inFunc );

void gen_SaveExprResult( generator_t* gen, char* name );

void gen_ClearExprResult( generator_t* gen, bool inFunc );

//void gen_Expr( generator_t* gen, ASTNode* node, bool inFunc );

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
