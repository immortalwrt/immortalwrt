/* $Id: rw_unaligned.h,v 1.1 2022/10/16 06:02:01 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2012-2022 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef RW_UNALIGNED_H_INCLUDED
#define RW_UNALIGNED_H_INCLUDED

#include <stdint.h>

#ifndef INLINE
#define INLINE static inline
#endif
/* theses macros are designed to read/write unsigned short/long int
 * from an unsigned char array in network order (big endian).
 * Avoid pointer casting, so avoid accessing unaligned memory, which
 * can crash with some cpu's */
INLINE uint32_t readnu32(const uint8_t * p)
{
	return (p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]);
}
#define READNU32(p) readnu32(p)
INLINE uint16_t readnu16(const uint8_t * p)
{
	return (p[0] << 8 | p[1]);
}
#define READNU16(p) readnu16(p)
INLINE void writenu32(uint8_t * p, uint32_t n)
{
	p[0] = (n & 0xff000000) >> 24;
	p[1] = (n & 0xff0000) >> 16;
	p[2] = (n & 0xff00) >> 8;
	p[3] = n & 0xff;
}
#define WRITENU32(p, n) writenu32(p, n)
INLINE void writenu16(uint8_t * p, uint16_t n)
{
	p[0] = (n & 0xff00) >> 8;
	p[1] = n & 0xff;
}
#define WRITENU16(p, n) writenu16(p, n)

#endif /* RW_UNALIGNED_H_INCLUDED */
