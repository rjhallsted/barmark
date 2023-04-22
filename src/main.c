#include <stdio.h>
#include <stdlib.h>
#include "symbols.h"
#include "lex.h"
#include "util.h"

void showUsage(char *errorMessage)
{
    const char *usageText = "\t./barmark <filename>";
    printf("%s\nUSAGE:\n%s\n", errorMessage, usageText);
}

void printSybmols(Symbol *symbols)
{
    size_t i = 0;
    while (symbols[i].base->id != BASE_SYMBOL_NULL_ID)
    {
        printf("i: %zu\n", i);
        char *symbolName = symbols[i].base->name;
        printf("%s: '%s'\n", symbolName, symbols[i].contents);
        i++;
    }
}

int main(int argc, char **argv)
{
    FILE *fd;

    if (argc > 1)
    {
        fd = openFile(argv[1]);
    }
    else
    {
        fd = stdin;
    }
    Symbol *symbols = lex(fd);
    printSybmols(symbols);
    return 0;
}
