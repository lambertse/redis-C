#include "redis-C/cli.h"
#include "io-multiplexing/client/client.h"
#include "io-multiplexing/server/config.h"
#include "redis-C/config.h"
#include "redis-C/rc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool setup_config(int port) {
  set_config(create_config(port));
  return true;
}

int main(int argc, char *argv[]) {
  int port = REDIS_C_DEFAULT_PORT;
  if (argc >= 2) {
    port = atoi(argv[1]);
  }
  ServerConfig conf = {.port = port};
  setup_config(port);

  char input[256];
  char server_ip[256];
  snprintf(server_ip, sizeof(server_ip), "%s:%d", REDIS_C_DEFAULT_HOST,
           conf.port);

  while (1) {
    printf("%s> ", server_ip);
    fflush(stdout);

    if (fgets(input, MAX_INPUT, stdin) == NULL) {
      break;
    }

    Client *client = client_create();
    client_connect(client, &conf);

    size_t req_size = strlen(input);
    size_t res_size = 0;

    char *response = client_send_request(client, input, req_size, &res_size);
    if (response == NULL) {
      printf("Connection error.\n");
      continue;
    }

    int rc = *(int *)response;
    char *msg = response + sizeof(int);

    if (REDIS_SUCCESS(rc)) {
      printf("OK. \n");
    } else {
      printf("Error: %d. %s\n", rc, msg);
    }

    free(client);
    free(response);
  }
}
