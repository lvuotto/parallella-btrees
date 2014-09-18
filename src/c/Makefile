CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -ggdb -O0 -std=c99
BINS=main
OBJS=b-tree.o
OBJDIR=../../objs
BINDIR=../../bin


.PHONY: all clean test

main: main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJDIR)/$(OBJS)
	mv $@ $(BINDIR)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<
	mv $@ $(OBJDIR)

all: $(BINS)

clean:
	rm -f $(OBJDIR)/$(OBJS)
	rm -f $(BINDIR)/$(BINS)
