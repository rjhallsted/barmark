
#include "../vendor/unity/unity.h"
#include <stdio.h>
#include "symbols.h"


void setUp(void) {}

void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();
    symbolsTests();
    return UNITY_END();
}
