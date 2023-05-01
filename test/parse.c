#include "../src/parse.h"

#include <stdlib.h>
#include <string.h>

#include "../src/ast.h"
#include "../src/symbols.h"
#include "../vendor/unity/unity.h"

void assert_ast_nodes_equal(ASTNode *expected, ASTNode *actual) {
  TEST_ASSERT_EQUAL(expected->type, actual->type);
  if (expected->contents) {
    TEST_ASSERT_EQUAL_STRING(expected->contents, actual->contents);
  } else {
    TEST_ASSERT_NULL(expected->contents);
    TEST_ASSERT_NULL(actual->contents);
  }

  TEST_ASSERT_EQUAL(expected->children_count, actual->children_count);

  for (size_t i = 0; i < expected->children_count; ++i) {
    assert_ast_nodes_equal(expected->children[i], actual->children[i]);
  }
}

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
  Token *expected_ptr = advance_token_list_by(*stream_ptr, 11);

  ASTNode *actual = produce_code_block(stream_ptr);
  assert_ast_nodes_equal(expected, actual);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  free_token_list(stream);
  ast_free_node(expected);
  ast_free_node(actual);
}

void parseTests(void) {
  printf("running parse tests\n");
  RUN_TEST(test_produce_code_block_double_newline_ending);
}
