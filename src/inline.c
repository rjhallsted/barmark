#include "inline.h"

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "string_mod.h"
#include "util.h"

/*
Parsing thoughts:

Will need to maintain a stack of openers.
When meet a closer for something on the stack, close it. (This may be different
for emphasis and such)

*/

Token *new_token(int unsigned type, size_t start, size_t length) {
  Token *t = malloc(sizeof(Token));
  t->type = type;
  t->start = start;
  t->length = length;
  return t;
}

// TODO: Will end up more abstracted away, but for now this works
Token *next_token(char const line[static 1], size_t line_pos[static 1]) {
  if (f_debug()) {
    printf("next token at char: '%c'\n", line[*line_pos]);
  }
  if (line[*line_pos] == '`') {
    size_t i = 0;
    while (line[(*line_pos) + i] == '`') {
      i++;
    }
    Token *t = new_token(TOKEN_BACKTICKS, *line_pos, i);
    *line_pos += i;
    return t;
  }
  size_t start = *line_pos;
  size_t i = 0;
  while (line[start + i] && line[start + i] != '`') {
    i++;
  }
  if (i > 0) {
    Token *t = new_token(TOKEN_TEXT, start, i);
    *line_pos += i;
    return t;
  } else {
    return NULL;
  }
}

Token **build_token_list(char const line[static 1]) {
  Token **list = malloc(sizeof(Token *));
  list[0] = NULL;
  size_t list_size = 0;
  size_t line_pos = 0;
  Token *token = NULL;
  while ((token = next_token(line, &line_pos))) {
    list[list_size] = token;
    list_size += 1;
    list = realloc(list, sizeof(Token *) * (list_size + 1));
    list[list_size] = NULL;
  }
  return list;
}

void print_token_list(char const line[static 1], Token **list) {
  printf("Tokens:\n\t");
  size_t i = 0;
  while (list[i]) {
    char *s = strndup(line + list[i]->start, list[i]->length);
    printf("%s(\"%s\") -> ", TOKEN_TYPE_NAMES[list[i]->type], s);
    free(s);
    i++;
  }
  printf("\n");
}

void free_token_list(Token **list) {
  size_t i = 0;
  while (list[i]) {
    free(list[i]);
    i++;
  }
  free(list);
}

size_t find_code_span(Token **token_list, size_t start) {
  if (token_list[start]->type != TOKEN_BACKTICKS) {
    return 0;
  }
  size_t i = 1;
  while (token_list[start + i] &&
         (token_list[start + i]->type != TOKEN_BACKTICKS ||
          token_list[start + i]->length != token_list[start]->length)) {
    i++;
  }
  if (token_list[start + i]) {
    return start + i + 1;
  }
  return 0;
}

void convert_line_endings_to_spaces(char str[static 1]) {
  size_t i = 0;
  while (str[i]) {
    if (str[i] == '\n') {
      str[i] = ' ';
    }
    i++;
  }
}

void prepare_code_span_contents(char *contents[static 1]) {
  string_mod_ref ref = make_unmodified_string_mod_ref(contents);
  size_t len = strlen(ref.proposed);
  convert_line_endings_to_spaces(ref.proposed);
  // special starting and trailing spaces trimming
  if (ref.proposed[0] == ' ' && ref.proposed[len - 1] == ' ' &&
      !is_all_whitespace(ref.proposed)) {
    char *new = strndup(ref.proposed + 1, len - 2);
    free(ref.proposed);
    ref.proposed = new;
  }
  commit_string_mod(ref);
}

void prepare_contents(int unsigned node_type, char *contents[static 1]) {
  if (node_type == ASTN_CODE_SPAN) {
    prepare_code_span_contents(contents);
  }
}

ASTNode *new_node_from_tokens(int unsigned type, Token **tokens, size_t start,
                              size_t length, char *line) {
  if (f_debug()) {
    printf("making node of type %s from tokens\n", NODE_TYPE_NAMES[type]);
  }
  size_t str_len = 0;
  for (size_t i = 0; i < length; i++) {
    str_len += tokens[start + i]->length;
  }
  char *contents = malloc(str_len + 1);
  size_t pos = 0;
  for (size_t i = 0; i < length; i++) {
    strncpy(contents + pos, line + tokens[start + i]->start,
            tokens[start + i]->length);
    pos += tokens[start + i]->length;
  }
  contents[str_len] = '\0';
  prepare_contents(type, &contents);

  ASTNode *new_node = ast_create_node(type);
  if (type != ASTN_TEXT) {
    ASTNode *child = ast_create_node(ASTN_TEXT);
    child->contents = contents;
    ast_add_child(new_node, child);
  } else {
    new_node->contents = contents;
  }
  return new_node;
}

void parse_text(ASTNode node[static 1]) {
  if (f_debug()) {
    printf("parsing text on node type %s\n", NODE_TYPE_NAMES[node->type]);
  }

  /*
    Build up list of token positions (token type, start pos, length)
    iterate, handle token based on rules. (backticks sequence of length x
    consumes everything until another backtick sequence of length x, if none
    found, then that no node is created).

    Once you've determined node start and end, if end of previous node is not 1
    before node start, put the middle content into text node.
  */
  // build up list of tokens
  // parse, appending children to this node as you go.
  // once finished, move children of this node to parent (in its place) and
  // remove this node
  Token **token_list = build_token_list(node->contents);
  // TODO: If token list ends in newline, drop newline
  if (f_debug()) {
    print_token_list(node->contents, token_list);
  }
  size_t i = 0;
  size_t next_split_pos = 0;
  size_t split_pos = 0;
  while (token_list[i]) {
    if ((next_split_pos = find_code_span(token_list, i))) {
      if (i > 0) {
        ASTNode *new_node = new_node_from_tokens(
            ASTN_TEXT, token_list, split_pos, i - split_pos, node->contents);
        ast_add_child(node, new_node);
      }
      split_pos = i + 1;
      size_t len = (next_split_pos - 1) - split_pos;
      ASTNode *new_node = new_node_from_tokens(ASTN_CODE_SPAN, token_list,
                                               split_pos, len, node->contents);
      ast_add_child(node, new_node);

      // reset
      split_pos = next_split_pos;
      i = next_split_pos;
    } else {
      i++;
    }
  }
  if (token_list[split_pos]) {
    ASTNode *new_node = new_node_from_tokens(ASTN_TEXT, token_list, split_pos,
                                             i - split_pos, node->contents);
    ast_add_child(node, new_node);
  }
  free(node->contents);
  node->contents = NULL;
  free_token_list(token_list);
}

void parse_inline(ASTNode node[static 1]) {
  if (node->type == ASTN_TEXT) {
    parse_text(node);
  } else if (!array_contains(DONT_PARSE_INLINE_BLOCKS_SIZE,
                             DONT_PARSE_INLINE_BLOCKS, node->type)) {
    for (size_t i = 0; i < node->children_count; i++) {
      parse_inline(node->children[i]);
    }
  }
}
