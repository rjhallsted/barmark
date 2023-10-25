#ifndef STRING_MOD_H
#define STRING_MOD_H

#include <stdlib.h>

typedef struct string_mod_ref {
  char **orig;
  char *proposed;
} string_mod_ref;

string_mod_ref make_unmodified_string_mod_ref(char *line[static 1]);
string_mod_ref begin_tab_expand_string_mod(char *line[static 1], size_t line_pos,
                                int unsigned lookahead);
void tab_expand_existing_ref(string_mod_ref ref[static 1], size_t line_pos,
                         int unsigned lookahead);
void commit_string_mod(string_mod_ref ref);
void abandon_string_mod(string_mod_ref ref);

#endif  // STRING_MOD_H
