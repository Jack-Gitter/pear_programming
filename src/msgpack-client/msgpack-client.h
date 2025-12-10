#include "msgpack/object.h"

typedef struct method_param method_param_t;

union msg_pack_value {
  void *nil;
  bool b;
  int i;
  float f;
  char *str;
  struct {
    method_param_t *items;
    int len;
  } arr;
};

struct method_param {
  msgpack_object_type type;
  union msg_pack_value value;
};

msgpack_object *read_response(int socket);
int set_cursor(int socket, int window_id, int x, int y);
int exchange_with_nvim_set_cursor(int socket, int window_id, int x, int y);

int real(int socket, char *method_name, method_param_t *params, int params_len);
