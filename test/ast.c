#include "../src/ast.h"

#include <stdlib.h>
#include <string.h>

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

void test_ast_get_next_node_basic_code_block(void) {
  Token *tokens[11] = {
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "bim"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
  };

  Token *stream = join_token_array(tokens, 8);
  Token **stream_ptr = &stream;
  ASTNode *expected = ast_create_node(ASTN_CODE_BLOCK);
  expected->contents = strdup("foo\tbaz\t\tbim\n");
  Token *expected_ptr = advance_token_list_by(*stream_ptr, 8);

  ASTNode *actual = ast_get_next_node(stream_ptr);
  assert_ast_nodes_equal(expected, actual);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  free_token_list(stream);
  ast_free_node(expected);
  ast_free_node(actual);
}

void astTests(void) {
  printf("running ast tests\n");
  printf("---join_token_contents\n");
  RUN_TEST(test_join_token_contents_joins_correctly);
  printf("---ast_get_next_node\n");
  RUN_TEST(test_ast_get_next_node_basic_code_block);
}
