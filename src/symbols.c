#include <string.h>
#include <stdlib.h>
#include "symbols.h"

///////////////////////////////
// SYMBOL TREE STUFF
///////////////////////////////

SymbolTreeItem *newSymbolTreeItem(char c, const BaseSymbol *symbol)
{
    SymbolTreeItem *item = malloc(sizeof(SymbolTreeItem));
    item->c = c;
    item->symbol = symbol;
    item->children = NULL;
    item->children_count = 0;
    return item;
}

void addChildToSymbolTreeItem(SymbolTreeItem *item, SymbolTreeItem *child)
{
    item->children = realloc(item->children, (sizeof(SymbolTreeItem *) * (item->children_count + 1)));
    item->children[item->children_count] = child;
    item->children_count += 1;
}

SymbolTreeItem *findSymbolTreeItemChild(SymbolTreeItem *item, char c)
{
    for (unsigned int i = 0; i < item->children_count; i++)
    {
        if (item->children[i]->c == c)
        {
            return item->children[i];
        }
    }
    return NULL;
}

void addToSymbolTree(SymbolTreeItem *root, const BaseSymbol *symbol)
{
    SymbolTreeItem *current, *child;

    current = root;
    for (unsigned int i = 0; symbol->constant[i]; i++)
    {
        child = findSymbolTreeItemChild(current, symbol->constant[i]);
        if (!child)
        {
            child = newSymbolTreeItem(symbol->constant[i], NULL);
            addChildToSymbolTreeItem(current, child);
        }
        current = child;
    }
    current->symbol = symbol;
}

SymbolTreeItem *buildSymbolTree(void)
{
    SymbolTreeItem *root = newSymbolTreeItem('\0', &(BASE_SYMBOLS[BASE_SYMBOL_TEXT_ID]));
    const BaseSymbol *symbol;
    // we skip the null symbol at index 0;
    for (unsigned int i = 1; i < BASE_SYMBOL_COUNT; i++)
    {
        symbol = &(BASE_SYMBOLS[i]);
        if (symbol->constant)
        {
            addToSymbolTree(root, symbol);
        }
    }
    return root;
}

void freeSymbolTree(SymbolTreeItem *root)
{
    for (unsigned int i = 0; i < root->children_count; i++)
    {
        freeSymbolTree(root->children[i]);
    }
    free(root);
}

///////////////////////////////
// SYMBOL LOOKUP STUFF
///////////////////////////////

unsigned int shouldStopLookAhead(const char *input, const BaseSymbol *symbol)
{
    if (strchr(symbol->lookAheadTerminators, *input))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

const BaseSymbol *lookupBaseSymbolInner(SymbolTreeItem *item, const char *input)
{
    SymbolTreeItem *child = findSymbolTreeItemChild(item, *input);
    const BaseSymbol *symbol = NULL;
    if (child)
    {
        symbol = lookupBaseSymbolInner(child, input + 1);
    }
    if (!symbol)
    {
        symbol = item->symbol;
    }
    return symbol;
}

/**
 * @brief find the next base symbol available
 *
 * @param tree A pointer to the root of the symbol tree to search
 * @param input A char * to begin lookup from
 * @param symbol A base symbol pointer to store the address of the found symbol in
 * @param contents A char ** to store the contents of the symbol in.
 *
 * @return A pointer to the next character after the found symbol. Returns null if at end of string
 */
const char *lookupBaseSymbol(SymbolTreeItem *tree, const char *input, const BaseSymbol **symbol, char **contents)
{
    *symbol = lookupBaseSymbolInner(tree, input);

    size_t offset;
    if ((*symbol)->lookAheadTerminators != NULL)
    {
        offset = 0;
        while (!shouldStopLookAhead(input + offset, *symbol))
        {
            offset += 1; // May need to modify this once lookahead is in terms of UTF-8 instead of ASCII
        }
    }
    else
    {
        offset = strlen((*symbol)->constant);
    }
    *contents = strndup(input, offset);

    if (offset > 0)
    {
        return input + offset;
    }
    else
    {
        return NULL;
    }
}

Symbol newSymbol(const BaseSymbol *base, char *contents)
{
    Symbol sym = {base, contents};
    return sym;
}

Symbol nullSymbol(void)
{
    return newSymbol(&(BASE_SYMBOLS[BASE_SYMBOL_NULL_ID]), strdup(""));
}
