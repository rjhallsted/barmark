#include "blocks.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "util.h"

// TODO: Probably rework setext heading handling so that we don't
// require different AST node types to handle them

/* Controls whether a late continuation is possible. Gets set to 1
when an empty line is encountered, and set to 0 otherwise. */
unsigned int LATE_CONTINUATION_LINES = 0;
char *LATE_CONTINUATION_CONTENTS = NULL;

void reset_late_continuation(void) {
  LATE_CONTINUATION_LINES = 0;
  free(LATE_CONTINUATION_CONTENTS);
  LATE_CONTINUATION_CONTENTS = strdup("");
}

bool matches_continuation_markers_with_leading_spaces(
    ASTNode node[static 1], char const line[static 1],
    size_t match_len[static 1]) {
  size_t i = 0;
  while (line[i] == ' ' && i < 3) {
    i++;
  }
  int res = matches_continuation_markers(node, line + i, match_len);
  if (res) {
    *match_len += i;
  }
  return res;
}

/**
 * @brief Returns whether a match was found. Takes a pointer to a size_t
 * and sets that to the number of bytes matched against. 0 is a valid value
 * for this.
 *
 * @param node
 * @param line
 * @param match_len
 * @return bool
 */
bool matches_continuation_markers(ASTNode node[static 1],
                                  char const line[static 1],
                                  size_t match_len[static 1]) {
  if (!(node->cont_markers)) {
    return 1;
  }
  if (f_debug()) printf("matching cont markers: '%s'\n", node->cont_markers);
  size_t ni = 0, li = 0;
  while (line[li] && node->cont_markers[ni] &&
         line[li] == node->cont_markers[ni]) {
    li++;
    ni++;
  }
  if (node->cont_markers[ni] == '\0') {
    *match_len = li;
    return true;
  }
  *match_len = 0;
  return false;
}

/**
 * @brief Get the last child node. Returns NULL if no children exist.
 *
 * @param node
 * @return ASTNode*
 */
ASTNode *get_last_child(ASTNode node[static 1]) {
  if (node->children_count == 0) {
    return NULL;
  }
  return node->children[node->children_count - 1];
}

ASTNode *get_deepest_non_text_child(ASTNode node[static 1]) {
  ASTNode *child;
  while ((child = get_last_child(node)) && child->type != ASTN_TEXT) {
    node = child;
  }
  return node;
}

void convert_last_text_child_to_paragraph(ASTNode node[static 1]) {
  ASTNode *text = get_last_child(node);
  if (!text || text->type != ASTN_TEXT) {
    printf("BAD OPERATION. Can't convert non-text node to paragraph.\n");
    exit(EXIT_FAILURE);
  }
  ASTNode *child = ast_create_node(ASTN_PARAGRAPH);
  node->children[node->children_count - 1] = child;
  ast_add_child(child, text);
}

bool is_whitespace(char c) { return (c == ' ' || c == '\t' || c == '\n'); }

bool is_all_whitespace(char const line[static 1]) {
  size_t i = 0;
  while (is_whitespace(line[i])) {
    i++;
  }
  return line[i] == '\0';
}

// converts all text children to paragraphs
void convert_texts_to_paragraphs(ASTNode node[static 1]) {
  ASTNode *text, *child;
  for (size_t i = 0; i < node->children_count; i++) {
    if (node->children[i]->type == ASTN_TEXT &&
        !is_all_whitespace(node->children[i]->contents)) {
      text = node->children[i];
      child = ast_create_node(ASTN_PARAGRAPH);
      node->children[i] = child;
      ast_add_child(child, text);
    }
  }
}

void add_line_to_node(ASTNode node[static 1], char *line) {
  ASTNode *child = get_last_child(node);

  if (f_debug()) {
    printf("adding line to %s\n", NODE_TYPE_NAMES[node->type]);
  }

  // force the thing we're adding content to be a text node
  if (!child || child->type != ASTN_TEXT) {
    child = ast_create_node(ASTN_TEXT);
    ast_add_child(node, child);
    child->contents = strdup("");
  }
  // should only apply to code blocks with existing content
  if (strlen(child->contents) > 0 && LATE_CONTINUATION_LINES) {
    if (f_debug()) printf("adding late continuation lines to contents\n");
    if (node->type == ASTN_CODE_BLOCK) {
      child->contents = str_append(child->contents, LATE_CONTINUATION_CONTENTS);
    } else {
      for (unsigned int i = 0; i < LATE_CONTINUATION_LINES; i++) {
        child->contents = str_append(child->contents, "\n");
      }
    }
  }
  child->contents = str_append(child->contents, line);
}

