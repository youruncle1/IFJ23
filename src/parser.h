
#include <stdbool.h>
#include "error.h"

#define SYNTAX_ERROR 2
#define ERROR_OCCURED 1
#define ERROR_FREE 0


/**
 * @brief Function for checking the type of the token
 * @param type expected type of the token
 * @return bool
 */
bool Check_Token_Type ( tk_type_t type );

/**
 * @brief Function for checking the type of the next token
 * @param type expected type of the token
 * @param errMsg error message
 * @return bool
 */
bool Check_Next_Token_Type ( tk_type_t type, char* errMsg );

/**
 * @brief Function for parsing the whole program
 * @return int
 */
int Program();

/**
 * @brief Function for parsing the block contents
 * @return int
 */
int Block_Contents();

/**
 * @brief Function for parsing the block content
 * @return int
*/
int Block_Content();

/**
 * @brief Funcion gets the type of the variable
 * @return int
*/
int Type_Opt();

/**
 * @brief Function for parsing the variable definition
 * @return int
*/
int Def_Var();

/**
 * @brief Function for parsing the variable value
 * @return int
*/
int Def_Value();

/**
 * @brief Function to get the type of token
*/
int Type();

/**
 * @brief Function for parsing the function definition
 * @return int
*/
int Function_Definition();

/**
 * @brief Function for parsing the function parameters
 * @return int
*/
int Parameter_List();

/**
 * @brief Function for parsing the function parameters
 * @return int
*/
int Parameter_List_Tail();

/**
 * @brief Function for parsing the function parameters
 * @return int
*/
int Parameter_Definition();

/**
 * @brief Function to get the return type of the function
 * @return int
*/
int Return_Type();

/**
 * @brief Function for parsing the function body
 * @return int
*/
int Function_Body();

/**
 * @brief Function for parsing the statement list
 * @return int
*/
int Statement_List();

/**
 * @brief Function for parsing individual statements
 * @return int
*/
int Statement();

/**
 * @brief Function for parsing function call
 * @return int
*/
int Function_Call();

/**
 * @brief Function for parsing the input parameters of function call
 * @return int
*/
int Input_Parameters();

/**
 * @brief Helper function for parsing the input parameters of function call
 * @return int
*/
int Input_Parameter_Tail();

/**
 * @brief Helper function to parse the input parameters of function call
 * @return int
*/
int Term();


