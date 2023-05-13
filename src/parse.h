#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "symbols.h"

// Parsing rules:
int pr_is_standard_node_ender(Token *token);
int pr_is_indented(Token *tokens);

// Consumption functions:
ASTNode *produce_code_block(Token **stream_ptr);

typedef ASTNode *(*ConsumerPtr)(Token **);

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

int matches_symbol_seq(Token *ptr, SymbolSeq seq);

static const unsigned int CODE_BLOCK_SEQ1[] = {SYMBOL_TAB_ID, SYMBOL_SPLAT_ID,
                                               SYMBOL_NL_ID};
#define CODE_BLOCK_SEQ1_SIZE 3
static const unsigned int CODE_BLOCK_SEQ2[] = {SYMBOL_SPACE_ID, SYMBOL_SPACE_ID,
                                               SYMBOL_SPACE_ID, SYMBOL_SPACE_ID,
                                               SYMBOL_SPLAT_ID, SYMBOL_NL_ID};
#define CODE_BLOCK_SEQ2_SIZE 6

#endif  // PARSE_H
