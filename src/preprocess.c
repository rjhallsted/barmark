#include <string.h>

#include "symbols.h"

typedef const unsigned int replacement_arr;

typedef struct {
  replacement_arr *sequence;
  unsigned int seq_size;
  replacement_arr *replacement;
} ReplacementRule;

#define REPLACEMENT_RULE_COUNT 4

static replacement_arr REPL_TAB_SEQ_1[] = {SYMBOL_NL_ID, SYMBOL_SPACE_ID,
                                           SYMBOL_TAB_ID, SYMBOL_NULL_ID};
static replacement_arr REPL_TAB_SEQ_2[] = {SYMBOL_NL_ID, SYMBOL_SPACE_ID,
                                           SYMBOL_SPACE_ID, SYMBOL_TAB_ID,
                                           SYMBOL_NULL_ID};
static replacement_arr REPL_TAB_SEQ_3[] = {SYMBOL_NL_ID,    SYMBOL_SPACE_ID,
                                           SYMBOL_SPACE_ID, SYMBOL_SPACE_ID,
                                           SYMBOL_TAB_ID,   SYMBOL_NULL_ID};
static replacement_arr REPL_TAB_SEQ_4[] = {SYMBOL_NL_ID,    SYMBOL_SPACE_ID,
                                           SYMBOL_SPACE_ID, SYMBOL_SPACE_ID,
                                           SYMBOL_SPACE_ID, SYMBOL_NULL_ID};
static replacement_arr REPL_TAB_REP[] = {SYMBOL_NL_ID, SYMBOL_TAB_ID,
                                         SYMBOL_NULL_ID};

static const ReplacementRule REPLACEMENT_RULES[REPLACEMENT_RULE_COUNT] = {
    // beginning of line tab replacements
    {.sequence = REPL_TAB_SEQ_1, .seq_size = 3, .replacement = REPL_TAB_REP},
    {.sequence = REPL_TAB_SEQ_2, .seq_size = 4, .replacement = REPL_TAB_REP},
    {.sequence = REPL_TAB_SEQ_3, .seq_size = 5, .replacement = REPL_TAB_REP},
    {.sequence = REPL_TAB_SEQ_4, .seq_size = 5, .replacement = REPL_TAB_REP},
};

// TODO: test
int sequence_matches(Token *token, const unsigned int *sequence) {
  unsigned int i = 0;
  while (token && *(sequence + i) != SYMBOL_NULL_ID &&
         token->symbol->id == *(sequence + i)) {
    i++;
    token = token->next;
  }
  if (*(sequence + i) == SYMBOL_NULL_ID) {
    return 1;
  }
  return 0;
}

// TODO: Test
/*
 * This will replace tokens beginning at the token immediately
 * AFTER the head_token you pass.
 */
void replace_sequence(Token *head_token, unsigned int seq_size,
                      const unsigned int *replacement) {
  Token *current = head_token;
  Token *tmp;
  unsigned int i;
  const Symbol *symbol;

  for (i = 0; i < seq_size; i++) {
    tmp = current->next;
    current->next = current->next->next;
    free_token(tmp);
  }

  i = 0;

  while (*(replacement + i) != SYMBOL_NULL_ID) {
    symbol = &(SYMBOLS[*(replacement + i)]);
    tmp = current->next;
    current->next = newToken(symbol, strdup(symbol->constant));
    current->next->next = tmp;
    current = current->next;
    i++;
  }
}

// TODO: test
Token *preprocess_tokens(Token *tokens) {
  Token *head = nullToken();
  head->next = tokens;
  Token *current = head;

  /*
   TODO: Change to prefix tree search, because this is reallllly gross
   We will just iterate through replacement rules at each position for now,
   but I should switch to prefix tree for performance at some point.
   */
  while (current) {
    for (unsigned int i = 0; i < REPLACEMENT_RULE_COUNT; i++) {
      if (sequence_matches(current->next, REPLACEMENT_RULES[i].sequence)) {
        replace_sequence(current, REPLACEMENT_RULES[i].seq_size,
                         REPLACEMENT_RULES[i].replacement);
        break;
      }
    }
    current = current->next;
  }

  current = head->next;
  free_token(head);
  return current;
}
