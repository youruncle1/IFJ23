/*
IFJ PROJEKT 2023/2024
file: "expr.h"

Expression parser - header file

authors: xbencs00 

*/
#ifndef EXPR_H
#define EXPR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "scanner.h"
#include "parser.h"
#include "error.h"
#include "generator.h"

typedef enum {
    TOKEN_TYPE,
    AST_NODE_TYPE
} ItemType;


typedef enum {
    AST_INTEGER,
    AST_NON_TERM,
    AST_DOUBLE,
    AST_STRING,
    AST_BINARY_OP,
    AST_UNARY_OP
} ASTNodeType;

typedef struct ASTNode {
    tk_type_t resultType;
    ItemType type;
    token_t token;
    struct ASTNode *left;
    struct ASTNode *right;
    
} ASTNode;

typedef struct StackItem {
    ItemType itemType;
    union {
        token_t token;
        ASTNode *node;
    } data;
    struct StackItem *next_item;
}StackItem;

typedef struct Stack {
    struct StackItem *top;
    size_t size;
}Stack;

typedef enum identType { 
    I_NEG,
    I_MUL,    	
    I_DIVIDE,       
    I_PLUS,        	
    I_MINUS,        	    	
    I_LESS,       
    I_LESS_EQ,    	
    I_GREATER,    
    I_GREATER_EQ, 	
    I_EQ,         	
    I_EQ_NIL,     
    I_COALESCE,	
    I_PAR_L,
    I_PAR_R,
    I_ID,    
    I_INTEGER,
    I_DOUBLE, 	
    I_STRING, 	
    I_NULL, 		 
    // I_NON_TERM, 	
    I_DOLAR,
    // I_EROR, 		  
} IdentType;


/**
 * @brief Initializes a stack.
 *
 * This function initializes a stack by setting its size to 0
 * and its top pointer to NULL. It should be called before using
 * any other stack-related functions.
 *
 * @param stack A pointer to the Stack structure to be initialized.
 *              The structure must be allocated before calling this function.
 */
void stack_init(Stack *stack);

/**
 * @brief Pushes a token onto the stack.
 *
 * This function pushes a token onto the stack by allocating memory for a new
 * stack item, setting its data to the provided token, and updating the stack's
 * top pointer. The size of the stack is also incremented.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 * @param token The token to be pushed onto the stack.
 *              It contains information about the token, such as its type and value.
 *
 * @return Returns true if the token is successfully pushed onto the stack,
 *         or false if there is a memory allocation error.
 */
bool stack_push_token(Stack *stack, token_t token);
/**
 * @brief Pushes an Abstract Syntax Tree (AST) node onto the stack.
 *
 * This function pushes an AST node onto the stack by allocating memory for
 * a new stack item, setting its data to the provided AST node, and updating
 * the stack's top pointer. The size of the stack is also incremented.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 * @param node A pointer to the ASTNode structure representing the node to be pushed.
 *             It contains information about the AST node, such as its type and properties.
 *
 * @note The ownership of the AST node's memory is transferred to the stack.
 *       It is the responsibility of the caller to ensure proper memory management.
 */
void stack_push_node(Stack *stack, ASTNode *node);
/**
 * @brief Pushes a token onto the stack after the topmost terminal.
 *
 * This function checks if the top item on the stack is a terminal token.
 * If it is, the provided token is pushed onto the stack. If the top item is
 * not a terminal, it recursively pops items from the stack until a terminal
 * is encountered, then pushes the provided token and the previously popped items
 * back onto the stack.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 * @param token The token to be pushed onto the stack after the topmost terminal.
 *              It contains information about the token, such as its type and value.
 */
void stack_push_after_terminal(Stack *stack, token_t token);
/**
 * @brief Retrieves the topmost terminal token from the stack.
 *
 * This function recursively searches the stack for the topmost terminal token,
 * starting from the specified stack item. It returns the terminal token found.
 * If no terminal token is found, it returns a default token (e.g., with type TK_UNKNOWN).
 *
 * @param item A pointer to the StackItem structure.
 *             It represents the starting point for searching the stack.
 *
 * @return Returns the topmost terminal token if found, or a default token otherwise.
 */
token_t stack_top_terminal(StackItem *item);

/**
 * @brief Pops the top item from the stack.
 *
 * This function removes the top item from the stack and returns a pointer to it.
 * If the stack is empty, it returns NULL.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 *
 * @return Returns a pointer to the popped StackItem if the stack is not empty,
 *         or NULL if the stack is empty.
 */
StackItem *stack_pop(Stack *stack);
/**
 * @brief Empties the stack.
 *
 * This function empties the stack by setting its top pointer to NULL
 * and its size to 0. It effectively removes all items from the stack.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 */
