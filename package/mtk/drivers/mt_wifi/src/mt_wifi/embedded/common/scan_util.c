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
 ***************************************************************************

    Module Name:
    fsm_sync.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
				2016-08-25		Unify the SCAN API into one
*/


#include "rt_config.h"

#ifdef CONFIG_STA_SUPPORT
VOID StaSiteSurvey(
	IN  PRTMP_ADAPTER pAd,
	IN  PNDIS_802_11_SSID pSsid,
	IN  UCHAR ScanType,
	IN  struct wifi_dev *wdev)
{
	MLME_SCAN_REQ_STRUCT    ScanReq;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
		/*
		 * Still scanning, ignore this scanning.
		 */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "StaSiteSurvey:: Scanning now\n");
#ifdef APCLI_CFG80211_SUPPORT
		if (pAd->cfg80211_ctrl.FlgCfg80211Scanning == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "scan Already running abort new scan\n");
			CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
			pAd->cfg80211_ctrl.FlgCfg80211Scanning = FALSE;
		}
#endif
		return;
	}

	if (INFRA_ON(pStaCfg))
		ScanType = SCAN_IMPROVED;

#ifdef WSC_STA_SUPPORT

	if (ScanType == SCAN_WSC_ACTIVE)
		ScanType = SCAN_ACTIVE;

#endif /* WSC_STA_SUPPORT */

	/*To do SiteSurvey, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN, FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "TakeChannelOpCharge fail for SCAN!!\n");
#ifdef APCLI_CFG80211_SUPPORT
		if (pAd->cfg80211_ctrl.FlgCfg80211Scanning == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "scan owenership failed abort new scan\n");
			CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
			pAd->cfg80211_ctrl.FlgCfg80211Scanning = FALSE;
		}
#endif
		return;
	}

	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;

	if ((pSsid) && (pSsid->SsidLength > 0) && (pSsid->SsidLength <= (NDIS_802_11_LENGTH_SSID))) {
		ScanReq.SsidLen = pSsid->SsidLength;
		NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}

	ScanReq.BssType = BSS_ANY;
	ScanReq.ScanType = ScanType;
	cntl_scan_request(wdev, &ScanReq);
}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID ApSiteSurvey_by_wdev(
	PRTMP_ADAPTER	pAd,
	PNDIS_802_11_SSID	pSsid,
	UCHAR				ScanType,
	BOOLEAN				ChannelSel,
	struct wifi_dev		*wdev)
{
	MLME_SCAN_REQ_STRUCT    ScanReq;
	SCAN_CTRL *ScanCtrl = NULL;
	BSS_TABLE *ScanTab = NULL;
	UINT8 band_idx;
#ifdef CON_WPS
	UCHAR ifIdx;
#endif /*ifIdx*/
	struct DOT11_H *pDot11hTest = NULL;
	BOOLEAN bSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"wdev = NULL!\n");
		return;
	}

	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	band_idx = HcGetBandByWdev(wdev);
	pDot11hTest = &pAd->Dot11_H[band_idx];

	if (bSupport5G && (pAd->CommonCfg.bIEEE80211H == 1)
#ifdef MT_DFS_SUPPORT
		&& (pAd->CommonCfg.DfsParameter.bDfsEnable == 1)
#endif
	)
	if (
#ifdef CONFIG_MAP_SUPPORT
		(IS_MAP_ENABLE(pAd)) && (wdev->wdev_type != WDEV_TYPE_STA) &&
#endif
		(pDot11hTest->RDMode == RD_SILENCE_MODE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "INFO::AP is in Silent Mode.DFS CAC is under process!\n");
		return;
	}
#ifdef SCAN_SUPPORT
	if (scan_in_run_state(pAd, wdev)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
			"Failed!!!Scan is running, please try again after scan done!\n");
		return;
	}
#endif

	/*To do SiteSurvey, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN, FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "TakeChannelOpCharge fail for SCAN!!\n");
		return;
	}

	/* Don't clear the scan table if we are doing partial scan */
