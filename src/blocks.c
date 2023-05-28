#include "blocks.h"

#include <stdio.h>

#include "ast.h"

ASTNode *build_block_structure(FILE fd) {
  ASTNode *document = ast_create_node(ASTN_DOCUMENT);
  // while get line
  // traverse to deepest/lastest open block, building up continuation markers
  // along the way consume continuation as you go, stop when no longer matching
  // look for new block starts
  // if found, close remaining unmatched blocks
  // begin new block as child of last matched block
  // incorporate remainder of line in last open block
}
