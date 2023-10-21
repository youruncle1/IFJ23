#include "symtable.h"

/* Funkce pro inicializaci zasobniku */
Stack *StackInit(Stack *stack){
    return NULL;
}
/* Pomocna funkce pro vytvoreni noveho uzlu binarniho stromu */
Node *newNode(Symbol symbol) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    newNode->symbol = symbol;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->height = 1;

    return newNode;
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

/* Funkce pro vymazani stromu ze zasobniku */
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

/* Funkce pro zjisteni, zda je zasobnik prazdny */
int isEmpty(Stack* stack) {
    return (stack == NULL);
}

/* Funkce, ktera vytvori zasobnik pro lokalni tabulku symbolu */
Node *newLocalSTable(Node *root,char *key){
    Node *func;
    func = search(root,key);
    func->symbol.localFrame = push(func->symbol.localFrame,NULL);
    return func;
}

/* Funkce, ktera vlozi symbol do lokalni tabulky symbolu */
Node *insert2Local(Node *root,Symbol symbol){
    root->symbol.localFrame->treeNode = insert(root->symbol.localFrame->treeNode,symbol);
    return root;
}

/* Funkce, ktera smaze lokalni tabulku symbolu */
Node *deleteLocalSTable(Node *root){
    pop(&root->symbol.localFrame);
}

/* Funkce, ktera vyhledava symbol v lokalni tabulce symbolu */
Node *searchLocal(Node *root,char *key) {
    Node *searchedNode = NULL;
    if(root->symbol.localFrame != NULL) {
        Stack *next = root->symbol.localFrame;
        Node *tree = root->symbol.localFrame->treeNode;
        do {
            next = next->next;
            searchedNode = search(tree, key);
            if (next != NULL) {
                tree = next->treeNode;
            }
        } while (next != NULL && searchedNode == NULL);
    }
    return searchedNode;
}

/* Funkce, ktera hleda symbol v lokalni tabulce, pokud nenajde, hleda v globalni */
Node *searchSymbol(Node *root,Node *local,char *key) {
    Node *searchedNode = NULL;
    searchedNode = searchLocal(local,key);
    if(searchedNode == NULL){
        searchedNode = search(root,key);
    }
    return searchedNode;
}

void inOrder(struct Node* node) {
    if (node == NULL) {
        return;
    }
    inOrder(node->left);
    printf("%s ", node->symbol.key);
    inOrder(node->right);
}

int max(int a, int b) {
    return (a > b) ? a : b;
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

Node *addParameter(Node *root,char *key,Parameter parameter){
    Node *func = search(root,key);
    func->symbol.parametersCount++;
    func->symbol.parameters = realloc(func->symbol.parameters, func->symbol.parametersCount * sizeof(Parameter));
    func->symbol.parameters[func->symbol.parametersCount - 1] = parameter;
    return root;
}
Symbol *initSymbol(const char *key, int type, bool isFunction) {
    Symbol *symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    strcpy(symbol->key, key);
    symbol->type = type;
    symbol->isFunction = isFunction;
    symbol->localFrame = NULL;
    symbol->parametersCount = 0;
    symbol->parameters = NULL;
    return symbol;
}

void freeTable(Node *root){
}

