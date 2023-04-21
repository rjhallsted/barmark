#include <stdio.h>
#include <stdlib.h>
#include "symbols.h"
#include "lex.h"


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

FILE* openFile(char *path) {
    FILE* fd = fopen(path, "r");
    if (fd == NULL) {
        printf("Error opening file: '%s'\n", path);
        exit(EXIT_FAILURE);
    }
    return fd;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE* fd = openFile(argv[1]);
        Symbol* symbols = lex(fd);
        printSybmols(symbols);
    } else {
        showUsage("No file name given.");
    }
    return 0;
}