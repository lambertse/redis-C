#ifndef REDIS_C_STORAGE_H__
#define REDIS_C_STORAGE_H__

#include "redis-C/rc.h"
#include "util/linked_list.h"
#include <stdbool.h>


REDIS_RC init_storage(void);
REDIS_RC save_to_file(const char* path);
REDIS_RC load_from_file(const char* path);

REDIS_RC create_cms_store(const char* sketch_name);

#endif
