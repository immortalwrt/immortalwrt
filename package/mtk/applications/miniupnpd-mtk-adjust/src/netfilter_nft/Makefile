CFLAGS?=-Wall -g -Wstrict-prototypes -Wdeclaration-after-statement
CPPFLAGS += -I.
CPPFLAGS += -D_GNU_SOURCE -DDEBUG
CPPFLAGS += -DUSE_NETFILTER -DUSE_NFTABLES
CPPFLAGS += -DENABLE_UPNPPINHOLE
CPPFLAGS += -DUSE_NFCT
CC = gcc

LIBS = -lnftnl -lmnl

ARCH := $(shell uname -m | grep -q "x86_64" && echo 64)

all:	test_nfct_get testnftnlrdr testnftpinhole

clean:
	$(RM) *.o config.h testnftnlcrdr testnftpinhole testnftnlrdr_peer \
		test_nfct_get testnftnlrdr 

config.h:
	touch $@

testnftnlrdr:	nftnlrdr.o nftnlrdr_misc.o testnftnlrdr.o $(LIBS)

testnftpinhole:	nftpinhole.o nftnlrdr.o nftnlrdr_misc.o testnftpinhole.o \
	../upnputils.o ../linux/getroute.o $(LIBS)

test_nfct_get:	test_nfct_get.o test_nfct_get.o -lmnl -lnetfilter_conntrack

test_nfct_get.o:	test_nfct_get.c

testnftnlrdr_peer.o:	testnftnlrdr_peer.c

testnftnlrdr_dscp.o:	testnftnlrdr_dscp.c

nftnlrdr.o:		nftnlrdr.c nftnlrdr.h config.h

nftnlrdr_misc.o:	nftnlrdr_misc.c

nftpinhole.o:		nftpinhole.c nftpinhole.h config.h

testnftnlrdr.o:		config.h nftnlrdr.h nftnlrdr_misc.h

testnftpinhole.o:	config.h nftpinhole.h
