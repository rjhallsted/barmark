#ifndef AST_TO_HTML_H
#define AST_TO_HTML_H

#include <stdio.h>

#include "ast.h"

void ast_to_html(ASTNode ast[static 1], FILE output_fd[static 1],
                 int unsigned print_tags);

typedef struct {
  const char *open;
  const char *close;
  unsigned int wrap_internals;
} HTML_TAG;

// NOTE: These must be in the same order as the AST_NODE_TYPES enum in ast.h
static const HTML_TAG HTML_TAGS[] = {
    {.open = "", .close = "", .wrap_internals = 1},  // DOCUMENT
    {.open = "", .close = "", .wrap_internals = 0},  // TEXT
    {.open = "<pre><code>",
     .close = "</code></pre>",
     .wrap_internals = 0},  // CODE_BLOCK
    {.open = "<blockquote>",
     .close = "</blockquote>",
     .wrap_internals = 1},                                    // BLOCK_QUOTE
    {.open = "<ul>", .close = "</ul>", .wrap_internals = 1},  // UNORDERED_LIST
    {.open = "<li>",
     .close = "</li>",
     .wrap_internals = 1},  // UNORDERED_LIST_ITEM
    {.open = "<ol>", .close = "</ol>", .wrap_internals = 1},  // ORDERED_LIST
    {.open = "<li>",
     .close = "</li>",
     .wrap_internals = 1},                                  // ORDERED_LIST_ITEM
    {.open = "<p>", .close = "</p>", .wrap_internals = 1},  // PARAGRAPH
    {.open = "<h1>", .close = "</h1>", .wrap_internals = 1},  // H1,
    {.open = "<h2>", .close = "</h2>", .wrap_internals = 1},  // H2,
    {.open = "<h3>", .close = "</h3>", .wrap_internals = 1},  // H3,
    {.open = "<h4>", .close = "</h4>", .wrap_internals = 1},  // H4,
    {.open = "<h5>", .close = "</h5>", .wrap_internals = 1},  // H5,
    {.open = "<h6>", .close = "</h6>", .wrap_internals = 1},  // H6,
    {.open = "<hr>", .close = "", .wrap_internals = 0},       // THEMATIC_BREAK
    {.open = "<h1>", .close = "</h1>", .wrap_internals = 1},  // SETEXT_H1,
    {.open = "<h2>", .close = "</h2>", .wrap_internals = 1},  // SETEXT_H2,
};

#endif  // AST_TO_HTML_H
