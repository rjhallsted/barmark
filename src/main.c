#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "symbols.h"
#include "util.h"

void showUsage(char *errorMessage) {
  const char *usageText = "\t./barmark <filename>";
  printf("%s\nUSAGE:\n%s\n", errorMessage, usageText);
}

void printTokens(Token *tokens) {
  size_t i = 0;
  while (tokens[i].symbol->id != SYMBOL_NULL_ID) {
    printf("i: %zu\n", i);
    char *symbolName = tokens[i].symbol->name;
    printf("%s: '%s'\n", symbolName, tokens[i].contents);
    i++;
  }
}

int main(int argc, char **argv) {
  FILE *fd;

  if (argc > 1) {
    fd = openFile(argv[1]);
  } else {
    fd = stdin;
  }
  Token *tokens = lex(fd);
  printTokens(tokens);
  return 0;
}
