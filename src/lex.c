#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lex.h"
#include "symbols.h"
#include "util.h"

Symbol* handle_line(char* line, size_t *num_sybmols, SymbolTreeItem *symbolTree) {
    Symbol *symbols = NULL;
    const BaseSymbol *base;
    *num_sybmols = 0;
    char *contents = NULL;
    
    char *lp = line;
    while ((lp = lookupBaseSymbol(symbolTree, lp, &base, &contents)) != NULL) {
        symbols = realloc(symbols, sizeof(Symbol) * (*num_sybmols + 1));
        symbols[*num_sybmols] = newSymbol(base, contents);
        *num_sybmols += 1;
    }

    // while ((token = strsep(&local_line, " ")) != NULL && token[0] != '\0') {
    //     char* newLineLoc = strchr(token, '\n');
    //     int trailingSpace = line != NULL;
    //     if (newLineLoc == token) { // have new line but no other token, token and newLineLoc are the same
    //         symbols = realloc(symbols, sizeof(Symbol) * (*num_sybmols + 1));
    //         symbols[*num_sybmols] = newSymbol(BASE_SYBMOL_NL_ID, token);
    //         *num_sybmols += 1;
    //     } else if (newLineLoc) { // have new line and some meaningful token
    //         symbols = realloc(symbols, sizeof(Symbol) * (*num_sybmols + 2));
    //         *newLineLoc = '\0';
    //         symbols[*num_sybmols] = newSymbol(find_symbol_id(token), token);
    //         symbols[*num_sybmols+1] = newSymbol(BASE_SYBMOL_NL_ID, "\n");
    //         *num_sybmols += 2;
    //     } else if (trailingSpace) {
    //         symbols = realloc(symbols, sizeof(Symbol) * (*num_sybmols + 2));
    //         symbols[*num_sybmols] = newSymbol(find_symbol_id(token), token);
    //         symbols[*num_sybmols+1] = newSymbol(BASE_SYMBOL_SPACE_ID, " ");
    //         *num_sybmols += 2;
    //     } else { // !newLineLoc && !trailingSpace
    //         symbols = realloc(symbols, sizeof(Symbol) * (*num_sybmols + 1));
    //         symbols[*num_sybmols] = newSymbol(find_symbol_id(token), token);
    //         *num_sybmols += 1;
    //     }
    // }
    return symbols;
}

Symbol* concat(Symbol* a, Symbol* b, size_t aSize, size_t bSize) {
    a = realloc(a, sizeof(Symbol) * (aSize + bSize + 1));
    for (size_t i = 0; i < bSize; i++) {
        a[aSize+i] = b[i];
    }
    a[aSize + bSize] = nullSymbol();
    return a;
}

Symbol* lex(FILE* fd) {
    size_t symbol_count, buff_len, line_len = 0;
    size_t *read_symbols = malloc(sizeof(size_t));
    char *line = NULL;
    Symbol* allSymbols = malloc(sizeof(Symbol) * 1);
    allSymbols[0] = nullSymbol();
    SymbolTreeItem *symbolTree = buildSymbolTree();

    while (!feof(fd)) {
        if (ferror(fd)) {
            printf("File reading error. Errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        
        line_len = getline(&line, &buff_len, fd);
        Symbol* symbols = handle_line(line, read_symbols, symbolTree);
        //copy symbol pointers to our all sybmols array
        allSymbols = concat(allSymbols, symbols, symbol_count, *read_symbols);
        symbol_count += *read_symbols;
        free(symbols); //free tmp symbol pointer array
    }
    free(read_symbols);
    freeSymbolTree(symbolTree);
    return allSymbols;
}