#include "blocks.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "util.h"

// TODO: Probably rework setext heading handling so that we don't
// require different AST node types to handle them

/* Controls whether a late continuation is possible. Gets set to 1
when an empty line is encountered, and set to 0 otherwise. */
unsigned int LATE_CONTINUATION_LINES = 0;

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

void add_line_to_node(ASTNode *node, char *line) {
  if (f_debug()) {
    printf("adding line to %s\n", NODE_TYPE_NAMES[node->type]);
  }

  if (node->contents == NULL) {
    node->contents = strdup("");
  }
  // should only apply to code blocks
  for (unsigned int i = 0; i < LATE_CONTINUATION_LINES; i++) {
    node->contents = str_append(node->contents, "\n");
  }
  node->contents = str_append(node->contents, line);
}

int is_whitespace(char c) { return (c == ' ' || c == '\t' || c == '\n'); }

int is_all_whitespace(const char *line) {
  size_t i = 0;
  while (is_whitespace(line[i])) {
    i++;
  }
  return (line[i] == '\0') ? 1 : 0;
}

/* matching functions */
/*
 * these return 0 if not matched. If matched, return the number
 * of bytes used in match.
 *
 * copy line and tab expand with required lookahead. If match,
 * replace original line with tab expanded one.
 */

size_t matches_code_block(char **line, size_t line_pos) {
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
  free(line_ref);
  return 0;
}

// TODO: Rework to make knowledgeable of indent level?
size_t matches_list_opening(char **line, size_t line_pos) {
  size_t res1 = match_str_then_space("-", line, line_pos);
  if (res1) {
    return res1;
  } else {
    return match_str_then_space("*", line, line_pos);
  }
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
  free(line_ref);
  return 0;
}

