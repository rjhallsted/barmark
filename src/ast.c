#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

ASTNode *ast_create_node(unsigned int type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->next = NULL;
  node->type = type;
  node->open = true;
  node->contents = NULL;
  node->cont_spaces = 0;
  node->first_child = NULL;
  node->parent = NULL;
  node->options = NULL;
  node->late_continuation_lines = 0;

  return node;
}

void ast_free_node_only(ASTNode node[static 1]) {
  if (node->contents) {
    free(node->contents);
  }
  if (node->options) {
    free(node->options);
  }
  free(node);
}

void ast_free_node(ASTNode node[static 1]) {
  if (node->first_child) {
    ASTNode *ptr = node->first_child;
    ASTNode *tmp;
    while (ptr) {
      tmp = ptr->next;
      ast_free_node(ptr);
      ptr = tmp;
    }
  }
  ast_free_node_only(node);
}

void ast_add_child(ASTNode parent[static 1], ASTNode child[static 1]) {
  add_item_to_list((SinglyLinkedItem **)&(parent->first_child),
                   (SinglyLinkedItem *)child);
  child->parent = parent;
}

/**
 * @brief Recursively moves the contents of children into the contents of this
 * node and deletes the children. Replaces existing contents if there is any
 *
 * @param node
 */
// TODO: Test
void ast_move_children_to_contents(ASTNode node[static 1]) {
  if (!node->first_child) {
    return;
  }
  if (node->contents) {
    free(node->contents);
  }
  char *new_contents = strdup("");
  ASTNode *ptr = node->first_child;
  ASTNode *tmp;
  while (ptr) {
    ast_move_children_to_contents(ptr);
    if (ptr->contents) {
      new_contents = str_append(new_contents, ptr->contents);
    }
    tmp = ptr->next;
    ast_free_node(ptr);
    ptr = tmp;
  }
  node->first_child = NULL;
  node->contents = new_contents;
}

/***
 * Recursively descends the tree and flattens the entire thing
 * so that all descendent children are now direct children of
 * the provided node. Effectively keeps only leaf nodes.
 */
// TODO: test
void ast_flatten_children(ASTNode node[static 1]) {
  // ASTNode **old_children = node->children;
  // size_t old_children_size = node->children_count;

  ASTNode *dummy = ast_create_node(ASTN_DOCUMENT);
  dummy->next = node->first_child;
  ASTNode *ptr = dummy;
  ASTNode *tmp;

  while (ptr->next) {
    if (ptr->next->first_child) {
      ast_flatten_children(ptr->next->first_child);
      tmp = ptr->next;
      ptr->next = ptr->next->first_child;
      ptr = (ASTNode *)last_item_of_list((SinglyLinkedItem *)(ptr->next));
      ptr->next = tmp;
    }
  }
  node->first_child = dummy->next;
  ast_free_node(dummy);
}

/***
 * Removes the child at index provided.
 * fails if the index is out of bounds
 */
// TODO: Test
void ast_remove_child_at_index(ASTNode node[static 1], size_t index) {
  ASTNode *dummy = ast_create_node(ASTN_DOCUMENT);
  dummy->next = node->first_child;
  ASTNode *ptr = dummy;
  size_t i = 0;
  while (ptr->next && i < index) {
    ptr = ptr->next;
    i++;
  }
  if (i < index) {
    printf("Bad index provided to ast_remove_first_child.\n");
    exit(EXIT_FAILURE);
  }
  ASTNode *to_remove = ptr->next;
  ptr->next = ptr->next->next;
  if (ptr == dummy) {
    node->first_child = ptr->next;
  }
  ast_free_node(to_remove);
  ast_free_node(dummy);
}

void ast_add_child_node_with_contents(ASTNode node[static 1],
                                      int unsigned child_type, char *contents) {
  ASTNode *child = ast_create_node(child_type);
  child->contents = contents;
  ast_add_child(node, child);
}

ASTNodeOptions *make_node_options(char id_char, long unsigned reference_num,
                                  int unsigned indentation) {
  ASTNodeOptions *options = malloc(sizeof(ASTNodeOptions));
  options->id_char = id_char;
  options->wide = false;
  options->reference_num = reference_num;
  options->indentation = indentation;
  return options;
}
