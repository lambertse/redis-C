#ifndef CMD_CMS_H__
#define CMD_CMS_H__

#include "command/cmd.h"
#include "redis-C/rc.h"
#include "storage.h"
typedef enum {
  CMS_INITBYDIM = 0,
  CMS_INITBYPROB,
  CMS_INCRBY,
  CMS_QUERY
} CMD_cms_type;

static REDIS_RC handle_cms_command(Command *cmd) {
  switch (cmd->sub_cmd) {
  case CMS_INITBYDIM: {
    return create_cms_store(cmd->arg[0]);
  }
  case CMS_INITBYPROB: {
    break;
  }
  case CMS_INCRBY: {
    break;
  }
  case CMS_QUERY: {
    break;
  }
  default:
    return REDIS_SUB_CMD_NOT_FOUND;
  }
  return REDIS_SUB_CMD_NOT_FOUND;
}

#endif
