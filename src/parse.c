#include "parse.h"

#include <string.h>

#include "ast.h"
#include "symbols.h"

int pr_is_double_newline(TokenStream tokens) {
  int tokens_read = 0;
  while (tokens[tokens_read].symbol->id != SYMBOL_NULL_ID && tokens_read < 2) {
    tokens_read += 1;
  }
  if (tokens_read != 2) {
    return 0;
  }
  return 1;
}

int pr_is_indented(TokenStream tokens) {
  if (tokens[0].symbol->id == SYMBOL_TAB_ID) {
    return 1;
  }
  return 0;
}

//////////////////
// Consumption
//////////////////

// TODO: test
ASTNode *produce_code_block(TokenStream *stream_ptr) {
  ASTNode *node = ast_create_node(ASTN_CODE_BLOCK);
  node->contents = strdup("");

  char *tmp;
  TokenStream ptr = *stream_ptr;

  while ((ptr->symbol->id != SYMBOL_NULL_ID) && !pr_is_double_newline(ptr)) {
    if (ptr->symbol->id == SYBMOL_NL_ID) {
      if ((ptr + 1)->symbol->id == SYMBOL_TAB_ID) {
        // copy contents thus far (including newline) into node contents as text
        tmp = join_token_contents(*stream_ptr, ptr - (*stream_ptr) + 1);
        node->contents = strcat(node->contents, tmp);
        free(tmp);
        // advance ptr 2
        *stream_ptr = ptr + 2;
        ptr = *stream_ptr;
      } else {  // Newline is not followed by tab, so we're exiting the code
                // block
        break;
      }
    } else {
      ptr += 1;
    }
  }
  tmp = join_token_contents(*stream_ptr, ptr - (*stream_ptr));
  *stream_ptr = ptr;
  node->contents = strcat(node->contents, tmp);
  free(tmp);
  return node;
}
