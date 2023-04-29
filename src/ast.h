#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include "symbols.h"

enum AST_NODE_TYPES {
    ASTN_DOCUMENT = 1,
    ASTN_CODE_BLOCK,
};

typedef struct ASTNode {
    unsigned int type;
    char *contents;
    struct ASTNode **children;
    size_t children_count; 
} ASTNode;

ASTNode *ast_create_node(unsigned int type, const char *contents);
ASTNode *ast_create_node_from_sybmols(unsigned int type, Symbol **symbols, size_t symbols_count);
void ast_free_node(ASTNode *node);
void ast_add_child(ASTNode *parent, ASTNode *child);
ASTNode *ast_create_document(void);

#endif // AST_H