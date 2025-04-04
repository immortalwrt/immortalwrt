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

#include "rt_config.h"

#ifdef SCAN_SUPPORT
extern UCHAR ZeroSsid[MAX_LEN_OF_SSID];

static inline UCHAR wdev_get_op_mode(struct wifi_dev *wdev)
{
	if ((wdev->wdev_type == WDEV_TYPE_STA) ||
		(wdev->wdev_type == WDEV_TYPE_ADHOC) ||
		(wdev->wdev_type == WDEV_TYPE_MESH))
		return OPMODE_STA;

	return OPMODE_AP;
}

static VOID restart_partial_scan(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	PARTIAL_SCAN *PartialScan;

	PartialScan = (PARTIAL_SCAN *) FunctionContext;
	PartialScan->bScanning = TRUE;
}

DECLARE_TIMER_FUNCTION(restart_partial_scan);
BUILD_TIMER_FUNCTION(restart_partial_scan);

#ifdef OFFCHANNEL_ZERO_LOSS
UINT32 diffu32(UINT32 New, UINT32 Old)
{
	if (New >= Old)
		return (New - Old);
	else
		return ((0xffffffff - Old) + New + 1);
}

void read_channel_stats(RTMP_ADAPTER *pAd, UINT8 BandIdx, void *ChStat)
{
	RTMP_MIB_PAIR Reg[8];
	P_MT_MIB_COUNTER_STAT pChStat = (P_MT_MIB_COUNTER_STAT)ChStat;

	Reg[0].Counter = RMAC_CNT_OBSS_AIRTIME;/* RMAC.AIRTIME14 OBSS Air time */
	Reg[1].Counter = MIB_CNT_TX_DUR_CNT;/* M0SDR36 TX Air time */
	Reg[2].Counter = MIB_CNT_RX_DUR_CNT;/* M0SDR37 RX Air time */
	Reg[3].Counter = RMAC_CNT_NONWIFI_AIRTIME;/* RMAC.AIRTIME13 Non Wifi Air time */
	Reg[4].Counter = MIB_CNT_CCA_NAV_TX_TIME;/* M0SDR9 Channel Busy Time */
	Reg[5].Counter = MIB_CNT_P_CCA_TIME;/* M0SDR16 Primary Channel Busy Time */
	Reg[6].Counter = MIB_CNT_MAC2PHY_TX_TIME;/* M0SDR35 MAC2PHY Tx Time */
	Reg[7].Counter = MIB_CNT_BA_CNT;/* M0SDR31 BA Count */

	MtCmdMultipleMibRegAccessRead(pAd, BandIdx, Reg, 8);

	pChStat->ObssAirtimeAcc[BandIdx] = (UINT32)(Reg[0].Value);
	pChStat->MyTxAirtimeAcc[BandIdx] = (UINT32)(Reg[1].Value);
	pChStat->MyRxAirtimeAcc[BandIdx] = (UINT32)(Reg[2].Value);
	pChStat->EdccaAirtimeAcc[BandIdx] = (UINT32)(Reg[3].Value);
	pChStat->CcaNavTxTimeAcc[BandIdx] = (UINT32)(Reg[4].Value);
	pChStat->PCcaTimeAcc[BandIdx] = (UINT32)(Reg[5].Value);
	pChStat->MyMac2PhyTxTimeAcc[BandIdx] = (UINT32)(Reg[6].Value);
	pChStat->BACountAcc[BandIdx] = (UINT32)(Reg[7].Value);
}

void update_scan_channel_stats(RTMP_ADAPTER *pAd, UINT8 BandIdx, SCAN_CTRL *ScanCtrl)
{
	UINT32	OBSSAirtime, MyTxAirtime, MyRxAirtime, PCCA_Time, MyTx2Airtime, BACount;
	UINT32 ChBusyTime;
	UINT64	BATime;

	//OBSS Air time
	OBSSAirtime = diffu32(ScanCtrl->OffCannelScanStopStats.ObssAirtimeAcc[BandIdx],
					ScanCtrl->OffCannelScanStartStats.ObssAirtimeAcc[BandIdx]);
	pAd->ScanChnlStats[BandIdx].Obss_Time = OBSSAirtime;

	//My Tx Air time
	MyTxAirtime = diffu32(ScanCtrl->OffCannelScanStopStats.MyTxAirtimeAcc[BandIdx],
					ScanCtrl->OffCannelScanStartStats.MyTxAirtimeAcc[BandIdx]);
	pAd->ScanChnlStats[BandIdx].Tx_Time = MyTxAirtime;

	//My Rx Air time
	MyRxAirtime = diffu32(ScanCtrl->OffCannelScanStopStats.MyRxAirtimeAcc[BandIdx],
					ScanCtrl->OffCannelScanStartStats.MyRxAirtimeAcc[BandIdx]);
	pAd->ScanChnlStats[BandIdx].Rx_Time = MyRxAirtime;

	//PCCA time
	PCCA_Time = diffu32(ScanCtrl->OffCannelScanStopStats.PCcaTimeAcc[BandIdx],
					ScanCtrl->OffCannelScanStartStats.PCcaTimeAcc[BandIdx]);
	pAd->ScanChnlStats[BandIdx].PCCA_Time = PCCA_Time;

	//Mac2PhyTxTime
	MyTx2Airtime = diffu32(ScanCtrl->OffCannelScanStopStats.MyMac2PhyTxTimeAcc[BandIdx],
					ScanCtrl->OffCannelScanStartStats.MyMac2PhyTxTimeAcc[BandIdx]);
	pAd->ScanChnlStats[BandIdx].Tx2_Time = MyTx2Airtime;

	/*BA time = BA_Count * 32 (32 = Tx time of 1 BA @ 24Mbps)
	*  32 = TxtimeBA=Tpreamble + Tsig + (Tsym*Nsym) = 16 + 4 +(4*3)
	*/
	BACount = diffu32(ScanCtrl->OffCannelScanStopStats.BACountAcc[BandIdx],
					ScanCtrl->OffCannelScanStartStats.BACountAcc[BandIdx]);
	BATime = BACount * 32;
	pAd->ScanChnlStats[BandIdx].BA_Time = BATime;

	//Obss time, Max( PCCA-MyRx-BA,(OBSS > 20% ?  OBSS + newSCCA : OBSS)  )
	if ((OBSSAirtime/(ScanCtrl->ScanTime[ScanCtrl->CurrentGivenChan_Index] * 10)) > 20) {	// OBSS+newSCCA
		if (MyRxAirtime + BATime > PCCA_Time)
			pAd->ScanChnlStats[BandIdx].Obss_Time = OBSSAirtime + pAd->Ch_Stats[BandIdx].SCCA_Time;
		else if (PCCA_Time - MyRxAirtime - BATime > OBSSAirtime + pAd->Ch_Stats[BandIdx].SCCA_Time)
			pAd->ScanChnlStats[BandIdx].Obss_Time = PCCA_Time - MyRxAirtime - BATime;
		else
			pAd->ScanChnlStats[BandIdx].Obss_Time = OBSSAirtime + pAd->Ch_Stats[BandIdx].SCCA_Time;
	} else {// OBSS
		pAd->ScanChnlStats[BandIdx].Obss_Time = OBSSAirtime;
	}

	//Ch Busy time, Max( MyTx+PCCA, MyTx+BA+(OBSS% > 20 ?  OBSS + newSCCA : OBSS))
	if ((OBSSAirtime/(ScanCtrl->ScanTime[ScanCtrl->CurrentGivenChan_Index] * 10)) > 20) {	// OBSS+IPI
		if (PCCA_Time > BATime + OBSSAirtime + pAd->Ch_Stats[BandIdx].SCCA_Time)
			ChBusyTime = MyTx2Airtime + PCCA_Time;
		else
			ChBusyTime = MyTx2Airtime + BATime + OBSSAirtime + pAd->Ch_Stats[BandIdx].SCCA_Time;
	} else {// OBSS
		ChBusyTime = MyTx2Airtime + BATime + OBSSAirtime;
	}

	pAd->ChannelInfo.chanbusytime[pAd->ApCfg.current_channel_index] = ChBusyTime ;

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[%d][%s]: band[%d] ChannelBusyTime: %u Rx-Time: %u PCCA Time : %u Obss-Time: %u\n",
		//__LINE__ ,__FUNCTION__, BandIdx, pAd->Ch_BusyTime[BandIdx], pAd->Ch_Stats[BandIdx].Rx_Time, pAd->Ch_Stats[BandIdx].PCCA_Time, pAd->Ch_Stats[DBDC_BAND0].Obss_Time));
}
#endif /*OFFCHANNEL_ZERO_LOSS*/

INT scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev)
{
	UCHAR bw, ch;
	struct wifi_dev *wdev = pwdev;
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	SCAN_INFO *ScanInfo;
	UINT  ScanType = ScanCtrl->ScanType;
	UCHAR ht_bw = HT_BW_20, ext_cha = EXTCHA_NONE;
	INT BssIdx;
	INT MaxNumBss = pAd->ApCfg.BssidNum;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
	struct DOT11_H *pDot11h = NULL;
#endif

#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
	PSTA_ADMIN_CONFIG pApCliEntry = pApCliEntry = &pAd->StaCfg[MAIN_MBSSID];
	struct wifi_dev *p2p_wdev = &pMbss->wdev;

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
		p2p_wdev = &pMbss->wdev;
	else if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd))
		p2p_wdev = &pApCliEntry->wdev;

#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (!wdev) {
			pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
			wdev = &pMbss->wdev;
		}
	}
