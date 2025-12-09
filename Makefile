CC = clang
CFLAGS = -Wall -Wextra

.PHONY: all clean

all: src/main.c
	$(CC) $(CFLAGS) -o dist/main src/main.c

clean:
	rm -r dist/*
