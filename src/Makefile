
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -ggdb -O0 -std=c99
BINS = main test
OBJS = b-tree.o


all: $(OBJS) $(BINS)


b-tree.o: b-tree.c b-tree.h
	$(CC) $(CFLAGS) -c -o $@ $<

main: main.c b-tree.o
	$(CC) $(CFLAGS) -o $@ $^

test: test.c b-tree.o
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -f $(BINS) $(OBJS)
