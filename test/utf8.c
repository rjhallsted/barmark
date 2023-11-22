#include "../src/utf8.h"

#include <stdlib.h>
#include <string.h>

#include "../vendor/unity/unity.h"

void test_utf_char_len(void) {
  unsigned char i;
  char *str = strdup("       ");
  for (i = 0; i < 128; i++) {
    sprintf(str, "%c", (char)i);
    TEST_ASSERT_EQUAL(1, utf8_char_len(str));
  }
  for (i = 128; i < 192; i++) {
    sprintf(str, "%c", (char)i);
    TEST_ASSERT_EQUAL(0, utf8_char_len(str));
  }
  for (i = 192; i < 224; i++) {
    sprintf(str, "%c", (char)i);
    TEST_ASSERT_EQUAL(2, utf8_char_len(str));
  }
  for (i = 224; i < 240; i++) {
    sprintf(str, "%c", (char)i);
    TEST_ASSERT_EQUAL(3, utf8_char_len(str));
  }
  for (i = 240; i < 248; i++) {
    sprintf(str, "%c", (char)i);
    TEST_ASSERT_EQUAL(4, utf8_char_len(str));
  }
  for (i = 248; i < 252; i++) {
    sprintf(str, "%c", (char)i);
    TEST_ASSERT_EQUAL(5, utf8_char_len(str));
  }
  for (i = 252; i < 254; i++) {
    sprintf(str, "%c", (char)i);
    TEST_ASSERT_EQUAL(6, utf8_char_len(str));
  }
  free(str);
}

void test_utf_char(void) {
  int unsigned len = 0;
  char *input = "foo";
  codepoint res = utf8_char(input, &len);
  TEST_ASSERT_EQUAL(1, len);
  TEST_ASSERT_EQUAL(0x0066, res);

  input = "\xC4\x8E";
  res = utf8_char(input, &len);
  TEST_ASSERT_EQUAL(2, len);
  TEST_ASSERT_EQUAL(0x010E, res);

  input = "\xE0\xAA\x89";
  res = utf8_char(input, &len);
  TEST_ASSERT_EQUAL(3, len);
  TEST_ASSERT_EQUAL(0x0A89, res);

  input = "\xF0\x90\x8D\x84";
  res = utf8_char(input, &len);
  TEST_ASSERT_EQUAL(4, len);
  TEST_ASSERT_EQUAL(0x10344, res);
}

void utf8_tests(void) {
  printf("--utf8 tests\n");
  printf("---utf_char\n");
  RUN_TEST(test_utf_char_len);
  RUN_TEST(test_utf_char);
}
