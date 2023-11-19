#ifndef SYMTABLE_H
#define SYMTABLE_H

#include<stdio.h>
#include<malloc.h>
#include <stdbool.h>
#include <string.h>
#include "error.h"
#include "scanner.h"

#define max(a,b) ((a) > (b) ? (a) : (b))

typedef struct Parameter{
    char *name;
    char *id;
    tk_type_t type;
} Parameter;

typedef struct Symbol{
    char key[50];
    
    tk_type_t type;
    
    bool isLet; //holds info if a variable can be modified
    
    bool isFunction; //says whether symbol is a function or variable
    int parametersCount;
    Parameter *parameters;
} Symbol;

typedef struct Node{
    Symbol symbol;
    struct Node *left;
    struct Node *right;
    int height;
} Node;

typedef struct StackNode{
    Node *symbolTable; //tree
    struct StackNode *next; 
} StackNode;

typedef struct {
    StackNode *top;
} SymbolTableStack;

/* STACK funcs */
SymbolTableStack *initStack();
void push(SymbolTableStack *stack, Node *treeNode);
void pop(SymbolTableStack* stack);
int isEmpty(SymbolTableStack *stack);
Node *stackSearch(SymbolTableStack* stack, const char* key);

/* AVLTree funcs */
Symbol *initSymbol(const char *key, tk_type_t type, bool isLet, bool isFunction);
Node *newNode(Symbol symbol);
Node *insert(Node *root, Symbol symbol);
Node *insertFunc(Node *root, token_t token);
void InsertParam(Node *root, const char *funcKey, const char *name, const char *id, tk_type_t type);
void InsertReturnType(Node *root, const char *funcKey, tk_type_t type);
bool insertVar(Node *root, Symbol symbol);
Node *search(Node* root, const char* key);
//void inOrder(struct Node* node);
int height(Node *root);
int getBalance(Node* root);
Node *rightRotate(Node *root);
Node *leftRotate(Node *root);
void freeTable(Node *root);
void define_builtin_functions(Node **root);

#endif