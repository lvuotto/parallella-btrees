
ESDK = $(EPIPHANY_HOME)
ELIBS = $(ESDK)/tools/host/lib
EINCS = $(ESDK)/tools/host/include
ELDF = $(ESDK)/bsps/current/internal.ldf
BINS = benchmark tiempos 
DEPS = e_write.srec e_read.srec e_write_asm.srec e_read_asm.srec
OBJS = e_write*.elf e_read*.elf e_tiempos.elf

CC = gcc
ECC = e-gcc
EOC = e-objcopy
CFLAGS = -Wall -Wextra -Werror -ggdb -std=gnu99 -Wl,-rpath,$(ELIBS)
EFLAGS = -funroll-loops -ffast-math
EOFLAGS = --srec-forceS3 --output-target srec


.PHONY: all clean


benchmark: benchmark.c bmmi.h $(DEPS)
	$(CC) $(CFLAGS) -I$(EINCS) -L$(ELIBS) -o $@ $< -le-hal -lrt

e_write.elf: e_write.c bmmi.h
	$(ECC) $(CFLAGS) $(EFLAGS) -O3 -T $(ELDF) -o $@ $< -le-lib

e_read.elf: e_read.c bmmi.h
	$(ECC) $(CFLAGS) $(EFLAGS) -O3 -T $(ELDF) -o $@ $< -le-lib

e_write_asm.elf: e_write_asm.c bmmi.h
	$(ECC) $(CFLAGS) $(EFLAGS) -O3 -T $(ELDF) -o $@ $< -le-lib

e_read_asm.elf: e_read_asm.c bmmi.h
	$(ECC) $(CFLAGS) $(EFLAGS) -O3 -T $(ELDF) -o $@ $< -le-lib


tiempos: tiempos.c compartido.h e_tiempos.srec
	$(CC) $(CFLAGS) -I$(EINCS) -L$(ELIBS) -o $@ $< -le-hal -lrt

e_tiempos.elf: e_tiempos.c compartido.h
	$(ECC) $(CFLAGS) $(EFLAGS) -O3 -T $(ELDF) -o $@ $< -le-lib


e_%.srec: e_%.elf
	$(EOC) $(EOFLAGS) $< $@


all: $(BINS)

clean:
	rm -f $(OBJS)
	rm -f *.srec
	rm -f $(BINS)
