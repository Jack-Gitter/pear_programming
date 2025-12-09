CC = clang
CFLAGS = -Wall -Wextra

.PHONY: all clean

all: src/main.c
	$(CC) $(CFLAGS) -I/usr/local/include -L/usr/local/lib -o dist/main -lmsgpack-c src/main.c

clean:
	rm -r dist/*
