#ifndef AST_H
#define AST_H

#include <stdlib.h>

#include "symbols.h"

enum AST_NODE_TYPES {
  ASTN_DOCUMENT = 1,
  ASTN_TEXT,
  ASTN_CODE_BLOCK,
  ASTN_PARAGRAPH,
  ASTN_CODE,
};

typedef struct ASTNode {
  unsigned int type;
  struct ASTNode **children;
  size_t children_count;
} ASTNode;

ASTNode *ast_create_node(unsigned int type);
void ast_free_node(ASTNode *node);
void ast_add_child(ASTNode *parent, ASTNode *child);

// ASTNode *ast_from_sybmols(Symbol **symbols);

#endif  // AST_H
