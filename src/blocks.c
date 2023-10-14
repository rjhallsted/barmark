#include "blocks.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "tab_expand.h"
#include "util.h"

// TODO: Probably rework setext heading handling so that we don't
// require different AST node types to handle them

// TODO: Handle multi-line code blocks
// TODO: Handle backslash escapes of block structural characters

char *LATE_CONTINUATION_CONTENTS = NULL;

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

bool has_open_child(ASTNode node[static 1]) {
  return (node->children_count > 0 &&
          node->children[node->children_count - 1]->open &&
          node->children[node->children_count - 1]->type != ASTN_TEXT);
}

ASTNode *find_in_edge_of_tree(ASTNode node[static 1], unsigned int type) {
  while (node) {
    if (node->type == type) {
      return node;
    }
    node = get_last_child(node);
  }
  return NULL;
}

bool scope_has_late_continuation(ASTNode node[static 1]) {
  while (node) {
    if (node->late_continuation_lines) {
      return true;
    }
    node = node->parent;
  }
  return false;
}

void reset_late_continuation_above_node(ASTNode node[static 1]) {
  free(LATE_CONTINUATION_CONTENTS);
  LATE_CONTINUATION_CONTENTS = strdup("");

  do {
    node->late_continuation_lines = 0;
    node = node->parent;
  } while (node);
}

/**
 * @brief Returns whether a match was found. Takes a pointer to a size_t
 * and sets that to the number of bytes matched against. 0 is a valid value
 * for this.
 */
bool matches_continuation_markers(ASTNode node[static 1], char *line[static 1],
                                  size_t line_pos, size_t match_len[static 1]) {
  if (node->type == ASTN_BLOCK_QUOTE) {  // special case for blockquote
    if ((*line)[line_pos] == '>') {
      tab_expand_ref ref = begin_tab_expand(line, line_pos + 1, 1);
      if (ref.proposed[line_pos + 1] == ' ') {
        if (f_debug()) printf("matching '> '\n");
        commit_tab_expand(ref);
        *match_len = 2;
      } else {
        abandon_tab_expand(ref);
        *match_len = 1;
      }
      return true;
    } else {
      return false;
    }
  } else {  // all other cases
    if (f_debug()) {
      printf("Using normal method\n");
      printf("matching against %s, with space count of %u\n",
             NODE_TYPE_NAMES[node->type], node->cont_spaces);
    }
    tab_expand_ref ref = begin_tab_expand(line, line_pos, node->cont_spaces);
    size_t i = 0;
    while (i < node->cont_spaces && ref.proposed[line_pos + i] == ' ') {
      i++;
    }
    if (i == node->cont_spaces) {
      commit_tab_expand(ref);
      *match_len = i;
      return true;
    }
    abandon_tab_expand(ref);
    return false;
  }
}

