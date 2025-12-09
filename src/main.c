#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <socket_name>\n", argv[0]);
    return 1;
  }

  int sock = socket(PF_LOCAL, SOCK_STREAM, 0);

  if (sock < 0) {
    fprintf(stderr, "failed");
    return 1;
  }

  struct sockaddr_un socket_address;
  socket_address.sun_family = AF_LOCAL;
  char *filename = argv[1];

  strncpy(socket_address.sun_path, filename, sizeof(socket_address.sun_path));
  socket_address.sun_path[sizeof(socket_address.sun_path) - 1] = '\0';

  // size_t size = SUN_LEN(&socket_address);

  char *content = "hello world";
  send(sock, content, strlen(content), 0);
  /*  if (bind(sock, (struct sockaddr *)&socket_address, size) < 0) {
      perror("bind");
      exit(EXIT_FAILURE);
    }

    close(sock);
    unlink(filename);*/

  return sock;
}
