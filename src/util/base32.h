#ifndef REDIS_C_BASE32_H__
#define REDIS_C_BASE32_H__

#include <stdint.h>
#include <stdlib.h>

char* base32_encode(const uint8_t* input, size_t size);
uint8_t* base32_decode(const char* input, size_t* size);

#endif

