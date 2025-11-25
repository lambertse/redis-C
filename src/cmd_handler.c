#include "cmd_handler.h"
#include "3rdparty/io-multiplexing/src/shared/logging.h"
#include "command/cmd.h"
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
  } else if (1) {
  }

  return REDIS_OK;
}
