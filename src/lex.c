#include "lex.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbols.h"
#include "util.h"

TokenStream handle_line(char *line, size_t *num_tokens,
                        SymbolTreeItem *symbolTree) {
  TokenStream tokens = NULL;
  const Symbol *symbol;
  *num_tokens = 0;
  char *contents = NULL;

  const char *lp = line;
  while ((lp = lookupSymbol(symbolTree, lp, &symbol, &contents)) != NULL) {
    tokens = realloc(tokens, sizeof(Symbol) * (*num_tokens + 1));
    tokens[*num_tokens] = newToken(symbol, contents);
    *num_tokens += 1;
  }
  return tokens;
}

TokenStream concat(Token *a, Token *b, size_t aSize, size_t bSize) {
  a = realloc(a, sizeof(Token) * (aSize + bSize + 1));
  for (size_t i = 0; i < bSize; i++) {
    a[aSize + i] = b[i];
  }
  a[aSize + bSize] = nullToken();
  return a;
}

void traverseSymbolTree(SymbolTreeItem *root, Token *container) {
  if (root->symbol) {
    container[root->symbol->id - 1] = newToken(root->symbol, strdup("foo"));
  }
  for (unsigned int i = 0; i < root->children_count; i++) {
    traverseSymbolTree(root->children[i], container);
  }
}

TokenStream lex(FILE *fd) {
  size_t token_count = 0, buff_len = 0;
  size_t *read_tokens = malloc(sizeof(size_t));
  char *line = NULL;
  TokenStream allTokens = malloc(sizeof(Token) * 1);
  allTokens[0] = nullToken();
  SymbolTreeItem *symbolTree = buildSymbolTree();

  while (!feof(fd)) {
    if (ferror(fd)) {
      printf("File reading error. Errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }

    getline(&line, &buff_len, fd);
    TokenStream tokens = handle_line(line, read_tokens, symbolTree);
    // copy symbol pointers to our all sybmols array
    allTokens = concat(allTokens, tokens, token_count, *read_tokens);
    token_count += *read_tokens;
    free(tokens);  // free tmp symbol pointer array
  }
  free(read_tokens);
  freeSymbolTree(symbolTree);
  return allTokens;
}
