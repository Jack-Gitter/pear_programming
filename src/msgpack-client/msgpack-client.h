#include "msgpack/object.h"

typedef struct method_param method_param_t;

typedef struct method_param_arr {
  method_param_t *items;
  int len;
} method_param_arr_t;

union msg_pack_value {
  void *nil;
  bool b;
  int i;
  float f;
  char *str;
  method_param_arr_t arr;
};

struct method_param {
  msgpack_object_type type;
  union msg_pack_value value;
};

int exchange_with_nvim_set_cursor(int socket, int window_id, int x, int y);

int send_message(int socket, char *method_name, method_param_t *params,
                 int params_len);

int set_lines(int socket);
