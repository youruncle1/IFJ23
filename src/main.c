#include "scanner.h"
#include "parser.h" 


int main() {
    scanner_t scanner;
    scanner.input = stdin; 
    scanner.line = 1;

    parser_t parser = initParser(&scanner);

    define_builtin_functions(&parser.global_frame);

    TokenArray *tokenArray = initTokenArray();

    firstParserPass(&parser, tokenArray);
    
    parseProgram(&parser,tokenArray);

    return 0;
}
