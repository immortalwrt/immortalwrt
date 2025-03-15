/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __SOC_MEDIATEK_MTK_PM_DOMAINS_H
#define __SOC_MEDIATEK_MTK_PM_DOMAINS_H

#define MTK_SCPD_ACTIVE_WAKEUP		BIT(0)
#define MTK_SCPD_FWAIT_SRAM		BIT(1)
#define MTK_SCPD_SRAM_ISO		BIT(2)
#define MTK_SCPD_KEEP_DEFAULT_OFF	BIT(3)
#define MTK_SCPD_DOMAIN_SUPPLY		BIT(4)
#define MTK_SCPD_CLAMP_PROTECTION	BIT(5)
#define MTK_SCPD_CAPS(_scpd, _x)	((_scpd)->data->caps & (_x))

/**
 * struct scpsys_domain_data - scp domain data for power on/off flow
 * @sta_mask: The mask for power on/off status bit.
 * @sta_2nd_mask: The mask for 2nd power on/off status bit.
 * @pwr_sta_offs: the main power status register.
 * @pwr_sta_2nd_offs: the 2nd power status register.
 * @pwr_on_bit: The power on/off bit.
 * @pwr_on_2nd_bit: The 2nd power on/off bit.
 * @pwr_on_offs: The offset for main power control register.
 * @pwr_on_2nd_offs: The offset for 2nd power control register.
 * @sram_pdn_bit: The mask for sram power control bit.
 * @sram_pdn_ack_bit: The sram power control acked bit.
 * @sram_clk_iso_bit: The sram  clk iso bit.
 * @sram_clk_dis_bit: The sram clk disable bit.
 * @sram_ctrl_offs: The sram power control register.
 * @caps: The flag for active wake-up action.
 * @bp_infracfg: bus protection for infracfg subsystem
 */
struct scpsys_domain_data {
	u32 sta_mask;
	u32 sta_2nd_mask;
	int pwr_sta_offs;
	int pwr_sta_2nd_offs;
	u32 pwr_on_bit;
	u32 pwr_on_2nd_bit;
	int pwr_on_offs;
	int pwr_on_2nd_offs;
	u32 pwr_clamp_bit;
	u32 pwr_rst_bit;
	u32 sram_pdn_bit;
	u32 sram_pdn_ack_bit;
	u32 sram_clk_iso_bit;
	u32 sram_clk_dis_bit;
	int sram_ctrl_offs;
	u32 sram_2nd_pdn_bit;
	u32 sram_2nd_pdn_ack_bit;
	u32 sram_2nd_clk_iso_bit;
	u32 sram_2nd_clk_dis_bit;
	int sram_2nd_ctrl_offs;
	u8 caps;

};

struct scpsys_soc_data {
	const struct scpsys_domain_data *domains_data;
	int num_domains;
};

#endif /* __SOC_MEDIATEK_MTK_PM_DOMAINS_H */
