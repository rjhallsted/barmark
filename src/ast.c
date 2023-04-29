#include "ast.h"

#include <stdlib.h>
#include <string.h>

#include "symbols.h"

ASTNode *ast_create_node(unsigned int type, const char *contents) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->contents = strdup(contents);
  node->children = NULL;
  node->children_count = 0;

  return node;
}

// TODO: Test
char *join_symbol_contents(Symbol **symbols, size_t symbols_count) {
  char *output;
  size_t len = 0, o_offset = 0, s_offset = 0;
  unsigned int i;

  for (i = 0; i < symbols_count; i++) {
    len += strlen(symbols[i]->contents);
  }
  output = malloc(o_offset + 1);
  while (o_offset < len) {
    if (symbols[i]->contents[s_offset] == '\0') {
      i += 1;
      s_offset = 0;
    }
    output[o_offset] = symbols[i]->contents[s_offset];
    o_offset += 1;
    s_offset += 1;
  }
  output[o_offset] = '\0';
  return output;
}

ASTNode *ast_create_node_from_sybmols(unsigned int type, Symbol **symbols,
                                      size_t symbols_count) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->contents = join_symbol_contents(symbols, symbols_count);
  node->children = NULL;
  node->children_count = 0;

  return node;
}

void ast_free_node(ASTNode *node) {
  for (unsigned int i = 0; i < node->children_count; i++) {
    ast_free_node(node->children[i]);
  }
  free(node->contents);
  free(node);
}

void ast_add_child(ASTNode *parent, ASTNode *child) {
  parent->children =
      realloc(parent->children, sizeof(ASTNode) * (parent->children_count + 1));
  parent->children[parent->children_count] = child;
  parent->children_count += 1;
}

ASTNode *ast_create_document(void) {
  return ast_create_node(ASTN_DOCUMENT, NULL);
}