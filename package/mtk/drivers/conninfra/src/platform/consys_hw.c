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
#include <linux/of_device.h>
#include <linux/of_reserved_mem.h>

#include "osal.h"
#include "consys_hw.h"
#include "emi_mng.h"
#include "pmic_mng.h"
#include "consys_reg_mng.h"

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

static int mtk_conninfra_probe(struct platform_device *pdev);
static int mtk_conninfra_remove(struct platform_device *pdev);
static int mtk_conninfra_suspend(struct platform_device *pdev, pm_message_t state);
static int mtk_conninfra_resume(struct platform_device *pdev);

static int consys_hw_init(struct platform_device *pdev);
static int consys_hw_deinit(void);
static int _consys_hw_conninfra_wakeup(void);
static int _consys_hw_conninfra_sleep(void);

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

extern const struct of_device_id apconninfra_of_ids[];

static struct platform_driver mtk_conninfra_dev_drv = {
	.probe = mtk_conninfra_probe,
	.remove = mtk_conninfra_remove,
	.suspend = mtk_conninfra_suspend,
	.resume = mtk_conninfra_resume,
	.driver = {
		.name = "mtk_conninfra",
		.owner = THIS_MODULE,
		.of_match_table = apconninfra_of_ids,
	},
};


struct consys_hw_env conn_hw_env[AIDE_NUM_MAX];

const struct consys_hw_ops_struct *consys_hw_ops;
struct platform_device *g_pdev;

int g_conninfra_wakeup_ref_cnt;

struct work_struct ap_resume_work;

struct conninfra_dev_cb *g_conninfra_dev_cb;
const struct conninfra_plat_data *g_conninfra_plat_data = NULL;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
struct platform_device *get_consys_device(void)
{
	return g_pdev;
}

int consys_hw_get_clock_schematic(void)
{
	if (consys_hw_ops->consys_plt_co_clock_type)
		return consys_hw_ops->consys_plt_co_clock_type();
	else
		pr_err("consys_hw_ops->consys_co_clock_type not supported\n");

	return -1;
}

unsigned int consys_hw_chipid_get(void)
{
	if (g_conninfra_plat_data && g_conninfra_plat_data->chip_id)
		return g_conninfra_plat_data->chip_id;
	else if (consys_hw_ops->consys_plt_soc_chipid_get)
		return consys_hw_ops->consys_plt_soc_chipid_get();
	else
		pr_err("consys_plt_soc_chipid_get not supported\n");

	return 0;
}

unsigned int consys_hw_get_hw_ver(void)
{
	if (consys_hw_ops->consys_plt_get_hw_ver)
		return consys_hw_ops->consys_plt_get_hw_ver();
	return 0;
}


int consys_hw_reg_readable(void)
{
	return consys_reg_mng_reg_readable();
}

int consys_hw_is_connsys_reg(phys_addr_t addr)
{
	return consys_reg_mng_is_connsys_reg(addr);
}

int consys_hw_is_bus_hang(void)
{
	return consys_reg_mng_is_bus_hang();
}

int consys_hw_dump_bus_status(void)
{
	return consys_reg_mng_dump_bus_status();
}

int consys_hw_dump_cpupcr(enum conn_dump_cpupcr_type dump_type, int times, unsigned long interval_us)
{
	return consys_reg_mng_dump_cpupcr(dump_type, times, interval_us);
}

