# $Id: Makefile,v 1.2 2011/05/20 09:34:25 nanard Exp $
# made for GNU Make
CFLAGS = -Wall -g
EXECUTABLES = testgetifstats testifacewatcher

all:	$(EXECUTABLES)

clean:
	rm -f *.o $(EXECUTABLES)

testobsdrdr.o:	testobsdrdr.c obsdrdr.h

testgetifstats:	testgetifstats.o getifstats.o
	$(CC) $(CFLAGS) -o $@ $> -lkvm

testifacewatcher:	testifacewatcher.o ifacewatcher.o upnputils.o
	$(CC) $(CFLAGS) -o $@ $>

upnputils.o:	../upnputils.c

