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
