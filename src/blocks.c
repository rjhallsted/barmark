#include "blocks.h"

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "util.h"

int matches_continuation_markers(ASTNode *node, const char *line) {
  printf("matching markers\n");
  if (!(node->cont_markers)) {
    return 1;
  }
  size_t i = 0;
  while (line[i] && node->cont_markers[i] && line[i] == node->cont_markers[i]) {
    i++;
  }
  if (node->cont_markers[i] == '\0') {
    return 1;
  }
  return 0;
}

void add_line_to_node(ASTNode *node, const char *line) {
  printf("adding line to node\n");
  if (node->contents == NULL) {
    node->contents = strdup("");
  }
  str_append(node->contents, line);
}

int matches_opening_tab(const char *line) {
  size_t i = 0;
  while (line[i] == ' ' && i < 4) {
    i++;
  }
  if (i == 4 || line[i] == '\t') {
    return 1;
  }
  return 0;
}

/***
 * Returns 0 if no block start is found.
 */
int block_start_type(const char *line) {
  if (matches_opening_tab(line)) {
    return ASTN_CODE_BLOCK;
  }
  return 0;
}

// traverse to deepest/lastest open block, building up continuation markers
// along the way consume continuation as you go, stop when no longer matching
// look for new block starts
// if found, close remaining unmatched blocks
// begin new block as child of last matched block
// incorporate remainder of line in last open block
void add_line_to_ast(ASTNode *root, const char *line) {
  printf("adding line\n");
  size_t line_pos = 0;
  ASTNode *node = root;
  unsigned int node_type;
  ASTNode *new_node;

  while (node->children_count > 0 && line[line_pos] &&
         matches_continuation_markers(node, line + line_pos)) {
    printf("in while loop\n");
    line_pos += strlen(node->cont_markers);
    node = node->children[node->children_count - 1];
  }
  line = line + line_pos;
  if ((node_type = block_start_type(line))) {
    printf("new node\n");
    new_node = ast_create_node(node_type);
    ast_add_child(node, new_node);
    node = new_node;
  }
  add_line_to_node(node, line);
}

ASTNode *build_block_structure(FILE *fd) {
  ASTNode *document = ast_create_node(ASTN_DOCUMENT);
  char *line = NULL;
  size_t buff_size = 0;
  while (getline(&line, &buff_size, fd)) {
    printf("beginning of get line loop\n");
    printf("line: %s", line);
    add_line_to_ast(document, line);
    printf("end of get line loop\n");
  }
  printf("built structure\n");
  return document;
}
