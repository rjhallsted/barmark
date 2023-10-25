#include "inline.h"

#include "ast.h"
#include "string_mod.h"
#include "util.h"

void parse_text(ASTNode node[static 1]) {
  if (f_debug()) {
    printf("parsing text on node type %s\n", NODE_TYPE_NAMES[node->type]);
  }

  string_mod_ref ref = make_unmodified_string_mod_ref(&(node->contents));
  // do stuff to proposed;
  commit_string_mod(ref);
}

void parse_inline(ASTNode node[static 1]) {
  if (node->type == ASTN_TEXT) {
    parse_text(node);
  } else {
    for (size_t i = 0; i < node->children_count; i++) {
      parse_inline(node->children[i]);
    }
  }
}
