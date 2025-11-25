#ifndef REDIS_C_CONFIG_H__
#define REDIS_C_CONFIG_H__

#include <stdbool.h>

#define REDIS_C_DEFAULT_PORT 8091
#define REDIS_C_DEFAULT_HOST "localhost"

typedef struct {
    int port;
} RedisCConfig;

RedisCConfig* create_config(int port); 
RedisCConfig* get_current_config();
bool set_config(RedisCConfig* conf);

#endif
