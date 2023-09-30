#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *openFile(char const *const path) {
  FILE *fd = fopen(path, "r");
  if (fd == NULL) {
    printf("Error opening file: '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  return fd;
}

char *str_append(char *dst, char const *const s2) {
  size_t len = strlen(dst) + strlen(s2) + 1;
  char *output = realloc(dst, len);
  strlcat(output, s2, len);
  return output;
}

char str_starts_with(char const *const str, char const *const sub) {
  size_t i = 0;
  while (str[i] && sub[i] && str[i] == sub[i]) {
    i++;
  }
  if (sub[i] != '\0') {
    return 0;
  }
  return 1;
}

char *repeat_x(char x, size_t times) {
  char *out = malloc(times + 1);
  for (size_t i = 0; i < times; i++) {
    out[i] = x;
  }
  out[times] = '\0';
  return out;
}

int array_contains(int unsigned const *const arr, int unsigned const arr_size,
                   int unsigned value) {
  for (int unsigned i = 0; i < arr_size; i++) {
    if (arr[i] == value) {
      return 1;
    }
  }
  return 0;
}

int f_debug(void) {
  char *res = getenv("DEBUG");
  if (res) {
    return 1;
  }
  return 0;
}
