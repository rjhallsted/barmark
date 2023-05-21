#ifndef PARSE_RULE_H
#define PARSE_RULE_H

#include "symbols.h"

typedef int MatcherFn(Token **);

int m_wild(MatcherFn matcher, size_t min_matches, size_t max_matches,
           Token **stream);
int m_or(MatcherFn m1, MatcherFn m2, Token **stream);
int m_then(MatcherFn m1, MatcherFn m2, Token **stream);

int m_matches_symbol(Token **stream, unsigned int symbol_id);
int m_text(Token **stream);
int m_space(Token **stream);
int m_newline(Token **stream);

int m_text_line(Token **stream);

#endif  // PARSE_RULE_H
