#include <stdlib.h>
#include <stdio.h>
#include "instructions.h"
#include <string.h>


instructionTape_t init_Tape(){
    instructionTape_t tape;
    tape.capacity = INIT_CAPACITY;
    tape.len = 0;
    tape.data = (char*)malloc(INIT_CAPACITY * sizeof(char));

    return tape;
}

void add_Instruction(instructionTape_t* tape, char* instruction){
    
    size_t newLen = tape->len + strlen(instruction);

    if (newLen > tape->capacity){
        tape->capacity = (tape->capacity + newLen) * 2;
        tape->data = (char*)realloc(tape->data, tape->capacity * sizeof(char));
    }
    strcat(tape->data, instruction);
    tape->len = newLen;
}


void print_Intructions(instructionTape_t* tape){
    add_Instruction(tape, "\0");

    printf("%s", tape->data);
}