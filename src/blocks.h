#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdio.h>

#include "ast.h"

ASTNode *build_block_structure(FILE *fd);
int matches_continuation_markers(ASTNode *node, const char *line);

#endif  // BLOCKS_H
