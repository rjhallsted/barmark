#ifndef SYMBOLS_H
#define SYMBOLS_H

typedef struct {
    char *name;
    int id;
    int hasConst;
    char *constant;
} BaseSymbol;

enum BASE_SYMBOL_IDS {
    /* Has constants */
    BASE_SYMBOL_NULL_ID = 0,
    BASE_SYMBOL_H1_ID = 1,
    BASE_SYMBOL_H2_ID = 2,
    BASE_SYMBOL_SPACE_ID = 3,
    BASE_SYBMOL_NL_ID = 4,
    /* Does not have constants */
    BASE_SYMBOL_NO_MORE_CONSTANTS = 5,
    BASE_SYBMOL_TEXT_ID = 6
};

static const BaseSymbol BASE_SYMBOLS[7] = {
    /* Has Constants */
    {"null", BASE_SYMBOL_NULL_ID, 1, ""},
    {"H1", BASE_SYMBOL_H1_ID, 1, "#"},
    {"H2", BASE_SYMBOL_H2_ID, 1, "##"},
    {"space", BASE_SYMBOL_SPACE_ID, 1, " "},
    {"newline", BASE_SYBMOL_NL_ID, 1, "\n"},
    /* Does not have constants */
    {"special_no_more_constants", BASE_SYMBOL_NO_MORE_CONSTANTS, 0, NULL},
    {"text", BASE_SYBMOL_TEXT_ID, 0, NULL},
};

typedef struct {
    int base_symbol_id;
    char *contents;
} Symbol;

int find_symbol_id(char * item);
Symbol newSymbol(int token_id, char* contents);
Symbol nullSymbol();

#endif // SYMBOLS_H