int consys_hw_pwr_on(unsigned int curr_status, unsigned int on_radio)
{
	//unsigned int next_status = (curr_status | (0x1 << on_radio));

	/* first power on */
	if (curr_status == 0) {
		/* POS PART 0:
		 * Set PMIC to turn on the power that AFE WBG circuit in D-die,
		 * OSC or crystal component, and A-die need.
		 */
		if (consys_hw_ops->consys_plt_xtal_ctrl_fast_mode)
			consys_hw_ops->consys_plt_xtal_ctrl_fast_mode();

		if (consys_hw_ops->consys_plt_connsys_sw_reset_ctrl)
			consys_hw_ops->consys_plt_connsys_sw_reset_ctrl(false);

		/* POS PART 1:
		 * 1. Pinmux setting
		 * 2. Turn on MTCMOS
		 * 3. Enable AHB bus
		 */
		if (consys_hw_ops->consys_plt_set_if_pinmux)
			consys_hw_ops->consys_plt_set_if_pinmux(true);

		udelay(500);

		if (consys_hw_ops->consys_plt_tx_rx_bus_slp_prot_ctrl)
			consys_hw_ops->consys_plt_tx_rx_bus_slp_prot_ctrl(true);

		if (consys_hw_ops->consys_plt_polling_consys_chipid)
			consys_hw_ops->consys_plt_polling_consys_chipid();

		/* POS PART 2:
		 * 1. Set connsys EMI mapping
		 * 2. d_die_cfg
		 * 3. spi_master_cfg
		 * 4. a_die_cfg
		 * 5. afe_wbg_cal
		 * 6. patch default value
		 * 7. CONN_INFRA low power setting (srcclken wait time, mtcmos HW ctl...)
		 */
		if (consys_hw_ops->consys_plt_bus_clock_ctrl)
			consys_hw_ops->consys_plt_bus_clock_ctrl(on_radio, CONNINFRA_BUS_CLOCK_ALL);

		emi_mng_set_remapping_reg();
		emi_mng_set_region_protection();

		if (consys_hw_ops->consys_plt_d_die_cfg)
			consys_hw_ops->consys_plt_d_die_cfg();

		if (consys_hw_ops->consys_plt_conninfra_sysram_hw_ctrl)
			consys_hw_ops->consys_plt_conninfra_sysram_hw_ctrl();

		if (consys_hw_ops->consys_plt_spi_master_cfg)
			consys_hw_ops->consys_plt_spi_master_cfg();

#ifndef CONFIG_FPGA_EARLY_PORTING
		if (consys_hw_ops->consys_plt_adie_type_check)
			consys_hw_ops->consys_plt_adie_type_check();

		if (consys_hw_ops->consys_plt_a_die_cfg)
			consys_hw_ops->consys_plt_a_die_cfg();
#endif

		if (consys_hw_ops->consys_plt_afe_wbg_cal)
			consys_hw_ops->consys_plt_afe_wbg_cal();

		if (consys_hw_ops->consys_plt_subsys_pll_initial)
			consys_hw_ops->consys_plt_subsys_pll_initial();

		if (consys_hw_ops->consys_plt_osc_legacy_mode)
			consys_hw_ops->consys_plt_osc_legacy_mode();

#ifndef CONFIG_FPGA_EARLY_PORTING
		if (consys_hw_ops->consys_plt_top_pwr_ctrl)
			consys_hw_ops->consys_plt_top_pwr_ctrl();
#endif

		if (consys_hw_ops->consys_plt_conn_infra_bus_timeout)
			consys_hw_ops->consys_plt_conn_infra_bus_timeout();

		if (consys_hw_ops->consys_plt_clkgen_wpll_hw_ctrl)
			consys_hw_ops->consys_plt_clkgen_wpll_hw_ctrl();

		/* POS PART 3:
		 * 1. A-die low power setting
		 * 2. bgfsys power on(BT/GPS on)
		 */
		consys_hw_force_conninfra_wakeup();
#ifndef CONFIG_FPGA_EARLY_PORTING
		consys_hw_adie_top_ck_en_on(on_radio);
		//consys_hw_adie_top_ck_en_off(on_radio);
#endif
		consys_hw_force_conninfra_sleep();
	}else {
		switch (on_radio) {
			case CONNDRV_TYPE_WIFI:
				/* Power on WFSYS PART 0:
				 * 1. wake up conn_infra
				 * 2. turn on MTCMOS power switch of "wfsys_top_on" and "wfsys_top_off" circuit in D-die ("wfsys_top_off" is turned on by "wfsys_top_on" automatically)
				 * 3. enable AHB bus(WF2conn/conn2WF)
				 * 4. downlad CONNSYS EMI code
				 * 5. patch default value
				 */
				if (consys_hw_ops->consys_plt_conninfra_wf_wakeup)
					consys_hw_ops->consys_plt_conninfra_wf_wakeup();

				if (consys_hw_ops->consys_plt_conn_wmcpu_sw_reset)
					consys_hw_ops->consys_plt_conn_wmcpu_sw_reset(true);

				if (consys_hw_ops->consys_plt_wf_bus_slp_prot_ctrl)
					consys_hw_ops->consys_plt_wf_bus_slp_prot_ctrl(false);

				if (consys_hw_ops->consys_plt_wfsys_top_on_ctrl)
					consys_hw_ops->consys_plt_wfsys_top_on_ctrl(true);

				if (consys_hw_ops->consys_plt_wfsys_bus_slp_prot_check)
					consys_hw_ops->consys_plt_wfsys_bus_slp_prot_check(true);

				if (consys_hw_ops->consys_plt_wfsys_bus_timeout_ctrl)
					consys_hw_ops->consys_plt_wfsys_bus_timeout_ctrl();

				if (consys_hw_ops->consys_plt_conn_wmcpu_sw_reset)
					consys_hw_ops->consys_plt_conn_wmcpu_sw_reset(false);

#ifndef CONFIG_FPGA_EARLY_PORTING
				if (consys_hw_ops->consys_plt_conn_wmcpu_idle_loop_check)
					consys_hw_ops->consys_plt_conn_wmcpu_idle_loop_check();

				if (consys_hw_ops->consys_plt_adie_type_cfg)
					consys_hw_ops->consys_plt_adie_type_cfg();
#endif
				/* No sleep requiremenct for rebb AP */
#if 0
				if (consys_hw_ops->consys_plt_conninfra_wf_sleep)
					consys_hw_ops->consys_plt_conninfra_wf_sleep();
#endif
				break;

			default:
				pr_err("Not support type now (on_radio = %d)\n", on_radio);
				break;
		}
	}

	return 0;
}

