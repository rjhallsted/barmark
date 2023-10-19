#ifndef AST_TO_HTML_H
#define AST_TO_HTML_H

#include <stdbool.h>
#include <stdio.h>

#include "ast.h"

void ast_to_html(ASTNode ast[static 1], FILE output_fd[static 1],
                 int unsigned print_tags);

typedef struct {
  const char *label;
  bool show_tag;
  bool close;
  const char *closing_label;
  bool wrap_internals;
} HTML_TAG;

// NOTE: These must be in the same order as the AST_NODE_TYPES enum in ast.h
static const HTML_TAG HTML_TAGS[NODE_TYPE_COUNT] = {
    {.label = "",
     .show_tag = false,
     .close = false,
     .wrap_internals = true},  // DOCUMENT
    {.label = "",
     .show_tag = false,
     .close = false,
     .wrap_internals = false},  // TEXT
    {.label = "pre><code",
     .show_tag = true,
     .close = true,
     .closing_label = "code></pre",
     .wrap_internals = false},  // CODE_BLOCK
    {.label = "pre><code",
     .show_tag = true,
     .close = true,
     .closing_label = "code></pre",
     .wrap_internals = false},  // CODE_BLOCK
    {.label = "blockquote",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // BLOCK_QUOTE
    {.label = "ul",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // UNORDERED_LIST
    {.label = "li",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // UNORDERED_LIST_ITEM
    {.label = "ol",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // ORDERED_LIST
    {.label = "li",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // ORDERED_LIST_ITEM
    {.label = "p",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // PARAGRAPH
    {.label = "h1",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // H1,
    {.label = "h2",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // H2,
    {.label = "h3",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // H3,
    {.label = "h4",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // H4,
    {.label = "h5",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // H5,
    {.label = "h6",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // H6,
    {.label = "hr",
     .show_tag = true,
     .close = false,
     .wrap_internals = false},  // THEMATIC_BREAK
    {.label = "h1",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // SETEXT_H1,
    {.label = "h2",
     .show_tag = true,
     .close = true,
     .wrap_internals = true},  // SETEXT_H2,
    {.label = "",
     .show_tag = false,
     .close = false,
     .wrap_internals = false},  // HTML_BLOCK_TYPE_1
    {.label = "",
     .show_tag = false,
     .close = false,
     .wrap_internals = false},  // HTML_BLOCK_TYPE_2
    {.label = "",
     .show_tag = false,
     .close = false,
     .wrap_internals = false}  // HTML_BLOCK_TYPE_3
};

#endif  // AST_TO_HTML_H
