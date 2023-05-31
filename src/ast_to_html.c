#include "ast_to_html.h"

#include <stdio.h>

#include "ast.h"

void ast_to_html(ASTNode *ast, FILE *output_fd, unsigned int print_tags) {
  if (print_tags) {
    printf("%s", HTML_TAGS[ast->type].open);
  }
  if (ast->contents) {
    printf("%s", ast->contents);
  } else {
    for (size_t i = 0; i < ast->children_count; i++) {
      ast_to_html(ast->children[i], output_fd,
                  HTML_TAGS[ast->type].wrap_internals && print_tags);
    }
  }
  if (print_tags) {
    printf("%s", HTML_TAGS[ast->type].close);
  }
}
