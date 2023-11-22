#include "expr.h"

#define TABLE_SIZE 20

const char Precedence_table[TABLE_SIZE][TABLE_SIZE] = {
    //         !    *    /    +    -    <   <=    >    >=   ==  !=    ??   (    )    ID  INT  DBL  STR  NIL   $
    /* ! */  {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>'},
    /* * */  {'<', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* / */  {'<', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* + */  {'<', '<', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* - */  {'<', '<', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* < */  {'<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* <= */ {'<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* > */  {'<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* >= */ {'<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* == */ {'<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* != */ {'<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* ?? */ {'<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* ( */  {'<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '=', '>', '>', '>', '>', '>', '>'},
    /* ) */  {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>'},
    /* ID  */{'<', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* INT */{'<', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* DBL */{'<', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* STR */{'<', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* NIL */{'<', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>'},
    /* $  */ {'<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '='},
};


void stack_init(Stack *stack){
    stack->size= 0;
    stack->top = NULL;
}

bool stack_push_token(Stack *stack, token_t token){
    StackItem *tmp = malloc(sizeof(StackItem));

    if ((tmp) == NULL)
        handle_error(INTERNAL_COMPILER_ERROR,token.line, "Memory allocation error");

    tmp->data.token = token;
    tmp->itemType = TOKEN_TYPE;
    tmp->next_item = stack->top;
    stack->top = tmp;
    stack->size++;
    return true;
}

void stack_push_node(Stack *stack, ASTNode *node){
     StackItem *tmp = malloc(sizeof(StackItem));

    if ((tmp) == NULL)
        handle_error(INTERNAL_COMPILER_ERROR,node->token.line, "Memory allocation error");

    tmp->data.node= node;
    tmp->itemType = AST_NODE_TYPE;
    tmp->next_item = stack->top;
    stack->top = tmp;
    stack->size++;
}
void stack_push_after_terminal(Stack *stack, token_t token){
    token_t tmp;
    if (stack->top->data.token.type >= TK_UNWRAP && stack->top->data.token.type <= TK_RPAR) // Check if top is terminal
    {
        stack_push(&stack, token);
    }else
    {
        tmp = stack->top->data.token;
        stack_pop(&stack);
        stack_push_after_terminal(&stack,token);
    }
    
}
bool stack_pop(Stack *stack) {
    StackItem *pop_item = NULL;
    if (stack->top != NULL) {
        pop_item = stack->top;
        stack->top = stack->top->next_item;
        stack->size--;
        free(pop_item);
        return true;
    }
    else {
        return false;
    }
}
bool stack_isempty(Stack *stack) {
    return (stack->top == NULL);
}

int performSemanticCheck(ASTNode* node,parser_t *parser) {
    tk_type_t leftType = node->left->type;
    tk_type_t rightType = node->right->type;

    if (node->left->type == TK_IDENTIFIER)
    {
        leftType = typeOf_ID(parser, node->left->token.data.String);
    } else 
        leftType = node->left->type;
    if (node->right->type == TK_IDENTIFIER)
    {
        rightType = typeOf_ID(parser, node->right->token.data.String);
    }


    if (leftType != rightType) {
        handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
        return 0;
    }

    switch (node->token.type) {
        case TK_PLUS:
        case TK_MINUS:
        case TK_MUL:
        case TK_DIV:
            if ((leftType == TK_DOUBLE || leftType == TK_INT) &&
                (rightType == TK_DOUBLE || rightType == TK_INT)) {
                return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        case '==':
        case '!=':
            break;
        case '<':
        case '>':
        case '<=':
        case '>=':
             if ((leftType == TK_DOUBLE || leftType == TK_INT) &&
                (rightType == TK_DOUBLE || rightType == TK_INT)) {
                return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        case '??':
             if (leftType == rightType) {
                return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        default:
            handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            return 0;
    }

    switch (node->token.type) {
        case TK_PLUS:
        case TK_MINUS:
        case TK_MUL:
            if ((leftType == TK_DOUBLE || leftType == TK_INT) &&
                (rightType == TK_DOUBLE || rightType == TK_INT)) {
                node->resultType = (leftType == TK_DOUBLE || rightType == TK_DOUBLE) ?
                                   TK_DOUBLE : TK_INT;
                return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        case TK_DIV:
            if ((leftType == TK_DOUBLE || leftType == TK_INT) &&
                (rightType == TK_DOUBLE || rightType == TK_INT)) {
                node->resultType = TK_DOUBLE;
                return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        case TK_EQ:
        case TK_NEQ:
            if (leftType == rightType) {
                node->resultType = leftType;
                return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        case TK_LT:
        case TK_GT:
        case TK_LE:
        case TK_GE:
            if ((leftType == TK_DOUBLE || leftType == TK_INT) &&
                (rightType == TK_DOUBLE || rightType == TK_INT)) {
                node->resultType = leftType;
               return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        case '??':
            if (leftType != TK_KW_NIL) {
                node->resultType = leftType;
                return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        default:
            handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            break;
    }

    return 1;
}

ASTNode *create_node(token_t token, ASTNodeType type) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (node == NULL) {
        handle_error(INTERNAL_COMPILER_ERROR,token.line, "Memory allocation error");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->token = token;
    return node;
}
ASTNode *parse_non_terminal(token_t token){
    return create_node(token,AST_NON_TERM);
}

ASTNode *parse_unary(Stack *stack){
    
    ASTNode *node = create_node(stack->top->next_item->data.token, AST_UNARY_OP);
    node->left = parse_non_terminal(stack->top->data.token);
    return node;
}
tk_type_t typeOf_ID(parser_t * parser, char* String){
    Node *node;
    if (node=stackSearch(parser->local_frame, String)!= NULL)
    {
        return node->symbol.type;
    } else if (node=search(parser->global_frame, String)!= NULL)
    {
        return node->symbol.type;
    } else
        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "");
}
ASTNode *parse_binary(Stack *stack, parser_t *parser){

    ASTNode *right = parse_non_terminal(stack->top->data.token);
    stack_pop(&stack);
    token_t op = stack->top->data.token;
    stack_pop(&stack);
    ASTNode *left = parse_non_terminal(stack->top->data.token);
    stack_pop(&stack);
    ASTNode *node = create_node(op, AST_BINARY_OP);

    node->right = right;
    node->left = left;
    performSemanticCheck(node,parser);
    
    return node;
}
ASTNode *parse_par(Stack *stack){
    ASTNode *node;
    if (stack->top->data.token.type != TK_RPAR)
    {
        // Error
        handle_error(SYNTAX_ERROR,stack->top->data.token.line, "Expression");
    } else
        stack_pop(&stack);
    if (stack->top->itemType == TOKEN_TYPE)
    {
        /* Error */
        handle_error(SYNTAX_ERROR,stack->top->data.token.line, "Expression");
    } else
    {
        node = stack->top->data.node;
        stack_pop(&stack);
    }
    if (stack->top->data.token.type != TK_LPAR)
    {
        /* Error */
        handle_error(SYNTAX_ERROR,stack->top->data.token.line, "Expression");
    }
    
    return node;
}
ASTNode *parse_expression(Stack *stack, parser_t *parser) {
    if (stack->size == 1){
        if (stack->top->data.token.type >= TK_IDENTIFIER && stack->top->data.token.type <= TK_MLSTRING)
        {
            ASTNode *nonTerm = create_node(stack->top->data.token, AST_NON_TERM);
            return nonTerm;
        } else
        {
            /* Error */
            handle_error(SYNTAX_ERROR,stack->top->data.token.line, "Expression");
        }
    } else if (stack->size == 2)
    {
        if ((stack->top->data.token.type >= TK_IDENTIFIER && stack->top->data.token.type <= TK_MLSTRING) && (stack->top->next_item->data.token.type == TK_UNWRAP))
        {
            ASTNode *node = parse_unary(&stack);
            return node;
        }
        else
        {
            /* Error */
            handle_error(SYNTAX_ERROR,stack->top->data.token.line, "Expression");
        }
        
    } else if (stack->size == 3)
    {
         if (stack->top->next_item->data.token.type <= TK_COALESCE && stack->top->next_item->data.token.type >= TK_MUL)
         {
            ASTNode *node = parse_binary(&stack, parser);
            return node;
         }
         
    }
    
}

int get_precedence(token_t top, token_t current) {
    if (top.type >= TABLE_SIZE || current.type >= TABLE_SIZE)
    {
        return '>';
    }
    return Precedence_table[top.type][current.type];
}

void rule_expresion(parser_t *parser, TokenArray tokenArray){
    Stack* stack= (Stack*)malloc(sizeof(Stack));
    stack_init(stack);
    Stack* expr= (Stack*)malloc(sizeof(Stack));
    stack_init(expr);
    token_t dollar = create_token(TK_DOLLAR,0,0);
    token_t start = create_token(TK_LEFT,0,0);
    stack_push(stack, dollar);

    while (parser->current_token.type > TK_KW_DOUBLE) {
        int precedence = get_precedence(stack->top->data.token, parser->current_token);
        // stack_push(stack, parser->current_token);

        switch (precedence){
            case '=':
                stack_push(stack,parser->current_token);
                parser_get_next_token(&parser,&tokenArray);
                break;
            case '>':
                while (stack->top->data.token.type != TK_LEFT)
                {
                    if (stack->top->data.token.type != TK_LEFT)
                    {   
                        stack_push(&expr, stack->top->data.token);
                        stack_pop(&stack);
                    }
                }
                stack_push_node(&stack, parse_expression(&expr, parser));
                break;

            case '<':
                stack_push_after_terminal(&stack, parser->current_token);
                parser_get_next_token(&parser, &tokenArray);

                break;
            default:
                break;
        }
    }
    tk_type_t result = stack->top->data.node->resultType;
}
