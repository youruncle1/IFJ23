#include<stdio.h>
#include<malloc.h>
#include <stdbool.h>
#include <string.h>
#define INT_TYPE 1
#define DOUBLE_TYPE 2
#define STRING_TYPE 3

typedef struct Stack Stack;

typedef struct Parameter{
    char *name;
    char *id;
    int type;
}Parameter;

typedef struct Symbol{
    char key[50];
    int type;
    bool isFunction;
    Stack *localFrame;
    Parameter *parameters;
    int parametersCount;
} Symbol;

typedef struct Node{
    Symbol symbol;
    struct Node *left;
    struct Node *right;
    int height;
} Node;

typedef struct Stack{
    Node *treeNode;
    struct Stack *next;
} Stack;

Stack *StackInit(Stack *stack);
Stack *push(Stack* stack, Node* treeNode);
Node *newNode(Symbol symbol);
Node *insert(Node *root, Symbol symbol);
Node* search(Node* root, const char* key);
int isEmpty(Stack *stack);
Node* pop(Stack** stack);
Node *newLocalSTable(Node *root,char *key);
Node *insert2Local(Node *root,Symbol symbol);
Node *deleteLocalSTable(Node *root);
Node *searchLocal(Node *root,char *key);
Node *searchSymbol(Node *root,Node *local,char *key);
void inOrder(struct Node* node);
int max(int a, int b);
int height(Node *root);
int getBalance(Node* root);
Node *rightRotate(Node *root);
Node *leftRotate(Node *root);
Node *addParameter(Node *root,char *key,Parameter parameter);
Symbol *initSymbol(const char *key, int type, bool isFunction);
void freeTable(Node *root);

