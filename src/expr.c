#include "expr.h"

#define TABLE_SIZE 20

const char Precedence_table[TABLE_SIZE][TABLE_SIZE] = {
    //         !    *    /    +    -    <   <=    >    >=   ==  !=    ??   (    )    ID  INT  DBL  STR  NIL   $   [riadky][stlpce]
    /* ! */  {'E', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>'},
    /* + */  {'E', '<', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* / */  {'E', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* - */  {'E', '<', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* * */  {'E', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* < */  {'E', '<', '<', '<', '>', 'E', 'E', 'E', 'E', 'E', 'E', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* <= */ {'E', '<', '<', '<', '>', 'E', 'E', 'E', 'E', 'E', 'E', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* > */  {'E', '<', '<', '<', '>', 'E', 'E', 'E', 'E', 'E', 'E', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* >= */ {'E', '<', '<', '<', '>', 'E', 'E', 'E', 'E', 'E', 'E', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* == */ {'E', '<', '<', '<', '>', 'E', 'E', 'E', 'E', 'E', 'E', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* != */ {'E', '<', '<', '<', '>', 'E', 'E', 'E', 'E', 'E', 'E', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* ?? */ {'E', '<', '<', '<', '>', '<', '<', '<', '<', '<', '<', '<', '<', '>', '<', '<', '<', '<', '<', '>'},
    /* ( */  {'<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '=', '<', '<', '<', '<', '<', '>'},
    /* ) */  {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'E', 'E', 'E', 'E', 'E', '>'},
    /* INT */{'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'E', '>', 'E', 'E', 'E', 'E', 'E', '>'},
    /* ID  */{'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'E', '>', 'E', 'E', 'E', 'E', 'E', '>'},
    /* DBL */{'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'E', '>', 'E', 'E', 'E', 'E', 'E', '>'},
    /* STR */{'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'E', '>', 'E', 'E', 'E', 'E', 'E', '>'},
    /* NIL */{'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'E', '>', 'E', 'E', 'E', 'E', 'E', '>'},
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

void stack_empty(Stack *stack){
    stack->top= NULL;
    stack->size = 0;
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
token_t stack_top_terminal(StackItem *item){
    if (item->itemType == TOKEN_TYPE)
        if (item->data.token.type >= TK_UNWRAP && item->data.token.type <= TK_DOLLAR) // Check if top is terminal
        {
            return item->data.token;
        }else
        {
            return stack_top_terminal(item->next_item);
        }
    else
        return stack_top_terminal(item->next_item);
}
void stack_push_after_terminal(Stack *stack, token_t token){
    token_t tmp;
    if (stack->top->itemType == TOKEN_TYPE)
        if (stack->top->data.token.type >= TK_UNWRAP && stack->top->data.token.type <= TK_DOLLAR ) // Check if top is terminal
        {
            stack_push_token(stack, token);
        }else
        {
            tmp = stack->top->data.token;
            stack_pop(stack);
            stack_push_after_terminal(stack,token);
            stack_push_token(stack,tmp);
        }
    else {
        ASTNode* tmpNode = stack->top->data.node;
        stack_pop(stack);
        stack_push_after_terminal(stack,token);
        stack_push_node(stack,tmpNode);
    }
}
StackItem *stack_pop(Stack *stack) {
    StackItem *pop_item = NULL;
    if (stack->top != NULL) {
        pop_item = stack->top;
        stack->top = stack->top->next_item;
        stack->size--;
        return pop_item;
    }
    else {
        return false;
    }
}
bool stack_isempty(Stack *stack) {
    return (stack->top == NULL);
}

int performSemanticCheck(ASTNode* node,parser_t *parser) {
    tk_type_t leftType;
    tk_type_t rightType;
    if (node->left->token.type >= TK_UNWRAP && node->left->token.type <= TK_COALESCE)
        leftType = node->left->resultType;
    else
        leftType = node->left->token.type;
    if (node->right->token.type >= TK_UNWRAP && node->right->token.type <= TK_COALESCE)
        rightType = node->left->resultType;
    else
        rightType = node->right->token.type;

    if (node->left->token.type == TK_IDENTIFIER)
    {
        leftType = expr_convert_literal_to_datatype(typeOf_ID(parser, node->left->token.data.String));
    }
    if (node->right->token.type == TK_IDENTIFIER)
    {
        rightType = expr_convert_literal_to_datatype(typeOf_ID(parser, node->right->token.data.String));
    }


    switch (node->token.type) {
        case TK_PLUS:
            if (leftType == TK_STRING && rightType == TK_STRING) 
            {
                node->resultType = TK_STRING;
                return 0;
            } else if ((leftType == TK_STRING || rightType == TK_STRING) &&
                       leftType != rightType)
            {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
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
                node->resultType = TK_BOOLEAN;
                return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        case TK_LT:
        case TK_GT:
        case TK_LE:
        case TK_GE:
            if ((leftType == rightType) && 
                (leftType >= TK_DOUBLE && leftType <= TK_MLSTRING)
                ) {
                node->resultType = TK_BOOLEAN;
               return 0;
            } else {
                handle_error(SEMANTIC_TYPE_COMPATIBILITY,node->token.line, "");
            }
            break;
        case TK_COALESCE:
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
    node->resultType = type;
    node->token = token;
    return node;
}
ASTNode *parse_non_terminal(token_t token){
    return create_node(token,AST_NON_TERM);
}

ASTNode *parse_unary(Stack *stack){
    ASTNode *right;
    if (stack->top->itemType == AST_NODE_TYPE)
    {
        right = stack_pop(stack)->data.node;    
    } else
        right = parse_non_terminal(stack_pop(stack)->data.token);
    token_t op = stack_pop(stack)->data.token;
    ASTNode *node = create_node(op, AST_UNARY_OP);
    node->resultType = right->resultType;
    node->right = right;
    return node;
}
tk_type_t typeOf_ID(parser_t * parser, char* String){
    Node *node;
    if ((node = stackSearch(parser->local_frame, String)) != NULL)
    {
        return node->symbol.type;
    } else if ((node = search(parser->global_frame, String)) != NULL)
    {
        return node->symbol.type;
    } else
        handle_error(SEMANTIC_UNDEFINED_VARIABLE, parser->current_token.line, "");
}
ASTNode *parse_binary(Stack *stack, parser_t *parser){
    ASTNode *right;
    ASTNode *left;
    if (stack->top->itemType == AST_NODE_TYPE)
    {
        left = stack_pop(stack)->data.node;    
    } else
        left = parse_non_terminal(stack_pop(stack)->data.token);


    
    // Opperator
    token_t op = stack_pop(stack)->data.token;
    ASTNode *node = create_node(op, AST_BINARY_OP);
    
    if (stack->top->itemType == AST_NODE_TYPE)
    {
        right = stack_pop(stack)->data.node;    
    } else
        right = parse_non_terminal(stack_pop(stack)->data.token);

    node->right = right;
    node->left = left;
    performSemanticCheck(node,parser);
    
    return node;
}
ASTNode *parse_par(Stack *stack, parser_t *parser){
    ASTNode *node;
    if (stack->top->data.token.type != TK_LPAR)
    {
        // Error
        handle_error(SYNTAX_ERROR,stack->top->data.token.line, "Expression");
    } else
        stack_pop(stack);
    if (stack->top->itemType == TOKEN_TYPE)
    {
        /* Error */
        handle_error(SYNTAX_ERROR,stack->top->data.token.line, "Expression");
    } else
    {
        node = stack->top->data.node;
        stack_pop(stack);
    }
    if (stack->top->data.token.type != TK_RPAR)
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
            nonTerm->resultType = nonTerm->token.type;
            return nonTerm;
        } else
        {
            /* Error */
            handle_error(SYNTAX_ERROR,stack->top->data.token.line, "Expression");
        }
    } else if (stack->size == 2)
    {
        if (stack->top->next_item->itemType == TOKEN_TYPE && (stack->top->next_item->data.token.type == TK_UNWRAP))
        {
            ASTNode *node = parse_unary(stack);
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
            ASTNode *node = parse_binary(stack, parser);
            return node;
         } else if (stack->top->itemType == TOKEN_TYPE && stack->top->data.token.type == TK_LPAR)
         {
            ASTNode *node = parse_par(stack, parser);
            return node;
         }
         
         
    }
    
}

int get_precedence(token_t top, token_t current) {
    if (top.type > TABLE_SIZE || current.type > TABLE_SIZE)
    {
        return '>';
    }
    return Precedence_table[top.type][current.type];
}
tk_type_t expr_convert_literal_to_datatype(tk_type_t tokenType){
    switch (tokenType) {
        case TK_KW_INT:
            return TK_INT;
        case TK_KW_STRING:
            return TK_STRING;
        case TK_KW_DOUBLE:
            return TK_DOUBLE;
        default:
            return tokenType;
    }
}

tk_type_t rule_expression(parser_t *parser, TokenArray *tokenArray){
    Stack *stack= (Stack*)malloc(sizeof(Stack));
    stack_init(stack);
    Stack *expr= (Stack*)malloc(sizeof(Stack));
    stack_init(expr);
    token_t dollar = create_token(TK_DOLLAR,0,0);
    token_t start = create_token(TK_LEFT,0,0);
    tk_type_t result;
    bool endOfExpre = false;
    stack_push_token(stack, dollar);
    

    while (true) {
        int precedence = get_precedence(stack_top_terminal(stack->top), parser->current_token);
        StackItem *Item;
        
        // checks if current token is out of expresion
        if (precedence == 'E' && (parser->current_token.eol_before == true || parser->current_token.type == TK_LBRACE))
        {
            endOfExpre = true;
        }
        if(endOfExpre)
            precedence = '>';
            
        if (stack->top->itemType == AST_NODE_TYPE && stack->size == 2 &&
            precedence == '>'){
            result = stack->top->data.node->resultType;
            //parser_get_previous_token(parser, tokenArray);
            parser_get_previous_token(parser, tokenArray);
            return result;
            }


        switch (precedence){
            case '=':
                stack_push_token(stack,parser->current_token);
                parser_get_next_token(parser,tokenArray);
                break;
            case '>':
                while (1) {
                    Item = stack_pop(stack);

                    if ((Item->itemType == TOKEN_TYPE && Item->data.token.type != TK_LEFT) || Item->itemType == AST_NODE_TYPE) {
                        if (Item->data.token.type != TK_LEFT) {
                            if (Item->itemType == AST_NODE_TYPE) {
                                stack_push_node(expr, Item->data.node);
                            } else {
                                stack_push_token(expr, Item->data.token);
                            }
                        } else  
                            free(Item);
                    } else {
                        // Break the loop if the conditions are not met
                        break;
                    }
                    free(Item);
                }
                stack_push_node(stack, parse_expression(expr, parser));
                break;

            case '<':
                stack_push_after_terminal(stack , start);
                stack_push_token(stack, parser->current_token);
                parser_get_next_token(parser, tokenArray);

                break;
            case 'E':
                handle_error(SYNTAX_ERROR,stack->top->data.token.line, "Expression");
            default:
                break;
        }
        stack_empty(expr);
    }
    // return result;
}