#endif /*CONFIG_AP_SUPPORT*/
	if (!wdev)
		return FALSE;
	ScanInfo = &wdev->ScanInfo;
	ch = wlan_operate_get_cen_ch_1(wdev);
	bw = wlan_operate_get_bw(wdev);

#ifdef CONFIG_STA_SUPPORT

	if (OpMode == OPMODE_STA) {
		/*check if wdev is mix mode and channel will go to other band, switch wdev to difference band*/
		HcCrossChannelCheck(pAd,  wdev, ch);
	}

#endif /*CONFIG_STA_SUPPORT*/
#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd) && (ch != p2p_wdev->channel) && (p2p_wdev->CentralChannel != 0))
		bw = wlan_operate_get_ht_bw(p2p_wdev);
	else if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) && (ch != p2p_wdev->channel) && (p2p_wdev->CentralChannel != 0))
		bw = wlan_operate_get_ht_bw(p2p_wdev);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "scan ch restore   ch %d  p2p_wdev->CentralChannel%d\n", ch, p2p_wdev->CentralChannel);

	/*If GO start, we need to change to GO Channel*/
	if ((ch != p2p_wdev->CentralChannel) && (p2p_wdev->CentralChannel != 0))
		ch = p2p_wdev->CentralChannel;

#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */
#ifdef OFFCHANNEL_SCAN_FEATURE
	if (ScanCtrl->state != OFFCHANNEL_SCAN_INVALID) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"restore channel selected from stored channel\n");
		ch = wdev->restore_channel;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"restore channel done in non-offchannel scan path\n");
		ch = wlan_operate_get_prim_ch(wdev);
	}
#else
		ch = wlan_operate_get_prim_ch(wdev);
#endif
	/*restore to original channel*/
	if (WMODE_CAP_2G(wdev->PhyMode) && pAd->CommonCfg.need_fallback == 1) {
		ht_bw = wlan_operate_get_ht_bw(wdev);
		ext_cha = wlan_operate_get_ext_cha(wdev);
	}

	if (ch != 0)
		wlan_operate_set_prim_ch(wdev, ch);
	if (WMODE_CAP_2G(wdev->PhyMode) && pAd->CommonCfg.need_fallback == 1) {
		wlan_operate_set_ht_bw(wdev, ht_bw, ext_cha);
	}
	ch = wlan_operate_get_cen_ch_1(wdev);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "End of SCAN(TYPE: %d, BandIdx: %d), restore to BW(%d) channel %d, Total BSS[%02d]\n",
			 ScanType, ScanCtrl->BandIdx, wlan_operate_get_bw(wdev), ch, ScanTab->BssNr);

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP && scan_in_run_state(pAd, wdev)) {
		UINT i;
		int alloc_size = 0;

		struct customer_bss_entry *bss_entry;

		RTMP_SPIN_LOCK(&ScanTab->event_bss_entry_lock);
		for (i = 0; i < ScanTab->BssNr; i++) {
			bss_entry = &ScanTab->BssEntry[i].CustomerBssEntry;
			alloc_size += sizeof(struct event_bss_entry);
			alloc_size += bss_entry->vendor_ie.length;
			alloc_size = (alloc_size + (sizeof(UINT32) - 1)) & (~(sizeof(UINT32) - 1));
		}
		ScanTab->EventBssEntryLen = alloc_size;
		RTMP_SPIN_UNLOCK(&ScanTab->event_bss_entry_lock);

		/* send event to userspace */
		RtmpOSWrielessEventSend(pAd->net_dev,
					RT_WLAN_EVENT_CUSTOM,
					OID_SCAN_DONE_EVENT,
					NULL,
					(char *)&alloc_size,
					sizeof(alloc_size));
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	pDot11h = wdev->pDot11_H;

	if (((wdev->channel > 14) &&
		 (pAd->CommonCfg.bIEEE80211H == TRUE) &&
		 RadarChannelCheck(pAd, wdev->channel)) &&
		(pDot11h && pDot11h->RDMode != RD_SWITCHING_MODE)) {
		if (pDot11h->InServiceMonitorCount) {
			struct wifi_dev *apWdev = NULL;

			pDot11h->RDMode = RD_NORMAL_MODE;
			AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
				HW_BSSID_0, OPMODE_AP);

			/* Enable beacon tx for all BSS */
			for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
				apWdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;

				if (!apWdev)
					continue;
				if (apWdev->channel != pwdev->channel)
					continue;
				if (apWdev->bAllowBeaconing)
					UpdateBeaconHandler(pAd, apWdev, BCN_UPDATE_ENABLE_TX);
			}
		} else
			pDot11h->RDMode = RD_SILENCE_MODE;
	} else {
		struct wifi_dev *apWdev = NULL;

		AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
			HW_BSSID_0, OPMODE_AP);

		/* Enable beacon tx for all BSS */
		for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
			apWdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;

			if (!apWdev)
				continue;
			if (apWdev->channel != pwdev->channel)
				continue;
			if (apWdev->bAllowBeaconing)
				UpdateBeaconHandler(pAd, apWdev, BCN_UPDATE_ENABLE_TX);
		}
	}

#ifdef CONFIG_STA_SUPPORT
	if (OpMode == OPMODE_STA) {
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

		ASSERT(pStaCfg);

		if (!pStaCfg)
			return FALSE;
#ifdef CONFIG_MAP_SUPPORT
		wapp_send_scan_complete_notification(pAd, wdev);
#endif /* CONFIG_MAP_SUPPORT */
#ifdef CONFIG_CPE_SUPPORT
		lppe_send_scan_complete_notification(pAd, wdev);
#endif
		/*
		If all peer Ad-hoc clients leave, driver would do LinkDown and LinkUp.
		In LinkUp, pStaCfg->Ssid would copy SSID from MlmeAux.
		To prevent SSID is zero or wrong in Beacon, need to recover MlmeAux.SSID here.
		*/
		if (ADHOC_ON(pAd)) {
			NdisZeroMemory(pStaCfg->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pStaCfg->MlmeAux.SsidLen = pStaCfg->SsidLen;
			NdisMoveMemory(pStaCfg->MlmeAux.Ssid, pStaCfg->Ssid, pStaCfg->SsidLen);
		}

		/*
		To prevent data lost.
		Send an NULL data with turned PSM bit on to current associated AP before SCAN progress.
		Now, we need to send an NULL data with turned PSM bit off to AP, when scan progress done
		*/
		if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pStaCfg))) {
			MAC_TABLE_ENTRY *pEntry = GetAssociatedAPByWdev(pAd, &pStaCfg->wdev);

			RTMPSendNullFrame(pAd,
							  pEntry,
							  pAd->CommonCfg.TxRate,
							  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE : FALSE),
							  pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pStaCfg->PwrMgmt.Psm);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "Send null frame\n");
		}
#ifdef APCLI_AUTO_CONNECT_SUPPORT

		if (pStaCfg && (pStaCfg->ApCliAutoConnectRunning == TRUE) &&
			(ScanCtrl->PartialScan.bScanning == FALSE)) {
			if (!ApCliAutoConnectExec(pAd, pwdev))
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Fail to switch channel for ApCliAutoConnect!!!\n");
		}

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef WSC_AP_SUPPORT

		if ((wdev->func_idx < MAX_APCLI_NUM) &&
#ifdef CON_WPS
			/* In case of concurrent WPS, the request might have come from a non APCLI interface. */
			((IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)) || (pAd->StaCfg[wdev->func_idx].wdev.WscControl.conWscStatus != CON_WPS_STATUS_DISABLED))
#else
			(IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA))
#endif
			) {
			WSC_CTRL *pWpsCtrlTemp = &pAd->StaCfg[wdev->func_idx].wdev.WscControl;

			if ((pWpsCtrlTemp->WscConfMode != WSC_DISABLE) &&
				(pWpsCtrlTemp->bWscTrigger == TRUE) &&
				(pWpsCtrlTemp->WscMode == WSC_PBC_MODE)) {
				if (pWpsCtrlTemp->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) {
					if ((ScanCtrl->PartialScan.bScanning == FALSE) &&
						(ScanCtrl->PartialScan.LastScanChannel == 0)) {
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
								 "%s AP-Client WPS Partial Scan done!!!\n",
								 (ch > 14 ? "5G" : "2G"));
#ifdef CONFIG_MAP_SUPPORT
						if (IS_MAP_ENABLE(pAd) && IS_MAP_TURNKEY_ENABLE(pAd)) {
							WscPBCBssTableSort(pAd, pWpsCtrlTemp);
							wapp_send_wsc_scan_complete_notification(pAd, pwdev);
						} else
#endif /* CONFIG_MAP_SUPPORT */
						{
							if (!pWpsCtrlTemp->WscPBCTimerRunning) {
								RTMPSetTimer(&pWpsCtrlTemp->WscPBCTimer, 1000);
								pWpsCtrlTemp->WscPBCTimerRunning = TRUE;
							}
						}
					}
				}

				else
				{
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_ENABLE(pAd) && IS_MAP_TURNKEY_ENABLE(pAd)) {
						WscPBCBssTableSort(pAd, pWpsCtrlTemp);
						wapp_send_wsc_scan_complete_notification(pAd, pwdev);
					}
#endif /* CONFIG_MAP_SUPPORT */
				}
			}
		}

