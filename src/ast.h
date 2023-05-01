#ifndef AST_H
#define AST_H

#include <stdlib.h>

#include "symbols.h"

enum AST_NODE_TYPES {
  ASTN_DOCUMENT = 0,
  ASTN_TEXT,
  ASTN_CODE_BLOCK,
  ASTN_PARAGRAPH,
  ASTN_CODE,
};

/* Note while using ASTNode: Contents may be null. */
typedef struct ASTNode {
  unsigned int type;
  char *contents;
  struct ASTNode **children;
  size_t children_count;
} ASTNode;

ASTNode *ast_create_node(unsigned int type);
char *join_token_contents(Token *token, size_t tokens_count);
void ast_free_node(ASTNode *node);
void ast_add_child(ASTNode *parent, ASTNode *child);

ASTNode *ast_from_tokens(Token *tokens);

int ast_nodes_equal(ASTNode *a, ASTNode *b);

#endif  // AST_H
