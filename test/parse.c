#include "../src/parse.h"

#include <stdlib.h>
#include <string.h>

#include "../src/ast.h"
#include "../src/symbols.h"
#include "../vendor/unity/unity.h"
#include "ast.h"

void test_produce_code_block_tab_start(void) {
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

  ASTNode *actual = produce_code_block(stream_ptr, 8);
  assert_ast_nodes_equal(expected, actual);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  free_token_list(stream);
  ast_free_node(expected);
  ast_free_node(actual);
}

void test_produce_code_block_space_tab_start(void) {
  Token *tokens[12] = {
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
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
  };

  Token *stream = join_token_array(tokens, 12);
  Token **stream_ptr = &stream;
  ASTNode *expected = ast_create_node(ASTN_CODE_BLOCK);
  expected->contents = strdup("foobar was here\n");
  Token *expected_ptr = advance_token_list_by(*stream_ptr, 9);

  ASTNode *actual = produce_code_block(stream_ptr, 9);
  assert_ast_nodes_equal(expected, actual);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  free_token_list(stream);
  ast_free_node(expected);
  ast_free_node(actual);
}

void test_produce_code_block_space_start(void) {
  Token *tokens[13] = {
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foobar"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "was"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "here"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "also"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "here"),
  };

  Token *stream = join_token_array(tokens, 13);
  Token **stream_ptr = &stream;
  ASTNode *expected = ast_create_node(ASTN_CODE_BLOCK);
  expected->contents = strdup("foobar was here\n");
  Token *expected_ptr = advance_token_list_by(*stream_ptr, 10);

  ASTNode *actual = produce_code_block(stream_ptr, 10);
  assert_ast_nodes_equal(expected, actual);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  free_token_list(stream);
  ast_free_node(expected);
  ast_free_node(actual);
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
  RUN_TEST(test_produce_code_block_tab_start);
  RUN_TEST(test_produce_code_block_space_tab_start);
  RUN_TEST(test_produce_code_block_space_start);
  printf("---matches_symbol_seq\n");
  RUN_TEST(test_matches_symbol_seq_code_block_seq1);
  RUN_TEST(test_matches_symbol_seq_code_block_seq2);
}