#ifdef CON_WPS
	ifIdx = wdev->func_idx;

	if ((ifIdx < MAX_APCLI_NUM)
		&& (pAd->ApCfg.ConWpsApCliDisabled == FALSE)
		&& (pAd->StaCfg[ifIdx].wdev.WscControl.conWscStatus & CON_WPS_STATUS_APCLI_RUNNING))
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "\033[1;32m ApSiteSurvey_by_wdev don't need Init BSS table\033[0m\n");
	else
#endif /*CON_WPS*/
		if ((ScanCtrl->PartialScan.bScanning == TRUE && ScanCtrl->Channel == 0) ||
			ScanCtrl->PartialScan.bScanning == FALSE)
			BssTableInit(ScanTab);

	pAd->ApCfg.bAutoChannelAtBootup[band_idx] = ChannelSel;
#ifdef OFFCHANNEL_SCAN_FEATURE
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "INFO::reset ChannelInfo!\n");
	ChannelInfoResetNew(pAd);
	RTMPZeroMemory(&ScanReq, sizeof(ScanReq));
#else
	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;
#endif

	if ((pSsid) && (pSsid->SsidLength > 0) && (pSsid->SsidLength <= (NDIS_802_11_LENGTH_SSID))) {
		ScanReq.SsidLen = pSsid->SsidLength;
		NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}

	ScanReq.BssType = BSS_ANY;
	ScanReq.ScanType = ScanType;
	cntl_scan_request(wdev, &ScanReq);
}
#ifdef OFFCHANNEL_SCAN_FEATURE
UCHAR Channel2Index(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel,
	IN UCHAR BandIdx)
{
	int i = 0;
	CHANNEL_CTRL *pChCtrl = NULL;
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	for (i = 0; i < pChCtrl->ChListNum ; i++) {
		if (channel == pChCtrl->ChList[i].Channel)
			return i;
	}
	 return -1;
}

INT ApSiteSurveyNew_by_wdev(
	IN	PRTMP_ADAPTER	pAd,
	IN  UINT 		channel,
	IN  UINT 		timeout,
	IN	UCHAR		ScanType,
	IN	BOOLEAN		ChannelSel,
	struct wifi_dev 	*wdev)
{
	UCHAR BandIdx = 0;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	MLME_SCAN_REQ_STRUCT    ScanReq;
	BandIdx = HcGetBandByWdev(wdev);

#ifdef SCAN_SUPPORT
	if (scan_in_run_state(pAd, wdev)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
			"Failed!!!Scan is running, please try again after scan done!\n");
		return FALSE;
	}
#endif

	RTMPZeroMemory(&ScanReq, sizeof(ScanReq));;
	AsicDisableSync(pAd, HW_BSSID_0);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG, "%u, %u, %u", channel, timeout, ScanType);
	BssTableInit(ScanTab);
	ChannelInfoResetNew(pAd);
	/* pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE; */
	/* TODO: Raghav: cancel existing scan */
	ScanCtrl->CurrentGivenChan_Index = 0;

	ScanReq.BssType = BSS_ANY;
	ScanReq.ScanType = ScanType;
	/* to make the code compatible with non application/iwpriv  path */
	if (!ScanCtrl->ScanTime[0]) {
		ScanCtrl->ScanTime[0] = timeout;
		ScanCtrl->ScanGivenChannel[0] = channel;
	}

	pAd->ApCfg.bAutoChannelAtBootup[BandIdx] = ChannelSel;
	pAd->ChannelInfo.bandidx = BandIdx;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
			"bandidx :%d!! \n", pAd->ChannelInfo.bandidx);
	if (channel) {
			pAd->ApCfg.current_channel_index = Channel2Index (pAd, channel, BandIdx);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"ApCfg.current_channel_index = %d\n", pAd->ApCfg.current_channel_index);
			pAd->ChannelInfo.ChannelNo = channel;
	}
	cntl_scan_request(wdev, &ScanReq);
	return CHANNEL_MONITOR_STRG_SUCCESS;
}
#endif
#endif /* CONFIG_AP_SUPPORT */