bool matches_continuation_markers_with_leading_spaces(
    ASTNode node[static 1], char *line[static 1], size_t line_pos,
    size_t match_len[static 1]) {
  size_t i = 0;
  while ((*line)[line_pos + i] == ' ' && i < 3) {
    i++;
  }
  int res = matches_continuation_markers(node, line, line_pos + i, match_len);
  if (res) {
    *match_len += i;
  }
  return res;
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
  if (node->type == ASTN_CODE_BLOCK && strlen(child->contents) &&
      strlen(LATE_CONTINUATION_CONTENTS)) {
    if (f_debug())
      printf("adding late continuation lines to code block contents\n");
    child->contents = str_append(child->contents, LATE_CONTINUATION_CONTENTS);
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
  size_t res = 0;
  tab_expand_ref t1 = begin_tab_expand(line, line_pos, 1);
  if (t1.proposed[line_pos] == ' ') {
    res = 1;
    tab_expand_ref t2 = begin_tab_expand(&(t1.proposed), line_pos + 1, 1);
    if (t2.proposed[line_pos + 1] == ' ') {
      res = 2;
      tab_expand_ref t3 = begin_tab_expand(&(t2.proposed), line_pos + 2, 1);
      if (t3.proposed[line_pos + 2] == ' ') {
        res = 3;
        commit_tab_expand(t3);
      } else {
        abandon_tab_expand(t3);
      }
      commit_tab_expand(t2);
    } else {
      abandon_tab_expand(t2);
    }
    commit_tab_expand(t1);
  } else {
    abandon_tab_expand(t1);
  }
  return res;
}

size_t matches_code_block(char *line[static 1], size_t line_pos) {
  size_t i = 0;
  tab_expand_ref t1 = begin_tab_expand(line, line_pos, 4);

  while (t1.proposed[line_pos + i] == ' ' && i < 4) {
    i++;
  }
  if (i == 4) {
    commit_tab_expand(t1);
    return i;
  }
  abandon_tab_expand(t1);
  return 0;
}

size_t matches_fenced_code_block(char *line[static 1], size_t line_pos) {
  size_t i = 0;
  tab_expand_ref t1 = begin_tab_expand(line, line_pos, 3);

  while (t1.proposed[line_pos + i] == ' ' && i < 3) {
    i++;
  }
  char c = t1.proposed[line_pos + i];
  size_t fence_start = i;
  if (c != '`' && c != '~') {
    abandon_tab_expand(t1);
    return 0;
  }
  while (t1.proposed[line_pos + i] == c) {
    i++;
  }
  if (i - fence_start >= 3) {
    commit_tab_expand(t1);
    return i;
  }
  abandon_tab_expand(t1);
  return 0;
}

size_t matches_fenced_code_block_close(char *line[static 1], size_t line_pos,
                                       char id_char, long unsigned fence_len) {
  size_t i = 0;
  tab_expand_ref t1 = begin_tab_expand(line, line_pos, 3);

  while (t1.proposed[line_pos + i] == ' ' && i < 3) {
    i++;
  }
  size_t fence_start = i;
  if (t1.proposed[line_pos + i] != id_char) {
    abandon_tab_expand(t1);
    return 0;
  }
  while (t1.proposed[line_pos + i] == id_char) {
    i++;
  }
  if (i - fence_start < fence_len ||
      !is_all_whitespace(t1.proposed + line_pos + i)) {
    abandon_tab_expand(t1);
    return 0;
  }
  commit_tab_expand(t1);
  return i;
}

size_t match_str_then_space(char const str[static 1], char *line[static 1],
                            size_t line_pos) {
  tab_expand_ref t1 = make_unmodified_tab_expand_ref(line);
  size_t i = match_up_to_3_spaces(&(t1.proposed), line_pos);

  if (str_starts_with(t1.proposed + line_pos + i, str)) {
    i += strlen(str);
    expand_existing_ref(&t1, line_pos + i, 1);
    if (t1.proposed[line_pos + i] == ' ') {
      commit_tab_expand(t1);
      return i + 1;
    }
  }
  abandon_tab_expand(t1);
  return 0;
}

size_t matches_unordered_list_opener_with_symbol(char const str[static 1],
                                                 char *line[static 1],
                                                 size_t line_pos) {
  tab_expand_ref t1 = make_unmodified_tab_expand_ref(line);
  size_t res = match_str_then_space(str, &(t1.proposed), line_pos);
  if (res) {
    commit_tab_expand(t1);
    return res;
  } else {
    // try case where we match the symbol, but then line is all whitespace
    res = match_up_to_3_spaces(&(t1.proposed), line_pos);
    if (str_starts_with(t1.proposed + line_pos + res, str)) {
      res += strlen(str);
      if (is_all_whitespace(t1.proposed + line_pos + res)) {
        commit_tab_expand(t1);
        return res;
      }
    }
    abandon_tab_expand(t1);
    return 0;
  }
}

size_t matches_unordered_list_opening(char *line[static 1], size_t line_pos) {
  size_t res = matches_unordered_list_opener_with_symbol("-", line, line_pos);
  if (!res)
    res = matches_unordered_list_opener_with_symbol("*", line, line_pos);
  if (!res)
    res = matches_unordered_list_opener_with_symbol("+", line, line_pos);
  return res;
}

// TODO: Allow in-place tab-expansion on existing refs
size_t matches_ordered_list_opening(char *line[static 1], size_t line_pos) {
  tab_expand_ref t1 = make_unmodified_tab_expand_ref(line);
  size_t i = match_up_to_3_spaces(&(t1.proposed), line_pos);  // leading spaces
  size_t num_start = i;
  // numbers
  while (t1.proposed[line_pos + i] >= '0' && t1.proposed[line_pos + i] <= '9') {
    i++;
  }
  // period
  if ((i - num_start > 9) || i == 0 ||
      (t1.proposed[line_pos + i] != '.' && t1.proposed[line_pos + i] != ')')) {
    abandon_tab_expand(t1);
    return 0;
  }
  i++;
  // trailing space (or line end)
  expand_existing_ref(&t1, line_pos + i, 1);
  if (t1.proposed[line_pos + i] != ' ' && t1.proposed[line_pos + i] != '\n' &&
      t1.proposed[line_pos + i] != '\0') {
    abandon_tab_expand(t1);
    return 0;
  }
  i++;

  if (f_debug()) printf("match_len for OL-LI opening is %zu\n", i + 1);

  commit_tab_expand(t1);
  return i;
}

size_t matches_paragraph_opening(char *line[static 1], size_t line_pos) {
  return (!is_all_whitespace((*line) + line_pos));
}

size_t matches_blockquote_opening(char *line[static 1], size_t line_pos) {
  tab_expand_ref t1 = make_unmodified_tab_expand_ref(line);
  size_t i = match_up_to_3_spaces(&(t1.proposed), line_pos);
  if (t1.proposed[line_pos + i] == '>') {
    i++;
    expand_existing_ref(&t1, line_pos + i, 1);
    if (t1.proposed[line_pos + i] == ' ') {
      i++;
    }
    commit_tab_expand(t1);
    return i;
  }
  abandon_tab_expand(t1);
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
  tab_expand_ref ref = make_unmodified_tab_expand_ref(line);
  size_t i = match_up_to_3_spaces(&(ref.proposed), line_pos);
  char c = ref.proposed[line_pos + i];
  if (c != '*' && c != '-' && c != '_') {
    abandon_tab_expand(ref);
    return 0;
  }
  i++;
  while (is_whitespace(ref.proposed[line_pos + i])) {
    i++;
  }
  if (ref.proposed[line_pos + i] != c) {
    abandon_tab_expand(ref);
    return 0;
  }
  i++;
  while (is_whitespace(ref.proposed[line_pos + i])) {
    i++;
  }
  if (ref.proposed[line_pos + i] != c) {
    abandon_tab_expand(ref);
    return 0;
  }
  i++;
  while (is_whitespace(ref.proposed[line_pos + i]) ||
         ref.proposed[line_pos + i] == c) {
    i++;
  }
  if (ref.proposed[line_pos + i] != '\0') {
    abandon_tab_expand(ref);
    return 0;
  }
  commit_tab_expand(ref);
  return i;
}

size_t matches_setext_h2(char *line[static 1], size_t line_pos) {
  tab_expand_ref ref = make_unmodified_tab_expand_ref(line);
  size_t i = match_up_to_3_spaces(&(ref.proposed), line_pos);
  if (ref.proposed[line_pos + i] != '-') {
    abandon_tab_expand(ref);
    return 0;
  }
  while (ref.proposed[line_pos + i] == '-') {
    i++;
  }
  while (is_whitespace(ref.proposed[line_pos + i])) {
    i++;
  }
  if (ref.proposed[line_pos + i] != '\0') {
    abandon_tab_expand(ref);
    return 0;
  }
  commit_tab_expand(ref);
  return i;
}

size_t matches_setext_h1(char *line[static 1], size_t line_pos) {
  tab_expand_ref ref = make_unmodified_tab_expand_ref(line);
  size_t i = match_up_to_3_spaces(&(ref.proposed), line_pos);
  if (ref.proposed[line_pos + i] != '=') {
    abandon_tab_expand(ref);
    return 0;
  }
  while (ref.proposed[line_pos + i] == '=') {
    i++;
  }
  while (is_whitespace(ref.proposed[line_pos + i])) {
    i++;
  }
  if (ref.proposed[line_pos + i] != '\0') {
    abandon_tab_expand(ref);
    return 0;
  }
  commit_tab_expand(ref);
  return i;
}

/* end matching functions */

void close_leaf_paragraph(ASTNode node[static 1]) {
  ASTNode *descendant = get_deepest_non_text_child(node);
  if (descendant->type == ASTN_PARAGRAPH) {
    descendant->open = 0;
  }
}

bool has_text_as_last_item(ASTNode node[static 1]) {
  ASTNode *child = get_last_child(node);
  return child && child->type == ASTN_TEXT;
}

char *get_node_last_item_contents(ASTNode node[static 1]) {
  ASTNode *child = get_last_child(node);
  if (!child || child->type != ASTN_TEXT) {
    return NULL;
  }
  return child->contents;
}

bool meets_setext_conditions(ASTNode node[static 1]) {
  ASTNode *child = get_last_child(node);
  return (child &&
          (child->type == ASTN_PARAGRAPH || child->type == ASTN_TEXT) &&
          !scope_has_late_continuation(node));
}

bool meets_code_block_conditions(ASTNode node[static 1]) {
  ASTNode *deepest = get_deepest_non_text_child(node);

  if (f_debug()) {
    printf("checking code block conditions:\n");
    printf("  node->type:    %s\n", NODE_TYPE_NAMES[node->type]);
    printf("  deepest->type: %s\n", NODE_TYPE_NAMES[deepest->type]);
  }

  if (!scope_has_late_continuation(node) &&
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
  } else if ((*match_len = matches_fenced_code_block(line, line_pos))) {
    return ASTN_FENCED_CODE_BLOCK;
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

ASTNode *add_child_block_with_cont_spaces(ASTNode node[static 1],
                                          int unsigned node_type,
                                          int unsigned cont_spaces) {
  ASTNode *child = ast_create_node(node_type);
  child->cont_spaces = cont_spaces;
  ast_add_child(node, child);
  return child;
}

/* Returns a pointer to the deepest added child.
Contains all logic for block creation.
*/
ASTNode *add_child_block(ASTNode node[static 1], int unsigned node_type,
                         size_t opener_match_len, char id_char,
                         long unsigned reference_num) {
  ASTNode *child;

  if (node_type == ASTN_CODE_BLOCK) {
    return add_child_block_with_cont_spaces(node, ASTN_CODE_BLOCK, 4);
  } else if (node_type == ASTN_FENCED_CODE_BLOCK) {
    child = ast_create_node(ASTN_FENCED_CODE_BLOCK);
    child->options = make_node_options(id_char, reference_num);
    ast_add_child(node, child);
    return child;
  } else if (node_type == ASTN_UNORDERED_LIST_ITEM &&
             node->type != ASTN_UNORDERED_LIST) {
    // TODO: extract to new_unorderd_list_node;
    child = ast_create_node(ASTN_UNORDERED_LIST);
    child->options = make_node_options(id_char, reference_num);
    ast_add_child(node, child);
    return add_child_block(child, ASTN_UNORDERED_LIST_ITEM, opener_match_len, 0,
                           0);
  } else if (node_type == ASTN_ORDERED_LIST_ITEM &&
             node->type != ASTN_ORDERED_LIST) {
    // TODO: extract to new_orderd_list_node;
    child = ast_create_node(ASTN_ORDERED_LIST);
    child->options = make_node_options(id_char, reference_num);
    ast_add_child(node, child);
    return add_child_block(child, ASTN_ORDERED_LIST_ITEM, opener_match_len, 0,
                           0);
  } else if (node_type == ASTN_UNORDERED_LIST_ITEM &&
             node->type == ASTN_UNORDERED_LIST) {
    return child = add_child_block_with_cont_spaces(
               node, ASTN_UNORDERED_LIST_ITEM, opener_match_len);
  } else if (node_type == ASTN_ORDERED_LIST_ITEM &&
             node->type == ASTN_ORDERED_LIST) {
    if (f_debug())
      printf("match len while add OL-LI is %zu\n", opener_match_len);
    return child = add_child_block_with_cont_spaces(
               node, ASTN_ORDERED_LIST_ITEM, opener_match_len);
  } else if (node_type == ASTN_BLOCK_QUOTE) {
    child = ast_create_node(ASTN_BLOCK_QUOTE);
    ast_add_child(node, child);
    return child;
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
    printf("%s%s-%s [%u] (%zu)\n", indent, NODE_TYPE_NAMES[node->type],
           node->options->wide ? "wide" : "tight", node->cont_spaces,
           node->late_continuation_lines);
  } else {
    printf("%s%s [%u] (%zu)\n", indent, NODE_TYPE_NAMES[node->type],
           node->cont_spaces, node->late_continuation_lines);
  }

  if (node->contents) {
    printf("%s+>%s\n", indent, node->contents);
  }
  for (size_t i = 0; i < node->children_count; i++) {
    print_tree(node->children[i], level + 1);
  }
  free(indent);
}

/*
Returns the id char if it exists, otherwise 0.
If the id char is found, it will also set fence_len.
*/
char find_fenced_code_block_details(char line[static 1],
                                    long unsigned fence_len[static 1]) {
  size_t i = 0;
  while (line[i] && line[i] != '`' && line[i] != '~') {
    i++;
  }
  if (!line[i]) {
    return 0;
  }
  char id_char = line[i];
  size_t fence_start = i;
  while (line[i] == id_char) {
    i++;
  }
  *fence_len = i - fence_start;
  return id_char;
}

char find_unordered_list_char(char line[static 1]) {
  size_t i = 0;
  while (line[i] && line[i] != '-' && line[i] != '*') {
    i++;
  }
  if (line[i]) {
    return line[i];
  }
  return 0;
}

char find_ordered_list_char(char line[static 1]) {
  size_t i = 0;
  while (line[i] == ' ') {
    i++;
  }
  while (line[i] >= '0' && line[i] <= '9') {
    i++;
  }
  if (line[i]) {
    return line[i];
  }
  return 0;
}

long unsigned find_starting_num(char line[static 1]) {
  char *end_ptr = line + strlen(line);
  return strtoul(line, &end_ptr, 10);
}

void add_late_cont_contents_to_code_block(ASTNode node[static 1]) {
  ASTNode *text = get_last_child(node);
  if (strlen(LATE_CONTINUATION_CONTENTS)) {
    text->contents = str_append(text->contents, LATE_CONTINUATION_CONTENTS);
  }
}

bool should_add_to_parent_instead(ASTNode node[static 1],
                                  int unsigned new_node_type, char list_char) {
  return ((node->type == ASTN_UNORDERED_LIST &&
           new_node_type != ASTN_UNORDERED_LIST_ITEM) ||
          (node->type == ASTN_UNORDERED_LIST &&
           new_node_type == ASTN_UNORDERED_LIST_ITEM &&
           node->options->id_char != list_char) ||
          (node->type == ASTN_ORDERED_LIST &&
           new_node_type == ASTN_ORDERED_LIST_ITEM &&
           node->options->id_char != list_char) ||
          (node->type == ASTN_ORDERED_LIST &&
           new_node_type != ASTN_ORDERED_LIST_ITEM));
}

ASTNode *traverse_to_last_match(ASTNode node[static 1], char *line[static 1],
                                size_t line_pos[static 1],
                                size_t match_len[static 1]) {
  ASTNode *child;
  // traverse to last matching node
  while ((*line)[*line_pos] && has_open_child(node)) {
    if (f_debug()) {
      printf("matching against %s\n",
             NODE_TYPE_NAMES[get_last_child(node)->type]);
      printf("remaining line is: '%s'\n", (*line) + (*line_pos));
    }
    child = get_last_child(node);
    // TODO: Figure out why I have to check both versions of cont markers here
    // instead of just the leading spaces version
    if (child->type != ASTN_PARAGRAPH &&
        (matches_continuation_markers(child, line, *line_pos, match_len) ||
         matches_continuation_markers_with_leading_spaces(
             child, line, *line_pos, match_len))) {
      node = child;
      *line_pos += *match_len;
    } else {
      break;
    }
  }
  return node;
}

bool is_leaf_only_node(int unsigned type) {
  return array_contains(LEAF_ONLY_NODES_SIZE, LEAF_ONLY_NODES, type);
}

void widen_list(ASTNode node[static 1]) {
  node->options->wide = true;
  // fix existing list items
  for (size_t i = 0; i < node->children_count; i++) {
    convert_texts_to_paragraphs(node->children[i]);
  }
}

bool is_new_item_in_wide_list(ASTNode node[static 1]) {
  return (node->type == ASTN_UNORDERED_LIST_ITEM ||
          node->type == ASTN_ORDERED_LIST_ITEM) &&
         node->parent->options->wide;
}

bool is_new_line_in_item_in_narrow_list(ASTNode node[static 1]) {
  ASTNode *child = get_last_child(node);
  return (has_open_child(node) &&
          (child->type == ASTN_UNORDERED_LIST ||
           child->type == ASTN_ORDERED_LIST) &&
          has_open_child(child));
}

bool has_continuable_paragraph(ASTNode node[static 1]) {
  ASTNode *child = get_last_child(node);
  return has_open_child(node) && child->type == ASTN_PARAGRAPH;
}

bool should_determine_context_from_child_blockquote(ASTNode node[static 1]) {
  ASTNode *child = get_last_child(node);
  return has_open_child(node) && child->type == ASTN_BLOCK_QUOTE &&
         has_open_child(child);
}

bool should_close_blockquote(ASTNode node[static 1]) {
  return has_open_child(node) &&
         find_in_edge_of_tree(get_last_child(node), ASTN_BLOCK_QUOTE);
}

bool should_add_paragraph_to_blockquote(ASTNode node[static 1]) {
  return node->type == ASTN_BLOCK_QUOTE && !has_open_child(node);
}

ASTNode *determine_writable_node_from_context(ASTNode node[static 1]) {
  /* logic to determine where to add line based on current node and context.
    Tread carefully, as predicate order matters.
   */

  //////////
  if (f_debug()) {
    printf("determining node from context on %s\n",
           NODE_TYPE_NAMES[node->type]);
  }

  ASTNode *child = get_last_child(node);

  if (is_new_item_in_wide_list(node)) {
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0, 0);
    return determine_writable_node_from_context(child);
  } else if (is_new_line_in_item_in_narrow_list(node)) {
    if (f_debug())
      printf("Adding trailing line to last item in unordered list.\n");
    return determine_writable_node_from_context(get_last_child(child));
  } else if (has_continuable_paragraph(node)) {
    if (f_debug()) printf("using continuable child paragraph for context\n");
    return determine_writable_node_from_context(child);
  } else if (should_determine_context_from_child_blockquote(node)) {
    if (f_debug()) printf("determining context from child blockquote\n");
    return determine_writable_node_from_context(child);
  } else if (should_add_paragraph_to_blockquote(node)) {
    if (f_debug()) printf("adding default paragraph to blockquote\n");
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0, 0);
    return determine_writable_node_from_context(child);
  } else if (node->type == ASTN_DOCUMENT) {
    // if we have content, but are still at the document node
    // this is just a paragraph
    if (f_debug()) printf("adding default paragraph\n");
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0, 0);
    return determine_writable_node_from_context(child);
  }
  // This context is correct. Use the current node
  return node;
}

