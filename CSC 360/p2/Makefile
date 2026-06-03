CC = gcc
CFLAGS = -g -Wall -Wextra -std=gnu11 -Iinclude
LDFLAGS = -pthread -lm

all: mts

mts: mts.c
	$(CC) $(CFLAGS) mts.c -o mts $(LDFLAGS)

clean:
	rm -f mts
