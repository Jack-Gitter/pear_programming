#include "constants.h"
#include "msgpack-client.h"
#include "socket.h"
#include "utils.h"
#include <errno.h>
#include <msgpack.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

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

  if (ret < 0) {
    cleanup_socket(socket);
    exit(EXIT_FAILURE);
  }

  ret = exchange_with_nvim_set_cursor(socket, 0, 100, 100);

  if (ret < 0) {
    cleanup_socket(socket);
    exit(EXIT_FAILURE);
  }

  ret = cleanup_socket(socket);
}
