#ifndef AST_H
#define AST_H

#include <stdlib.h>

enum AST_NODE_TYPES {
  ASTN_DOCUMENT = 0,
  ASTN_TEXT,
  ASTN_CODE_BLOCK,
  ASTN_BLOCK_QUOTE,
  ASTN_UNORDERED_LIST,
  ASTN_UNORDERED_LIST_ITEM,
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

static const unsigned int NODE_TYPE_COUNT = 16;

static const char *NODE_TYPE_NAMES[] = {
    "DOC", "TEXT",      "CODE_BLOCK", "BLOCKQUOTE", "UL", "LI",
    "P",   "H1",        "H2",         "H3",         "H4", "H5",
    "H6",  "THM_BREAK", "SETEXT_H1",  "SETEXT_H2"};

static const unsigned int LEAF_ONLY_NODES_SIZE = 11;
static const unsigned int LEAF_ONLY_NODES[LEAF_ONLY_NODES_SIZE] = {
    ASTN_CODE_BLOCK, ASTN_H1,        ASTN_H2,       ASTN_H3,
    ASTN_H4,         ASTN_H5,        ASTN_H6,       ASTN_THEMATIC_BREAK,
    ASTN_PARAGRAPH,  ASTN_SETEXT_H1, ASTN_SETEXT_H2};

// NOTE: These are unappendable "to"
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

static const unsigned int NOT_INTERRUPTIBLE_BY_CODE_BLOCK_SIZE = 2;
static const unsigned int
    NOT_INTERRUPTIBLE_BY_CODE_BLOCK[NOT_INTERRUPTIBLE_BY_CODE_BLOCK_SIZE] = {
        ASTN_PARAGRAPH, ASTN_UNORDERED_LIST};

typedef struct {
  char marker;
  unsigned int wide;
} ASTListOptions;

typedef struct ASTNode {
  unsigned int type;
  unsigned int open;
  char *contents;
  char *cont_markers;
  struct ASTNode **children;
  size_t children_count;
  struct ASTNode *parent;
  // TODO: Replace with union of different options types once there's more than
  // one
  ASTListOptions *options;
} ASTNode;

ASTNode *ast_create_node(unsigned int type);
void ast_free_node_only(ASTNode *node);
void ast_free_node(ASTNode *node);
void ast_add_child(ASTNode *parent, ASTNode *child);
void ast_move_children_to_contents(ASTNode *node);
void ast_remove_child_at_index(ASTNode *node, size_t index);
void ast_flatten_children(ASTNode *node);

#endif  // AST_H
