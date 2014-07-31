
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -g -O0
BINS = main
OBJS = b-tree.o


all: $(OBJS) $(BINS)


b-tree.o: b-tree.c b-tree.h
	$(CC) $(CFLAGS) -DB_DEBUG -c -o $@ $<

main: main.c b-tree.o
	$(CC) $(CFLAGS) -DB_DEBUG -o $@ $^


clean:
	rm -f $(BINS) $(OBJS)
