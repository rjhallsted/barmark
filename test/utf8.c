#include "../src/utf8.h"

#include <stdlib.h>

#include "../vendor/unity/unity.h"

void test_utf_char(void) {
  int unsigned len = 0;
  unsigned char* input = "foo";
  codepoint res = utf8_char(input, &len);
  TEST_ASSERT_EQUAL(1, len);
  TEST_ASSERT_EQUAL(0x66, res);

  input = "\xC4\x8E";
  res = utf8_char(input, &len);
  TEST_ASSERT_EQUAL(2, len);
  TEST_ASSERT_EQUAL(0x10E, res);
}

void utf8_tests(void) {
  printf("--utf8 tests\n");
  printf("---utf_char\n");
  RUN_TEST(test_utf_char);
}