int consys_hw_pwr_off(unsigned int curr_status, unsigned int off_radio)
{
	//int ret = 0;
	unsigned int next_status = curr_status & ~(0x1 << off_radio);

	if (next_status == 0) {
		pr_info("Last pwoer off: %d\n", off_radio);

		/* Power off CONNSYS PART 0:
		 * 1. A-die low power setting
		 */
		consys_hw_force_conninfra_wakeup();
#ifndef CONFIG_FPGA_EARLY_PORTING
		//consys_hw_adie_top_ck_en_off(off_radio);
#endif
		consys_hw_force_conninfra_sleep();

		/* Power off CONNSYS PART 1:
		 * 1. disable AXI bus
		 * 2. turn off MTCMOS power switch of "conn_top_on" and "conn_top_off" circuit in D-die
		 */
		if (consys_hw_ops->consys_plt_tx_rx_bus_slp_prot_ctrl)
			consys_hw_ops->consys_plt_tx_rx_bus_slp_prot_ctrl(false);

		if (consys_hw_ops->consys_plt_connsys_sw_reset_ctrl)
			consys_hw_ops->consys_plt_connsys_sw_reset_ctrl(true);

		udelay(1);
	} else {
		switch (off_radio) {
			case CONNDRV_TYPE_WIFI:
				/* Power off WFSYS PART 1:
				 * 1. disable AXI bus(wf2conn/conn2wf)
				 * 2. turn off MTCMOS power switch of "wf_top_on" and "wf_top_off" circuit in D-die ("wf_top_off" is turned off by "wf_top_on" automatically)
				 */
				if (consys_hw_ops->consys_plt_conninfra_wf_wakeup)
					consys_hw_ops->consys_plt_conninfra_wf_wakeup();

				if (consys_hw_ops->consys_plt_wf_bus_slp_prot_ctrl)
					consys_hw_ops->consys_plt_wf_bus_slp_prot_ctrl(true);

				if (consys_hw_ops->consys_plt_wfsys_bus_slp_prot_check)
					consys_hw_ops->consys_plt_wfsys_bus_slp_prot_check(false);

				if (consys_hw_ops->consys_plt_wpll_ctrl)
					consys_hw_ops->consys_plt_wpll_ctrl(false);

				if (consys_hw_ops->consys_plt_wfsys_top_on_ctrl)
					consys_hw_ops->consys_plt_wfsys_top_on_ctrl(false);

				if (consys_hw_ops->consys_plt_wpll_ctrl)
					consys_hw_ops->consys_plt_wpll_ctrl(true);

				consys_hw_adie_top_ck_en_off(off_radio);

				if (consys_hw_ops->consys_plt_conninfra_wf_req_clr)
					consys_hw_ops->consys_plt_conninfra_wf_req_clr();

				if (consys_hw_ops->consys_plt_conninfra_wf_sleep)
					consys_hw_ops->consys_plt_conninfra_wf_sleep();
				break;

			default:
				consys_hw_force_conninfra_wakeup();
				consys_hw_adie_top_ck_en_off(off_radio);
				consys_hw_force_conninfra_sleep();
				break;
		}
	}

	return 0;
}

