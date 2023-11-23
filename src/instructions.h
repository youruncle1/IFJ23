#ifndef INSTRUCTION_TAPE
#define INSTRUCTION_TAPE

#include <stdlib.h>





typedef struct{
    int len;             //length of tape
    int capacity;        //total capacity of tape
    char* data;             //instructions
}instructionTape_t;


#define INIT_CAPACITY 100

instructionTape_t init_Tape();

void add_newLine( instructionTape_t* tape );

void add_Instruction( instructionTape_t* tape, char* instruction );

void print_Intructions( instructionTape_t* tape );

#endif //INSTRUCTION_TAPE