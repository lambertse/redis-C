#include "skip_list.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define MAX_LEVEL 32
#define P_VALUE 0.25  // Lower P = flatter tree = better cache performance

// Node structure with flexible array member
struct SkipListNode {
    void* value;
    int level;
    SkipListNode* next[1]; // Flexible array member
};

// SkipList structure
struct SkipList {
    SkipListNode* head;
    int current_max_level;
    CompareFunc compare;
    FreeFunc free_value;
    CopyFunc copy_value;
    
    // Fast random state (LCG)
    uint32_t rng_state;
    
    // Reusable update cache
    SkipListNode* update_cache[MAX_LEVEL];
};

// ===== Internal Helper Functions =====

// Fast Linear Congruential Generator (Numerical Recipes parameters)
static inline uint32_t fast_rand(SkipList* list) {
    list->rng_state = list->rng_state * 1664525u + 1013904223u;
    return list->rng_state;
}

// Generate random level using bit manipulation
// Much faster than calling random() multiple times
static inline int random_level(SkipList* list) {
    int level = 1;
    uint32_t rnd = fast_rand(list);
    
    // Use bit manipulation: for P=0.25, check 2 bits at a time
    // Both bits must be 0 to advance to next level
    while ((rnd & 3) == 0 && level < MAX_LEVEL) {
        level++;
        rnd >>= 2;
    }
    return level;
}

// Create a new node with specified level
static SkipListNode* node_create(void* value, int level) {
    // Allocate space for node + additional next pointers
    size_t node_size = sizeof(SkipListNode) + (level - 1) * sizeof(SkipListNode*);
    SkipListNode* node = (SkipListNode*)malloc(node_size);
    
    if (!node) {
        return NULL;
    }
    
    node->value = value;
    node->level = level;
    memset(node->next, 0, level * sizeof(SkipListNode*));
    
    return node;
}

// Free a node
static void node_destroy(SkipListNode* node, FreeFunc free_value) {
    if (node && free_value) {
        free_value(node->value);
    }
    free(node);
}

// ===== Public API =====

// Create a new skip list
SkipList* skiplist_create(CompareFunc compare, FreeFunc free_value, CopyFunc copy_value) {
    if (!compare) {
        return NULL;
    }
    
    SkipList* list = (SkipList*)malloc(sizeof(SkipList));
    if (!list) {
        return NULL;
    }
    
    // Initialize head node
    list->head = node_create(NULL, MAX_LEVEL);
    if (!list->head) {
        free(list);
        return NULL;
    }
    
    list->current_max_level = 1;
    list->compare = compare;
    list->free_value = free_value;
    list->copy_value = copy_value;
    
    // Initialize RNG with time-based seed + address entropy
    list->rng_state = (uint32_t)time(NULL) ^ (uint32_t)(uintptr_t)list;
    if (list->rng_state == 0) {
        list->rng_state = 1; // Ensure non-zero state
    }
    
    return list;
}

// Destroy skip list and free all nodes
void skiplist_destroy(SkipList* list) {
    if (!list) {
        return;
    }
    
    SkipListNode* current = list->head;
    while (current) {
        SkipListNode* next = current->next[0];
        node_destroy(current, current == list->head ? NULL : list->free_value);
        current = next;
    }
    
    free(list);
}

// Search for a value with optimized loop
bool skiplist_contain(const SkipList* list, const void* value) {
    if (!list || !value) {
        return false;
    }
    
    SkipListNode* current = list->head;
    
    // Unroll search loop slightly for better branch prediction
    for (int level = list->current_max_level - 1; level >= 0; level--) {
        while (current->next[level]) {
            SkipListNode* next = current->next[level];
            int cmp = list->compare(next->value, value);
            
            if (cmp < 0) {
                current = next;
            } else if (cmp == 0) {
                return true;
            } else {
                break;
            }
        }
    }
    
    return false;
}

// Insert a value
bool skiplist_insert(SkipList* list, void* value) {
    if (!list || !value) {
        return false;
    }
    
    SkipListNode* current = list->head;
    
    // Combined search and predecessor tracking
    for (int level = list->current_max_level - 1; level >= 0; level--) {
        while (current->next[level]) {
            SkipListNode* next = current->next[level];
            int cmp = list->compare(next->value, value);
            
            if (cmp < 0) {
                current = next;
            } else if (cmp == 0) {
                return false;  // Duplicate found
            } else {
                break;
            }
        }
        list->update_cache[level] = current;
    }
    
    int new_level = random_level(list);
    
    // If new level exceeds current max, initialize higher levels
    if (new_level > list->current_max_level) {
        for (int level = list->current_max_level; level < new_level; level++) {
            list->update_cache[level] = list->head;
        }
        list->current_max_level = new_level;
    }
    
    // Copy value if copy function provided
    void* node_value = value;
    if (list->copy_value) {
        node_value = list->copy_value(value);
        if (!node_value) {
            return false;
        }
    }
    
    SkipListNode* new_node = node_create(node_value, new_level);
    if (!new_node) {
        if (list->copy_value && list->free_value) {
            list->free_value(node_value);
        }
        return false;
    }
    
    // Link new node - unroll first few iterations for better performance
    if (new_level >= 1) {
        new_node->next[0] = list->update_cache[0]->next[0];
        list->update_cache[0]->next[0] = new_node;
    }
    if (new_level >= 2) {
        new_node->next[1] = list->update_cache[1]->next[1];
        list->update_cache[1]->next[1] = new_node;
    }
    for (int level = 2; level < new_level; level++) {
        new_node->next[level] = list->update_cache[level]->next[level];
        list->update_cache[level]->next[level] = new_node;
    }
    
    return true;
}

// Delete a value
bool skiplist_erase(SkipList* list, const void* value) {
    if (!list || !value) {
        return false;
    }
    
    SkipListNode* current = list->head;
    bool found = false;

    // Find node and track predecessors
    for (int level = list->current_max_level - 1; level >= 0; level--) {
      while (current->next[level]) {
        SkipListNode *next = current->next[level];
        int cmp = list->compare(next->value, value);

        if (cmp < 0) {
          current = next;
        } else if (cmp == 0) {
          found = true;
          break;
        } else {
          break;
        }
      }
      list->update_cache[level] = current;
    }

    if (!found) {
      return false;
    }

    SkipListNode *node_to_delete = list->update_cache[0]->next[0];

    // Unlink from all levels
    for (int level = 0; level < node_to_delete->level; level++) {
      list->update_cache[level]->next[level] = node_to_delete->next[level];
    }

    node_destroy(node_to_delete, list->free_value);

    // Update current max level
    while (list->current_max_level > 1 &&
           list->head->next[list->current_max_level - 1] == NULL) {
      list->current_max_level--;
    }

    return true;
}
