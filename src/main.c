#include "msgpack/pack.h"
#include "msgpack/sbuffer.h"
#include <errno.h>
#include <msgpack.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int msgid = 0;

typedef struct message {
  int type;
  int msgid;
  char *method;
  void *params;
} message_t;

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

// [type, msgid, method, params]
int set_cursor(int socket, int window_id, int x, int y) {
  msgpack_sbuffer sbuf;
  msgpack_packer pk;
  msgpack_sbuffer_init(&sbuf);
  msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

  // Pack [type, msgid, method, params]
  msgpack_pack_array(&pk, 4);

  // Pack type = 0 = request
  msgpack_pack_int(&pk, 0);
  // Pack msgid
  msgpack_pack_int(&pk, msgid++);

  // Pack method = method*
  char *method = "nvim_win_set_cursor";
  int len = strlen(method);
  msgpack_pack_str(&pk, len);
  msgpack_pack_str_body(&pk, method, len);

  // Pack params array
  msgpack_pack_array(&pk, 2);

  // Pack window ID
  msgpack_pack_int(&pk, window_id);

  // Pack [x,y] position tuple
  msgpack_pack_array(&pk, 2);
  msgpack_pack_int(&pk, x);
  msgpack_pack_int(&pk, y);

  size_t bytes = 0;

  while (bytes != sbuf.size) {
    int ret = send(socket, sbuf.data + bytes, sbuf.size - bytes, 0);
    if (ret < 0) {
      printf("ret is %d\n", ret);
      fprintf(stderr, "failed to send message on socket\n");
      return ret;
    }
    bytes += ret;
  }
  return 0;
}

char *parse_file_path(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <socket_name>\n", argv[0]);
    return NULL;
  }

  return argv[1];
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

  char *mes = ":nvim_command echo 'hello world'";
  ret = set_cursor(socket, 1000, 10, 10);

  if (ret < 0) {
    cleanup_socket(socket);
    exit(EXIT_FAILURE);
  }

  ret = cleanup_socket(socket);

  if (ret != 0) {
    exit(EXIT_FAILURE);
  }
}
