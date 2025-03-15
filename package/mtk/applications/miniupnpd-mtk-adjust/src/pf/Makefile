# $Id: Makefile,v 1.7 2020/05/21 00:10:56 nanard Exp $
# made for GNU Make (and BSD make)
# only for testing purposes.
# Please follow instructions in INSTALL file for building miniUPnPd
CFLAGS = -Wall -g -DTEST
CFLAGS += -Wextra
CFLAGS += -I.. -I../build
EXECUTABLES = testobsdrdr testpfpinhole
# to test libpfctl
CFLAGS += -I/usr/local/include/
LDFLAGS += -L/usr/local/lib
LIBS += -lpfctl

all:	$(EXECUTABLES)

clean:
	rm -f *.o $(EXECUTABLES)

testobsdrdr:	testobsdrdr.o obsdrdr.o getifaddr.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $>

testpfpinhole:	testpfpinhole.o obsdrdr.o pfpinhole.o \
	getifaddr.o upnputils.o getroute.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $>

obsdrdr.o:	obsdrdr.c obsdrdr.h

pfpinhole.o:	pfpinhole.c pfpinhole.h

testobsdrdr.o:	testobsdrdr.c obsdrdr.h

testpfpinhole.o:	testpfpinhole.c pfpinhole.h

getifaddr.o:	../getifaddr.c

upnputils.o:	../upnputils.c

getroute.o:	../bsd/getroute.c
