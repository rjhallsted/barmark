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
  size_t ni = 0, li = 0;
  while (line[li] && node->cont_markers[ni]) {
    if (node->cont_markers[ni] == '\t') {
      unsigned int spaces = 0;
      while (line[li + spaces] && line[li + spaces] == ' ' && spaces < 4) {
        spaces++;
      }
      if (spaces == 4) {
        li += 4;
        ni++;
        continue;
      }
    }
    if (line[li] == node->cont_markers[ni]) {
      li++;
      ni++;
    } else {
      break;
    }
  }
  if (node->cont_markers[ni] == '\0') {
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
  ASTNode *tmp;

  /*
  * Theres 3(?) steps here:
    - traverse line and ast following continuation markers
    - From there, examine contents, determine if it continues block or not
    - If not, add new block(s) (depenending on context) and consume necessary
  tokens
    - Add remaining line to this block
  */

  // traverse to last matching node
  while (line[line_pos] && node->open && node->children_count > 0 &&
         matches_continuation_markers(node->children[node->children_count - 1],
                                      line + line_pos)) {
    node = node->children[node->children_count - 1];
    if (node->cont_markers) {
      line_pos += strlen(node->cont_markers);
    }
  }
  while ((node_type = block_start_type(line + line_pos, &match_len))) {
    // close remaining open nodes
    tmp = node;
    while (tmp->children_count > 0) {
      tmp = tmp->children[node->children_count - 1];
      tmp->open = 0;
    }
    if (tmp != node) {
      tmp->open = 0;
    }
    line_pos += match_len;
    node = ast_child_node_from_line_opening(node, node_type, line, line_pos);
  }
  if (line[0] == '\n' && node->type == ASTN_PARAGRAPH) {
    node->open = 0;
  } else if (line[0] != '\n' && node->type != ASTN_PARAGRAPH) {
    node =
        ast_child_node_from_line_opening(node, ASTN_PARAGRAPH, line, line_pos);
  }
  add_line_to_node(node, line + line_pos);
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
