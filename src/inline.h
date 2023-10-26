#ifndef INLINE_H
#define INLINE_H

#include "ast.h"

void parse_inline(ASTNode node[static 1]);

enum TOKEN_TYPES {
  TOKEN_TEXT = 0,
  TOKEN_BACKTICK,
};

static const int unsigned TOKEN_TYPES_SIZE = 2;
static char const *TOKEN_TYPE_NAMES[TOKEN_TYPES_SIZE] = {
    "TEXT",
    "BACKTICK",
};

typedef struct Token {
  int unsigned type;
  size_t start;
  size_t length;
} Token;

#endif  // INLINE_H