/* modifies the structure depending on the scenario, and returns the updated
 * contextual node */
ASTNode *handle_late_continuation_for_new_blocks(ASTNode node[static 1]) {
  if (f_debug()) printf("handling late continuation for new blocks\n");

  if ((node->type == ASTN_ORDERED_LIST_ITEM ||
       node->type == ASTN_UNORDERED_LIST_ITEM) &&
      node->parent->late_continuation_lines) {
    if (f_debug()) {
      printf("widening list due to late continuation (from new list item)\n");
    }
    widen_list(node->parent);
    reset_late_continuation_above_node(node);
  } else if (node->parent &&
             (node->parent->type == ASTN_ORDERED_LIST_ITEM ||
              node->parent->type == ASTN_UNORDERED_LIST_ITEM) &&
             !(node->parent->parent->options->wide)) {
    if (f_debug()) {
      printf(
          "widening list due to late continuation (from new item in existing "
          "list item)\n");
    }
    widen_list(node->parent->parent);
    reset_late_continuation_above_node(node);
  }

  return node;
}

ASTNode *handle_late_continuation(ASTNode node[static 1]) {
  if (f_debug())
    printf("enacting late continuation effects on node %s\n",
           NODE_TYPE_NAMES[node->type]);

  if ((node->type == ASTN_ORDERED_LIST_ITEM ||
       node->type == ASTN_UNORDERED_LIST_ITEM) &&
      node->parent->late_continuation_lines && !node->children_count) {
    // list items are only late continuable if the first or second lines have
    // contents, in which case there would be children by now
    if (f_debug()) {
      printf("closing list due to unusable late continuation lines");
    }
    node->parent->open = false;
    node = node->parent->parent;
  } else if (node->type == ASTN_ORDERED_LIST_ITEM ||
             node->type == ASTN_UNORDERED_LIST_ITEM) {
    if (f_debug()) {
      printf("widening list due to late continuation (from list item)\n");
    }
    ASTNode *list = node->parent;
    if (!node->children_count) {
      // no children indicates this is the first line in list item, and we
      // should widen the parent list
      list = node->parent->parent->parent;
    }
    widen_list(list);
  } else if (node->type == ASTN_UNORDERED_LIST ||
             node->type == ASTN_ORDERED_LIST) {
    if (f_debug()) {
      printf("closing list due to empty lines\n");
    }
    node->open = false;
    node = node->parent;
    if (node->type == ASTN_ORDERED_LIST_ITEM ||
        node->type == ASTN_UNORDERED_LIST_ITEM) {
      widen_list(node->parent);
    }
  } else if (node->type == ASTN_BLOCK_QUOTE && !node->late_continuation_lines &&
             node->children_count) {
    /*
    Checking late continuation lines to make sure they're not on _this_
    blockquote, which would indicate we had something like: > foo
    >
    > baz
    checking children count as a means of confirming this isn't a new blockquote
    just after a late continuation
    */
    if (f_debug()) {
      printf("splitting blockquote due to late continuation lines\n");
    }
    reset_late_continuation_above_node(node);
    node = add_child_block(node->parent, ASTN_BLOCK_QUOTE, 0, 0, 0);
  } else if (should_close_blockquote(node)) {
    ASTNode *descendant = find_in_edge_of_tree(node, ASTN_BLOCK_QUOTE);
    descendant->open = false;
  } else if (node->type == ASTN_CODE_BLOCK && node->children_count) {
    // we should only add contents to existing code blocks, not new ones, hence
    // the child check
    add_late_cont_contents_to_code_block(node);
  }
  reset_late_continuation_above_node(node);
  return node;
}

