#ifndef SKIPLIST_H__
#define SKIPLIST_H__

#include <stdbool.h>

// Forward declarations
typedef struct SkipListNode SkipListNode;
typedef struct SkipList SkipList;

// Function pointer types for generic value handling
typedef int (*CompareFunc)(const void *a, const void *b);
typedef void (*FreeFunc)(void *value);
typedef void *(*CopyFunc)(const void *value);

SkipList *skiplist_create(CompareFunc compare, FreeFunc free_value,
                          CopyFunc copy_value);
bool skiplist_contain(const SkipList *list, const void *value);
bool skiplist_insert(SkipList *list, void *value);
bool skiplist_erase(SkipList *list, const void *value);
void skiplist_destroy(SkipList* list);

#endif // SKIPLIST_H
