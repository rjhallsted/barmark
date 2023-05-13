#include "parse.h"

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "symbols.h"

int pr_is_standard_node_ender(Token *token) {
  int tokens_read = 0;
  while (token && tokens_read < 2 && token->symbol->id == SYMBOL_NL_ID) {
    tokens_read += 1;
    token = token->next;
  }
  if (tokens_read != 2 || token == NULL ||
      (token && is_whitespace_token(token))) {
    return 0;
  }
  return 1;
}

int pr_is_indented(Token *token) {
  if (token->symbol->id == SYMBOL_TAB_ID) {
    return 1;
  }
  return 0;
}

int matches_symbol_seq(Token *ptr, SymbolSeq seq) {
  size_t i = 0;
  while (ptr && i < seq.seq_len &&
         (ptr->symbol->id == seq.seq[i] || seq.seq[i] == SYMBOL_SPLAT_ID)) {
    while (seq.seq[i] == SYMBOL_SPLAT_ID && ptr->next &&
           ptr->next->symbol->id != seq.seq[i + 1]) {
      // NOTE: Not sure if being greedy with the splat will ultimately work. may
      // need to be more exhaustive.
      ptr = ptr->next;
      i += 1;
    }
    ptr = ptr->next;
    i += 1;
  }
  if (i == seq.seq_len) {
    return 1;
  }
  return 0;
}

//////////////////
// Consumption
//////////////////

int is_end_of_code_block(Token *ptr) {
  if (pr_is_standard_node_ender(ptr) ||
      (ptr->symbol->id == SYMBOL_NL_ID && ptr->next &&
       !is_whitespace_token(ptr->next))) {
    return 1;
  }
  return 0;
}

/* Note that this expects the first token to always be a tab. */
ASTNode *produce_code_block(Token **stream_ptr) {
  if ((*stream_ptr) && (*stream_ptr)->symbol->id != SYMBOL_TAB_ID) {
    printf("Bad token found for producing code block: %s\n",
           (*stream_ptr)->contents);
    exit(EXIT_FAILURE);
  }
  *stream_ptr = (*stream_ptr)->next;

  ASTNode *node = ast_create_node(ASTN_CODE_BLOCK);
  node->contents = strdup("");

  char *tmp;
  Token *ptr = *stream_ptr;
  size_t tokens_read = 0;

  while (ptr && !is_end_of_code_block(ptr)) {
    if (ptr->symbol->id == SYMBOL_NL_ID && ptr->next &&
        ptr->next->symbol->id == SYMBOL_TAB_ID) {
      // copy contents thus far (including newline) into node contents as text
      tmp = join_token_contents(*stream_ptr, tokens_read + 1);
      node->contents = strcat(node->contents, tmp);
      free(tmp);
      // advance ptr 2
      *stream_ptr = ptr->next->next;
      ptr = *stream_ptr;
      tokens_read = 0;
    } else {
      ptr = ptr->next;
      tokens_read += 1;
    }
  }
  tmp = join_token_contents(*stream_ptr, tokens_read);
  *stream_ptr = ptr;
  node->contents = strcat(node->contents, tmp);
  free(tmp);
  return node;
}
