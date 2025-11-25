#ifndef SERIALIZE_H__
#define SERIALIZE_H__

#include "command/cmd.h"
#include <_stdio.h>
#include <_stdlib.h>
#include <stdbool.h>
#include <string.h>

bool __strcmp(const char *l, const char *r, size_t cmp_size) {
  for (size_t i = 0; i < cmp_size; i++) {
    if (l[i] != r[i])
      return false;
  }
  return true;
}

Command *serializer(const char *buffer, size_t buf_size) {
  char cmd_type[16];
  char cmd_subtype[16];
  size_t idx = 0;

  // Skip any leading spaces
  while (idx < buf_size && buffer[idx] == ' ')
    idx++;

  size_t type_idx = 0;
  while (idx < buf_size && buffer[idx] != ' ' && buffer[idx] != '.' &&
         type_idx < sizeof(cmd_type) - 1) {
    cmd_type[type_idx++] = buffer[idx++];
  }
  // NUL-terminate
  cmd_type[type_idx] = '\0';

  // If we stopped because of '.', parse subtype
  if (idx < buf_size && buffer[idx] == '.') {
    idx++; // skip '.'
    size_t subtype_idx = 0;
    while (idx < buf_size && buffer[idx] != ' ' &&
           subtype_idx < sizeof(cmd_subtype) - 1) {
      cmd_subtype[subtype_idx++] = buffer[idx++];
    }
    cmd_subtype[subtype_idx] = '\0';
  }

  printf("Command: %s\n", cmd_type);
  if (__strcmp(cmd_type, "PING", 4)) {
    return create_command(CMD_PING, -1, NULL);
  } else if (__strcmp(cmd_type, "CMS", 3)) {
    if (__strcmp(cmd_subtype, "INITBYDIM", 9)) {
      printf("Command INITBYDIM\n");
      return NULL;
    } else if (__strcmp(cmd_subtype, "INITBYPROB", 10)) {
      printf("Command INITBYPROB\n");
      return NULL;
    } else if (__strcmp(cmd_subtype, "INCRBY", 6)) {
      printf("Command INCRBY\n");
      return NULL;
    } else if (__strcmp(cmd_subtype, "QUERY", 5)) {
      printf("Command QUERY\n");
      return NULL;
    }
    printf("Not match any CMS command\n");
    return NULL;
  }

  return NULL;
}

#endif
