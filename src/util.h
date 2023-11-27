#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

typedef struct {
  size_t start;
  size_t len;
} slice;

FILE *openFile(char const path[static 1]);
char *str_append(char dst[static 1], char const s2[static 1]);
char str_starts_with(char const str[static 1], char const sub[static 1]);
char str_starts_with_case_insensitive(char const str[static 1],
                                      char const sub[static 1]);
char *repeat_x(char x, size_t times);
bool array_contains(int unsigned const arr_size,
                    int unsigned const arr[arr_size], int unsigned value);
bool f_debug(void);
void print_tree(ASTNode node[static 1], size_t level);
bool is_whitespace(char c);
bool is_all_whitespace(char const line[static 1]);

// interface for singly-linked list functions
typedef struct SinglyLinkedItem {
  struct SinglyLinkedItem *next;
} SinglyLinkedItem;

SinglyLinkedItem *add_item_to_list(SinglyLinkedItem *head_ptr,
                                   SinglyLinkedItem *item);
SinglyLinkedItem *last_item_of_list(SinglyLinkedItem *head);
SinglyLinkedItem *reverse_list(SinglyLinkedItem *head);
size_t item_distance(SinglyLinkedItem *from, SinglyLinkedItem *to);
size_t list_len(SinglyLinkedItem *head);

// interface for doubly-linked list functions
typedef struct DoublyLinkedItem {
  struct DoublyLinkedItem *next;
  struct DoublyLinkedItem *prev;
} DoublyLinkedItem;

DoublyLinkedItem *add_dl_item_to_list(DoublyLinkedItem *head_ptr,
                                      DoublyLinkedItem *item);

#endif  // UTIL_H
