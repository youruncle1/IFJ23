#include <stdlib.h>
#include <stdio.h>
#include "generator.h"
#include "instructions.h"


int main(){
    instructionTape_t tape = init_Tape();
    generator_t gen = gen_Init();

    // gen_buildin_readString(&gen);

    // gen_buildin_readInt(&gen);

    // gen_buildin_readDouble(&gen);

    // gen_buildin_write(&gen);

    // gen_buildin_Int2Double(&gen);

    // gen_buildin_Double2Int(&gen);

    // gen_buildin_length(&gen);

    // gen_buildin_substring(&gen);

    // gen_buildin_ord(&gen);

    // gen_buildin_chr(&gen);

    add_Instruction( &gen.functionHead, "HEAD\n" );
    add_Instruction( &gen.functionFoot, "FOOT\n" );
    add_Instruction( &gen.functionBody, "BODY\n");
    add_Int( &gen.functionFoot, 87 );
    gen_Function( &gen );

    print_Code(&gen);
}