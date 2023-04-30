#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "symbols.h"

// Parsing rules:
int pr_is_double_newline(TokenStream tokens);
int pr_is_indented(TokenStream tokens);

// Consumption functions:
ASTNode *produce_code_block(TokenStream *stream_ptr);

typedef ASTNode *(*ConsumerPtr)(TokenStream *);

static const ConsumerPtr AST_CONSUMERS[] = {
    (ConsumerPtr)NULL,  // document standin
    (ConsumerPtr)NULL,  // TODO: Replace with text consumer,
    &produce_code_block,
    (ConsumerPtr)NULL,  // TODO: Replace with paragraph consumer,
    (ConsumerPtr)NULL,  // TODO: Replace with code consumer
};

#endif  // PARSE_H
