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
    bcn.h

    Abstract:
    bcn related fucntions definition.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Carter      2014-1121     Created.
*/

#ifndef __BCN_H__
#define __BCN_H__

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
#define MAX_BEACONV2_LENGTH       (sizeof(HEADER_802_11) + \
									 TIMESTAMP_FIELD_LEN + \
									 BEACON_INTERVAL_FIELD_LEN + \
									 CAP_INFO_FIELD_LEN + \
									 MAX_IE_V2_LENGTH)
#endif
#define MAX_BEACON_LENGTH       (sizeof(HEADER_802_11) + \
								 TIMESTAMP_FIELD_LEN + \
								 BEACON_INTERVAL_FIELD_LEN + \
								 CAP_INFO_FIELD_LEN + \
								 MAX_IE_LENGTH)

typedef enum _BCN_UPDATE_REASON {
	BCN_UPDATE_INIT		= 0,	/* beacon resource initial. */
	BCN_UPDATE_IF_STATE_CHG	= 1,	/* there is interface up or down, check related TXD handle. */
	BCN_UPDATE_IE_CHG		= 2,	/* simple IE change, just update the corresponding interface content. */
	BCN_UPDATE_ALL_AP_RENEW	= 3,	/* prepared All beacon for All active interface. */
	BCN_UPDATE_PRETBTT		= 4,	/* update function routine, source could from INT isr or timer or event notify. */
	BCN_UPDATE_ENABLE_TX	= 5,	/* Enable Beacon TX. */
	BCN_UPDATE_DISABLE_TX	= 6,	/* Disable Beacon TX. */
	BCN_UPDATE_TIM		= 7,	/* TIM preparing */
	BCN_UPDATE_CSA		= 8,	/* CSA (Channel Switch Announcement) */
	BCN_UPDATE_BTWT_IE		= 9,	/* bTWT element */
	BCN_UPDATE_RESERVE		= 10,
} BCN_UPDATE_REASON;

typedef enum _BCN_GEN_METHOD {
	BCN_GEN_BY_HW_SHARED_MEM = 0,   /* RT chip */
	BCN_GEN_BY_HOST_IN_PRETBTT,     /* MT_chip with small size fw */
	BCN_GEN_BY_FW,                  /* MT_chip with large size fw */
	BCN_GEN_BY_HOST_TOUCH_PSE_CTNT  /* TODO:MT_chip don't free bcn in BcnQ, recycle update by Host */
} BCN_GEN_METHOD;

INT bcn_buf_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT bcn_buf_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT clearHwBcnTxMem(RTMP_ADAPTER *pAd);

ULONG ComposeBcnPktHead(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pBeaconFrame);
VOID BcnCheck(RTMP_ADAPTER *pAd);

#ifdef CONFIG_AP_SUPPORT
INT BcnTimUpdate(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *ptr);
#endif
VOID ComposeBcnPktTail(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame);

UINT16 MakeBeacon(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine);

BOOLEAN BeaconTransmitRequired(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine);

VOID UpdateBeaconHandler(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BCN_UPDATE_REASON reason);

BOOLEAN UpdateBeaconProc(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine,
	UCHAR UpdatePktType,
	BOOLEAN bMakeBeacon,
	UCHAR UpdateReason);

VOID updateBeaconRoutineCase(
	RTMP_ADAPTER *pAd,
	BOOLEAN UpdateAfterTim);

#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)
VOID MakeHotSpotIE(
	struct wifi_dev *wdev,
	ULONG *pFrameLen,
	UCHAR *pBeaconFrame);
#endif /* defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT) */

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
#define IS_BSSID_11V_ENABLED(_pAd, _band) \
	((_pAd)->ApCfg.dot11v_mbssid_bitmap[_band] != 0)

#define IS_BSSID_11V_TRANSMITTED(_pAd, _pMbss, _band) \
	(IS_BSSID_11V_ENABLED(_pAd, _band) && \
	(((BSS_STRUCT *)_pMbss)->mbss_idx == (_pAd)->ApCfg.dot11v_trans_bss_idx[_band]))

#define IS_BSSID_11V_NON_TRANS(_pAd, _pMbss, _band) \
	(IS_BSSID_11V_ENABLED(_pAd, _band) && \
	(!IS_BSSID_11V_TRANSMITTED(_pAd, _pMbss, _band)) && \
	((_pAd)->ApCfg.dot11v_mbssid_bitmap[_band] & (1 << ((BSS_STRUCT *)_pMbss)->mbss_grp_idx)))

#define IS_BSSID_11V_CO_HOSTED(_pAd, _pMbss, _band) \
	(IS_BSSID_11V_ENABLED(_pAd, _band) && \
	(!IS_BSSID_11V_TRANSMITTED(_pAd, _pMbss, _band)) && \
	(!IS_BSSID_11V_NON_TRANS(_pAd, _pMbss, _band)))

/*
 * transmitted-BSSID is responsible for (Multiple BSSID) IEs creating to Nontransmitted-BSSID
 * check if any Nontransmitted-BSSID IE needed
 */
#define IS_MBSSID_IE_NEEDED(_pAd, _pMbss, _band) \
	(IS_BSSID_11V_TRANSMITTED(_pAd, _pMbss, _band) && \
	((_pAd)->ApCfg.dot11v_mbssid_bitmap[_band] & (~(1 << ((BSS_STRUCT *)_pMbss)->mbss_grp_idx))))

#define SUB_IE_NON_TRANS_PROFILE			0
#define SUB_IE_NON_TRANS_VENDOR_SPECIFIC	221

typedef struct _MULTIPLE_BSSID_SUB_IE_T {
	UCHAR sub_eid;
	UCHAR len;
	UCHAR non_trans_profiles[];
} MULTIPLE_BSSID_SUB_IE_T, *P_MULTIPLE_BSSID_SUB_IE_T;

typedef struct _MULTIPLE_BSSID_IE_T {
	UCHAR eid;
	UCHAR len;
	UCHAR dot11v_max_bssid_indicator;
	UCHAR sub_ies[];
} MULTIPLE_BSSID_IE_T, *P_MULTIPLE_BSSID_IE_T;

VOID make_multiple_bssid_ie(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	ULONG *pFrameLen,
	UCHAR *pOutBuffer,
	UINT32 Bitmap,
	BOOLEAN isProbeRsp);
#endif
#endif /* CONFIG_AP_SUPPORT */

#endif  /* __BCN_H__ */
