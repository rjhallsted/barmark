#ifndef SYMBOLS_H
#define SYMBOLS_H

typedef struct {
    char *name;
    int id;
} BaseSymbol;

enum BASE_SYMBOL_IDS {
    BASE_SYMBOL_NULL_ID = 0,
    BASE_SYMBOL_H1_ID = 1,
    BASE_SYMBOL_SPACE_ID = 2,
    BASE_SYBMOL_NL_ID = 3,
    BASE_SYBMOL_TEXT_ID = 4,
};

static const BaseSymbol BASE_SYMBOLS[5] = {
    {"null", 0},
    {"H1", 1},
    {"space", 2},
    {"newline", 3},
    {"text", 4}
};

typedef struct {
    int base_symbol_id;
    char *contents;
} Symbol;

#endif // SYMBOLS_H