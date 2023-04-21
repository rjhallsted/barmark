#include <string.h>
#include "symbols.h"

// TODO: Change this to a lookup table at some point for O(symbols) ops instead of 0(strlen * symbols)
int find_symbol_id(char * item) {
    if (strcmp(item, "#") == 0) {
        return BASE_SYMBOL_H1_ID;
    } else if (strcmp(item, "##") == 0) {
        return BASE_SYMBOL_H2_ID;
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
