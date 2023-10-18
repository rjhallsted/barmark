#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *openFile(char const path[static 1]) {
  FILE *fd = fopen(path, "r");
  if (fd == NULL) {
    printf("Error opening file: '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  return fd;
}

char *str_append(char dst[static 1], char const s2[static 1]) {
  size_t len = strlen(dst) + strlen(s2) + 1;
  char *output = realloc(dst, len);
  strlcat(output, s2, len);
  return output;
}

char str_starts_with(char const str[static 1], char const sub[static 1]) {
  size_t i = 0;
  while (str[i] && sub[i] && str[i] == sub[i]) {
    i++;
  }
  if (sub[i] != '\0') {
    return 0;
  }
  return 1;
}

char str_starts_with_case_insensitive(char const str[static 1],
                                      char const sub[static 1]) {
  size_t i = 0;
  while (str[i] && sub[i] && tolower(str[i]) == tolower(sub[i])) {
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

bool array_contains(int unsigned const arr_size,
                    int unsigned const arr[arr_size], int unsigned value) {
  for (int unsigned i = 0; i < arr_size; i++) {
    if (arr[i] == value) {
      return true;
    }
  }
  return false;
}

// TODO: Change to preprocessor stuff so code paths that use this
// aren't included in builds when DEBUG is not set.
bool f_debug(void) {
  char *res = getenv("DEBUG");
  if (res) {
    return true;
  }
  return false;
}
