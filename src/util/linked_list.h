#ifndef REDISC_LINKED_LIST_H__
#define REDISC_LINKED_LIST_H__

#include <stdbool.h>
#include <stddef.h>

// Node structure
typedef struct Node {
    void *data;
    struct Node *next;
} Node;

// Linked list structure
typedef struct {
    Node *head;
    size_t size;
} LinkedList;

// Function declarations
LinkedList* list_create(void);
Node* node_create(void *data);
bool list_push_front(LinkedList *list, void *data);
bool list_push_back(LinkedList *list, void *data);
void* list_pop_front(LinkedList *list);
void* list_get(LinkedList *list, size_t index);
bool list_is_empty(LinkedList *list);
size_t list_size(LinkedList *list);
void list_clear(LinkedList *list);
void list_destroy(LinkedList *list);
void list_foreach(LinkedList *list, void (*callback)(void *data));
bool list_remove(LinkedList *list, void *data);


#endif
