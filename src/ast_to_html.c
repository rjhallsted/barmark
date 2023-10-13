#include "ast_to_html.h"

#include <stdio.h>

#include "ast.h"

void ast_to_html(ASTNode ast[static 1], FILE output_fd[static 1],
                 int unsigned print_tags) {
  HTML_TAG tag = HTML_TAGS[ast->type];
  if (print_tags && tag.show_tag) {
    if (ast->type == ASTN_ORDERED_LIST && ast->options->starting_num != 1) {
      printf("<%s start=\"%lu\">", tag.label, ast->options->starting_num);
    } else {
      printf("<%s>", tag.label);
    }
  }
  if (ast->contents) {
    printf("%s", ast->contents);
  }
  for (size_t i = 0; i < ast->children_count; i++) {
    ast_to_html(ast->children[i], output_fd, tag.wrap_internals && print_tags);
  }
  if (print_tags && tag.show_tag && tag.close) {
    printf("</%s>", (tag.closing_label) ? tag.closing_label : tag.label);
  }
}
