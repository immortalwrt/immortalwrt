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
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Mediatek Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_bss_mgmt.h

    Abstract:
    BSS entry management

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __AP_BSS_MGMT_H__
#define __AP_BSS_MGMT_H__

#ifdef CONFIG_6G_SUPPORT

#define MAX_NET_IF_CNT			64

#define MAX_6G_SELF_RNR_IE_LEN	255
#define MAX_6G_NHBR_RNR_IE_LEN	1024

#define BSS_MGMT_SET_RSP_HANDLER(entry, callback) \
	((entry)->bss_mgmt_get_info_rsp = callback)

/* JEDI ---(set)---> BSS_MGMT */
typedef enum {
	BMG_SET_INIT 			= 0,	/* init global variable */
	BMG_SET_DEINIT 			= 1,	/* de-init global variable */
	BMG_SET_DEV_REG 		= 2,	/* register net device */
	BMG_SET_DEV_DEREG		= 3,	/* de-register net device */
	BMG_SET_DISCOV_RULE		= 4,	/* Iob/OoB discovery rules */

	BMG_GET_2G_5G_EXIST 	= 100,	/* 2G/5G (non-6g) bss existed */

	BMG_SET_SHOW_INFO		= 255,	/* dump stat message */
} BMG_SET_ID;

typedef struct _BMG_DEV_REG_INFO {
	USHORT		phymode;
	USHORT		mbss_grp_idx;

	UCHAR		is_trans_bss;	/* Transmitted-BSSID */
	UCHAR		is_multi_bss;	/* Multiple-BSSID enabled */
	UCHAR		channel;
	UCHAR		op_class;

	CHAR		ssid[MAX_LEN_OF_SSID+1];
	UCHAR		ssid_len;
	BOOLEAN		is_hide_ssid;

	UCHAR		bssid[MAC_ADDR_LEN];

	UINT32 		auth_mode;
	UINT32 		PairwiseCipher;
	UINT32 		GroupCipher;

	/* response handler */
	NDIS_STATUS (*bss_mgmt_get_info_rsp)(struct wifi_dev *wdev, UCHAR *rsp, UINT16 rsp_len);
} BMG_DEV_REG_INFO, *PBMG_DEV_REG_INFO;

typedef struct _bmg_discov_rule {
	UCHAR		iob_type;
	UCHAR		iob_interval;
	UCHAR		iob_txmode;
	UCHAR		iob_by_cfg;
	UCHAR		oob_repting_2g;
	UCHAR		oob_repting_5g;
	UCHAR		oob_repting_6g;
} bmg_discov_rule, *pbmg_discov_rule;

typedef union _bss_mgmt_set_info_data {
	BMG_DEV_REG_INFO 	dev_reg_info;
	bmg_discov_rule		discov_rule;
	UINT32 				data;
} bss_mgmt_set_info_data;

typedef struct _BMG_SET_PARAM {
	UCHAR					cmd_id;
	UCHAR					rsvd;
	USHORT					len;	/* input data length to BSS manager */
	bss_mgmt_set_info_data	data;
} BMG_SET_PARAM, *PBMG_SET_PARAM;
/* END */

/* JEDI <---(get)--- BSS_MGMT */
typedef enum {
	BMG_RESP_REPTED_BSS_INFO	= 1,
	BMG_RESP_IOB_TYPE_CHG		= 2,
} BMG_GET_ID;

typedef struct _bmg_repted_bss {
	UCHAR		repted_bss_cnt;
	UCHAR		rsvd[3];
	UCHAR		repted_bss_list[MAX_6G_NHBR_RNR_IE_LEN];
} bmg_repted_bss, *pbmg_repted_bss;

typedef union _bss_mgmt_rsp_info_data {
	UINT32				value;
	bmg_repted_bss		repted_bss;
} bss_mgmt_rsp_info_data;

typedef struct _bmg_rsp_param {
	UCHAR					rsp_id;
	UCHAR					rsvd;
	USHORT					len;	/* output data length from BSS manager */
	bss_mgmt_rsp_info_data	data;
} bmg_rsp_param, *pbmg_rsp_param;
/* END */

/* info per BSS entry */
typedef struct _BMG_ENTRY {
	DL_LIST				List;
	PNET_DEV			pNetDev;
	UCHAR				valid;

	/* BSS info */
	BMG_DEV_REG_INFO	entry_info;

	/* Iob/OoB discovery config */
	UCHAR				iob_dsc_type;		/* type (0:Disalbe, 1:Probe.Rsp, 2:FD) */
	UCHAR				iob_dsc_interval;	/* interval(ms) */
	UCHAR				iob_dsc_txmode;		/* mode (0:Non-HT, 1:Non-HT-Dup, 2:HE-SU)*/
	UCHAR				iob_dsc_by_cfg;		/* force setting by config */

	UINT64				repting_bmap;		/* reporting netif index */
	UCHAR				repting_rule_2g;	/* rule to neighbor 2G bss */
	UCHAR				repting_rule_5g;	/* rule to neighbor 5G bss */
	UCHAR				repting_rule_6g;	/* rule to neighbor 6G bss */
} BMG_ENTRY, *PBMG_ENTRY;

/* global manager for BSSs */
typedef struct _BSS_MGMT {
	UCHAR			inited;
	UCHAR			dev_cnt;
	NDIS_SPIN_LOCK	lock;
	DL_LIST			entry_list;
} BSS_MGMT, *PBSS_MGMT;
struct wifi_dev *get_6G_wdev_by_bss_mgmt(void);
struct wifi_dev *get_5G_wdev_by_bss_mgmt(void);
struct wifi_dev *get_2G_wdev_by_bss_mgmt(void);
NDIS_STATUS bss_mgmt_init(VOID);
NDIS_STATUS bss_mgmt_deinit(VOID);
NDIS_STATUS bss_mgmt_show_info(VOID);
NDIS_STATUS bss_mgmt_dev_reg(IN struct wifi_dev *wdev);
NDIS_STATUS bss_mgmt_dev_dereg(IN struct wifi_dev *wdev);
NDIS_STATUS bss_mgmt_set_discovery_rules(IN struct wifi_dev *wdev);

/* get info from bss manager */
NDIS_STATUS bss_mgmt_get_info(
	IN struct wifi_dev	*wdev,
	IN UCHAR			cmd_id
);

NDIS_STATUS bss_mgmt_get_info_rsp(
	IN struct wifi_dev		*wdev,
	IN UCHAR				*rsp,
	IN UINT16				rsp_len);

NDIS_STATUS ap_6g_build_discovery_frame(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

bool bss_mgmt_is_2g_5g_exist(VOID);

#endif /* CONFIG_6G_SUPPORT */
#endif /*__AP_BMG_H__*/