#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */

		/* Suspend scanning and Resume TxData for Fast Scanning*/
		if ((ScanCtrl->Channel != 0) &&
			(ScanInfo->bImprovedScan)) {	/* it is scan pending*/
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "bFastRoamingScan ~~~ Get back to send data ~~~\n");
		}
	}

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP) {

#ifdef P2P_APCLI_SUPPORT

		/* P2P CLIENT in WSC Scan or Re-Connect scanning. */
		if (P2P_CLI_ON(pAd) && (ApScanRunning(pAd, wdev) == TRUE)) {
			/*MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, 0);*/
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG, "Scan Done! reset APCLI CTRL State Machine!\n");
			pAd->StaCfg[0].CtrlCurrState = APCLI_CTRL_DISCONNECTED;
		}

#endif /* P2P_APCLI_SUPPORT */

		/* iwpriv set auto channel selection*/
		/* scanned all channels*/
	}

#endif /* CONFIG_AP_SUPPORT */

#ifdef APCLI_CFG80211_SUPPORT
		RTEnqueueInternalCmd(pAd, CMDTHREAD_SCAN_END, NULL, 0);
#endif /* APCLI_CFG80211_SUPPORT */
#ifdef SCAN_RADAR_COEX_SUPPORT
	pAd->scan_wdev = NULL;
#endif
	return TRUE;
}

#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
INT ch_switch_monitor_scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev)
{
	UCHAR bw = 0;
	UCHAR ch = 0;
	struct wifi_dev *wdev = pwdev;
	struct DOT11_H *pDot11h = NULL;
	UCHAR target_band_idx = 0;
	UCHAR band_idx_per_wdev = 0;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"error! wdev is null!!!\n");
		return FALSE;
	}
	target_band_idx = HcGetBandByWdev(wdev);
	ch = wlan_operate_get_prim_ch(wdev);
	ASSERT((ch != 0));
	/*restore to original channel*/
	wlan_operate_set_prim_ch(wdev, ch);
	ch = wlan_operate_get_cen_ch_1(wdev);
	bw = wlan_operate_get_bw(wdev);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "Restore to BW(%d) channel %d\n", bw, ch);

#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP) {

		INT BssIdx;
		INT MaxNumBss = pAd->ApCfg.BssidNum;

		pDot11h = wdev->pDot11_H;

		if (pDot11h == NULL)
			return FALSE;

		if (((wdev->channel > 14) &&
			 (pAd->CommonCfg.bIEEE80211H == TRUE) &&
			 RadarChannelCheck(pAd, wdev->channel)) &&
			pDot11h->RDMode != RD_SWITCHING_MODE) {
			if (pDot11h->InServiceMonitorCount) {
				struct wifi_dev *apWdev = NULL;

				pDot11h->RDMode = RD_NORMAL_MODE;
				AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
					HW_BSSID_0, OPMODE_AP);

				/* Enable beacon tx for target BSS */
				for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
					apWdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
					if (!apWdev)
						continue;
					band_idx_per_wdev = HcGetBandByWdev(apWdev);
					if (band_idx_per_wdev != target_band_idx)
						continue;
					if (apWdev->bAllowBeaconing)
						UpdateBeaconHandler(pAd, apWdev, BCN_UPDATE_ENABLE_TX);
				}
			} else
				pDot11h->RDMode = RD_SILENCE_MODE;
		} else {
			struct wifi_dev *apWdev = NULL;

			AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
				HW_BSSID_0, OPMODE_AP);

			/* Enable beacon tx for target BSS */
			for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
				apWdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
				if (!apWdev)
					continue;
				band_idx_per_wdev = HcGetBandByWdev(apWdev);
				if (band_idx_per_wdev != target_band_idx)
					continue;
				if (apWdev->bAllowBeaconing)
					UpdateBeaconHandler(pAd, apWdev, BCN_UPDATE_ENABLE_TX);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}
#endif
static INT scan_active(RTMP_ADAPTER *pAd, UCHAR OpMode, UCHAR ScanType, struct wifi_dev *wdev)
{
	UCHAR *frm_buf = NULL;
	HEADER_802_11 Hdr80211;
	ULONG FrameLen = 0;
	UCHAR SsidLen = 0;
	struct _build_ie_info ie_info = {0};
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
#ifdef APCLI_SUPPORT
	PSTA_ADMIN_CONFIG apcli_entry = NULL;
	struct customer_vendor_ie *apcli_vendor_ie;
#endif /* APCLI_SUPPORT */
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	struct legacy_rate *rate;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (OpMode == OPMODE_STA) {
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return FALSE;
	}
#endif /* CONFIG_STA_SUPPORT */

	if (WMODE_CAP_6G(wdev->PhyMode) && wdev->wdev_type == WDEV_TYPE_STA
		&& ScanCtrl->SsidLen == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Disable active scan for 6G ApCli\n");
		return TRUE;
	}

	ie_info.frame_subtype = SUBTYPE_PROBE_REQ;
	ie_info.channel = ScanCtrl->Channel;
	ie_info.phy_mode = wdev->PhyMode;
	ie_info.wdev = wdev;

	if (MlmeAllocateMemory(pAd, &frm_buf) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "allocate memory fail\n");
		return FALSE;
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (ScanType == SCAN_2040_BSS_COEXIST)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG, "SYNC - SCAN_2040_BSS_COEXIST !! Prepare to send Probe Request\n");

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	/* There is no need to send broadcast probe request if active scan is in effect.*/
	SsidLen = 0;

	if ((ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE)
#ifdef WSC_STA_SUPPORT
		|| ((ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_STA))
#endif /* WSC_STA_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
		|| (ScanType == SCAN_P2P)
#endif /* RT_CFG80211_P2P_SUPPORT */
	   ) {
		SsidLen = ScanCtrl->SsidLen;
	}

	MgtMacHeaderInitExt(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
							wdev->if_addr, BROADCAST_ADDR);

#ifdef P2P_SUPPORT
	if ((ScanCtrl->ScanType == SCAN_P2P) || (ScanCtrl->ScanType == SCAN_P2P_SEARCH)
#ifdef P2P_APCLI_SUPPORT
		|| ((ScanCtrl->ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_AP) && (P2P_CLI_ON(pAd)))
#endif /* P2P_APCLI_SUPPORT */
	   ) {
		PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
		UCHAR		SupRate[MAX_LEN_OF_SUPPORTED_RATES];
		UCHAR		SupRateLen = 0;

		SsidLen = WILDP2PSSIDLEN; /* Use Wildword SSID */
		SupRate[0]	= 0x8C;    /* 6 mbps, in units of 0.5 Mbps, basic rate */
		SupRate[1]	= 0x12;    /* 9 mbps, in units of 0.5 Mbps */
		SupRate[2]	= 0x98;    /* 12 mbps, in units of 0.5 Mbps, basic rate */
		SupRate[3]	= 0x24;    /* 18 mbps, in units of 0.5 Mbps */
		SupRate[4]	= 0xb0;    /* 24 mbps, in units of 0.5 Mbps, basic rate */
		SupRate[5]	= 0x48;    /* 36 mbps, in units of 0.5 Mbps */
		SupRate[6]	= 0x60;    /* 48 mbps, in units of 0.5 Mbps */
		SupRate[7]	= 0x6c;    /* 54 mbps, in units of 0.5 Mbps */
		SupRateLen	= 8;
		/* P2P scan must use P2P mac address. */
		MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
						 pP2PCtrl->CurrentAddress,
						 BROADCAST_ADDR);
		MakeOutgoingFrame(frm_buf,				&FrameLen,
						  sizeof(HEADER_802_11),	&Hdr80211,
						  1,						&SsidIe,
						  1,						&SsidLen,
						  SsidLen,					&WILDP2PSSID[0],
						  END_OF_ARGS);
		FrameLen += build_support_rate_ie(wdev, SupRate, SupRateLen, frm_buf + FrameLen);
	} else
#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
		if (ScanType == SCAN_P2P) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "this is a p2p scan from cfg80211 layer\n");
			MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
							 pAd->CurrentAddress, BROADCAST_ADDR);
			MakeOutgoingFrame(frm_buf,               &FrameLen,
							  sizeof(HEADER_802_11),    &Hdr80211,
							  1,                        &SsidIe,
							  1,                        &SsidLen,
							  SsidLen,                  ScanCtrl->Ssid,
							  END_OF_ARGS);
			FrameLen += build_support_rate_ie(wdev, pAd->cfg80211_ctrl.P2pSupRate,
						pAd->cfg80211_ctrl.P2pSupRateLen, frm_buf + FrameLen);
		} else
#endif /* RT_CFG80211_P2P_SUPPORT */
		{
			rate = &wdev->rate.legacy_rate;
			MakeOutgoingFrame(frm_buf,               &FrameLen,
							  sizeof(HEADER_802_11),    &Hdr80211,
							  1,                        &SsidIe,
							  1,                        &SsidLen,
							  SsidLen,			        ScanCtrl->Ssid,
							  END_OF_ARGS);
			FrameLen += build_support_rate_ie(wdev, rate->sup_rate, rate->sup_rate_len, frm_buf + FrameLen);

			FrameLen += build_support_ext_rate_ie(wdev, rate->sup_rate_len,
					rate->ext_rate, rate->ext_rate_len, frm_buf + FrameLen);
		}

#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode)) {
		ie_info.frame_buf = (UCHAR *)(frm_buf + FrameLen);
		ie_info.is_draft_n_type = FALSE;
		FrameLen += build_ht_ies(pAd, &ie_info);

		if (pAd->bBroadComHT == TRUE) {
			ie_info.is_draft_n_type = TRUE;
			ie_info.frame_buf = (UCHAR *)(frm_buf + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);
		}

#ifdef DOT11_VHT_AC
		ie_info.frame_buf = (UCHAR *)(frm_buf + FrameLen);
		FrameLen += build_vht_ies(pAd, &ie_info);
#endif /* DOT11_VHT_AC */
	}

