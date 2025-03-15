/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */
#include "ring.h"
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/bug.h>


void ring_init(void *base, unsigned int max_size, unsigned int read,
	unsigned int write, struct ring *ring)
{
	WARN_ON(!base);

	/* making sure max_size is power of 2 */
	WARN_ON(!max_size || (max_size & (max_size - 1)));

	/* making sure write largger than read */
	WARN_ON(read > write);

	ring->base = base;
	ring->read = read;
	ring->write = write;
	ring->max_size = max_size;
}

void ring_dump(const char *title, struct ring *ring)
{
	pr_info("[%s] ring:{write=%d, read=%d, max_size=%d}\n",
			title, ring->write, ring->read, ring->max_size);
}

void ring_dump_segment(const char *title, struct ring_segment *seg)
{
	pr_info("[%s] seg:{ring_pt=0x%p, data_pos=%d, sz=%d, remain=%d}\n",
			title, seg->ring_pt, seg->data_pos,
			seg->sz, seg->remain);
}

/*
 * Function prepares the ring_segment and
 * returns the number of valid bytes for read.
 */
unsigned int ring_read_prepare(unsigned int sz,
					struct ring_segment *seg,
					struct ring *ring)
{
	unsigned int wt = ring->write;
	unsigned int rd = ring->read;

	memset(seg, 0, sizeof(struct ring_segment));
	if (sz > wt - rd)
		sz = wt - rd;
	seg->remain = sz;
	/* ring_dump(__func__, ring); */
	/* ring_dump_segment(__func__, seg); */
	return seg->remain;
}

/*
 * Function prepares the ring_segment and
 * returns the number of bytes available for write.
 */
unsigned int ring_write_prepare(unsigned int sz,
						struct ring_segment *seg,
						struct ring *ring)
{
	unsigned int wt = ring->write;
	unsigned int rd = ring->read;

	memset(seg, 0, sizeof(struct ring_segment));
	if (sz > ring->max_size - (wt - rd))
		sz = ring->max_size - (wt - rd);
	seg->remain = sz;
	/* ring_dump(__func__, ring); */
	/* ring_dump_segment(__func__, seg); */
	return seg->remain;
}

unsigned int ring_overwrite_prepare(unsigned int sz, struct ring_segment *seg,
						      struct ring *ring)
{
	unsigned int wt = ring->write;
	unsigned int rd = ring->read;

	memset(seg, 0, sizeof(struct ring_segment));
	if (sz > ring->max_size - (wt - rd))
		ring->read += sz - (ring->max_size - (wt - rd));
	seg->remain = sz;
	/* ring_dump(__func__, ring); */
	/* ring_dump_segment(__func__, seg); */
	return seg->remain;
}

void __ring_segment_prepare(unsigned int from, unsigned int sz,
						struct ring_segment *seg,
						struct ring *ring)
{
	unsigned int ring_pos = from & (ring->max_size - 1);

	seg->ring_pt = ring->base + ring_pos;
	seg->data_pos = (seg->sz ? seg->data_pos + seg->sz : 0);
	if (ring_pos + sz <= ring->max_size)
		seg->sz = sz;
	else
		seg->sz = ring->max_size - ring_pos;
	seg->remain -= seg->sz;
	/* ring_dump(__func__, ring); */
	/* ring_dump_segment(__func__, seg); */
}

void _ring_segment_prepare(unsigned int from,
					struct ring_segment *seg,
					struct ring *ring)
{
	__ring_segment_prepare(from, seg->remain, seg, ring);
}

void _ring_segment_prepare_item(unsigned int from,
					struct ring_segment *seg,
					struct ring *ring)
{
	unsigned int size;

	size = (seg->remain ? 1 : 0);
	__ring_segment_prepare(from, size, seg, ring);
}

void _ring_read_commit(struct ring_segment *seg, struct ring *ring)
{
	ring->read += seg->sz;
	/* ring_dump(__func__, ring); */
	/* ring_dump_segment(__func__, seg); */
}
void _ring_write_commit(struct ring_segment *seg, struct ring *ring)
{
	ring->write += seg->sz;
	/* ring_dump(__func__, ring); */
	/* ring_dump_segment(__func__, seg); */
}

