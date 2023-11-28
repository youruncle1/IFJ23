/*
IFJ PROJEKT 2023/2024
file: "instructions.c"

Code tape

authors: xrusna08

*/

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
    
    //if instruction is longer that the tape capacity expand tape capacity
    size_t newLen = tape->len + strlen(instruction);

    if (newLen >= tape->capacity){
        tape->capacity = (tape->capacity + newLen) * 2;
        tape->data = (char*)realloc(tape->data, tape->capacity * sizeof(char));
    }
    strcat(tape->data, instruction);
    tape->len = newLen;
}

void add_newLine( instructionTape_t* tape ) {
    
    //if newline is outside of capacity, expand the capacity
    size_t plus_new_line = tape->len + 1;

    if ( plus_new_line >= tape->capacity ){
        tape->capacity = (tape->capacity + plus_new_line) * 2;
        tape->data = (char*)realloc(tape->data, tape->capacity * sizeof(char));   
    }
    strcat( tape->data, "\n" );
    tape->len = plus_new_line;
}

void add_Int( instructionTape_t* tape, int n ) {

    //if length of tape plus n is > capacity, expand capacity
    char intostr[20];
    sprintf( intostr, "%d", n );
    add_Instruction(tape, intostr);
}


void print_Intructions(instructionTape_t* tape){
    
    // tape->capacity += 1;
    // tape->len += 1;
    add_Instruction(tape, "\0");

    fprintf(stdout, "%s", tape->data);
}