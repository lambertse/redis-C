#include "io-multiplexing/server/server.h"
#include "cmd_handler.h"
#include "command/cmd.h"
#include "io-multiplexing/server/config.h"
#include "redis-C/config.h"
#include "redis-C/rc.h"
#include "redis-C/server.h"
#include "serialize.h"
#include <stdlib.h>
#include <string.h>

// Callback function implementation
char *process_request(const char *request, size_t req_size, size_t *res_size) {
  /*
   * response format:
   *    - First 4 bytes is return code
   *    - Rest 252 bytes is explanation
   * */

  Command *cmd = serializer(request, req_size);
  REDIS_RC rc = handle_command(cmd);
  printf("rc: %d\n", rc);
  char *response = malloc(256);
  *res_size = 256;
  memcpy(response, &rc, sizeof(int));

  if (REDIS_SUCCESS(rc)) {
    return response;
  } else if (rc == REDIS_CMD_NULL) {
    char *msg = "Command generation failed\0";
    memcpy(response + sizeof(int), msg, strlen(msg));
    return response;
  }

  return NULL;
}

bool setup_config(int port) {
  set_config(create_config(port));
  return true;
}

int main(int argc, char* argv[]) {
  int port = REDIS_C_DEFAULT_PORT;
  if (argc >= 2) {
    port = atoi(argv[1]);
  }

  ServerConfig server_conf;
  server_conf.port = port;

  setup_config(port);

  Server *server = server_create(server_conf);
  if (!server) {
    printf("Failed to create server\n");
    return 0;
  }

  // Initialize server with callback function
  if (!server_init(server, process_request)) {
    printf("Init failed\n");
    server_destroy(server);
    return 0;
  }

  server_start(server);
  server_destroy(server);

  return 1;
}