/* matching functions */
/*
 * these return 0 if not matched. If matched, return the number
 * of bytes used in match.
 *
 * copy line and tab expand with required lookahead. If match,
 * replace original line with tab expanded one.
 */

size_t match_up_to_3_spaces(char *line[static 1], size_t line_pos) {
  size_t i = 0;
  char *line_ref = strdup(*line);
  tab_expand(&line_ref, line_pos, 3);

  while (line_ref[line_pos + i] == ' ' && i < 3) {
    i++;
  }
  if (i > 0) {
    free(*line);
    *line = line_ref;
    return i;
  } else {
    free(line_ref);
    return 0;
  }
}

size_t matches_code_block(char *line[static 1], size_t line_pos) {
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
  free(line_ref);
  return 0;
}

size_t match_str_then_space(char const str[static 1], char *line[static 1],
                            size_t line_pos) {
  char *line_ref = strdup(*line);
  size_t i = match_up_to_3_spaces(&line_ref, line_pos);

  if (str_starts_with(line_ref + line_pos + i, str)) {
    i += strlen(str);
    tab_expand(&line_ref, line_pos + i, 1);
    if (line_ref[line_pos + i] == ' ') {
      free(*line);
      *line = line_ref;
      return i + 1;
    }
  }
  free(line_ref);
  return 0;
}

size_t matches_unoredered_list_opener_with_symbol(char const str[static 1],
                                                  char *line[static 1],
                                                  size_t line_pos) {
  size_t res = match_str_then_space(str, line, line_pos);
  if (res) {
    return res;
  } else {
    // try case where we match the symbol, but then line is all whitespace
    char *line_ref = strdup(*line);
    res = match_up_to_3_spaces(&line_ref, line_pos);
    if (str_starts_with(line_ref + line_pos + res, str)) {
      res += strlen(str);
      if (is_all_whitespace(line_ref + line_pos + res)) {
        free(*line);
        *line = line_ref;
        return res;
      }
    }
    free(line_ref);
    return 0;
  }
}

size_t matches_unordered_list_opening(char *line[static 1], size_t line_pos) {
  size_t res = matches_unoredered_list_opener_with_symbol("-", line, line_pos);
  if (!res)
    res = matches_unoredered_list_opener_with_symbol("*", line, line_pos);
  if (!res)
    res = matches_unoredered_list_opener_with_symbol("+", line, line_pos);
  return res;
}

size_t matches_ordered_list_opening(char *line[static 1], size_t line_pos) {
  char *line_ref = strdup(*line);
  line_pos += match_up_to_3_spaces(&line_ref, line_pos);
  size_t i = 0;
  while (line_ref[line_pos + i] >= '0' && line_ref[line_pos + i] <= '9') {
    i++;
  }
  if (i == 0 || line_ref[line_pos + i] != '.') {
    free(line_ref);
    return 0;
  }
  i++;
  tab_expand(&line_ref, line_pos + i, 1);
  if (line_ref[line_pos + i] != ' ' && line_ref[line_pos + i] != '\n' &&
      line_ref[line_pos + i] != '\0') {
    free(line_ref);
    return 0;
  }
  return i + 1;
}

size_t matches_paragraph_opening(char *line[static 1], size_t line_pos) {
  return (!is_all_whitespace((*line) + line_pos));
}

size_t matches_blockquote_opening(char *line[static 1], size_t line_pos) {
  char *line_ref = strdup(*line);
  size_t i = match_up_to_3_spaces(&line_ref, line_pos);
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
  free(line_ref);
  return 0;
}

size_t matches_atx_header(char *line[static 1], size_t line_pos,
                          int unsigned heading_level) {
  char *header_str = repeat_x('#', heading_level);
  size_t res1 = match_str_then_space(header_str, line, line_pos);
  if (res1) {
    free(header_str);
    return res1;
  }
  if (strcmp((*line) + line_pos, header_str) == 0) {
    free(header_str);
    return heading_level;
  }
  header_str = str_append(header_str, "\n");
  if (strcmp((*line) + line_pos, header_str) == 0) {
    free(header_str);
    return heading_level;
  }
  free(header_str);
  return 0;
}

