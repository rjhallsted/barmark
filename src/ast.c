#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "symbols.h"
#include "util.h"

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

void ast_free_node_only(ASTNode *node) {
  if (node->contents) {
    free(node->contents);
  }
  free(node);
}

void ast_free_node(ASTNode *node) {
  for (unsigned int i = 0; i < node->children_count; i++) {
    ast_free_node(node->children[i]);
  }
  ast_free_node_only(node);
}

void ast_add_child(ASTNode *parent, ASTNode *child) {
  parent->children =
      realloc(parent->children, sizeof(ASTNode) * (parent->children_count + 1));
  parent->children[parent->children_count] = child;
  parent->children_count += 1;
}

/**
 * @brief Recursively moves the contents of children into the contents of this
 * node and deletes the children. Replaces existing contents if there is any
 *
 * @param node
 */
// TODO: Test
void ast_move_children_to_contents(ASTNode *node) {
  if (node->children_count == 0) {
    return;
  }
  if (node->contents) {
    free(node->contents);
  }
  char *new_contents = strdup("");
  for (size_t i = 0; i < node->children_count; i++) {
    ast_move_children_to_contents(node->children[i]);
    if (node->children[i]->contents) {
      new_contents = str_append(new_contents, node->children[i]->contents);
    }
    ast_free_node(node->children[i]);
  }
  free(node->children);
  node->children = NULL;
  node->children_count = 0;
  node->contents = new_contents;
}

// TODO: test
void ast_flatten_children(ASTNode *node) {
  ASTNode **old_children = node->children;
  size_t old_children_size = node->children_count;

  node->children = NULL;
  node->children_count = 0;
  for (size_t i = 0; i < old_children_size; i++) {
    ast_flatten_children(old_children[i]);
    if (old_children[i]->children_count > 0) {
      for (size_t j = 0; j < old_children[i]->children_count; j++) {
        ast_add_child(node, old_children[i]->children[j]);
      }
      ast_free_node_only(old_children[i]);
    } else {
      ast_add_child(node, old_children[i]);
    }
  }
  free(old_children);
}

/***
 * Removes the child at index provided.
 * fails if the index is out of bounds
 */
// TODO: Test
void ast_remove_child_at_index(ASTNode *node, size_t index) {
  if (node->children_count <= index) {
    printf("Bad index provided to ast_remove_first_child.\n");
    exit(EXIT_FAILURE);
  }
  ASTNode *child = node->children[index];
  ASTNode **new_children =
      malloc(sizeof(ASTNode *) * (node->children_count - 1));
  for (size_t i = 0; i < index; i++) {
    new_children[i] = node->children[i];
  }
  for (size_t i = index + 1; i < node->children_count; i++) {
    new_children[i - 1] = node->children[i];
  }
  ASTNode **old_children = node->children;
  node->children = new_children;
  node->children_count -= 1;
  ast_free_node(child);
  free(old_children);
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
    consumer = AST_CONSUMERS[ASTN_CODE_BLOCK];
    return (*consumer)(stream_ptr, tokens_read);
  } else {
    printf(
        "INVALID token sequence found. Could not determine AST node type.\n");
    exit(EXIT_FAILURE);
  }
}

ASTNode *ast_merge_nodes(ASTNode *a, ASTNode *b) {
  if (a->type != b->type) {
    printf("Tried to merge AST Nodes of differing types\n");
    exit(EXIT_FAILURE);
  }
  ASTNode *new_node = ast_create_node(a->type);
  size_t new_contents_size = strlen(a->contents) + strlen(b->contents) + 1;
  new_node->contents = malloc(new_contents_size);
  strlcat(new_node->contents, a->contents, new_contents_size);
  strlcat(new_node->contents, b->contents, new_contents_size);
  for (size_t i = 0; i < a->children_count; i++) {
    ast_add_child(new_node, a->children[i]);
  }
  for (size_t i = 0; i < b->children_count; i++) {
    ast_add_child(new_node, b->children[i]);
  }
  return new_node;
}

/*
* Condenses the provided ast. This mainly involves things like
* joining combinable node types, (for instance, consecutive code
blocks can be merged)
*
* Currently only does top-level condensation
*/
ASTNode *ast_condense_tree(ASTNode *root) {
  ASTNode *new_root = ast_create_node(ASTN_DOCUMENT);
  ASTNode *a, *b;

  for (size_t i = 0; i < root->children_count; i++) {
    // join code blocks
    if (root->children[i]->type == ASTN_CODE_BLOCK &&
        new_root->children_count > 0 &&
        new_root->children[new_root->children_count - 1]->type ==
            ASTN_CODE_BLOCK) {
      a = new_root->children[new_root->children_count - 1];
      b = root->children[i];
      new_root->children[new_root->children_count - 1] = ast_merge_nodes(a, b);
      ast_free_node_only(a);
      ast_free_node_only(b);
    } else {
      ast_add_child(new_root, root->children[i]);
    }
  }
  ast_free_node_only(root);
  return new_root;
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

  root = ast_condense_tree(root);

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
