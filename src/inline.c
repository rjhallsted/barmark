#include "inline.h"

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "grammar.h"
#include "string_mod.h"
#include "utf8.h"
#include "util.h"

/*
Parsing thoughts:

Will need to maintain a stack of openers.
When meet a closer for something on the stack, close it. (This may be different
for emphasis and such)

*/

// TODO: Begin using slices everywhere I currently pass start & length;

Token *new_token(int unsigned type, slice token_slice) {
  Token *t = malloc(sizeof(Token));
  t->type = type;
  t->start = token_slice.start;
  t->length = token_slice.len;
  return t;
}

bool codepoint_collection_contains(codepoint_collection coll, codepoint cp) {
  for (size_t i = 0; i < coll.length; i++) {
    if (cp >= coll.ranges[i].min && cp <= coll.ranges[i].max) {
      return true;
    }
  }
  return false;
}

Token *get_token_of_type(int unsigned token_type, codepoint_collection coll,
                         char const line[static 1], size_t line_pos[static 1]) {
  size_t i = 0;
  int unsigned char_len = 0;
  if (codepoint_collection_contains(
          coll, utf8_char((line + *line_pos + i), &char_len))) {
    i += char_len;
  }
  if (coll.runnable) {
    while (codepoint_collection_contains(
        coll, utf8_char((line + *line_pos + i), &char_len))) {
      i += char_len;
    }
  }
  if (i == 0) {
    return NULL;
  }
  Token *t = new_token(token_type, (slice){.start = *line_pos, .len = i});
  *line_pos += i;
  return t;
}

bool is_known_codepoint(codepoint cp) {
  for (size_t i = 1; i < TOKEN_TYPES_SIZE; i++) {
    if (codepoint_collection_contains(TOKEN_CP_RANGES[i], cp)) {
      return true;
    }
  }
  return false;
}

Token *next_text_token(char const line[static 1], size_t line_pos[static 1]) {
  size_t start = *line_pos;
  if (!line[start]) {
    return NULL;
  }
  size_t i = 0;
  int unsigned len = 0;
  codepoint cp;
  while (line[start + i] && (cp = utf8_char(line + start + i, &len)) &&
         !is_known_codepoint(cp)) {
    i += len;
  }
  Token *t = new_token(TOKEN_TEXT, (slice){.start = start, .len = i});
  *line_pos += i;
  return t;
}

Token *next_token(char const line[static 1], size_t line_pos[static 1]) {
  if (f_debug()) {
    printf("next token at char: '%c'\n", line[*line_pos]);
  }
  Token *t;
  for (size_t i = 1; i < TOKEN_TYPES_SIZE; i++) {
    if ((t = get_token_of_type(i, TOKEN_CP_RANGES[i], line, line_pos))) {
      return t;
    }
  }
  return next_text_token(line, line_pos);
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

size_t sum_token_slice_lengths(Token **tokens, slice token_slice) {
  size_t str_len = 0;
  for (size_t i = 0; i < token_slice.len; i++) {
    str_len += tokens[token_slice.start + i]->length;
  }
  return str_len;
}

char *duplicate_substr_from_token_slice(Token **tokens, slice token_slice,
                                        char *line) {
  size_t str_len = sum_token_slice_lengths(tokens, token_slice);
  char *contents = strndup(line + tokens[token_slice.start]->start, str_len);
  return contents;
}

ASTNode *new_node_from_tokens(int unsigned type, Token **tokens,
                              slice token_slice, char *line) {
  if (f_debug()) {
    printf("making node of type %s from tokens\n", NODE_TYPE_NAMES[type]);
  }
  char *contents = duplicate_substr_from_token_slice(tokens, token_slice, line);
  prepare_contents(type, &contents);

  ASTNode *new_node = ast_create_node(type);
  if (type != ASTN_TEXT) {
    ast_add_child_node_with_contents(new_node, ASTN_TEXT, contents);
  } else {
    new_node->contents = contents;
  }
  return new_node;
}

void add_new_node_from_tokens(ASTNode *dest, int unsigned type,
                              Token *token_list[static 1], slice token_slice) {
  ASTNode *new_node =
      new_node_from_tokens(type, token_list, token_slice, dest->contents);
  ast_add_child(dest, new_node);
}

/*
Returns the type of the next split, and sets next_split_pos
*/
int unsigned find_next_split(Token *token_list[static 1], size_t pos,
                             size_t *next_split_pos) {
  size_t next;
  if ((next = find_code_span(token_list, pos))) {
    *next_split_pos = next;
    return ASTN_CODE_SPAN;
  }
  // TODO: add matcher for delimeters (*,_)
  return 0;
}

slice slice_based_on_split_type(int unsigned type, size_t split_pos,
                                size_t next_split_pos) {
  size_t len = next_split_pos - split_pos;
  if (type == ASTN_CODE_SPAN) {  // don't include backticks
    split_pos += 1;
    len -= 2;
  }
  return (slice){.start = split_pos, .len = len};
}

/*
  Build up list of token positions (token type, start pos, length)
  iterate, handle token based on rules. (backticks sequence of length x
  consumes everything until another backtick sequence of length x, if none
  found, then that no node is created).

  Once you've determined node start and end, if end of previous node is not 1
  before node start, put the middle content into text node.
*/
void parse_text(ASTNode node[static 1]) {
  if (f_debug()) {
    printf("parsing text on node type %s\n", NODE_TYPE_NAMES[node->type]);
  }
  Token **token_list = build_token_list(node->contents);
  // TODO: If token list ends in newline, drop newline
  if (f_debug()) {
    print_token_list(node->contents, token_list);
  }
  size_t i = 0;
  size_t next_split_pos = 0;
  size_t split_pos = 0;
  int unsigned split_type = 0;
  slice split_slice;
  while (token_list[i]) {
    if ((split_type = find_next_split(token_list, i, &next_split_pos))) {
      if (i > split_pos) {
        split_slice = slice_based_on_split_type(ASTN_TEXT, split_pos, i);
        add_new_node_from_tokens(node, ASTN_TEXT, token_list, split_slice);
        split_pos = i;
      }
      split_slice =
          slice_based_on_split_type(split_type, split_pos, next_split_pos);
      add_new_node_from_tokens(node, split_type, token_list, split_slice);
      // TODO: Look at contents of new split, determine if delimeter. If so, add
      // new node to delimeter list
      split_pos = next_split_pos;
      i = next_split_pos;
    } else {
      i++;
    }
  }
  if (token_list[split_pos]) {
    ASTNode *new_node = new_node_from_tokens(
        ASTN_TEXT, token_list,
        (slice){.start = split_pos, .len = i - split_pos}, node->contents);
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
    node = node->first_child;
    while (node) {
      parse_inline(node);
      node = node->next;
    }
  }
}