size_t matches_h1_opening(char *line[static 1], size_t line_pos) {
  return matches_atx_header(line, line_pos, 1);
}

size_t matches_h2_opening(char *line[static 1], size_t line_pos) {
  return matches_atx_header(line, line_pos, 2);
}

size_t matches_h3_opening(char *line[static 1], size_t line_pos) {
  return matches_atx_header(line, line_pos, 3);
}

size_t matches_h4_opening(char *line[static 1], size_t line_pos) {
  return matches_atx_header(line, line_pos, 4);
}

size_t matches_h5_opening(char *line[static 1], size_t line_pos) {
  return matches_atx_header(line, line_pos, 5);
}

size_t matches_h6_opening(char *line[static 1], size_t line_pos) {
  return matches_atx_header(line, line_pos, 6);
}

size_t matches_thematic_break(char *line[static 1], size_t line_pos) {
  char *line_ref = strdup(*line);
  size_t i = match_up_to_3_spaces(&line_ref, line_pos);
  char c = line_ref[line_pos + i];
  if (c != '*' && c != '-' && c != '_') {
    free(line_ref);
    return 0;
  }
  i++;
  while (is_whitespace(line_ref[line_pos + i])) {
    i++;
  }
  if (line_ref[line_pos + i] != c) {
    free(line_ref);
    return 0;
  }
  i++;
  while (is_whitespace(line_ref[line_pos + i])) {
    i++;
  }
  if (line_ref[line_pos + i] != c) {
    free(line_ref);
    return 0;
  }
  i++;
  while (is_whitespace(line_ref[line_pos + i]) || line_ref[line_pos + i] == c) {
    i++;
  }
  if (line_ref[line_pos + i] != '\0') {
    free(line_ref);
    return 0;
  }
  free(*line);
  *line = line_ref;
  return i;
}

size_t matches_setext_h2(char *line[static 1], size_t line_pos) {
  char *line_ref = strdup(*line);
  size_t i = match_up_to_3_spaces(&line_ref, line_pos);
  if (line_ref[line_pos + i] != '-') {
    free(line_ref);
    return 0;
  }
  while (line_ref[line_pos + i] == '-') {
    i++;
  }
  while (is_whitespace(line_ref[line_pos + i])) {
    i++;
  }
  if (line_ref[line_pos + i] != '\0') {
    free(line_ref);
    return 0;
  }
  *line = line_ref;
  return i;
}

size_t matches_setext_h1(char *line[static 1], size_t line_pos) {
  char *line_ref = strdup(*line);
  size_t i = match_up_to_3_spaces(&line_ref, line_pos);
  if (line_ref[line_pos + i] != '=') {
    free(line_ref);
    return 0;
  }
  while (line_ref[line_pos + i] == '=') {
    i++;
  }
  while (is_whitespace(line_ref[line_pos + i])) {
    i++;
  }
  if (line_ref[line_pos + i] != '\0') {
    free(line_ref);
    return 0;
  }
  *line = line_ref;
  return i;
}

/* end matching functions */

void close_descendent_blocks(ASTNode node[static 1]) {
  node->open = 0;
  if (node->children_count > 0) {
    close_descendent_blocks(node->children[node->children_count - 1]);
  }
}

bool has_open_child(ASTNode node[static 1]) {
  return (node->children_count > 0 &&
          node->children[node->children_count - 1]->open &&
          node->children[node->children_count - 1]->type != ASTN_TEXT);
}

bool has_text(ASTNode node[static 1]) {
  for (size_t i = 0; i < node->children_count; i++) {
    if (node->children[i]->type == ASTN_TEXT) return true;
  }
  return false;
}

char *get_node_contents(ASTNode node[static 1]) {
  ASTNode *child = get_last_child(node);
  if (!child || child->type != ASTN_TEXT) {
    return NULL;
  }
  return child->contents;
}

ASTNode *find_in_edge_of_tree(ASTNode node[static 1], unsigned int type) {
  if (node->type == type) {
    return node;
  }
  if (has_open_child(node)) {
    node = get_last_child(node);
    return find_in_edge_of_tree(node, type);
  }
  return NULL;
}

bool meets_setext_conditions(ASTNode node[static 1]) {
  ASTNode *child = get_last_child(node);
  return (child &&
          (child->type == ASTN_PARAGRAPH || child->type == ASTN_TEXT) &&
          !LATE_CONTINUATION_LINES);
}

