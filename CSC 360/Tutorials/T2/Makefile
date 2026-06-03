CC=gcc
CFLAGS=-Wall -std=c18 -g

all: multiping 

multiping: multiping.c
	$(CC) $(CFLAGS) -o multiping multiping.c
clean:
	rm -f multiping 

.PHONY: all clean
