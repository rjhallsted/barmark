#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "symbols.h"

/* Note that this allows, by design, passing NULL as the value of contents */
ASTNode *ast_create_node(unsigned int type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = type;
  node->contents = NULL;
  node->children = NULL;
  node->children_count = 0;

  return node;
}

/* Note that this expects tokens_count > 0 */
char *join_token_contents(Token *token, size_t tokens_count) {
  char *output;
  size_t len = 0, o_offset = 0, t_offset = 0;
  Token *ptr = token;

  for (size_t i = 0; i < tokens_count; i++) {
    len += strlen(ptr->contents);
    ptr = ptr->next;
  }
  output = malloc(o_offset + 1);
  ptr = token;
  while (o_offset < len) {
    if (ptr->contents[t_offset] == '\0') {
      ptr = ptr->next;
      t_offset = 0;
    }
    output[o_offset] = ptr->contents[t_offset];
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
  if (node->contents) {
    free(node->contents);
  }
  free(node);
}

void ast_add_child(ASTNode *parent, ASTNode *child) {
  parent->children =
      realloc(parent->children, sizeof(ASTNode) * (parent->children_count + 1));
  parent->children[parent->children_count] = child;
  parent->children_count += 1;
}

/*
- determine type of next node
- consume for next node (this will entail handling possible children) (returns
a new node, and advances stream ptr)
- return this node
*/
ASTNode *ast_get_next_node(Token **stream_ptr) {
  const SymbolSeq code_block1 = {CODE_BLOCK_SEQ1, CODE_BLOCK_SEQ1_SIZE};
  const SymbolSeq code_block2 = {CODE_BLOCK_SEQ2, CODE_BLOCK_SEQ2_SIZE};

  size_t tokens_read;
  ConsumerPtr consumer;

  if ((tokens_read = matches_symbol_seq(*stream_ptr, code_block1)) ||
      (tokens_read = matches_symbol_seq(*stream_ptr, code_block2))) {
    consume_x_tokens(stream_ptr, tokens_read);
    consumer = AST_CONSUMERS[ASTN_CODE_BLOCK];
    return (*consumer)(stream_ptr, tokens_read);
  } else {
    printf(
        "INVALID token sequence found. Could not determine AST node type.\n");
    exit(EXIT_FAILURE);
  }
}

ASTNode *ast_from_tokens(Token *stream) {
  ASTNode *root = ast_create_node(ASTN_DOCUMENT);
  ASTNode *node;
  Token **stream_ptr = &stream;

  while ((*stream_ptr)) {
    // skip new lines
    while ((*stream_ptr) && (*stream_ptr)->symbol->id == SYMBOL_NL_ID) {
      *stream_ptr = (*stream_ptr)->next;
    }
    if (*stream_ptr) {
      node = ast_get_next_node(stream_ptr);
      ast_add_child(root, node);
    }
  }

  return root;
  /*
  GENERAL STRATEGY:

  try each rule in turn, apply transformation, repeat through loop. If rule
  is matching, apply relevant consumption process of tokens. advance

  need two pointers. AST position and symbol position. AST position ascends
  and descends. Symbol position strictly advances. Since we're doing
  recursive descent, need a way of knowing if should continue to recurse or
  not. How this works will necessarily be rule-specific (or node-type
  specific. Likely means a different parse function per node type. Will need
  a function for determing rule to apply. For most cases, I think the logic
  to recurse or not is basically:
  - Is the condition to continue parsing as the contents of this node met?
    - If yes, and this node can have children, is the next number of indents
  correct?
      - if so, recurse
      - if not, treat as contents of this node
    - If not, recurse

  The code marking a node type as allowing children or not will be part of
  that nodes parsing function.
  */

  // if is tab, make code block. Contents is everything until there is a line
  // that does not begin with a tab
}
