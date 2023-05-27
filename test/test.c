
#include <stdio.h>

#include "../vendor/unity/unity.h"
#include "ast.h"
#include "parse_rule.h"
#include "symbols.h"
#include "util.h"

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  symbolsTests();
  astTests();
  parse_rule_tests();
  util_tests();
  return UNITY_END();
}
