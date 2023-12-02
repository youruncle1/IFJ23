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


instructionTape_t init_Tape() {
    instructionTape_t tape;
    tape.capacity = INIT_CAPACITY;
    tape.len = 0;
    tape.data = (char*)malloc(tape.capacity * sizeof(char));
    tape.data[0] = '\0';

    return tape;
}

void add_Instruction(instructionTape_t* tape, char* instruction) {
    size_t newLen = tape->len + strlen(instruction);

    if (newLen >= tape->capacity) {
        // Zvětšíme kapacitu podle velikosti nové instrukce
        while (newLen >= tape->capacity) {
            tape->capacity += strlen(instruction);
        }

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

void add_Double( instructionTape_t* tape, double d ) {
    
    char doubletostr[40];
    sprintf( doubletostr, "%a", d );
    add_Instruction( tape, doubletostr );
}

void add_Int( instructionTape_t* tape, int n ) {

    char intostr[20];
    sprintf( intostr, "%d", n );
    add_Instruction( tape, intostr );
}

void add_Char( instructionTape_t* tape, char c ) {
    char str[2];
    str[0] = c;
    str[1] = '\0';
    add_Instruction( tape, str );
}

void clear_Tape( instructionTape_t* tape ) {
    if(tape->data == NULL){
        return;
    }
    free( tape->data );
    tape->len = 0;
    tape->capacity = INIT_CAPACITY;
    tape->data = (char*)malloc(INIT_CAPACITY * sizeof(char));
    tape->data[0] = '\0';
}


void print_Intructions(instructionTape_t* tape){
    
    add_Instruction(tape, "\0");

    fprintf(stdout, "%s", tape->data);
}
