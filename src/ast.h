#ifndef AST_H
#define AST_H

#include <stdlib.h>

enum AST_NODE_TYPES {
  ASTN_DOCUMENT = 0,
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
};

static const char *NODE_TYPE_NAMES[] = {
    "DOC", "CODE_BLOCK", "BLOCKQUOTE", "UL", "LI", "P",        "H1",
    "H2",  "H3",         "H4",         "H5", "H6", "THM_BREAK"};

static const unsigned int LEAF_ONLY_NODES_SIZE = 9;
static const unsigned int LEAF_ONLY_NODES[LEAF_ONLY_NODES_SIZE] = {
    ASTN_CODE_BLOCK, ASTN_H1, ASTN_H2, ASTN_H3,
    ASTN_H4,         ASTN_H5, ASTN_H6, ASTN_THEMATIC_BREAK,
    ASTN_PARAGRAPH};

static const unsigned int SINGLE_LINE_NODES_SIZE = 7;
static const unsigned int SINGLE_LINE_NODES[SINGLE_LINE_NODES_SIZE] = {
    ASTN_H1, ASTN_H2, ASTN_H3, ASTN_H4, ASTN_H5, ASTN_H6, ASTN_THEMATIC_BREAK,
};

static const unsigned int REQ_NL_AFTER_PARAGRAPH_NODES_SIZE = 1;
static const unsigned int
    REQ_NL_AFTER_PARAGRAPH_NODES[REQ_NL_AFTER_PARAGRAPH_NODES_SIZE] = {
        ASTN_CODE_BLOCK};

typedef struct {
  char marker;
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
