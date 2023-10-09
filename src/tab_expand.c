#include "tab_expand.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *expand(char const line[static 1], size_t line_pos,
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

tab_expand_ref make_unmodified_tab_expand_ref(char *line[static 1]) {
  char *proposed = strdup(*line);
  tab_expand_ref ref = {.orig = line, .proposed = proposed};
  return ref;
}

tab_expand_ref begin_tab_expand(char *line[static 1], size_t line_pos,
                                int unsigned lookahead) {
  char *proposed = expand(*line, line_pos, lookahead);
  tab_expand_ref ref = {.orig = line, .proposed = proposed};
  return ref;
}

void expand_existing_ref(tab_expand_ref ref[static 1], size_t line_pos,
                         int unsigned lookahead) {
  char *new_version = expand(ref->proposed, line_pos, lookahead);
  free(ref->proposed);
  ref->proposed = new_version;
}

// TODO: Test
void commit_tab_expand(tab_expand_ref ref) {
  free(*(ref.orig));
  *(ref.orig) = ref.proposed;
}

void abandon_tab_expand(tab_expand_ref ref) { free(ref.proposed); }
