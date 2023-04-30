#include "../src/symbols.h"

#include <stdlib.h>

#include "../vendor/unity/unity.h"

void test_constructSymbolTree_constructsCorrectly(void) {
  SymbolTreeItem *tree = buildSymbolTree();

  const Symbol *actual;
  const Symbol *found;
  char *contents;

  // test that it contains every base symbol
  // NOTE: Will cause problems if I ever add symbols other than "text" that have
  // no constant
  for (unsigned int i = 1; i < SYMBOL_COUNT; i++) {  // ignoring null symbol
    actual = &(SYMBOLS[i]);
    if (actual->constant) {
      lookupSymbol(tree, actual->constant, &found, &contents);
      TEST_ASSERT_EQUAL(actual->id, found->id);
      TEST_ASSERT_EQUAL_STRING(actual->constant, contents);
    } else {
      lookupSymbol(tree, "foobar", &found, &contents);
      TEST_ASSERT_EQUAL(actual->id, found->id);
      TEST_ASSERT_EQUAL_STRING("foobar", contents);
    }
    free(contents);
  }
  free(tree);
}

void test_lookupBaseSybmol_findsSymbolAndAdvancesPtrCorrectly(void) {
  SymbolTreeItem *tree = buildSymbolTree();
  const Symbol *expected = &(SYMBOLS[SYMBOL_H2_ID]);
  const Symbol *found;
  char *contents;
  const char *advanced;
  const char *input = "## This is a heading";
  const char *expectedAfterAdvance = " This is a heading";

  advanced = lookupSymbol(tree, input, &found, &contents);
  TEST_ASSERT_EQUAL(expected->id, found->id);
  TEST_ASSERT_EQUAL_STRING("##", contents);
  TEST_ASSERT_EQUAL_STRING(expectedAfterAdvance, advanced);
  free(contents);
  free(tree);
}

void symbolsTests(void) {
  printf("running symbols tests\n");
  RUN_TEST(test_constructSymbolTree_constructsCorrectly);
  RUN_TEST(test_lookupBaseSybmol_findsSymbolAndAdvancesPtrCorrectly);
}
