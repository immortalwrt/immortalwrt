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

#include <linux/memblock.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/random.h>
#include "consys_reg_mng.h"
#include "mt7981_consys_reg.h"
#include "mt7981_consys_reg_offset.h"
#include "consys_hw.h"
#include "consys_reg_util.h"

#define CFG_REG_LOAD_FROM_DTS_CTRL 0

static int consys_reg_init(struct platform_device *pdev);
static int consys_reg_deinit(void);

struct consys_base_addr conn_reg = {
	.reg_base_addr[TOP_MISC_BASE]                                 = {0x11D10000, 0x1000, 0},
	.reg_base_addr[TOPRGU_BASE]                                   = {0x1001C000, 0x1000, 0},
	.reg_base_addr[GPIO_BASE]                                     = {0x11D00000, 0x1000, 0},
	.reg_base_addr[IOCFG_TM_BASE]                                 = {0x11F00000, 0x1000, 0},
	.reg_base_addr[IOCFG_LT_BASE]                                 = {0x11F10000, 0x1000, 0},
	.reg_base_addr[INFRACFG_AO_BASE]                              = {0x10003000, 0x1000, 0},
	.reg_base_addr[CONN_INFRA_CFG_BASE]                           = {0x18001000, 0x1000, 0},
	.reg_base_addr[CONN_INFRA_SYSRAM_BASE]                        = {0x18050000, 0x1000, 0},
	.reg_base_addr[CONN_INFRA_CLKGEN_ON_TOP_BASE]                 = {0x18009000, 0x1000, 0},
	.reg_base_addr[CONN_HOST_CSR_TOP_BASE]                        = {0x18060000, 0x1000, 0},
	.reg_base_addr[CONN_INFRA_BUS_CR_BASE]                        = {0x1800E000, 0x1000, 0},
	.reg_base_addr[CONN_INFRA_RGU_BASE]                           = {0x18000000, 0x1000, 0},
	.reg_base_addr[CONN_WT_SLP_CTL_REG_BASE]                      = {0x18005000, 0x1000, 0},
	.reg_base_addr[INST2_CONN_WT_SLP_CTL_REG_BASE]                = {0x18085000, 0x1000, 0},
	.reg_base_addr[CONN_RF_SPI_MST_REG_BASE]                      = {0x18004000, 0x1000, 0},
	.reg_base_addr[INST2_CONN_RF_SPI_MST_REG_BASE]                = {0x18084000, 0x1000, 0},
	.reg_base_addr[CONN_SEMAPHORE_BASE]                           = {0x18070000, 0x10000, 0},
	.reg_base_addr[CONN_AFE_CTL_BASE]                             = {0x18003000, 0x1000, 0},
	.reg_base_addr[CONN_AFE_CTL_2ND_BASE]                         = {0x18083000, 0x1000, 0},
	.reg_base_addr[WF_TOP_SLPPROT_ON_BASE]                        = {0x184C0000, 0x10000, 0},
	.reg_base_addr[WF_TOP_CFG_BASE]                               = {0x184B0000, 0x1000, 0},
	.reg_base_addr[WF_MCU_CONFIG_LS_BASE]                         = {0x184F0000, 0x1000, 0},
	.reg_base_addr[WF_MCU_BUS_CR_BASE]                            = {0x18400000, 0x1000, 0},
	.reg_base_addr[WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_BASE] = {0x18500000, 0x1000, 0},
	.reg_base_addr[WF_TOP_CFG_ON_BASE]                            = {0x184C0000, 0x10000, 0},
};

