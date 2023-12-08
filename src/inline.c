#include "inline.h"

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "grammar.h"
#include "string_mod.h"
#include "utf8.h"
#include "util.h"

Token *new_token(int unsigned type, slice token_slice) {
  Token *t = malloc(sizeof(Token));
  t->next = NULL;
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

Token *build_token_list(char const line[static 1]) {
  Token *dummy = new_token(ASTN_DOCUMENT, (slice){0, 0});
  Token *ptr = dummy;
  size_t line_pos = 0;
  Token *token = NULL;
  while ((token = next_token(line, &line_pos))) {
    ptr->next = token;
    ptr = ptr->next;
  }
  ptr = dummy->next;
  free(dummy);
  return ptr;
}

void print_token_list(char const line[static 1], Token *head) {
  printf("Tokens:\n\t");
  while (head) {
    char *s = strndup(line + head->start, head->length);
    printf("%s(\"%s\") -> ", TOKEN_TYPE_NAMES[head->type], s);
    free(s);
    head = head->next;
  }
  printf("\n");
}

void free_token_list(Token *head) {
  Token *next;
  while (head) {
    next = head->next;
    free(head);
    head = next;
  }
}

// find_x functions return the last token in the slice they are composed of, or
// NULL

Token *find_code_span(Token *token) {
  if (token->type != TOKEN_BACKTICKS) {
    return NULL;
  }
  Token *opener = token;
  token = token->next;
  while (token &&
         (token->type != TOKEN_BACKTICKS || token->length != opener->length)) {
    token = token->next;
  }
  if (token) {
    return token;
  }
  return NULL;
}

Token *find_emphasis_or_link_delimeter(Token *token) {
  if (token->type == TOKEN_STARS || token->type == TOKEN_UNDERSCORES) {
    return token;
  }
  return NULL;
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

size_t sum_token_lengths(Token *head, size_t num_to_use) {
  size_t str_len = 0;
  for (size_t i = 0; i < num_to_use; i++) {
    str_len += head->length;
    head = head->next;
  }
  return str_len;
}

char *duplicate_substr_from_tokens(Token *token, size_t count, char *line) {
  size_t str_len = sum_token_lengths(token, count);
  char *contents = strndup(line + token->start, str_len);
  return contents;
}

ASTNode *new_node_from_tokens(int unsigned type, Token *ptr, size_t count,
                              char *line) {
  if (f_debug()) {
    printf("making node of type %s from tokens\n", NODE_TYPE_NAMES[type]);
  }
  char *contents = duplicate_substr_from_tokens(ptr, count, line);
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
                              Token ptr[static 1], size_t count) {
  ASTNode *new_node = new_node_from_tokens(type, ptr, count, dest->contents);
  ast_add_child(dest, new_node);
}

/*
Returns the type of the next split, and sets end_of_split
*/
int unsigned find_next_split(Token ptr[static 1], Token **end_of_split) {
  Token *next = NULL;
  int unsigned type = 0;
  if ((next = find_code_span(ptr))) {
    type = ASTN_CODE_SPAN;
  } else if ((next = find_emphasis_or_link_delimeter(ptr))) {
    type = ASTN_TEXT;
  }
  *end_of_split = next;
  return type;
  // TODO: add matcher for delimeters (*,_)
}

slice slice_based_on_split_type(int unsigned type, Token *opener,
                                Token *next_split_pos) {
  size_t len = item_distance((SinglyLinkedItem *)opener,
                             (SinglyLinkedItem *)next_split_pos);
  size_t skip = 0;
  if (type == ASTN_CODE_SPAN) {  // don't include backticks
    skip += 1;
    len -= 2;
  }
  return (slice){.start = skip, .len = len};
}

Token *skip_tokens(Token *head, size_t skip) {
  while (skip) {
    head = head->next;
    skip--;
  }
  return head;
}

size_t token_list_len(Token *head) {
  return list_len((SinglyLinkedItem *)head);
}

Delimiter *new_delimiter_from_token(Token *token) {
  char type;
  switch (token->type) {
    case TOKEN_UNDERSCORES:
      type = '_';
      break;
    case TOKEN_STARS:
      type = '*';
      break;
    default:
      printf("Cannot create a delimiter from token of type %s\n",
             TOKEN_TYPE_NAMES[token->type]);
      exit(EXIT_FAILURE);
  }
  Delimiter *delim = malloc(sizeof(Delimiter));
  delim->next = NULL;
  delim->prev = NULL;
  delim->type = type;
  delim->count = token->length;
  delim->active = true;
  delim->potential_opener = false;  // TODO: determine opener potential
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
  Token *head = build_token_list(node->contents);
  Token *ptr = head;
  Token *last_split = head;
  // TODO: If token list ends in newline, drop newline
  if (f_debug()) {
    print_token_list(node->contents, head);
  }
  Token *end_of_split = NULL;
  int unsigned split_type = 0;
  slice split_slice;
  Delimiter *delim_list_head = NULL;
  while (ptr) {
    if ((split_type = find_next_split(ptr, &end_of_split))) {
      if (ptr != last_split) {
        split_slice = slice_based_on_split_type(ASTN_TEXT, last_split, ptr);
        add_new_node_from_tokens(node, ASTN_TEXT,
                                 skip_tokens(last_split, split_slice.start),
                                 split_slice.len);
      }
      split_slice =
          slice_based_on_split_type(split_type, ptr, end_of_split->next);
      add_new_node_from_tokens(node, split_type,
                               skip_tokens(ptr, split_slice.start),
                               split_slice.len);
      if (ptr->type == TOKEN_STARS || ptr->type == TOKEN_UNDERSCORES) {
        // TODO: Look at contents of new split, determine if delimeter. If so,
        // add new node to delimeter list
        Delimiter *delim = new_delimiter_from_token(ptr);

        // is_potential_opener
        // add delimeter to list
      }
      ptr = end_of_split->next;
      last_split = ptr;
    } else {
      ptr = ptr->next;
    }
  }
  if (last_split) {
    ASTNode *new_node = new_node_from_tokens(
        ASTN_TEXT, last_split, token_list_len(last_split), node->contents);
    ast_add_child(node, new_node);
  }
  free(node->contents);
  node->contents = NULL;
  free_token_list(head);
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
