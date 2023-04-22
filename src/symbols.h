#ifndef SYMBOLS_H
#define SYMBOLS_H

typedef struct {
    char *name;
    int id;
} BaseSymbol;

enum BASE_SYMBOL_IDS {
    BASE_SYMBOL_NULL_ID = 0,
    BASE_SYMBOL_H1_ID = 1,
    BASE_SYMBOL_H2_ID = 2,
    BASE_SYMBOL_SPACE_ID = 3,
    BASE_SYBMOL_NL_ID = 4,
    BASE_SYBMOL_TEXT_ID = 5,
};

static const BaseSymbol BASE_SYMBOLS[6] = {
    {"null", BASE_SYMBOL_NULL_ID},
    {"H1", BASE_SYMBOL_H1_ID},
    {"H2", BASE_SYMBOL_H2_ID},
    {"space", BASE_SYMBOL_SPACE_ID},
    {"newline", BASE_SYBMOL_NL_ID},
    {"text", BASE_SYBMOL_TEXT_ID},
};

typedef struct {
    int base_symbol_id;
    char *contents;
} Symbol;

int find_symbol_id(char * item);
Symbol newSymbol(int token_id, char* contents);
Symbol nullSymbol();

#endif // SYMBOLS_H