CC = clang
CFLAGS = -Wall -Wextra

.PHONY: all clean

all: src/main.c
	$(CC) $(CFLAGS) \
	-I/usr/local/include -I./src/constants/ -I./src/msgpack-client/ \
	-o dist/main \
	src/msgpack-client/msgpack-client.c /usr/local/lib/libmsgpack-c.a src/main.c 


clean:
	rm -r dist/*
