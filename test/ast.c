#include "../src/ast.h"

#include <stdlib.h>
#include <string.h>

#include "../vendor/unity/unity.h"

// void assert_strings_equal_or_both_null(const char *a, const char *b) {
//   if (a) {
//     TEST_ASSERT_EQUAL_STRING(a, b);
//   } else {
//     TEST_ASSERT_NULL(b);
//   }
// }

// void assert_ast_nodes_equal(ASTNode *expected, ASTNode *actual) {
//   TEST_ASSERT_EQUAL(expected->type, actual->type);
//   assert_strings_equal_or_both_null(expected->contents, actual->contents);

//   TEST_ASSERT_EQUAL(expected->children_count, actual->children_count);
//   for (size_t i = 0; i < expected->children_count; ++i) {
//     assert_ast_nodes_equal(expected->children[i], actual->children[i]);
//   }
//   TEST_ASSERT_EQUAL_PTR(expected->parent, actual->parent);
// }

void astTests(void) { printf("--ast tests\n"); }