/* 3: Customer CH List Scan */

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID  TriEventInit(RTMP_ADAPTER *pAd)
{
	UCHAR i;

	for (i = 0; i < MAX_TRIGGER_EVENT; i++)
		pAd->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;

	pAd->CommonCfg.TriggerEventTab.EventANo = 0;
	pAd->CommonCfg.TriggerEventTab.EventBCountDown = 0;
}

INT TriEventTableSetEntry(
	IN RTMP_ADAPTER *pAd,
	OUT TRIGGER_EVENT_TAB * Tab,
	IN UCHAR *pBssid,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN BOOLEAN has_ht_cap,
	IN UCHAR RegClass,
	IN UCHAR ChannelNo)
{
	/* Event A, legacy AP exist.*/
	if (!has_ht_cap) {
		UCHAR index;

		/*
			Check if we already set this entry in the Event Table.
		*/
		for (index = 0; index < MAX_TRIGGER_EVENT; index++) {
			if ((Tab->EventA[index].bValid == TRUE) &&
				(Tab->EventA[index].Channel == ChannelNo) &&
				(Tab->EventA[index].RegClass == RegClass)
			   ) {
				return 0;
			}
		}

		/*
			If not set, add it to the Event table
		*/
		if (Tab->EventANo < MAX_TRIGGER_EVENT) {
			RTMPMoveMemory(Tab->EventA[Tab->EventANo].BSSID, pBssid, 6);
			Tab->EventA[Tab->EventANo].bValid = TRUE;
			Tab->EventA[Tab->EventANo].Channel = ChannelNo;

			if (RegClass != 0) {
				/* Beacon has Regulatory class IE. So use beacon's*/
				Tab->EventA[Tab->EventANo].RegClass = RegClass;
			} else {
				/* Use Station's Regulatory class instead.*/
				/* If no Reg Class in Beacon, set to "unknown"*/
				/* TODO:  Need to check if this's valid*/
				Tab->EventA[Tab->EventANo].RegClass = 0; /* ????????????????? need to check*/
			}

			Tab->EventANo++;
		}
	}
	else if (pHtCapability->HtCapInfo.Forty_Mhz_Intolerant) {
		/* Event B.   My BSS beacon has Intolerant40 bit set*/
		Tab->EventBCountDown = pAd->CommonCfg.Dot11BssWidthChanTranDelay;
	}

	return 0;
}

BOOLEAN build_trigger_event_table(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem, BCN_IE_LIST *ie_list)
{
	/* Check if this scan channel is the effeced channel */
	struct wifi_dev *wdev = (struct wifi_dev *)Elem->wdev;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
		((ie_list->Channel > 0) && (ie_list->Channel <= 14))) {
		int chListIdx = 0;

		/*
			First we find the channel list idx by the channel number
		*/
		for (chListIdx = 0; chListIdx < pChCtrl->ChListNum; chListIdx++) {
			if (ie_list->Channel == pChCtrl->ChList[chListIdx].Channel)
				break;
		}

		if (chListIdx < pChCtrl->ChListNum) {

			/*
				If this channel is effected channel for the 20/40 coex operation. Check the related IEs.
			*/
			if (pChCtrl->ChList[chListIdx].bEffectedChannel == TRUE) {
				UCHAR RegClass;
				OVERLAP_BSS_SCAN_IE BssScan;
				/* Read Beacon's Reg Class IE if any. */
				PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, ie_list, &RegClass);
				/* printk("\x1b[31m TriEventTableSetEntry \x1b[m\n"); */
				TriEventTableSetEntry(pAd, &pAd->CommonCfg.TriggerEventTab, ie_list->Bssid,
						&ie_list->cmm_ies.ht_cap, HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists),
						RegClass, ie_list->Channel);
			}
		}
	}

	return TRUE;
}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */

