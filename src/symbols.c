#include <string.h>
#include "symbols.h"

#include <stdio.h>

// TODO: Change this to a lookup table at some point for O(symbols) ops instead of 0(strlen * symbols)
int find_symbol_id(char * item) {
    int i = 0;
    while (i < BASE_SYMBOL_NO_MORE_CONSTANTS) {
        if (strcmp(item, BASE_SYMBOLS[i].constant) == 0) {
            return BASE_SYMBOLS[i].id;
        }
        i++;
    }
    return BASE_SYBMOL_TEXT_ID;
}

Symbol newSymbol(int token_id, char* contents) {
    Symbol sym = {token_id, strdup(contents)};
    return sym;
}

Symbol nullSymbol() {
    return newSymbol(BASE_SYMBOL_NULL_ID, "");
}