const char* consys_base_addr_index_to_str[CONSYS_BASE_ADDR_MAX] = {
	"TOP_MISC_BASE",
	"TOPRGU_BASE",
	"GPIO_BASE",
	"IOCFG_TR_BASE",
	"IOCFG_TL_BASE",
	"INFRACFG_AO_BASE",
	"CONN_INFRA_CFG_BASE",
	"CONN_INFRA_SYSRAM_BASE",
	"CONN_INFRA_CLKGEN_ON_TOP_BASE",
	"CONN_HOST_CSR_TOP_BASE",
	"CONN_INFRA_BUS_CR_BASE",
	"CONN_INFRA_RGU_BASE",
	"CONN_WT_SLP_CTL_REG_BASE",
	"INST2_CONN_WT_SLP_CTL_REG_BASE",
	"CONN_RF_SPI_MST_REG_BASE",
	"INST2_CONN_RF_SPI_MST_REG_BASE",
	"CONN_SEMAPHORE_BASE",
	"CONN_AFE_CTL_BASE",
	"CONN_AFE_CTL_2ND_BASE",
	"WF_TOP_SLPPROT_ON_BASE",
	"WF_TOP_CFG_BASE",
	"WF_MCU_CONFIG_LS_BASE",
	"WF_MCU_BUS_CR_BASE",
	"WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_BASE",
	"WF_TOP_CFG_ON_BASE"
};

struct consys_reg_mng_ops g_dev_consys_reg_ops_mt7981 = {
	.consys_reg_mng_init = consys_reg_init,
	.consys_reg_mng_deinit = consys_reg_deinit,
	.consys_reg_mng_check_reable = NULL,
	.consys_reg_mng_is_consys_reg = NULL,
	.consys_reg_mng_is_bus_hang = NULL,
	.consys_reg_mng_dump_bus_status = NULL,
	.consys_reg_mng_dump_conninfra_status = NULL,
	.consys_reg_mng_dump_cpupcr = NULL,
	.consys_reg_mng_is_host_csr = NULL,
};

struct consys_base_addr* get_conn_reg_base_addr()
{
	return &conn_reg;
}

static int consys_reg_init(struct platform_device *pdev)
{
	int ret = -1;
	struct device_node *node = NULL;
	struct consys_reg_base_addr *base_addr = NULL;
	int i = 0;

	node = pdev->dev.of_node;
	if (node) {
#if (CFG_REG_LOAD_FROM_DTS_CTRL == 1)
		struct resource res;
		int flag;

		for (i = 0; i < CONSYS_BASE_ADDR_MAX; i++) {
			base_addr = &conn_reg.reg_base_addr[i];
			ret = of_address_to_resource(node, i, &res);
			if (ret) {
				pr_err("Get Reg Index(%d-%s) failed\n", i, consys_base_addr_index_to_str[i]);
				continue;
			}
			base_addr->phy_addr = res.start;
			base_addr->vir_addr = (unsigned long)of_iomap(node, i);
			of_get_address(node, i, &(base_addr->size), &flag);
#if 0
			pr_info("Get Index(%d-%s) phy_addr(0x%zx) vir_addr=(0x%zx) size=(0x%zx)\n",
					i, consys_base_addr_index_to_str[i], base_addr->phy_addr,
					base_addr->vir_addr, base_addr->size);
#endif
		}
#else
		for (i = 0; i < CONSYS_BASE_ADDR_MAX; i++) {
			base_addr = &conn_reg.reg_base_addr[i];
			if (base_addr->vir_addr == 0)
				base_addr->vir_addr = (unsigned long)ioremap(base_addr->phy_addr, base_addr->size);

			pr_info("Get Index(%d-%s) phy_addr(0x%zx) vir_addr=(0x%zx) size=(0x%zx)\n",
					i, consys_base_addr_index_to_str[i], base_addr->phy_addr,
					base_addr->vir_addr, base_addr->size);
		}
#endif
	} else {
		pr_err("[%s] can't find CONSYS compatible node\n", __func__);
		return ret;
	}

	return 0;
}

static int consys_reg_deinit(void)
{
	int i = 0;

	for (i = 0; i < CONSYS_BASE_ADDR_MAX; i++) {
		if (conn_reg.reg_base_addr[i].vir_addr) {
			pr_info("[%d] Unmap %s (0x%zx)\n", i, consys_base_addr_index_to_str[i],
					conn_reg.reg_base_addr[i].vir_addr);
			iounmap((void __iomem*)conn_reg.reg_base_addr[i].vir_addr);
			conn_reg.reg_base_addr[i].vir_addr = 0;
		}
	}

	return 0;
}

