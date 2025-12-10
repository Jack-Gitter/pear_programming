CC = clang
CFLAGS = -Wall -Wextra

.PHONY: all clean

all: src/main.c
	$(CC) $(CFLAGS) -I/usr/local/include /usr/local/lib/libmsgpack-c.a -o dist/main src/main.c

clean:
	rm -r dist/*
