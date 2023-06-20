#include "blocks.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "util.h"

/* Controls whether a late continuation is possible. Gets set to 1
when an empty line is encountered, and set to 0 otherwise. */
unsigned int LATE_CONTINUATION_POSSBILE = 0;

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
  while (line[li] && node->cont_markers[ni] &&
         line[li] == node->cont_markers[ni]) {
    li++;
    ni++;
  }
  if (node->cont_markers[ni] == '\0') {
    *match_len = li;
    return 1;
  }
  *match_len = 0;
  return 0;
}

void move_contents_to_child_paragraph(ASTNode *node) {
  ASTNode *child = ast_create_node(ASTN_PARAGRAPH);
  ast_add_child(node, child);
  child->contents = node->contents;
  node->contents = NULL;
}

void add_line_to_node(ASTNode *node, const char *line) {
  // this is here to handle cases of late continuation where a new
  // block has not been created
  if (LATE_CONTINUATION_POSSBILE && node->type != ASTN_PARAGRAPH) {
    move_contents_to_child_paragraph(node);
    // set up another child paragraph for this line
    ASTNode *child = ast_create_node(ASTN_PARAGRAPH);
    ast_add_child(node, child);
    node = child;
  }
  if (node->type != ASTN_THEMATIC_BREAK) {
    if (node->contents == NULL) {
      node->contents = strdup("");
    }
    node->contents = str_append(node->contents, line);
  }
  if (node->type == ASTN_H1 || node->type == ASTN_THEMATIC_BREAK) {
    node->open = 0;
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

/* matching functions */
/*
 * these return 0 if not matched. If matched, return the number
 * of bytes used in match.
 *
 * copy line and tab expand with required lookahead. If match,
 * replace original line with tab expanded one.
 */

size_t matches_opening_tab(char **line, size_t line_pos) {
  size_t i = 0;
  char *line_ref = strdup(*line);
  tab_expand(&line_ref, line_pos, 4);

  while (line_ref[line_pos + i] == ' ' && i < 4) {
    i++;
  }
  if (i == 4) {
    free(*line);
    *line = line_ref;
    return i;
  }
  return 0;
}

size_t match_str_then_space(char *str, char **line, size_t line_pos) {
  size_t i = 0;
  char *line_ref = strdup(*line);
  tab_expand(&line_ref, line_pos, 3);

  while (line_ref[line_pos + i] == ' ' && i < 3) {
    i++;
  }
  if (str_starts_with(line_ref + line_pos + i, str)) {
    i += strlen(str);
    tab_expand(&line_ref, line_pos + i, 1);
    if (line_ref[line_pos + i] == ' ') {
      free(*line);
      *line = line_ref;
      return i + 1;
    }
  }
  return 0;
}

// TODO: Rework to make knowledgeable of indent level?
size_t matches_list_opening(char **line, size_t line_pos) {
  return match_str_then_space("-", line, line_pos);
}

size_t matches_paragraph_opening(char **line, size_t line_pos) {
  return (!is_all_whitespace((*line) + line_pos));
}

size_t matches_blockquote_opening(char **line, size_t line_pos) {
  size_t i = 0;
  char *line_ref = strdup(*line);
  tab_expand(&line_ref, line_pos, 3);

  while (line_ref[line_pos + i] == ' ' && i < 3) {
    i++;
  }
  if (line_ref[line_pos + i] == '>') {
    i++;
    tab_expand(&line_ref, line_pos + i, 1);
    if (line_ref[line_pos + i] == ' ') {
      i++;
    }
    free(*line);
    *line = line_ref;
    return i;
  }
  return 0;
}

size_t matches_h1_opening(char **line, size_t line_pos) {
  return match_str_then_space("#", line, line_pos);
}

size_t matches_thematic_break(char **line, size_t line_pos) {
  size_t i = 0;
  char *line_ref = *line;
  char c;

  while (line_ref[line_pos + i] == ' ' || line_ref[line_pos + i] == '\t') {
    i++;
  }
  if (line_ref[line_pos + i] == '*' || line_ref[line_pos + i] == '-' ||
      line_ref[line_pos + i] == '_') {
    c = line_ref[line_pos + i];
    i += 1;
  } else {
    return 0;
  }
  while (line_ref[line_pos + i] == ' ' || line_ref[line_pos + i] == '\t') {
    i++;
  }
  if (line_ref[line_pos + i] == c) {
    i++;
  } else {
    return 0;
  }
  while (line_ref[line_pos + i] == ' ' || line_ref[line_pos + i] == '\t') {
    i++;
  }
  if (line_ref[line_pos + i] == c) {
    i++;
  } else {
    return 0;
  }
  while (line_ref[line_pos + i] == ' ' || line_ref[line_pos + i] == '\t') {
    i++;
  }
  if (line_ref[line_pos + i] == '\n') {
    return i;
  } else {
    return 0;
  }
}

/* end matching functions */

void close_descendent_blocks(ASTNode *node) {
  node->open = 0;
  if (node->children_count > 0) {
    close_descendent_blocks(node->children[node->children_count - 1]);
  }
}

/***
 * Returns 0 if no block start is found.
 */
int block_start_type(char **line, size_t line_pos,
                     unsigned int current_node_type, size_t *match_len) {
  if ((*match_len = matches_opening_tab(line, line_pos))) {
    return ASTN_CODE_BLOCK;
  } else if ((*match_len = matches_list_opening(line, line_pos))) {
    return ASTN_UNORDERED_LIST_ITEM;
  } else if ((*match_len = matches_blockquote_opening(line, line_pos))) {
    return ASTN_BLOCK_QUOTE;
  } else if ((*match_len = matches_h1_opening(line, line_pos))) {
    return ASTN_H1;
  } else if ((*match_len = matches_thematic_break(line, line_pos))) {
    return ASTN_THEMATIC_BREAK;
  } else if (current_node_type != ASTN_PARAGRAPH &&
             matches_paragraph_opening(line, line_pos)) {
    return 0;
    //   *match_len = 0;
    //   return ASTN_PARAGRAPH;
  }
  return 0;
}

/**
 * @brief Recursively checks if this node or any of its open children are
 * closed by this line. Returns NULL if false, and a pointer to the
 * node-to-be-closed if true.
 *
 * @param node_type
 * @param line
 * @return int
 */

ASTNode *is_block_end(ASTNode *node, const char *line) {
  if (node->type == ASTN_PARAGRAPH && is_all_whitespace(line)) {
    return node;
  }
  if (node->children_count == 0) {
    return NULL;
  }
  return is_block_end(node->children[node->children_count - 1], line);
}

/* Returns a pointer to the deepest added child */
ASTNode *add_child_block(ASTNode *node, unsigned int node_type,
                         size_t opener_match_len) {
  ASTNode *child;

  if (LATE_CONTINUATION_POSSBILE && node->contents) {
    move_contents_to_child_paragraph(node);
    LATE_CONTINUATION_POSSBILE = 0;
  }

  if (node_type == ASTN_CODE_BLOCK) {
    child = ast_create_node(ASTN_CODE_BLOCK);
    child->cont_markers = repeat_x(' ', 4);
    ast_add_child(node, child);
    return child;
  } else if (node_type == ASTN_UNORDERED_LIST_ITEM &&
             node->type != ASTN_UNORDERED_LIST) {
    child = ast_create_node(ASTN_UNORDERED_LIST);
    child->cont_markers = strdup("");
    ast_add_child(node, child);
    return add_child_block(child, ASTN_UNORDERED_LIST_ITEM, opener_match_len);
  } else if (node_type == ASTN_UNORDERED_LIST_ITEM &&
             node->type == ASTN_UNORDERED_LIST) {
    child = ast_create_node(ASTN_UNORDERED_LIST_ITEM);
    child->cont_markers = repeat_x(' ', opener_match_len);
    ast_add_child(node, child);
    return child;
  } else if (node_type == ASTN_BLOCK_QUOTE) {
    child = ast_create_node(ASTN_BLOCK_QUOTE);
    child->cont_markers = strdup(">");
    ast_add_child(node, child);
    return child;
  } else if (node_type == ASTN_H1) {
    child = ast_create_node(ASTN_H1);
    ast_add_child(node, child);
    return child;
  } else if (node_type == ASTN_THEMATIC_BREAK) {
    child = ast_create_node(ASTN_THEMATIC_BREAK);
    ast_add_child(node, child);
    return child;
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

void print_tree(ASTNode *node, size_t level) {
  char *indent = repeat_x(' ', level * 2);
  printf("%s%s\n", indent, NODE_TYPE_NAMES[node->type]);
  if (node->children_count > 0) {
    for (size_t i = 0; i < node->children_count; i++) {
      print_tree(node->children[i], level + 1);
    }
  } else {
    printf("%s->%s\n", indent, node->contents);
  }
  free(indent);
}

/**
 * @brief Expands the next tab if present in the next 4 chars,
 * starting at line_pos. returns new line_pos
 *
 * @param line
 * @param line_pos
 * @return size_t
 */
void tab_expand(char **line, size_t line_pos, size_t lookahead) {
  char *line_ref = *line;
  unsigned int i = 0;

  while (line_ref[line_pos + i] == ' ' && i < (lookahead - 1)) {
    i++;
  }
  if (line_ref[line_pos + i] != '\t') {
    return;
  }
  unsigned int spaces_to_add = 4 - ((line_pos + i) % 4);

  char *out = strndup(line_ref, line_pos + i);
  char *tmp = repeat_x(' ', spaces_to_add);
  out = str_append(out, tmp);
  free(tmp);
  tmp = strdup(line_ref + line_pos + i + 1);
  out = str_append(out, tmp);
  free(tmp);
  free(line_ref);
  *line = out;
}

// traverse to deepest/lastest open block, building up continuation markers
// along the way consume continuation as you go, stop when no longer matching
// look for new block starts
// if found, close remaining unmatched blocks
// begin new block as child of last matched block
// incorporate remainder of line in last open block
void add_line_to_ast(ASTNode *root, char **line) {
  size_t line_pos = 0;
  size_t match_len = 0;
  ASTNode *node = root;
  ASTNode *node_to_close = NULL;
  unsigned int node_type;

  if (is_all_whitespace(*line)) {
    LATE_CONTINUATION_POSSBILE = 1;
    return;
  }

  // traverse to last matching node
  while ((*line)[line_pos] && node->children_count > 0 &&
         node->children[node->children_count - 1]->open) {
    tab_expand(line, line_pos, 4);
    if (node->children[node->children_count - 1]->type == ASTN_PARAGRAPH ||
        !matches_continuation_markers(node->children[node->children_count - 1],
                                      (*line) + line_pos, &match_len)) {
      break;
    }
    node = node->children[node->children_count - 1];
    line_pos += match_len;
  }
  // If a block start exists, create it, then keep checking for more.
  // add the line to the last one
  if ((node_type = block_start_type(line, line_pos, node->type, &match_len))) {
    // close_descendent_blocks(node);
    line_pos += match_len;
    node = add_child_block(node, node_type, match_len);
    while ((node_type =
                block_start_type(line, line_pos, node->type, &match_len))) {
      line_pos += match_len;
      node = add_child_block(node, node_type, match_len);
    }
    add_line_to_node(node, (*line) + line_pos);
  } else if (node->children_count > 0 &&
             node->children[node->children_count - 1]->type == ASTN_PARAGRAPH) {
    node = node->children[node->children_count - 1];
    add_line_to_node(node, (*line) + line_pos);
  } else if (node_type == ASTN_DOCUMENT) {
    // if we have content, but not block starts and we havent descended at all,
    // this is just a paragraph
    node = add_child_block(node, ASTN_PARAGRAPH, 0);
    add_line_to_node(node, (*line) + line_pos);
  } else if ((node_to_close = is_block_end(node, (*line) + line_pos))) {
    node_to_close->open = 0;
  } else if (node->type != ASTN_DOCUMENT) {
    add_line_to_node(node, (*line) + line_pos);
  }
  // printf("---------------\n");
  // print_tree(root, 0);
  LATE_CONTINUATION_POSSBILE = 0;
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
    add_line_to_ast(document, &line);
  }
  return document;
}
