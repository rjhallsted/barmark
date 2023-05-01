#include "../src/ast.h"

#include <stdlib.h>

#include "../src/symbols.h"
#include "../vendor/unity/unity.h"

void test_join_token_contents_joins_correctly(void) {
  Token *tokens[8] = {
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foobar"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "was"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "here"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "not_included"),
  };
  Token *stream = join_token_array(tokens, 8);
  const char *expected = "foobar was here\n";

  char *actual = join_token_contents(stream, 6);

  TEST_ASSERT_EQUAL_STRING(expected, actual);

  free_token_list(stream);
  free(actual);
}

void astTests(void) {
  printf("running ast tests\n");
  RUN_TEST(test_join_token_contents_joins_correctly);
}
