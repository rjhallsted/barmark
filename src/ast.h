#ifndef AST_H
#define AST_H

#include <stdlib.h>

#include "symbols.h"

enum AST_NODE_TYPES {
  ASTN_DOCUMENT = 0,
  ASTN_DEFAULT,
  ASTN_TEXT,
  ASTN_CODE_BLOCK,
  ASTN_PARAGRAPH,
  ASTN_CODE,
};

/* NOTE: ASTNodes should only ever have contents or children, never both.
 * Output functions will preference contents over children.
 */
typedef struct ASTNode {
  unsigned int type;
  char *contents;
  struct ASTNode **children;
  size_t children_count;
} ASTNode;

ASTNode *ast_create_node(unsigned int type);
char *join_token_contents(Token *token, size_t tokens_count);
void ast_free_node_only(ASTNode *node);
void ast_free_node(ASTNode *node);
void ast_add_child(ASTNode *parent, ASTNode *child);
void ast_move_children_to_contents(ASTNode *node);
void ast_remove_child_at_index(ASTNode *node, size_t index);
void ast_flatten_children(ASTNode *node);

ASTNode *ast_get_next_node(Token **stream_ptr);
ASTNode *ast_condense_tree(ASTNode *root);
ASTNode *ast_from_tokens(Token *tokens);

#endif  // AST_H
