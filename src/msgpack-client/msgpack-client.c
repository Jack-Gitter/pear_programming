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

int real(int socket, char *method_name, method_param_t *params,
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

  for (int i = 0; i < params_len; i++) {
    msgpack_object_type type = params[i].type;
    switch (type) {
    case MSGPACK_OBJECT_NIL:
      msgpack_pack_nil(&pk);
      break;
    case MSGPACK_OBJECT_BOOLEAN:
      if (params[i].value.b) {
        msgpack_pack_true(&pk);
      } else {
        msgpack_pack_false(&pk);
      }
      break;
    case MSGPACK_OBJECT_POSITIVE_INTEGER:
    case MSGPACK_OBJECT_NEGATIVE_INTEGER:
      msgpack_pack_int(&pk, params[i].value.i);
      break;
    case MSGPACK_OBJECT_FLOAT:
    case MSGPACK_OBJECT_FLOAT32:
      msgpack_pack_float(&pk, params[i].value.f);
      break;
    case MSGPACK_OBJECT_STR: {
      char *str = params[i].value.str;
      int str_len = strlen(str);
      msgpack_pack_str(&pk, str_len);
      msgpack_pack_str_body(&pk, str, str_len);
      break;
    }
    default:
      break;
    }
  }

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
