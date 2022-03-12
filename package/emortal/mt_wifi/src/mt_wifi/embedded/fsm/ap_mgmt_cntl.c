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
/***************************************************************************
 ***************************************************************************

	Module Name:
	Common mgmt cntl

*/
#include "rt_config.h"

struct _cntl_api_ops ap_cntl_api_ops;


static VOID ap_cntl_scan(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	if (pAd == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: pAd is NULL!\n", __func__));
		return;
	}

#ifdef CONFIG_ATE
/* Disable scanning when ATE is running. */
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */

	MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_SCAN_REQ,
			Elem->MsgLen, Elem->Msg, 0, wdev);

	cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
}

static VOID ap_cntl_scan_conf(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	USHORT	status = MLME_SUCCESS;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	SCAN_INFO *ScanInfo;
	INT BssIdx;
	INT MaxNumBss = 0;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	if (pAd == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: pAd is NULL!\n", __func__));
		return;
	}
	MaxNumBss = pAd->ApCfg.BssidNum;
	ScanInfo = &wdev->ScanInfo;

	os_move_mem(&status, Elem->Msg, sizeof(USHORT));

	/* scan completed, init to not FastScan */
	ScanInfo->bImprovedScan = FALSE;

#ifdef RT_CFG80211_SUPPORT
	 RTEnqueueInternalCmd(pAd, CMDTHREAD_SCAN_END, NULL, 0);
#endif /* RT_CFG80211_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	/* */
	/* Set LED status to previous status. */
	/* */
	if (pAd->LedCntl.bLedOnScanning) {
		pAd->LedCntl.bLedOnScanning = FALSE;
		RTMPSetLED(pAd, pAd->LedCntl.LedStatus, HcGetBandByWdev(wdev));
	}
#endif /* LED_CONTROL_SUPPORT */

	if (status == MLME_SUCCESS)	{
		/*
			Maintain Scan Table
			MaxBeaconRxTimeDiff: 120 seconds
			MaxSameBeaconRxTimeCount: 1
		*/

		/* MaintainBssTable(pAd, wdev, &pAd->ScanTab, 120, 2); */

		RTMPSendWirelessEvent(pAd, IW_SCAN_COMPLETED_EVENT_FLAG, NULL, BSS0, 0);
#ifdef WPA_SUPPLICANT_SUPPORT
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_SCAN, -1, NULL, NULL, 0);
#endif /* WPA_SUPPLICANT_SUPPORT */
	}

	cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);

	AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0, OPMODE_AP);
	/* ap_beacon_disabled(pAd, FALSE);*/
	/* Enable beacon tx for all BSS */
	for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
		struct wifi_dev *apWdev = NULL;

		apWdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;

		if (wdev->bAllowBeaconing)
			UpdateBeaconHandler(pAd, apWdev, BCN_UPDATE_ENABLE_TX);
	}
}

static VOID ap_cntl_error_handle(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	/* USHORT	status = MLME_SUCCESS; */
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	ASSERT(pAd);

	wdev = Elem->wdev;

}

static VOID ap_cntl_disconnect_proc(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev;
	CNTL_MLME_DISCONNECT_STRUCT *cntl_disconn;
	MLME_DISCONNECT_STRUCT mlme_disconn;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (PRTMP_ADAPTER)wdev->sys_handle;
	cntl_disconn = (CNTL_MLME_DISCONNECT_STRUCT *)Elem->Msg;
	mlme_disconn.reason = cntl_disconn->mlme_disconn.reason;
	os_move_mem(mlme_disconn.addr, cntl_disconn->mlme_disconn.addr, MAC_ADDR_LEN);

	if (cntl_disconn->cntl_disconn_type == CNTL_DEAUTH) {
		MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_MLME_DEAUTH_REQ,
			sizeof(MLME_DISCONNECT_STRUCT), &mlme_disconn, 0,
			wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_DEAUTH, __func__);
		RTMP_MLME_HANDLER(pAd);
	} else if (cntl_disconn->cntl_disconn_type == CNTL_DISASSOC) {
		MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_MLME_DISASSOC_REQ,
			sizeof(MLME_DISCONNECT_STRUCT), &mlme_disconn, 0,
			wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_DISASSOC, __func__);
		RTMP_MLME_HANDLER(pAd);
	}

}

VOID ap_cntl_init(
	struct wifi_dev *wdev)
{
	ap_cntl_api_ops.cntl_disconnect_proc = ap_cntl_disconnect_proc;
	ap_cntl_api_ops.cntl_scan_proc = ap_cntl_scan;
	ap_cntl_api_ops.cntl_scan_conf = ap_cntl_scan_conf;
	ap_cntl_api_ops.cntl_error_handle = ap_cntl_error_handle;
	wdev->cntl_api = &ap_cntl_api_ops;
}
