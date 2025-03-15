# $Id: Makefile,v 1.2 2011/02/20 23:43:41 nanard Exp $
# made for GNU Make
CFLAGS = -Wall -g
EXECUTABLES = testgetifstats

all:	$(EXECUTABLES)

clean:
	rm -f *.o $(EXECUTABLES)

testmacrdr.o:	testmacrdr.c macrdr.h

testgetifstats:	testgetifstats.o getifstats.o

