CFLAGS = -g -Wall -Wextra -Wconversion
LDLIBS = -lrsync
.PHONY: all clean

all: server client

server: server.c common.h

client: client.c common.h

clean:
	rm -f server
	rm -f client
	rm -f *.o
