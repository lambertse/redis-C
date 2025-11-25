#include "redis-C/config.h"
#include <stdlib.h>

static RedisCConfig *current_conf = NULL;

RedisCConfig *create_config(int port) {
  RedisCConfig *cfg = malloc(sizeof(RedisCConfig));
  cfg->port = port;
  return cfg;
}

RedisCConfig *get_current_config() { return current_conf; }

bool set_config(RedisCConfig *conf) {
  current_conf = conf;
  return true;
}
