/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mt79xx-afe-clk.h  --  Mediatek 79xx afe clock ctrl definition
 *
 * Copyright (c) 2021 MediaTek Inc.
 * Author: Vic Wu <vic.wu@mediatek.com>
 */

#ifndef _MT79XX_AFE_CLK_H_
#define _MT79XX_AFE_CLK_H_

struct mtk_base_afe;

int mt79xx_init_clock(struct mtk_base_afe *afe);
int mt79xx_afe_enable_clock(struct mtk_base_afe *afe);
int mt79xx_afe_disable_clock(struct mtk_base_afe *afe);
#endif
