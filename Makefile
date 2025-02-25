CC = gcc
CFLAGS = -g -Wall -Wextra -Wconversion

.PHONY: all clean

all: server client

server: server.o
	$(CC) $^ -o $@

server.o: server.c
	$(CC) $(CFLAGS) -c $< -o $@

client: client.o
	$(CC) $^ -o $@

client.o: client.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f server
	rm -f client
	rm -f *.o
