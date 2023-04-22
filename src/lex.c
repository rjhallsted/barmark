#include "lex.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbols.h"
#include "util.h"

Symbol *handle_line(char *line, size_t *num_sybmols,
                    SymbolTreeItem *symbolTree) {
  Symbol *symbols = NULL;
  const BaseSymbol *base;
  *num_sybmols = 0;
  char *contents = NULL;

  const char *lp = line;
  while ((lp = lookupBaseSymbol(symbolTree, lp, &base, &contents)) != NULL) {
    symbols = realloc(symbols, sizeof(Symbol) * (*num_sybmols + 1));
    symbols[*num_sybmols] = newSymbol(base, contents);
    *num_sybmols += 1;
  }
  return symbols;
}

Symbol *concat(Symbol *a, Symbol *b, size_t aSize, size_t bSize) {
  a = realloc(a, sizeof(Symbol) * (aSize + bSize + 1));
  for (size_t i = 0; i < bSize; i++) {
    a[aSize + i] = b[i];
  }
  a[aSize + bSize] = nullSymbol();
  return a;
}

void traverseSymbolTree(SymbolTreeItem *root, Symbol *container) {
  if (root->symbol) {
    container[root->symbol->id - 1] = newSymbol(root->symbol, strdup("foo"));
  }
  for (unsigned int i = 0; i < root->children_count; i++) {
    traverseSymbolTree(root->children[i], container);
  }
}

Symbol *lex(FILE *fd) {
  size_t symbol_count = 0, buff_len = 0;
  size_t *read_symbols = malloc(sizeof(size_t));
  char *line = NULL;
  Symbol *allSymbols = malloc(sizeof(Symbol) * 1);
  allSymbols[0] = nullSymbol();
  SymbolTreeItem *symbolTree = buildSymbolTree();

  while (!feof(fd)) {
    if (ferror(fd)) {
      printf("File reading error. Errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }

    getline(&line, &buff_len, fd);
    Symbol *symbols = handle_line(line, read_symbols, symbolTree);
    // copy symbol pointers to our all sybmols array
    allSymbols = concat(allSymbols, symbols, symbol_count, *read_symbols);
    symbol_count += *read_symbols;
    free(symbols);  // free tmp symbol pointer array
  }
  free(read_symbols);
  freeSymbolTree(symbolTree);
  return allSymbols;
}
