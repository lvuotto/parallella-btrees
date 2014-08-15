
ESDK = $(EPIPHANY_HOME)
ELIBS = $(ESDK)/tools/host/lib
EINCS = $(ESDK)/tools/host/include
ELDF = $(ESDK)/bsps/current/internal.ldf
BINS = benchmark e_benchmark.srec
OBJS = e_benchmark.elf

CC = gcc
ECC = e-gcc
EOC = e-objcopy
CFLAGS = -Wall -Wextra -Werror -O0 -ggdb -std=c99 -Wl,-rpath,$(ELIBS)
EFLAGS = -funroll-loops -ffast-math
EOFLAGS = --srec-forceS3 --output-target srec


.PHONY: all clean

benchmark: benchmark.c bmmi.h
	$(CC) $(CFLAGS) -I$(EINCS) -L$(ELIBS) -o $@ $< -le-hal -lrt

e_benchmark.elf: e_benchmark.c bmmi.h
	$(ECC) $(CFLAGS) $(EFLAGS) -O3 -T $(ELDF) -o $@ $< -le-lib

e_benchmark.srec: e_benchmark.elf
	$(EOC) $(EOFLAGS) $< $@


all: $(BINS)

clean:
	rm -f $(OBJS)
	rm -f $(BINS)
