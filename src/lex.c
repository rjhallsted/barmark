#include "lex.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbols.h"
#include "util.h"

Token *handle_line(char *line, SymbolTreeItem *symbolTree) {
  Token *head = nullToken();
  Token *current = head;
  const Symbol *symbol;
  char *contents = NULL;

  const char *lp = line;
  while ((lp = lookupSymbol(symbolTree, lp, &symbol, &contents)) != NULL) {
    current->next = newToken(symbol, contents);
  }
  current = head->next;
  free_token(head);
  return current;
}

Token *concat(Token *a, Token *b) {
  Token *head = nullToken();
  Token *current = head;
  current->next = a;
  while (current->next) {
    current = current->next;
  }
  current->next = b;
  current = head->next;
  free_token(head);
  return current;
}

void traverseSymbolTree(SymbolTreeItem *root, Token **container) {
  if (root->symbol) {
    container[root->symbol->id - 1] = newToken(root->symbol, strdup("foo"));
  }
  for (unsigned int i = 0; i < root->children_count; i++) {
    traverseSymbolTree(root->children[i], container);
  }
}

Token *find_last_token(Token *head) {
  while (head->next) {
    head = head->next;
  }
  return head;
}

Token *lex(FILE *fd) {
  size_t buff_len = 0;
  char *line = NULL;
  Token *head = nullToken();
  Token *current = head;
  SymbolTreeItem *symbolTree = buildSymbolTree();

  while (!feof(fd)) {
    if (ferror(fd)) {
      printf("File reading error. Errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }

    getline(&line, &buff_len, fd);
    Token *new_tokens = handle_line(line, symbolTree);
    // copy symbol pointers to our all sybmols array
    current = concat(current, new_tokens);
    current =
        find_last_token(current);  // advance to save iterations in the future
  }
  freeSymbolTree(symbolTree);
  current = head->next;
  free_token(head);
  return current;
}
