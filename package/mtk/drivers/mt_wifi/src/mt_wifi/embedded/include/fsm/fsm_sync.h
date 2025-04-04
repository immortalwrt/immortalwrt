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
    fsm_sync.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      --------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/
#ifndef __FSM_SYNC_H__
#define __FSM_SYNC_H__

#define SYNC_FSM_IDLE                   0
#define SYNC_FSM_LISTEN                 1
#define SYNC_FSM_JOIN_WAIT              2
#define SYNC_FSM_PENDING				3
#define SYNC_FSM_MAX_STATE              4

#define SYNC_FSM_BASE                   0
#define SYNC_FSM_JOIN_REQ               0
#define SYNC_FSM_JOIN_TIMEOUT           1
#define SYNC_FSM_SCAN_REQ               2
#define SYNC_FSM_SCAN_TIMEOUT			3
#define SYNC_FSM_PEER_PROBE_REQ         4
#define SYNC_FSM_PEER_PROBE_RSP         5
#define SYNC_FSM_PEER_BEACON            6
#define SYNC_FSM_ADHOC_START_REQ		7
#define SYNC_FSM_CANCEL_REQ				8
#define SYNC_FSM_WSC_SCAN_COMP_CHECK_REQ			9
#define SYNC_FSM_MAX_MSG                10

#define SYNC_FSM_FUNC_SIZE              (SYNC_FSM_MAX_STATE * SYNC_FSM_MAX_MSG)

#ifdef HE_SUPPORT
#define IMPROVED_SCAN_CHANNEL_COUNT     3
#else /* !HE_SUPPORT */
#define IMPROVED_SCAN_CHANNEL_COUNT     7
#endif /* HE_SUPPORT */

#ifdef HE_SUPPORT
#define IMPROVED_SCAN_CHANNEL_COUNT     3
#else /* !HE_SUPPORT */
#define IMPROVED_SCAN_CHANNEL_COUNT     7
#endif /* HE_SUPPORT */
#define OBSS_BEACON_RSSI_THRESHOLD		(-85)

#define SHORT_CHANNEL_TIME          90        /* unit: msec */
#define MIN_CHANNEL_TIME            110       /* unit: msec, for dual band scan */
#define MAX_CHANNEL_TIME            140       /* unit: msec, for single band scan */
#define FAST_ACTIVE_SCAN_TIME	    30		  /* Active scan waiting for probe response time */

struct _RTMP_ADAPTER;
struct _PEER_PROBE_REQ_PARAM;
struct sync_fsm_ops {
	BOOLEAN(*tx_probe_response_allowed)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
		struct _PEER_PROBE_REQ_PARAM *ProbeReqParam, MLME_QUEUE_ELEM *Elem);
	BOOLEAN(*tx_probe_response_xmit)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
		struct _PEER_PROBE_REQ_PARAM *ProbeReqParam, MLME_QUEUE_ELEM *Elem);

	BOOLEAN(*rx_peer_response_allowed)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
		BCN_IE_LIST *bcn_ie_list, MLME_QUEUE_ELEM *Elem);
	BOOLEAN(*rx_peer_response_updated)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
		BCN_IE_LIST *bcn_ie_list, MLME_QUEUE_ELEM *Elem, NDIS_802_11_VARIABLE_IEs *pVIE, USHORT LenVIE);

	BOOLEAN(*join_peer_response_matched)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
		BCN_IE_LIST *bcn_ie_list, MLME_QUEUE_ELEM *Elem);
	BOOLEAN(*join_peer_response_updated)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
		BCN_IE_LIST *bcn_ie_list, MLME_QUEUE_ELEM *Elem, NDIS_802_11_VARIABLE_IEs *pVIE, USHORT LenVIE);
};

typedef struct _SCAN_INFO_ {
	UCHAR SyncCurrState;
	ULONG LastScanTime;	/* Record last scan time for issue BSSID_SCAN_LIST */
	/* ULONG LastBeaconRxTime;*/	 /*OS's timestamp of the last BEACON RX time */

	BOOLEAN bImprovedScan;
	UCHAR ScanChannelCnt;	/* 0 at the beginning of scan, stop at 7 */
	UCHAR LastScanChannel;

    UINT32 *ChanList;    /* the channel list from from wpa_supplicant */
    UCHAR ChanListLen;   /* channel list length */
	UCHAR ChanListIdx;   /* current index in channel list when driver in scanning */

	UCHAR *ExtraIe;  /* Carry on Scan action from supplicant */
	UINT   ExtraIeLen;

	BOOLEAN bFastConnect;
	BOOLEAN bNotFirstScan;	/* Sam add for ADHOC flag to do first scan when do initialization */
} SCAN_INFO;

typedef struct _SCAN_ACTION_INFO_ {
	BOOLEAN isScanDone;
	BOOLEAN isScanPending;
} SCAN_ACTION_INFO;

VOID sync_fsm_init(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx, STATE_MACHINE *Sm, STATE_MACHINE_FUNC Trans[]);
VOID sync_fsm_reset(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
BOOLEAN sync_fsm_msg_pre_checker(struct _RTMP_ADAPTER *pAd, PFRAME_802_11 pFrame,
											INT *Machine, INT *MsgType);
VOID sync_fsm_cancel_req_action(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID sync_cntl_fsm_to_idle_when_scan_req(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

BOOLEAN scan_next_channel(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT SCAN_ACTION_INFO *scan_action_info);

BOOLEAN scan_in_run_state(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
BOOLEAN scan_active_probe_disallowed(RTMP_ADAPTER *pAd, UCHAR channel);
VOID ScanParmFill(
	IN  RTMP_ADAPTER *pAd,
	IN  OUT MLME_SCAN_REQ_STRUCT *ScanReq,
	IN  RTMP_STRING Ssid[],
	IN  UCHAR SsidLen,
	IN  UCHAR BssType,
	IN  UCHAR ScanType);

#endif /* __FSM_SYNC_H__ */
