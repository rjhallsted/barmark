#include "../src/ast.h"

#include <stdlib.h>
#include <string.h>

#include "../vendor/unity/unity.h"

void assert_ast_nodes_equal(ASTNode *expected, ASTNode *actual) {
  TEST_ASSERT_EQUAL(expected->type, actual->type);
  TEST_ASSERT_EQUAL(expected->open, actual->open);
  if (expected->contents) {
    TEST_ASSERT_EQUAL_STRING(expected->contents, actual->contents);
  } else {
    TEST_ASSERT_NULL(expected->contents);
    TEST_ASSERT_NULL(actual->contents);
  }

  TEST_ASSERT_EQUAL(expected->children_count, actual->children_count);

  for (size_t i = 0; i < expected->children_count; ++i) {
    assert_ast_nodes_equal(expected->children[i], actual->children[i]);
  }
}

void astTests(void) { printf("--ast tests\n"); }
