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
	cmm_asic_mt.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __CMM_ASIC_MT_FMAC_H__
#define __CMM_ASIC_MT_FMAC_H__

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _CIPHER_KEY;
struct _MT_TX_COUNTER;
struct _EDCA_PARM;
struct _EXT_CMD_CHAN_SWITCH_T;
struct _BCTRL_INFO_T;
struct _RX_BLK;
struct _BSS_INFO_ARGUMENT_T;

VOID mtf_wrap_protinfo_in_bssinfo(struct _RTMP_ADAPTER *ad, VOID *cookie);
VOID MtfAsicSwitchChannel(struct _RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg);
INT MtfAsicSetRxFilter(struct _RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilter);
INT MtfAsicSetGPTimer(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
#ifndef MT7915_MT7916_COEXIST_COMPATIBLE
INT MtfAsicGetTsfTimeByDriver(struct _RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part, UCHAR HwBssidIdx);
#if defined(MT7986) || defined(MT7916) || defined(MT7981)
INT MtfAsicGetTsfTimeByDriver_FMAC(struct _RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part, UCHAR HwBssidIdx);
#endif
#endif
UINT32 MtfAsicGetWmmParam(struct _RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType);
VOID MtfAsicTxCapAndRateTableUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2Wcid,
	RA_PHY_CFG_T *prTxPhyCfg,
	UINT32 *Rate,
	BOOL fgSpeEn);
VOID MtfAsicUpdateRxWCIDTable(struct _RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo);
INT32 MtfAsicSetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx);
INT MtfAsicSetTxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
INT MtfAsicSetBW(struct _RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx);
INT32 MtfAsicGetFwSyncValue(struct _RTMP_ADAPTER *pAd);
VOID MtfAsicInitMac(struct _RTMP_ADAPTER *pAd);
UINT32 MtfAsicGetRxStat(struct _RTMP_ADAPTER *pAd, UINT type);
VOID MtfDmacSetExtTTTTHwCRSetting(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID MtfDmacSetMbssHwCRSetting(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID MtfDmacSetExtMbssEnableCR(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
INT32 MtfAsicSetBssidByDriver(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info_argument);
BOOLEAN MtfDmacAsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
BOOLEAN MtfDmacAsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
INT MtfAsicTOPInit(struct _RTMP_ADAPTER *pAd);
VOID MtfAsicGetTxTscByDriver(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc);
INT mtf_asic_rts_on_off(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en);
INT mtf_asic_set_agglimit(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UCHAR ac, struct wifi_dev *wdev, UINT32 agg_limit);
INT mtf_asic_set_rts_retrylimit(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 limit);
VOID MtfSetTmrCal(struct _RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Channel, UCHAR Bw);
VOID MtfAsicRcpiReset(RTMP_ADAPTER *pAd, UINT16 wcid);
#ifdef DOT11_VHT_AC
INT mtf_asic_set_rts_signal_ta(struct _RTMP_ADAPTER *ad, UINT8 band_idx, BOOLEAN enable);
#endif
UINT32 mtf_get_mib_bcn_tx_cnt(RTMP_ADAPTER *pAd, UINT8 band_idx);
#ifdef AIR_MONITOR
INT mtf_set_air_monitor_enable(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx);
INT mtf_set_air_monitor_rule(struct _RTMP_ADAPTER *pAd, UCHAR *rule, UCHAR band_idx);
INT mtf_set_air_monitor_idx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR mnt_idx, UCHAR band_idx);
#endif

#endif
