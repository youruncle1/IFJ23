/*
IFJ PROJEKT 2023/2024
file: "symtable.h"

Symtable (BVS) - header file

authors: xpolia05
         xhonze01

*/
#ifndef SYMTABLE_H
#define SYMTABLE_H

#include<stdio.h>
#include<malloc.h>
#include <stdbool.h>
#include <string.h>
#include "error.h"
#include "scanner.h"

#define max(a,b) ((a) > (b) ? (a) : (b))

/**
 * @brief Structure representing a parameter in a function.
 *
 * @param name Name of the parameter.
 * @param id Identifier for the parameter.
 * @param type Type of the parameter as defined by tk_type_t.
 */
typedef struct Parameter{
    char *name;
    char *id;
    tk_type_t type;
} Parameter;

/**
 * @brief Structure representing a symbol in the symbol table.
 *
 * @param key Unique key identifying the symbol.
 * @param type Type of the symbol as defined by tk_type_t.
 * @param isLet Flag indicating immutableness.
 * @param isInit Flag indicating if the symbol is initialized.
 * @param isFunction Flag indicating if the symbol represents a function.
 * @param parametersCount Number of parameters if the symbol is a function.
 * @param parameters Pointer to an array of Parameters if the symbol is a function.
 */
typedef struct Symbol{
    char key[50];
    
    tk_type_t type;
    
    bool isLet; 
    bool isInit; 
    
    bool isFunction;
    int parametersCount;
    Parameter *parameters;
} Symbol;


/**
 * @brief Node structure for AVL tree implementation in the symbol table.
 *
 * @param symbol The symbol stored in the node.
 * @param left Pointer to the left child.
 * @param right Pointer to the right child.
 * @param height Height of the node for AVL tree balancing.
 */
typedef struct Node{
    Symbol symbol;
    struct Node *left;
    struct Node *right;
    int height;
} Node;

/**
 * @brief Node structure for a stack of symbol tables.
 *
 * @param symbolTable Pointer to a symbol table (AVL tree root).
 * @param next Pointer to the next StackNode in the stack.
 */
typedef struct StackNode{
    Node *symbolTable; 
    struct StackNode *next; 
} StackNode;

/**
 * @brief Structure representing a stack of symbol tables.
 *
 * @param top Pointer to the top node in the stack.
 */
typedef struct {
    StackNode *top;
} SymbolTableStack;


/* STACK funcs */
/**
 * @brief Initializes a new stack for symbol tables.
 *
 * @return Pointer to the newly created SymbolTableStack.
 */
SymbolTableStack *initStack();

/**
 * @brief Pushes a new symbol table (tree node) onto the stack.
 *
 * @param stack Pointer to the stack.
 * @param treeNode Pointer to the symbol table to be pushed.
 */
void push(SymbolTableStack *stack, Node *treeNode);

/**
 * @brief Pops the top symbol table (tree node) from the stack.
 *
 * @param stack Pointer to the stack.
 */
void pop(SymbolTableStack* stack);

/**
 * @brief Checks if the stack is empty.
 *
 * @param stack Pointer to the stack.
 * @return Integer indicating if the stack is empty (1) or not (0).
 */
int isEmpty(SymbolTableStack *stack);

/**
 * @brief Searches for a symbol with the given key in the stack of symbol tables.
 *
 * @param stack Pointer to the stack.
 * @param key Key of the symbol to search for.
 * @return Pointer to the Node containing the symbol, or NULL if not found.
 */
Node *stackSearch(SymbolTableStack* stack, const char* key);


/* AVLTree funcs */
/**
 * @brief Initializes a new Symbol.
 *
 * @param key Unique key identifying the symbol.
 * @param type Type of the symbol as defined by tk_type_t.
 * @param isLet Flag indicating if the symbol is a 'let' variable.
 * @param isInit Flag indicating if the symbol is initialized.
 * @param isFunction Flag indicating if the symbol represents a function.
 * @return Pointer to the newly created Symbol.
 */
Symbol *initSymbol(const char *key, tk_type_t type, bool isLet, bool isInit, bool isFunction);

/**
 * @brief Creates a new AVL tree node with the given symbol.
 *
 * @param symbol The symbol to be inserted in the new node.
 * @return Pointer to the newly created Node.
 */
Node *newNode(Symbol symbol);

/**
 * @brief Inserts a new symbol into the AVL tree.
 *
 * @param root Pointer to the root node of the tree.
 * @param symbol The symbol to be inserted.
 * @return Pointer to the root node after insertion.
 */
Node *insert(Node *root, Symbol symbol);

/**
 * @brief Inserts a new function symbol into the AVL tree.
 *
 * @param root Pointer to the root node of the tree.
 * @param token Token representing the function.
 * @return Pointer to the root node after insertion.
 */
Node *insertFunc(Node *root, token_t token);

/**
 * @brief Inserts a new parameter into a function symbol in the AVL tree.
 *
 * @param root Pointer to the root node of the tree.
 * @param funcKey Key of the function to which the parameter belongs.
 * @param name Name of the parameter.
 * @param id Identifier of the parameter.
 * @param type Type of the parameter as defined by tk_type_t.
 */
void InsertParam(Node *root, const char *funcKey, const char *name, const char *id, tk_type_t type);

/**
 * @brief Updates the type of a symbol in the AVL tree.
 *
 * @param root Pointer to the root node of the tree.
 * @param key Key of the symbol to update.
 * @param type New type for the symbol as defined by tk_type_t.
 */
void InsertType(Node *root, const char *key, tk_type_t type);

/**
 * @brief Inserts a variable symbol into the AVL tree.
 *
 * @param root Pointer to the root node of the tree.
 * @param token Token representing the variable.
 * @param isLet Flag indicating if the variable is a 'let' variable.
 * @param isInit Flag indicating if the variable is initialized.
 * @return Pointer to the root node after insertion.
 */
Node *insertVar(Node *root, token_t token, bool isLet, bool isInit);

/**
 * @brief Searches for a symbol with the given key in the AVL tree.
 *
 * @param root Pointer to the root node of the tree.
 * @param key Key of the symbol to search for.
 * @return Pointer to the Node containing the symbol, or NULL if not found.
 */
Node *search(Node* root, const char* key);

/**
 * @brief Returns the height of the node in the AVL tree.
 *
 * @param root Pointer to the node.
 * @return Height of the node.
 */
int height(Node *root);

/**
 * @brief Calculates the balance factor of a node in the AVL tree.
 *
 * @param root Pointer to the node.
 * @return Balance factor of the node.
 */
int getBalance(Node* root);

/**
 * @brief Performs a right rotation on the given node in the AVL tree.
 *
 * @param root Pointer to the node to rotate.
 * @return Pointer to the new root after rotation.
 */
Node *rightRotate(Node *root);

/**
 * @brief Performs a left rotation on the given node in the AVL tree.
 *
 * @param root Pointer to the node to rotate.
 * @return Pointer to the new root after rotation.
 */
Node *leftRotate(Node *root);

/**
 * @brief Frees all nodes in the AVL tree.
 *
 * @param root Pointer to the root node of the tree.
 */
void freeTable(Node *root);

/**
 * @brief Defines built-in functions in the given AVL tree.
 *
 * @param root Double pointer to the root node of the tree.
 */
void define_builtin_functions(Node **root);

#endif