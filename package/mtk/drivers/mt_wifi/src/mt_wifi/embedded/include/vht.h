/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#ifndef __VHT_H__
#define __VHT_H__
#include "dot11ac_vht.h"

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _RT_PHY_INFO;
struct _build_ie_info;
struct _op_info;

enum vht_config_bw {
	VHT_BW_2040,
	VHT_BW_80,
	VHT_BW_160,
	VHT_BW_8080
};

struct vht_ch_layout {
	UCHAR ch_low_bnd;
	UCHAR ch_up_bnd;
	UCHAR cent_freq_idx;
};

enum vht_caps {
	VHT_RX_LDPC = 1,
	VHT_BW80_SGI = (1 << 1),
	VHT_BW160_8080_SGI = (1 << 2),
	VHT_TX_STBC = (1 << 3),
	VHT_SU_BFER = (1 << 4),
	VHT_SU_BFEE = (1 << 5),
	VHT_MU_BFER = (1 << 6),
	VHT_MU_BFEE = (1 << 7),
	VHT_TXOP_PS = (1 << 8),
	VHT_HTC = (1 << 9),
	VHT_RX_ANT_PATTERN_CONSIST = (1 << 10),
	VHT_TX_ANT_PATTERN_CONSIST = (1 << 11)
};

struct oper_mode {
	UINT8 ch_width;
	UINT8 bw160_8080;
	UINT8 no_ldpc;
	UINT8 rx_nss;
	UINT8 rx_nss_type; /*std: ap always set to 0*/
};


struct vht_ch_layout *get_ch_array(UINT8 bw, UCHAR ch_band);

VOID dump_vht_cap(struct _RTMP_ADAPTER *pAd, VHT_CAP_IE *vht_ie);
VOID dump_vht_op(struct _RTMP_ADAPTER *pAd, VHT_OP_IE *vht_ie);

INT build_vht_txpwr_envelope(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf);
INT build_vht_ies(struct _RTMP_ADAPTER *pAd, struct _build_ie_info *info);
INT build_vht_cap_ie(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf);
void update_vht_op_info(UINT8 cap_bw, struct vht_opinfo *vht_op_info, struct _op_info *op_info);

UCHAR vht_prim_ch_idx(UCHAR vht_cent_ch, UCHAR prim_ch, UINT8 rf_bw);
UCHAR vht_cent_ch_freq(UCHAR prim_ch, UCHAR vht_bw, UCHAR ch_band);
UCHAR vht_cent_ch_freq_40mhz(UCHAR prim_ch, UCHAR vht_bw, UCHAR ch_band);
INT vht_mode_adjust(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry,
	VHT_CAP_IE *cap, VHT_OP_IE *op, OPERATING_MODE *op_mode, UCHAR *bw_from_opclass);
INT dot11_vht_mcs_to_internal_mcs(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	VHT_CAP_IE *vht_cap,
	HTTRANSMIT_SETTING *tx);
VOID set_vht_cap(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *entry, VHT_CAP_IE *vht_cap_ie);
INT SetCommonVHT(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID rtmp_set_vht(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RT_PHY_INFO *phy_info);
char *VhtBw2Str(INT VhtBw);

#ifdef VHT_TXBF_SUPPORT
VOID trigger_vht_ndpa(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *entry);
#endif /* VHT_TXBF_SUPPORT */

VOID assoc_vht_info_debugshow(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry,
	VHT_CAP_IE *vht_cap,
	VHT_OP_IE *vht_op);

BOOLEAN vht40_channel_group(struct _RTMP_ADAPTER *pAd, UCHAR channel, struct wifi_dev *wdev);
BOOLEAN vht80_channel_group(struct _RTMP_ADAPTER *pAd, UCHAR channel, struct wifi_dev *wdev);
BOOLEAN vht160_channel_group(struct _RTMP_ADAPTER *pAd, UCHAR channel, struct wifi_dev *wdev);
void print_vht_op_info(struct vht_opinfo *vht_op);
UINT32 starec_vht_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature);
UCHAR rf_bw_2_vht_bw(UCHAR rf_bw);


#define IS_VHT_STA(_pMacEntry)	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_VHT)

#define IS_VHT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_VHT)

#endif /*__VHT_H__*/
