CC = clang
CFLAGS = -Wall -Wextra
INCLUDES = -I/usr/local/include -I./src/constants/ -I./src/msgpack-client/ -I./src/socket/ -I./src/utils/ 
FILES = src/msgpack-client/msgpack-client.c src/socket/socket.c /usr/local/lib/libmsgpack-c.a src/utils/utils.c src/main.c 

.PHONY: all clean

all: src/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -o dist/main $(FILES)

clean:
	rm -r dist/*
