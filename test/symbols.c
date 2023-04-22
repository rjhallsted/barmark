#include "../vendor/unity/unity.h"
#include "../src/symbols.h"
#include <stdlib.h>

void test_constructSymbolTree_constructsCorrectly(void) {
    SymbolTreeItem *tree = buildSymbolTree();

    const BaseSymbol *actual;
    const BaseSymbol *found;
    char *contents;
    
    // test that it contains every base symbol
    // NOTE: Will cause problems if I ever add symbols other than "text" that have no constant
    for (unsigned int i = 1; i < BASE_SYMBOL_COUNT; i++) { // ignoring null symbol
        actual = &(BASE_SYMBOLS[i]);
        if (actual->constant) {
            lookupBaseSymbol(tree, actual->constant, &found, &contents);
            TEST_ASSERT_EQUAL(actual->id, found->id);
            TEST_ASSERT_EQUAL_STRING(actual->constant, contents);
        } else {
            lookupBaseSymbol(tree, "foobar", &found, &contents);
            TEST_ASSERT_EQUAL(actual->id, found->id);
            TEST_ASSERT_EQUAL_STRING("foobar", contents);
        }
        free(contents);
    }
    free(tree);
}

void test_lookupBaseSybmol_findsSymbolAndAdvancesPtrCorrectly(void) {
    SymbolTreeItem *tree = buildSymbolTree();
    const BaseSymbol *expected = &(BASE_SYMBOLS[BASE_SYMBOL_H2_ID]);
    const BaseSymbol *found;
    char *contents, *advanced;
    char *input = "## This is a heading";
    char *expectedAfterAdvance = " This is a heading";

    advanced = lookupBaseSymbol(tree, input, &found, &contents);
    TEST_ASSERT_EQUAL(expected->id, found->id);
    TEST_ASSERT_EQUAL_STRING("##", contents);
    TEST_ASSERT_EQUAL_STRING(expectedAfterAdvance, advanced);
}

void symbolsTests(void) {
    printf("running symbols tests\n");
    RUN_TEST(test_constructSymbolTree_constructsCorrectly);
    RUN_TEST(test_lookupBaseSybmol_findsSymbolAndAdvancesPtrCorrectly);
}
