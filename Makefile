
CC = gcc
CFLAGS = -g -Wall -Wextra -Wconversion
LDFLAGS = 

.PHONY: all clean run

all: prog

prog: main.o
	$(CC) $^ -o $@ $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f prog
	rm -f *.o
	chattr -i immutable.txt
	rm -f immutable.txt

run: prog 
	./prog
