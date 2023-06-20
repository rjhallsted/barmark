#include "../src/util.h"

#include <stdlib.h>
#include <string.h>

#include "../vendor/unity/unity.h"

void test_str_append(void) {
  char *dst = strdup("");
  const char *str1 = "Hello, ";
  const char *str2 = "World!";

  str_append(dst, str1);
  TEST_ASSERT_EQUAL_STRING("Hello, ", dst);

  str_append(dst, str2);
  TEST_ASSERT_EQUAL_STRING("Hello, World!", dst);
  free(dst);
}

void test_str_append_with_nl(void) {
  char *dst = strdup("This is a paragraph.\n");
  const char *str1 = "Same paragraph";

  str_append(dst, str1);
  TEST_ASSERT_EQUAL_STRING("This is a paragraph.\nSame paragraph", dst);
  free(dst);
}

void util_tests(void) {
  printf("--util tests\n");
  printf("---str_append\n");
  RUN_TEST(test_str_append);
  RUN_TEST(test_str_append_with_nl);
}
