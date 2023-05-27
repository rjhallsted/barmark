#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse_rule.h"
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

/***
 * Recursively descends the tree and flattens the entire thing
 * so that all descendent children are now direct children of
 * the provided node.
 */
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
ASTNode *ast_get_next_node(Token ***stream_ptr) {
  ASTNode *node;

  if ((node = m_code_block(stream_ptr))) {
    return node;
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

  while (*stream_ptr && (node = ast_get_next_node(&stream_ptr))) {
    ast_add_child(root, node);
  }
  // root = ast_condense_tree(root);
  return root;
}
