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
Symbol *initSymbol(const char *key, tk_type_t type, bool isLet, bool isInit, bool isFunction) {
    
    Symbol *symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
    }

    strcpy(symbol->key, key);
    symbol->type = type;
    symbol->isLet = isLet;
    symbol->isInit = isInit;
    symbol->isFunction = isFunction;
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

Node *insertVar(Node *root, token_t token, bool isLet, bool isInit) {
    if (search(root, token.data.String) != NULL){
        handle_error(SEMANTIC_UNDEFINED_FUNCTION, 0, "variable redefinition");
    }

    Symbol *symbol = initSymbol(token.data.String, TK_KW_NIL, isLet, isInit, false);
    return insert(root, *symbol);
}

Node *insertFunc(Node *root, token_t token){
    Node *found = search(root, token.data.String);

    if (found != NULL){
        handle_error(OTHER_SEMANTIC_ERROR, 0, "Function redefinition error");
    }

    Symbol *symbol = initSymbol(token.data.String, TK_KW_NIL, false, false, true); // initialize with NIL(return type yet unknown)
    return insert(root, *symbol);
}

void InsertParam(Node *root, const char *funcKey, const char *name, const char *id, tk_type_t type) {
    Node *funcNode = search(root, funcKey);
    if (funcNode == NULL || !funcNode->symbol.isFunction) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "Desired function not found in symtable");
        return;
    }

    Parameter *parameter = (Parameter *)malloc(sizeof(Parameter));
    if (parameter == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
        return;
    }

    // Parameter init
    parameter->name = strdup(name); 
    parameter->id = strdup(id);     
    parameter->type = type;

    // Resize parameter list
    funcNode->symbol.parametersCount++;
    funcNode->symbol.parameters = realloc(funcNode->symbol.parameters, funcNode->symbol.parametersCount * sizeof(Parameter));
    if (funcNode->symbol.parameters == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "FATAL: Memory allocation error");
        return;
    }

    // Add the new parameter to the function's parameter list
    funcNode->symbol.parameters[funcNode->symbol.parametersCount - 1] = *parameter;

}

/* CAN BE USED FOR VARIABLES TOO! function inserts either type of variable or return type of function */
void InsertType(Node *root, const char *key, tk_type_t type){
    Node *funcNode = search(root, key);
    if (funcNode == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR, 0, "Desired function not found in symtable");
        return;
    }
    
    funcNode->symbol.type = type;
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

/* Funkce pro vyhledani symbolu v tabulce symbolu */
Node *search(Node* root, const char* key) {
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

    freeTable(topNode->symbolTable);

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

// void inOrder(struct Node* node) {
//     if (node == NULL) {
//         return;
//     }
//     inOrder(node->left);
//     printf("%s ", node->symbol.key);
//     inOrder(node->right);
// }


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

void define_builtin_functions(Node **root) {

    // func readString() -> String?
    Symbol readString = *initSymbol("readString", TK_KW_STRING_OPT, false, false, true);
    *root = insert(*root, readString);

    // func readInt() -> Int?
    Symbol readInt = *initSymbol("readInt", TK_KW_INT_OPT, false, false, true);
    *root = insert(*root, readInt);

    // func readDouble() -> Double?
    Symbol readDouble = *initSymbol("readDouble", TK_KW_DOUBLE_OPT, false, false, true);
    *root = insert(*root, readDouble);

    // func write(term1, term2, ..., termN)
    Symbol write = *initSymbol("write", TK_KW_NIL, false, false, true);
    *root = insert(*root, write);
    // !!!! The parameters of 'write' function are variable. 

     // Int2Double(_ term: Int) -> Double
    Symbol int2Double = *initSymbol("Int2Double", TK_KW_DOUBLE, false, false, true);
    *root = insert(*root, int2Double);
    InsertParam(*root, "Int2Double", "_", "term", TK_KW_INT);

    // Double2Int(_ term: Double) -> Int
    Symbol double2Int = *initSymbol("Double2Int", TK_KW_INT, false, false, true);
    *root = insert(*root, double2Int);
    InsertParam(*root, "Double2Int", "_", "term", TK_KW_DOUBLE);

    // length(_ s: String) -> Int
    Symbol length = *initSymbol("length", TK_KW_INT, false, false, true);
    *root = insert(*root, length);
    InsertParam(*root, "length", "_", "s", TK_KW_STRING);

    // substring(of s: String, startingAt i: Int, endingBefore j: Int) -> String?
    Symbol substring = *initSymbol("substring", TK_KW_STRING_OPT, false, false, true);
    *root = insert(*root, substring);
    InsertParam(*root, "substring", "of", "s", TK_KW_STRING);
    InsertParam(*root, "substring", "startingAt", "i", TK_KW_INT);
    InsertParam(*root, "substring", "endingBefore", "j", TK_KW_INT);

    // ord(_ c: String) -> Int
    Symbol ord = *initSymbol("ord", TK_KW_INT, false, false, true);
    *root = insert(*root, ord);
    InsertParam(*root, "ord", "_", "c", TK_KW_STRING);

    // chr(_ i: Int) -> String
    Symbol chr = *initSymbol("chr", TK_KW_STRING, false, false, true);
    *root = insert(*root, chr);
    InsertParam(*root, "chr", "_", "i", TK_KW_INT);

}