bool meets_paragraph_interruption_by_ol_criteria(ASTNode node[static 1],
                                                 long unsigned reference_num) {
  if (!has_open_child(node) || get_last_child(node)->type != ASTN_PARAGRAPH ||
      scope_has_late_continuation(node)) {
    return true;
  }
  if (reference_num != 1) {
    return false;
  }
  return true;
}

ASTNode *handle_new_block_starts(ASTNode node[static 1], char *line[static 1],
                                 size_t line_pos[static 1],
                                 size_t match_len[static 1]) {
  char id_char = 0;
  long unsigned reference_num = 0;
  int unsigned node_type;

  // If a block start exists, create it, then keep checking for more.
  // Intelligently rework blocks if new block needs to be moved to parent
  while ((node_type = block_start_type(line, *line_pos, node, match_len)) &&
         !is_leaf_only_node(node->type)) {
    if (node_type == ASTN_UNORDERED_LIST_ITEM) {
      id_char = find_unordered_list_char((*line) + (*line_pos));
    }
    if (node_type == ASTN_ORDERED_LIST_ITEM) {
      reference_num = find_starting_num((*line) + (*line_pos));
      if (!meets_paragraph_interruption_by_ol_criteria(node, reference_num)) {
        break;
      }
      id_char = find_ordered_list_char((*line) + (*line_pos));
    }
    if (node_type == ASTN_FENCED_CODE_BLOCK) {
      id_char =
          find_fenced_code_block_details((*line) + (*line_pos), &reference_num);
    }
    *line_pos += *match_len;

    if (f_debug())
      printf("block start node_type: %s\n", NODE_TYPE_NAMES[node_type]);

    // enforce cases of required child types
    if (should_add_to_parent_instead(node, node_type, id_char)) {
      node = node->parent;
    }
    node = add_child_block(node, node_type, *match_len, id_char, reference_num);
    if (scope_has_late_continuation(node)) {
      node = handle_late_continuation_for_new_blocks(node);
    }
    if (f_debug()) printf("new block start: %s\n", NODE_TYPE_NAMES[node->type]);
  }

  // cleanup list items so that continuation markers use all leading spaces.
  if ((node->type == ASTN_UNORDERED_LIST_ITEM ||
       node->type == ASTN_ORDERED_LIST_ITEM) &&
      !is_all_whitespace((*line) + (*line_pos))) {
    int unsigned diff = match_up_to_3_spaces(line, *line_pos);
    if (f_debug()) {
      printf("cleaning up list item cont markers\n");
      printf("line is '%s'", (*line) + (*line_pos));
      printf("diff is %u\n", diff);
    }
    node->cont_spaces += diff;
    *line_pos += diff;
  }

  return node;
}

