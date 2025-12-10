#include "constants.h"
#include "msgpack-client.h"
#include "msgpack/object.h"
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

  method_param_arr_t *arr = malloc(sizeof(method_param_arr_t));
  method_param_t x;
  x.type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  x.value.i = 1;
  method_param_t y;
  y.type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  y.value.i = 1;
  arr->items = malloc(sizeof(method_param_t) * 2);
  arr->items[0] = x;
  arr->items[1] = y;
  arr->len = 2;

  method_param_t *params = malloc(sizeof(method_param_t) * 2);
  params[0].type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  params[0].value.i = 1000;
  params[1].type = MSGPACK_OBJECT_ARRAY;
  params[1].value.arr = arr;

  real(socket, "nvim_win_set_cursor", params, 2);

  if (ret < 0) {
    exit(EXIT_FAILURE);
  }

  ret = cleanup_socket(socket);

  if (ret != 0) {
    exit(EXIT_FAILURE);
  }
}
