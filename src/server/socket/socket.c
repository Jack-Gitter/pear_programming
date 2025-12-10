#include "constants.h"
#include "msgpack-client.h"
#include <errno.h>
#include <msgpack.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int init_socket(char *filename) {

  int sock = socket(PF_LOCAL, SOCK_STREAM, 0);

  if (sock < 0) {
    fprintf(stderr, "failed to create socket\n");
    return -1;
  }

  struct sockaddr_un socket_address;
  socket_address.sun_family = AF_LOCAL;

  strncpy(socket_address.sun_path, filename, sizeof(socket_address.sun_path));
  socket_address.sun_path[sizeof(socket_address.sun_path) - 1] = '\0';

  size_t size = SUN_LEN(&socket_address);
  if (connect(sock, (struct sockaddr *)&socket_address, size) < 0) {
    fprintf(stderr, "failed to connect to the address\n");
    close(sock);
    return -1;
  }

  return sock;
}

int cleanup_socket(int socket) {
  int ret = close(socket);
  if (ret < 0) {
    fprintf(stderr, "failed to cleanup socket\n");
  }
  return ret;
}
