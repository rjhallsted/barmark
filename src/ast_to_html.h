#ifndef AST_TO_HTML_H
#define AST_TO_HTML_H

#include <stdio.h>

#include "ast.h"

void ast_to_html(ASTNode *ast, FILE *output_fd, unsigned int print_tags);

typedef struct {
  const char *open;
  const char *close;
  unsigned int wrap_internals;
} HTML_TAG;

// NOTE: These must be in the same order as the AST_NODE_TYPES enum in ast.h
static const HTML_TAG HTML_TAGS[] = {
    {.open = "", .close = "", .wrap_internals = 1},  // DOCUMENT
    {.open = "<pre><code>",
     .close = "</code></pre>",
     .wrap_internals = 0},  // CODE_BLOCK
    {.open = "<blockquote>",
     .close = "</blockquote>",
     .wrap_internals = 1},                                    // BLOCK_QUOTE
    {.open = "<ul>", .close = "</ul>", .wrap_internals = 1},  // UNORDERED_LIST
    {.open = "<li>", .close = "</li>", .wrap_internals = 1},  // LIST_ITEM
    {.open = "<p>", .close = "</p>", .wrap_internals = 1},    // PARAGRAPH
    {.open = "<h1>", .close = "</h1>", .wrap_internals = 1},  // H1
};

#endif  // AST_TO_HTML_H
