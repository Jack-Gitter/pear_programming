CC = clang
CFLAGS = -Wall -Wextra

.PHONY: all clean

all: src/main.c
	$(CC) $(CFLAGS) -o dist/main -lmsgpack-c src/main.c

clean:
	rm -r dist/*
