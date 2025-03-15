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
/*! \file
*    \brief  Declaration of library functions
*
*    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

#ifndef _PLATFORM_CONSYS_REG_UTIL_H_
#define _PLATFORM_CONSYS_REG_UTIL_H_

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
/* platform dependent */
#include "plat_def.h"

#ifndef BIT
#define BIT(x) (1<<(x))
#endif

#define KBYTE (1024*sizeof(char))
#ifndef GENMASK
#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#endif

#define GET_BIT_MASK(value, mask) ((value) & (mask))
#define SET_BIT_MASK(pdest, value, mask) (*(pdest) = (GET_BIT_MASK(*(pdest), ~(mask)) | GET_BIT_MASK(value, mask)))
#define GET_BIT_RANGE(data, end, begin) ((data) & GENMASK(end, begin))
#define SET_BIT_RANGE(pdest, data, end, begin) (SET_BIT_MASK(pdest, data, GENMASK(end, begin)))

#define CONSYS_SET_BIT(REG, BITVAL) (*((volatile unsigned int *)(REG)) |= ((unsigned int)(BITVAL)))
#define CONSYS_CLR_BIT(REG, BITVAL) ((*(volatile unsigned int *)(REG)) &= ~((unsigned int)(BITVAL)))
#define CONSYS_CLR_BIT_WITH_KEY(REG, BITVAL, KEY) {\
	unsigned int val = (*(volatile unsigned int *)(REG)); \
	val &= ~((unsigned int)(BITVAL)); \
	val |= ((unsigned int)(KEY)); \
	(*(volatile unsigned int *)(REG)) = val;\
}
#define CONSYS_REG_READ(addr) (*((volatile unsigned int *)(addr)))
#define CONSYS_REG_READ_BIT(addr, BITVAL) (*((volatile unsigned int *)(addr)) & ((unsigned int)(BITVAL)))
#define CONSYS_REG_WRITE(addr, data)  mt_reg_sync_writel(data, addr)
#define CONSYS_REG_WRITE_RANGE(reg, data, end, begin) {\
	unsigned int val = CONSYS_REG_READ(reg); \
	SET_BIT_RANGE(&val, data, end, begin); \
	CONSYS_REG_WRITE(reg, val); \
}
#define CONSYS_REG_WRITE_MASK(reg, data, mask) {\
	unsigned int val = CONSYS_REG_READ(reg); \
	SET_BIT_MASK(&val, data, mask); \
	CONSYS_REG_WRITE(reg, val); \
}

/*
 * Write value with value_offset bits of right shift and size bits,
 * to the reg_offset-th bit of address reg
 * value  -----------XXXXXXXXXXXX-------------------
 *                   |<--size-->|<--value_offset-->|
 * reg    -------------OOOOOOOOOOOO-----------------
 *                     |<--size-->|<--reg_offset-->|
 * result -------------XXXXXXXXXXXX-----------------
 */
#define CONSYS_REG_WRITE_OFFSET_RANGE(reg, value, reg_offset, value_offset, size) ({\
	unsigned int data = (value) >> (value_offset); \
	data = GET_BIT_RANGE(data, size, 0); \
	data = data << (reg_offset); \
	CONSYS_REG_WRITE_RANGE(reg, data, ((reg_offset) + ((size) - 1)), reg_offset); \
})

#define CONSYS_REG_WRITE_BIT(reg, offset, val) CONSYS_REG_WRITE_OFFSET_RANGE(reg, ((val) & 1), offset, 0, 1)

#define CONSYS_REG_BIT_POLLING(addr, bit_index, exp_val, loop, delay, success) {\
	unsigned int polling_count = 0; \
	unsigned int reg_value = 0; \
	success = 0; \
	reg_value = (CONSYS_REG_READ_BIT(addr, (0x1 << bit_index)) >> bit_index); \
	while (reg_value != exp_val) { \
		if (polling_count > loop) { \
			success = -1; \
			break; \
		} \
		reg_value = (CONSYS_REG_READ_BIT(addr, (0x1 << bit_index)) >> bit_index); \
		udelay(delay); \
		polling_count++; \
	} \
}

#define CONSYS_REG_POLLING_LARGER_OR_EQUAL(addr, mask, bit_index, exp_val, loop, delay, success) {\
	unsigned int polling_count = 0; \
	unsigned int reg_value = 0; \
	success = 0; \
	reg_value = ((CONSYS_REG_READ(addr) & mask) >> bit_index); \
	while (reg_value < exp_val) { \
		if (polling_count > loop) { \
			success = -1; \
			break; \
		} \
		reg_value = ((CONSYS_REG_READ(addr) & mask) >> bit_index); \
		udelay(delay); \
		polling_count++; \
	} \
}

#define CONSYS_REG_POLLING_EQUAL(addr, mask, bit_index, exp_val, loop, delay, success) {\
	unsigned int polling_count = 0; \
	unsigned int reg_value = 0; \
	success = 0; \
	reg_value = ((CONSYS_REG_READ(addr) & mask) >> bit_index); \
	while (reg_value != exp_val) { \
		if (polling_count > loop) { \
			success = -1; \
			break; \
		} \
		reg_value = ((CONSYS_REG_READ(addr) & mask) >> bit_index); \
		udelay(delay); \
		polling_count++; \
	} \
}

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/


/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/


/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif				/* _PLATFORM_CONSYS_REG_UTIL_H_ */
