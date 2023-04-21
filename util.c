#include <stdlib.h>

char* addNullTerm(char* str, size_t len) {
    str = realloc(str, len+1);
    str[len] = '\0';
    return str;
}