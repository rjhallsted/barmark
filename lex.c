#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lex.h"
#include "symbols.h"
#include "util.h"

// TODO: Change this to a lookup table at some point for O(symbols) ops instead of 0(strlen * symbols)
int find_symbol_id(char * item) {
    if (strcmp(item, "#") == 0) {
        return BASE_SYMBOL_H1_ID;
    } else if (strcmp(item, " ") == 0) {
        return BASE_SYMBOL_SPACE_ID;
    } else if (strcmp(item, "\n") == 0) {
        return BASE_SYBMOL_NL_ID;
    } else {
        return BASE_SYBMOL_TEXT_ID;
    }
}

Symbol newSymbol(int token_id, char* contents) {
    Symbol sym = {token_id, strdup(contents)};
    return sym;
}

Symbol nullSymbol() {
    return newSymbol(BASE_SYMBOL_NULL_ID, "");
}

Symbol* handle_line(char* line, size_t len, size_t *num_sybmols) {
    line = addNullTerm(line, len);
    char *token, *toFree;
    Symbol *symbols = NULL;
    
    *num_sybmols = 0; 
    toFree = line;
    while ((token = strsep(&line, " ")) != NULL) {
        char* newLineLoc = strchr(token, '\n');
        int trailingSpace = line != NULL;
        if (newLineLoc) {
            symbols = realloc(symbols, sizeof(Symbol) * (*num_sybmols + 2));
            *newLineLoc = '\0';
            symbols[*num_sybmols] = newSymbol(find_symbol_id(token), token);
            symbols[*num_sybmols+1] = newSymbol(BASE_SYBMOL_NL_ID, "\n");
            *num_sybmols += 2;
        } else if (trailingSpace) {
            symbols = realloc(symbols, sizeof(Symbol) * (*num_sybmols + 2));
            symbols[*num_sybmols] = newSymbol(find_symbol_id(token), token);
            symbols[*num_sybmols+1] = newSymbol(BASE_SYMBOL_SPACE_ID, " ");
            *num_sybmols += 2;
        } else { // !newLineLoc && !trailingSpace
            symbols = realloc(symbols, sizeof(Symbol) * (*num_sybmols + 1));
            symbols[*num_sybmols] = newSymbol(find_symbol_id(token), token);
            *num_sybmols += 1;
        }
    }
    free(toFree);
    return symbols;
}

Symbol* lex(FILE* fd) {
    Symbol* allSymbols = malloc(sizeof(Symbol) * 1);
    allSymbols[0] = nullSymbol();
    size_t symbol_count = 0;

    size_t *len;
    char *line;
    size_t *read_symbols = malloc(sizeof(size_t));
    while (!feof(fd)) {
        line = fgetln(fd, len);
        Symbol* symbols = handle_line(line, *len, read_symbols); //line is freed by handle_line
        //copy symbol pointers to our all sybmols array
        allSymbols = realloc(allSymbols, sizeof(Symbol) * (symbol_count + *read_symbols + 1));
        for (size_t i = 0; i < *read_symbols; i++) {
            allSymbols[symbol_count+i] = symbols[i];
        }
        symbol_count += *read_symbols;
        allSymbols[symbol_count] = nullSymbol();
        free(symbols); //free tmp symbol pointer array
    }
    free(read_symbols);
    return allSymbols;
}