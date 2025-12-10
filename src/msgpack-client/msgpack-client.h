#include "msgpack/object.h"

msgpack_object *read_response(int socket);
int set_cursor(int socket, int window_id, int x, int y);
