#include "../src/blocks.h"

#include <stdlib.h>
#include <string.h>

#include "../src/ast.h"
#include "../vendor/unity/unity.h"

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
  printf("---get_deepest_non_text_child\n");
  RUN_TEST(test_get_deepest_non_text_child);
  // printf("---construction tests\n");
  // RUN_TEST(test_nested_lists_construction);
}
