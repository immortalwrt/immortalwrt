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

#include "consys_hw.h"
#include "pmic_mng.h"
#include "osal.h"
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

const struct consys_platform_pmic_ops* consys_platform_pmic_ops = NULL;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

int pmic_mng_init(
	struct platform_device *pdev,
	struct conninfra_dev_cb* dev_cb,
	const struct conninfra_plat_data* plat_data)
{
	if (consys_platform_pmic_ops == NULL) {
		consys_platform_pmic_ops = (const struct consys_platform_pmic_ops*)plat_data->platform_pmic_ops;
	}

	if (consys_platform_pmic_ops && consys_platform_pmic_ops->consys_pmic_get_from_dts)
		consys_platform_pmic_ops->consys_pmic_get_from_dts(pdev, dev_cb);

	return 0;
}

int pmic_mng_deinit(void)
{
	return 0;
}

int pmic_mng_common_power_ctrl(unsigned int enable)
{
	int ret = 0;

	if (consys_platform_pmic_ops &&
		consys_platform_pmic_ops->consys_pmic_common_power_ctrl)
		ret = consys_platform_pmic_ops->consys_pmic_common_power_ctrl(enable);

	return ret;
}

int pmic_mng_wifi_power_ctrl(unsigned int enable)
{
	int ret = 0;
	if (consys_platform_pmic_ops &&
		consys_platform_pmic_ops->consys_pmic_wifi_power_ctrl)
		ret = consys_platform_pmic_ops->consys_pmic_wifi_power_ctrl(enable);
	return ret;

}

int pmic_mng_bt_power_ctrl(unsigned int enable)
{
	int ret = 0;
	if (consys_platform_pmic_ops &&
		consys_platform_pmic_ops->consys_pmic_bt_power_ctrl)
		ret = consys_platform_pmic_ops->consys_pmic_bt_power_ctrl(enable);
	return ret;
}

int pmic_mng_gps_power_ctrl(unsigned int enable)
{
	int ret = 0;
	if (consys_platform_pmic_ops &&
		consys_platform_pmic_ops->consys_pmic_gps_power_ctrl)
		ret = consys_platform_pmic_ops->consys_pmic_gps_power_ctrl(enable);
	return ret;
}

int pmic_mng_fm_power_ctrl(unsigned int enable)
{
	int ret = 0;
	if (consys_platform_pmic_ops &&
		consys_platform_pmic_ops->consys_pmic_fm_power_ctrl)
		ret = consys_platform_pmic_ops->consys_pmic_fm_power_ctrl(enable);
	return ret;
}


int pmic_mng_event_cb(unsigned int id, unsigned int event)
{
	if (consys_platform_pmic_ops &&
		consys_platform_pmic_ops->consys_pmic_event_notifier)
		consys_platform_pmic_ops->consys_pmic_event_notifier(id, event);
	return 0;
}
