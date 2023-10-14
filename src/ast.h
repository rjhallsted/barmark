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
  ASTN_SETEXT_H2
};

static const unsigned int NODE_TYPE_COUNT = 19;

static const char *NODE_TYPE_NAMES[NODE_TYPE_COUNT] = {
    "DOC",        "TEXT",      "CODE_BLOCK", "FENCED_CODE_BLOCK",
    "BLOCKQUOTE", "UL",        "UL-LI",      "OL",
    "OL-LI",      "P",         "H1",         "H2",
    "H3",         "H4",        "H5",         "H6",
    "THM_BREAK",  "SETEXT_H1", "SETEXT_H2"};

static const unsigned int LEAF_ONLY_NODES_SIZE = 12;
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
    ASTN_SETEXT_H2};

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

typedef struct {
  char id_char;
  bool wide;
  /*
  reference_num is used in the following ways:
  - Ordered lists use it as the starting num
  - Fenced code blocks use it to indicate the fence length
  */
  long unsigned reference_num;
} ASTNodeOptions;

typedef struct ASTNode {
  int unsigned type;
  bool open;
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
ASTNodeOptions *make_node_options(char id_char, long unsigned reference_num);

#endif  // AST_H
