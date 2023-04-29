#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdlib.h>

/* Base Symbol lookup stuff */
typedef struct {
  char *name;
  unsigned int id;
  char *constant;
  char *lookAheadTerminators;
} BaseSymbol;

enum BASE_SYMBOL_IDS {
  /* Has constants */
  BASE_SYMBOL_NULL_ID = 0,
  BASE_SYMBOL_H1_ID = 1,
  BASE_SYMBOL_H2_ID = 2,
  BASE_SYMBOL_SPACE_ID = 3,
  BASE_SYMBOL_TAB_ID = 4,
  BASE_SYBMOL_NL_ID = 5,
  /* Does not have constants */
  BASE_SYMBOL_TEXT_ID = 6
};

#define BASE_SYMBOL_COUNT 7

static const BaseSymbol BASE_SYMBOLS[BASE_SYMBOL_COUNT] = {
    /* Has Constants */
    {"null", BASE_SYMBOL_NULL_ID, "", NULL},
    {"H1", BASE_SYMBOL_H1_ID, "#", NULL},
    {"H2", BASE_SYMBOL_H2_ID, "##", NULL},
    {"space", BASE_SYMBOL_SPACE_ID, " ", NULL},
    {"tab", BASE_SYMBOL_TAB_ID, "\t", NULL},
    {"newline", BASE_SYBMOL_NL_ID, "\n", NULL},
    {"text", BASE_SYMBOL_TEXT_ID, NULL,
     " \t\n"},  // NOTE: Add more terminators as I add symbols
};

/* Symbol Tree stuff */
typedef struct SymbolTreeItem {
  char c;
  const BaseSymbol *symbol;
  struct SymbolTreeItem **children;
  size_t children_count;
} SymbolTreeItem;

SymbolTreeItem *buildSymbolTree(void);
void freeSymbolTree(SymbolTreeItem *root);

/* Symbol lookup stuff */
typedef struct {
  const BaseSymbol *base;
  char *contents;
} Symbol;

const char *lookupBaseSymbol(SymbolTreeItem *tree, const char *input,
                             const BaseSymbol **symbol, char **contents);
Symbol newSymbol(const BaseSymbol *base, char *contents);
Symbol nullSymbol(void);

#endif  // SYMBOLS_H
