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

#define pr_fmt(fmt) KBUILD_MODNAME "@(%s:%d) " fmt, __func__, __LINE__

#include <linux/memblock.h>
#include <linux/platform_device.h>
#include <linux/of_reserved_mem.h>
#include "mt7986_emi.h"
#include "mt7986.h"
#include "mt7986_consys_reg.h"
#include "consys_hw.h"
#include "consys_reg_util.h"
#include "mt7986_pos.h"

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
unsigned int consys_emi_get_fw_emi_size(void)
{
	return 0x100000;
}

struct consys_platform_emi_ops g_consys_platform_emi_ops_mt7986 = {
	.consys_ic_emi_set_region_protection = consys_emi_set_region_protection,
	.consys_ic_emi_set_remapping_reg = consys_emi_set_remapping_reg,
	.consys_ic_emi_get_fw_emi_size = consys_emi_get_fw_emi_size,
};

struct consys_platform_emi_ops* get_consys_platform_emi_ops(void)
{
	return &g_consys_platform_emi_ops_mt7986;
}

