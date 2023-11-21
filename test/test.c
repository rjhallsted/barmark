#include <stdio.h>

#include "../vendor/unity/unity.h"
#include "ast.h"
#include "blocks.h"
#include "string_mod.h"
#include "unicode.h"
#include "util.h"

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  astTests();
  run_blocks_tests();
  util_tests();
  string_mod_tests();
  unicode_tests();
  return UNITY_END();
}
