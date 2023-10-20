#include<stdio.h>
#include<malloc.h>
#include <stdbool.h>
#include <string.h>
#define INT_TYPE 1
#define DOUBLE_TYPE 2
#define STRING_TYPE 3

typedef struct Stack Stack;

typedef struct Symbol{
    char key[10];
    int type;
    bool isFunction;
    Stack *localFrame;
} Symbol;

typedef struct Node{
    Symbol symbol;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct Stack{
    Node *treeNode;
    struct Stack *next;
} Stack;

Stack *StackInit(Stack *stack);
Stack *push(Stack* stack, Node* treeNode);
Node *newNode(Symbol symbol);
Node *insert(Node *node, Symbol symbol);
Node* search(Node* root, const char* key);
int isEmpty(Stack *stack);
Node* pop(Stack** stack);

