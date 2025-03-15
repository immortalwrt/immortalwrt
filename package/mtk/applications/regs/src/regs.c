/*
 * pcimem.c: Simple program to read/write from/to a pci device from userspace.
 *
 *  Copyright (C) 2010, Bill Farrow (bfarrow@beyondelectronics.us)
 *
 *  Based on the devmem2.c code
 *  Copyright (C) 2000, Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#define PRINT_ERROR \
	do { \
		fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
		__LINE__, __FILE__, errno, strerror(errno)); exit(1); \
	} while(0)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

void dump_page(uint32_t *vaddr, uint32_t *vbase, uint32_t *pbase)
{
	int i =0;
	uint32_t *end = vaddr + (MAP_SIZE >> 6);
	uint32_t *start = vaddr;

	while(start  < end) {
		printf("%p:%08x %08x %08x %08x\n",
			start - vbase + pbase, start[0], start[1] , start[2], start[3]);
		start+=4;
	}
}

void reg_mod_bits(uint32_t *virt_addr, int data, int  start_bit, int data_len)
{
    int mask=0;
    int value;
    int i;

	if ((start_bit < 0) || (start_bit > 31) ||
	    (data_len < 1) || (data_len > 32) ||
	    (start_bit + data_len > 32)) {
		fprintf(stderr, "Startbit range[0~31], and DataLen range[1~32], and Startbit + DataLen <= 32\n");
		return;
	}

	for (i = 0; i < data_len; i++) {
		if (start_bit + i > 31)
			break;

		mask |= 1 << (start_bit + i);
	}

	value = *((volatile uint32_t *) virt_addr);
	value &= ~mask;
	value |= (data << start_bit) & mask;;

	*((uint32_t *) virt_addr) = value;

	printf("Modify 0x%X[%d:%d]; ", data, start_bit + data_len - 1, start_bit);
}

void usage(void)
{
		fprintf(stderr, "\nUsage:\tregs [Type] [ Offset:Hex ] [ Data:Hex ] [StartBit:Dec] [DataLen:Dec]\n"
			"\tType    : access operation type : [m]odify, [w]wite, [d]ump\n"
			"\tOffset  : offset into memory region to act upon\n"
			"\tData    : data to be written\n"
			"\tStartbit: Startbit of Addr that want to be modified. Range[0~31]\n"
			"\tDataLen : Data length of Data. Range[1~32], and Startbit + DataLen <= 32\n\n"
			"Example:\tRead/Write/Modify register \n"
			"\tRead    : regs d 0x1b100000           //dump 0x1b100000~0x1b1000f0 \n"
			"\tWrite   : regs w 0x1b100000 0x1234    //write 0x1b100000=0x1234\n"
			"\tModify  : regs m 0x1b100000 0x0 29 3  //modify 0x1b100000[29:31]=0\n");
}

int main(int argc, char **argv) {
	int fd;
	void *map_base = NULL;
        void *virt_addr = NULL;
	uint32_t read_result =0;
        uint32_t writeval = 0;
	uint32_t startbit = 0;
       	uint32_t datalen = 0;
	char *filename = NULL;
	off_t offset = 0;
	int access_type = 0;

	if(argc < 3) {
		usage();
		exit(1);
	}

	access_type = tolower(argv[1][0]);
	if ((access_type == 'w' && argc < 4) || (access_type == 'm' && argc < 6)) {
		usage();
		exit(1);
	}

	filename = "/dev/mem";
	if((fd = open(filename, O_RDWR | O_SYNC)) == -1)
		PRINT_ERROR;

	/* Map one page */
	offset = strtoul(argv[2], NULL, 16);
	map_base = mmap(0, 2*MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset & ~MAP_MASK);
	if(map_base == (void *) -1)
		PRINT_ERROR;

	virt_addr = map_base + (offset & MAP_MASK);
	read_result = *((volatile uint32_t *) virt_addr);
	printf("Value at 0x%llX (%p): 0x%X\n",
	       (unsigned long long)offset, virt_addr, read_result);

	switch(access_type) {
		case 'm':
			writeval = strtoul(argv[3], 0, 16);
			startbit = strtoul(argv[4], 0, 10);
			datalen  = strtoul(argv[5], 0, 10);
			reg_mod_bits((uint32_t *)virt_addr, writeval, startbit, datalen);
			break;
		case 'w':
			writeval = strtoul(argv[3], 0, 16);
			*((uint32_t *) virt_addr) = writeval;
			printf("Written 0x%X; ", writeval);
			break;
		case 'd':
			dump_page(virt_addr, map_base, (uint32_t *)(offset & ~MAP_MASK));
			goto out;
		default:
			fprintf(stderr, "Illegal data type '%c'.\n", access_type);
			goto out;
	}

	read_result = *((volatile uint32_t *) virt_addr);
	printf("Readback 0x%X\n", read_result);

out:
	if(munmap(map_base, MAP_SIZE) == -1)
		PRINT_ERROR;

	close(fd);
	return 0;
}
