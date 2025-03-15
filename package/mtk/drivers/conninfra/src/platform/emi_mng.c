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

#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/io.h>
#include <linux/types.h>
#include "osal.h"

#include "consys_hw.h"
#include "emi_mng.h"

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

unsigned long long gConEmiSize = 0;
phys_addr_t gConEmiPhyBase = 0x0;

const struct consys_platform_emi_ops* consys_platform_emi_ops = NULL;

struct consys_emi_addr_info connsys_emi_addr_info = {
	.emi_ap_phy_base = 0,
	.emi_ap_phy_size = 0,
	.fw_emi_size = 0,
};

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

int emi_mng_set_region_protection(void)
{
	if (consys_platform_emi_ops &&
		consys_platform_emi_ops->consys_ic_emi_set_region_protection)
		return consys_platform_emi_ops->consys_ic_emi_set_region_protection();

	return -1;
}

int emi_mng_set_remapping_reg(void)
{
	if (consys_platform_emi_ops &&
		consys_platform_emi_ops->consys_ic_emi_set_remapping_reg)
		return consys_platform_emi_ops->consys_ic_emi_set_remapping_reg();

	return -1;
}

struct consys_emi_addr_info* emi_mng_get_phy_addr(void)
{
	return &connsys_emi_addr_info;
}

int emi_mng_init(struct platform_device *pdev, const struct conninfra_plat_data* plat_data)
{
	unsigned int fw_emi_size = 0;

#ifdef CONFIG_CONNINFRA_EMI_SUPPORT
	struct device_node *np;
	struct reserved_mem *rmem;

	np = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
	if (!np) {
		pr_info("[%s] memory region not found.\n", __func__);
		return -1;
	}

	rmem = of_reserved_mem_lookup(np);
	if (!rmem) {
		pr_info("[%s] no memory-region\n", __func__);
		return -1;
	} else {
		gConEmiPhyBase = rmem->base;
		gConEmiSize = rmem->size;
	}
#else
	pr_info("Conninfra not support EMI reservation for %04x\n", plat_data->chip_id);
#endif /* CONFIG_CONNINFRA_EMI_SUPPORT */

	if (consys_platform_emi_ops == NULL) {
		consys_platform_emi_ops = (const struct consys_platform_emi_ops*)plat_data->platform_emi_ops;
	}

	if (consys_platform_emi_ops && consys_platform_emi_ops->consys_ic_emi_get_fw_emi_size)
		fw_emi_size = consys_platform_emi_ops->consys_ic_emi_get_fw_emi_size();

	pr_info("[emi_mng_init] gConEmiPhyBase = [0x%llx] size = [0x%llx] fw size = [0x%x] ops=[%p]\n",
			gConEmiPhyBase, gConEmiSize, fw_emi_size, consys_platform_emi_ops);

	if (gConEmiPhyBase) {
		connsys_emi_addr_info.emi_ap_phy_base = gConEmiPhyBase;
		connsys_emi_addr_info.emi_ap_phy_size = gConEmiSize;
		connsys_emi_addr_info.fw_emi_size = fw_emi_size;
	} else {
		pr_err("consys emi memory address gConEmiPhyBase invalid\n");
	}

	return 0;
}

int emi_mng_deinit(void)
{
	return 0;
}
