#include "scanner.h"
#include "parser.h" 
#include "generator.h"


int main() {
    scanner_t scanner;
    scanner.input = stdin; 
    scanner.line = 1;

    generator_t gen = gen_Init();

    parser_t parser = initParser(&scanner);

    define_builtin_functions(&parser.global_frame);

    TokenArray *tokenArray = initTokenArray();

    firstParserPass(&parser, tokenArray);
    
    parseProgram(&parser,tokenArray, &gen);

    return 0;
}
