#include "symtable.h"

/* Funkce pro inicializaci zasobniku */
SymbolTableStack *initStack(){
    SymbolTableStack *stack = malloc(sizeof(SymbolTableStack));
    if (stack == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
    }
    stack->top = NULL;
    return stack;
}

/* Funkce pro inicializaci symbolu */
Symbol *initSymbol(const char *key, int type, bool isLet, bool isFunction, bool isDefined) {
    
    Symbol *symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
    }

    strcpy(symbol->key, key);
    symbol->type = type;
    symbol->isLet = isLet;
    symbol->isFunction = isFunction;
    symbol->isDefined = isDefined;  //defined function or defined variable
    symbol->parametersCount = 0;
    symbol->parameters = NULL;
    return symbol;
}

/* Pomocna funkce pro vytvoreni noveho uzlu binarniho stromu */
Node *newNode(Symbol symbol) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
    }
    newNode->symbol = symbol;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->height = 1;

    return newNode;
}

bool insertVar(Node *root, Symbol symbol) {
    if (search(root, symbol.key) != NULL){
        handle_error(SEMANTIC_UNDEFINED_FUNCTION, 0, "variable redefinition");
    }

    insert(root, symbol);
    return true;
}

Node *insertFunc(Node *root, Symbol symbol, bool definition){
    Node *found = search(root, symbol.key);

    if (found != NULL && found->symbol.isDefined && definition){
        handle_error(SEMANTIC_UNDEFINED_FUNCTION, 0, "function redefinition");
    }

    if (found != NULL){
        if (definition) {
            found->symbol.isDefined = true;
        }
        return found;
    }

    return insert(root, symbol);
}

/* Funkce pro vlozeni symbolu do tabulky symbolu */
Node *insert(Node *root, Symbol symbol) {
    if (root == NULL) {
        return newNode(symbol);
    }

    if (strcmp(symbol.key, root->symbol.key) < 0){
        root->left = insert(root->left, symbol);
    }
    else if (strcmp(symbol.key, root->symbol.key) > 0){
        root->right = insert(root->right, symbol);
    } 

    root->height = max(height(root->left), height(root->right)) + 1;

    int balance = getBalance(root);

    /* Rotace L */
    if (balance > 1 && strcmp(symbol.key, root->symbol.key) < 0) {
        root = rightRotate(root);
    }

    /* Rotace R */
    if (balance > -1 && strcmp(symbol.key, root->symbol.key) > 0) {
        root = leftRotate(root);
    }

    /* Rotace LR */
    if (balance > 1 && strcmp(symbol.key, root->symbol.key) > 0) {
        root->left = leftRotate(root->left);
        root = rightRotate(root);
    }

    /* Rotace RL */
    if (balance < -1 && strcmp(symbol.key, root->symbol.key) < 0) {
        root->right = rightRotate(root->right);
        root = leftRotate(root);
    }

    return root;
}

Node *addParameter(Node *root,char *key,Parameter parameter){
    Node *func = search(root,key);
    func->symbol.parametersCount++;
    func->symbol.parameters = realloc(func->symbol.parameters, func->symbol.parametersCount * sizeof(Parameter));
    func->symbol.parameters[func->symbol.parametersCount - 1] = parameter;
    return root;
}

void checkAllFunctionsDefined(Node* root) {
    if (root == NULL) {
        return;
    }

    checkAllFunctionsDefined(root->left);

    if (root->symbol.isFunction && !root->symbol.isDefined) {
        handle_error(SEMANTIC_UNDEFINED_FUNCTION, 0, "Function not defined");
    }

    checkAllFunctionsDefined(root->right);
}

/* Funkce pro vyhledani symbolu v tabulce symbolu */
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

/* Funkce pro vlozeni stromu do zasobniku */
void push(SymbolTableStack *stack, Node *treeNode) {
    StackNode *newNode = (StackNode*)malloc(sizeof(StackNode));
    if (newNode == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
    }

    newNode->symbolTable = treeNode;
    newNode->next = stack->top;
    stack->top = newNode;
}

/* Funkce pro vymazani stromu ze zasobniku */
void pop(SymbolTableStack* stack) {
    if (isEmpty(stack)) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "SymbolStack is empty");
    }

    StackNode* topNode = stack->top;
    stack->top = topNode->next;

    // Free the symbol table associated with the top node.
    freeTable(topNode->symbolTable);

    // Free the top node itself.
    free(topNode);
}

/* Funkce, ktera prohleda stack od top do bottom */
Node* stackSearch(SymbolTableStack* stack, const char* key) {
    StackNode* current = stack->top;
    while (current != NULL) {
        Node* found = search(current->symbolTable, key);
        if (found != NULL) {
            return found;
        }
        current = current->next;
    }
    return NULL;
}

/* Funkce pro zjisteni, zda je zasobnik prazdny */
int isEmpty(SymbolTableStack* stack) {
    return (stack->top == NULL);
}

void inOrder(struct Node* node) {
    if (node == NULL) {
        return;
    }
    inOrder(node->left);
    printf("%s ", node->symbol.key);
    inOrder(node->right);
}


int height(Node *root) {
    if (root == NULL) {
        return 0;
    }
    return root->height;
}

int getBalance(Node* root) {
    if (root == NULL) {
        return 0;
    }
    return height(root->left) - height(root->right);
}

Node *rightRotate(Node *root) {
    if (root == NULL || root->left == NULL) {
        return root;
    }
    Node *x = root->left;
    Node *T2 = x->right;

    x->right = root;
    root->left = T2;

    root->height = max(height(root->left), height(root->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

Node *leftRotate(Node *root) {
    if (root == NULL || root->right == NULL) {
        return root;
    }
    Node *y = root->right;
    Node *T2 = y->left;

    y->left = root;
    root->right = T2;

    root->height = max(height(root->left), height(root->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

void freeTable(Node *root){
    if (root == NULL) return;

    freeTable(root->left);
    freeTable(root->right);

    // Free the parameters array if it exists
    if (root->symbol.parameters != NULL) {
        free(root->symbol.parameters);
    }

    free(root);
}

