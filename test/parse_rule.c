#include "../src/parse_rule.h"

#include <stdlib.h>
#include <string.h>

#include "../src/symbols.h"
#include "../vendor/unity/unity.h"

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
  TEST_ASSERT_TRUE(m_text_line(stream_ptr));
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

  free_token_list(stream);
}

void test_m_text_line_text_start(void) {
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
  TEST_ASSERT_TRUE(m_text_line(stream_ptr));
  TEST_ASSERT_EQUAL_PTR(expected_ptr, *stream_ptr);

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

  TEST_ASSERT_FALSE(m_text_line(stream_ptr));
  TEST_ASSERT_EQUAL_PTR(stream, *stream_ptr);

  free_token_list(stream);
}

void parse_rule_tests(void) {
  printf("--Parse Rule tests\n");
  printf("---m_text_line\n");
  RUN_TEST(test_m_text_line_space_start);
  RUN_TEST(test_m_text_line_text_start);
  RUN_TEST(test_m_text_line_text_no_newline);
}
