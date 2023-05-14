#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "ast_to_html.h"
#include "lex.h"
#include "parse.h"
#include "symbols.h"
#include "util.h"

void showUsage(char *errorMessage) {
  const char *usageText = "\t./barmark <filename>";
  printf("%s\nUSAGE:\n%s\n", errorMessage, usageText);
}

void printTokens(Token *token) {
  size_t i = 0;
  while (token) {
    printf("i: %zu\n", i);
    printf("%s: '%s'\n", token->symbol->name, token->contents);
    i++;
    token = token->next;
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
  // printTokens(tokens);
  ASTNode *root = ast_from_tokens(tokens);
  free_token_list(tokens);
  ast_to_html(root, stdout);  // TODO: allow for output files other than stdout
  ast_free_node(root);
  return 0;
}
