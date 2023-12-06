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

/**
 * @brief initializes a new instruction tape
 * @return new instruction tape
*/
instructionTape_t init_Tape();

/**
 * @brief adds a newline to a instruction tape
 * @param tape
*/
void add_newLine( instructionTape_t* tape );

/**
 * @brief adds an integer to a instruction tape
 * @param tape
 * @param n integer to be added
*/
void add_Int( instructionTape_t* tape, int n );

/**
 * @brief adds a char to a instruction tape
 * @param taoe
 * @param c char to be added
*/
void add_Char( instructionTape_t* tape, char c );

/**
 * @bired adds a double to a instruction tape
 * @param tape
 * @param d double to be added
*/
void add_Double( instructionTape_t* tape, double d );

/**
 * @brief adds a new instruction to a instruction tape.
 * @param instruction
*/
void add_Instruction( instructionTape_t* tape, char* instruction );

/**
 * @brief clears and initializes an instruction tape
 * @param tape
*/
void clear_Tape( instructionTape_t* tape );

/**
 * @brief prints instructions on a instruction tape
 * @param tape
*/
void print_Intructions( instructionTape_t* tape );

#endif //INSTRUCTION_TAPE