CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11 -Iinclude

all: ssi

ssi: src/ssi.c
	$(CC) $(CFLAGS) src/ssi.c -lreadline -ltermcap -o ssi

clean:
	rm -f ssi
