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

#ifndef _PLATFORM_EMI_MNG_H_
#define _PLATFORM_EMI_MNG_H_

#include <linux/platform_device.h>
#include <linux/types.h>
#include "osal.h"

#include "consys_hw.h"

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

struct consys_emi_addr_info {
	/* This include BT/WF FW and WFDMA */
	phys_addr_t emi_ap_phy_base;
	unsigned int emi_ap_phy_size;
	unsigned int fw_emi_size;
};

typedef int(*CONSYS_IC_EMI_SET_REGION_PROTECTION) (void);
typedef int(*CONSYS_IC_EMI_SET_REMAPPING_REG) (void);
typedef unsigned int (*CONSYS_IC_GET_FW_EMI_SIZE)(void);

struct consys_platform_emi_ops {
	CONSYS_IC_EMI_SET_REGION_PROTECTION consys_ic_emi_set_region_protection;
	CONSYS_IC_EMI_SET_REMAPPING_REG consys_ic_emi_set_remapping_reg;
	CONSYS_IC_GET_FW_EMI_SIZE consys_ic_emi_get_fw_emi_size;
};

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

int emi_mng_init(struct platform_device *pdev, const struct conninfra_plat_data* plat_data);
int emi_mng_deinit(void);

int emi_mng_set_region_protection(void);
int emi_mng_set_remapping_reg(void);
struct consys_emi_addr_info* emi_mng_get_phy_addr(void);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif				/* _PLATFORM_EMI_MNG_H_ */

