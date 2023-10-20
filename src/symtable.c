#include "symtable.h"

Stack *StackInit(Stack *stack){
    return NULL;
}
Node *newNode(Symbol symbol) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode != NULL) {
        newNode->symbol = symbol;
        newNode->left = NULL;
        newNode->right = NULL;
    }
    return newNode;
}

Node *insert(Node *node, Symbol symbol) {
    if (node == NULL) {
        return newNode(symbol);
    }
    if (strcmp(symbol.key, node->symbol.key) < 0){
        node->left = insert(node->left, symbol);
    }
    else if (strcmp(symbol.key, node->symbol.key) > 0){
        node->right = insert(node->right, symbol);
    }
    return node;
}
Node* search(Node* root, const char* key) {
    if (root == NULL) {
        return NULL;
    }

    if (strcmp(key, root->symbol.key) == 0) {
        return root;
    } else if (strcmp(key, root->symbol.key) < 0) {
        return search(root->left, key);
    } else {
        return search(root->right, key);
    }
}

Stack *push(Stack *stack, Node *treeNode) {
    Stack* newNode = (Stack*)malloc(sizeof(Stack));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    newNode->treeNode = treeNode;
    newNode->next = stack;
    return newNode;
}

Node* pop(Stack** stack) {
    if (isEmpty(*stack)) {
        fprintf(stderr, "Stack is empty\n");
        exit(1);
    }
    Node* poppedNode = (*stack)->treeNode;
    Stack* top = *stack;
    *stack = top->next;
    free(top);
    return poppedNode;
}

int isEmpty(Stack* stack) {
    return (stack == NULL);
}