void stack_empty(Stack *stack);
/**
 * @brief Checks if the stack is empty.
 *
 * This function determines whether the stack is empty by checking if its
 * top pointer is NULL.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 *
 * @return Returns true if the stack is empty, or false if it contains items.
 */
bool stack_isempty(Stack *stack);
/**
 * @brief Performs semantic checks on an Abstract Syntax Tree (AST) node.
 *
 * This function analyzes the given AST node and checks for semantic errors,
 * such as type compatibility and validity of expressions. It assigns the result
 * type to the AST node based on the semantic analysis.
 *
 * @param node A pointer to the ASTNode structure representing the expression node.
 *             It contains information about the expression, such as its type and properties.
 * @param parser A pointer to the parser_t structure.
 *               The parser must be initialized before calling this function.
 *
 * @return Returns 0 if the semantic check passes without errors, or 1 if there are errors.
 *
 * @note This function may report errors using the `handle_error` function.
 *       It also updates the `resultType` field of the AST node with the inferred type.
 */
int performSemanticCheck(ASTNode* node, parser_t *parser);

/**
 * @brief Retrieves the topmost terminal token from the stack.
 *
 * This function recursively searches the stack for the topmost terminal token,
 * starting from the specified stack item. It returns the terminal token found.
 * If no terminal token is found, it returns a default token (e.g., with type TK_UNKNOWN).
 *
 * @param item A pointer to the StackItem structure.
 *             It represents the starting point for searching the stack.
 *
 * @return Returns the topmost terminal token if found, or a default token otherwise.
 */
token_t stack_top_terminal(StackItem *item);

/**
 * @brief Creates a new Abstract Syntax Tree (AST) node.
 *
 * This function allocates memory for a new AST node, initializes its fields
 * with the provided token and node type, and returns a pointer to the created node.
 *
 * @param token The token associated with the AST node.
 *              It contains information about the token, such as its type and value.
 * @param type The type of the AST node, indicating its role or purpose in the syntax tree.
 *
 * @return Returns a pointer to the newly created AST node.
 *         The caller is responsible for managing the memory of the returned node.
 *
 * @note If memory allocation fails, this function reports an error using the
 *       `handle_error` function and terminates the program.
 */
ASTNode *create_node(token_t token, ASTNodeType type);

/**
 * @brief Parses a non-terminal token and creates an Abstract Syntax Tree (AST) node.
 *
 * This function takes a non-terminal token and creates an AST node of type AST_NON_TERM.
 * The AST node is initialized with the provided token, and its result type is set to AST_NON_TERM.
 *
 * @param token The non-terminal token to be parsed and associated with the AST node.
 *              It contains information about the non-terminal, such as its type and value.
 *
 * @return Returns a pointer to the newly created AST node of type AST_NON_TERM.
 *         The caller is responsible for managing the memory of the returned node.
 *
 * @note This function is typically used during the parsing phase to represent non-terminal symbols.
 */
ASTNode *parse_non_terminal(token_t token);

/**
 * @brief Parses a unary operation and creates an Abstract Syntax Tree (AST) node.
 *
 * This function parses a unary operation by popping the top items from the stack,
 * creating an AST node of type AST_UNARY_OP, and setting its fields accordingly.
 * The result type of the AST node is determined by the result type of the right operand.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 *
 * @return Returns a pointer to the newly created AST node representing the unary operation.
 *         The caller is responsible for managing the memory of the returned node.
 *
 * @note This function assumes that the stack contains the necessary items for parsing
 *       the unary operation (operand and operator). It may modify the stack during execution.
 */
ASTNode *parse_unary(Stack *stack);

/**
 * @brief Parses a binary operation and creates an Abstract Syntax Tree (AST) node.
 *
 * This function parses a binary operation by popping the top items from the stack,
 * creating an AST node of type AST_BINARY_OP, and setting its fields accordingly.
 * The result type of the AST node is determined by performing semantic checks.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 * @param parser A pointer to the parser_t structure.
 *               The parser must be initialized before calling this function.
 *
 * @return Returns a pointer to the newly created AST node representing the binary operation.
 *         The caller is responsible for managing the memory of the returned node.
 *
 * @note This function assumes that the stack contains the necessary items for parsing
 *       the binary operation (left operand, operator, and right operand). It may modify
 *       the stack during execution. Semantic checks are performed using the `performSemanticCheck` function.
 */
ASTNode *parse_binary(Stack *stack, parser_t *parser);

/**
 * @brief Parses a parenthesized expression and creates an Abstract Syntax Tree (AST) node.
 *
 * This function parses a parenthesized expression by checking for the presence of
 * opening and closing parentheses. It then retrieves the expression inside the parentheses
 * and returns an AST node representing the parenthesized expression.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 * @param parser A pointer to the parser_t structure.
 *               The parser must be initialized before calling this function.
 *
 * @return Returns a pointer to the newly created AST node representing the parenthesized expression.
 *         The caller is responsible for managing the memory of the returned node.
 *
 * @note This function assumes that the stack contains the necessary items for parsing
 *       the parenthesized expression (opening and closing parentheses, and the expression inside).
 *       It may modify the stack during execution.
 */
