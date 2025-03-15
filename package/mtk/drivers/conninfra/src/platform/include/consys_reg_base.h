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

#ifndef _PLATFORM_CONSYS_REG_BASE_H_
#define _PLATFORM_CONSYS_REG_BASE_H_

struct consys_reg_base_addr {
	unsigned long phy_addr;
	unsigned long long size;
	unsigned long vir_addr;
};

#endif				/* _PLATFORM_CONSYS_REG_BASE_H_ */
