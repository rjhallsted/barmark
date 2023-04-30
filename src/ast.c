#include "ast.h"

#include <stdlib.h>
#include <string.h>

#include "symbols.h"

/* Note that this allows, by design, passing NULL as the value of contents */
ASTNode *ast_create_node(unsigned int type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = type;
  node->children = NULL;
  node->children_count = 0;

  return node;
}

// TODO: Test
/* Note that this expects tokens_count > 0 */
char *join_token_contents(Token **tokens, size_t tokens_count) {
  char *output;
  size_t len = 0, o_offset = 0, t_offset = 0;
  unsigned int i;

  for (i = 0; i < tokens_count; i++) {
    len += strlen(tokens[i]->contents);
  }
  output = malloc(o_offset + 1);
  while (o_offset < len) {
    if (tokens[i]->contents[t_offset] == '\0') {
      i += 1;
      t_offset = 0;
    }
    output[o_offset] = tokens[i]->contents[t_offset];
    o_offset += 1;
    t_offset += 1;
  }
  output[o_offset] = '\0';
  return output;
}

void ast_free_node(ASTNode *node) {
  for (unsigned int i = 0; i < node->children_count; i++) {
    ast_free_node(node->children[i]);
  }
  free(node);
}

void ast_add_child(ASTNode *parent, ASTNode *child) {
  parent->children =
      realloc(parent->children, sizeof(ASTNode) * (parent->children_count + 1));
  parent->children[parent->children_count] = child;
  parent->children_count += 1;
}

// ASTNode *ast_from_tokens(Token **tokens) {
//   Token *token = tokens[0];
//   sizeof(token);
//   ASTNode *root = ast_create_node(ASTN_DOCUMENT);
//   return root;

//   /*
//   GENERAL STRATEGY:

//   try each rule in turn, apply transformation, repeat through loop. If rule
//   is matching, apply relevant consumption process of tokens. advance

//   need two pointers. AST position and symbol position. AST position ascends
//   and descends. Symbol position strictly advances. Since we're doing
//   recursive descent, need a way of knowing if should continue to recurse or
//   not. How this works will necessarily be rule-specific (or node-type
//   specific. Likely means a different parse function per node type. Will need
//   a function for determing rule to apply. For most cases, I think the logic
//   to recurse or not is basically:
//   - Is the condition to continue parsing as the contents of this node met?
//     - If yes, and this node can have children, is the next number of indents
//   correct?
//       - if so, recurse
//       - if not, treat as contents of this node
//     - If not, recurse

//   The code marking a node type as allowing children or not will be part of
//   that nodes parsing function.
//   */

//   // if is tab, make code block. Contents is everything until there is a line
//   // that does not begin with a tab
// }
