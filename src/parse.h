#ifndef PARSE_H
#define PARSE_H

#include "ast.h"

// Parsing rules:
int is_double_newline(Symbol **symbols);
int is_indented(Symbol **symbols); 

#endif // PARSE_H
