#ifndef AST_TO_HTML_H
#define AST_TO_HTML_H

#include <stdio.h>

#include "ast.h"

void ast_to_html(ASTNode *ast, FILE *output_fd);

typedef struct {
  const char *open;
  const char *close;
} HTML_TAG;

// NOTE: These must be in the same order as the AST_NODE_TYPES enum in ast.h
static const HTML_TAG HTML_TAGS[] = {
    {.open = "", .close = ""},                          // DOCUMENT
    {.open = "", .close = ""},                          // TEXT
    {.open = "<pre><code>", .close = "</code></pre>"},  // CODE_BLOCK
    {.open = "<p>", .close = "</p>"},                   // PARAGRAPH
    {.open = "<code>", .close = "</code>"},             // CODE
};

#endif  // AST_TO_HTML_H
