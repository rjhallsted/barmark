#include "../src/parse_rule.h"

#include <stdlib.h>
#include <string.h>

#include "../src/ast.h"
#include "../src/symbols.h"
#include "../vendor/unity/unity.h"

void test_m_wild(void) {
  Token *tokens[6] = {
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), "1"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), "2"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), "3"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), "4"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), "5"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
  };

  Token *stream = join_token_array(tokens, 6);
  Token **stream_ptr = &stream;

  // No restrictions
  Token *excpected_ptr = advance_token_list_by(stream, 5);
  ASTNode *res = m_wild(m_space, 0, SIZE_MAX, &stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(5, res->children_count);
  // check child contents:
  for (size_t i = 0; i < 5; i++) {
    TEST_ASSERT_EQUAL_STRING(tokens[i]->contents, res->children[i]->contents);
  }
  TEST_ASSERT_EQUAL_PTR(excpected_ptr, *stream_ptr);
  ast_free_node(res);

  // Max only
  stream_ptr = &stream;
  excpected_ptr = advance_token_list_by(stream, 3);
  res = m_wild(m_space, 0, 3, &stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(3, res->children_count);
  TEST_ASSERT_EQUAL_PTR(excpected_ptr, *stream_ptr);

  // Min only
  stream_ptr = &stream;
  excpected_ptr = stream;
  res = m_wild(m_space, 6, SIZE_MAX, &stream_ptr);
  TEST_ASSERT_NULL(res);
  TEST_ASSERT_EQUAL_PTR(excpected_ptr, *stream_ptr);

  // equal min & max
  stream_ptr = &stream;
  excpected_ptr = advance_token_list_by(stream, 4);
  res = m_wild(m_space, 4, 4, &stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(4, res->children_count);
  TEST_ASSERT_EQUAL_PTR(excpected_ptr, *stream_ptr);
  ast_free_node(res);

  free_token_list(stream);
}

void test_m_text_line_space_start(void) {
  Token *tokens[7] = {
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "bim"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "trailing"),
  };

  Token *stream = join_token_array(tokens, 7);
  Token **stream_ptr = &stream;

  Token *expected_ptr = advance_token_list_by(stream, 6);
  ASTNode *res = m_text_line(&stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(0, res->children_count);
  TEST_ASSERT_EQUAL_STRING(" foo bazbim\n", res->contents);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  free_token_list(stream);
}

void test_m_text_line_text_word_start(void) {
  Token *tokens[7] = {
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "bim"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "trailing"),
  };

  Token *stream = join_token_array(tokens, 7);
  Token **stream_ptr = &stream;

  Token *expected_ptr = advance_token_list_by(stream, 6);
  ASTNode *res = m_text_line(&stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(0, res->children_count);
  TEST_ASSERT_EQUAL_STRING("foo  bazbim\n", res->contents);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);
  ast_free_node(res);

  free_token_list(stream);
}

void test_m_text_line_text_no_newline(void) {
  Token *tokens[5] = {
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "bim"),
  };

  Token *stream = join_token_array(tokens, 5);
  Token **stream_ptr = &stream;

  ASTNode *res = m_text_line(&stream_ptr);
  TEST_ASSERT_NULL(res);
  TEST_ASSERT_EQUAL_PTR(stream, *stream_ptr);

  free_token_list(stream);
}

