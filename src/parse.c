#include "parse.h"

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "symbols.h"

// TODO: Add spec tests to document all cases
int pr_is_standard_node_ender(Token *token) {
  int tokens_read = 0;
  while (token && tokens_read < 2 && token->symbol->id == SYMBOL_NL_ID) {
    tokens_read += 1;
    token = token->next;
  }
  if (tokens_read != 2 || (token && is_whitespace_token(token->next))) {
    return 0;
  }
  return 1;
}

int pr_is_indented(Token *token) {
  if (token->symbol->id == SYMBOL_TAB_ID) {
    return 1;
  }
  return 0;
}

//////////////////
// Consumption
//////////////////

// TODO: test
/* Note that this expects the first token to always be a tab. */
ASTNode *produce_code_block(Token **stream_ptr) {
  if ((*stream_ptr) && (*stream_ptr)->symbol->id != SYMBOL_TAB_ID) {
    printf("Bad token found for producing code block: %s\n",
           (*stream_ptr)->contents);
    exit(EXIT_FAILURE);
  }
  *stream_ptr = (*stream_ptr)->next;

  ASTNode *node = ast_create_node(ASTN_CODE_BLOCK);
  node->contents = strdup("");

  char *tmp;
  Token *ptr = *stream_ptr;
  size_t tokens_read = 0;

  while (ptr && !pr_is_standard_node_ender(ptr)) {
    if (ptr->symbol->id == SYMBOL_NL_ID) {
      if (ptr->next && ptr->next->symbol->id == SYMBOL_TAB_ID) {
        // copy contents thus far (including newline) into node contents as text
        tmp = join_token_contents(*stream_ptr, tokens_read + 1);
        node->contents = strcat(node->contents, tmp);
        free(tmp);
        // advance ptr 2
        *stream_ptr = ptr->next->next;
        ptr = *stream_ptr;
        tokens_read = 0;
      } else {  // Newline is not followed by tab, so we're exiting the code
                // block
        break;
      }
    } else {
      ptr = ptr->next;
      tokens_read += 1;
    }
  }
  tmp = join_token_contents(*stream_ptr, tokens_read);
  *stream_ptr = ptr;
  node->contents = strcat(node->contents, tmp);
  free(tmp);
  return node;
}
