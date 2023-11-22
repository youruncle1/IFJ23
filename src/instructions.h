#ifndef INSTRUCTION_TAPE
#define INSTRUCTION_TAPE

#include <stdlib.h>





typedef struct{
    size_t len;             //length of tape
    size_t capacity;        //total capacity of tape
    char* data;             //instructions
}instructionTape_t;


#define INIT_CAPACITY 10

instructionTape_t init_Tape();

void add_Instruction(instructionTape_t* tape, char* instruction);

void print_Intructions(instructionTape_t* tape);

#endif //INSTRUCTION_TAPE