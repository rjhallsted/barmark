#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

FILE *openFile(char *path);
char *str_append(char *dst, char const *const s2);
char str_starts_with(char const *const str, char const *const sub);
char *repeat_x(char x, size_t times);
int array_contains(int unsigned const *const arr, int unsigned const arr_size,
                   int unsigned value);
int f_debug(void);

#endif  // UTIL_H
