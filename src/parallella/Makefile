ESDK=$(EPIPHANY_HOME)
ELIBS=$(ESDK)/tools/host/lib
EINCS=$(ESDK)/tools/host/include
ELDF=$(ESDK)/bsps/current/internal.ldf
BINS=main e_b-tree.srec
OBJS=e_b-tree.elf b-tree.o

CC=gcc
ECC=e-gcc
EOC=e-objcopy
CFLAGS=-Wall -Wextra -Werror -O2 -ggdb -std=gnu99 -Wl,-rpath,$(ELIBS)
EFLAGS=-funroll-loops -ffast-math
EOFLAGS=--srec-forceS3 --output-target srec


.PHONY: all clean


all: $(BINS)

main: main.c b-tree.o
	$(CC) $(CFLAGS) -I$(EINCS) -L$(ELIBS) -o $@ $^ -le-hal -lrt

b-tree.o: b-tree.c b-tree.h
	$(CC) $(CFLAGS) -c -o $@ $<

e_b-tree.elf: e_b-tree.c b-tree.h
	$(ECC) $(CFLAGS) $(EFLAGS) -O3 -T $(ELDF) -o $@ $< -le-lib

e_b-tree.srec: e_b-tree.elf
	$(EOC) $(EOFLAGS) $^ $@


clean:
	rm -f $(BINS) $(OBJS)
