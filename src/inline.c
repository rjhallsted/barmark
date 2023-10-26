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
    Token *t = new_token(TOKEN_BACKTICK, *line_pos, 1);
    *line_pos += 1;
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
  free_token_list(token_list);
}

void parse_inline(ASTNode node[static 1]) {
  if (node->type == ASTN_TEXT) {
    parse_text(node);
  } else {
    for (size_t i = 0; i < node->children_count; i++) {
      parse_inline(node->children[i]);
    }
  }
}
