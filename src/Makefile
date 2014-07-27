
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -O3 -g
BINS = main
OBJS = b-tree.o


all: $(OBJS) $(BINS)


b-tree.o: b-tree.c b-tree.h
	$(CC) $(CFLAGS) -c -o $@ $<

main: main.c b-tree.o
	$(CC) $(CFLAGS) -DB_DEBUG -o $@ $^


clean:
	rm -f $(BINS)
