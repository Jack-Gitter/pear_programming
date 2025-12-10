#include "msgpack-client.h"
#include "constants.h"
#include "msgpack/object.h"
#include "msgpack/pack.h"
#include "msgpack/sbuffer.h"
#include "msgpack/unpack.h"
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

msgpack_object *read_response(int socket) {
  msgpack_unpacker unp;
  bool result = msgpack_unpacker_init(&unp, DEFAULT_MES_LEN);
  if (result) {
    msgpack_unpack_return ret = MSGPACK_UNPACK_CONTINUE;
    while (ret == MSGPACK_UNPACK_CONTINUE) {
      if (msgpack_unpacker_buffer_capacity(&unp) < DEFAULT_MES_LEN) {
        bool result = msgpack_unpacker_reserve_buffer(&unp, DEFAULT_MES_LEN);
        if (!result) {
          fprintf(stderr, "failed to allocate memory for unpacker\n");
          return NULL;
        }
        int bytes_read =
            recv(socket, msgpack_unpacker_buffer(&unp), DEFAULT_MES_LEN, 0);
        msgpack_unpacker_buffer_consumed(&unp, bytes_read);
        if (bytes_read == 0) {
          fprintf(stdout, "client closed connection gracefully\n");
          return NULL;
        }
        msgpack_unpacked und;
        msgpack_unpacked_init(&und);
        ret = msgpack_unpacker_next(&unp, &und);
        switch (ret) {
        case MSGPACK_UNPACK_SUCCESS: {
          msgpack_object *obj = malloc(sizeof(msgpack_object));
          *obj = und.data;
          msgpack_unpacked_destroy(&und);
          return obj;
        }
        case MSGPACK_UNPACK_CONTINUE:
          printf("need more data\n");
          break;
        case MSGPACK_UNPACK_PARSE_ERROR:
          printf("parsing error\n");
          msgpack_unpacked_destroy(&und);
          return NULL;
        default:
          printf("unexpected error occured\n");
          msgpack_unpacked_destroy(&und);
          return NULL;
        }
      }
    }
  }
  return NULL;
}

void pack_params(msgpack_packer *pk, method_param_t *params, int i, int len) {
  if (i == len) {
    printf("base case\n");
    return;
  }
  msgpack_object_type type = params[i].type;
  switch (type) {
  case MSGPACK_OBJECT_NIL:
    printf("packing nil\n");
    msgpack_pack_nil(pk);
    break;
  case MSGPACK_OBJECT_BOOLEAN:
    printf("packing bool: %d\n", params[i].value.b);
    if (params[i].value.b) {
      msgpack_pack_true(pk);
    } else {
      msgpack_pack_false(pk);
    }
    break;
  case MSGPACK_OBJECT_POSITIVE_INTEGER:
  case MSGPACK_OBJECT_NEGATIVE_INTEGER:
    printf("packing int: %d\n", params[i].value.i);
    msgpack_pack_int(pk, params[i].value.i);
    break;
  case MSGPACK_OBJECT_FLOAT:
  case MSGPACK_OBJECT_FLOAT32:
    printf("packing float %f\n", params[i].value.f);
    msgpack_pack_float(pk, params[i].value.f);
    break;
  case MSGPACK_OBJECT_STR: {
    printf("packing str: %s\n", params[i].value.str);
    char *str = params[i].value.str;
    int str_len = strlen(str);
    msgpack_pack_str(pk, str_len);
    msgpack_pack_str_body(pk, str, str_len);
    break;
  }
  case MSGPACK_OBJECT_ARRAY: {
    printf("packing arr\n");
    msgpack_pack_array(pk, params[i].value.arr.len);
    int arr_len = params[i].value.arr.len;
    pack_params(pk, params[i].value.arr.items, 0, arr_len);
    break;
  }
  default:
    break;
  }
  pack_params(pk, params, ++i, len);
}

int send_message(int socket, char *method_name, method_param_t *params,
                 int params_len) {
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
  int method_len = strlen(method_name);
  msgpack_pack_str(&pk, method_len);
  msgpack_pack_str_body(&pk, method_name, method_len);

  msgpack_pack_array(&pk, params_len);

  pack_params(&pk, params, 0, params_len);

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

int set_cursor(int socket, int window_id, int x, int y) {
  method_param_t x_param;
  x_param.type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  x_param.value.i = x;
  method_param_t y_param;
  y_param.type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  y_param.value.i = y;
  method_param_arr_t arr;

  arr.items = malloc(sizeof(method_param_t) * 2);

  if (arr.items == NULL) {
    fprintf(stderr, "malloc failed\n");
    return -1;
  }

  arr.items[0] = x_param;
  arr.items[1] = y_param;
  arr.len = 2;

  method_param_t *params = malloc(sizeof(method_param_t) * 2);

  if (params == NULL) {
    fprintf(stderr, "malloc failed\n");
    return -1;
  }

  params[0].type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  params[0].value.i = window_id;
  params[1].type = MSGPACK_OBJECT_ARRAY;
  params[1].value.arr = arr;

  send_message(socket, "nvim_win_set_cursor", params, 2);

  free(arr.items);
  return 0;
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
