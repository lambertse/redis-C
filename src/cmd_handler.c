#include "cmd_handler.h"
#include "3rdparty/io-multiplexing/src/shared/logging.h"
#include "command/cmd.h"
#include "command/cmd_cms.h"
#include "redis-C/config.h"
#include "redis-C/rc.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

REDIS_RC handle_ping(void) {
  // When a client send a request and it reaches here, mean that the connection
  // is OK
  return REDIS_OK;
}

REDIS_RC handle_command(Command *cmd) {
  if (cmd == NULL) {
    return REDIS_CMD_NULL;
  }

  if (cmd->type == CMD_PING) {
    return handle_ping();
  } else if (cmd->type == CMD_CMS) {
    return handle_cms_command(cmd); 
  }

  return REDIS_OK;
}
