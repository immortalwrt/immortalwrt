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

*/

#ifndef __HT_H__
#define __HT_H__
#include "dot11n_ht.h"

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _ADD_HT_INFO_IE;
struct _build_ie_info;

enum ht_mcs {
	HT_MCS_0,
	HT_MCS_1,
	HT_MCS_2,
	HT_MCS_3,
	HT_MCS_4,
	HT_MCS_5,
	HT_MCS_6,
	HT_MCS_7,
	HT_MCS_8,
	HT_MCS_9,
	HT_MCS_10,
	HT_MCS_11,
	HT_MCS_12,
	HT_MCS_13,
	HT_MCS_14,
	HT_MCS_15,
	HT_MCS_16,
	HT_MCS_17,
	HT_MCS_18,
	HT_MCS_19,
	HT_MCS_20,
	HT_MCS_21,
	HT_MCS_22,
	HT_MCS_23,
	HT_MCS_24,
	HT_MCS_25,
	HT_MCS_26,
	HT_MCS_27,
	HT_MCS_28,
	HT_MCS_29,
	HT_MCS_30,
	HT_MCS_31,
	HT_MCS_32,
	HT_MCS_33,
	HT_MCS_34,
	HT_MCS_35,
	HT_MCS_36,
	HT_MCS_37,
	HT_MCS_38,
	HT_MCS_39,
	HT_MCS_40,
	HT_MCS_41,
	HT_MCS_42,
	HT_MCS_43,
	HT_MCS_44,
	HT_MCS_45,
	HT_MCS_46,
	HT_MCS_47,
	HT_MCS_48,
	HT_MCS_49,
	HT_MCS_50,
	HT_MCS_52,
	HT_MCS_53,
	HT_MCS_54,
	HT_MCS_55,
	HT_MCS_56,
	HT_MCS_57,
	HT_MCS_58,
	HT_MCS_59,
	HT_MCS_60,
	HT_MCS_61,
	HT_MCS_62,
	HT_MCS_63,
	HT_MCS_64,
	HT_MCS_65,
	HT_MCS_66,
	HT_MCS_67,
	HT_MCS_68,
	HT_MCS_69,
	HT_MCS_70,
	HT_MCS_71,
	HT_MCS_72,
	HT_MCS_73,
	HT_MCS_74,
	HT_MCS_75,
	HT_MCS_76
};

enum ht_bw_def {
	HT_BW_20,
	HT_BW_40
};

enum ht_caps {
	HT_LDPC = 1,
	HT_CH_WIDTH_40 = (1 << 1),
	HT_GREEN_FIELD = (1 << 2),
	HT_BW20_SGI = (1 << 3),
	HT_BW40_SGI = (1 << 4),
	HT_TX_STBC = (1 << 5),
	HT_DELAY_BA = (1 << 6),
	HT_BW40_DSSS_CCK = (1 << 7),
	HT_BW40_INTOLERANT = (1 << 8),
	HT_LSIG_TXOP = (1 << 9),
	HT_EXT_PCO = (1 << 10),
	HT_EXT_HTC = (1 << 11),
	HT_EXT_RD_RESPONDER = (1 << 12)
};

#define IS_HT_STA(_pMacEntry)	\
	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#define IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#define PEER_IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

VOID set_sta_ht_cap(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *ent, HT_CAPABILITY_IE *ht_ie);

VOID RTMPSetHT(struct _RTMP_ADAPTER *pAd, OID_SET_HT_PHYMODE *pHTPhyMode, struct wifi_dev *wdev);
VOID RTMPSetIndividualHT(struct _RTMP_ADAPTER *pAd, UCHAR apidx);
UINT32 starec_ht_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature);

UCHAR get_cent_ch_by_htinfo(
	struct _RTMP_ADAPTER *pAd,
	struct _ADD_HT_INFO_IE *ht_op,
	HT_CAPABILITY_IE *ht_cap);

INT get_ht_cent_ch(struct _RTMP_ADAPTER *pAd, UINT8 *rf_bw, UINT8 *ext_ch, UCHAR Channel);
INT ht_mode_adjust(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry, HT_CAPABILITY_IE *peer_ht_cap);
UINT8 get_max_nss_by_htcap_ie_mcs(UCHAR *cap_mcs);
INT set_ht_fixed_mcs(struct _MAC_TABLE_ENTRY *pEntry, UCHAR fixed_mcs, UCHAR mcs_bound);
INT get_ht_max_mcs(UCHAR *desire_mcs, UCHAR *cap_mcs);
UCHAR cal_ht_cent_ch(UCHAR prim_ch, UCHAR phy_bw, UCHAR ext_cha, UCHAR *cen_ch);
INT build_ht_ies(struct _RTMP_ADAPTER *pAd, struct _build_ie_info *info);
VOID ie_field_value_decision(struct wifi_dev *wdev, BCN_IE_LIST *ie_list);

#define MAKE_IE_TO_BUF(__BUF, __CONTENT, __CONTENT_LEN, __CUR_LEN) \
{																   \
	NdisMoveMemory((__BUF+__CUR_LEN), (UCHAR *)(__CONTENT), __CONTENT_LEN);    \
	__CUR_LEN += __CONTENT_LEN;									   \
}
#endif /*__HT_H__*/
