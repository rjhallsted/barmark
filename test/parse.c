#include "../src/parse.h"

#include <stdlib.h>
#include <string.h>

#include "../src/ast.h"
#include "../src/symbols.h"
#include "../vendor/unity/unity.h"

void test_produce_code_block_double_newline_ending(void) {
  Token *tokens[15] = {
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foobar"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "was"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "here"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "also"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "here"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "not_included"),
  };

  Token *stream = join_token_array(tokens, 15);
  Token **stream_ptr = &stream;
  ASTNode *expected = ast_create_node(ASTN_CODE_BLOCK);
  expected->contents = strdup("foobar was here\nalso here");
  printf("setup expected node\n");
  Token *expected_ptr = advance_token_list_by(*stream_ptr, 13);

  ASTNode *actual = produce_code_block(stream_ptr);
  printf("created actual node\n");
  TEST_ASSERT_TRUE(ast_nodes_equal(expected, actual));
  printf("compared nodes\n");
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);
}

void parseTests(void) {
  printf("running parse tests\n");
  RUN_TEST(test_produce_code_block_double_newline_ending);
}
