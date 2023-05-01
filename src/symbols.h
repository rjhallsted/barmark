#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdlib.h>

/* Base Symbol lookup stuff */
typedef struct {
  char *name;
  unsigned int id;
  char *constant;
  char *lookAheadTerminators;
} Symbol;

enum SYMBOL_IDS {
  /* Has constants */
  SYMBOL_NULL_ID = 0,
  SYMBOL_H1_ID = 1,
  SYMBOL_H2_ID = 2,
  SYMBOL_SPACE_ID = 3,
  SYMBOL_TAB_ID = 4,
  SYMBOL_NL_ID = 5,
  /* Does not have constants */
  SYMBOL_TEXT_ID = 6
};

#define SYMBOL_COUNT 7

static const Symbol SYMBOLS[SYMBOL_COUNT] = {
    /* Has Constants */
    {"null", SYMBOL_NULL_ID, "", NULL},
    {"H1", SYMBOL_H1_ID, "#", NULL},
    {"H2", SYMBOL_H2_ID, "##", NULL},
    {"space", SYMBOL_SPACE_ID, " ", NULL},
    {"tab", SYMBOL_TAB_ID, "\t", NULL},
    {"newline", SYMBOL_NL_ID, "\n", NULL},
    {"text", SYMBOL_TEXT_ID, NULL, " \t\n"},  // NOTE: Add more terminators as I
                                              // add symbols
};

/* Symbol Tree stuff */
typedef struct SymbolTreeItem {
  char c;
  const Symbol *symbol;
  struct SymbolTreeItem **children;
  size_t children_count;
} SymbolTreeItem;

SymbolTreeItem *buildSymbolTree(void);
void freeSymbolTree(SymbolTreeItem *root);

/* Symbol lookup stuff */
const char *lookupSymbol(SymbolTreeItem *tree, const char *input,
                         const Symbol **symbol, char **contents);

/* Token stuff */
typedef struct Token {
  const Symbol *symbol;
  char *contents;
  struct Token *next;
} Token;

Token *newToken(const Symbol *symbol, char *contents);
Token *nullToken(void);
void free_token(Token *token);
void free_token_list(Token *head);
int is_whitespace_token(Token *token);

Token *join_token_array(Token **tokens, size_t count);
Token *advance_token_list_by(Token *head, size_t num);

#endif  // SYMBOLS_H
