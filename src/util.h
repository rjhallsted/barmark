#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

FILE *openFile(char *path);
char *str_append(char *dst, const char *s2);
char *repeat_x(char x, size_t times);

#endif  // UTIL_H
