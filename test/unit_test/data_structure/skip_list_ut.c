#define CTEST_IMPLEMENTATION
#include "ctest.h"
#include "data_structure/skip_list.h"

// ===== Helper Functions for Testing =====

// Compare doubles
int compare_double(const void *a, const void *b) {
  double da = *(const double *)a;
  double db = *(const double *)b;
  if (da < db)
    return -1;
  if (da > db)
    return 1;
  return 0;
}

// Copy double
void *copy_double(const void *value) {
  double *new_val = (double *)malloc(sizeof(double));
  if (new_val) {
    *new_val = *(const double *)value;
  }
  return new_val;
}

// Integer comparison (returns -1, 0, or 1)
int compare_int(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

// String comparison
int compare_string(const void* a, const void* b) {
    return strcmp((const char*)a, (const char*)b);
}

// Copy integer
void* copy_int(const void* value) {
    int* new_val = (int*)malloc(sizeof(int));
    if (new_val) {
        *new_val = *(const int*)value;
    }
    return new_val;
}

// Copy string
void* copy_string(const void* value) {
    return strdup((const char*)value);
}

// Generic free function
void free_generic(void* value) {
    free(value);
}

// ===== Basic Creation and Destruction Tests =====

TEST(SkipList, CreateNullCompare) {
  SkipList *list = skiplist_create(NULL, free_generic, copy_int);
  EXPECT_EQ(list, (SkipList *)NULL);
}

TEST(SkipList, DestroyNull) {
  skiplist_destroy(NULL);
  // Should not crash
  EXPECT_EQ(1, 1);
}

TEST(SkipList, DestroyEmpty) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  skiplist_destroy(list);
  // Should not crash
  EXPECT_EQ(1, 1);
}

// ===== Insert Tests =====

TEST(SkipList, InsertSingle) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;

  bool result = skiplist_insert(list, &value);
  EXPECT_EQ(result, true);
  EXPECT_EQ(skiplist_contain(list, &value), true);

  skiplist_destroy(list);
}

TEST(SkipList, InsertMultiple) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int values[] = {5, 2, 8, 1, 9, 3};

  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(skiplist_insert(list, &values[i]), true);
  }

  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(skiplist_contain(list, &values[i]), true);
  }

  skiplist_destroy(list);
}

TEST(SkipList, InsertDuplicate) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;

  EXPECT_EQ(skiplist_insert(list, &value), true);
  EXPECT_EQ(skiplist_insert(list, &value), false); // Duplicate

  skiplist_destroy(list);
}

TEST(SkipList, InsertSorted) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(skiplist_insert(list, &values[i]), true);
  }

  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(skiplist_contain(list, &values[i]), true);
  }

  skiplist_destroy(list);
}

TEST(SkipList, InsertReverseSorted) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int values[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(skiplist_insert(list, &values[i]), true);
  }

  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(skiplist_contain(list, &values[i]), true);
  }

  skiplist_destroy(list);
}

TEST(SkipList, InsertNull) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);

  EXPECT_EQ(skiplist_insert(NULL, NULL), false);
  EXPECT_EQ(skiplist_insert(list, NULL), false);

  skiplist_destroy(list);
}

TEST(SkipList, InsertLarge) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);

  // Insert 1000 elements
  for (int i = 0; i < 1000; i++) {
    EXPECT_EQ(skiplist_insert(list, &i), true);
  }

  // Verify all elements
  for (int i = 0; i < 1000; i++) {
    EXPECT_EQ(skiplist_contain(list, &i), true);
  }

  skiplist_destroy(list);
}

// ===== Contain Tests =====

TEST(SkipList, ContainEmpty) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;

  EXPECT_EQ(skiplist_contain(list, &value), false);

  skiplist_destroy(list);
}

TEST(SkipList, ContainNotFound) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int values[] = {1, 3, 5, 7, 9};
  int search = 4;

  for (int i = 0; i < 5; i++) {
    skiplist_insert(list, &values[i]);
  }

  EXPECT_EQ(skiplist_contain(list, &search), false);

  skiplist_destroy(list);
}

