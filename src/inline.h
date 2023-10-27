#ifndef INLINE_H
#define INLINE_H

#include "ast.h"

void parse_inline(ASTNode node[static 1]);

enum TOKEN_TYPES {
  TOKEN_TEXT = 0,
  TOKEN_BACKTICKS,
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

static const int unsigned DONT_PARSE_INLINE_BLOCKS_SIZE = 9;
static const int unsigned
    DONT_PARSE_INLINE_BLOCKS[DONT_PARSE_INLINE_BLOCKS_SIZE] = {
        ASTN_CODE_BLOCK,        ASTN_FENCED_CODE_BLOCK, ASTN_HTML_BLOCK_TYPE_1,
        ASTN_HTML_BLOCK_TYPE_2, ASTN_HTML_BLOCK_TYPE_3, ASTN_HTML_BLOCK_TYPE_4,
        ASTN_HTML_BLOCK_TYPE_5, ASTN_HTML_BLOCK_TYPE_6, ASTN_HTML_BLOCK_TYPE_7,
};

#endif  // INLINE_H
