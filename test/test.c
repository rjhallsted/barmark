
#include <stdio.h>

#include "../vendor/unity/unity.h"
#include "ast.h"
#include "parse.h"
#include "symbols.h"

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  symbolsTests();
  astTests();
  // parseTests();
  return UNITY_END();
}
