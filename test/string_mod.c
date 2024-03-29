#include "../src/string_mod.h"

#include <stdlib.h>
#include <string.h>

#include "../vendor/unity/unity.h"

void test_begin_tab_expand_string_mod(void) {
  char *line = strdup("");
  string_mod_ref res;

  res = begin_tab_expand_string_mod(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup(" ");
  res = begin_tab_expand_string_mod(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING(" ", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("  ");
  res = begin_tab_expand_string_mod(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("  ", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("   ");
  res = begin_tab_expand_string_mod(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("   ", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("    ");
  res = begin_tab_expand_string_mod(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    ", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("\tx");
  res = begin_tab_expand_string_mod(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    x", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup(" \tx");
  res = begin_tab_expand_string_mod(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    x", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("  \tx");
  res = begin_tab_expand_string_mod(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    x", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("   \tx");
  res = begin_tab_expand_string_mod(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    x", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("    \tx");
  res = begin_tab_expand_string_mod(&line, 2, 4);
  TEST_ASSERT_EQUAL_STRING("        x", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("     \tx");
  res = begin_tab_expand_string_mod(&line, 2, 4);
  TEST_ASSERT_EQUAL_STRING("        x", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup(" \tx");
  res = begin_tab_expand_string_mod(&line, 0, 1);
  TEST_ASSERT_EQUAL_STRING(" \tx", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup(" \tx");
  res = begin_tab_expand_string_mod(&line, 0, 2);
  TEST_ASSERT_EQUAL_STRING("    x", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("  - foo");
  res = begin_tab_expand_string_mod(&line, 3, 1);
  TEST_ASSERT_EQUAL_STRING("  - foo", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("  -\tfoo");
  res = begin_tab_expand_string_mod(&line, 3, 1);
  TEST_ASSERT_EQUAL_STRING("  - foo", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  // below this are the "bug-inspired" cases

  line = strdup("12345");
  res = begin_tab_expand_string_mod(&line, 2, 0);
  TEST_ASSERT_EQUAL_STRING("12345", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);

  line = strdup("          indented code\n");
  res = begin_tab_expand_string_mod(&line, 0, 0);
  TEST_ASSERT_EQUAL_STRING("          indented code\n", res.proposed);
  TEST_ASSERT_EQUAL_PTR(line, *res.orig);
  free(line);
  free(res.proposed);
}

void string_mod_tests(void) {
  printf("--string_mod tests\n");
  printf("---begin_tab_expand_string_mod\n");
  RUN_TEST(test_begin_tab_expand_string_mod);
}
