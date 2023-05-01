#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "symbols.h"

// Parsing rules:
int pr_is_double_newline(Token *tokens);
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

#endif  // PARSE_H