void test_m_opening_tab_all_spaces(void) {
  Token *tokens[5] = {
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
  };

  Token *stream = join_token_array(tokens, 5);
  Token **stream_ptr = &stream;
  Token *expected_ptr = advance_token_list_by(stream, 4);
  ASTNode *res = m_opening_tab(&stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(0, res->children_count);
  TEST_ASSERT_EQUAL_STRING("    ", res->contents);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  ast_free_node(res);
  free_token_list(stream);
}

void test_m_opening_tab_3_spaces_tab(void) {
  Token *tokens[5] = {
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
  };

  Token *stream = join_token_array(tokens, 5);
  Token **stream_ptr = &stream;
  Token *expected_ptr = advance_token_list_by(stream, 4);
  ASTNode *res = m_opening_tab(&stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(0, res->children_count);
  TEST_ASSERT_EQUAL_STRING("   \t", res->contents);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  ast_free_node(res);
  free_token_list(stream);
}

void test_m_opening_tab_tab_only(void) {
  Token *tokens[2] = {
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
  };

  Token *stream = join_token_array(tokens, 2);
  Token **stream_ptr = &stream;
  Token *expected_ptr = advance_token_list_by(stream, 1);
  ASTNode *res = m_opening_tab(&stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(0, res->children_count);
  TEST_ASSERT_EQUAL_STRING("\t", res->contents);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  ast_free_node(res);
  free_token_list(stream);
}

void test_m_opening_tab_all_3_spaces_no_tab(void) {
  Token *tokens[4] = {
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
  };

  Token *stream = join_token_array(tokens, 4);
  Token **stream_ptr = &stream;
  TEST_ASSERT_NULL(m_opening_tab(&stream_ptr));
  TEST_ASSERT_EQUAL_PTR(stream, *stream_ptr);

  free_token_list(stream);
}

void test_m_opening_tab_all_no_whitespace(void) {
  Token *tokens[2] = {
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
  };

  Token *stream = join_token_array(tokens, 2);
  Token **stream_ptr = &stream;
  TEST_ASSERT_NULL(m_opening_tab(&stream_ptr));
  TEST_ASSERT_EQUAL_PTR(stream, *stream_ptr);

  free_token_list(stream);
}

void test_m_code_block_one_line_tab_start(void) {
  Token *tokens[6] = {
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "bar"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
  };

  Token *stream = join_token_array(tokens, 6);
  Token **stream_ptr = &stream;
  Token *expected_ptr = advance_token_list_by(stream, 5);
  ASTNode *res = m_code_block(&stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(0, res->children_count);
  TEST_ASSERT_EQUAL_STRING("foo bar\n", res->contents);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  ast_free_node(res);
  free_token_list(stream);
}

void test_m_code_block_multiple_lines_mixed_starts(void) {
  Token *tokens[19] = {
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "bar"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "baz"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_SPACE_ID]), " "),
      newToken(&(SYMBOLS[SYMBOL_TAB_ID]), "\t"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "foo"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "bar"),
      newToken(&(SYMBOLS[SYMBOL_NL_ID]), "\n"),
      newToken(&(SYMBOLS[SYMBOL_TEXT_ID]), "fin"),
  };

  Token *stream = join_token_array(tokens, 19);
  Token **stream_ptr = &stream;
  Token *expected_ptr = advance_token_list_by(stream, 18);
  ASTNode *res = m_code_block(&stream_ptr);
  TEST_ASSERT_NOT_NULL(res);
  TEST_ASSERT_EQUAL(0, res->children_count);
  TEST_ASSERT_EQUAL_STRING("foo bar\nbaz\nfoobar\n", res->contents);
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  ast_free_node(res);
  free_token_list(stream);
}

void parse_rule_tests(void) {
  printf("--Parse Rule tests\n");
  printf("---m_wild\n");
  RUN_TEST(test_m_wild);
  printf("---m_text_line\n");
  RUN_TEST(test_m_text_line_space_start);
  RUN_TEST(test_m_text_line_text_word_start);
  RUN_TEST(test_m_text_line_text_no_newline);
  printf("---m_opening_tab\n");
  RUN_TEST(test_m_opening_tab_3_spaces_tab);
  RUN_TEST(test_m_opening_tab_all_spaces);
  RUN_TEST(test_m_opening_tab_tab_only);
  RUN_TEST(test_m_opening_tab_all_3_spaces_no_tab);
  RUN_TEST(test_m_opening_tab_all_no_whitespace);
  printf("---m_code_block\n");
  RUN_TEST(test_m_code_block_one_line_tab_start);
  RUN_TEST(test_m_code_block_multiple_lines_mixed_starts);
}
