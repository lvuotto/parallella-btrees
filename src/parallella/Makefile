ESDK=$(EPIPHANY_HOME)
ELIBS=$(ESDK)/tools/host/lib
EINCS=$(ESDK)/tools/host/include
ELDF=$(ESDK)/bsps/current/internal.ldf
BINS=main e_b-tree.srec
OBJS=e_b-tree.elf b-tree.o

CC=gcc
ECC=e-gcc
EOC=e-objcopy
CFLAGS=-Wall -Wextra -pedantic -O0 -ggdb -std=gnu99
CLFLAGS=-Wl,-rpath,$(ELIBS)
EFLAGS=-funroll-loops -ffast-math -O0
EOFLAGS=--srec-forceS3 --output-target srec


.PHONY: all clean


all: $(BINS)

main: main.c b-tree.o queue.o
	$(CC) $(CFLAGS) $(CLFLAGS) -I$(EINCS) -L$(ELIBS) -o $@ $^ -le-hal -lrt

b-tree.o: b-tree.c b-tree.h btmi.h
	$(CC) $(CFLAGS) $(CLFLAGS) -I$(EINCS) -L$(ELIBS) -c -o $@ $<

e_b-tree.elf: e_b-tree.c e_b-tree.h
	$(ECC) $(CFLAGS) $(CLFLAGS) $(EFLAGS) -T$(ELDF) -o $@ $< -le-lib

e_b-tree.srec: e_b-tree.elf
	$(EOC) $(EOFLAGS) $^ $@


clean:
	rm -f $(BINS) $(OBJS)
