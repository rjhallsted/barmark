#include "parse.h"

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "symbols.h"

/*
 * If matching, returns the number of tokens the sequence takes.
 * If not matching, returns 0
 */
size_t matches_symbol_seq(Token *ptr, SymbolSeq seq) {
  size_t i = 0;
  size_t tokens_read = 0;
  unsigned int splat_symbol_id;
  size_t splat_max_count;

  while (ptr && i < seq.seq_len) {
    // handle splats
    if (seq.seq[i] == SYMBOL_WILDCARD_ID) {
      splat_symbol_id = seq.seq[i + 1];
      if (seq.seq[i + 2] == SYMBOL_WILDCARD_ID) {
        splat_max_count = -1;  // Intentially underflow to get max value
      } else {
        splat_max_count = seq.seq[i + 2];
      }
      // while token == symbol, advance.
      while (ptr && splat_max_count > 0 &&
             (splat_symbol_id == SYMBOL_WILDCARD_ID ||
              ptr->symbol->id == splat_symbol_id) &&
             ptr->symbol->id != seq.seq[i + 3]) {
        ptr = ptr->next;
        splat_max_count -= 1;
        tokens_read += 1;
      }
      i += 3;
      continue;  // move to next iteration of outer loop
    }
    if (ptr && ptr->symbol->id != seq.seq[i]) {
      return 0;
    }
    ptr = ptr->next;
    i += 1;
    tokens_read += 1;
  }
  if (i == seq.seq_len) {
    return tokens_read;
  }
  return 0;
}

//////////////////
// Consumption
//////////////////

void consume_x_tokens(Token **stream_ptr, size_t to_consume) {
  for (size_t i = 0; i < to_consume; i++) {
    *stream_ptr = (*stream_ptr)->next;
  }
}

ASTNode *produce_code_block(Token **stream_ptr, size_t tokens_read) {
  // advance past indent:
  SymbolSeq tabOpening = {
      .seq = (const unsigned int[]){SYMBOL_WILDCARD_ID, SYMBOL_SPACE_ID, 3,
                                    SYMBOL_TAB_ID},
      .seq_len = 4};
  SymbolSeq spaceOpening = {
      .seq = (const unsigned int[]){SYMBOL_SPACE_ID, SYMBOL_SPACE_ID,
                                    SYMBOL_SPACE_ID, SYMBOL_SPACE_ID},
      .seq_len = 4};
  size_t opening_size;
  if ((opening_size = matches_symbol_seq(*stream_ptr, tabOpening)) ||
      (opening_size = matches_symbol_seq(*stream_ptr, spaceOpening))) {
    consume_x_tokens(stream_ptr, opening_size);
    tokens_read -= opening_size;
  } else {
    printf("Bad token sequence found for producing code block at token: '%s'\n",
           (*stream_ptr)->contents);
    exit(EXIT_FAILURE);
  }

  ASTNode *node = ast_create_node(ASTN_CODE_BLOCK);
  node->contents = join_token_contents(*stream_ptr, tokens_read);
  consume_x_tokens(stream_ptr, tokens_read);
  return node;
}

/***
 * Recursive Descent notes:
 *
 * Grammar definitions:
 * - wild(C, min, max) - Matches C at least min times and no more than max times
 * - * - expression that matches any token
 * - Opening tab
 *   (wild(\s, 3) > \t) || wild(\s, 4, 4)
 * - Code block:
 *   (Opening tab > wild(*, 0, INF) > wild(\n, 1, INF)) > wild(Code block, 0,
 * INF)
 * - text line:
 *   wild((text || \s), 0, INF) > \n
 * - h1
 *   # > \s > text line
 * - h2
 *   # > # > \s > text line
 *
 *
 *
 *
 */
