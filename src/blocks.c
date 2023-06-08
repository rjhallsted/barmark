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

/* matching functions */
/*
 * these return 0 if not matched. If matched, return the number
 * of bytes used in match.
 */

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

int matches_list_opening(const char *line) {
  size_t i = 0;
  while (line[i] == ' ' && i < 3) {
    i++;
  }
  if (line[i] == '-') {
    i++;
    return i;
  }
  return 0;
}

/* end matching functions */

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
int block_start_type(const char *line, unsigned int current_node_type,
                     size_t *match_len) {
  if ((*match_len = matches_opening_tab(line))) {
    return ASTN_CODE_BLOCK;
  } else if ((*match_len = matches_list_opening(line))) {
    return ASTN_UNORDERED_LIST_ITEM;
  } else if (current_node_type != ASTN_PARAGRAPH && !is_all_whitespace(line)) {
    *match_len = 0;
    return ASTN_PARAGRAPH;
  }
  return 0;
}

/**
 * @brief Recursively checks if this node or any of its open children are closed
 * by this line. Returns NULL if false, and a pointer to the node-to-be-closed
 * if true.
 *
 * @param node_type
 * @param line
 * @return int
 */

ASTNode *is_block_end(ASTNode *node, const char *line) {
  // printf("   is_block_end on type %u\n", node->type);
  if (node->type == ASTN_PARAGRAPH && is_all_whitespace(line)) {
    return node;
  }
  if (node->children_count == 0) {
    return NULL;
  }
  return is_block_end(node->children[node->children_count - 1], line);
}

/* Returns a pointer to the deepest added child */
ASTNode *add_child_block(ASTNode *node, unsigned int node_type) {
  ASTNode *child;
  if (node_type == ASTN_CODE_BLOCK) {
    child = ast_create_node(ASTN_CODE_BLOCK);
    child->cont_markers = strdup("\t");
    ast_add_child(node, child);
    return add_child_block(child, ASTN_PARAGRAPH);
  } else if (node_type == ASTN_UNORDERED_LIST_ITEM &&
             node->type != ASTN_UNORDERED_LIST) {
    child = ast_create_node(ASTN_UNORDERED_LIST);
    child->cont_markers = strdup("");
    ast_add_child(node, child);
    return add_child_block(child, ASTN_UNORDERED_LIST_ITEM);
  } else if (node_type == ASTN_UNORDERED_LIST_ITEM &&
             node->type == ASTN_UNORDERED_LIST) {
    child = ast_create_node(ASTN_UNORDERED_LIST_ITEM);
    child->cont_markers = strdup("\t");
    ast_add_child(node, child);
    return add_child_block(child, ASTN_PARAGRAPH);
  } else if (node_type == ASTN_PARAGRAPH) {
    child = ast_create_node(ASTN_PARAGRAPH);
    child->cont_markers = strdup("");
    ast_add_child(node, child);
    return child;
  } else {
    printf("BAD CHILD BLOCK TYPE %u. CANT ADD IT\n", node_type);
    exit(EXIT_FAILURE);
  }
}

void print_trailing_end_of_tree(ASTNode *node) {
  char *output = strdup("");
  char tmp[100];
  while (1) {
    sprintf(tmp, "%u->", node->type);
    output = str_append(output, tmp);
    if (node->children_count == 0) {
      break;
    }
    node = node->children[node->children_count - 1];
  }
  printf("%s\n", output);
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
  ASTNode *node_to_close = NULL;
  unsigned int node_type;

  // printf("line: '%s'\n", line);

  // traverse to last matching node
  while (line[line_pos] && node->children_count > 0 &&
         node->children[node->children_count - 1]->open &&
         matches_continuation_markers(node->children[node->children_count - 1],
                                      line + line_pos, &match_len)) {
    node = node->children[node->children_count - 1];
    // printf("   matches %u\n", node->type);
    line_pos += match_len;
  }
  if ((node_type = block_start_type(line + line_pos, node->type, &match_len))) {
    // printf("   block start %u\n", node_type);
    close_descendent_blocks(node);
    line_pos += match_len;
    node = add_child_block(node, node_type);
    // print_trailing_end_of_tree(root);
    add_line_to_node(node, line + line_pos);
  } else if ((node_to_close = is_block_end(node, line + line_pos))) {
    // printf("   block end %u\n", node_to_close->type);
    node_to_close->open = 0;
  } else if (node->type != ASTN_DOCUMENT) {
    // printf("   add to %u\n", node->type);
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
