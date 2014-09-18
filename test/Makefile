CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -ggdb -O0 -std=c99
INCDIR=../src/c
BINS=test size
OBJS=b-tree.o
BINDIR=../bin
OBJDIR=../objs


.PHONY: all clean test

all: $(BINS)

test: test.c $(OBJS)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $< $(OBJDIR)/$(OBJS)
	mv $@ $(BINDIR)

size: size.c $(OBJS)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $< $(OBJDIR)/$(OBJS)
	mv $@ $(BINDIR)

b-tree.o:
	make -C ../src/c $@

clean:
	rm -f $(OBJDIR)/$(OBJS)
	rm -f $(BINDIR)/$(BINS)
