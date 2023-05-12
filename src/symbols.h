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

#define SYMBOL_NULL \
  { "null", SYMBOL_NULL_ID, "", NULL }
#define SYMBOL_H1 \
  { "H1", SYMBOL_H1_ID, "#", NULL }
#define SYMBOL_H2 \
  { "H2", SYMBOL_H2_ID, "##", NULL }
#define SYMBOL_SPACE \
  { "space", SYMBOL_SPACE_ID, " ", NULL }
#define SYMBOL_TAB \
  { "tab", SYMBOL_TAB_ID, "\t", NULL }
#define SYMBOL_NL \
  { "newline", SYMBOL_NL_ID, "\n", NULL }
#define SYMBOL_TEXT \
  { "text", SYMBOL_TEXT_ID, NULL, " \t\n" }

static const Symbol SYMBOLS[SYMBOL_COUNT] = {
    /* Has Constants */
    SYMBOL_NULL, SYMBOL_H1,     SYMBOL_H2, SYMBOL_SPACE,
    SYMBOL_NL,   SYMBOL_TEXT_ID
    // NOTE: Add more terminators
    // as I add symbols
};

/*
  SymbolPattern pattern[] = [or(SYMBOL_TAB, [SYMBOL_SPACE,SYMBOL_SPACE,
  SYMBOL_SPACE, SYMBOL_SPACE]), SYMBOL_SPLAT, SYMBOL_NL]
  if (match_symbol_pattern(token, pattern)) {
    ....
  }

  to do this, i'll need a symbol pattern struct. It can contain multiple
  sequences of tokens to match against. and has a pointer to another symbol
  pattern or null. Basically, will try to match against all possible sequences,
  if one matches, move forward to next pattern. If null, return true.

  Then will need to write some defines (or other preprocessor directive) that
  generate those easily.
*/

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

/* Token stream DSL stuff */

#endif  // SYMBOLS_H
