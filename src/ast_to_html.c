#include "ast_to_html.h"

#include <stdio.h>

#include "ast.h"

void ast_to_html(ASTNode ast[static 1], FILE output_fd[static 1],
                 int unsigned print_tags) {
  HTML_TAG tag = HTML_TAGS[ast->type];
  if (print_tags && tag.show_tag) {
    if (ast->type == ASTN_ORDERED_LIST && ast->options->reference_num != 1) {
      printf("<%s start=\"%lu\">", tag.label, ast->options->reference_num);
    } else if (ast->type == ASTN_FENCED_CODE_BLOCK && ast->contents) {
      printf("<%s class=\"language-%s\">", tag.label, ast->contents);
    } else {
      printf("<%s>", tag.label);
    }
  }
  if (ast->type == ASTN_TEXT && ast->contents) {
    printf("%s", ast->contents);
  }
  ast = ast->first_child;
  while (ast) {
    ast_to_html(ast, output_fd, tag.wrap_internals && print_tags);
    ast = ast->next;
  }
  if (print_tags && tag.show_tag && tag.close) {
    printf("</%s>", (tag.closing_label) ? tag.closing_label : tag.label);
  }
}
