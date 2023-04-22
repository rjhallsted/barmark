#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>

char* addNullTerm(char* str, size_t len);
char* barmarkGetLine(FILE *fd);
FILE* openFile(char *path);


#endif // UTIL_H