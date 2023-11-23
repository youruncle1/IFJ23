
#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "instructions.h"
#include "parser.h"
// #include "expressions.h"        //Tu bude includovany expression parser ktory bude mat ASS


typedef struct{
    instructionTape_t mainBody;
    instructionTape_t header;
    instructionTape_t functions;
    instructionTape_t functionHead;
    instructionTape_t functionBody;
    instructionTape_t functionFoot;
    instructionTape_t functionName;
}generator_t;

generator_t gen_Init();

void gen_VarDefinition( generator_t* gen, parser_t* parser );

void gen_FunctionHeader( generator_t* gen, parser_t* parser );

void gen_FunctionFooter( generator_t* gen, parser_t* parser );

void gen_FunctionCall( generator_t* gen, parser_t* parser );

void gen_FunctionParam( generator_t* gen, parser_t* parser );

void gen_Function( generator_t* gen );

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
