#ifndef PARSE_RULE_H
#define PARSE_RULE_H

#include "ast.h"
#include "symbols.h"

typedef ASTNode *MatcherFn(Token ***);

ASTNode *m_wild(MatcherFn matcher, size_t min_matches, size_t max_matches,
                Token ***stream);
ASTNode *m_or(MatcherFn m1, MatcherFn m2, Token ***stream);
ASTNode *m_then(MatcherFn m1, MatcherFn m2, Token ***stream);

ASTNode *m_matches_symbol(Token ***stream, unsigned int symbol_id);
ASTNode *m_text(Token ***stream);
ASTNode *m_space(Token ***stream);
ASTNode *m_newline(Token ***stream);
ASTNode *m_tab(Token ***stream);

ASTNode *m_text_line(Token ***stream);
ASTNode *m_opening_tab(Token ***stream);
ASTNode *m_code_block(Token ***stream);

#endif  // PARSE_RULE_H
