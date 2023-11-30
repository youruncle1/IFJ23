/*
IFJ PROJEKT 2023/2024
file: "instructions.h"

Code tape - header file

authors: xrusna08 

*/

#ifndef INSTRUCTION_TAPE
#define INSTRUCTION_TAPE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>




typedef struct{
    size_t len;             //length of tape
    size_t capacity;        //total capacity of tape
    char* data;             //instructions
}instructionTape_t;


#define INIT_CAPACITY 100

instructionTape_t init_Tape();

void add_newLine( instructionTape_t* tape );

void add_Int( instructionTape_t* tape, int n );

void add_Instruction( instructionTape_t* tape, char* instruction );

void clear_Tape( instructionTape_t* tape );

void print_Intructions( instructionTape_t* tape );

#endif //INSTRUCTION_TAPE