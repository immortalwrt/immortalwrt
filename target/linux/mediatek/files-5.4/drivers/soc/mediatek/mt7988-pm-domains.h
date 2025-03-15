/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021 MediaTek Inc.
 *
 */

#ifndef __SOC_MEDIATEK_MT7988_PM_DOMAINS_H
#define __SOC_MEDIATEK_MT7988_PM_DOMAINS_H

#include "mtk-pm-domains.h"
//#include "mt7988-power.h"
#include <dt-bindings/power/mt7988-power.h>

/*
 * MT8139 power domain support
 */

static const struct scpsys_domain_data scpsys_domain_data_mt7988[] = {
	[MT7988_POWER_DOMAIN_TOPS0] = {
		.sta_mask = BIT(30),
		.sta_2nd_mask = BIT(31),
		.pwr_sta_offs = 0x040,
		.pwr_sta_2nd_offs = 0x040,
		.pwr_on_bit = BIT(1),
		.pwr_on_2nd_bit = BIT(2),
		.pwr_on_offs  = 0x040,
		.pwr_on_2nd_offs = 0x040,
		.pwr_clamp_bit = BIT(4),
		.pwr_rst_bit = BIT(0),
		.sram_pdn_bit = BIT(2),
		.sram_pdn_ack_bit = BIT(28),
		.sram_clk_iso_bit = BIT(0),
		.sram_ctrl_offs = 0x048,
		.sram_2nd_pdn_bit = BIT(8),
		.sram_2nd_pdn_ack_bit = BIT(24),
		.sram_2nd_clk_iso_bit = BIT(5),
		.sram_2nd_clk_dis_bit = BIT(3),
		.sram_2nd_ctrl_offs = 0x040,
		.caps = MTK_SCPD_KEEP_DEFAULT_OFF,
	},
	[MT7988_POWER_DOMAIN_TOPS1] = {
		.sta_mask = BIT(30),
		.sta_2nd_mask = BIT(31),
		.pwr_sta_offs = 0x044,
		.pwr_sta_2nd_offs = 0x044,
		.pwr_on_bit = BIT(1),
		.pwr_on_2nd_bit = BIT(2),
		.pwr_on_offs  = 0x044,
		.pwr_on_2nd_offs = 0x044,
		.pwr_clamp_bit = BIT(4),
		.pwr_rst_bit = BIT(0),
		.sram_pdn_bit = BIT(6),
		.sram_pdn_ack_bit = BIT(30),
		.sram_clk_iso_bit = BIT(4),
		.sram_ctrl_offs = 0x048,
		.sram_2nd_pdn_bit = BIT(8),
		.sram_2nd_pdn_ack_bit = BIT(24),
		.sram_2nd_clk_iso_bit = BIT(5),
		.sram_2nd_clk_dis_bit = BIT(3),
		.sram_2nd_ctrl_offs = 0x044,
		.caps = MTK_SCPD_KEEP_DEFAULT_OFF,

	},
	[MT7988_POWER_DOMAIN_ETH2P5] = {
		.sta_mask = BIT(30),
		.sta_2nd_mask = BIT(31),
		.pwr_sta_offs = 0x060,
		.pwr_sta_2nd_offs = 0x060,
		.pwr_on_bit = BIT(1),
		.pwr_on_2nd_bit = BIT(2),
		.pwr_on_offs  = 0x060,
		.pwr_on_2nd_offs = 0x060,
		.pwr_clamp_bit = BIT(4),
	    .pwr_rst_bit = BIT(0),
		.sram_2nd_pdn_bit = BIT(8),
		.sram_2nd_clk_dis_bit = BIT(5),
		.sram_2nd_ctrl_offs = 0x060,
		.caps = MTK_SCPD_CLAMP_PROTECTION,

	},

};

static const struct scpsys_soc_data mt7988_scpsys_data = {
	.domains_data = scpsys_domain_data_mt7988,
	.num_domains = ARRAY_SIZE(scpsys_domain_data_mt7988),
};

#endif /* __SOC_MEDIATEK_MT7988_PM_DOMAINS_H */
