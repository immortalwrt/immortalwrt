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

#ifndef _PLATFORM_MT7981_CONSYS_REG_H_
#define _PLATFORM_MT7981_CONSYS_REG_H_

#include "consys_reg_base.h"
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
enum consys_base_addr_index {
	TOP_MISC_BASE = 0,									/* top_misc */
	TOPRGU_BASE   = 1,									/* TOPRGU */
	GPIO_BASE = 2,										/* GPIO */
	IOCFG_TM_BASE = 3, 									/* IOCFG_TM */
	IOCFG_LT_BASE = 4, 									/* IOCFG_LT */
	INFRACFG_AO_BASE = 5,								/* infracfg_ao_auto_gen_reg */
	CONN_INFRA_CFG_BASE = 6,							/* conn_infra_cfg */
	CONN_INFRA_SYSRAM_BASE = 7,							/* conn_infra_sysram */
	CONN_INFRA_CLKGEN_ON_TOP_BASE = 8,  				/* conn_infra_clkgen_on_top */
	CONN_HOST_CSR_TOP_BASE = 9,							/* conn_host_csr_top */
	CONN_INFRA_BUS_CR_BASE = 10,						/* conn_infra_bus_cr */
	CONN_INFRA_RGU_BASE = 11,							/* conn_infra_rgu */
	CONN_WT_SLP_CTL_REG_BASE = 12,						/* conn_wt_slp_ctl_reg */
	INST2_CONN_WT_SLP_CTL_REG_BASE = 13,				/* Inst2_conn_wt_slp_ctl_reg */
	CONN_RF_SPI_MST_REG_BASE = 14,						/* conn_rf_spi_mst_reg */
	INST2_CONN_RF_SPI_MST_REG_BASE = 15,				/* Inst2_conn_rf_spi_mst_reg */
	CONN_SEMAPHORE_BASE = 16, 							/* conn_semaphore */
	CONN_AFE_CTL_BASE = 17,								/* conn_afe_ctl */
	CONN_AFE_CTL_2ND_BASE = 18,							/* conn_afe_ctl_2nd */
	WF_TOP_SLPPROT_ON_BASE = 19,						/* wf_top_slpprot_on by remapping to 0x81020000 */
	WF_TOP_CFG_BASE = 20,								/* wf_top_cfg by remapping to 0x80020000 */
	WF_MCU_CONFIG_LS_BASE = 21,							/* wf_mcu_confg_ls by remapping to 0x88000000 */
	WF_MCU_BUS_CR_BASE = 22,							/* wf_mcu_bus_cr by remapping to 0x830C0XXX */
	WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_BASE = 23,	/* wf_mcusys_infra_bus_full_u_debug_ctrl_ao by remapping to 0x810F0000 */
	WF_TOP_CFG_ON_BASE = 24,							/* wf_top_cfg_on by remapping to 0x81021000 */
	CONSYS_BASE_ADDR_MAX
};

struct consys_base_addr {
	struct consys_reg_base_addr reg_base_addr[CONSYS_BASE_ADDR_MAX];
};

extern struct consys_base_addr conn_reg;

#define REG_TOP_MISC_ADDR										conn_reg.reg_base_addr[TOP_MISC_BASE].vir_addr
#define REG_TOP_RGU_ADDR										conn_reg.reg_base_addr[TOPRGU_BASE].vir_addr
#define REG_GPIO_BASE_ADDR										conn_reg.reg_base_addr[GPIO_BASE].vir_addr
#define REG_IOCFG_TM_ADDR										conn_reg.reg_base_addr[IOCFG_TM_BASE].vir_addr
#define REG_IOCFG_LT_ADDR										conn_reg.reg_base_addr[IOCFG_LT_BASE].vir_addr
#define REG_INFRACFG_AO_ADDR									conn_reg.reg_base_addr[INFRACFG_AO_BASE].vir_addr
#define REG_CONN_INFRA_CFG_ADDR									conn_reg.reg_base_addr[CONN_INFRA_CFG_BASE].vir_addr
#define REG_CONN_INFRA_SYSRAM_ADDR								conn_reg.reg_base_addr[CONN_INFRA_SYSRAM_BASE].vir_addr
#define REG_CONN_INFRA_CLKGEN_ON_TOP_ADDR						conn_reg.reg_base_addr[CONN_INFRA_CLKGEN_ON_TOP_BASE].vir_addr
#define REG_CONN_HOST_CSR_TOP_ADDR								conn_reg.reg_base_addr[CONN_HOST_CSR_TOP_BASE].vir_addr
#define REG_CONN_INFRA_BUS_CR_ADDR								conn_reg.reg_base_addr[CONN_INFRA_BUS_CR_BASE].vir_addr
#define REG_CONN_INFRA_RGU_ADDR									conn_reg.reg_base_addr[CONN_INFRA_RGU_BASE].vir_addr
#define REG_CONN_WT_SLP_CTL_REG_ADDR							conn_reg.reg_base_addr[CONN_WT_SLP_CTL_REG_BASE].vir_addr
#define REG_INST2_CONN_WT_SLP_CTL_REG_ADDR						conn_reg.reg_base_addr[INST2_CONN_WT_SLP_CTL_REG_BASE].vir_addr
#define REG_CONN_RF_SPI_MST_REG_ADDR							conn_reg.reg_base_addr[CONN_RF_SPI_MST_REG_BASE].vir_addr
#define REG_INST2_CONN_RF_SPI_MST_REG_ADDR						conn_reg.reg_base_addr[INST2_CONN_RF_SPI_MST_REG_BASE].vir_addr
#define REG_CONN_SEMAPHORE_ADDR									conn_reg.reg_base_addr[CONN_SEMAPHORE_BASE].vir_addr
#define REG_CONN_AFE_CTL_ADDR									conn_reg.reg_base_addr[CONN_AFE_CTL_BASE].vir_addr
#define REG_CONN_AFE_CTL_2ND_ADDR								conn_reg.reg_base_addr[CONN_AFE_CTL_2ND_BASE].vir_addr
#define REG_WF_TOP_SLPPROT_ON_ADDR								conn_reg.reg_base_addr[WF_TOP_SLPPROT_ON_BASE].vir_addr
#define REG_WF_TOP_CFG_ADDR										conn_reg.reg_base_addr[WF_TOP_CFG_BASE].vir_addr
#define REG_WF_MCU_CONFIG_LS_ADDR								conn_reg.reg_base_addr[WF_MCU_CONFIG_LS_BASE].vir_addr
#define REG_WF_MCU_BUS_CR_ADDR									conn_reg.reg_base_addr[WF_MCU_BUS_CR_BASE].vir_addr
#define REG_WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_ADDR		conn_reg.reg_base_addr[WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_BASE].vir_addr
#define REG_WF_TOP_CFG_ON_ADDR									conn_reg.reg_base_addr[WF_TOP_CFG_ON_BASE].vir_addr

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

struct consys_base_addr* get_conn_reg_base_addr(void);

#endif				/* _PLATFORM_MT7981_CONSYS_REG_H_ */
