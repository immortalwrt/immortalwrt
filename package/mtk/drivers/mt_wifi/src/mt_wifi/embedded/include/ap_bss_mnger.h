/*
 ***************************************************************************
 * Mediatek Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2018, Mediatek Technology, Inc.
 *
 * All rights reserved. Mediatek's source code is an unpublished work and the
 * use of a copyright notice doeas not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Mediatek Technology, Inc. is obtained.
 ***************************************************************************
*/

#ifndef __AP_BSS_MNGER_H__
#define __AP_BSS_MNGER_H__
#ifdef CONFIG_6G_SUPPORT

#define MAX_NET_IF_CNT 64

struct sec_info {
	UINT32 auth_mode;
	UINT32 PairwiseCipher;
	UINT32 GroupCipher;
};

struct radio_info {
	UINT8 channel;
	UINT8 bssid[MAC_ADDR_LEN];
	UINT8 opclass;
	UINT16 phymode;
};

struct bss_info {
	UINT8 bandidx;
	BOOLEAN is_hide_ssid;
	BOOLEAN is_trans_bss;
	UINT8 is_multi_bss;
	UINT16 mbss_grp_idx;
	INT8 ssid[MAX_LEN_OF_SSID+1];
	UINT8 ssid_len;
};

struct reg_info {
	struct radio_info radioinfo;
	struct bss_info bssinfo;
	struct sec_info secinfo;
};

struct netdev_info {
	UINT8 ifname[IFNAMSIZ];
	UINT32 ifindex;
};

struct iob_info {
	UINT8 iob_dsc_type;	/* type (0:Disalbe, 1:Probe.Rsp, 2:FD) */
	UINT8 iob_dsc_interval;	/* interval(ms) */
	UINT8 iob_dsc_txmode;	/* mode (0:Non-HT, 1:Non-HT-Dup, 2:HE-SU)*/
	UINT8 iob_dsc_by_cfg;	/* force setting by config */
};

struct oob_info {
	UINT8 repting_rule_2g;	/* rule to neighbor 2G bss */
	UINT8 repting_rule_5g;	/* rule to neighbor 5G bss */
	UINT8 repting_rule_6g;	/* rule to neighbor 6G bss */
	UINT64 repting_bmap;	/* reporting netif index */
};

struct module_info {
	INT32 chip_id;
};

struct bmg_entry {
	UINT8 valid;
	struct module_info modinfo;
	struct netdev_info devinfo;
	struct reg_info reginfo;
	struct iob_info iobinfo;
	struct oob_info oobinfo;
	DL_LIST list;
};

struct bss_mnger {
	BOOLEAN inited;
	UINT8 dev_cnt;
	/*
	 * kernel_band_bitmap mapping
	 * 1: Self module interface
	 * 0: Other module interface
	 * b0: 2G interface
	 * b1: 5G interface
	 * b2: 6G interface
	 */
	UINT8 kernel_band_bitmap;
	NDIS_SPIN_LOCK lock;
	DL_LIST entry_list;
};

struct bmg_entry *get_bmg_entry_by_ifname(char *ifname);
struct wifi_dev *get_6G_wdev_by_bssmnger(void);
struct wifi_dev *get_5G_wdev_by_bssmnger(void);
struct wifi_dev *get_2G_wdev_by_bssmnger(void);
NDIS_STATUS bssmnger_update_discovery_rule(struct wifi_dev *wdev);
NDIS_STATUS bssmnger_update_radio_info(struct wifi_dev *wdev);
int bssmnger_is_entry_exist_2g_5g(void);
NDIS_STATUS bssmnger_reg_bmg_entry(struct wifi_dev *wdev);
NDIS_STATUS bssmnger_dereg_bmg_entry(struct wifi_dev *wdev);
NDIS_STATUS bssmnger_init(void);
NDIS_STATUS bssmnger_deinit(void);
void bssmnger_show_bsslist_info(void);
NDIS_STATUS ap_6g_build_discovery_frame(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

#ifdef BSSMGR_CROSS_MODULE_SUPPORT
enum BSSMNGER_MSG_TYPE {
	BSSMNGER_MSG_OOB_DISCOVERY = 0,
	BSSMNGER_MSG_IOB_DISCOVERY,
	BSSMNGER_MSG_QOS_INJECTOR,
	BSSMNGER_MSG_REG_BMGENTRY,
	BSSMNGER_MSG_DEREG_BMGENTRY,
	BSSMNGER_MSG_UPDATE_RADIO,
};

struct bssmnger_msg {
	UINT32 ifindex;
	UINT8 datalen;
	UINT8 type;
	struct bmg_entry MsgBody;
};

NDIS_STATUS bssmnger_set_event_handle(
	PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR *buf);
NDIS_STATUS bssmnger_get_event_handle(
	PRTMP_ADAPTER pAd, UCHAR **buf, UCHAR *ifname);
NDIS_STATUS indicate_bssentry_to_wappd(
	struct wifi_dev *wdev, enum BSSMNGER_MSG_TYPE type);
#endif /* BSSMGR_CROSS_MODULE_SUPPORT */

#endif /* CONFIG_6G_SUPPORT */
#endif /* __AP_BSS_MNGER_H__ */
