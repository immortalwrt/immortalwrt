# $Id: Makefile,v 1.6 2012/04/26 13:50:48 nanard Exp $
CFLAGS?=-Wall -g -D_GNU_SOURCE -DDEBUG -Wstrict-prototypes -Wdeclaration-after-statement
CC = gcc

#LIBS = -liptc
LIBS = -lip4tc

ARCH := $(shell uname -m | grep -q "x86_64" && echo 64)
ifdef IPTABLESPATH
CFLAGS := $(CFLAGS) -I$(IPTABLESPATH)/include/
LDFLAGS := $(LDFLAGS) -L$(IPTABLESPATH)/libiptc/
# get iptables version and set IPTABLES_143 macro if needed
IPTABLESVERSION := $(shell grep "\#define VERSION" $(IPTABLESPATH)/config.h | tr -d \" |cut -d" " -f3 )
IPTABLESVERSION1 := $(shell echo $(IPTABLESVERSION) | cut -d. -f1 )
IPTABLESVERSION2 := $(shell echo $(IPTABLESVERSION) | cut -d. -f2 )
IPTABLESVERSION3 := $(shell echo $(IPTABLESVERSION) | cut -d. -f3 )
# test if iptables version >= 1.4.3
TEST := $(shell [ \( \( $(IPTABLESVERSION1) -ge 1 \) -a \( $(IPTABLESVERSION2) -ge 4 \) \) -a \( $(IPTABLESVERSION3) -ge 3 \) ] && echo 1 )
ifeq ($(TEST), 1)
CFLAGS := $(CFLAGS) -DIPTABLES_143
# the following sucks, but works
LIBS = $(IPTABLESPATH)/libiptc/.libs/libip4tc.o
#LIBS = $(IPTABLESPATH)/libiptc/.libs/libiptc.a
else
LIBS = $(IPTABLESPATH)/libiptc/libiptc.a
endif
else
# check for system-wide iptables files. Test if iptables version >= 1.4.3
#TEST := $(shell test -f /usr/include/iptables/internal.h && grep -q "\#define IPTABLES_VERSION" /usr/include/iptables/internal.h && echo 1)
TEST := $(shell test -f /usr/include/xtables.h && grep -q "XTABLES_VERSION_CODE" /usr/include/xtables.h && echo 1)
ifeq ($(TEST), 1)
CFLAGS := $(CFLAGS) -DIPTABLES_143
LIBS = -liptc
TEST_LIB := $(shell test -f /usr/lib$(ARCH)/libiptc.a && echo 1)
ifeq ($(TEST_LIB), 1)
LIBS = -liptc /usr/lib$(ARCH)/libiptc.a
endif
endif
endif

LIBS +=  /lib/libip4tc.so /lib/libip6tc.so

all:	iptcrdr.o testiptcrdr iptpinhole.o \
        testiptcrdr_peer testiptcrdr_dscp test_nfct_get
#        testiptpinhole

clean:
	$(RM) *.o testiptcrdr testiptpinhole testiptcrdr_peer test_nfct_get \
        testiptcrdr_dscp

testiptcrdr:	testiptcrdr.o upnpglobalvars.o $(LIBS)

testiptcrdr_peer:	testiptcrdr_peer.o upnpglobalvars.o $(LIBS)

testiptcrdr_dscp:	testiptcrdr_dscp.o upnpglobalvars.o $(LIBS)

testiptpinhole:	testiptpinhole.o iptpinhole.o upnpglobalvars.o $(LIBS)

test_nfct_get:	test_nfct_get.o test_nfct_get.o -lmnl -lnetfilter_conntrack

test_nfct_get.o:	test_nfct_get.c

testiptcrdr_peer.o:	testiptcrdr_peer.c

testiptcrdr_dscp.o:	testiptcrdr_dscp.c

iptcrdr.o:	iptcrdr.c iptcrdr.h

iptpinhole.o:	iptpinhole.c iptpinhole.h

upnpglobalvars.o:	../upnpglobalvars.c ../upnpglobalvars.h
	$(CC) -c -o $@ $<


#depends
testiptcrdr.o: testiptcrdr.c iptcrdr.c
testiptcrdr_dscp.o:	testiptcrdr_dscp.c iptcrdr.c
testiptcrdr_peer.o:	testiptcrdr_peer.c iptcrdr.c
test_nfct_get.o:	test_nfct_get.c nfct_get.c
