#include "blocks.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "util.h"

/**
 * @brief Returns whether a match was found. Takes a pointer to a size_t
 * and sets that to the number of bytes matched against. 0 is a valid value
 * for this.
 *
 * @param node
 * @param line
 * @param match_len
 * @return int
 */
int matches_continuation_markers(ASTNode *node, const char *line,
                                 size_t *match_len) {
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
    *match_len = li;
    return 1;
  }
  *match_len = 0;
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

void close_descendent_blocks(ASTNode *node) {
  node->open = 0;
  if (node->children_count > 0) {
    close_descendent_blocks(node->children[node->children_count - 1]);
  }
}

int is_all_whitespace(const char *line) {
  size_t i = 0;
  while (line[i]) {
    if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {
      return 0;
    }
    i++;
  }
  return 1;
}

/***
 * Returns 0 if no block start is found.
 */
int block_start_type(const char *line, size_t *match_len) {
  if ((*match_len = matches_opening_tab(line))) {
    return ASTN_CODE_BLOCK;
  } else if (!is_all_whitespace(line)) {
    *match_len = 0;
    return ASTN_PARAGRAPH;
  }
  return 0;
}

int is_block_end(unsigned int node_type, const char *line) {
  if (node_type == ASTN_PARAGRAPH && is_all_whitespace(line)) {
    return 1;
  }
  return 0;
}

/* Returns a pointer to the deepest added child */
ASTNode *add_child_block(ASTNode *node, unsigned int node_type,
                         const char *base_cont_markers) {
  ASTNode *child;
  char *cont_markers = strdup(base_cont_markers);
  if (node_type == ASTN_CODE_BLOCK) {
    child = ast_create_node(ASTN_CODE_BLOCK);
    cont_markers = str_append(cont_markers, "\t");
    child->cont_markers = cont_markers;
    ast_add_child(node, child);
    return add_child_block(child, ASTN_PARAGRAPH, cont_markers);
  } else if (node_type == ASTN_PARAGRAPH) {
    child = ast_create_node(ASTN_PARAGRAPH);
    child->cont_markers = cont_markers;
    ast_add_child(node, child);
    return child;
  } else {
    printf("BAD CHILD BLOCK TYPE %u. CANT ADD IT\n", node_type);
    exit(EXIT_FAILURE);
  }
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

  // traverse to last matching node
  while (line[line_pos] && node->children_count > 0 &&
         node->children[node->children_count - 1]->open &&
         matches_continuation_markers(node->children[node->children_count - 1],
                                      line + line_pos, &match_len)) {
    node = node->children[node->children_count - 1];
    line_pos += match_len;
  }
  if ((node_type = block_start_type(line + line_pos, &match_len))) {
    close_descendent_blocks(node);
    line_pos += match_len;
    node = add_child_block(node, node_type, node->cont_markers);
    add_line_to_node(node, line + line_pos);
  } else if (is_block_end(node->type, line + line_pos)) {
    node->open = 0;
  } else if (node->type != ASTN_DOCUMENT) {
    add_line_to_node(node, line + line_pos);
  }
}

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
