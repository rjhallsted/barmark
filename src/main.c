#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "ast_to_html.h"
#include "blocks.h"
#include "util.h"

void showUsage(char *errorMessage) {
  const char *usageText = "\t./barmark <filename>";
  printf("%s\nUSAGE:\n%s\n", errorMessage, usageText);
}

int main(int argc, char **argv) {
  FILE *fd;

  if (argc > 1) {
    fd = openFile(argv[1]);
  } else {
    fd = stdin;
  }

  ASTNode *root = build_block_structure(fd);
  // TODO: allow for output files other than stdout
  ast_to_html(root, stdout, 1);
  ast_free_node(root);
  return 0;
}
