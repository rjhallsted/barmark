#ifndef HTML_GRAMMAR_H
#define HTML_GRAMMAR_H

#include <stdbool.h>
#include <stdlib.h>

bool m_open_tag(char line[static 1], size_t match_len[static 1],
                int unsigned forbidden_tags_size,
                const char *forbidden_tags[forbidden_tags_size]);
bool m_closing_tag(char line[static 1], size_t match_len[static 1],
                   int unsigned forbidden_tags_size,
                   const char *forbidden_tags[forbidden_tags_size]);
bool m_block_opening_tag(char line[static 1], size_t match_len[static 1]);
bool m_block_closing_tag(char line[static 1], size_t match_len[static 1]);

#endif  // HTML_GRAMMAR_H
