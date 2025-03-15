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

#ifndef _PLATFORM_MT7981_POS_H_
#define _PLATFORM_MT7981_POS_H_

int consys_plt_hw_init(void);
int consys_xtal_ctrl_fast_mode(void);
int consys_sw_reset_ctrl(bool bassert);
int consys_tx_rx_bus_slp_prot_ctrl(bool enable);
void consys_set_if_pinmux(bool enable);
int consys_polling_chipid(void);
int consys_plt_adie_type_cfg(void);
int consys_bus_clock_ctrl(enum consys_drv_type drv_type, unsigned int bus_clock);
int consys_emi_set_remapping_reg(void);
int consys_emi_set_region_protection(void);
int connsys_d_die_cfg(void);
int connsys_conninfra_sysram_hw_ctrl(void);
int connsys_spi_master_cfg(void);
int consys_sema_acquire_timeout(unsigned int index, unsigned int usec);
void consys_sema_release(unsigned int index);
int consys_spi_read(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int *data);
int consys_spi_write(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int data);
int consys_spi_write_offset_range(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int value,
												unsigned int reg_offset, unsigned int value_offset, unsigned int size);
int connsys_a_die_cfg(void);
int connsys_afe_wbg_cal(void);
int connsys_subsys_pll_initial(void);
int connsys_osc_legacy_mode(void);
int connsys_top_pwr_ctrl(void);
int connsys_conn_infra_bus_timeout(void);
int connsys_clkgen_wpll_hw_ctrl(void);
int consys_conninfra_top_wakeup(void);
int consys_conninfra_top_sleep(void);
int consys_adie_top_ck_en_on_off_ctrl(enum consys_drv_type type, unsigned char on);
int consys_conninfra_wf_wakeup(void);
int consys_conninfra_wf_sleep(void);
int consys_conn_wmcpu_sw_reset(bool bassert);
int consys_wf_bus_slp_prot_ctrl(bool enable);
int consys_wfsys_top_on_ctrl(bool enable);
int consys_wfsys_bus_slp_prot_check(bool enable);
int consys_wfsys_bus_timeout_ctrl(void);
int consys_wmcpu_idle_loop_check(void);
int consys_wpll_ctrl(bool enable);
int consys_conninfra_wf_req_clr(void);


#endif	/* _PLATFORM_MT7981_POS_H_ */