int consys_hw_wifi_power_ctl(unsigned int enable)
{
	return pmic_mng_wifi_power_ctrl(enable);
}

int consys_hw_bt_power_ctl(unsigned int enable)
{
	return pmic_mng_bt_power_ctrl(enable);
}

int consys_hw_gps_power_ctl(unsigned int enable)
{
	return pmic_mng_gps_power_ctrl(enable);
}

int consys_hw_fm_power_ctl(unsigned int enable)
{
	return pmic_mng_fm_power_ctrl(enable);
}

int consys_hw_dump_power_state(void)
{
	if (consys_hw_ops && consys_hw_ops->consys_plt_power_state)
		consys_hw_ops->consys_plt_power_state();
	return 0;
}

int consys_hw_spi_read(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int *data)
{
	if (consys_hw_ops->consys_plt_spi_read)
		return consys_hw_ops->consys_plt_spi_read(subsystem, addr, data);
	return -1;
}

int consys_hw_spi_write(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int data)
{
	if (consys_hw_ops->consys_plt_spi_write)
		return consys_hw_ops->consys_plt_spi_write(subsystem, addr, data);
	return -1;
}

int consys_hw_adie_top_ck_en_on(enum consys_drv_type type)
{
	if (consys_hw_ops->consys_plt_adie_top_ck_en_on_off_ctrl)
		return consys_hw_ops->consys_plt_adie_top_ck_en_on_off_ctrl(type, 1);
	return -1;
}

int consys_hw_adie_top_ck_en_off(enum consys_drv_type type)
{
	if (consys_hw_ops->consys_plt_adie_top_ck_en_on_off_ctrl)
		return consys_hw_ops->consys_plt_adie_top_ck_en_on_off_ctrl(type, 0);
	return -1;
}


static int _consys_hw_conninfra_wakeup(void)
{
	int ref = g_conninfra_wakeup_ref_cnt;
	bool wakeup = false, ret;

	if (consys_hw_ops->consys_plt_conninfra_wakeup) {
		if (g_conninfra_wakeup_ref_cnt == 0)  {
			ret = consys_hw_ops->consys_plt_conninfra_wakeup();
			if (ret) {
				pr_err("wakeup fail!! ret=[%d]\n", ret);
				return ret;
			}
			wakeup = true;
		}

		g_conninfra_wakeup_ref_cnt++;
	}

	pr_info("conninfra_wakeup refcnt=[%d]->[%d] %s\n",
			ref, g_conninfra_wakeup_ref_cnt, (wakeup ? "wakeup!!" : ""));

	return 0;
}

static int _consys_hw_conninfra_sleep(void)
{
	int ref = g_conninfra_wakeup_ref_cnt;
	bool sleep = false;

	if (consys_hw_ops->consys_plt_conninfra_sleep &&
		--g_conninfra_wakeup_ref_cnt == 0) {
		sleep = true;
		consys_hw_ops->consys_plt_conninfra_sleep();
	}

	if (g_conninfra_wakeup_ref_cnt < 0)
		g_conninfra_wakeup_ref_cnt = 0;
	
	pr_info("conninfra_sleep refcnt=[%d]->[%d] %s\n",
			ref, g_conninfra_wakeup_ref_cnt, (sleep ? "sleep!!" : ""));

	return 0;
}

int consys_hw_force_conninfra_wakeup(void)
{
	return _consys_hw_conninfra_wakeup();
}

int consys_hw_force_conninfra_sleep(void)
{
	return _consys_hw_conninfra_sleep();
}

int consys_hw_spi_clock_switch(enum connsys_spi_speed_type type)
{
	if (consys_hw_ops->consys_plt_spi_clock_switch)
		return consys_hw_ops->consys_plt_spi_clock_switch(type);
	return -1;
}

