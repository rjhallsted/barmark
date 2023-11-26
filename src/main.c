#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "ast_to_html.h"
#include "blocks.h"
#include "inline.h"
#include "util.h"

void showUsage(char const errorMessage[static 1]) {
  char const *const usageText = "\t./barmark <filename>";
  printf("%s\nUSAGE:\n%s\n", errorMessage, usageText);
}

int main(int argc, char *argv[argc + 1]) {
  FILE *fd = stdin;

  if (argc > 1) {
    fd = openFile(argv[1]);
  }

  ASTNode *root = build_block_structure(fd);
  parse_inline(root);
  finalize_tree(root);
  if (f_debug()) {
    print_tree(root, 0);
  }

  // TODO: allow for output files other than stdout
  ast_to_html(root, stdout, 1);
  ast_free_node(root);
  return 0;
}
