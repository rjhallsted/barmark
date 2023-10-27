#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

FILE *openFile(char path[static 1]);
char *str_append(char dst[static 1], char const s2[static 1]);
char str_starts_with(char const str[static 1], char const sub[static 1]);
char str_starts_with_case_insensitive(char const str[static 1],
                                      char const sub[static 1]);
char *repeat_x(char x, size_t times);
bool array_contains(int unsigned const arr_size,
                    int unsigned const arr[arr_size], int unsigned value);
bool f_debug(void);
void print_tree(ASTNode node[static 1], size_t level);
bool is_whitespace(char c);
bool is_all_whitespace(char const line[static 1]);

#endif  // UTIL_H
