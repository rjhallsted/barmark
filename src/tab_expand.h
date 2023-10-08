#ifndef TAB_EXPAND_H
#define TAB_EXPAND_H

#include <stdlib.h>

typedef struct tab_expand_ref {
  char **orig;
  char *proposed;
} tab_expand_ref;

tab_expand_ref begin_tab_expand(char *line[static 1], size_t line_pos,
                                size_t lookahead);
void commit_tab_expand(tab_expand_ref ref);
void reject_tab_expand(tab_expand_ref ref);

#endif  // TAB_EXPAND_H
