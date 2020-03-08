/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

 #ifndef _SSDK_DTS_H_
#define _SSDK_DTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BOARD_AR71XX
#include <linux/version.h>
#endif
#include "ssdk_init.h"
#include <linux/phy.h>

typedef struct {
	a_uint16_t ucastq_start;
	a_uint16_t ucastq_end;
	a_uint16_t mcastq_start;
	a_uint16_t mcastq_end;
	a_uint8_t l0sp_start;
	a_uint8_t l0sp_end;
	a_uint8_t l0cdrr_start;
	a_uint8_t l0cdrr_end;
	a_uint8_t l0edrr_start;
	a_uint8_t l0edrr_end;
	a_uint8_t l1cdrr_start;
	a_uint8_t l1cdrr_end;
	a_uint8_t l1edrr_start;
	a_uint8_t l1edrr_end;
} ssdk_dt_portscheduler_cfg;

typedef struct {
	a_uint8_t valid;
	a_uint8_t port_id;
	a_uint8_t cpri;
	a_uint8_t cdrr_id;
	a_uint8_t epri;
	a_uint8_t edrr_id;
	a_uint8_t sp_id;
} ssdk_dt_l0scheduler_cfg;

typedef struct {
	a_uint8_t valid;
	a_uint8_t port_id;
	a_uint8_t cpri;
	a_uint8_t cdrr_id;
	a_uint8_t epri;
	a_uint8_t edrr_id;
} ssdk_dt_l1scheduler_cfg;

typedef struct {
	ssdk_dt_portscheduler_cfg pool[SSDK_MAX_PORT_NUM];
	ssdk_dt_l0scheduler_cfg l0cfg[SSDK_L0SCHEDULER_CFG_MAX];
	ssdk_dt_l1scheduler_cfg l1cfg[SSDK_L1SCHEDULER_CFG_MAX];
} ssdk_dt_scheduler_cfg;

typedef struct
{
	a_uint8_t port_id;
	a_uint8_t phy_addr;
	phy_features_t phy_features;
	struct mii_bus *miibus;
} ssdk_port_phyinfo;

typedef struct
{
	a_uint32_t switchreg_base_addr;
	a_uint32_t switchreg_size;
	a_uint32_t psgmiireg_base_addr;
	a_uint32_t psgmiireg_size;
	a_uint8_t *reg_access_mode;
	a_uint8_t *psgmii_reg_access_str;
	hsl_reg_mode switch_reg_access_mode;
	hsl_reg_mode psgmii_reg_access_mode;
	struct clk *ess_clk;
	struct clk *cmnblk_clk;
	ssdk_port_cfg   port_cfg;
	a_uint32_t phyinfo_num;
	ssdk_port_phyinfo *port_phyinfo;
	a_uint32_t      mac_mode;
	a_uint32_t      mac_mode1;
	a_uint32_t      mac_mode2;
	a_uint32_t uniphyreg_base_addr;
	a_uint32_t uniphyreg_size;
	a_uint8_t *uniphy_access_mode;
	hsl_reg_mode uniphy_reg_access_mode;
	ssdk_dt_scheduler_cfg scheduler_cfg;
	a_uint8_t bm_tick_mode;
	a_uint8_t tm_tick_mode;
	a_bool_t ess_switch_flag;
	a_uint32_t device_id;
	struct device_node *of_node;
} ssdk_dt_cfg;

#define SSDK_MAX_NR_ETH 6

typedef struct
{
	a_uint32_t num_devices;
	ssdk_dt_cfg **ssdk_dt_switch_nodes;
	a_uint32_t num_intf_mac;
	fal_mac_addr_t intf_mac[SSDK_MAX_NR_ETH];
} ssdk_dt_global_t;

typedef struct
{
	a_uint32_t base_addr;
	a_uint32_t size;
} ssdk_reg_map_info;

/* DTS info for get */
a_uint8_t ssdk_tm_tick_mode_get(a_uint32_t dev_id);
a_uint8_t ssdk_bm_tick_mode_get(a_uint32_t dev_id);
a_uint16_t ssdk_ucast_queue_start_get(a_uint32_t dev_id, a_uint32_t port);
ssdk_dt_scheduler_cfg* ssdk_bootup_shceduler_cfg_get(a_uint32_t dev_id);
a_uint32_t ssdk_intf_mac_num_get(void);
a_uint8_t* ssdk_intf_macaddr_get(a_uint32_t index);
a_uint32_t ssdk_dt_global_get_mac_mode(a_uint32_t dev_id, a_uint32_t index);
a_uint32_t ssdk_dt_global_set_mac_mode(a_uint32_t dev_id, a_uint32_t index, a_uint32_t mode);
a_uint32_t ssdk_cpu_bmp_get(a_uint32_t dev_id);
a_uint32_t ssdk_lan_bmp_get(a_uint32_t dev_id);
a_uint32_t ssdk_wan_bmp_get(a_uint32_t dev_id);
ssdk_port_phyinfo* ssdk_port_phyinfo_get(a_uint32_t dev_id, a_uint32_t port_id);
struct mii_bus *
ssdk_dts_miibus_get(a_uint32_t dev_id, a_uint32_t phy_addr);
hsl_reg_mode ssdk_switch_reg_access_mode_get(a_uint32_t dev_id);
hsl_reg_mode ssdk_uniphy_reg_access_mode_get(a_uint32_t dev_id);
hsl_reg_mode ssdk_psgmii_reg_access_mode_get(a_uint32_t dev_id);
void ssdk_switch_reg_map_info_get(a_uint32_t dev_id, ssdk_reg_map_info *info);
void ssdk_psgmii_reg_map_info_get(a_uint32_t dev_id, ssdk_reg_map_info *info);
void ssdk_uniphy_reg_map_info_get(a_uint32_t dev_id, ssdk_reg_map_info *info);
a_bool_t ssdk_ess_switch_flag_get(a_uint32_t dev_id);
a_uint32_t ssdk_device_id_get(a_uint32_t index);
struct device_node *ssdk_dts_node_get(a_uint32_t dev_id);
struct clk *ssdk_dts_essclk_get(a_uint32_t dev_id);
struct clk *ssdk_dts_cmnclk_get(a_uint32_t dev_id);

int ssdk_switch_device_num_init(void);
void ssdk_switch_device_num_exit(void);
a_uint32_t ssdk_switch_device_num_get(void);
#ifndef BOARD_AR71XX
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
sw_error_t ssdk_dt_parse(ssdk_init_cfg *cfg, a_uint32_t num, a_uint32_t *dev_id);
#endif
#endif

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _SSDK_DTS_H */
