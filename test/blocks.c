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

void test_tab_expand(void) {
  char *line = strdup("");

  tab_expand(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("", line);
  free(line);

  line = strdup(" ");
  tab_expand(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING(" ", line);
  free(line);

  line = strdup("  ");
  tab_expand(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("  ", line);
  free(line);

  line = strdup("   ");
  tab_expand(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("   ", line);
  free(line);

  line = strdup("    ");
  tab_expand(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    ", line);
  free(line);

  line = strdup("\tx");
  tab_expand(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    x", line);
  free(line);

  line = strdup(" \tx");
  tab_expand(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    x", line);
  free(line);

  line = strdup("  \tx");
  tab_expand(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    x", line);
  free(line);

  line = strdup("   \tx");
  tab_expand(&line, 0, 4);
  TEST_ASSERT_EQUAL_STRING("    x", line);
  free(line);

  line = strdup("    \tx");
  tab_expand(&line, 2, 4);
  TEST_ASSERT_EQUAL_STRING("        x", line);
  free(line);

  line = strdup("     \tx");
  tab_expand(&line, 2, 4);
  TEST_ASSERT_EQUAL_STRING("        x", line);
  free(line);

  line = strdup(" \tx");
  tab_expand(&line, 0, 1);
  TEST_ASSERT_EQUAL_STRING(" \tx", line);
  free(line);

  line = strdup(" \tx");
  tab_expand(&line, 0, 2);
  TEST_ASSERT_EQUAL_STRING("    x", line);
  free(line);

  line = strdup("  - foo");
  tab_expand(&line, 3, 1);
  TEST_ASSERT_EQUAL_STRING("  - foo", line);
  free(line);

  line = strdup("  -\tfoo");
  tab_expand(&line, 3, 1);
  TEST_ASSERT_EQUAL_STRING("  - foo", line);
  free(line);
}

void test_get_deepest_non_text_child(void) {
  ASTNode *root = ast_create_node(ASTN_DOCUMENT);
  ASTNode *node = root;

  ASTNode *deepest = get_deepest_non_text_child(root);
  TEST_ASSERT_EQUAL(ASTN_DOCUMENT, deepest->type);

  ASTNode *child = ast_create_node(ASTN_ORDERED_LIST);
  ast_add_child(node, child);
  node = child;
  deepest = get_deepest_non_text_child(root);
  TEST_ASSERT_EQUAL(ASTN_ORDERED_LIST, deepest->type);

  child = ast_create_node(ASTN_ORDERED_LIST_ITEM);
  ast_add_child(node, child);
  node = child;
  deepest = get_deepest_non_text_child(root);
  TEST_ASSERT_EQUAL(ASTN_ORDERED_LIST_ITEM, deepest->type);

  child = ast_create_node(ASTN_TEXT);
  ast_add_child(node, child);
  node = child;
  deepest = get_deepest_non_text_child(root);
  // Text doesn't get returned
  TEST_ASSERT_EQUAL(ASTN_ORDERED_LIST_ITEM, deepest->type);
}

// void test_nested_lists_construction(void) {
//   ASTNode *root = ast_create_node(ASTN_DOCUMENT);
//   char *line1 = strdup(" - foo\n");
//   add_line_to_ast(root, &line1);

//   TEST_ASSERT_EQUAL(1, root->children_count);
//   ASTNode *fooList = root->children[0];
//   TEST_ASSERT_EQUAL(ASTN_UNORDERED_LIST, fooList->type);
//   TEST_ASSERT_EQUAL_STRING("", fooList->cont_markers);
//   TEST_ASSERT_EQUAL(1, fooList->children_count);
//   ASTNode *fooItem = fooList->children[0];
//   TEST_ASSERT_EQUAL(ASTN_UNORDERED_LIST_ITEM, fooItem->type);
//   TEST_ASSERT_EQUAL_STRING("", fooItem->cont_markers);

//   // char *line2 = strdup("   - bar\n");
//   // add_line_to_ast(root, &line2);
// }

void run_blocks_tests(void) {
  printf("--blocks tests\n");
  printf("---matches_continuation_markers\n");
  RUN_TEST(test_matches_continuation_markers_matches);
  printf("---tab_expand\n");
  RUN_TEST(test_tab_expand);
  printf("---get_deepest_non_text_child\n");
  RUN_TEST(test_get_deepest_non_text_child);
  // printf("---construction tests\n");
  // RUN_TEST(test_nested_lists_construction);
}
