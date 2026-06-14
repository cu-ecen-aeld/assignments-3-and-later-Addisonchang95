CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror

all: aesdsocket

aesdsocket: aesdsocket.o
	$(CC) $(CFLAGS) aesdsocket.o -o aesdsocket
aesdsocket.o: aesdsocket.c aesdsocket.h
	$(CC) $(CFLAGS) -c aesdsocket.c -o aesdsocket.o

clean:
	rm -f aesdsocket *.o

.PHONY: all clean