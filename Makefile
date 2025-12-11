CC = clang
CFLAGS = -Wall -Wextra -g
INCLUDES = -I/usr/local/include -I./src/constants/ -I./src/msgpack-client/ -I./src/server/socket/ -I./src/server/utils/ 
FILES = src/msgpack-client/msgpack-client.c src/server/socket/socket.c /usr/local/lib/libmsgpack-c.a src/server/utils/utils.c src/server/server.c 

.PHONY: all clean

all: src/server/server.c
	$(CC) $(CFLAGS) $(INCLUDES) -o dist/server $(FILES)

clean:
	rm -r dist/*
