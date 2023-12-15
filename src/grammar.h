#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <stdbool.h>
#include <stdlib.h>

#include "inline.h"

bool m_open_tag(char line[static 1], size_t match_len[static 1],
                int unsigned forbidden_tags_size,
                const char *forbidden_tags[forbidden_tags_size]);
bool m_closing_tag(char line[static 1], size_t match_len[static 1],
                   int unsigned forbidden_tags_size,
                   const char *forbidden_tags[forbidden_tags_size]);
bool m_block_opening_tag(char line[static 1], size_t match_len[static 1]);
bool m_block_closing_tag(char line[static 1], size_t match_len[static 1]);

// Token-based
bool mt_left_flanking_delimiter_run(Token *token);
bool mt_right_flanking_delimiter_run(Token *token);

#endif  // GRAMMAR_H
