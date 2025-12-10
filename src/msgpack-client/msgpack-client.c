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