#endif /* DOT11_N_SUPPORT */
#ifdef WSC_INCLUDED
	ie_info.frame_buf = (UCHAR *)(frm_buf + FrameLen);
	FrameLen += build_wsc_ie(pAd, &ie_info);
#endif /* WSC_INCLUDED */
	ie_info.frame_buf = (UCHAR *)(frm_buf + FrameLen);
	FrameLen +=  build_extra_ie(pAd, &ie_info);

#ifdef P2P_SUPPORT
	if ((ScanCtrl->ScanType == SCAN_P2P) || (ScanCtrl->ScanType == SCAN_P2P_SEARCH)
#ifdef P2P_APCLI_SUPPORT
		|| ((ScanCtrl->ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_AP) && (P2P_CLI_ON(pAd)))
#endif /* P2P_APCLI_SUPPORT */
	   ) {
		ULONG P2PIeLen;
		UCHAR tmp_len;
		PUCHAR ptr;

		ptr = frm_buf + FrameLen;
		P2pMakeProbeReqIE(pAd, ptr, &tmp_len);
		FrameLen += tmp_len;
		/* Put P2P IE to the last. */
		ptr = frm_buf + FrameLen;
		P2pMakeP2pIE(pAd, SUBTYPE_PROBE_REQ, ptr, &P2PIeLen);
		FrameLen += P2PIeLen;
#ifdef WFD_SUPPORT
		ptr = frm_buf + FrameLen;
		WfdMakeWfdIE(pAd, SUBTYPE_PROBE_REQ, ptr, &P2PIeLen);
		FrameLen += P2PIeLen;
#endif /* WFD_SUPPORT */
	}

#endif /* P2P_SUPPORT */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef CONFIG_STA_SUPPORT
		apcli_entry = GetStaCfgByWdev(pAd, wdev);
#endif
		if (apcli_entry) {
			apcli_vendor_ie = &apcli_entry->apcli_vendor_ie;

			RTMP_SPIN_LOCK(&apcli_vendor_ie->vendor_ie_lock);
			if (apcli_vendor_ie->pointer != NULL) {
				ULONG TmpLen;

				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"[scan_active] add apcli vendor ie.\n");
				MakeOutgoingFrame(frm_buf + FrameLen,
						&TmpLen,
						apcli_vendor_ie->length,
						apcli_vendor_ie->pointer,
						END_OF_ARGS);
				FrameLen += TmpLen;
			}
			RTMP_SPIN_UNLOCK(&apcli_vendor_ie->vendor_ie_lock);
		}
#endif /* APCLI_SUPPORT */
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	MiniportMMRequest(pAd, 0, frm_buf, FrameLen);
#ifdef MT_MAC_BTCOEX

	if (pAd->BtCoexMode == MT7636_COEX_MODE_TDD)
		MiniportMMRequest(pAd, 0, frm_buf, FrameLen);

#endif
#ifdef CONFIG_STA_SUPPORT

	if (OpMode == OPMODE_STA) {
		/*
			To prevent data lost.
			Send an NULL data with turned PSM bit on to current associated AP when SCAN in the channel where
			associated AP located.
		*/
		if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED) &&
			(INFRA_ON(pStaCfg)) &&
			(pStaCfg->wdev.channel == ScanCtrl->Channel)) {

			MAC_TABLE_ENTRY *pEntry = GetAssociatedAPByWdev(pAd, &pStaCfg->wdev);

			RTMPSendNullFrame(pAd,
							  pEntry,
							  pAd->CommonCfg.TxRate,
							  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE : FALSE),
							  PWR_SAVE);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "Send PWA NullData frame to notify the associated AP!\n");
		}
	}

#endif /* CONFIG_STA_SUPPORT */
	MlmeFreeMemory(frm_buf);
	return TRUE;
}

static BOOLEAN scan_type_stay_time_checker(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT UCHAR *ScanType)
{
	UINT ScanTimeIn5gChannel = SHORT_CHANNEL_TIME;
	UINT stay_time = 0;
	BOOLEAN bScanPassive = FALSE;
	RALINK_TIMER_STRUCT *sc_timer = NULL;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
	UCHAR band_idx = BAND0;
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	band_idx = HcGetBandByWdev(wdev);
	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, band_idx);
	bScanPassive = scan_active_probe_disallowed(pAd, ScanCtrl->Channel);

	if (bScanPassive) {
		*ScanType = SCAN_PASSIVE;

		if (ScanCtrl->Channel > 14)
			ScanTimeIn5gChannel = MIN_CHANNEL_TIME;
	}

	sc_timer = &ScanCtrl->ScanTimer;

	if (!sc_timer) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "ScanTimer not assigned!\n");
		return FALSE;
	}

	if (ScanCtrl->Usr_dwell.isActive) {
		stay_time = (ScanCtrl->Channel > 14) ? (ScanCtrl->Usr_dwell.dwell_t_5g) : (ScanCtrl->Usr_dwell.dwell_t_2g);
		if (stay_time)
			goto set_stay_time;
	}
	/* We need to shorten active scan time in order for WZC connect issue */
	/* Chnage the channel scan time for CISCO stuff based on its IAPP announcement */
	if (*ScanType == FAST_SCAN_ACTIVE)
		stay_time = FAST_ACTIVE_SCAN_TIME;
	else { /* must be SCAN_PASSIVE or SCAN_ACTIVE*/
#ifdef CONFIG_AP_SUPPORT
		if ((pAd->ApCfg.bAutoChannelAtBootup[band_idx] == TRUE))
			stay_time = AUTO_CHANNEL_SEL_TIMEOUT;
		else
#endif /* CONFIG_AP_SUPPORT */
			if (WMODE_CAP_2G(wdev->PhyMode) &&
				WMODE_CAP_5G(wdev->PhyMode)) {
				if (ScanCtrl->Channel > 14)
					stay_time = ScanTimeIn5gChannel;
				else
					stay_time = MIN_CHANNEL_TIME;

				} else {
						stay_time = MAX_CHANNEL_TIME;
				}
	}

#ifdef CONFIG_STA_SUPPORT
#ifdef RT_CFG80211_SUPPORT
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#ifdef CONFIG_MULTI_CHANNEL

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
		stay_time = FAST_ACTIVE_SCAN_TIME;

#endif /* CONFIG_MULTI_CHANNEL */
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

	/* CFG_TODO: for testing. */
	/* Since the Channel List is from Upper layer */
	if (CFG80211DRV_OpsScanRunning(pAd) &&
		(pAd->cfg80211_ctrl.Cfg80211ChanListLen == 1) &&
		(wdev->wdev_type == WDEV_TYPE_P2P_DEVICE))
		stay_time = 500;

#endif /* RT_CFG80211_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef OFFCHANNEL_SCAN_FEATURE
	if (ScanCtrl->state == OFFCHANNEL_SCAN_START) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"offchannel scan setting stay time current state = %d\n", ScanCtrl->state);
		stay_time = ScanCtrl->ScanTime[ScanCtrl->CurrentGivenChan_Index];
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"stay time configured of channel index = %d time = %d\n",
				ScanCtrl->CurrentGivenChan_Index, stay_time);
	}
#endif
#endif
#ifdef OFFCHANNEL_SCAN_FEATURE
	ScanCtrl->OffChScan = TRUE;
	ScanCtrl->OffChScan_Ongoing = TRUE;

	if (ScanCtrl->state == OFFCHANNEL_SCAN_START) {
#ifdef PROPRIETARY_DRIVER_SUPPORT
		struct timespec64 kts64 = {0};
		ktime_t kts;
#endif
		RTMP_MIB_PAIR Reg[1];
#ifdef OFFCHANNEL_SCAN_FEATURE
		if (bScanPassive)
			*ScanType = ScanCtrl->Offchan_Scan_Type[ScanCtrl->CurrentGivenChan_Index];
#endif
		NdisZeroMemory(Reg, sizeof(Reg));
		Reg[0].Counter = MIB_CNT_P_CCA_TIME;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdMib(pAd, band_idx, Reg, 1);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdMultipleMibRegAccessRead(pAd, band_idx, Reg, 1);
		pAutoChCtrl->AutoChSelCtrl.pre_pcca_time = Reg[0].Value;

#ifdef PROPRIETARY_DRIVER_SUPPORT
		ktime_get_real_ts64(&kts64);
		kts = timespec64_to_ktime(kts64);
		ScanCtrl->ScanTimeActualStart = kts;
#else
		ScanCtrl->ScanTimeActualStart = ktime_get();
#endif
#ifdef OFFCHANNEL_ZERO_LOSS
		/*read channel stats initial reading*/
#ifdef PROPRIETARY_DRIVER_SUPPORT
		pAd->ScanCtrl[band_idx].ScanTimeStartMibEvent = kts;
#else
		pAd->ScanCtrl[band_idx].ScanTimeStartMibEvent = ktime_get();
#endif
		read_channel_stats(pAd, band_idx, &(ScanCtrl->OffCannelScanStartStats));
#endif
		pAutoChCtrl->AutoChSelCtrl.pre_cca_nav_tx_time = AsicGetCCACnt(pAd, band_idx);

		/*enable getting NF, read CR per 20ms, 2 counts, about 40ms*/
		EnableNF(pAd, 1, 20, 2, 0);
	}
#endif
set_stay_time:
	RTMPSetTimer(sc_timer, stay_time);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "SYNC - stay_time:%d\n", stay_time);
	return TRUE;

}

