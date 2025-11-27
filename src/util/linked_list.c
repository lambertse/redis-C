#include "linked_list.h"
#include <stdlib.h>

// Create a new linked list
LinkedList* list_create(void) {
    LinkedList *list = (LinkedList*)malloc(sizeof(LinkedList));
    if (list) {
        list->head = NULL;
        list->size = 0;
    }
    return list;
}

// Create a new node
Node* node_create(void *data) {
    Node *node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->data = data;
        node->next = NULL;
    }
    return node;
}

// Push to front
bool list_push_front(LinkedList *list, void *data) {
    if (!list) return false;
    
    Node *new_node = node_create(data);
    if (!new_node) return false;
    
    new_node->next = list->head;
    list->head = new_node;
    list->size++;
    return true;
}

// Push to back
bool list_push_back(LinkedList *list, void *data) {
    if (!list) return false;
    
    Node *new_node = node_create(data);
    if (!new_node) return false;
    
    if (!list->head) {
        list->head = new_node;
    } else {
        Node *current = list->head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }
    list->size++;
    return true;
}

// Pop from front
void* list_pop_front(LinkedList *list) {
    if (!list || !list->head) return NULL;
    
    Node *temp = list->head;
    void *data = temp->data;
    list->head = list->head->next;
    free(temp);
    list->size--;
    return data;
}

// Get element at index
void* list_get(LinkedList *list, size_t index) {
    if (!list || index >= list->size) return NULL;
    
    Node *current = list->head;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    return current->data;
}

// Check if list is empty
bool list_is_empty(LinkedList *list) {
    return !list || list->head == NULL;
}

// Get size
size_t list_size(LinkedList *list) {
    return list ? list->size : 0;
}

// Clear all nodes
void list_clear(LinkedList *list) {
    if (!list) return;
    
    Node *current = list->head;
    while (current) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->size = 0;
}

// Destroy list
void list_destroy(LinkedList *list) {
    if (!list) return;
    list_clear(list);
    free(list);
}

// For-each iteration (callback style)
void list_foreach(LinkedList *list, void (*callback)(void *data)) {
    if (!list || !callback) return;
    
    Node *current = list->head;
    while (current) {
        callback(current->data);
        current = current->next;
    }
}

// Remove first occurrence of matching data
bool list_remove(LinkedList *list, void *data) {
    if (!list || !list->head) return false;
    
    // Check head
    if (list->head->data == data) {
        Node *temp = list->head;
        list->head = list->head->next;
        free(temp);
        list->size--;
        return true;
    }
    
    // Check rest
    Node *current = list->head;
    while (current->next) {
        if (current->next->data == data) {
            Node *temp = current->next;
            current->next = current->next->next;
            free(temp);
            list->size--;
            return true;
        }
        current = current->next;
    }
    return false;
}
