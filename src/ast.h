#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stdlib.h>

enum AST_NODE_TYPES {
  ASTN_DOCUMENT = 0,
  ASTN_TEXT,
  ASTN_CODE_BLOCK,
  ASTN_FENCED_CODE_BLOCK,
  ASTN_BLOCK_QUOTE,
  ASTN_UNORDERED_LIST,
  ASTN_UNORDERED_LIST_ITEM,
  ASTN_ORDERED_LIST,
  ASTN_ORDERED_LIST_ITEM,
  ASTN_PARAGRAPH,
  ASTN_H1,
  ASTN_H2,
  ASTN_H3,
  ASTN_H4,
  ASTN_H5,
  ASTN_H6,
  ASTN_THEMATIC_BREAK,
  ASTN_SETEXT_H1,
  ASTN_SETEXT_H2,
  ASTN_HTML_BLOCK_TYPE_1,
  ASTN_HTML_BLOCK_TYPE_2,
  ASTN_HTML_BLOCK_TYPE_3,
  ASTN_HTML_BLOCK_TYPE_4,
  ASTN_HTML_BLOCK_TYPE_5,
  ASTN_HTML_BLOCK_TYPE_6,
};

static const unsigned int NODE_TYPE_COUNT = 25;

static const char *NODE_TYPE_NAMES[NODE_TYPE_COUNT] = {
    "DOC",
    "TEXT",
    "CODE_BLOCK",
    "FENCED_CODE_BLOCK",
    "BLOCKQUOTE",
    "UL",
    "UL-LI",
    "OL",
    "OL-LI",
    "P",
    "H1",
    "H2",
    "H3",
    "H4",
    "H5",
    "H6",
    "THM_BREAK",
    "SETEXT_H1",
    "SETEXT_H2",
    "HTML_BLOCK_TYPE_1",
    "HTML_BLOCK_TYPE_2",
    "HTML_BLOCK_TYPE_3",
    "HTML_BLOCK_TYPE_4",
    "HTML_BLOCK_TYPE_5",
    "HTML_BLOCK_TYPE_6",
};

static const unsigned int LEAF_ONLY_NODES_SIZE = 18;
static const unsigned int LEAF_ONLY_NODES[LEAF_ONLY_NODES_SIZE] = {
    ASTN_CODE_BLOCK,
    ASTN_FENCED_CODE_BLOCK,
    ASTN_H1,
    ASTN_H2,
    ASTN_H3,
    ASTN_H4,
    ASTN_H5,
    ASTN_H6,
    ASTN_THEMATIC_BREAK,
    ASTN_PARAGRAPH,
    ASTN_SETEXT_H1,
    ASTN_SETEXT_H2,
    ASTN_HTML_BLOCK_TYPE_1,
    ASTN_HTML_BLOCK_TYPE_2,
    ASTN_HTML_BLOCK_TYPE_3,
    ASTN_HTML_BLOCK_TYPE_4,
    ASTN_HTML_BLOCK_TYPE_5,
    ASTN_HTML_BLOCK_TYPE_6,
};

// NOTE: You cannot "append to" these, rather than being unable to append
// them to other nodes
static const unsigned int UNNAPENDABLE_NODES_SIZE = 9;
static const unsigned int UNAPPENDABLE_NODES[UNNAPENDABLE_NODES_SIZE] = {
    ASTN_H1,
    ASTN_H2,
    ASTN_H3,
    ASTN_H4,
    ASTN_H5,
    ASTN_H6,
    ASTN_THEMATIC_BREAK,
    ASTN_SETEXT_H1,
    ASTN_SETEXT_H2};

static const unsigned int ATX_HEADINGS_SIZE = 6;
static const unsigned int ATX_HEADINGS[ATX_HEADINGS_SIZE] = {
    ASTN_H1, ASTN_H2, ASTN_H3, ASTN_H4, ASTN_H5, ASTN_H6};

