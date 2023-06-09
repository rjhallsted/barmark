#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdio.h>

#include "ast.h"

ASTNode *build_block_structure(FILE *fd);
int matches_continuation_markers(ASTNode *node, const char *line,
                                 size_t *match_len);
void tab_expand(char **line, size_t line_pos);

#endif  // BLOCKS_H
