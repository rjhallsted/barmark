#include "symbols.h"

#include <stdlib.h>
#include <string.h>

///////////////////////////////
// SYMBOL TREE STUFF
///////////////////////////////

SymbolTreeItem *newSymbolTreeItem(char c, const Symbol *symbol) {
  SymbolTreeItem *item = malloc(sizeof(SymbolTreeItem));
  item->c = c;
  item->symbol = symbol;
  item->children = NULL;
  item->children_count = 0;
  return item;
}

void addChildToSymbolTreeItem(SymbolTreeItem *item, SymbolTreeItem *child) {
  item->children = realloc(
      item->children, (sizeof(SymbolTreeItem *) * (item->children_count + 1)));
  item->children[item->children_count] = child;
  item->children_count += 1;
}

SymbolTreeItem *findSymbolTreeItemChild(SymbolTreeItem *item, char c) {
  for (unsigned int i = 0; i < item->children_count; i++) {
    if (item->children[i]->c == c) {
      return item->children[i];
    }
  }
  return NULL;
}

void addToSymbolTree(SymbolTreeItem *root, const Symbol *symbol) {
  SymbolTreeItem *current, *child;

  current = root;
  for (unsigned int i = 0; symbol->constant[i]; i++) {
    child = findSymbolTreeItemChild(current, symbol->constant[i]);
    if (!child) {
      child = newSymbolTreeItem(symbol->constant[i], NULL);
      addChildToSymbolTreeItem(current, child);
    }
    current = child;
  }
  current->symbol = symbol;
}

SymbolTreeItem *buildSymbolTree(void) {
  SymbolTreeItem *root = newSymbolTreeItem('\0', &(SYMBOLS[SYMBOL_TEXT_ID]));
  const Symbol *symbol;
  // we skip the null symbol at index 0;
  for (unsigned int i = 1; i < SYMBOL_COUNT; i++) {
    symbol = &(SYMBOLS[i]);
    if (symbol->constant) {
      addToSymbolTree(root, symbol);
    }
  }
  return root;
}

void freeSymbolTree(SymbolTreeItem *root) {
  for (unsigned int i = 0; i < root->children_count; i++) {
    freeSymbolTree(root->children[i]);
  }
  free(root);
}

///////////////////////////////
// SYMBOL LOOKUP STUFF
///////////////////////////////

unsigned int shouldStopLookAhead(const char *input, const Symbol *symbol) {
  if (strchr(symbol->lookAheadTerminators, *input)) {
    return 1;
  } else {
    return 0;
  }
}

const Symbol *lookupSymbolInner(SymbolTreeItem *item, const char *input) {
  SymbolTreeItem *child = findSymbolTreeItemChild(item, *input);
  const Symbol *symbol = NULL;
  if (child) {
    symbol = lookupSymbolInner(child, input + 1);
  }
  if (!symbol) {
    symbol = item->symbol;
  }
  return symbol;
}

/**
 * @brief find the next symbol symbol available
 *
 * @param tree A pointer to the root of the symbol tree to search
 * @param input A char * to begin lookup from
 * @param symbol A symbol symbol pointer to store the address of the found
 * symbol in
 * @param contents A char ** to store the contents of the symbol in.
 *
 * @return A pointer to the next character after the found symbol. Returns null
 * if at end of string
 */
const char *lookupSymbol(SymbolTreeItem *tree, const char *input,
                         const Symbol **symbol, char **contents) {
  *symbol = lookupSymbolInner(tree, input);

  size_t offset;
  if ((*symbol)->lookAheadTerminators != NULL) {
    offset = 0;
    while (!shouldStopLookAhead(input + offset, *symbol)) {
      offset += 1;  // May need to modify this once lookahead is in terms of
                    // UTF-8 instead of ASCII
    }
  } else {
    offset = strlen((*symbol)->constant);
  }
  *contents = strndup(input, offset);

  if (offset > 0) {
    return input + offset;
  } else {
    return NULL;
  }
}

Token *newToken(const Symbol *symbol, char *contents) {
  Token *token = malloc(sizeof(Token));
  token->symbol = symbol;
  token->contents = contents;
  token->next = NULL;
  return token;
}

Token *nullToken(void) {
  return newToken(&(SYMBOLS[SYMBOL_NULL_ID]), strdup(""));
}

void free_token(Token *token) {
  free(token->contents);
  free(token);
}

void free_token_list(Token *head) {
  Token *next = head;
  while (head) {
    free(head);
    next = next->next;
    head = next;
  }
}

int is_whitespace_token(Token *token) {
  const unsigned int whitespace_symbols[3] = {SYMBOL_SPACE_ID, SYMBOL_TAB_ID,
                                              SYMBOL_NL_ID};

  if (token == NULL) {
    return 0;
  }

  for (int i = 0; i < 3; ++i) {
    if (token->symbol->id == whitespace_symbols[i]) {
      return 0;
    }
  }
  return 1;
}

Token *join_token_array(Token **tokens, size_t count) {
  Token *head = nullToken();
  Token *current = head;

  for (size_t i = 0; i < count; i++) {
    current->next = tokens[i];
    current = current->next;
  }
  current = head->next;
  free_token(head);
  return current;
}

Token *advance_token_list_by(Token *head, size_t num) {
  size_t i = 0;
  while (i < num && head) {
    head = head->next;
    i += 1;
  }
  return head;
}
