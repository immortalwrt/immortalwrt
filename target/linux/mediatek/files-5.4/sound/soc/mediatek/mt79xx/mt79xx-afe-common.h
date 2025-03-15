/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mt79xx-afe-common.h  --  Mediatek 79xx audio driver definitions
 *
 * Copyright (c) 2021 MediaTek Inc.
 * Author: Vic Wu <vic.wu@mediatek.com>
 */

#ifndef _MT_79XX_AFE_COMMON_H_
#define _MT_79XX_AFE_COMMON_H_

#include <sound/soc.h>
#include <linux/list.h>
#include <linux/regmap.h>
#include "../common/mtk-base-afe.h"

enum {
	MT79XX_MEMIF_DL1,
	MT79XX_MEMIF_VUL12,
	MT79XX_MEMIF_NUM,
	MT79XX_DAI_ETDM = MT79XX_MEMIF_NUM,
	MT79XX_DAI_NUM,
};

enum {
	MT79XX_IRQ_0,
	MT79XX_IRQ_1,
	MT79XX_IRQ_2,
	MT79XX_IRQ_NUM,
};

struct clk;

struct mt79xx_afe_private {
	struct clk **clk;

	int pm_runtime_bypass_reg_ctl;

	/* dai */
	void *dai_priv[MT79XX_DAI_NUM];
};

unsigned int mt79xx_afe_rate_transform(struct device *dev,
				       unsigned int rate);

/* dai register */
int mt79xx_dai_etdm_register(struct mtk_base_afe *afe);
#endif
