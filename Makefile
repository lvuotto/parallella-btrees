
CDIR = src/c
PARALLELLADIR = src/parallella
TESTDIR = test
BENCHMARKSDIR = benchmarks
OBJDIR = objs
BINDIR = bin

.PHONY: all test benchmarks clean

all:
	make -C $(CDIR)
	make -C $(PARALLELLADIR)

test:
	make -C $(TESTDIR)
	$(BINDIR)/$@

benchmarks:
	make -C $(BENCHMARKSDIR)

clean:
	rm -f $(OBJDIR)/*
	rm -f $(BINDIR)/*

