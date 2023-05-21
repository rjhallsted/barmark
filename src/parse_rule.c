
/***
 * Recursive Descent notes:
 *
 * Grammar definitions:
 * - wild(C, min, max) - Matches C at least min times and no more than max times
 * - * - expression that matches any token
 * - Opening tab
 *   (wild(\s, 3) > \t) || wild(\s, 4, 4)
 * - Code block:
 *   (Opening tab > wild(*, 0, INF) > wild(\n, 1, INF)) > wild(Code block, 0,
 * INF)
 * - text line:
 *   wild((text || \s), 0, INF) > \n
 * - h1
 *   # > \s > text line
 * - h2
 *   # > # > \s > text line
 *
 *
 *
 *
 */

#include "parse_rule.h"

#include <stdint.h>
#include <stdio.h>

#include "symbols.h"

/*
 * Each matcher should modify the stream ptr it is given, but only if the match
 * is valid. If the match is invalid, do not modify the stream ptr.
 */

/******************
 * Symbol matchers
 *******************/

int m_matches_symbol(Token ***stream, unsigned int symbol_id) {
  if (**stream == NULL) {
    return 0;
  }
  if ((**stream)->symbol->id == symbol_id) {
    *stream = &((**stream)->next);
    return 1;
  }
  return 0;
}

int m_wildcard(Token ***stream) {
  if (**stream == NULL) {
    return 0;
  }
  *stream = &((**stream)->next);
  return 1;
}

int m_anything_but(Token ***stream, unsigned int symbol_id) {
  if (**stream == NULL || (**stream)->symbol->id == symbol_id) {
    return 0;
  }
  *stream = &((**stream)->next);
  return 1;
}

int m_text(Token ***stream) { return m_matches_symbol(stream, SYMBOL_TEXT_ID); }

int m_space(Token ***stream) {
  return m_matches_symbol(stream, SYMBOL_SPACE_ID);
}

int m_newline(Token ***stream) {
  return m_matches_symbol(stream, SYMBOL_NL_ID);
}

int m_tab(Token ***stream) { return m_matches_symbol(stream, SYMBOL_TAB_ID); }

/*******************
** Generic matchers
*******************/

int m_wild(MatcherFn matcher, size_t min_matches, size_t max_matches,
           Token ***stream) {
  Token **stream_cpy = *stream;
  size_t i = 0;
  while (i < max_matches && matcher(&stream_cpy)) {
    i += 1;
  }
  if (min_matches <= i && i <= max_matches) {
    *stream = stream_cpy;
    return 1;
  }
  return 0;
}

int m_or(MatcherFn m1, MatcherFn m2, Token ***stream) {
  Token **stream_cpy = *stream;
  int res;

  if ((res = m1(&stream_cpy))) {
    *stream = stream_cpy;
    return 1;
  }
  stream_cpy = *stream;
  if ((res = m2(&stream_cpy))) {
    *stream = stream_cpy;
    return 1;
  }
  return 0;
}

int m_then(MatcherFn m1, MatcherFn m2, Token ***stream) {
  Token **stream_cpy = *stream;
  int res, res2;

  res = m1(&stream_cpy);
  if (res) {
    res2 = m2(&stream_cpy);
    if (res2) {
      *stream = stream_cpy;
      return 1;
    }
  }
  return 0;
}

/*********************
 * Utility matchers
 **********************/

int m_up_to_3_spaces(Token ***stream) { return m_wild(m_space, 0, 3, stream); }
int m_exactly_4_spaces(Token ***stream) {
  return m_wild(m_space, 4, 4, stream);
}
int m_wild_newline(Token ***stream) {
  return m_wild(m_newline, 0, SIZE_MAX, stream);
}

int m_anything_but_newline(Token ***stream) {
  return m_anything_but(stream, SYMBOL_NL_ID);
}

int m_wild_anything_but_newline(Token ***stream) {
  return m_wild(m_anything_but_newline, 0, SIZE_MAX, stream);
}

/*********************
 * m_text_line
 **********************/

int m_text_line_or1(Token ***stream) { return m_or(m_text, m_space, stream); }

int m_text_line_wild1(Token ***stream) {
  return m_wild(m_text_line_or1, 0, SIZE_MAX, stream);
}

int m_text_line(Token ***stream) {
  return m_then(m_text_line_wild1, m_newline, stream);
}

/*******************
 * m_opening_tab
 *******************/
int m_opening_tab_with_tab(Token ***stream) {
  return m_then(m_up_to_3_spaces, m_tab, stream);
}

int m_opening_tab(Token ***stream) {
  return m_or(m_opening_tab_with_tab, m_exactly_4_spaces, stream);
}

/*******************
 * m_code_block
 *******************/
int m_code_block_opening_then_contents(Token ***stream) {
  return m_then(m_opening_tab, m_wild_anything_but_newline, stream);
}

int m_code_block_line(Token ***stream) {
  return m_then(m_code_block_opening_then_contents, m_wild_newline, stream);
}

int m_wild_code_block(Token ***stream) {
  return m_wild(m_code_block, 0, SIZE_MAX, stream);
}

int m_code_block(Token ***stream) {
  return m_then(m_code_block_line, m_wild_code_block, stream);
}