#if defined(CONFIG_STA_SUPPORT)
void scan_extra_probe_req(RTMP_ADAPTER *pAd, UCHAR OpMode, UCHAR ScanType,
							  struct wifi_dev *wdev,  UCHAR *desSsid, UCHAR desSsidLen)
{
	UCHAR backSsid[MAX_LEN_OF_SSID];
	UCHAR backSsidLen = 0;
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	NdisZeroMemory(backSsid, MAX_LEN_OF_SSID);
	/* 1. backup the original MlmeAux */
	backSsidLen = ScanCtrl->SsidLen;
	NdisCopyMemory(backSsid, ScanCtrl->Ssid, backSsidLen);
	/* 2. fill the desried ssid into SM */
	ScanCtrl->SsidLen = desSsidLen;
	NdisCopyMemory(ScanCtrl->Ssid, desSsid, desSsidLen);
	/* 3. scan action */
	scan_active(pAd, OpMode, ScanType, wdev);
	/* 4. restore to ScanCtrl */
	ScanCtrl->SsidLen  = backSsidLen;
	NdisCopyMemory(ScanCtrl->Ssid, backSsid, backSsidLen);
}
#endif /* CONFIG_STA_SUPPORT */


/* PUBLIC */
BOOLEAN scan_active_probe_disallowed(RTMP_ADAPTER *pAd, UCHAR channel)
{
	BOOLEAN bScanPassive = FALSE;

	if (channel > 14) {
		if ((pAd->CommonCfg.bIEEE80211H == 1) &&
			RadarChannelCheck(pAd, channel))
			bScanPassive = TRUE;

	}

#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		bScanPassive = TRUE;

#endif /* CARRIER_DETECTION_SUPPORT */

	/* Check if channel if passive scan under current regulatory domain */
	if (CHAN_PropertyCheck(pAd, channel, CHANNEL_PASSIVE_SCAN) == TRUE)
		bScanPassive = TRUE;

#if defined(WIFI_REGION32_HIDDEN_SSID_SUPPORT)

	/* Ch 12~14 is passive scan, No matter DFS and 80211H setting is y or n */
	if ((channel >= 12) && (channel <= 14))
		bScanPassive = TRUE;

#endif /* WIFI_REGION32_HIDDEN_SSID_SUPPORT */
	return bScanPassive;
}

/*
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */
BOOLEAN scan_next_channel(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT SCAN_ACTION_INFO * scan_action_info)
{
	UCHAR ScanType = SCAN_TYPE_MAX;
	BOOLEAN ScanPending = FALSE;
#ifdef APCLI_SUPPORT
#ifdef CONFIG_MAP_SUPPORT
	int index_map = 0;
#endif
#endif
#ifdef WIDI_SUPPORT
	static int count;
#endif /* WIDI_SUPPORT */
#ifdef OFFCHANNEL_SCAN_FEATURE
	OFFCHANNEL_SCAN_MSG Rsp;
#ifdef OFFCHANNEL_ZERO_LOSS
	UINT8 BandIdx;
#endif
#endif
	SCAN_CTRL *ScanCtrl = NULL;
	SCAN_INFO *ScanInfo = NULL;
	UCHAR OpMode = 0;
/* TODO: Star, fix me when Scan is prepare to modify */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif
	if (wdev != NULL) {
		ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
		ScanInfo = &wdev->ScanInfo;
		OpMode = wdev_get_op_mode(wdev);
#ifdef CONFIG_STA_SUPPORT
		if (OpMode == OPMODE_STA) {
			/* TODO: Star, fix me when Scan is prepare to modify */
			pStaCfg = GetStaCfgByWdev(pAd, wdev);
		}
#endif
#ifdef APCLI_CFG80211_SUPPORT
			if (pStaCfg != NULL && pStaCfg->MarkToClose) {
				ScanCtrl->Channel = 0;
				scan_ch_restore(pAd, OpMode, wdev);
				RTMP_OS_COMPLETE(&pStaCfg->scan_complete);
				return FALSE;
			}
#endif /* APCLI_CFG80211_SUPPORT */

	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
			ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
			ScanInfo = &wdev->ScanInfo;
		}
#endif
	}
#ifdef SCAN_RADAR_COEX_SUPPORT
	if (wdev != NULL && wdev->RadarDetected) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "%s: Stopping Scan!\n", __func__);
		ScanCtrl->Channel = 0;
		scan_ch_restore(pAd, OpMode, wdev);
#ifdef APCLI_CFG80211_SUPPORT
		if (pAd->cfg80211_ctrl.FlgCfg80211Scanning)
			RT_CFG80211_SCAN_END(pAd, TRUE);
#endif /* APCLI_CFG80211_SUPPORT */
		return FALSE;
	}
#endif /* SCAN_RADAR_COEX_SUPPORT */

	if (wdev != NULL && wdev->ch_set_in_progress) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "%s: Stopping Scan as channel set pending!\n", __func__);
		ScanCtrl->Channel = 0;
		scan_ch_restore(pAd, OpMode, wdev);
#ifdef APCLI_CFG80211_SUPPORT
		if (pAd->cfg80211_ctrl.FlgCfg80211Scanning)
			RT_CFG80211_SCAN_END(pAd, TRUE);
#endif /* APCLI_CFG80211_SUPPORT */

		ScanCtrl->PartialScan.bScanning = FALSE;
		return FALSE;
	}

#ifdef CONFIG_STA_SUPPORT
	if (OpMode == OPMODE_STA) { /* snowpin for ap/sta ++ */
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return FALSE;
	}
#endif



#ifdef CONFIG_ATE

	/* Nothing to do in ATE mode. */
	if (ATE_ON(pAd))
		return FALSE;

#endif /* CONFIG_ATE */
#ifdef WIDI_SUPPORT
#ifdef CONFIG_STA_SUPPORT

	if ((pStaCfg->bWIDI && (pAd->StaCfg[0].bSendingProbe == TRUE))
#ifdef P2P_SUPPORT
		|| (pAd->P2pCfg.bWIDI && (pAd->gP2pSendingProbeResponse == 1))
#endif /* P2P_SUPPORT */
	   ) {
		RTMPSetTimer(&pStaCfg->MlmeAux.ScanTimer, MAX_CHANNEL_TIME);
		return TRUE;
	}

#endif /* CONFIG_STA_SUPPORT */
#endif /* WIDI_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (MONITOR_ON(pAd))
			return FALSE;
	}
#endif /* CONFIG_STA_SUPPORT */

	scan_action_info->isScanDone = FALSE;
	ScanType = ScanCtrl->ScanType;

	if (ScanType == SCAN_TYPE_MAX) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "Incorrect ScanType!\n");
		return FALSE;
	}

	if (SCAN_MODE_SEG(ScanType)) {
		if (ScanCtrl->Channel == 0)
			ScanPending = FALSE;
		else if (ScanType == SCAN_IMPROVED)
			ScanPending = ((ScanInfo->bImprovedScan) && (ScanInfo->ScanChannelCnt >= IMPROVED_SCAN_CHANNEL_COUNT));
		else if (ScanType == SCAN_PARTIAL)
			ScanPending = (ScanCtrl->PartialScan.bScanning && (ScanInfo->ScanChannelCnt >= ScanCtrl->PartialScan.NumOfChannels));
	}

#ifdef CONFIG_STA_SUPPORT
#ifdef RT_CFG80211_SUPPORT

	/* Since the Channel List is from Upper layer */
	if (CFG80211DRV_OpsScanRunning(pAd) && !ScanPending) {
		int ChannelFound = 0;
		while (!ChannelFound) {
		ScanCtrl->Channel = CFG80211DRV_OpsScanGetNextChannel(pAd);
			if (ScanCtrl->Channel == 0)
				break;
			if ((ScanCtrl->Channel) > 14 && (!WMODE_CAP_5G(wdev->PhyMode)))
				continue;
			else if ((ScanCtrl->Channel) <= 14 && (!WMODE_CAP_2G(wdev->PhyMode)))
				continue;
			else
				ChannelFound = 1;
		}
	}

#endif /* RT_CFG80211_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	if ((ScanCtrl->Channel == 0) || ScanPending) {
#ifdef WIDI_SUPPORT
		count++;

		if (count > 10) {
			count = 0;

			if (ScanType != SCAN_PASSIVE)
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "Driver is Alive; ScanType %d\n", ScanType);
		}

#endif /* WIDI_SUPPORT */
		scan_action_info->isScanPending = ScanPending;
		scan_action_info->isScanDone = TRUE;
		if (scan_ch_restore(pAd, OpMode, wdev) == FALSE)
			return FALSE;

#ifdef OFFCHANNEL_SCAN_FEATURE
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "Scan complete for band %d channel:%d pending:%d \n",
							pAd->ChannelInfo.bandidx, ScanCtrl->Channel, ScanPending);
			ScanCtrl->OffChScan_Ongoing = FALSE;

		if (ScanCtrl->state == OFFCHANNEL_SCAN_COMPLETE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"in finish path channel no : %d : obss time :%d channel_idx = %d\n",
					pAd->ChannelInfo.ChannelNo, pAd->ChannelInfo.ChStats.Obss_Time, pAd->ApCfg.current_channel_index);
			Rsp.Action = OFFCHANNEL_INFO_RSP;
			memcpy(Rsp.ifrn_name, ScanCtrl->if_name, IFNAMSIZ);
			Rsp.data.channel_data.channel_busy_time = pAd->ChannelInfo.chanbusytime[pAd->ApCfg.current_channel_index];
			Rsp.data.channel_data.NF = pAd->ChannelInfo.AvgNF ;
			Rsp.data.channel_data.channel = pAd->ChannelInfo.ChannelNo;
