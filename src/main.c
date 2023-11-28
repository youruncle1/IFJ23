#include "scanner.h"
#include "generator.h"
#include "parser.h"
#include "symtable.h" // Assuming this is the name of your symbol table header file

// Forward declarations
void printTokenArray(TokenArray *tokenArray);
void inOrder(Node *node);

const char* token_type_to_string(tk_type_t type) {
    switch (type) {
        case TK_KW_DOUBLE: return "TK_KW_DOUBLE";
        case TK_KW_DOUBLE_OPT: return "TK_KW_DOUBLE_OPT";
        case TK_KW_ELSE: return "TK_KW_ELSE";
        case TK_KW_FUNC: return "TK_KW_FUNC";
        case TK_KW_IF: return "TK_KW_IF";
        case TK_KW_INT: return "TK_KW_INT";
        case TK_KW_INT_OPT: return "TK_KW_INT_OPT";
        case TK_KW_LET: return "TK_KW_LET";
        case TK_KW_NIL: return "TK_KW_NIL";
        case TK_KW_RETURN: return "TK_KW_RETURN";
        case TK_KW_STRING: return "TK_KW_STRING";
        case TK_KW_STRING_OPT: return "TK_KW_STRING_OPT";
        case TK_KW_VAR: return "TK_KW_VAR";
        case TK_KW_WHILE: return "TK_KW_WHILE";
        case TK_IDENTIFIER: return "TK_IDENTIFIER";
        case TK_DOUBLE: return "TK_DOUBLE";
        case TK_INT: return "TK_INT";
        case TK_STRING: return "TK_STRING";
        case TK_MLSTRING: return "TK_MLSTRING";
        case TK_PLUS: return "TK_PLUS";
        case TK_MINUS: return "TK_MINUS";
        case TK_MUL: return "TK_MUL";
        case TK_DIV: return "TK_DIV";
        case TK_COALESCE: return "TK_COALESCE";
        case TK_UNWRAP: return "TK_UNWRAP";
        case TK_ASSIGN: return "TK_ASSIGN";
        case TK_EQ: return "TK_EQ";
        case TK_NEQ: return "TK_NEQ";
        case TK_LT: return "TK_LT";
        case TK_GT: return "TK_GT";
        case TK_LE: return "TK_LE";
        case TK_GE: return "TK_GE";
        case TK_LPAR: return "TK_LPAR";
        case TK_RPAR: return "TK_RPAR";
        case TK_COMMA: return "TK_COMMA";
        case TK_COLON: return "TK_COLON";
        case TK_SEMICOLON: return "TK_SEMICOLON";
        case TK_LBRACE: return "TK_LBRACE";
        case TK_RBRACE: return "TK_RBRACE";
        case TK_ARROW: return "TK_ARROW";
        case TK_UNDERSCORE: return "TK_UNDERSCORE";
        case TK_EOF: return "TK_EOF";
        default: return "UNKNOWN_TOKEN_TYPE";
    }
}

int main() {
    scanner_t scanner;
    scanner.input = stdin; 
    scanner.line = 1;

    generator_t gen = gen_Init();

    parser_t parser = initParser(&scanner);

    define_builtin_functions(&parser.global_frame);

    TokenArray *tokenArray = initTokenArray();

    firstParserPass(&parser, tokenArray);

    // printTokenArray(tokenArray);

    printf("\nSymbol Table (Inorder Traversal):\n");
    // inOrder(parser.global_frame);

    gen_Header(&gen);
    gen_inbuild(&gen);
    parseProgram(&parser,tokenArray, &gen);

    print_Code(&gen);

    return 0;
}

void printTokenArray(TokenArray *tokenArray) {
    printf("Token Array:\n");
    for (unsigned int i = 0; i < tokenArray->size; i++) {
        token_t token = tokenArray->tokens[i];

        printf("[%d] Token type: %s, Line: %u, EOL Before: %d", 
               i, token_type_to_string(token.type), 
               token.line, 
               token.eol_before);

        switch (token.type) {
            case TK_IDENTIFIER:
            case TK_STRING:
            case TK_MLSTRING:
                printf(", Data: %s", token.data.String);
                break;
            case TK_DOUBLE:
                printf(", Data: %lf", token.data.Double);
                break;
            case TK_INT:
                printf(", Data: %llu", token.data.Int);
                break;
            default:
                break;
        }

        printf("\n");
    }
}

void inOrder(Node *node) {
    if (node == NULL) {
        return;
    }
    inOrder(node->left);

    // Print the symbol key (function name or variable name)
    printf("\n%s", node->symbol.key);

    // If the symbol is a function, print its parameters
    if (node->symbol.isFunction) {
        printf(" ->  %s\n\t Parameters: ", token_type_to_string(node->symbol.type));
        for (int i = 0; i < node->symbol.parametersCount; i++) {
            printf("[Name: %s, ID: %s, Type: %s]", 
                   node->symbol.parameters[i].name, 
                   node->symbol.parameters[i].id, 
                   token_type_to_string(node->symbol.parameters[i].type));
            if (i < node->symbol.parametersCount - 1) {
                printf(", ");
            }
        }
        printf("\n");
    } else {
        printf(" (Variable)\n");
    }

    inOrder(node->right);
}