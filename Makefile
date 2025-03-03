CC = gcc
CFLAGS = -g -Wall -Wextra -Wconversion
LDFLAGS = -lrsync

.PHONY: all clean

all: server client

server: server.o
	$(CC) server.o -o server $(LDFLAGS)

server.o: server.c common.h
	$(CC) $(CFLAGS) -c server.c -o server.o

client: client.o
	$(CC) client.o -o client $(LDFLAGS)

client.o: client.c common.h
	$(CC) $(CFLAGS) -c client.c -o client.o

clean:
	rm -f server
	rm -f client
	rm -f *.o