TEST(SkipList, ContainNull) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;
  skiplist_insert(list, &value);

  EXPECT_EQ(skiplist_contain(NULL, &value), false);
  EXPECT_EQ(skiplist_contain(list, NULL), false);

  skiplist_destroy(list);
}

TEST(SkipList, ContainBoundaries) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int values[] = {1, 5, 10};

  for (int i = 0; i < 3; i++) {
    skiplist_insert(list, &values[i]);
  }

  // Test boundaries
  int before = 0;
  int after = 15;
  EXPECT_EQ(skiplist_contain(list, &before), false);
  EXPECT_EQ(skiplist_contain(list, &after), false);
  EXPECT_EQ(skiplist_contain(list, &values[0]), true);
  EXPECT_EQ(skiplist_contain(list, &values[2]), true);

  skiplist_destroy(list);
}

// ===== Erase Tests =====

TEST(SkipList, EraseSingle) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;

  skiplist_insert(list, &value);
  EXPECT_EQ(skiplist_erase(list, &value), true);
  EXPECT_EQ(skiplist_contain(list, &value), false);

  skiplist_destroy(list);
}

TEST(SkipList, EraseMultiple) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int values[] = {1, 2, 3, 4, 5};

  for (int i = 0; i < 5; i++) {
    skiplist_insert(list, &values[i]);
  }

  EXPECT_EQ(skiplist_erase(list, &values[2]), true);
  EXPECT_EQ(skiplist_contain(list, &values[2]), false);

  EXPECT_EQ(skiplist_erase(list, &values[0]), true);
  EXPECT_EQ(skiplist_contain(list, &values[0]), false);

  EXPECT_EQ(skiplist_erase(list, &values[4]), true);
  EXPECT_EQ(skiplist_contain(list, &values[4]), false);

  // Remaining elements should still exist
  EXPECT_EQ(skiplist_contain(list, &values[1]), true);
  EXPECT_EQ(skiplist_contain(list, &values[3]), true);

  skiplist_destroy(list);
}

TEST(SkipList, EraseNotFound) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;
  int missing = 99;

  skiplist_insert(list, &value);
  EXPECT_EQ(skiplist_erase(list, &missing), false);

  skiplist_destroy(list);
}

TEST(SkipList, EraseEmpty) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;

  EXPECT_EQ(skiplist_erase(list, &value), false);

  skiplist_destroy(list);
}

TEST(SkipList, EraseNull) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;

  EXPECT_EQ(skiplist_erase(NULL, &value), false);
  EXPECT_EQ(skiplist_erase(list, NULL), false);

  skiplist_destroy(list);
}

TEST(SkipList, EraseAll) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int values[] = {1, 2, 3, 4, 5};

  for (int i = 0; i < 5; i++) {
    skiplist_insert(list, &values[i]);
  }

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(skiplist_erase(list, &values[i]), true);
  }

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(skiplist_contain(list, &values[i]), false);
  }

  skiplist_destroy(list);
}

TEST(SkipList, EraseReinsert) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;

  skiplist_insert(list, &value);
  EXPECT_EQ(skiplist_erase(list, &value), true);
  EXPECT_EQ(skiplist_insert(list, &value), true);
  EXPECT_EQ(skiplist_contain(list, &value), true);

  skiplist_destroy(list);
}

// ===== String Tests =====

TEST(SkipList, StringInsertAndContain) {
  SkipList *list = skiplist_create(compare_string, free_generic, copy_string);

  char *strings[] = {"apple", "banana", "cherry", "date", "elderberry"};

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(skiplist_insert(list, strings[i]), true);
  }

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(skiplist_contain(list, strings[i]), true);
  }

  char *missing = "fig";
  EXPECT_EQ(skiplist_contain(list, missing), false);

  skiplist_destroy(list);
}

