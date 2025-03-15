// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include <linux/of_device.h>

#include "consys_hw.h"

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/* Platform data */
#ifdef CONNINFRA_APSOC_MT7986
extern struct conninfra_plat_data mt7986_plat_data;
const struct of_device_id apconninfra_of_ids[] = {
	{
		.compatible = "mediatek,mt7986-consys",
		.data = (void*)&mt7986_plat_data,
	},
	{}
};
#endif

#ifdef CONNINFRA_APSOC_MT7981
extern struct conninfra_plat_data mt7981_plat_data;
const struct of_device_id apconninfra_of_ids[] = {
	{
		.compatible = "mediatek,mt7981-consys",
		.data = (void*)&mt7981_plat_data,
	},
	{}
};
#endif
