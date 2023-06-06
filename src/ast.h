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
};

static const unsigned int NODE_ALLOWS_CONTENTS[] = {
    0,  // DOCUMENT
    1,  // CODE_BLOCK
    0,  // BLOCK_QUOTE
    0,  // UNORDERED_LIST
    0,  // UNORDERED_LIST_ITEM
    1,  // PARAGRAPH
};

/* TODO: Figure out how to add "options"
Things like:
- list type
- beginning val for ordered lists
*/

typedef struct ASTNode {
  unsigned int type;
  unsigned int open;
  char *contents;
  char *cont_markers;
  struct ASTNode **children;
  size_t children_count;
} ASTNode;

ASTNode *ast_create_node(unsigned int type);
void ast_free_node_only(ASTNode *node);
void ast_free_node(ASTNode *node);
void ast_add_child(ASTNode *parent, ASTNode *child);
void ast_move_children_to_contents(ASTNode *node);
void ast_remove_child_at_index(ASTNode *node, size_t index);
void ast_flatten_children(ASTNode *node);

#endif  // AST_H