void add_line_to_ast(ASTNode root[static 1], char *line[static 1]) {
  size_t line_pos = 0;
  size_t match_len = 0;
  ASTNode *node = root;
  // ASTNode *tmp;

  if (f_debug()) {
    printf("------------\n");
    printf("line: '%s'\n", *line);
  }

  node = traverse_to_last_match(node, line, &line_pos, &match_len);
  if (f_debug()) {
    printf("matched node: %s\n", NODE_TYPE_NAMES[node->type]);
    printf("line now is: '%s'\n", (*line) + line_pos);
  }

  if (is_all_whitespace((*line) + line_pos)) {
    if (f_debug()) {
      printf("Found late continuation line on node %s\n",
             NODE_TYPE_NAMES[node->type]);
    }
    node->late_continuation_lines += 1;
    close_leaf_paragraph(node);
    if (line_pos >= 4) {
      LATE_CONTINUATION_CONTENTS =
          str_append(LATE_CONTINUATION_CONTENTS, (*line) + line_pos);
    } else {
      LATE_CONTINUATION_CONTENTS = str_append(LATE_CONTINUATION_CONTENTS, "\n");
    }
    if (f_debug()) print_tree(root, 0);
    return;
  }
  if (node->type == ASTN_FENCED_CODE_BLOCK &&
      matches_fenced_code_block_close(line, line_pos, node->options->id_char,
                                      node->options->reference_num)) {
    node->open = false;
    if (scope_has_late_continuation(node)) {
      add_late_cont_contents_to_code_block(node);
      reset_late_continuation_above_node(node);
    }
    return;
  }

  node = handle_new_block_starts(node, line, &line_pos, &match_len);
  if (scope_has_late_continuation(node)) {
    node = handle_late_continuation(node);
  }
  if (node->type == ASTN_UNORDERED_LIST || node->type == ASTN_ORDERED_LIST) {
    // resulting node shouldn't ever be the list instead of a list item, so
    // use parent instead
    node = node->parent;
  }
  if (f_debug()) {
    printf("line: '%s'\n", (*line) + line_pos);
  }

  if (node->type != ASTN_THEMATIC_BREAK &&
      !is_all_whitespace((*line) + line_pos)) {
    node = determine_writable_node_from_context(node);
    add_line_to_node(node, (*line) + line_pos);
  }

  // cleanup
  if (array_contains(UNNAPENDABLE_NODES_SIZE, UNAPPENDABLE_NODES, node->type)) {
    node->open = false;
  }

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
    free(line);
    line = NULL;
  }
  free(LATE_CONTINUATION_CONTENTS);
  return document;
}
