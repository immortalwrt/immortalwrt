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

#include "consys_hw.h"
#include "consys_reg_mng.h"
#include "consys_reg_util.h"

const struct consys_reg_mng_ops* g_consys_reg_ops = NULL;

int consys_reg_mng_reg_readable(void)
{
	if (g_consys_reg_ops &&
		g_consys_reg_ops->consys_reg_mng_check_reable)
		return g_consys_reg_ops->consys_reg_mng_check_reable();

	return -1;
}

int consys_reg_mng_is_connsys_reg(phys_addr_t addr)
{
	if (g_consys_reg_ops &&
		g_consys_reg_ops->consys_reg_mng_is_consys_reg)
		return g_consys_reg_ops->consys_reg_mng_is_consys_reg(addr);

	return -1;
}


int consys_reg_mng_is_bus_hang(void)
{
	if (g_consys_reg_ops &&
		g_consys_reg_ops->consys_reg_mng_is_bus_hang)
		return g_consys_reg_ops->consys_reg_mng_is_bus_hang();

	return -1;
}

int consys_reg_mng_dump_bus_status(void)
{
	if (g_consys_reg_ops &&
		g_consys_reg_ops->consys_reg_mng_dump_bus_status)
		return g_consys_reg_ops->consys_reg_mng_dump_bus_status();

	return -1;
}

int consys_reg_mng_dump_conninfra_status(void)
{
	if (g_consys_reg_ops &&
		g_consys_reg_ops->consys_reg_mng_dump_conninfra_status)
		return g_consys_reg_ops->consys_reg_mng_dump_conninfra_status();

	return -1;
}

int consys_reg_mng_dump_cpupcr(enum conn_dump_cpupcr_type dump_type, int times, unsigned long interval_us)
{
	if (g_consys_reg_ops &&
		g_consys_reg_ops->consys_reg_mng_dump_cpupcr)
		return g_consys_reg_ops->consys_reg_mng_dump_cpupcr(dump_type, times, interval_us);

	return -1;
}

int consys_reg_mng_init(struct platform_device *pdev, const struct conninfra_plat_data* plat_data)
{
	int ret = 0;
	if (g_consys_reg_ops == NULL)
		g_consys_reg_ops = (const struct consys_reg_mng_ops*)plat_data->reg_ops;

	if (g_consys_reg_ops &&
		g_consys_reg_ops->consys_reg_mng_init)
		ret = g_consys_reg_ops->consys_reg_mng_init(pdev);
	else
		ret = EFAULT;

	return ret;
}

int consys_reg_mng_deinit(void)
{
	if (g_consys_reg_ops&&
		g_consys_reg_ops->consys_reg_mng_deinit)
		g_consys_reg_ops->consys_reg_mng_deinit();

	return 0;
}

int consys_reg_mng_reg_read(unsigned long addr, unsigned int *value, unsigned int mask)
{
	void __iomem *vir_addr = NULL;

	vir_addr = ioremap_nocache(addr, 0x100);
	if (!vir_addr) {
		pr_err("ioremap fail\n");
		return -1;
	}

	*value = (unsigned int)CONSYS_REG_READ(vir_addr) & mask;

	pr_info("[%x] mask=[%x]\n", *value, mask);

	iounmap(vir_addr);
	return 0;
}

int consys_reg_mng_reg_write(unsigned long addr, unsigned int value, unsigned int mask)
{
	void __iomem *vir_addr = NULL;

	vir_addr = ioremap_nocache(addr, 0x100);
	if (!vir_addr) {
		pr_err("ioremap fail\n");
		return -1;
	}

	CONSYS_REG_WRITE_MASK(vir_addr, value, mask);

	iounmap(vir_addr);
	return 0;
}


int consys_reg_mng_is_host_csr(unsigned long addr)
{
	if (g_consys_reg_ops &&
		g_consys_reg_ops->consys_reg_mng_is_host_csr)
		return g_consys_reg_ops->consys_reg_mng_is_host_csr(addr);

	return -1;
}
