CC = gcc
CFLAGS = -gno-split-dwarf -Wall -Wextra -std=gnu11 -Iinclude
LDFLAGS = -lm

all: disklist diskinfo diskget diskput

diskinfo: diskinfo.c
	$(CC) $(CFLAGS) diskinfo.c -o diskinfo $(LDFLAGS)

disklist: disklist.c
	$(CC) $(CFLAGS) disklist.c -o disklist $(LDFLAGS)

diskget: diskget.c
	$(CC) $(CFLAGS) diskget.c -o diskget $(LDFLAGS)

diskput: diskput.c
	$(CC) $(CFLAGS) diskput.c -o diskput $(LDFLAGS)

clean:
	rm -f diskinfo disklist diskget diskput
	rm -rf *.dSYM
