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
#ifndef _RING_H_
#define _RING_H_

struct ring {
	/* addr where ring buffer starts */
	void *base;
	/* addr storing the next writable pos, guaranteed to be >= read except when write overflow, but it's ok. */
	unsigned int write;
	/* addr storing the next readable pos, except when read == write as buffer empty */
	unsigned int read;
	/* must be power of 2 */
	unsigned int max_size;
};

struct ring_segment {
	/* addr points into ring buffer for read/write */
	void *ring_pt;
	/* size to read/write */
	unsigned int sz;
	/* pos in external data buffer to read/write */
	unsigned int data_pos;
	/* the size to be read/write after this segment completed */
	unsigned int remain;
};

void ring_init(void *base, unsigned int max_size, unsigned int read,
	unsigned int write, struct ring *ring);
unsigned int ring_read_prepare(unsigned int sz, struct ring_segment *seg, struct ring *ring);
#define ring_read_all_prepare(seg, ring)  ring_read_prepare((ring)->max_size, seg, ring)
unsigned int ring_write_prepare(unsigned int sz, struct ring_segment *seg, struct ring *ring);
unsigned int ring_overwrite_prepare(unsigned int sz,
	struct ring_segment *seg, struct ring *ring);

/* making sure max_size is power of 2 */
#define RING_VALIDATE_SIZE(max_size) WARN_ON(!max_size || (max_size & (max_size - 1)))

#define RING_EMPTY(ring) ((ring)->read == (ring)->write)
/* equation works even when write overflow */
#define RING_SIZE(ring) ((ring)->write - (ring)->read)
#define RING_FULL(ring) (RING_SIZE(ring) == (ring)->max_size)
#define RING_WRITE_REMAIN_SIZE(ring) ((ring)->max_size - RING_SIZE(ring))

#define RING_READ_FOR_EACH(_sz, _seg, _ring) \
	for (ring_read_prepare(_sz, &(_seg), _ring), \
		_ring_segment_prepare((_ring)->read, &(_seg), (_ring)); \
		(_seg).sz > 0; \
		_ring_read_commit(&(_seg), (_ring)), _ring_segment_prepare((_ring)->read, \
			&(_seg), (_ring)))

#define RING_READ_ALL_FOR_EACH(seg, ring) RING_READ_FOR_EACH((ring)->max_size, seg, ring)

#define RING_READ_FOR_EACH_ITEM(_sz, _seg, _ring) \
	for (ring_read_prepare(_sz, &(_seg), _ring), \
		_ring_segment_prepare_item((_ring)->read, &(_seg), (_ring)); \
		(_seg).sz > 0; \
		_ring_read_commit(&(_seg), (_ring)), _ring_segment_prepare_item((_ring)->read, \
			&(_seg), (_ring)))

#define RING_WRITE_FOR_EACH(_sz, _seg, _ring) \
	for (ring_write_prepare(_sz, &(_seg), _ring),\
		_ring_segment_prepare((_ring)->write, &(_seg), (_ring)); \
		(_seg).sz > 0; \
		_ring_write_commit(&(_seg), (_ring)), _ring_segment_prepare((_ring)->write, \
			&(_seg), (_ring)))

#define RING_OVERWRITE_FOR_EACH(_sz, _seg, _ring) \
	for (ring_overwrite_prepare(_sz, &(_seg), _ring), \
		_ring_segment_prepare((_ring)->write, &(_seg), (_ring)); \
		(_seg).sz > 0; \
		_ring_write_commit(&(_seg), (_ring)), _ring_segment_prepare((_ring)->write, \
			&(_seg), (_ring)))

void ring_dump(const char *title, struct ring *ring);
void ring_dump_segment(const char *title, struct ring_segment *seg);


/* ring Buffer Internal API */
void _ring_segment_prepare(unsigned int from, struct ring_segment *seg, struct ring *ring);
void _ring_segment_prepare_item(unsigned int from, struct ring_segment *seg, struct ring *ring);
void _ring_read_commit(struct ring_segment *seg, struct ring *ring);
void _ring_write_commit(struct ring_segment *seg, struct ring *ring);

#endif
