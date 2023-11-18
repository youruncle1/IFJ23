
#include <stdbool.h>
#include "error.h"
#include "scanner.h"  
#include "symtable.h" 

#define SYNTAX_ERROR 2
#define ERROR_OCCURED 1
#define ERROR_FREE 0


typedef struct {
    scanner_t *scanner;         // Pointer to the scanner
    token_t current_token;      // Current token being processed
    ErrorCode error_code;       // To store any encountered error
} parser_t;

void parser_init(parser_t *parser, scanner_t *scanner);

/**
 * @brief Function for checking the type of the token
 * @param type expected type of the token
 * @return bool
 */
bool Check_TK_Type ( parser_t *parser, tk_type_t type );

/**
 * @brief Function for getting next token
 * @param parser
 */
void Next_Token(parser_t *parser);

/**
 * @brief Function for checking the type of the next token
 * @param type expected type of the token
 * @param errMsg error message
 * @return bool
 */
bool Check_Next_TK_Type ( tk_type_t type, char* errMsg );

/**
 * @brief Function for parsing the whole program
 * @return int
 */
void Program(parser_t *parser);

/**
 * @brief Function for parsing the block contents
 * @return int
 */
void Block_Contents(parser_t *parser);

/**
 * @brief Function for parsing the block content
 * @return int
*/
void Block_Content(parser_t *parser);

/**
 * @brief Funcion gets the type of the variable
 * @return int
*/
void Type_Opt(parser_t *parser);

/**
 * @brief Function for parsing the variable definition
 * @return int
*/
void Def_Var(parser_t *parser);

/**
 * @brief Function for parsing the variable value
 * @return int
*/
void Def_Value(parser_t *parser);

/**
 * @brief Function to get the type of token
*/
void Type(parser_t *parser);

/**
 * @brief Function for parsing the function definition
 * @return int
*/
void Function_Definition(parser_t *parser);

/**
 * @brief Function for parsing the function parameters
 * @return int
*/
void Parameter_List(parser_t *parser);

/**
 * @brief Function for parsing the function parameters
 * @return int
*/
void Parameter_List_Tail(parser_t *parser);

/**
 * @brief Function for parsing the function parameters
 * @return int
*/
void Parameter_Definition(parser_t *parser);

/**
 * @brief Function to get the return type of the function
 * @return int
*/
void Return_Type(parser_t *parser);

/**
 * @brief Function for parsing the function body
 * @return int
*/
void Function_Body(parser_t *parser);

/**
 * @brief Function for parsing the statement list
 * @return int
*/
void Statement_List(parser_t *parser);

/**
 * @brief Function for parsing individual statements
 * @return int
*/
void Statement(parser_t *parser);

/**
 * @brief Function for parsing function call
 * @return int
*/
void Function_Call(parser_t *parser);

/**
 * @brief Function for parsing the input parameters of function call
 * @return int
*/
void Input_Parameters(parser_t *parser);

/**
 * @brief Helper function for parsing the input parameters of function call
 * @return int
*/
void Input_Parameter_Tail(parser_t *parser);

/**
 * @brief Helper function to parse the input parameters of function call
 * @return int
*/
void Term(parser_t *parser);