size_t matches_atx_header(char **line, size_t line_pos,
                          unsigned int heading_level) {
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

size_t matches_h1_opening(char **line, size_t line_pos) {
  return matches_atx_header(line, line_pos, 1);
}

size_t matches_h2_opening(char **line, size_t line_pos) {
  return matches_atx_header(line, line_pos, 2);
}

size_t matches_h3_opening(char **line, size_t line_pos) {
  return matches_atx_header(line, line_pos, 3);
}

size_t matches_h4_opening(char **line, size_t line_pos) {
  return matches_atx_header(line, line_pos, 4);
}

size_t matches_h5_opening(char **line, size_t line_pos) {
  return matches_atx_header(line, line_pos, 5);
}

size_t matches_h6_opening(char **line, size_t line_pos) {
  return matches_atx_header(line, line_pos, 6);
}

size_t matches_thematic_break(char **line, size_t line_pos) {
  size_t i = 0;
  char *line_ref = strdup(*line);
  char c;

  tab_expand(&line_ref, line_pos, 3);
  while (i < 3 && line_ref[line_pos + i] == ' ') {
    i++;
  }
  if (line_ref[line_pos + i] == '*' || line_ref[line_pos + i] == '-' ||
      line_ref[line_pos + i] == '_') {
    c = line_ref[line_pos + i];
    i += 1;
  } else {
    free(line_ref);
    return 0;
  }
  while (is_whitespace(line_ref[line_pos + i])) {
    i++;
  }
  if (line_ref[line_pos + i] == c) {
    i++;
  } else {
    free(line_ref);
    return 0;
  }
  while (is_whitespace(line_ref[line_pos + i])) {
    i++;
  }
  if (line_ref[line_pos + i] == c) {
    i++;
  } else {
    free(line_ref);
    return 0;
  }
  while (is_whitespace(line_ref[line_pos + i]) || line_ref[line_pos + i] == c) {
    i++;
  }
  if (line_ref[line_pos + i] == '\0') {
    free(*line);
    *line = line_ref;
    return i;
  } else {
    free(line_ref);
    return 0;
  }
}

size_t matches_setext_h2(char **line, size_t line_pos) {
  size_t i = 0;
  char *line_ref = strdup(*line);

  tab_expand(&line_ref, line_pos, 3);
  while (i < 3 && line_ref[line_pos + i] == ' ') {
    i++;
  }
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

size_t matches_setext_h1(char **line, size_t line_pos) {
  size_t i = 0;
  char *line_ref = strdup(*line);

  tab_expand(&line_ref, line_pos, 3);
  while (i < 3 && line_ref[line_pos + i] == ' ') {
    i++;
  }
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

void close_descendent_blocks(ASTNode *node) {
  node->open = 0;
  if (node->children_count > 0) {
    close_descendent_blocks(node->children[node->children_count - 1]);
  }
}

/**
 * @brief Get the last child node. Returns NULL if no children exist.
 *
 * @param node
 * @return ASTNode*
 */
ASTNode *get_last_child(ASTNode *node) {
  if (node->children_count == 0) {
    return NULL;
  }
  return node->children[node->children_count - 1];
}

/***
 * Returns 0 if no block start is found.
 */
int block_start_type(char **line, size_t line_pos, ASTNode *current_node,
                     size_t *match_len) {
  ASTNode *child = get_last_child(current_node);
  if (f_debug()) {
    printf("line: '%s'\n", (*line) + line_pos);
    printf("current_node_type: %s\n", NODE_TYPE_NAMES[current_node->type]);
    if (child) {
      printf("child node type: %s\n", NODE_TYPE_NAMES[child->type]);
    }
  }

  if ((*match_len = matches_code_block(line, line_pos))) {
    return ASTN_CODE_BLOCK;
  } else if (child && child->type == ASTN_PARAGRAPH &&
             !LATE_CONTINUATION_LINES &&
             (*match_len = matches_setext_h2(line, line_pos))) {
    return ASTN_SETEXT_H2;
  } else if (child && child->type == ASTN_PARAGRAPH &&
             !LATE_CONTINUATION_LINES &&
             (*match_len = matches_setext_h1(line, line_pos))) {
    return ASTN_SETEXT_H1;
  } else if ((*match_len = matches_thematic_break(line, line_pos))) {
    return ASTN_THEMATIC_BREAK;
  } else if ((*match_len = matches_list_opening(line, line_pos))) {
    return ASTN_UNORDERED_LIST_ITEM;
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
ASTNode *is_block_end(ASTNode *node, const char *line) {
  if (node->type == ASTN_PARAGRAPH && is_all_whitespace(line)) {
    return node;
  }
  if (node->children_count == 0) {
    return NULL;
  }
  return is_block_end(node->children[node->children_count - 1], line);
}

/* returns a pointer to the child */
ASTNode *add_child_block_with_cont_markers(ASTNode *node,
                                           unsigned int node_type,
                                           char *cont_markers) {
  ASTNode *child = ast_create_node(node_type);
  child->cont_markers = cont_markers;
  ast_add_child(node, child);
  return child;
}

/* Returns a pointer to the deepest added child.
Contains all logic for block creation.
*/
ASTNode *add_child_block(ASTNode *node, unsigned int node_type,
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
  } else if (node_type == ASTN_UNORDERED_LIST_ITEM &&
             node->type == ASTN_UNORDERED_LIST) {
    return child = add_child_block_with_cont_markers(
               node, ASTN_UNORDERED_LIST_ITEM, repeat_x(' ', opener_match_len));
  } else if (node_type == ASTN_BLOCK_QUOTE) {
    return add_child_block_with_cont_markers(node, ASTN_BLOCK_QUOTE,
                                             strdup(">"));
  } else if ((node_type == ASTN_SETEXT_H1 || node_type == ASTN_SETEXT_H2) &&
             (child = get_last_child(node))) {
    // Instead of adding a new child, for setext headings we just change the
    // type of its "sibling"
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
    printf("BAD CHILD BLOCK TYPE %u. CANT ADD IT\n", node_type);
    exit(EXIT_FAILURE);
  }
}

void print_tree(ASTNode *node, size_t level) {
  char *indent = repeat_x(' ', level * 2);
  if (node->options)
    printf("%s%s-%s\n", indent, NODE_TYPE_NAMES[node->type],
           node->options->wide ? "wide" : "tight");
  else
    printf("%s%s\n", indent, NODE_TYPE_NAMES[node->type]);
  if (node->contents) {
    printf("%s->%s\n", indent, node->contents);
  }
  for (size_t i = 0; i < node->children_count; i++) {
    print_tree(node->children[i], level + 1);
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

char find_list_char(char *line) {
  size_t i = 0;
  while (line[i] && line[i] != '-' && line[i] != '*') {
    i++;
  }
  if (line[i]) {
    return line[i];
  }
  return 0;
}

unsigned int meets_req_nl_after_paragraph_rule(ASTNode *node,
                                               unsigned int new_node_type) {
  // certain blocks require a newline after paragraphs.
  return (array_contains(REQ_NL_AFTER_PARAGRAPH_NODES,
                         REQ_NL_AFTER_PARAGRAPH_NODES_SIZE, new_node_type) &&
          node->children_count > 0 &&
          node->children[node->children_count - 1]->type == ASTN_PARAGRAPH &&
          !LATE_CONTINUATION_LINES);
}

unsigned int should_add_to_parent_instead(ASTNode *node,
                                          unsigned int new_node_type,
                                          char list_char) {
  return ((node->type == ASTN_UNORDERED_LIST &&
           new_node_type != ASTN_UNORDERED_LIST_ITEM) ||
          (node->type == ASTN_UNORDERED_LIST &&
           new_node_type == ASTN_UNORDERED_LIST_ITEM && node->options &&
           node->options->marker != list_char));
}

unsigned int has_open_child(ASTNode *node) {
  return (node->children_count > 0 &&
          node->children[node->children_count - 1]->open);
}

ASTNode *traverse_to_last_match(ASTNode *node, char **line, size_t *line_pos,
                                size_t *match_len) {
  // traverse to last matching node
  while ((*line)[*line_pos] && has_open_child(node)) {
    tab_expand(line, *line_pos, 4);
    if (f_debug()) {
      printf("matching against %s\n",
             NODE_TYPE_NAMES[get_last_child(node)->type]);
    }
    if (get_last_child(node)->type == ASTN_PARAGRAPH ||
        !matches_continuation_markers(get_last_child(node),
                                      (*line) + (*line_pos), match_len)) {
      break;
    }
    node = get_last_child(node);
    *line_pos += *match_len;
  }
  return node;
}

ASTNode *handle_new_block_starts(ASTNode *node, char **line, size_t *line_pos,
                                 size_t *match_len) {
  char list_char = 0;
  unsigned int node_type;

  // If a block start exists, create it, then keep checking for more.
  // Intelligently rework blocks if new block needs to be moved to parent
  while ((node_type = block_start_type(line, *line_pos, node, match_len)) &&
         !meets_req_nl_after_paragraph_rule(node, node_type) &&
         !array_contains(LEAF_ONLY_NODES, LEAF_ONLY_NODES_SIZE, node->type)) {
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
  return node;
}

void widen_list(ASTNode *node) {
  node->options->wide = 1;
  // fix existing list items
  for (size_t i = 0; i < node->children_count; i++) {
    move_contents_to_child_paragraph(node->children[i]);
  }
}

void swap_nodes(ASTNode *a, ASTNode *b) {
  ASTNode tmp = *a;
  *a = *b;
  *b = tmp;
}

ASTNode *determine_writable_node_from_context(ASTNode *node) {
  /* logic to determine where to add line based on current node and context */

  /* actual cases:
  - have continuation lines, and node or previous node is unordered list. (makes
  list not tight, and fix previous items)
  - has continuable paragraph && no late continuation lines
  - has continuable blockquote
  - with late continuation lines and is continuable block
  - node type is document and the above are not true
  - otherwise this node is correct
  */

  // TODO: Refactor "wide" list detection. Current version is too messy

  //////////
  if (f_debug()) {
    printf("determining node from context on %s\n",
           NODE_TYPE_NAMES[node->type]);
  }

  ASTNode *child = get_last_child(node);

  // if (node->parent && node->parent->type == ASTN_UNORDERED_LIST_ITEM &&
  if (node->type == ASTN_UNORDERED_LIST_ITEM && !has_open_child(node) &&
      LATE_CONTINUATION_LINES && !node->parent->options->wide &&
      node != node->parent->children[0]) {
    // detect that we should convert to a wide list
    widen_list(node->parent);
    if (f_debug())
      printf("resetting LATE_CONTINUATION_LINES because of wide list\n");
    LATE_CONTINUATION_LINES = 0;
    // have the next case handle the new item now that the list state is fixed
    return determine_writable_node_from_context(node);
  } else if (node->type == ASTN_UNORDERED_LIST_ITEM && !has_open_child(node) &&
             node->parent->options->wide) {
    // fix up new wide list items;
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0);
    // Late continuation lines can be ignored in wide lists
    if (f_debug())
      printf("resetting LATE_CONTINUATION_LINES because of wide list\n");
    LATE_CONTINUATION_LINES = 0;
    return determine_writable_node_from_context(child);
  } else if (node->parent && node->parent->type == ASTN_UNORDERED_LIST_ITEM &&
             !node->parent->parent->options->wide && node->parent->contents) {
    // new item list item that is not yet wide
    if (f_debug()) printf("widening list as child of list item\n");
    // widen list (which adds another item to the parent, then swap the
    // children)
    widen_list(node->parent->parent);
    swap_nodes(node, node->parent->children[1]);
    node = node->parent->children[1];
    // this is the first line in this block (otherwise we'd be in wide mode
    // already) so can ignore late continuation lines
    LATE_CONTINUATION_LINES = 0;
    return determine_writable_node_from_context(node);
  } else if (has_open_child(node) && child->type == ASTN_PARAGRAPH &&
             !LATE_CONTINUATION_LINES) {
    // case where we can continue a paragraph
    return determine_writable_node_from_context(child);
  } else if (has_open_child(node) && child->type == ASTN_BLOCK_QUOTE) {
    return determine_writable_node_from_context(child);
  } else if (LATE_CONTINUATION_LINES &&
             (node->type == ASTN_BLOCK_QUOTE ||
              node->type == ASTN_UNORDERED_LIST_ITEM)) {
    // continuable blocks whose content we can convert to paragraphs, and add
    // another paragraph for this line
    if (f_debug()) printf("converting contents to paragraphs\n");
    move_contents_to_child_paragraph(node);
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0);
    return determine_writable_node_from_context(child);
  } else if (node->type == ASTN_BLOCK_QUOTE && !has_open_child(node)) {
    if (f_debug()) printf("adding default paragraph to blockquote\n");
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0);
    return determine_writable_node_from_context(child);
  } else if (node->type == ASTN_DOCUMENT) {
    // if we have content, but are still at the document node
    // this is just a paragraph
    if (f_debug()) printf("adding default paragraph\n");
    child = add_child_block(node, ASTN_PARAGRAPH, 0, 0);
    return determine_writable_node_from_context(child);
  }
  return node;
}

void add_line_to_ast(ASTNode *root, char **line) {
  size_t line_pos = 0;
  size_t match_len = 0;
  ASTNode *node = root;

  if (is_all_whitespace(*line)) {
    LATE_CONTINUATION_LINES += 1;
    return;
  }

  if (f_debug()) {
    printf("------------\n");
    printf("line: '%s'\n", *line);
  }

  node = traverse_to_last_match(node, line, &line_pos, &match_len);
  if (f_debug()) printf("matched node: %s\n", NODE_TYPE_NAMES[node->type]);

  node = handle_new_block_starts(node, line, &line_pos, &match_len);
  node = determine_writable_node_from_context(node);

  if (node->type != ASTN_THEMATIC_BREAK)
    add_line_to_node(node, (*line) + line_pos);

  // cleanup
  if (array_contains(UNAPPENDABLE_NODES, UNNAPENDABLE_NODES_SIZE, node->type)) {
    node->open = 0;
  }

  LATE_CONTINUATION_LINES = 0;

  if (f_debug()) {
    printf("---------------\n");
    print_tree(root, 0);
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
    add_line_to_ast(document, &line);
  }
  return document;
}
