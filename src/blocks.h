#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdio.h>

#include "ast.h"

ASTNode *build_block_structure(FILE *fd);
int matches_continuation_markers(ASTNode *node, char const *const line,
                                 size_t *match_len);
void tab_expand(char **line, size_t line_pos, size_t lookahead);
void add_line_to_ast(ASTNode *root, char **line);

#endif  // BLOCKS_H
