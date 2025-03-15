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

#ifndef _PLATFORM_CONSYS_HW_H_
#define _PLATFORM_CONSYS_HW_H_

#include <linux/platform_device.h>
#include "conninfra.h"

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

#define CONN_SEMA_GET_SUCCESS	0
#define CONN_SEMA_GET_FAIL	1

#define CONN_SEMA_TIMEOUT	(1*1000) /* 1ms */

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

struct conninfra_dev_cb {
	int (*conninfra_suspend_cb) (void);
	int (*conninfra_resume_cb) (void);
	int (*conninfra_pmic_event_notifier) (unsigned int, unsigned int);
};

typedef int(*CONSYS_PLT_HW_INIT)(void);
typedef int(*CONSYS_PLT_XTAL_CTRL_FAST_MODE)(void);
typedef int(*CONSYS_PLT_CONNSYS_SW_RESET_CTRL)(bool bassert);
typedef void(*CONSYS_PLT_SET_IF_PINMUX)(bool enable);
typedef int(*CONSYS_PLT_TX_RX_BUS_SLP_PROT_CTRL)(bool enable);
typedef int(*CONSYS_PLT_POLLING_CONSYS_CHIPID)(void);
typedef int(*CONSYS_PLT_BUS_CLOCK_CTRL)(enum consys_drv_type drv_type, unsigned int bus_clock);
typedef int(*CONSYS_PLT_D_DIE_CFG)(void);
typedef int(*CONSYS_PLT_CONNINFRA_SYSRAM_HW_CTRL)(void);
typedef int(*CONSYS_PLT_SPI_MASTER_CFG)(void);
typedef int(*CONSYS_PLT_A_DIE_CFG)(void);
typedef int(*CONSYS_PLT_AFE_WBG_CAL)(void);
typedef int(*CONSYS_PLT_SUBSYS_PLL_INITIAL)(void);
typedef int(*CONSYS_PLT_OSC_LEGACY_MODE)(void);
typedef int(*CONSYS_PLT_TOP_PWR_CTRL)(void);
typedef int(*CONSYS_PLT_CONN_INFRA_BUS_TIMEOUT)(void);
typedef int(*CONSYS_PLT_CLKGEN_WPLL_HW_CTRL)(void);
typedef int(*CONSYS_PLT_CONNINFRA_TOP_WAKEUP) (void);
typedef int(*CONSYS_PLT_CONNINFRA_TOP_SLEEP) (void);
typedef int(*CONSYS_PLT_ADIE_TOP_CK_EN_ON_OFF_CTRL)(enum consys_drv_type type, unsigned char on);
typedef int(*CONSYS_PLT_CONNINFRA_WF_WAKEUP) (void);
typedef int(*CONSYS_PLT_CONNINFRA_WF_SLEEP) (void);
typedef int(*CONSYS_PLT_CONN_WMCPU_SW_RESET) (bool bassert);
typedef int(*CONSYS_PLT_WF_BUS_SLP_PROT_CTRL)(bool enable);
typedef int(*CONSYS_PLT_WFSYS_TOP_ON_CTRL) (bool enable);
typedef int(*CONSYS_PLT_WFSYS_BUS_SLP_PROT_CHECK)(bool enable);
typedef int(*CONSYS_PLT_WFSYS_BUS_TIMEOUT_CTRL) (void);
typedef int(*CONSYS_PLT_CONN_WMCPU_IDLE_LOOP_CHECK) (void);
typedef int(*CONSYS_PLT_WPLL_CTRL)(bool enable);
typedef int(*CONSYS_PLT_CONNINFRA_WF_REQ_CLR) (void);
typedef int(*CONSYS_PLT_CLK_GET_FROM_DTS) (struct platform_device *pdev);
typedef int(*CONSYS_PLT_CLK_DETACH) (void);
typedef int(*CONSYS_PLT_CO_CLOCK_TYPE) (void);
typedef unsigned int(*CONSYS_PLT_SOC_CHIPID_GET) (void);
typedef unsigned int(*CONSYS_PLT_GET_HW_VER)(void);
typedef int(*CONSYS_PLT_SPI_READ)(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int *data);
typedef int(*CONSYS_PLT_SPI_WRITE)(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int data);
typedef int(*CONSYS_PLT_SPI_CLOCK_SWITCH)(enum connsys_spi_speed_type type);
typedef int(*CONSYS_PLT_POWER_STATE)(void);
typedef int(*CONSYS_PLT_AIDE_TYPE_CHECK)(void);
typedef int(*CONSYS_PLT_AIDE_TYPE_CFG)(void);

