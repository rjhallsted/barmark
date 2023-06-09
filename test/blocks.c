#include "../src/blocks.h"

#include <stdlib.h>
#include <string.h>

#include "../src/ast.h"
#include "../vendor/unity/unity.h"

void test_matches_continuation_markers_matches(void) {
  ASTNode *node = ast_create_node(ASTN_PARAGRAPH);
  node->cont_markers = strdup("abcdef");
  size_t match_len = 0;
  char *line = "abcdefghi";
  TEST_ASSERT_TRUE(matches_continuation_markers(node, line, &match_len));
  TEST_ASSERT_EQUAL(6, match_len);

  char *line2 = "abcdef";
  TEST_ASSERT_TRUE(matches_continuation_markers(node, line2, &match_len));
  TEST_ASSERT_EQUAL(6, match_len);

  char *line3 = "abcde";
  TEST_ASSERT_FALSE(matches_continuation_markers(node, line3, &match_len));
  TEST_ASSERT_EQUAL(0, match_len);
  ast_free_node(node);
}

// void test_matches_continuation_markers_treats_4spaces_as_tab(void) {
//   ASTNode *node = ast_create_node(ASTN_PARAGRAPH);
//   node->cont_markers = strdup("\txyz");
//   size_t match_len = 0;
//   char *four_spaces = "    xyz";
//   TEST_ASSERT_TRUE(matches_continuation_markers(node, four_spaces,
//   &match_len)); TEST_ASSERT_EQUAL(7, match_len);

//   char *three_spaces = "   xyz";
//   TEST_ASSERT_FALSE(
//       matches_continuation_markers(node, three_spaces, &match_len));
//   TEST_ASSERT_EQUAL(0, match_len);

//   ast_free_node(node);
// }

void test_tab_expand(void) {
  char *line = strdup("");

  tab_expand(&line, 0);
  TEST_ASSERT_EQUAL_STRING("", line);
  free(line);

  line = strdup(" ");
  tab_expand(&line, 0);
  TEST_ASSERT_EQUAL_STRING(" ", line);
  free(line);

  line = strdup("  ");
  tab_expand(&line, 0);
  TEST_ASSERT_EQUAL_STRING("  ", line);
  free(line);

  line = strdup("   ");
  tab_expand(&line, 0);
  TEST_ASSERT_EQUAL_STRING("   ", line);
  free(line);

  line = strdup("    ");
  tab_expand(&line, 0);
  TEST_ASSERT_EQUAL_STRING("    ", line);
  free(line);

  line = strdup("\tx");
  tab_expand(&line, 0);
  TEST_ASSERT_EQUAL_STRING("    x", line);
  free(line);

  line = strdup(" \tx");
  tab_expand(&line, 0);
  TEST_ASSERT_EQUAL_STRING("    x", line);
  free(line);

  line = strdup("  \tx");
  tab_expand(&line, 0);
  TEST_ASSERT_EQUAL_STRING("    x", line);
  free(line);

  line = strdup("   \tx");
  tab_expand(&line, 0);
  TEST_ASSERT_EQUAL_STRING("    x", line);
  free(line);

  line = strdup("    \t x");
  tab_expand(&line, 2);
  TEST_ASSERT_EQUAL_STRING("       x", line);
  free(line);
}

void run_blocks_tests(void) {
  printf("--blocks tests\n");
  printf("---matches_continuation_markers\n");
  RUN_TEST(test_matches_continuation_markers_matches);
  // RUN_TEST(test_matches_continuation_markers_treats_4spaces_as_tab);
  printf("---tab_expand\n");
  RUN_TEST(test_tab_expand);
}
