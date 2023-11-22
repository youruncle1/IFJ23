#include <stdlib.h>
#include <string.h>
#include "generator.h"


generator_t gen_Init(){
    generator_t gen;
    gen.functions = init_Tape();
    gen.mainBody = init_Tape();
    gen.header = init_Tape();
    add_Instruction(&gen.header, ".IFJcode23\n");   //every IFJcode23 needs to start with .IFJcode23 header
    return gen;
}

void gen_VarDefinition( generator_t* gen, token_t token , bool inFunction) {

    //Is token in global or local frame
    if( inFunction ) {
        add_Instruction( &gen->functionBody, "DEFVAR LF@" );
        add_Instruction( &gen->functionBody, token.data.String );      //add the variable name to definition
        add_newLine( &gen->functionBody );                                             //append new line
    } else {
        add_Instruction( &gen->mainBody, "DEFVAR GF@" );
        add_Instruction( &gen->mainBody, token.data.String );       //add the variable name to definition
        add_newLine( &gen->mainBody );                                              //append new line
    }
                                                  
}

void gen_Label( generator_t* gen, token_t* token ) {
    return;
}

void gen_FunctionHeader( generator_t* gen, token_t token ) {
    
    add_Instruction( &gen->functionHead, "JUMP _skip_" );
    add_Instruction( &gen->functionHead, token.data.String );
    add_newLine( &gen->functionHead );
    add_Instruction( &gen->functionHead, "LABEL ");
    add_Instruction( &gen->functionHead, token.data.String );
    add_newLine( &gen->functionHead );
    add_Instruction( &gen->functionHead, "CREATEFRAME\n"
                                         "PUSHFRAME\n"
                                         );
}

void gen_FunctionFooter( generator_t* gen, token_t token ) {
    
    add_Instruction( &gen->functionFoot, "POPFRAME\n"
                                         "RETURN\n"
                                         );
    
    add_Instruction( &gen->functionFoot, "LABEL _skip_" );
    add_Instruction( &gen->functionFoot, token.data.String );
    add_newLine( &gen->functionFoot );
}

void gen_FunctionCall( generator_t* gen, token_t token, bool inFunction ) {
    
    if ( inFunction ) {
        add_Instruction( &gen->functionBody, "CALL ");
        add_Instruction( &gen->functionBody, token.data.String );
        add_newLine( &gen->functionBody );
    } else {
        add_Instruction( &gen->mainBody, "CALL ");
        add_Instruction( &gen->mainBody, token.data.String );
        add_newLine( &gen->mainBody );
    }
}

void gen_FunctionParams( generator_t* gen, token_t token, bool inFunction ) {
    
}







void print_Code(generator_t* gen){
    print_Intructions(&gen->header);
    print_Intructions(&gen->functions);
    print_Intructions(&gen->mainBody);
}



/*
##########################################################################################################
##########################################################################################################
################################ functions to generate inbuild functions #################################
##########################################################################################################
##########################################################################################################
*/


void gen_buildin_readString(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_readString\n"
                                    "LABEL readString\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "READ LF@tmp1 string\n"
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_readString\n"
                                    );
}

void gen_buildin_readInt(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_readInt\n"
                                    "LABEL readInt\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@type\n"
                                    "READ LF@tmp1 int\n"
                                    "TYPE LF@type LF@tmp1\n"                           //check the type of input
                                    "JUMPIFNEQ _retnil LF@type string@int\n"           //If the input is not int, return nil
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retnil\n"
                                    "PUSHS nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_readInt\n"
                                    );
}

void gen_buildin_readDouble(generator_t* gen){
    
    add_Instruction(&gen->functions, "JUMP _skip_readDouble\n"
                                    "LABEL readDouble\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@type\n"
                                    "READ LF@tmp1 float\n"
                                    "TYPE LF@type LF@tmp1\n"
                                    "JUMPIFNEQ _retnil LF@type string@float\n"
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retnil\n"
                                    "PUSHS nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_readDouble\n"
                                    );
}
/*#################### termy budu musiet byt napushovane odzadu ####################*/
void gen_buildin_write(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_write\n"
                                    "LABEL write\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@term\n"
                                    "DEFVAR LF@termCount\n"
                                    "POPS LF@termCount\n"
                                    "LABEL _writeTerms\n"
                                    "JUMPIFEQ _writeEnd LF@termCount int@0\n"
                                    "SUB LF@termCount LF@termCount int@1\n"
                                    "POPS LF@term\n"
                                    "WRITE LF@term\n"
                                    "JUMP _writeTerms\n"
                                    "LABEL _writeEnd\n"
                                    "PUSHS nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_write\n"
                                    );
}

