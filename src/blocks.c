#include "blocks.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "util.h"

int matches_continuation_markers(ASTNode *node, const char *line) {
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
  if (i < 4 && line[i] == '\t') {
    i++;
    return i;
  }
  if (i == 4) {
    return i;
  }
  return 0;
}

/***
 * Returns 0 if no block start is found.
 */
int block_start_type(const char *line, size_t *match_len) {
  if ((*match_len = matches_opening_tab(line))) {
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
  size_t line_pos = 0;
  size_t match_len = 0;
  ASTNode *node = root;
  unsigned int node_type;
  ASTNode *new_node;

  while (node->children_count > 0 && line[line_pos] &&
         matches_continuation_markers(node, line + line_pos)) {
    line_pos += strlen(node->cont_markers);
    node = node->children[node->children_count - 1];
  }
  if ((node_type = block_start_type(line + line_pos, &match_len))) {
    line_pos += match_len;
    new_node = ast_create_node(node_type);
    new_node->cont_markers = strndup(line, line_pos);
    ast_add_child(node, new_node);
    node = new_node;
    add_line_to_node(node, line + line_pos);
  } else {
    while (node->children_count > 0) {
      node = node->children[node->children_count - 1];
    }
    add_line_to_node(node, line + line_pos);
  }
}

/*
TODO: In order to get spec_test 1 to pass:
- paragraph is the only node type with contents. everything else spits out
children instead
- if adding a new block that is not a paragraph, add paragraph as child to start
- if last node is paragraph and line is new line only, close paragraph and open
a new one.
-
*/

ASTNode *build_block_structure(FILE *fd) {
  ASTNode *document = ast_create_node(ASTN_DOCUMENT);
  char *line = NULL;
  size_t buff_len = 0;
  while (!feof(fd)) {
    if (ferror(fd)) {
      printf("File reading error. Errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }

    getline(&line, &buff_len, fd);
    add_line_to_ast(document, line);
  }
  return document;
}