struct consys_hw_ops_struct {
	/* HW init */
	CONSYS_PLT_HW_INIT consys_plt_hw_init;

	/* Power on/off CONNSYS PART (by Conn_infra Driver) */
	CONSYS_PLT_XTAL_CTRL_FAST_MODE consys_plt_xtal_ctrl_fast_mode;
	CONSYS_PLT_CONNSYS_SW_RESET_CTRL consys_plt_connsys_sw_reset_ctrl;
	CONSYS_PLT_SET_IF_PINMUX consys_plt_set_if_pinmux;
	CONSYS_PLT_TX_RX_BUS_SLP_PROT_CTRL consys_plt_tx_rx_bus_slp_prot_ctrl;
	CONSYS_PLT_POLLING_CONSYS_CHIPID consys_plt_polling_consys_chipid;
	CONSYS_PLT_BUS_CLOCK_CTRL consys_plt_bus_clock_ctrl;
	CONSYS_PLT_D_DIE_CFG consys_plt_d_die_cfg;
	CONSYS_PLT_CONNINFRA_SYSRAM_HW_CTRL consys_plt_conninfra_sysram_hw_ctrl;
	CONSYS_PLT_SPI_MASTER_CFG consys_plt_spi_master_cfg;
	CONSYS_PLT_A_DIE_CFG consys_plt_a_die_cfg;
	CONSYS_PLT_AFE_WBG_CAL consys_plt_afe_wbg_cal;
	CONSYS_PLT_SUBSYS_PLL_INITIAL consys_plt_subsys_pll_initial;
	CONSYS_PLT_OSC_LEGACY_MODE consys_plt_osc_legacy_mode;
	CONSYS_PLT_TOP_PWR_CTRL consys_plt_top_pwr_ctrl;
	CONSYS_PLT_CONN_INFRA_BUS_TIMEOUT consys_plt_conn_infra_bus_timeout;
	CONSYS_PLT_CLKGEN_WPLL_HW_CTRL consys_plt_clkgen_wpll_hw_ctrl;
	CONSYS_PLT_CONNINFRA_TOP_WAKEUP consys_plt_conninfra_wakeup;
	CONSYS_PLT_CONNINFRA_TOP_SLEEP consys_plt_conninfra_sleep;
	CONSYS_PLT_ADIE_TOP_CK_EN_ON_OFF_CTRL consys_plt_adie_top_ck_en_on_off_ctrl;
	CONSYS_PLT_WPLL_CTRL consys_plt_wpll_ctrl;

	/* Power on/off WFSYS PART 0 (by WF Driver) */
	CONSYS_PLT_CONNINFRA_WF_WAKEUP consys_plt_conninfra_wf_wakeup;
	CONSYS_PLT_CONNINFRA_WF_SLEEP consys_plt_conninfra_wf_sleep;
	CONSYS_PLT_CONN_WMCPU_SW_RESET consys_plt_conn_wmcpu_sw_reset;
	CONSYS_PLT_WF_BUS_SLP_PROT_CTRL consys_plt_wf_bus_slp_prot_ctrl;
	CONSYS_PLT_WFSYS_TOP_ON_CTRL consys_plt_wfsys_top_on_ctrl;
	CONSYS_PLT_WFSYS_BUS_SLP_PROT_CHECK consys_plt_wfsys_bus_slp_prot_check;
	CONSYS_PLT_WFSYS_BUS_TIMEOUT_CTRL consys_plt_wfsys_bus_timeout_ctrl;
	CONSYS_PLT_CONN_WMCPU_IDLE_LOOP_CHECK consys_plt_conn_wmcpu_idle_loop_check;
	CONSYS_PLT_CONNINFRA_WF_REQ_CLR consys_plt_conninfra_wf_req_clr;