bool meets_code_block_conditions(ASTNode node[static 1]) {
  ASTNode *deepest = get_deepest_non_text_child(node);
  if (!LATE_CONTINUATION_LINES &&
      (array_contains(NOT_INTERRUPTIBLE_BY_CODE_BLOCK_SIZE,
                      NOT_INTERRUPTIBLE_BY_CODE_BLOCK, node->type) ||
       array_contains(NOT_INTERRUPTIBLE_BY_CODE_BLOCK_SIZE,
                      NOT_INTERRUPTIBLE_BY_CODE_BLOCK, deepest->type))) {
    return false;
  }
  return true;
}

bool is_empty_line_following_paragraph(char const line[static 1],
                                       ASTNode preceding_node[static 1]) {
  if (preceding_node) {
    return is_all_whitespace(line) && preceding_node->type == ASTN_PARAGRAPH;
  } else
    return false;
}

/***
 * Returns 0 if no block start is found.
 */
int unsigned block_start_type(char *line[static 1], size_t line_pos,
                              ASTNode current_node[static 1],
                              size_t match_len[static 1]) {
  ASTNode *child = get_last_child(current_node);
  if (f_debug()) {
    printf("line: '%s'\n", (*line) + line_pos);
    printf("current_node_type: %s\n", NODE_TYPE_NAMES[current_node->type]);
    if (child) {
      printf("child node type: %s\n", NODE_TYPE_NAMES[child->type]);
    }
  }

  if (meets_code_block_conditions(current_node) &&
      (*match_len = matches_code_block(line, line_pos))) {
    return ASTN_CODE_BLOCK;
  } else if (meets_setext_conditions(current_node) &&
             (*match_len = matches_setext_h2(line, line_pos))) {
    return ASTN_SETEXT_H2;
  } else if (meets_setext_conditions(current_node) &&
             (*match_len = matches_setext_h1(line, line_pos))) {
    return ASTN_SETEXT_H1;
  } else if ((*match_len = matches_thematic_break(line, line_pos))) {
    return ASTN_THEMATIC_BREAK;
  } else if ((*match_len = matches_unordered_list_opening(line, line_pos)) &&
             !is_empty_line_following_paragraph((*line) + (*match_len),
                                                child)) {
    return ASTN_UNORDERED_LIST_ITEM;
  } else if ((*match_len = matches_ordered_list_opening(line, line_pos)) &&
             !is_empty_line_following_paragraph((*line) + (*match_len),
                                                child)) {
    return ASTN_ORDERED_LIST_ITEM;
  } else if ((*match_len = matches_blockquote_opening(line, line_pos))) {
    return ASTN_BLOCK_QUOTE;
  } else if ((*match_len = matches_h6_opening(line, line_pos))) {
    return ASTN_H6;
  } else if ((*match_len = matches_h5_opening(line, line_pos))) {
    return ASTN_H5;
  } else if ((*match_len = matches_h4_opening(line, line_pos))) {
    return ASTN_H4;
  } else if ((*match_len = matches_h3_opening(line, line_pos))) {
    return ASTN_H3;
  } else if ((*match_len = matches_h2_opening(line, line_pos))) {
    return ASTN_H2;
  } else if ((*match_len = matches_h1_opening(line, line_pos))) {
    return ASTN_H1;
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
ASTNode *is_block_end(ASTNode node[static 1], char const line[static 1]) {
  if (node->type == ASTN_PARAGRAPH && is_all_whitespace(line)) {
    return node;
  }
  if (node->children_count == 0) {
    return NULL;
  }
  return is_block_end(node->children[node->children_count - 1], line);
}

/* returns a pointer to the child */
ASTNode *add_child_block_with_cont_markers(ASTNode node[static 1],
                                           int unsigned node_type,
                                           char cont_markers[static 1]) {
  ASTNode *child = ast_create_node(node_type);
  child->cont_markers = cont_markers;
  ast_add_child(node, child);
  return child;
}

/* Returns a pointer to the deepest added child.
Contains all logic for block creation.
*/
ASTNode *add_child_block(ASTNode node[static 1], int unsigned node_type,
                         size_t opener_match_len, char list_char) {
  ASTNode *child;

  if (node_type == ASTN_CODE_BLOCK) {
    return add_child_block_with_cont_markers(node, ASTN_CODE_BLOCK,
                                             repeat_x(' ', 4));
  } else if (node_type == ASTN_UNORDERED_LIST_ITEM &&
             node->type != ASTN_UNORDERED_LIST) {
    child = ast_create_node(ASTN_UNORDERED_LIST);
    child->options = malloc(sizeof(ASTListOptions));
    child->options->marker = list_char;
    child->options->wide = 0;
    ast_add_child(node, child);
    return add_child_block(child, ASTN_UNORDERED_LIST_ITEM, opener_match_len,
                           0);
  } else if (node_type == ASTN_ORDERED_LIST_ITEM &&
             node->type != ASTN_ORDERED_LIST) {
    child = ast_create_node(ASTN_ORDERED_LIST);
    child->options = malloc(sizeof(ASTListOptions));
    child->options->marker = list_char;
    child->options->wide = 0;
    ast_add_child(node, child);
    return add_child_block(child, ASTN_ORDERED_LIST_ITEM, opener_match_len, 0);
  } else if (node_type == ASTN_UNORDERED_LIST_ITEM &&
             node->type == ASTN_UNORDERED_LIST) {
    return child = add_child_block_with_cont_markers(
               node, ASTN_UNORDERED_LIST_ITEM, repeat_x(' ', opener_match_len));
  } else if (node_type == ASTN_ORDERED_LIST_ITEM &&
             node->type == ASTN_ORDERED_LIST) {
    return child = add_child_block_with_cont_markers(
               node, ASTN_ORDERED_LIST_ITEM, repeat_x(' ', opener_match_len));
  } else if (node_type == ASTN_BLOCK_QUOTE) {
    return add_child_block_with_cont_markers(node, ASTN_BLOCK_QUOTE,
                                             strdup(">"));
  } else if (node_type == ASTN_SETEXT_H1 || node_type == ASTN_SETEXT_H2) {
    // Instead of adding a new child, for setext headings we just change the
    // type of its "sibling" paragraph. Or in the case there is no paragraph, we
    // take the parent node's contents and convert it to a paragraph first
    child = get_last_child(node);
    if (!child) {
      printf(
          "FAILURE. There should always be a child when you add a Setext "
          "heading.\n");
      exit(EXIT_FAILURE);
    }
    if (child->type == ASTN_TEXT) {
      if (f_debug())
        printf("converting block text to paragraph for setext conversion\n");
      convert_last_text_child_to_paragraph(node);
      child = get_last_child(node);
    }
    child->type = node_type;
    return child;
  } else if (node_type == ASTN_H1 || node_type == ASTN_H2 ||
             node_type == ASTN_H3 || node_type == ASTN_H4 ||
             node_type == ASTN_H5 || node_type == ASTN_H6 ||
             node_type == ASTN_PARAGRAPH || node_type == ASTN_THEMATIC_BREAK) {
    child = ast_create_node(node_type);
    ast_add_child(node, child);
    return child;
  } else {
    printf("BAD CHILD BLOCK TYPE %s. CANT ADD IT\n",
           NODE_TYPE_NAMES[node_type]);
    exit(EXIT_FAILURE);
  }
}

void print_tree(ASTNode node[static 1], size_t level) {
  char *indent = repeat_x(' ', level * 2);
  if (node->options) {
    printf("%s%s-%s\n", indent, NODE_TYPE_NAMES[node->type],
           node->options->wide ? "wide" : "tight");
  } else if (node->cont_markers) {
    printf("%s%s->'%s'\n", indent, NODE_TYPE_NAMES[node->type],
           node->cont_markers);
  } else {
    printf("%s%s\n", indent, NODE_TYPE_NAMES[node->type]);
  }

  if (node->contents) {
    printf("%s+>%s\n", indent, node->contents);
  }
  for (size_t i = 0; i < node->children_count; i++) {
    print_tree(node->children[i], level + 1);
  }
  free(indent);
}

/**
 * @brief Expands the next tab if present in the next <lookahead> chars,
 * starting at line_pos. returns new line_pos
 *
 * @param line
 * @param line_pos
 * @return size_t
 */
void tab_expand(char *line[static 1], size_t line_pos, size_t lookahead) {
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

char find_list_char(char line[static 1]) {
  size_t i = 0;
  while (line[i] && line[i] != '-' && line[i] != '*') {
    i++;
  }
  if (line[i]) {
    return line[i];
  }
  return 0;
}

bool should_add_to_parent_instead(ASTNode node[static 1],
                                  int unsigned new_node_type, char list_char) {
  return ((node->type == ASTN_UNORDERED_LIST &&
           new_node_type != ASTN_UNORDERED_LIST_ITEM) ||
          (node->type == ASTN_UNORDERED_LIST &&
           new_node_type == ASTN_UNORDERED_LIST_ITEM && node->options &&
           node->options->marker != list_char));
}

ASTNode *traverse_to_last_match(ASTNode node[static 1], char *line[static 1],
                                size_t line_pos[static 1],
                                size_t match_len[static 1]) {
  ASTNode *child;
  // traverse to last matching node
  while ((*line)[*line_pos] && has_open_child(node)) {
    tab_expand(line, *line_pos, 4);
    if (f_debug()) {
      printf("matching against %s\n",
             NODE_TYPE_NAMES[get_last_child(node)->type]);
    }
    child = get_last_child(node);
    if (child->type == ASTN_PARAGRAPH ||
        (!matches_continuation_markers(child, (*line) + (*line_pos),
                                       match_len) &&
         !matches_continuation_markers_with_leading_spaces(
             child, (*line) + (*line_pos), match_len))) {
      break;
    }
    node = child;
    *line_pos += *match_len;
  }
  return node;
}

bool is_leaf_only_node(int unsigned type) {
  return array_contains(LEAF_ONLY_NODES_SIZE, LEAF_ONLY_NODES, type);
}

ASTNode *handle_new_block_starts(ASTNode node[static 1], char *line[static 1],
                                 size_t line_pos[static 1],
                                 size_t match_len[static 1]) {
  char list_char = 0;
  int unsigned node_type;

  // If a block start exists, create it, then keep checking for more.
  // Intelligently rework blocks if new block needs to be moved to parent
  while ((node_type = block_start_type(line, *line_pos, node, match_len)) &&
         !is_leaf_only_node(node->type)  // &&
         //  !is_disallowed_child(node->type, node_type)
  ) {
    if (node_type == ASTN_UNORDERED_LIST_ITEM) {
      list_char = find_list_char((*line) + (*line_pos));
    }
    *line_pos += *match_len;

    if (f_debug())
      printf("block start node_type: %s\n", NODE_TYPE_NAMES[node_type]);

    // enforce cases of required child types
    if (should_add_to_parent_instead(node, node_type, list_char)) {
      node = node->parent;
    }
    node = add_child_block(node, node_type, *match_len, list_char);
    if (f_debug()) printf("new block start: %s\n", NODE_TYPE_NAMES[node->type]);
  }
  // cleanup list items so that continuation markers use all leading spaces.
  if ((node->type == ASTN_UNORDERED_LIST_ITEM ||
       node->type == ASTN_ORDERED_LIST_ITEM) &&
      !is_all_whitespace((*line) + (*line_pos))) {
    size_t diff = match_up_to_3_spaces(line, *line_pos);
    char *tmp = repeat_x(' ', diff);
    node->cont_markers = str_append(node->cont_markers, tmp);
    free(tmp);
    *line_pos += diff;
  }

  return node;
}

void widen_list(ASTNode node[static 1]) {
  node->options->wide = 1;
  // fix existing list items
  for (size_t i = 0; i < node->children_count; i++) {
    convert_texts_to_paragraphs(node->children[i]);
  }
}

void swap_nodes(ASTNode a[static 1], ASTNode b[static 1]) {
  ASTNode tmp = *a;
  *a = *b;
  *b = tmp;
}

bool is_item_in_narrow_list_that_should_become_wide(ASTNode node[static 1]) {
  return (node->type == ASTN_UNORDERED_LIST_ITEM ||
          node->type == ASTN_ORDERED_LIST_ITEM) &&
         !node->parent->options->wide && LATE_CONTINUATION_LINES == 1 &&
         has_text(node) && !is_all_whitespace(get_node_contents(node));
}

bool is_new_item_in_wide_list(ASTNode node[static 1]) {
  return (node->type == ASTN_UNORDERED_LIST_ITEM ||
          node->type == ASTN_ORDERED_LIST_ITEM) &&
         !has_open_child(node) && node->parent->options->wide;
}

bool is_new_line_in_item_in_narrow_list(ASTNode node[static 1],
                                        char const line[static 1]) {
  return (node->type == ASTN_UNORDERED_LIST ||
          node->type == ASTN_ORDERED_LIST) &&
         !LATE_CONTINUATION_LINES && has_open_child(node) &&
         !is_all_whitespace(line);
}

bool is_child_of_item_in_narrow_list_that_should_become_wide(
    ASTNode node[static 1]) {
  return node->parent &&
         (node->parent->type == ASTN_UNORDERED_LIST_ITEM ||
          node->parent->type == ASTN_ORDERED_LIST_ITEM) &&
         !node->parent->parent->options->wide && has_text(node->parent) &&
         LATE_CONTINUATION_LINES == 1;
}

bool is_list_item_with_unusable_late_continuation_lines(
    ASTNode node[static 1]) {
  // list items can only use empty continuation lines if the first or second
  // line actually has contents
  return LATE_CONTINUATION_LINES &&
         (node->type == ASTN_UNORDERED_LIST_ITEM ||
          node->type == ASTN_ORDERED_LIST_ITEM) &&
         has_text(node) && is_all_whitespace(get_node_contents(node));
}

bool is_continuable_list_item(ASTNode node[static 1]) {
  return LATE_CONTINUATION_LINES && (node->type == ASTN_UNORDERED_LIST_ITEM ||
                                     node->type == ASTN_ORDERED_LIST_ITEM);
}

bool is_continuable_paragraph(ASTNode node[static 1],
                              char const line[static 1]) {
  ASTNode *child = get_last_child(node);
  return has_open_child(node) && child->type == ASTN_PARAGRAPH &&
         !LATE_CONTINUATION_LINES && !is_all_whitespace(line);
}

bool has_child_paragraph_that_should_be_closed(ASTNode node[static 1],
                                               char const line[static 1]) {
  ASTNode *child = get_last_child(node);
  return has_open_child(node) && child->type == ASTN_PARAGRAPH &&
         !LATE_CONTINUATION_LINES && is_all_whitespace(line);
}

bool should_determine_context_from_blockquote_child(ASTNode node[static 1],
                                                    char const line[static 1]) {
  ASTNode *child = get_last_child(node);
  return has_open_child(node) && child->type == ASTN_BLOCK_QUOTE &&
         !is_all_whitespace(line) && has_open_child(child);
}

bool should_split_blockquote(ASTNode node[static 1]) {
  return node->type == ASTN_BLOCK_QUOTE && LATE_CONTINUATION_LINES &&
         has_open_child(node);
}

bool should_add_default_paragraph_to_blockquote(ASTNode node[static 1],
                                                char const line[static 1]) {
  ASTNode *child = get_last_child(node);
  return node->type == ASTN_BLOCK_QUOTE && !is_all_whitespace(line) &&
         (!has_open_child(node) ||
          (has_open_child(node) && child->type != ASTN_PARAGRAPH));
}

ASTNode *determine_writable_node_from_context(ASTNode node[static 1],
                                              char const line[static 1]) {
  /* logic to determine where to add line based on current node and context.
    Tread carefully, as predicate order matters.
   */

  //////////
  if (f_debug()) {
    printf("determining node from context on %s, line: '%s'\n",
           NODE_TYPE_NAMES[node->type], line);
  }

  ASTNode *child = get_last_child(node);

  if (is_item_in_narrow_list_that_should_become_wide(node)) {
    if (f_debug()) printf("widening list\n");
    widen_list(node->parent);
    // have the next case handle the new item now that the list state is fixed
    return determine_writable_node_from_context(node, line);
  } else if (is_new_item_in_wide_list(node)) {
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0);
    return determine_writable_node_from_context(child, line);
  } else if (is_new_line_in_item_in_narrow_list(node, line)) {
    if (f_debug())
      printf("Adding trailing line to last item in unordered list.\n");
    node = child;
    return determine_writable_node_from_context(node, line);
  } else if (is_child_of_item_in_narrow_list_that_should_become_wide(node)) {
    if (f_debug()) printf("widening list as child of list item\n");
    // widen list, which adds another item to the parent, then swap the
    // children, to maintain correct order
    widen_list(node->parent->parent);
    swap_nodes(node, node->parent->children[1]);
    node = node->parent->children[1];
    return determine_writable_node_from_context(node, line);
  } else if (is_list_item_with_unusable_late_continuation_lines(node)) {
    if (f_debug())
      printf(
          "Closing this list due to continuation lines and adding a paragraph "
          "instead\n");
    node->parent->open = 0;
    node = node->parent->parent;
    return determine_writable_node_from_context(node, line);
  } else if (is_continuable_list_item(node)) {
    if (!node->parent->options->wide) {
      if (f_debug())
        printf("widening list and adding a new paragraph to this node\n");
      widen_list(node->parent);
    } else {
      if (f_debug()) printf("adding a new paragraph to this node\n");
    }

    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0);
    return determine_writable_node_from_context(child, line);
  } else if (node->type == ASTN_UNORDERED_LIST ||
             node->type == ASTN_ORDERED_LIST) {
    // matched the list, but not as a list item, so shouldn't actually be in the
    // list
    node = node->parent;
    return determine_writable_node_from_context(node, line);
  } else if (is_continuable_paragraph(node, line)) {
    return determine_writable_node_from_context(child, line);
  } else if (has_child_paragraph_that_should_be_closed(node, line)) {
    if (f_debug()) printf("closing child paragraph due to empty line\n");
    child->open = 0;
    return determine_writable_node_from_context(node, line);
  } else if (should_determine_context_from_blockquote_child(node, line)) {
    if (f_debug()) printf("determining context from blockquote child\n");
    return determine_writable_node_from_context(child, line);
  } else if (should_split_blockquote(node)) {
    if (f_debug()) printf("splitting a blockquote due to empty lines\n");
    node = node->parent;
    child = add_child_block(node, ASTN_BLOCK_QUOTE, 0, 0);
    return determine_writable_node_from_context(child, line);
  } else if (should_add_default_paragraph_to_blockquote(node, line)) {
    if (f_debug()) printf("adding default paragraph to blockquote\n");
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0);
    return determine_writable_node_from_context(child, line);
  } else if (node->type == ASTN_DOCUMENT) {
    // if we have content, but are still at the document node
    // this is just a paragraph
    if (f_debug()) printf("adding default paragraph\n");
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0);
    return determine_writable_node_from_context(child, line);
  }
  // This context is correct. Use the current node
  return node;
}

