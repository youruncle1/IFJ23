/*
IFJ PROJEKT 2023/2024
file: "src/scanner.c"

Lexical analysis

authors: xpolia05
         xbencs00
         xrusna08
         xhonze01
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "scanner.h"
#include "parser.h"
#include "error.h"

typedef enum {
    TOKEN_TYPE,
    AST_NODE_TYPE
} ItemType;

typedef struct StackItem {
    ItemType itemType;
    union {
        token_t token;
        ASTNode *node;
    } data;
    struct StackItem *next_item;
}StackItem;

typedef struct Stack {
    struct StackItem *top;
    size_t size;
}Stack;

typedef enum {
    AST_INTEGER,
    AST_NON_TERM,
    AST_DOUBLE,
    AST_STRING,
    AST_BINARY_OP,
    AST_UNARY_OP
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    ItemType resultType;
    token_t token;
    struct ASTNode *left;
    struct ASTNode *right;
    
} ASTNode;

typedef enum identType { 
    I_NEG,
    I_MUL,    	
    I_DIVIDE,       
    I_PLUS,        	
    I_MINUS,        	    	
    I_LESS,       
    I_LESS_EQ,    	
    I_GREATER,    
    I_GREATER_EQ, 	
    I_EQ,         	
    I_EQ_NIL,     
    I_COALESCE,	
    I_PAR_L,
    I_PAR_R,
    I_ID,    
    I_INTEGER,
    I_DOUBLE, 	
    I_STRING, 	
    I_NULL, 		 
    // I_NON_TERM, 	
    I_DOLAR,
    // I_EROR, 		  
} IdentType;



void stack_init(Stack *stack);
bool stack_push(Stack *stack, token_t token);

bool stack_pop(Stack *stack);
bool stack_isempty(Stack *stack);
bool is_operator(token_t* op);

int get_precedence(token_t top, token_t current);
void to_postfix(token_t* postfix);
int parse();

