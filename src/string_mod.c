#include "string_mod.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *tab_expand(char const line[static 1], size_t line_pos,
                 int unsigned lookahead) {
  if (lookahead == 0) {
    return strdup(line);
  }
  size_t lookahead_bound = line_pos + lookahead;
  while (line[line_pos] == ' ' && line_pos < lookahead_bound - 1) {
    line_pos++;
  }
  if (line[line_pos] != '\t') {
    return strdup(line);
  }

  unsigned int spaces_to_add = 4 - (line_pos % 4);
  char *proposed = malloc(strlen(line) + spaces_to_add + 1);

  // copy first part
  strncpy(proposed, line, line_pos);

  // add spaces
  size_t j = line_pos;
  while (j < line_pos + spaces_to_add) {
    proposed[j] = ' ';
    j++;
  }

  // copy second part
  line_pos += 1;
  while (line[line_pos]) {
    proposed[j] = line[line_pos];
    j++;
    line_pos++;
  }

  // finish up
  proposed[j] = '\0';
  return proposed;
}

string_mod_ref make_unmodified_string_mod_ref(char *line[static 1]) {
  char *proposed = strdup(*line);
  string_mod_ref ref = {.orig = line, .proposed = proposed};
  return ref;
}

string_mod_ref begin_tab_expand_string_mod(char *line[static 1],
                                           size_t line_pos,
                                           int unsigned lookahead) {
  char *proposed = tab_expand(*line, line_pos, lookahead);
  string_mod_ref ref = {.orig = line, .proposed = proposed};
  return ref;
}

void tab_expand_existing_ref(string_mod_ref ref[static 1], size_t line_pos,
                             int unsigned lookahead) {
  char *new_version = tab_expand(ref->proposed, line_pos, lookahead);
  free(ref->proposed);
  ref->proposed = new_version;
}

void commit_string_mod(string_mod_ref ref) {
  free(*(ref.orig));
  *(ref.orig) = ref.proposed;
}

void abandon_string_mod(string_mod_ref ref) { free(ref.proposed); }
