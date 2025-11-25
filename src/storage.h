#ifndef STORAGE_H__
#define STORAGE_H__

#include <stdbool.h>

typedef struct {

} Storage;

Storage* create_storage(void);
bool save_to_file(const char* path){}
bool load_from_file(const char* path){}


#endif
