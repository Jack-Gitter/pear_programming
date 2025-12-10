CC = clang
CFLAGS = -Wall -Wextra

.PHONY: all clean

all: src/main.c
	$(CC) $(CFLAGS) \
	-I/usr/local/include -I./src/constants/ -I./src/msgpack-client/ \
	/usr/local/lib/libmsgpack-c.a ./src/msgpack-client/mspack-client.c \
	-o dist/main src/main.c 


clean:
	rm -r dist/*