#ifdef OFFCHANNEL_ZERO_LOSS
			BandIdx = HcGetBandByWdev(wdev);
			Rsp.data.channel_data.tx_time = pAd->ScanChnlStats[BandIdx].Tx_Time;
			Rsp.data.channel_data.rx_time = pAd->ScanChnlStats[BandIdx].Rx_Time;
			if (pAd->ScanChnlStats[BandIdx].Obss_Time >= pAd->ScanCtrl[BandIdx].ScanTimeDiff * 1000)
				Rsp.data.channel_data.obss_time = (pAd->ScanCtrl[BandIdx].ScanTimeDiff * 1000);
			else
				Rsp.data.channel_data.obss_time = pAd->ScanChnlStats[BandIdx].Obss_Time;
			Rsp.data.channel_data.actual_measured_time = pAd->ScanCtrl[BandIdx].ScanTimeDiff;
#else
			Rsp.data.channel_data.tx_time = pAd->ChannelInfo.ChStats.Tx_Time;
			Rsp.data.channel_data.rx_time = pAd->ChannelInfo.ChStats.Rx_Time;
			Rsp.data.channel_data.obss_time = pAd->ChannelInfo.ChStats.Obss_Time;
			/* This value to be used by application to calculate  channel busy percentage */
			Rsp.data.channel_data.actual_measured_time = ScanCtrl->ScanTimeActualDiff;
#endif
			Rsp.data.channel_data.channel_idx = pAd->ApCfg.current_channel_index;

#ifdef MAP_R2
			if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd)) {
				asic_update_mib_bucket(pAd);
				Update_Mib_Bucket_for_map(pAd);
				/* Rsp.data.channel_data.edcca =
				*	(Get_EDCCA_Time(pAd, ScanCtrl->BandIdx)*255)/ONE_SEC_2_US;
				*/
				Rsp.data.channel_data.edcca = pAd->OneSecMibBucket.EDCCAtime[ScanCtrl->BandIdx];
			}
#endif

			RtmpOSWrielessEventSend(
					pAd->net_dev,
					RT_WLAN_EVENT_CUSTOM,
					OID_OFFCHANNEL_INFO,
					NULL,
					(UCHAR *) &Rsp,
					sizeof(OFFCHANNEL_SCAN_MSG));
			pAd->MsMibBucket.Enabled = TRUE;
			ScanCtrl->ScanTime[ScanCtrl->CurrentGivenChan_Index] = 0;
			ScanCtrl->state = OFFCHANNEL_SCAN_INVALID;
			ScanCtrl->Num_Of_Channels = 0;
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_ENABLE(pAd))
				memset(ScanCtrl->ScanGivenChannel, 0, MAX_AWAY_CHANNEL);
			else
#endif
				ScanCtrl->ScanGivenChannel[ScanCtrl->CurrentGivenChan_Index] = 0;
			ScanCtrl->CurrentGivenChan_Index = 0;
		}
#endif

		if (ScanPending == FALSE) {
			ScanInfo->LastScanChannel = 0;

			if (ScanType == SCAN_PARTIAL) {
				if (ScanCtrl->PartialScan.TimerInterval > 0) {
					ScanCtrl->PartialScan.bScanning = FALSE;
					RTMPSetTimer(&ScanCtrl->PartialScan.PartialScanTimer,
									ScanCtrl->PartialScan.TimerInterval);
				} else {
					ScanCtrl->PartialScan.bScanning = FALSE;
					ScanCtrl->PartialScan.pwdev = NULL;
					ScanCtrl->ScanType = SCAN_ACTIVE;
				}
			}
		}

	}

	else {
#ifdef CONFIG_STA_SUPPORT

		/* keep the latest scan channel, could be 0 for scan complete, or other channel*/
		ScanInfo->LastScanChannel = ScanCtrl->Channel;

		if (OpMode == OPMODE_STA) {
			/*check if wdev is mix mode and channel will go to other band, switch wdev to difference band*/
			HcCrossChannelCheck(pAd,  wdev, ScanCtrl->Channel);
		}

#endif /* CONFIG_STA_SUPPORT */
		{
#ifdef OFFCHANNEL_SCAN_FEATURE
			/* For OffChannel scan command dont change the BW */
			if (ScanCtrl->state == OFFCHANNEL_SCAN_START) {
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_ENABLE(pAd) && ScanCtrl->Off_Ch_Scan_BW != BW_20_SCAN) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
						"Performing Scan Without changing BW\n");
					wdev->restore_channel = wlan_operate_get_prim_ch(wdev);
					wlan_operate_set_prim_ch(wdev, ScanCtrl->Channel);
				} else
#endif
				{
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
						"Performing Scan in 20 Mhz\n");
					if (ScanCtrl->CurrentGivenChan_Index == 0)
						wdev->restore_channel = wlan_operate_get_prim_ch(wdev);
					if (wlan_operate_scan(wdev, ScanCtrl->Channel) != TRUE)
						return FALSE;
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
						"Performing Scan in 20 Mhz\n");
				if (wlan_operate_scan(wdev, ScanCtrl->Channel) != TRUE)
					return FALSE;
			}
#else
			if (wlan_operate_scan(wdev, ScanCtrl->Channel) != TRUE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					 "Radio Resource not available on ch%d\n", ScanCtrl->Channel);
				return FALSE;
		}
#endif
		}

		if (scan_type_stay_time_checker(pAd, wdev, &ScanType) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "Incorrect ScanType!\n");
			return FALSE;
		}

#ifdef APCLI_SUPPORT
#ifdef CONFIG_MAP_SUPPORT
		wdev->MAPCfg.FireProbe_on_DFS = FALSE;
		if ((IS_MAP_TURNKEY_ENABLE(pAd)) &&
			(!((pAd->CommonCfg.bIEEE80211H == 1) &&
				RadarChannelCheck(pAd, ScanCtrl->Channel)))) {
			while (index_map < MAX_BH_PROFILE_CNT) {
				if (wdev->MAPCfg.scan_bh_ssids.scan_SSID_val[index_map].SsidLen > 0) {
				scan_extra_probe_req(pAd,	OpMode, SCAN_ACTIVE, wdev,
						wdev->MAPCfg.scan_bh_ssids.scan_SSID_val[index_map].ssid,
						 wdev->MAPCfg.scan_bh_ssids.scan_SSID_val[index_map].SsidLen);
				}
				index_map++;
			}
		}
#endif
#endif
		ScanInfo->ScanChannelCnt++;

		if (SCAN_MODE_ACT(ScanType)) {
			if (scan_active(pAd, OpMode, ScanType, wdev) == FALSE) {
				return FALSE;
			}

			/*
				Active scan with specific SSID to find hidden AP.
			*/
#ifdef APCLI_SUPPORT
			if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)) {
				PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[wdev->func_idx];

				if (pApCliEntry->CfgSsidLen > 0) {
					BOOLEAN needUnicastScan;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
					needUnicastScan = pApCliEntry->ApCliAutoConnectRunning;
#else
					needUnicastScan = ScanCtrl->PartialScan.bScanning;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
					if (needUnicastScan)
						scan_extra_probe_req(pAd,  OpMode, ScanType, wdev,
										  pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen);
				}
			}
#endif /* APCLI_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef CONFIG_MAP_SUPPORT

			if ((wdev->wdev_type == WDEV_TYPE_AP)) {
				int i = 0, bh_index = 0;
				PSTA_ADMIN_CONFIG pApCliEntry = NULL;
				struct wifi_dev *wdev_temp = NULL;

				for (i = 0; i < MAX_APCLI_NUM; i++) {

					pApCliEntry = &pAd->StaCfg[i];
					wdev_temp = &pApCliEntry->wdev;

					if (wdev->channel == wdev_temp->channel) {

						while (bh_index < MAX_BH_PROFILE_CNT) {
							if (wdev_temp->MAPCfg.scan_bh_ssids.scan_SSID_val[bh_index].SsidLen > 0) {
								scan_extra_probe_req(pAd,	OpMode, SCAN_ACTIVE, wdev,
										wdev_temp->MAPCfg.scan_bh_ssids.scan_SSID_val[bh_index].ssid,
										 wdev_temp->MAPCfg.scan_bh_ssids.scan_SSID_val[bh_index].SsidLen);
							}
							bh_index++;
						}
					}
				}
			}
#endif
#endif


#ifdef CONFIG_STA_SUPPORT
			if ((wdev->wdev_type == WDEV_TYPE_STA) &&
				(ScanType == SCAN_ACTIVE) &&
				(ScanCtrl->SsidLen > 0)) {
				/* Enhance Connectivity when Hidden Ssid Scanning */
				CHAR desiredSsid[MAX_LEN_OF_SSID] = {0};
				UCHAR desiredSsidLen = 0;

				scan_extra_probe_req(pAd,  OpMode, ScanType, wdev,
									 desiredSsid, desiredSsidLen);
			}

#endif /* CONFIG_STA_SUPPORT */
		}

		/* For SCAN_CISCO_PASSIVE, do nothing and silently wait for beacon or other probe reponse*/
}

	return TRUE;
}

