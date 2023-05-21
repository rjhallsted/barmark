#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *openFile(char *path) {
  FILE *fd = fopen(path, "r");
  if (fd == NULL) {
    printf("Error opening file: '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  return fd;
}

char *str_append(char *dst, const char *s2) {
  size_t len = strlen(dst) + strlen(s2) + 1;
  char *output = realloc(dst, len);
  output[0] = '\0';
  strlcat(output, s2, len);
  return output;
}
