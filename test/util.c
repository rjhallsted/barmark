#include "../src/util.h"

#include <stdlib.h>
#include <string.h>

#include "../vendor/unity/unity.h"

void test_str_append(void) {
  char *dst = strdup("");
  const char *str1 = "Hello, ";
  const char *str2 = "World!";

  dst = str_append(dst, str1);
  TEST_ASSERT_EQUAL_STRING("Hello, ", dst);

  dst = str_append(dst, str2);
  TEST_ASSERT_EQUAL_STRING("Hello, World!", dst);
  free(dst);
}

void test_str_append_with_nl(void) {
  char *dst = strdup("This is a paragraph.\n");
  const char *str1 = "Same paragraph";

  dst = str_append(dst, str1);
  TEST_ASSERT_EQUAL_STRING("This is a paragraph.\nSame paragraph", dst);
  free(dst);
}

typedef struct TestList {
  struct TestList *next;
  struct TestList *prev;
  int value;
} TestList;

void test_reverse_list(void) {
  TestList *a = malloc(sizeof(TestList));
  a->value = 1;
  TestList *b = malloc(sizeof(TestList));
  b->value = 2;
  TestList *c = malloc(sizeof(TestList));
  c->value = 3;
  TestList *head = (TestList *)add_item_to_list((SinglyLinkedItem *)NULL,
                                                (SinglyLinkedItem *)a);
  head = (TestList *)add_item_to_list((SinglyLinkedItem *)head,
                                      (SinglyLinkedItem *)b);
  head = (TestList *)add_item_to_list((SinglyLinkedItem *)head,
                                      (SinglyLinkedItem *)c);

  TestList *new_head = (TestList *)reverse_list((SinglyLinkedItem *)c);
  TEST_ASSERT_EQUAL(1, new_head->value);
  TEST_ASSERT_EQUAL(2, new_head->next->value);
  TEST_ASSERT_EQUAL(3, new_head->next->next->value);
  free(a);
  free(b);
  free(c);
}

void test_reverse_dl_list(void) {
  TestList *a = malloc(sizeof(TestList));
  a->value = 1;
  TestList *b = malloc(sizeof(TestList));
  b->value = 2;
  TestList *c = malloc(sizeof(TestList));
  c->value = 3;
  TestList *head = (TestList *)add_item_to_dl_list((DoublyLinkedItem *)NULL,
                                                   (DoublyLinkedItem *)a);
  head = (TestList *)add_item_to_dl_list((DoublyLinkedItem *)head,
                                         (DoublyLinkedItem *)b);
  head = (TestList *)add_item_to_dl_list((DoublyLinkedItem *)head,
                                         (DoublyLinkedItem *)c);

  TestList *new_head = (TestList *)reverse_dl_list((DoublyLinkedItem *)c);
  TEST_ASSERT_EQUAL(1, new_head->value);
  TEST_ASSERT_EQUAL(1, new_head->next->prev->value);
  TEST_ASSERT_EQUAL(2, new_head->next->value);
  TEST_ASSERT_EQUAL(2, new_head->next->next->prev->value);
  TEST_ASSERT_EQUAL(3, new_head->next->next->value);
  free(a);
  free(b);
  free(c);
}

void util_tests(void) {
  printf("--util tests\n");
  printf("---str_append\n");
  RUN_TEST(test_str_append);
  RUN_TEST(test_str_append_with_nl);
  printf("---reverse_list\n");
  RUN_TEST(test_reverse_list);
  printf("---reverse_dl_list\n");
  RUN_TEST(test_reverse_dl_list);
}
