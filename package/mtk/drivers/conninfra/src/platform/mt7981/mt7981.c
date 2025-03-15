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

#include <linux/delay.h>
#include <linux/memblock.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include "osal.h"
#include "conninfra.h"
#include "consys_hw.h"
#include "consys_reg_mng.h"
#include "consys_reg_util.h"
#include "mt7981.h"
#include "mt7981_pos.h"
#include "emi_mng.h"
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
#define PLATFORM_SOC_CHIP 0x7981
#define CONN_IP_VER		  0x02090000

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
struct consys_hw_ops_struct g_consys_hw_ops_mt7981 = {
	/* HW init */
	.consys_plt_hw_init = consys_plt_hw_init,

	/* POS */
	.consys_plt_xtal_ctrl_fast_mode = consys_xtal_ctrl_fast_mode,
	.consys_plt_connsys_sw_reset_ctrl = consys_sw_reset_ctrl,
	.consys_plt_set_if_pinmux = consys_set_if_pinmux,
	.consys_plt_tx_rx_bus_slp_prot_ctrl = consys_tx_rx_bus_slp_prot_ctrl,
	.consys_plt_polling_consys_chipid = consys_polling_chipid,
	.consys_plt_bus_clock_ctrl = consys_bus_clock_ctrl,
	.consys_plt_d_die_cfg = connsys_d_die_cfg,
	.consys_plt_conninfra_sysram_hw_ctrl = connsys_conninfra_sysram_hw_ctrl,
	.consys_plt_spi_master_cfg = connsys_spi_master_cfg,
	.consys_plt_a_die_cfg = connsys_a_die_cfg,
	.consys_plt_afe_wbg_cal = connsys_afe_wbg_cal,
	.consys_plt_subsys_pll_initial = connsys_subsys_pll_initial,
	.consys_plt_osc_legacy_mode = connsys_osc_legacy_mode,
	.consys_plt_top_pwr_ctrl = connsys_top_pwr_ctrl,
	.consys_plt_conn_infra_bus_timeout = connsys_conn_infra_bus_timeout,
	.consys_plt_clkgen_wpll_hw_ctrl = connsys_clkgen_wpll_hw_ctrl,
	.consys_plt_conninfra_wakeup = consys_conninfra_top_wakeup,
	.consys_plt_conninfra_sleep = consys_conninfra_top_sleep,
	.consys_plt_adie_top_ck_en_on_off_ctrl = consys_adie_top_ck_en_on_off_ctrl,
	.consys_plt_conninfra_wf_wakeup = consys_conninfra_wf_wakeup,
	.consys_plt_conninfra_wf_sleep = consys_conninfra_wf_sleep,
	.consys_plt_conn_wmcpu_sw_reset = consys_conn_wmcpu_sw_reset,
	.consys_plt_wf_bus_slp_prot_ctrl = consys_wf_bus_slp_prot_ctrl,
	.consys_plt_wfsys_top_on_ctrl = consys_wfsys_top_on_ctrl,
	.consys_plt_wfsys_bus_slp_prot_check = consys_wfsys_bus_slp_prot_check,
	.consys_plt_wfsys_bus_timeout_ctrl = consys_wfsys_bus_timeout_ctrl,
	.consys_plt_conn_wmcpu_idle_loop_check = consys_wmcpu_idle_loop_check,
	.consys_plt_wpll_ctrl = consys_wpll_ctrl,
	.consys_plt_conninfra_wf_req_clr = consys_conninfra_wf_req_clr,

	/* load from dts */
	/* TODO: mtcmos should move to a independent module */
	.consys_plt_clk_get_from_dts = NULL,
	.consys_plt_clk_detach = NULL,

	/* clock */
	.consys_plt_soc_chipid_get = consys_soc_chipid_get,

	/* debug */
	.consys_plt_get_hw_ver = consys_get_hw_ver,
	.consys_plt_spi_read = consys_spi_read,
	.consys_plt_spi_write = consys_spi_write,
	.consys_plt_spi_clock_switch = NULL,
	.consys_plt_power_state = NULL,

	/* others */
	.consys_plt_adie_type_cfg = consys_plt_adie_type_cfg,
};

/* For mt7981 */
extern struct consys_hw_ops_struct g_consys_hw_ops_mt7981;
extern struct consys_reg_mng_ops g_dev_consys_reg_ops_mt7981;
extern struct consys_platform_emi_ops g_consys_platform_emi_ops_mt7981;
extern struct consys_platform_pmic_ops g_consys_platform_pmic_ops_mt7981;

const struct conninfra_plat_data mt7981_plat_data = {
	.chip_id = PLATFORM_SOC_CHIP,
	.hw_ops = &g_consys_hw_ops_mt7981,
	.reg_ops = &g_dev_consys_reg_ops_mt7981,
	.platform_emi_ops = &g_consys_platform_emi_ops_mt7981,
	.platform_pmic_ops = &g_consys_platform_pmic_ops_mt7981,
};

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
unsigned int consys_soc_chipid_get(void)
{
	return PLATFORM_SOC_CHIP;
}

unsigned int consys_get_hw_ver(void)
{
	return CONN_IP_VER;
}