void gen_buildin_Int2Double(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_Int2Double\n"
                                    "LABEL Int2Double\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp2\n"
                                    "DEFVAR LF@tmp1\n"
                                    "POPS LF@tmp2\n"
                                    "INT2FLOAT LF@tmp1 LF@tmp2\n"
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_Int2Double\n" 
                                    );
}

void gen_buildin_Double2Int(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_Double2Int\n"
                                    "LABEL Double2Int\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@tmp1\n"
                                    "DEFVAR LF@tmp2\n"
                                    "POPS LF@tmp2\n"
                                    "FLOAT2INT LF@tmp1 LF@tmp2\n"
                                    "PUSHS LF@tmp1\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_Double2Int\n"
                                    );
}

void gen_buildin_length(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_length\n"
                                    "LABEL length\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@len\n"
                                    "DEFVAR LF@str\n"
                                    "POPS LF@str\n"
                                    "STRLEN LF@len LF@str\n"
                                    "PUSHS LF@len\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_length\n"
                                    );
}

/*########################## Doriesit j otazku #############################*/
void gen_buildin_substring(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_substring\n"
                                    "LABEL substring\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@j\n"
                                    "DEFVAR LF@i\n"
                                    "DEFVAR LF@str\n"
                                    "DEFVAR LF@len\n"                           //define var for length of string
                                    "POPS LF@j\n"                               //get j
                                    "POPS LF@i\n"                               //get i
                                    "POPS LF@str\n"                             //get string from stack
                                    "STRLEN LF@len LF@str\n"                    //get the length of string
                                    "DEFVAR LF@cmp\n"                           //help var to store result of comparison
                                    "#Guard functionality\n"
                                    "LT LF@cmp LF@i int@0\n"                    //Compare = i < 0
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if i < 0
                                    "LT LF@cmp LF@j int@0\n"                    //compare = j < 0
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if j < 0
                                    "GT LF@cmp LF@i LF@j\n"                     //compare = i > j
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if j < j
                                    "EQ LF@cmp LF@i LF@len\n"                   //compare = i = len
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if i = len
                                    "GT LF@cmp LF@i LF@len\n"                   //compare = i > len
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if j > len
                                    "GT LF@cmp LF@j LF@len\n"                   //compare = j > len
                                    "JUMPIFEQ _retnil LF@cmp bool@true\n"       //jump to retnil if j > len
                                    "#Making of substring\n"
                                    "DEFVAR LF@c\n"                             //here the chars of substring will be stored
                                    "DEFVAR LF@resultString\n"
                                    "MOVE LF@resultString string@\n"            //initiate resultString to ""
                                    "LABEL _writeSubstring\n"
                                    "JUMPIFEQ _finished LF@i LF@j\n"
                                    "GETCHAR LF@c LF@str LF@i\n"
                                    "CONCAT LF@resultString LF@resultString LF@c\n"
                                    "ADD LF@i LF@i int@1\n"                     //i++;
                                    "JUMP _writeSubstring\n"
                                    "LABEL _finished\n"
                                    "PUSHS LF@resultString\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retnil\n"
                                    "PUSHS nil@nil\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_substring\n"
                                    );
}                      

void gen_buildin_ord(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_ord\n"
                                    "LABEL ord\n"  
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@string\n"
                                    "POPS LF@string\n"
                                    "DEFVAR LF@c\n"                             //var to store first char of string
                                    "DEFVAR LF@len\n"                           //var to store length of string
                                    "STRLEN LF@len LF@string\n"                 //get the length of string
                                    "JUMPIFEQ _retzero LF@len int@0\n"          //if length of string is 0, return 0
                                    "STRI2INT LF@c LF@string int@0\n"           //get the char        
                                    "PUSHS LF@c\n"                              //return char
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _retzero\n"
                                    "PUSHS int@0\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_ord\n"
                                    );
}

void gen_buildin_chr(generator_t* gen){

    add_Instruction(&gen->functions, "JUMP _skip_chr\n"
                                    "LABEL chr\n"
                                    "CREATEFRAME\n"
                                    "PUSHFRAME\n"
                                    "DEFVAR LF@c\n"
                                    "DEFVAR LF@i\n"
                                    "POPS LF@i\n"
                                    "INT2CHAR LF@c LF@i\n"                      //covert int to ascii char
                                    "PUSHS LF@c\n"
                                    "POPFRAME\n"
                                    "RETURN\n"
                                    "LABEL _skip_chr\n"
                                    );
}

