# $Id: Makefile,v 1.1 2007/09/25 19:44:43 nanard Exp $
CC=gcc
CFLAGS=-Wall -g -I.

all:	testipfrdr

clean:
	rm *.o testipfrdr

testipfrdr:	testipfrdr.o ipfrdr.o
	$(CC) -o $@ $^

ipfrdr.o:	ipfrdr.c

testipfrdr.o:	testipfrdr.c

