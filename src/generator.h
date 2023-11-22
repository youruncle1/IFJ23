
#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "instructions.h"


typedef struct{
    instructionTape_t mainBody;
    instructionTape_t header;
}generator_t;

generator_t gen_Init();

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
