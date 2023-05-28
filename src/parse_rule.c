
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
#include <string.h>

#include "ast.h"
#include "symbols.h"

ASTNode *flatten_and_change_type(ASTNode *node, unsigned int new_type) {
  if (node) {
    ast_move_children_to_contents(node);
    node->type = new_type;
  }
  return node;
}

/*
 * Each matcher should modify the stream ptr it is given, but only if the match
 * is valid. If the match is invalid, do not modify the stream ptr.
 */

/******************
 * Symbol matchers
 *******************/
ASTNode *m_text_node_from_next_token(Token ***stream) {
  ASTNode *node = ast_create_node(ASTN_TEXT);
  node->contents = strdup((**stream)->contents);
  *stream = &((**stream)->next);
  return node;
}

ASTNode *m_matches_symbol(Token ***stream, unsigned int symbol_id) {
  if (**stream == NULL) {
    return 0;
  }
  if ((**stream)->symbol->id == symbol_id) {
    return m_text_node_from_next_token(stream);
  }
  return NULL;
}

ASTNode *m_wildcard(Token ***stream) {
  if (**stream == NULL) {
    return 0;
  }
  return m_text_node_from_next_token(stream);
}

ASTNode *m_anything_but(Token ***stream, unsigned int symbol_id) {
  if (**stream == NULL || (**stream)->symbol->id == symbol_id) {
    return 0;
  }
  return m_text_node_from_next_token(stream);
}

ASTNode *m_text(Token ***stream) {
  return m_matches_symbol(stream, SYMBOL_TEXT_ID);
}

ASTNode *m_space(Token ***stream) {
  return m_matches_symbol(stream, SYMBOL_SPACE_ID);
}

ASTNode *m_newline(Token ***stream) {
  return m_matches_symbol(stream, SYMBOL_NL_ID);
}

ASTNode *m_tab(Token ***stream) {
  return m_matches_symbol(stream, SYMBOL_TAB_ID);
}

ASTNode *m_dash(Token ***stream) {
  return m_matches_symbol(stream, SYMBOL_DASH_ID);
}

/*******************
** Generic matchers
*******************/

ASTNode *m_wild(MatcherFn matcher, size_t min_matches, size_t max_matches,
                Token ***stream) {
  Token **stream_cpy = *stream;
  size_t i = 0;
  ASTNode *node = ast_create_node(ASTN_DEFAULT);
  ASTNode *child;
  while (i < max_matches && (child = matcher(&stream_cpy))) {
    i += 1;
    ast_add_child(node, child);
  }
  if (min_matches <= i && i <= max_matches) {
    *stream = stream_cpy;
    return node;
  }
  ast_free_node(node);
  return NULL;
}

ASTNode *m_or(MatcherFn m1, MatcherFn m2, Token ***stream) {
  Token **stream_cpy = *stream;
  ASTNode *node;

  if ((node = m1(&stream_cpy))) {
    *stream = stream_cpy;
    return node;
  }
  stream_cpy = *stream;
  if ((node = m2(&stream_cpy))) {
    *stream = stream_cpy;
    return node;
  }
  return NULL;
}

ASTNode *m_then(MatcherFn m1, MatcherFn m2, Token ***stream) {
  Token **stream_cpy = *stream;
  ASTNode *node, *res, *res2;

  node = ast_create_node(ASTN_DEFAULT);
  res = m1(&stream_cpy);
  if (res) {
    ast_add_child(node, res);
    res2 = m2(&stream_cpy);
    if (res2) {
      ast_add_child(node, res2);
      *stream = stream_cpy;
      return node;
    }
  }
  ast_free_node(node);
  return NULL;
}

/*********************
 * Utility matchers
 **********************/

ASTNode *m_up_to_3_spaces(Token ***stream) {
  return m_wild(m_space, 0, 3, stream);
}
ASTNode *m_exactly_4_spaces(Token ***stream) {
  return m_wild(m_space, 4, 4, stream);
}
ASTNode *m_wild_newline(Token ***stream) {
  return m_wild(m_newline, 0, SIZE_MAX, stream);
}

ASTNode *m_anything_but_newline(Token ***stream) {
  return m_anything_but(stream, SYMBOL_NL_ID);
}

ASTNode *m_wild_anything_but_newline(Token ***stream) {
  return m_wild(m_anything_but_newline, 0, SIZE_MAX, stream);
}

/*********************
 * m_text_line
 **********************/

ASTNode *m_text_line_or1(Token ***stream) {
  return m_or(m_text, m_space, stream);
}

ASTNode *m_text_line_wild1(Token ***stream) {
  return m_wild(m_text_line_or1, 0, SIZE_MAX, stream);
}

ASTNode *m_text_line(Token ***stream) {
  ASTNode *res = m_then(m_text_line_wild1, m_newline, stream);
  flatten_and_change_type(res, ASTN_TEXT);
  return res;
}

/*******************
 * m_opening_tab
 *******************/
ASTNode *m_opening_tab_with_tab(Token ***stream) {
  return m_then(m_up_to_3_spaces, m_tab, stream);
}

/**
 * @brief If successfully matches, will return a single ASTNode
 * With the contents of the opening tab, and no children
 *
 * @param stream
 * @return ASTNode*
 */
ASTNode *m_opening_tab(Token ***stream) {
  ASTNode *res = m_or(m_opening_tab_with_tab, m_exactly_4_spaces, stream);
  flatten_and_change_type(res, ASTN_TEXT);
  return res;
}

/*******************
 * m_code_block
 *******************/
/***
 * Will flatten result into 1 node with children: (opening tab, ...line
 * contents)
 */
ASTNode *m_code_block_opening_then_contents(Token ***stream) {
  ASTNode *res = m_then(m_opening_tab, m_wild_anything_but_newline, stream);
  if (res) {
    ast_flatten_children(res);
  }
  return res;
}

/***
 * Will drop opening tab, and join the rest of the nodes into the contents of
 * this node.
 */
ASTNode *m_code_block_line(Token ***stream) {
  ASTNode *res =
      m_then(m_code_block_opening_then_contents, m_wild_newline, stream);
  if (res) {
    ast_flatten_children(res);
    ast_remove_child_at_index(res, 0);
    flatten_and_change_type(res, ASTN_TEXT);
  }
  return res;
}

ASTNode *m_wild_code_block(Token ***stream) {
  return m_wild(m_code_block, 0, SIZE_MAX, stream);
}

/* Returns a code block node */
ASTNode *m_code_block(Token ***stream) {
  ASTNode *node = m_then(m_code_block_line, m_wild_code_block, stream);
  if (node) {
    flatten_and_change_type(node, ASTN_CODE_BLOCK);
  }
  return node;
}

/****************************
 * m_unordered_list_item
 ***************************/

ASTNode *m_ul_item_opener_wild1(Token ***stream) {
  return m_wild(m_space, 0, 3, stream);
}

ASTNode *m_ul_item_opener_then1(Token ***stream) {
  return m_then(m_ul_item_opener_wild1, m_dash, stream);
}

ASTNode *m_ul_item_opener(Token ***stream) {
  return m_then(m_ul_item_opener_then1, m_space, stream);
}

// TODO: Finish writing this
// ASTNode *m_ul_item(Token ***stream) {
//   /*
//     wild(' ') then '-' then text_line then m_wild(\n) then
//     m_wild(then(opening_tab, text_line))
//   */
// }
