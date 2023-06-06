#include <stdio.h>

#include "../vendor/unity/unity.h"
#include "ast.h"
#include "blocks.h"
#include "util.h"

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  astTests();
  run_blocks_tests();
  util_tests();
  return UNITY_END();
}