static const unsigned int NOT_INTERRUPTIBLE_BY_CODE_BLOCK_SIZE = 3;
static const unsigned int
    NOT_INTERRUPTIBLE_BY_CODE_BLOCK[NOT_INTERRUPTIBLE_BY_CODE_BLOCK_SIZE] = {
        ASTN_PARAGRAPH, ASTN_UNORDERED_LIST, ASTN_ORDERED_LIST};

static const unsigned int SHOULD_CONSUME_EMPTY_LINES_SIZE = 6;
static const int unsigned
    SHOULD_CONSUME_EMPTY_LINES[SHOULD_CONSUME_EMPTY_LINES_SIZE] = {
        ASTN_FENCED_CODE_BLOCK, ASTN_HTML_BLOCK_TYPE_1, ASTN_HTML_BLOCK_TYPE_2,
        ASTN_HTML_BLOCK_TYPE_3, ASTN_HTML_BLOCK_TYPE_4, ASTN_HTML_BLOCK_TYPE_5};

////////////////////////
// HTML BLOCKS
////////////////////////

// TYPE 1
static const unsigned int HTML_BLOCK_1_OPENERS_SIZE = 4;
static char const *HTML_BLOCK_1_OPENERS[HTML_BLOCK_1_OPENERS_SIZE] = {
    "<pre", "<script", "<style", "<textarea"};
static const unsigned int HTML_BLOCK_1_CLOSERS_SIZE = 4;
static char const *HTML_BLOCK_1_CLOSERS[HTML_BLOCK_1_CLOSERS_SIZE] = {
    "</pre>", "</script>", "</style>", "</textarea>"};

// TYPE 6
static const int unsigned HTML_BLOCK_6_TAGS_SIZE = 62;
static char const *HTML_BLOCK_6_TAGS[HTML_BLOCK_6_TAGS_SIZE] = {
    "address",  "article",    "aside",  "base",     "basefont", "blockquote",
    "body",     "caption",    "center", "col",      "colgroup", "dd",
    "details",  "dialog",     "dir",    "div",      "dl",       "dt",
    "fieldset", "figcaption", "figure", "footer",   "form",     "frame",
    "frameset", "h1",         "h2",     "h3",       "h4",       "h5",
    "h6",       "head",       "header", "hr",       "html",     "iframe",
    "legend",   "li",         "link",   "main",     "menu",     "menuitem",
    "nav",      "noframes",   "ol",     "optgroup", "option",   "p",
    "param",    "section",    "source", "summary",  "table",    "tbody",
    "td",       "tfoot",      "th",     "thead",    "title",    "tr",
    "track",    "ul",
};

////////////////////////
// END HTML BLOCKS
////////////////////////

typedef struct {
  /* Id char is used by:
  - Unordered lists for the bullet type
  - Ordered lists for the delimiter
  - Fenced code blocks for the fence type
  */
  char id_char;
  bool wide;
  /*
  reference_num is used in the following ways:
  - Ordered lists use it as the starting num
  - Fenced code blocks use it to indicate the fence length
  */
  long unsigned reference_num;
  /*
  Indentation is only used by fenced code blocks
  */
  int unsigned indentation;
} ASTNodeOptions;

typedef struct ASTNode {
  int unsigned type;
  bool open;
  /* Contents is only used by the following node types:
  - Text: has actual output context
  - Fenced code blocks: Contains the info string if one exists
  */
  char *contents;
  int unsigned cont_spaces;
  struct ASTNode **children;
  size_t children_count;
  struct ASTNode *parent;
  // TODO: Replace with union of different options types once there's more than
  // one
  ASTNodeOptions *options;
  size_t late_continuation_lines;
} ASTNode;

ASTNode *ast_create_node(unsigned int type);
void ast_free_node_only(ASTNode node[static 1]);
void ast_free_node(ASTNode node[static 1]);
void ast_add_child(ASTNode parent[static 1], ASTNode child[static 1]);
void ast_move_children_to_contents(ASTNode node[static 1]);
void ast_remove_child_at_index(ASTNode node[static 1], size_t index);
void ast_flatten_children(ASTNode node[static 1]);
ASTNodeOptions *make_node_options(char id_char, long unsigned reference_num,
                                  int unsigned indentation);

#endif  // AST_H