	/* load from dts */
	CONSYS_PLT_CLK_GET_FROM_DTS consys_plt_clk_get_from_dts;
	CONSYS_PLT_CLK_DETACH consys_plt_clk_detach;

	/* clock */
	CONSYS_PLT_CO_CLOCK_TYPE consys_plt_co_clock_type;

	CONSYS_PLT_SOC_CHIPID_GET consys_plt_soc_chipid_get;

	/* debug */
	CONSYS_PLT_GET_HW_VER consys_plt_get_hw_ver;

	/* For SPI operation */
	CONSYS_PLT_SPI_READ consys_plt_spi_read;
	CONSYS_PLT_SPI_WRITE consys_plt_spi_write;

	/* For SPI clock switch */
	CONSYS_PLT_SPI_CLOCK_SWITCH consys_plt_spi_clock_switch;

	/* power state */
	CONSYS_PLT_POWER_STATE consys_plt_power_state;

	/* others */
	CONSYS_PLT_AIDE_TYPE_CHECK consys_plt_adie_type_check;
	CONSYS_PLT_AIDE_TYPE_CFG consys_plt_adie_type_cfg;
};

struct consys_hw_env {
	bool valid;
	unsigned int adie_hw_version;
	unsigned int adie_id;
	int is_rc_mode;
};

struct conninfra_plat_data {
	const unsigned int chip_id;
	const void* hw_ops;
	const void* reg_ops;
	const void* platform_emi_ops;
	const void* platform_pmic_ops;
};

extern struct consys_hw_env conn_hw_env[AIDE_NUM_MAX];
extern struct consys_base_addr conn_reg;
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
int mtk_conninfra_drv_init(struct conninfra_dev_cb *dev_cb);
int mtk_conninfra_drv_deinit(void);

int consys_hw_pwr_on(unsigned int curr_status, unsigned int on_radio);
int consys_hw_pwr_off(unsigned int curr_status, unsigned int off_radio);

int consys_hw_wifi_power_ctl(unsigned int enable);
int consys_hw_bt_power_ctl(unsigned int enable);
int consys_hw_gps_power_ctl(unsigned int enable);
int consys_hw_fm_power_ctl(unsigned int enable);
int consys_hw_pmic_event_cb(unsigned int id, unsigned int event);

unsigned int consys_hw_chipid_get(void);

int consys_hw_get_clock_schematic(void);
unsigned int consys_hw_get_hw_ver(void);

/*******************************************************************************
* tempoary for STEP
********************************************************************************
*/
/*
 * return
 * 1 : can read
 * 0 : can't read
 * -1: not consys register
 */
int consys_hw_reg_readable(void);
int consys_hw_is_connsys_reg(phys_addr_t addr);
/*
 * 0 means NO hang
 * > 0 means hang!!
 */
int consys_hw_is_bus_hang(void);
int consys_hw_dump_bus_status(void);

int consys_hw_spi_read(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int *data);
int consys_hw_spi_write(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int data);

int consys_hw_adie_top_ck_en_on(enum consys_drv_type type);
int consys_hw_adie_top_ck_en_off(enum consys_drv_type type);

/* NOTE: debug only*/
int consys_hw_force_conninfra_wakeup(void);
int consys_hw_force_conninfra_sleep(void);

int consys_hw_spi_clock_switch(enum connsys_spi_speed_type type);

struct platform_device *get_consys_device(void);
struct consys_base_addr *get_conn_reg_base_addr(void);

int consys_hw_dump_power_state(void);
/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif				/* _PLATFORM_CONSYS_HW_H_ */