BOOLEAN scan_in_run_state(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN	isScanOn = FALSE;
	UCHAR BandIdx = 0;
	SCAN_CTRL *ScanCtrl = NULL;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
	struct ch_switch_cfg *ch_sw_info = NULL;
#endif

	if (wdev) {
		ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
		BandIdx = HcGetBandByWdev(wdev);
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
		ch_sw_info = &pAd->ch_sw_cfg[BandIdx];
#endif

		if (((ScanCtrl->SyncFsm.CurrState != SYNC_FSM_IDLE) &&
			(ScanCtrl->SyncFsm.CurrState != SYNC_FSM_PENDING)) ||
			(pAutoChCtrl && (pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState == AUTO_CH_SEL_SCAN_LISTEN))
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
			|| ((ch_sw_info) && (ch_sw_info->ch_sw_on_going == TRUE))
#endif
			)
			isScanOn = TRUE;
	} else {
		/* YF: wdev null means check all of band */
		for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
			ScanCtrl = &pAd->ScanCtrl[BandIdx];
			pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
			ch_sw_info = &pAd->ch_sw_cfg[BandIdx];
#endif

			if (((ScanCtrl->SyncFsm.CurrState != SYNC_FSM_IDLE) &&
				(ScanCtrl->SyncFsm.CurrState != SYNC_FSM_PENDING)) ||
				(pAutoChCtrl && (pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState == AUTO_CH_SEL_SCAN_LISTEN))
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
				|| ((ch_sw_info) && (ch_sw_info->ch_sw_on_going == TRUE))
#endif
			)
				isScanOn |= TRUE;
		}
	}

	return isScanOn;
}

/*
	==========================================================================
	Description:

	Return:
		scan_channel - channel to scan.
	Note:
		return 0 if no more next channel
	==========================================================================
 */
UCHAR scan_find_next_channel(
	RTMP_ADAPTER *pAd,
	SCAN_CTRL *ScanCtrl,
	UINT8 LastScanChannel)
{
	UCHAR scan_channel = 0;
	struct wifi_dev *wdev;
	SCAN_INFO *ScanInfo;
	UCHAR ScanType;
	int i;

	ASSERT(ScanCtrl->ScanReqwdev);

	if (!ScanCtrl->ScanReqwdev)
		return scan_channel;

	wdev = ScanCtrl->ScanReqwdev;
	ScanInfo = &wdev->ScanInfo;
	ScanType = ScanCtrl->ScanType;

	if ((ScanInfo->bImprovedScan) &&
		(ScanCtrl->SyncFsm.CurrState == SYNC_FSM_PENDING))
		LastScanChannel = ScanInfo->LastScanChannel;

	if (ScanInfo->bFastConnect) {
		if (!ScanInfo->bNotFirstScan) {
			ScanInfo->bNotFirstScan = TRUE;
			return 0;
		}

		if ((wdev->channel != 0) && !ScanInfo->bNotFirstScan)
			return wdev->channel;
	}

	/* Only one channel scanned for CISCO beacon request */
	if ((ScanType == SCAN_CISCO_ACTIVE) ||
		(ScanType == SCAN_CISCO_PASSIVE) ||
		(ScanType == SCAN_CISCO_NOISE) ||
		(ScanType == SCAN_CISCO_CHANNEL_LOAD))
		return 0;

	if ((ScanCtrl->PartialScan.bScanning == TRUE) &&
		(ScanCtrl->SyncFsm.CurrState == SYNC_FSM_PENDING))
		LastScanChannel = ScanInfo->LastScanChannel;

find_next_channel:

	if (LastScanChannel == 0)
		scan_channel = FirstChannel(pAd, wdev);
	else
		scan_channel = NextChannel(pAd, ScanCtrl, LastScanChannel, wdev);

	if (scan_channel == 0)
		return scan_channel;

	for (i = 0; i < ScanCtrl->SkipCh_Num; i++) {
		if (scan_channel == ScanCtrl->SkipList[i].Channel) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
						"Skip channel %d\n", scan_channel);
			LastScanChannel = scan_channel;
			goto find_next_channel;
		}
	}

	if (!ScanCtrl->dfs_ch_utilization)
		if ((scan_channel >= 52 && scan_channel <= 112) ||
			((scan_channel >= 136 && scan_channel <= 144))) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
						"Skip DFS channel %d\n", scan_channel);
			LastScanChannel = scan_channel;
			goto find_next_channel;
		}

	if ((WMODE_CAP_6G(ScanCtrl->ScanReqwdev->PhyMode) && (scan_channel <= CHANNEL_6G_MAX)) ||
		(WMODE_CAP_5G(ScanCtrl->ScanReqwdev->PhyMode) && (scan_channel > 14)) ||
		(WMODE_CAP_2G(ScanCtrl->ScanReqwdev->PhyMode) && (scan_channel <= 14))) {
	} else {
		LastScanChannel = scan_channel;
		goto find_next_channel;
	}

	return scan_channel;
}

VOID scan_partial_trigger_checker(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev = NULL;
	SCAN_INFO *ScanInfo = NULL;
	SCAN_CTRL *ScanCtrl = NULL;
	UINT ScanType = SCAN_TYPE_MAX;
	UCHAR BandIdx = 0;

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		ScanCtrl = &pAd->ScanCtrl[BandIdx];
		wdev = ScanCtrl->ImprovedScanWdev;

		/* resume Improved Scanning*/
		if (wdev && (wdev->ScanInfo.bImprovedScan) &&
			(ScanCtrl->SyncFsm.CurrState == SYNC_FSM_PENDING) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))) {
			MLME_SCAN_REQ_STRUCT ScanReq;

			NdisZeroMemory(&ScanReq, sizeof(MLME_SCAN_REQ_STRUCT));
			ScanParmFill(pAd, &ScanReq, ScanCtrl->Ssid, ScanCtrl->SsidLen, BSS_ANY, SCAN_IMPROVED);
			if (MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_SCAN_REQ,
					sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0, wdev))
				RTMP_MLME_HANDLER(pAd);
			else {
				//TODO: error handler
			}
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
					 "bImprovedScan ............. Resume for bImprovedScan, SCAN_PENDING ..............\n");
			continue;
		}

		/* (pAd->ScanCtrl.PartialScan.NumOfChannels == DEFLAUT_PARTIAL_SCAN_CH_NUM)
		    means that one partial scan is finished
		 */
		if (ScanCtrl->PartialScan.bScanning == TRUE) {
			wdev = ScanCtrl->PartialScan.pwdev;

			if (!wdev)
				continue;

#ifdef SCAN_RADAR_COEX_SUPPORT
			if (wdev != NULL && wdev->RadarDetected) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE, "%s: Stopping Scan!\n", __func__);
				ScanCtrl->Channel = 0;
				ScanCtrl->PartialScan.bScanning = FALSE;
				if (GetCurrentChannelOpOwner(pAd, wdev) == CH_OP_OWNER_PARTIAL_SCAN)
					ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_PARTIAL_SCAN);
				RTMP_OS_COMPLETE(&wdev->scan_complete);
				return;
			}
#endif /* MT_DFS_SUPPORT */

			ScanInfo = &wdev->ScanInfo;
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
					 "(%s) NumOfChannels = %u, LastScanChannel = %u, bScanning = %u, BreakTime = %u[%d]\n",
					  wdev->if_dev->name,
					  ScanCtrl->PartialScan.NumOfChannels,
					  ScanInfo->LastScanChannel,
					  ScanCtrl->PartialScan.bScanning,
					  ScanCtrl->PartialScan.BreakTime, scan_in_run_state(pAd, wdev));
			if (scan_in_run_state(pAd, wdev) == FALSE)
				ScanCtrl->PartialScan.BreakTime++;
			/* When partialscan is pending, count breaktime;
			*  We should wait (BeaconPeriod + 100ms), then trigger next scan.*/
			if (ScanCtrl->PartialScan.BreakTime >
				(pAd->CommonCfg.BeaconPeriod[BandIdx] / MLME_TASK_EXEC_INTV) + 1) {
				MLME_SCAN_REQ_STRUCT ScanReq;
				BSS_TABLE *pScanTab = NULL;

				ScanCtrl->PartialScan.BreakTime = 0;
				ScanType = SCAN_PARTIAL;
#ifdef WSC_AP_SUPPORT
#ifdef APCLI_SUPPORT
				if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA) &&
					(wdev->func_idx < MAX_MULTI_STA)) {
					WSC_CTRL *pWpsCtrl = &pAd->StaCfg[wdev->func_idx].wdev.WscControl;

					if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
						(pWpsCtrl->bWscTrigger == TRUE))
						ScanType = SCAN_WSC_ACTIVE;
				}
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */
				if (ScanInfo->LastScanChannel == 0) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
							"Reset BssTable for partial scan\n");
					pScanTab = &ScanCtrl->ScanTab;
					if (pScanTab)
						BssTableInit(pScanTab);
				}
				NdisZeroMemory(&ScanReq, sizeof(MLME_SCAN_REQ_STRUCT));
				ScanParmFill(pAd, &ScanReq, ScanCtrl->Ssid, ScanCtrl->SsidLen, BSS_ANY, SCAN_PARTIAL);
				if (MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_SCAN_REQ,
						sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0, wdev))
					RTMP_MLME_HANDLER(pAd);
				else {
					ScanCtrl->PartialScan.bScanning = FALSE;
					if (GetCurrentChannelOpOwner(pAd, wdev) == CH_OP_OWNER_PARTIAL_SCAN)
						ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_PARTIAL_SCAN);
					MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"(%s) Enq SYNC_FSM_SCAN_REQ Fail, Disable PartialScan(%d).\n",
						wdev->if_dev->name, ScanCtrl->PartialScan.bScanning);
				}

				continue;
			}
		}
	}
}

INT scan_partial_init(RTMP_ADAPTER *pAd)
{
	PARTIAL_SCAN *PartialScanCtrl = NULL;
	UCHAR BandIdx = 0;

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		PartialScanCtrl = &pAd->ScanCtrl[BandIdx].PartialScan;
	PartialScanCtrl->bScanning = FALSE;
	PartialScanCtrl->NumOfChannels = DEFLAUT_PARTIAL_SCAN_CH_NUM;
	PartialScanCtrl->LastScanChannel = 0;
	PartialScanCtrl->BreakTime = 0;
	RTMPInitTimer(pAd, &PartialScanCtrl->PartialScanTimer,
					GET_TIMER_FUNCTION(restart_partial_scan), PartialScanCtrl, FALSE);
	}
	return 0;
}

