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

void test_produce_code_block_newline_missing_tab_ending(void) {
  Token *tokens[14] = {
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foobar"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "was"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "here"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "also"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "here"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "not_included"),
  };

  Token *stream = join_token_array(tokens, 14);
  Token **stream_ptr = &stream;
  ASTNode *expected = ast_create_node(ASTN_CODE_BLOCK);
  expected->contents = strdup("foobar was here");
  Token *expected_ptr = advance_token_list_by(*stream_ptr, 6);

  ASTNode *actual = produce_code_block(stream_ptr);
  assert_ast_nodes_equal(expected, actual);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  free_token_list(stream);
  ast_free_node(expected);
  ast_free_node(actual);
}

void test_produce_code_block_end_of_stream_ending(void) {
  Token *tokens[10] = {
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
  };

  Token *stream = join_token_array(tokens, 10);
  Token **stream_ptr = &stream;
  ASTNode *expected = ast_create_node(ASTN_CODE_BLOCK);
  expected->contents = strdup("foobar was here\nalso ");

  ASTNode *actual = produce_code_block(stream_ptr);
  assert_ast_nodes_equal(expected, actual);
  TEST_ASSERT_NULL(*stream_ptr);

  free_token_list(stream);
  ast_free_node(expected);
  ast_free_node(actual);
}

void test_pr_is_standard_node_ender_double_nl(void) {
  Token *tokens[3] = {
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foobar"),
  };

  Token *stream = join_token_array(tokens, 3);
  TEST_ASSERT_TRUE(pr_is_standard_node_ender(stream));

  free_token_list(stream);
}

void test_pr_is_standard_node_ender_triple_nl(void) {
  Token *tokens[3] = {
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
  };

  Token *stream = join_token_array(tokens, 3);
  TEST_ASSERT_FALSE(pr_is_standard_node_ender(stream));

  free_token_list(stream);
}

void test_pr_is_standard_node_ender_double_nl_end_of_stream(void) {
  Token *tokens[2] = {
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
  };

  Token *stream = join_token_array(tokens, 2);
  TEST_ASSERT_FALSE(pr_is_standard_node_ender(stream));

  free_token_list(stream);
}

void test_matches_symbol_seq_code_block_seq1(void) {
  Token *tokens[5] = {newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
                      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
                      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
                      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "bar"),
                      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n")};

  Token *stream = join_token_array(tokens, 5);
  SymbolSeq seq = {CODE_BLOCK_SEQ1, CODE_BLOCK_SEQ1_SIZE};
  TEST_ASSERT_TRUE(matches_symbol_seq(stream, seq));

  Token *head = stream;
  stream = stream->next;
  free(head);
  TEST_ASSERT_FALSE(matches_symbol_seq(stream, seq));

  free_token_list(stream);
}

void test_matches_symbol_seq_code_block_seq2(void) {
  Token *tokens[8] = {newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
                      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
                      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
                      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
                      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
                      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
                      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "bar"),
                      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n")};

  Token *stream = join_token_array(tokens, 8);
  SymbolSeq seq = {CODE_BLOCK_SEQ2, CODE_BLOCK_SEQ2_SIZE};
  TEST_ASSERT_TRUE(matches_symbol_seq(stream, seq));

  Token *head = stream;
  stream = stream->next;
  free(head);
  TEST_ASSERT_FALSE(matches_symbol_seq(stream, seq));

  free_token_list(stream);
}

void parseTests(void) {
  printf("running parse tests\n");
  printf("---produce_code_block:\n");
  RUN_TEST(test_produce_code_block_double_newline_ending);
  RUN_TEST(test_produce_code_block_newline_missing_tab_ending);
  RUN_TEST(test_produce_code_block_end_of_stream_ending);
  printf("---pr_is_standard_node_ender:\n");
  RUN_TEST(test_pr_is_standard_node_ender_double_nl);
  RUN_TEST(test_pr_is_standard_node_ender_triple_nl);
  RUN_TEST(test_pr_is_standard_node_ender_double_nl_end_of_stream);
  printf("---matches_symbol_seq\n");
  RUN_TEST(test_matches_symbol_seq_code_block_seq1);
  RUN_TEST(test_matches_symbol_seq_code_block_seq2);
}
