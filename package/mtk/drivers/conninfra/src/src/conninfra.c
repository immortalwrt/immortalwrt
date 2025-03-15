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

#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/fb.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/ratelimit.h>
#include "conninfra.h"
#include "emi_mng.h"
#include "conninfra_core.h"
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

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

#define CONNINFRA_RST_RATE_LIMIT 0

#if CONNINFRA_RST_RATE_LIMIT
DEFINE_RATELIMIT_STATE(g_rs, HZ, 1);

#define DUMP_LOG() if (__ratelimit(&g_rs)) \
			pr_info("rst is ongoing\n")

#else
#define DUMP_LOG()
#endif

struct conninfra_rst_data {
	enum consys_drv_type drv;
	char *reason;
};

struct conninfra_rst_data rst_data;

void conninfra_get_emi_phy_addr(enum connsys_emi_type type, phys_addr_t* base, unsigned int *size)
{
	struct consys_emi_addr_info* addr_info = emi_mng_get_phy_addr();

	switch (type) {
		case CONNSYS_EMI_FW:
			if (base)
				*base = addr_info->emi_ap_phy_base;

			if (size)
				*size = addr_info->fw_emi_size;
			break;

		default:
			pr_err("Wrong EMI type: %d\n", type);
			if (base)
				*base = 0x0;

			if (size)
				*size = 0;
			break;
	}
}
EXPORT_SYMBOL(conninfra_get_emi_phy_addr);

int conninfra_pwr_on(enum consys_drv_type drv_type)
{
	pr_info("[%s] drv=[%d]\n", __func__, drv_type);

	if (conninfra_core_is_rst_locking()) {
		DUMP_LOG();
		return CONNINFRA_ERR_RST_ONGOING;
	}

	return conninfra_core_power_on(drv_type);
}
EXPORT_SYMBOL(conninfra_pwr_on);

int conninfra_pwr_off(enum consys_drv_type drv_type)
{
	if (conninfra_core_is_rst_locking()) {
		DUMP_LOG();
		return CONNINFRA_ERR_RST_ONGOING;
	}

	return conninfra_core_power_off(drv_type);
}
EXPORT_SYMBOL(conninfra_pwr_off);

int conninfra_is_bus_hang(void)
{
	if (conninfra_core_is_rst_locking()) {
		DUMP_LOG();
		return CONNINFRA_ERR_RST_ONGOING;
	}
	return conninfra_core_is_bus_hang();
}
EXPORT_SYMBOL(conninfra_is_bus_hang);

int conninfra_trigger_whole_chip_rst(enum consys_drv_type who, char *reason)
{
	/* use schedule worker to trigger ??? */
	/* so that function can be returned immediately */
	int r;

	r = conninfra_core_lock_rst();
	if (r >= CHIP_RST_START) {
		/* reset is ongoing */
		pr_warn("r=[%d] chip rst is ongoing\n", r);
		return 1;
	}
	pr_info("rst lock [%d] [%d] reason=%s\n", r, who, reason);

	conninfra_core_trg_chip_rst(who, reason);

	return 0;
}
EXPORT_SYMBOL(conninfra_trigger_whole_chip_rst);

int conninfra_sub_drv_ops_register(enum consys_drv_type type,
				struct sub_drv_ops_cb *cb)
{
	/* type validation */
	if (type < 0 || type >= CONNDRV_TYPE_MAX) {
		pr_err("incorrect drv type [%d]\n", type);
		return -EINVAL;
	}
	pr_info("----\n");
	conninfra_core_subsys_ops_reg(type, cb);
	return 0;
}
EXPORT_SYMBOL(conninfra_sub_drv_ops_register);

int conninfra_sub_drv_ops_unregister(enum consys_drv_type type)
{
	/* type validation */
	if (type < 0 || type >= CONNDRV_TYPE_MAX) {
		pr_err("[%s] incorrect drv type [%d]\n", __func__, type);
		return -EINVAL;
	}
	pr_info("----\n");
	conninfra_core_subsys_ops_unreg(type);
	return 0;
}
EXPORT_SYMBOL(conninfra_sub_drv_ops_unregister);

int conninfra_spi_read(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int *data)
{
	if (conninfra_core_is_rst_locking()) {
		DUMP_LOG();
		return CONNINFRA_ERR_RST_ONGOING;
	}
	if (subsystem >= SYS_SPI_MAX) {
		pr_err("wrong subsys %d\n", subsystem);
		return -EINVAL;
	}
	conninfra_core_spi_read(subsystem, addr, data);
	return 0;
}
EXPORT_SYMBOL(conninfra_spi_read);

int conninfra_spi_write(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int data)
{
	if (conninfra_core_is_rst_locking()) {
		DUMP_LOG();
		return CONNINFRA_ERR_RST_ONGOING;
	}

	if (subsystem >= SYS_SPI_MAX) {
		pr_err("wrong subsys %d\n", subsystem);
		return -EINVAL;
	}
	conninfra_core_spi_write(subsystem, addr, data);
	return 0;
}
EXPORT_SYMBOL(conninfra_spi_write);

int conninfra_adie_top_ck_en_on(enum consys_drv_type type)
{
	if (conninfra_core_is_rst_locking()) {
		DUMP_LOG();
		return CONNINFRA_ERR_RST_ONGOING;
	}

	return conninfra_core_adie_top_ck_en_on(type);
}
EXPORT_SYMBOL(conninfra_adie_top_ck_en_on);

int conninfra_adie_top_ck_en_off(enum consys_drv_type type)
{
	if (conninfra_core_is_rst_locking()) {
		DUMP_LOG();
		return CONNINFRA_ERR_RST_ONGOING;
	}

	return conninfra_core_adie_top_ck_en_off(type);
}
EXPORT_SYMBOL(conninfra_adie_top_ck_en_off);

int conninfra_spi_clock_switch(enum connsys_spi_speed_type type)
{
	return conninfra_core_spi_clock_switch(type);
}
EXPORT_SYMBOL(conninfra_spi_clock_switch);

int conninfra_debug_dump(void)
{
	return conninfra_core_debug_dump();

}
EXPORT_SYMBOL(conninfra_debug_dump);
