#include<stdio.h>
#include<malloc.h>
#include <stdbool.h>
#include <string.h>
#include "error.h"

#define max(a,b) ((a) > (b) ? (a) : (b))

typedef enum {
    INT_TYPE,
    DOUBLE_TYPE,
    STRING_TYPE,
    INT_TYPE_NIL, 
    DOUBLE_TYPE_NIL, 
    STRING_TYPE_NIL,
    UNKNWN_TYPE
} dataType;

typedef struct Parameter{
    char *name;
    char *id;
    dataType type;
} Parameter;

typedef struct Symbol{
    char key[50];
    
    dataType type;
    
    bool isLet; //holds info if a variable can be modified
    
    bool isFunction; //says whether symbol is a function or variable
    bool isDefined;
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
    Node *symbolTable;
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
Node* stackSearch(SymbolTableStack* stack, const char* key);

/* AVLTree funcs */
Symbol *initSymbol(const char *key, dataType type, bool isFunction);
Node *newNode(Symbol symbol);
Node *insert(Node *root, Symbol symbol);
Node *addParameter(Node *root,char *key,Parameter parameter);
Node* search(Node* root, const char* key);
void inOrder(struct Node* node);
int height(Node *root);
int getBalance(Node* root);
Node *rightRotate(Node *root);
Node *leftRotate(Node *root);
void freeTable(Node *root);

