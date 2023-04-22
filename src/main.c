#include <stdio.h>
#include <stdlib.h>
#include "symbols.h"
#include "lex.h"
#include "util.h"


void showUsage(char* errorMessage) {
    const char* usageText = "\t./barmark <filename>";
    printf("%s\nUSAGE:\n%s\n", errorMessage, usageText);
}

void printSybmols(Symbol* symbols) {
    size_t i = 0;
    while(symbols[i].base_symbol_id != BASE_SYMBOL_NULL_ID) {
        char *symbolName = BASE_SYMBOLS[symbols[i].base_symbol_id].name;
        printf("%s: '%s'\n", symbolName, symbols[i].contents);
        i++;
    }
}

// TODO: Need to be able to handle UTF-8 characters.

int main(int argc, char **argv) {
    printf("%d\n", '→');

    // char *line = NULL;
    // size_t len = 0;
    // size_t line_size = 0;
    // line_size = getline(&line, &len, stdin);
    // printf("line: %s\n", line);

    // FILE* fd;

    // if (argc > 1) {
    //     fd = openFile(argv[1]);
    // } else {
    //     fd = stdin;
    // }
    // Symbol* symbols = lex(fd);
    // printSybmols(symbols);
    return 0;
}