ASTNode *parse_par(Stack *stack, parser_t *parser);

/**
 * @brief Parses an expression and creates an Abstract Syntax Tree (AST) node.
 *
 * This function parses an expression based on the number and types of items on the stack.
 * It handles different cases, such as single tokens, unary operations, binary operations,
 * and parenthesized expressions, and creates the corresponding AST node accordingly.
 *
 * @param stack A pointer to the Stack structure.
 *              The stack must be initialized before calling this function.
 * @param parser A pointer to the parser_t structure.
 *               The parser must be initialized before calling this function.
 *
 * @return Returns a pointer to the newly created AST node representing the parsed expression.
 *         The caller is responsible for managing the memory of the returned node.
 *
 * @note This function assumes that the stack contains the necessary items for parsing
 *       the expression. It may modify the stack during execution. Error handling is done
 *       using the `handle_error` function in case of syntax errors.
 */
ASTNode *parse_expression(Stack *stack, parser_t *parser);

/**
 * @brief Gets the precedence relationship between two tokens.
 *
 * This function determines the precedence relationship between two tokens based on
 * their types. It looks up the precedence in a predefined table and returns the result.
 * If either the top or current token has an invalid type, it returns 'E' indicating a syntax error.
 *
 * @param top The token on the top of the stack.
 *             It contains information about the token, such as its type.
 * @param current The current token being processed.
 *                It contains information about the token, such as its type.
 *
 * @return Returns a character indicating the precedence relationship between the tokens:
 *         '<' if the top token has lower precedence,
 *         '>' if the top token has higher precedence,
 *         '=' if the tokens have equal precedence,
 *         'E' if there is a syntax error.
 *
 * @note This function assumes that the tokens have valid types within the specified range.
 *       The result is determined based on a predefined precedence table.
 */
int get_precedence(token_t top, token_t current);

/**
 * @brief Implements the expression parsing rule using the shunting yard algorithm.
 *
 * This function parses an expression using the shunting yard algorithm, which converts
 * infix expressions to postfix form. It utilizes two stacks, one for operators and one for
 * operands, and follows the precedence and associativity rules to build an Abstract Syntax Tree (AST).
 * The result type of the expression is returned.
 *
 * @param parser A pointer to the parser_t structure.
 *               The parser must be initialized before calling this function.
 * @param tokenArray A pointer to the TokenArray structure containing the input tokens.
 *                   The tokenArray must be initialized before calling this function.
 * @param gen A pointer to the generator_t structure.
 *            The generator must be initialized before calling this function.
 *
 * @return Returns the result type of the parsed expression.
 *         The caller is responsible for handling the result accordingly.
 *
 * @note This function assumes valid input and may report syntax errors using the `handle_error` function.
 *       It uses the shunting yard algorithm to build an AST and returns the result type of the expression.
 *       The `gen_Expr` function is called to generate code for the expression if needed.
 */
tk_type_t rule_expression(parser_t *parser, TokenArray *tokenArray, generator_t* gen);

/**
 * @brief Determines the data type of an identifier in the symbol table.
 *
 * This function searches for the identifier in the local and global symbol tables
 * and returns its data type. If the identifier is not found, it reports a semantic
 * error using the `handle_error` function.
 *
 * @param parser A pointer to the parser_t structure.
 *               The parser must be initialized before calling this function.
 * @param String The identifier (variable name) for which to determine the data type.
 *
 * @return Returns the data type of the identifier if found in the symbol tables.
 *         If the identifier is not found, it reports a semantic error and returns
 *         a default data type (e.g., TK_UNKNOWN).
 *
 * @note This function is typically used during semantic analysis to determine the
 *       data type of variables in expressions.
 */
tk_type_t typeOf_ID(parser_t * parser, char* String);
/**
 * @brief Converts a keyword token representing a literal type to its corresponding data type.
 *
 * This function takes a keyword token representing a literal type (e.g., TK_KW_INT, TK_KW_STRING)
 * and returns its corresponding data type (e.g., TK_INT, TK_STRING). If the input token type is
 * not a recognized literal type, it returns the input token type as is.
 *
 * @param tokenType The token type representing a literal keyword.
 *
 * @return Returns the corresponding data type if the input is a recognized literal type,
 *         otherwise returns the input token type as is.
 *
 * @note This function is useful for converting keyword tokens representing literal types to
 *       their corresponding data types during semantic analysis.
 */
tk_type_t expr_convert_literal_to_datatype(tk_type_t tokenType);

#endif