void add_line_to_ast(ASTNode root[static 1], char *line[static 1]) {
  size_t line_pos = 0;
  size_t match_len = 0;
  ASTNode *node = root;
  ASTNode *tmp;

  if (f_debug()) {
    printf("------------\n");
    printf("line: '%s'\n", *line);
  }

  node = traverse_to_last_match(node, line, &line_pos, &match_len);
  if (f_debug()) printf("matched node: %s\n", NODE_TYPE_NAMES[node->type]);

  if (is_all_whitespace(*line)) {
    if ((tmp = find_in_edge_of_tree(node, ASTN_BLOCK_QUOTE))) {
      if (f_debug())
        printf("closing %s due to empty line\n", NODE_TYPE_NAMES[tmp->type]);
      tmp->open = 0;
    } else {
      LATE_CONTINUATION_LINES += 1;
      if (line_pos >= 4) {
        LATE_CONTINUATION_CONTENTS =
            str_append(LATE_CONTINUATION_CONTENTS, (*line) + line_pos);
      } else {
        LATE_CONTINUATION_CONTENTS =
            str_append(LATE_CONTINUATION_CONTENTS, "\n");
      }
    }
    return;
  }

  node = handle_new_block_starts(node, line, &line_pos, &match_len);
  node = determine_writable_node_from_context(node, (*line) + line_pos);

  if (node->type != ASTN_THEMATIC_BREAK)
    add_line_to_node(node, (*line) + line_pos);

  // cleanup
  if (array_contains(UNNAPENDABLE_NODES_SIZE, UNAPPENDABLE_NODES, node->type)) {
    node->open = 0;
  }

  reset_late_continuation();

  if (f_debug()) {
    printf("---------------\n");
    print_tree(root, 0);
  }
}

ASTNode *build_block_structure(FILE fd[static 1]) {
  ASTNode *document = ast_create_node(ASTN_DOCUMENT);
  char *line = NULL;
  size_t buff_len = 0;
  LATE_CONTINUATION_CONTENTS = strdup("");
  while (!feof(fd)) {
    if (ferror(fd)) {
      printf("File reading error. Errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }

    getline(&line, &buff_len, fd);
    add_line_to_ast(document, &line);
  }
  free(LATE_CONTINUATION_CONTENTS);
  return document;
}