INT scan_release_mem(struct _RTMP_ADAPTER *ad)
{
	int i;

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		ad->ScanCtrl[i].SkipCh_Num = 0;
		if (ad->ScanCtrl[i].SkipList) {
			os_free_mem(ad->ScanCtrl[i].SkipList);
			ad->ScanCtrl[i].SkipList = NULL;
		}
	}
	return TRUE;
}

#ifdef CONFIG_STA_SUPPORT
VOID sta_2040_coex_scan_check(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (wdev) {
		if (!WMODE_CAP_2G(wdev->PhyMode))
			return;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"wdev is NULL!\n");
		return;
	}
	/* Perform 20/40 BSS COEX scan every Dot11BssWidthTriggerScanInt	*/
	if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) &&
		(pAd->CommonCfg.Dot11BssWidthTriggerScanInt != 0) &&
		((pAd->Mlme.OneSecPeriodicRound % pAd->CommonCfg.Dot11BssWidthTriggerScanInt) == (pAd->CommonCfg.Dot11BssWidthTriggerScanInt - 1))
#if defined(MT7603_FPGA) || defined(MT7628_FPGA)  || defined(MT7636_FPGA) || \
	defined(MT7637_FPGA) || defined(AXE_FPGA) || defined(MT7915_FPGA) || \
	defined(MT7986_FPGA) || defined(MT7916_FPGA) || defined(MT7981_FPGA)
		/* TODO: shiang-MT7603, remove me after verification done!! */
		&& (pAd->fpga_ctl.fpga_on == 0)
#endif
	   ) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "MMCHK - LastOneSecTotalTxCount/LastOneSecRxOkDataCnt  = %d/%d\n",
				 pAd->RalinkCounters.LastOneSecTotalTxCount,
				 pAd->RalinkCounters.LastOneSecRxOkDataCnt);

		/*
			Check last scan time at least 30 seconds from now.
			Check traffic is less than about 1.5~2Mbps.
			it might cause data lost if we enqueue scanning.
			This criteria needs to be considered
		*/

		if ((pAd->RalinkCounters.LastOneSecTotalTxCount < 70) && (pAd->RalinkCounters.LastOneSecRxOkDataCnt < 70)) {
			MLME_SCAN_REQ_STRUCT ScanReq;
			/* Fill out stuff for scan request and kick to scan*/
			ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_2040_BSS_COEXIST);
			cntl_scan_request(wdev, &ScanReq);
			/* Set InfoReq = 1, So after scan , alwats sebd 20/40 Coexistence frame to AP*/
			pAd->CommonCfg.BSSCoexist2040.field.InfoReq = 1;
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, " LastOneSecTotalTxCount/LastOneSecRxOkDataCnt  = %d/%d\n",
				 pAd->RalinkCounters.LastOneSecTotalTxCount,
				 pAd->RalinkCounters.LastOneSecRxOkDataCnt);
	}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
}
#endif /* CONFIG_STA_SUPPORT */

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID ScanParmFill(
	IN RTMP_ADAPTER *pAd,
	IN OUT MLME_SCAN_REQ_STRUCT *ScanReq,
	IN RTMP_STRING Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR BssType,
	IN UCHAR ScanType)
{
	NdisZeroMemory(ScanReq->Ssid, MAX_LEN_OF_SSID);
	ScanReq->SsidLen = (SsidLen > MAX_LEN_OF_SSID) ? MAX_LEN_OF_SSID : SsidLen;
	NdisMoveMemory(ScanReq->Ssid, Ssid, ScanReq->SsidLen);
	ScanReq->BssType = BssType;
	ScanReq->ScanType = ScanType;
}

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
INT RTMPIoctlQueryScanResult(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	UINT i;
	int alloc_size = 0;
	int ret;
	struct event_bss_entry *EventBssEntry;
	struct event_bss_entry *show_bss_entry;
	struct customer_bss_entry *bss_entry;
	UCHAR *pstr = NULL;
	INT	Status;

	struct wifi_dev *wdev = NULL;
	BSS_TABLE *ScanTab = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[apidx].wdev;
	} else
#endif /* APCLI_SUPPORT */
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	}
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	RTMP_SPIN_LOCK(&ScanTab->event_bss_entry_lock);
	alloc_size = ScanTab->EventBssEntryLen;
	RTMP_SPIN_UNLOCK(&ScanTab->event_bss_entry_lock);

	ret = os_alloc_mem(pAd, (UCHAR **)&EventBssEntry, alloc_size);
	NdisZeroMemory(EventBssEntry, alloc_size);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"alloc_size = %d\n", alloc_size);

	show_bss_entry = EventBssEntry;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"%-4s%-4s%-33s%-18s%-5s%-4s%-6s%-5s%-6s%-9s%-9s%-9s%-9s%-9s%-8s%-8s%-50s\n",
		"No", "Ch", "SSID", "BSSID", "RSSI",
		"TIM", "NOISE", "HTBW", "VHTBW", "VHT_TX_S",
		"VHT_RX_S", "P-cipher", "PHY-MODE",
		"BIT_RATE", "HT_TX_S", "HT_RX_S", "VENDOR_IE");

	RTMP_SPIN_LOCK(&ScanTab->event_bss_entry_lock);

	for (i = 0; i < ScanTab->BssNr; i++) {
		bss_entry = &ScanTab->BssEntry[i].CustomerBssEntry;

		NdisMoveMemory(show_bss_entry->ssid, bss_entry->ssid, bss_entry->ssid_len);
		show_bss_entry->ssid_len = bss_entry->ssid_len;

		NdisMoveMemory(show_bss_entry->bssid, bss_entry->bssid, MAC_ADDR_LEN);
		show_bss_entry->channel = bss_entry->channel;
		show_bss_entry->beacon_period = bss_entry->beacon_period;
		show_bss_entry->rssi = bss_entry->rssi;
		show_bss_entry->noise = bss_entry->noise;
		show_bss_entry->ht_ch_bandwidth = bss_entry->ht_ch_bandwidth;
		show_bss_entry->vht_ch_bandwidth = bss_entry->vht_ch_bandwidth;
		show_bss_entry->vht_tx_ss = bss_entry->vht_tx_ss;
		show_bss_entry->vht_rx_ss = bss_entry->vht_rx_ss;
		show_bss_entry->PairwiseCipher = bss_entry->PairwiseCipher;
		show_bss_entry->phy_mode = bss_entry->phy_mode;
		show_bss_entry->max_bit_rate = bss_entry->max_bit_rate;
		show_bss_entry->vendor_ie_len = bss_entry->vendor_ie.length;
		pstr = wmode_2_str(show_bss_entry->phy_mode);
		show_bss_entry->ht_tx_ss = bss_entry->ht_tx_ss;
		show_bss_entry->ht_rx_ss = bss_entry->ht_rx_ss;

		if (bss_entry->vendor_ie.pointer != NULL)
			NdisMoveMemory(show_bss_entry->vendor_ie,
				       bss_entry->vendor_ie.pointer,
				       bss_entry->vendor_ie.length);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-4d%-4d%-33s", i, show_bss_entry->channel, show_bss_entry->ssid);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			MACSTR,
			MAC2STR(show_bss_entry->bssid));
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-5d", show_bss_entry->rssi);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-4d", show_bss_entry->beacon_period);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-6d", show_bss_entry->noise);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-5d", show_bss_entry->ht_ch_bandwidth);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-6d", show_bss_entry->vht_ch_bandwidth);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-9d", show_bss_entry->vht_tx_ss);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-9d", show_bss_entry->vht_rx_ss);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%08x ", show_bss_entry->PairwiseCipher);

		if (pstr) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"%-9s", pstr);
			os_free_mem(pstr);
		}
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-9d", show_bss_entry->max_bit_rate);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-8d", show_bss_entry->ht_tx_ss);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"%-8d", show_bss_entry->ht_rx_ss);

		if (bss_entry->vendor_ie.pointer != NULL &&
		    show_bss_entry->vendor_ie_len != 0) {
			PEID_STRUCT pEid = (PEID_STRUCT)show_bss_entry->vendor_ie;
			UCHAR len = 0;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"len=%02x ", show_bss_entry->vendor_ie_len);

			while (len + 2 + pEid->Len <= show_bss_entry->vendor_ie_len) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
					"%02x%02x--%02x%02x%02x ",
					pEid->Eid,
					pEid->Len,
					pEid->Octet[0],
					pEid->Octet[1],
					pEid->Octet[2]);
				len = len + 2 + pEid->Len;
				pEid = (PEID_STRUCT)((char *)pEid + 2 + pEid->Len);
			}
		}

		show_bss_entry = PTR_ALIGN((struct event_bss_entry *)(
			(char *)show_bss_entry->vendor_ie + show_bss_entry->vendor_ie_len),
			sizeof(UINT32));

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "\n");
	}
	RTMP_SPIN_UNLOCK(&ScanTab->event_bss_entry_lock);

	wrq->u.data.length = alloc_size;

	/* copy_to_user: Return the number of bytes NOT copied.  */
	Status = copy_to_user(wrq->u.data.pointer, EventBssEntry,
			      wrq->u.data.length) ? -EFAULT : 0;
	if (Status)
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"copy_to_user() fail\n");
	os_free_mem(EventBssEntry);

	return Status;
}
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#endif /* SCAN_SUPPORT */

