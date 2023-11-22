#ifndef INLINE_H
#define INLINE_H

#include "ast.h"
#include "utf8.h"
#include <stdbool.h>

void parse_inline(ASTNode node[static 1]);

enum TOKEN_TYPES {
  TOKEN_TEXT = 0,
  TOKEN_BACKTICKS,
  TOKEN_STARS,
  TOKEN_UNDERSCORES,
};

static const int unsigned TOKEN_TYPES_SIZE = 4;
static char const *TOKEN_TYPE_NAMES[TOKEN_TYPES_SIZE] = {
    "TEXT",
    "BACKTICKS",
    "STARS",
    "UNDERSCORES",
};

typedef struct {
  codepoint min;
  codepoint max;
} codepoint_range;

typedef struct codepoint_collection{
  size_t length;
  codepoint_range *ranges;
  bool runnable;
} codepoint_collection;

static const codepoint_range BACKTICK_RANGES[1] = {
    {'`', '`'},
};

static const codepoint_range STAR_RANGES[1] = {
    {'*', '*'},
};

static const codepoint_range UNDERSCORE_RANGES[1] = {
    {'_', '_'},
};

static const codepoint_collection TOKEN_RANGES[TOKEN_TYPES_SIZE] = {
    {0, NULL, false},
    {1, (codepoint_range *)BACKTICK_RANGES, true},
    {1, (codepoint_range *)STAR_RANGES, true},
    {1, (codepoint_range *)UNDERSCORE_RANGES, true},
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
