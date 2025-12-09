#include <errno.h>
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

int send_message(int socket, char *message) {

  size_t len = strlen(message);
  size_t bytes = 0;

  while (bytes != len) {
    int ret = send(socket, message, strlen(message), 0);
    if (ret < 0) {
      printf("ret is %d\n", ret);
      fprintf(stderr, "failed to send message on socket\n");
      return ret;
    }
    bytes += ret;
  }
  return 1;
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

  ret = send_message(socket, "hello world");

  if (ret < 0) {
    cleanup_socket(socket);
    exit(EXIT_FAILURE);
  }

  ret = cleanup_socket(socket);

  if (ret != 0) {
    exit(EXIT_FAILURE);
  }
}
