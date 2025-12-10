#include "constants.h"
#include "msgpack-client.h"
#include "msgpack/object.h"
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

char *parse_file_path(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <socket_name>\n", argv[0]);
    return NULL;
  }

  return argv[1];
}

int exchange_with_nvim_set_cursor(int socket, int window_id, int x, int y) {
  int resp = set_cursor(socket, window_id, x, y);
  if (resp < 0) {
    fprintf(stderr, "failed to set cursor\n");
    return resp;
  }
  msgpack_object *obj = read_response(socket);
  if (obj == NULL) {
    fprintf(stderr, "failed to read response\n");
    return -1;
  }

  msgpack_object_type type = obj->via.array.ptr[2].type;
  if (type != MSGPACK_OBJECT_NIL) {
    fprintf(stderr, "Error returned from server for RPC call\n");
    free(obj);
    return -1;
  }

  free(obj);
  return 0;
}

int main(int argc, char *argv[]) {

  int ret = 0;

  char *file_path = parse_file_path(argc, argv);

  if (file_path == NULL) {
    exit(EXIT_FAILURE);
  }

  int socket = init_socket(file_path);

  if (socket < 0) {
    exit(EXIT_FAILURE);
  }

  ret = set_cursor(socket, 1000, 1, 1);

  if (ret < 0) {
    cleanup_socket(socket);
    exit(EXIT_FAILURE);
  }

  ret = exchange_with_nvim_set_cursor(socket, 1000, 1, 1);

  if (ret < 0) {
    exit(EXIT_FAILURE);
  }

  ret = cleanup_socket(socket);

  if (ret != 0) {
    exit(EXIT_FAILURE);
  }
}
