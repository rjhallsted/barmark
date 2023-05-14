#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "symbols.h"

// Consumption functions:
void consume_x_tokens(Token **stream_ptr, size_t to_consume);
ASTNode *produce_code_block(Token **stream_ptr, size_t tokens_read);

typedef ASTNode *(*ConsumerPtr)(Token **, size_t);

static const ConsumerPtr AST_CONSUMERS[] = {
    (ConsumerPtr)NULL,  // document standin
    (ConsumerPtr)NULL,  // TODO: Replace with text consumer,
    &produce_code_block,
    (ConsumerPtr)NULL,  // TODO: Replace with paragraph consumer,
    (ConsumerPtr)NULL,  // TODO: Replace with code consumer
};

typedef struct {
  const unsigned int *seq;
  const size_t seq_len;
} SymbolSeq;

size_t matches_symbol_seq(Token *ptr, SymbolSeq seq);

/*
 * SYMBOL SEQ DSL RULES
 * - All splats must have at least one symbol following the splat + 2 arguments,
 *   or you will get a segfault.
 *  BASIC SPLAT:
 *   - def: SYMBOL_SPLAT_ID, <symbol_id>, <max_count>
 *   - kind is the symbol to match
 *   - max count is the number to match up to.
 * UNIVERSAL SPLAT:
 *   - def: SYMBOL_SPLAT_ID, SYMBOL_SPLAT_ID, <max_count>
 *   - 2 splats followed my max_count match any symbols for up to max count
 * UNIVERSAL INFINITE SPLAT:
 *   - def : SYMBOL_SPLAT_ID, SYMBOL_SPLAT_ID, SYMBOL_SPLAT_ID
 *   - 3 splats match any symbols for any length
 */

/* [splat(' ', 4), \t, splat(*, *), \n] */
static const unsigned int CODE_BLOCK_SEQ1[] = {
    SYMBOL_SPLAT_ID, SYMBOL_SPACE_ID, 4,
    SYMBOL_TAB_ID,   SYMBOL_SPLAT_ID, SYMBOL_SPLAT_ID,
    SYMBOL_SPLAT_ID, SYMBOL_NL_ID};
#define CODE_BLOCK_SEQ1_SIZE 8
/* [' ', ' ', ' ', ' ', splat(*, *), \n] */
static const unsigned int CODE_BLOCK_SEQ2[] = {
    SYMBOL_SPACE_ID, SYMBOL_SPACE_ID, SYMBOL_SPACE_ID, SYMBOL_SPACE_ID,
    SYMBOL_SPLAT_ID, SYMBOL_SPLAT_ID, SYMBOL_SPLAT_ID, SYMBOL_NL_ID};
#define CODE_BLOCK_SEQ2_SIZE 8

#endif  // PARSE_H
