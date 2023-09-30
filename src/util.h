#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

FILE *openFile(char path[static 1]);
char *str_append(char dst[static 1], char const s2[static 1]);
char str_starts_with(char const str[static 1], char const sub[static 1]);
char *repeat_x(char x, size_t times);
bool array_contains(int unsigned const arr_size,
                    int unsigned const arr[arr_size], int unsigned value);
bool f_debug(void);

#endif  // UTIL_H
