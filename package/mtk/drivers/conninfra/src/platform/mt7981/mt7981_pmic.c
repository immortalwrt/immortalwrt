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
#include <linux/regulator/consumer.h>
#include <linux/notifier.h>

#include "consys_hw.h"
#include "consys_reg_util.h"
#include "osal.h"
#include "mt7981_pmic.h"
#include "mt7981_pos.h"
#include "mt7981_consys_reg.h"
#include "mt7981_consys_reg_offset.h"

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


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
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

struct consys_platform_pmic_ops g_consys_platform_pmic_ops_mt7981 = {
	.consys_pmic_get_from_dts = NULL,
	.consys_pmic_common_power_ctrl = NULL,
	.consys_pmic_wifi_power_ctrl = NULL,
	.consys_pmic_bt_power_ctrl = NULL,
	.consys_pmic_gps_power_ctrl = NULL,
	.consys_pmic_fm_power_ctrl = NULL,
	.consys_pmic_event_notifier = NULL,
};

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

struct consys_platform_pmic_ops* get_consys_platform_pmic_ops(void)
{
	return &g_consys_platform_pmic_ops_mt7981;
}

