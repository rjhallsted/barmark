#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

FILE *openFile(char *path);
char *str_append(char *dst, const char *s2);
char str_starts_with(const char *str, const char *sub);
char *repeat_x(char x, size_t times);
int array_contains(const unsigned int *arr, unsigned int arr_size,
                   unsigned int value);
int f_debug(void);

#endif  // UTIL_H