int consys_hw_pmic_event_cb(unsigned int id, unsigned int event)
{
	pmic_mng_event_cb(id, event);
	return 0;
}

int mtk_conninfra_probe(struct platform_device *pdev)
{
	int ret = -1;

	if (pdev)
		g_pdev = pdev;
	else {
		pr_err("pdev is NULL\n");
		return -1;
	}

	g_conninfra_plat_data = (const struct conninfra_plat_data*)of_device_get_match_data(&pdev->dev);
	if (g_conninfra_plat_data == NULL) {
		pr_err("Get platform data fail.\n");
		return -2;
	}

	if (consys_hw_ops == NULL)
		consys_hw_ops = (const struct consys_hw_ops_struct*)g_conninfra_plat_data->hw_ops;
	if (consys_hw_ops == NULL) {
		pr_err("Get HW op fail\n");
		return -3;
	}

	/* Read device node */
	if (consys_reg_mng_init(pdev, g_conninfra_plat_data) != 0) {
		pr_err("consys_plt_read_reg_from_dts fail\n");
		return -4;
	}

	if (consys_hw_ops->consys_plt_clk_get_from_dts) {
		if (consys_hw_ops->consys_plt_clk_get_from_dts(pdev) != 0) {
			pr_err("consys_plt_clk_get_from_dts fail\n");
			return -5;
		}
	}

	/* HW operation init */
	if (consys_hw_init(pdev) != 0) {
		pr_err("consys_hw_init fail\n");
		return -6;
	}

	/* emi mng init */
	ret = emi_mng_init(pdev, g_conninfra_plat_data);
	if (ret) {
		pr_err("emi_mng init fail, %d\n", ret);
		return -7;
	}

	ret = pmic_mng_init(pdev, g_conninfra_dev_cb, g_conninfra_plat_data);
	if (ret) {
		pr_err("pmic_mng init fail, %d\n", ret);
		return -8;
	}

	return ret;
}

int mtk_conninfra_remove(struct platform_device *pdev)
{
	int ret;

	ret = pmic_mng_deinit();
	pr_info("pmic_mng_deinit ret=%d\n", ret);

	ret = emi_mng_deinit();
	pr_info("emi_mng_deinit ret=%d\n", ret);

	if (consys_hw_ops->consys_plt_clk_detach)
		consys_hw_ops->consys_plt_clk_detach();
	else
		pr_err("consys_plt_clk_detach is null\n");

	ret = consys_reg_mng_deinit();
	pr_info("consys_reg_mng_deinit ret=%d\n", ret);

	ret = consys_hw_deinit();
	pr_info("consys_hw_deinit ret=%d\n", ret);

	if (g_pdev)
		g_pdev = NULL;

	return 0;
}

int mtk_conninfra_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

int mtk_conninfra_resume(struct platform_device *pdev)
{
	/* suspend callback is in atomic context, use schedule work to execute STEP */

	schedule_work(&ap_resume_work);
	return 0;
}

static void consys_hw_ap_resume_handler(struct work_struct *work)
{
	if (g_conninfra_dev_cb && g_conninfra_dev_cb->conninfra_resume_cb)
		(*g_conninfra_dev_cb->conninfra_resume_cb)();
}

int consys_hw_init(struct platform_device *pdev)
{
	int iRet = 0;

	if (consys_hw_ops->consys_plt_hw_init)
		iRet = consys_hw_ops->consys_plt_hw_init();

	return iRet;
}

int consys_hw_deinit(void)
{
	return 0;
}

int mtk_conninfra_drv_init(struct conninfra_dev_cb *dev_cb)
{
	int iRet = 0;

	g_conninfra_dev_cb = dev_cb;

	pr_info("Before platform_driver_register\n");

	iRet = platform_driver_register(&mtk_conninfra_dev_drv);
	if (iRet)
		pr_err("Conninfra platform driver registered failed(%d)\n", iRet);

	pr_info("After platform_driver_register\n");

	INIT_WORK(&ap_resume_work, consys_hw_ap_resume_handler);

	return iRet;
}

int mtk_conninfra_drv_deinit(void)
{
	platform_driver_unregister(&mtk_conninfra_dev_drv);
	g_conninfra_dev_cb = NULL;
	return 0;
}

