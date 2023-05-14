#ifndef AST_TEST_H
#define AST_TEST_H

#include "../src/ast.h"

void assert_ast_nodes_equal(ASTNode *expected, ASTNode *actual);
void astTests(void);

#endif  // AST_TEST_H
