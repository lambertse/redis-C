#include "storage.h"
#include "data_structure/count_min_sketch.h"
#include "redis-C/rc.h"
#include "util/linked_list.h"
#include <_stdio.h>
#include <_string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *name;
  void *container;
} storage_node_t;

static bool g_initialized = false;
static LinkedList *g_cms_storage = NULL;

REDIS_RC init_storage(void) {
  if (g_initialized) {
    printf("g_storage is already exists\n");
    return REDIS_OK;
  }
  
  g_cms_storage = (LinkedList*)malloc(sizeof(LinkedList));

  return REDIS_OK;
}

REDIS_RC save_to_file(const char *path) { return REDIS_OK; }
REDIS_RC load_from_file(const char *path) { return REDIS_OK; }

REDIS_RC create_cms_store(const char* sketch_name) {
  Node* prev = NULL;
  Node *head = g_cms_storage->head;
  while (head) {
    storage_node_t *node_val = (storage_node_t *)(head->data);
    if (strcmp(sketch_name, node_val->name) == 0 /* TODO: error here*/) {
      return REDIS_CMS_SKETCH_EXISTED;
    }
    prev = head;
    head = head->next;
  }
  CountMinSketch *cms = malloc(sizeof(CountMinSketch));
  cms_init_by_dim(cms, 100, 5);

  storage_node_t *new_sketch = (storage_node_t *)malloc(sizeof(storage_node_t));
  new_sketch->name = malloc(strlen(sketch_name));
  strcpy(new_sketch->name, sketch_name);
  new_sketch->container = cms;

  if (prev) {
    prev->next = (Node *)malloc(sizeof(Node));
    prev->next->data = new_sketch;
    prev->next->next = NULL;
  } else {
    g_cms_storage->head = (Node *)malloc(sizeof(Node));
    g_cms_storage->head->data = new_sketch;
    g_cms_storage->head->next = NULL;
  }

  return REDIS_OK;
}
