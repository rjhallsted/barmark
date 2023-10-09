#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdbool.h>
#include <stdio.h>

#include "ast.h"

ASTNode *build_block_structure(FILE fd[static 1]);
ASTNode *get_deepest_non_text_child(ASTNode node[static 1]);
bool matches_continuation_markers(ASTNode node[static 1], char *line[static 1],
                                  size_t line_pos, size_t match_len[static 1]);
void add_line_to_ast(ASTNode root[static 1], char *line[static 1]);

#endif  // BLOCKS_H