TEST(SkipList, StringErase) {
  SkipList *list = skiplist_create(compare_string, free_generic, copy_string);

  char *str1 = "hello";
  char *str2 = "world";

  skiplist_insert(list, str1);
  skiplist_insert(list, str2);

  EXPECT_EQ(skiplist_erase(list, str1), true);
  EXPECT_EQ(skiplist_contain(list, str1), false);
  EXPECT_EQ(skiplist_contain(list, str2), true);

  skiplist_destroy(list);
}

TEST(SkipList, StringDuplicate) {
  SkipList *list = skiplist_create(compare_string, free_generic, copy_string);

  char *str = "duplicate";

  EXPECT_EQ(skiplist_insert(list, str), true);
  EXPECT_EQ(skiplist_insert(list, str), false);

  skiplist_destroy(list);
}

// ===== Double Tests =====

TEST(SkipList, DoubleInsert) {
  SkipList *list = skiplist_create(compare_double, free_generic, copy_double);

  double values[] = {3.14, 2.71, 1.41, 0.57, 9.99};

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(skiplist_insert(list, &values[i]), true);
  }

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(skiplist_contain(list, &values[i]), true);
  }

  skiplist_destroy(list);
}

TEST(SkipList, DoubleErase) {
  SkipList *list = skiplist_create(compare_double, free_generic, copy_double);

  double val1 = 3.14;
  double val2 = 2.71;

  skiplist_insert(list, &val1);
  skiplist_insert(list, &val2);

  EXPECT_EQ(skiplist_erase(list, &val1), true);
  EXPECT_EQ(skiplist_contain(list, &val1), false);

  skiplist_destroy(list);
}

// ===== Stress Tests =====

TEST(SkipList, StressInsertDelete) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);

  // Insert many elements
  for (int i = 0; i < 100; i++) {
    EXPECT_EQ(skiplist_insert(list, &i), true);
  }

  // Delete half
  for (int i = 0; i < 50; i++) {
    EXPECT_EQ(skiplist_erase(list, &i), true);
  }

  // Verify remaining
  for (int i = 0; i < 50; i++) {
    EXPECT_EQ(skiplist_contain(list, &i), false);
  }
  for (int i = 50; i < 100; i++) {
    EXPECT_EQ(skiplist_contain(list, &i), true);
  }

  skiplist_destroy(list);
}

TEST(SkipList, StressRandomOrder) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);

  int values[] = {47, 23, 91, 15, 68, 34, 82, 56, 29, 73};

  for (int i = 0; i < 10; i++) {
    skiplist_insert(list, &values[i]);
  }

  // Delete in different order
  int delete_order[] = {3, 7, 1, 9, 5};
  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(skiplist_erase(list, &values[delete_order[i]]), true);
  }

  // Verify
  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(skiplist_contain(list, &values[delete_order[i]]), false);
  }

  skiplist_destroy(list);
}

// ===== Edge Cases =====

TEST(SkipList, NegativeNumbers) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int values[] = {-5, -1, 0, 1, 5};

  for (int i = 0; i < 5; i++) {
    skiplist_insert(list, &values[i]);
  }

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(skiplist_contain(list, &values[i]), true);
  }

  skiplist_destroy(list);
}

TEST(SkipList, SingleElement) {
  SkipList *list = skiplist_create(compare_int, free_generic, copy_int);
  int value = 42;

  skiplist_insert(list, &value);
  EXPECT_EQ(skiplist_contain(list, &value), true);
  EXPECT_EQ(skiplist_erase(list, &value), true);
  EXPECT_EQ(skiplist_contain(list, &value), false);

  skiplist_destroy(list);
}

TEST(SkipList, EmptyStringTest) {
  SkipList *list = skiplist_create(compare_string, free_generic, copy_string);

  char *empty = "";
  char *nonempty = "hello";

  EXPECT_EQ(skiplist_insert(list, empty), true);
  EXPECT_EQ(skiplist_insert(list, nonempty), true);
  EXPECT_EQ(skiplist_contain(list, empty), true);
  EXPECT_EQ(skiplist_contain(list, nonempty), true);

  skiplist_destroy(list);
}

CTEST_MAIN()
