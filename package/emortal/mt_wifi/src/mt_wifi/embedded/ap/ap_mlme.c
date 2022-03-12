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
/****************************************************************************
 ****************************************************************************

    Module Name:
    mlme.c

    Abstract:
    Major MLME state machiones here

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"
#include <stdarg.h>

#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */

#ifdef DOT11_N_SUPPORT

int DetectOverlappingPeriodicRound;


#ifdef DOT11N_DRAFT3
VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	int apidx;
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Bss2040CoexistTimeOut(): Recovery to original setting!\n"));
	/* Recovery to original setting when next DTIM Interval. */
	pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_TIMER_FIRED);
	NdisZeroMemory(&pAd->CommonCfg.LastBSSCoexist2040, sizeof(BSS_2040_COEXIST_IE));
	pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;

	if (pAd->CommonCfg.bBssCoexEnable == FALSE) {
		/* TODO: Find a better way to handle this when the timer is fired and we disable the bBssCoexEable support!! */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Bss2040CoexistTimeOut(): bBssCoexEnable is FALSE, return directly!\n"));
		return;
	}

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		SendBSS2040CoexistMgmtAction(pAd, MCAST_WCID_TO_REMOVE, apidx, 0);
}
#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */


VOID APDetectOverlappingExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
#ifdef DOT11_N_SUPPORT
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	struct freq_oper oper;
	BOOLEAN bSupport2G = HcIsRfSupport(pAd, RFIC_24GHZ);
	int i;
	struct wifi_dev *wdev;
	UCHAR cfg_ht_bw;
	UCHAR cfg_ext_cha;

	if (DetectOverlappingPeriodicRound == 0) {
		/* switch back 20/40 */
		if (bSupport2G) {
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
				cfg_ht_bw = wlan_config_get_ht_bw(wdev);
				if (wmode_2_rfic(wdev->PhyMode) == RFIC_24GHZ && (cfg_ht_bw == HT_BW_40)) {
					cfg_ext_cha = wlan_config_get_ext_cha(wdev);
					wlan_operate_set_ht_bw(wdev, HT_BW_40, cfg_ext_cha);
				}
			}
		}
	} else {
		if ((DetectOverlappingPeriodicRound == 25) || (DetectOverlappingPeriodicRound == 1)) {
			if (hc_radio_query_by_rf(pAd, RFIC_24GHZ, &oper) != HC_STATUS_OK) {
				return;
			}
			if (oper.ht_bw == HT_BW_40) {
				SendBeaconRequest(pAd, 1);
				SendBeaconRequest(pAd, 2);
				SendBeaconRequest(pAd, 3);
			}
		}

		DetectOverlappingPeriodicRound--;
	}

#endif /* DOT11_N_SUPPORT */
}

#ifdef WIFI_IAP_BCN_STAT_FEATURE
INT  calculate_beacon_lose(RTMP_ADAPTER *pAd)
{
	PSTA_ADMIN_CONFIG pstacfg = NULL;
	UINT8 idx = 0;
	ULONG now = 0;
	ULONG difftime = 0;
	UINT32 add_loss = 0;
	USHORT beaconperiod = 0;
	UINT8 apclivalid = 0;
	static UINT32 last_add_loss[MAX_MULTI_STA] = {0};
	UINT8 th_bcn_loss = 2;
	struct wifi_dev *wdev = NULL;

	if (!pAd) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s](%d): ERROR, pAd = %p;\n",
		__func__, __LINE__, pAd));
		return FALSE;
	}

	for (idx = 0; idx < MAX_MULTI_STA; idx++) {
		pstacfg = &pAd->StaCfg[idx];
		wdev = &pstacfg->wdev;
		apclivalid =  pstacfg->ApcliInfStat.Valid;

		NdisGetSystemUpTime(&now);
		difftime = (UINT32)(now - (pstacfg->LastBeaconRxTime));
		difftime = (UINT32)(difftime * 1000)/OS_HZ;	/*ms*/
		beaconperiod = pstacfg->BeaconPeriod;

		if (wdev && wdev->DevInfo.WdevActive &&
			apclivalid && (difftime > (th_bcn_loss * beaconperiod))) {
			add_loss = (UINT32)(difftime/beaconperiod);
			pstacfg->beacon_loss_count += (UINT32)(add_loss - last_add_loss[idx]);
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s](%d): beacon_loss = %u; rx_beacon: %lu;\n",
			__func__, __LINE__, pstacfg->beacon_loss_count,
			pstacfg->rx_beacon));

			last_add_loss[idx] = add_loss;
		} else {
			if (!apclivalid) {
				pstacfg->beacon_loss_count = 0;
				pstacfg->rx_beacon = 0;
			}
			last_add_loss[idx] = 0;
		}
	}
	return TRUE;
}
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/


/*
    ==========================================================================
    Description:
	This routine is executed every second -
	1. Decide the overall channel quality
	2. Check if need to upgrade the TX rate to any client
	3. perform MAC table maintenance, including ageout no-traffic clients,
	   and release packet buffer in PSQ is fail to TX in time.
    ==========================================================================
 */
VOID APMlmePeriodicExec(
	PRTMP_ADAPTER pAd)
{
#ifdef A_BAND_SUPPORT
	BOOLEAN bSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);

#endif /*A_BAND_SUPPORT*/
#ifdef A4_CONN
	UCHAR mbss_idx;
#endif
	/*
		Reqeust by David 2005/05/12
		It make sense to disable Adjust Tx Power on AP mode, since we can't
		take care all of the client's situation
		ToDo: need to verify compatibility issue with WiFi product.
	*/
#ifdef CARRIER_DETECTION_SUPPORT

	if (isCarrierDetectExist(pAd) == TRUE) {
		PCARRIER_DETECTION_STRUCT pCarrierDetect = &pAd->CommonCfg.CarrierDetect;

		if (pCarrierDetect->OneSecIntCount < pCarrierDetect->CarrierGoneThreshold) {
			pCarrierDetect->CD_State = CD_NORMAL;
			pCarrierDetect->recheck = pCarrierDetect->recheck1;

			if (pCarrierDetect->Debug != DBG_LVL_TRACE) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Carrier gone\n"));
				/* start all TX actions. */
				UpdateBeaconHandler(
					pAd,
					get_default_wdev(pAd),
					BCN_UPDATE_ALL_AP_RENEW);
				AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0, OPMODE_AP);
			} else
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Carrier gone\n"));
		}

		pCarrierDetect->OneSecIntCount = 0;
	}

#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef VOW_SUPPORT
	vow_display_info_periodic(pAd);
#endif /* VOW_SUPPORT */
#ifdef RED_SUPPORT
	if (pAd->red_debug_en && (pAd->red_mcu_offload == FALSE))
		appShowRedDebugMessage(pAd);
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
	if (pAd->fq_ctrl.dbg_en)
		app_show_fq_dbgmsg(pAd);
#endif /* RRSCH_SUPPORT */

	RTMP_CHIP_HIGH_POWER_TUNING(pAd, &pAd->ApCfg.RssiSample);
	/* Disable Adjust Tx Power for WPA WiFi-test. */
	/* Because high TX power results in the abnormal disconnection of Intel BG-STA. */
	/*#ifndef WIFI_TEST */
	/*	if (pAd->CommonCfg.bWiFiTest == FALSE) */
	/* for SmartBit 64-byte stream test */
	/* removed based on the decision of Ralink congress at 2011/7/06 */
	/*	if (pAd->MacTab.Size > 0) */
	RTMP_CHIP_ASIC_ADJUST_TX_POWER(pAd);
	/*#endif // WIFI_TEST */
	RTMP_CHIP_ASIC_TEMPERATURE_COMPENSATION(pAd);
	/* walk through MAC table, see if switching TX rate is required */

	/* MAC table maintenance */
	if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0) {
		/* one second timer */
		MacTableMaintenance(pAd);

		RTMPMaintainPMKIDCache(pAd);
#ifdef WDS_SUPPORT
		{
			UCHAR band_idx = DBDC_BAND0;

			for (; band_idx < DBDC_BAND_NUM; band_idx++)
				WdsTableMaintenance(pAd, band_idx);
		}
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
		CliWds_ProxyTabMaintain(pAd);
#endif /* CLIENT_WDS */
#ifdef A4_CONN
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++)
			a4_proxy_maintain(pAd, mbss_idx);
		pAd->a4_need_refresh = FALSE;
#endif /* A4_CONN */
#ifdef WH_EVENT_NOTIFIER
		WHCMlmePeriodicExec(pAd);
#endif /* WH_EVENT_NOTIFIER */

#ifdef WIFI_DIAG
		diag_ap_mlme_one_sec_proc(pAd);
#endif

	}

#ifdef AP_SCAN_SUPPORT
	AutoChannelSelCheck(pAd);
#endif /* AP_SCAN_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef WIFI_IAP_BCN_STAT_FEATURE
	calculate_beacon_lose(pAd);
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/

	if (pAd->Mlme.OneSecPeriodicRound % 2 == 0)
		ApCliIfMonitor(pAd);

	if (pAd->Mlme.OneSecPeriodicRound % 2 == 1)
#if defined(APCLI_AUTO_CONNECT_SUPPORT) || defined(CONFIG_MAP_SUPPORT)
		if (
#ifdef APCLI_AUTO_CONNECT_SUPPORT
				(pAd->ApCfg.ApCliAutoConnectChannelSwitching == FALSE)
#ifdef CONFIG_MAP_SUPPORT
				|| (IS_MAP_TURNKEY_ENABLE(pAd))
#endif /* CONFIG_MAP_SUPPORT */
#else
#ifdef CONFIG_MAP_SUPPORT
				(IS_MAP_TURNKEY_ENABLE(pAd))
#endif /* CONFIG_MAP_SUPPORT */
#endif
		   )
#endif
			ApCliIfUp(pAd);

	{
		INT loop;
		ULONG Now32;
		MAC_TABLE_ENTRY *pEntry;
#if 0 //def MAC_REPEATER_SUPPORT  // reslov:partialscan后又做一次SiteSurvey

		if (pAd->ApCfg.bMACRepeaterEn)
			RTMPRepeaterReconnectionCheck(pAd);

#endif /* MAC_REPEATER_SUPPORT */

		NdisGetSystemUpTime(&Now32);

		for (loop = 0; loop < MAX_APCLI_NUM; loop++) {
			PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[loop];


			if ((pApCliEntry->bBlockAssoc == TRUE) &&
				RTMP_TIME_AFTER(Now32, pApCliEntry->LastMicErrorTime + (60*OS_HZ)))
				pApCliEntry->bBlockAssoc = FALSE;


			if ((pApCliEntry->ApcliInfStat.Valid == TRUE)
				&& (VALID_UCAST_ENTRY_WCID(pAd, pApCliEntry->MacTabWCID))) {
				pEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
				/* update channel quality for Roaming and UI LinkQuality display */
				if (pEntry)
					MlmeCalculateChannelQuality(pAd, pEntry, Now32);
			}
		}
	}
#endif /* APCLI_SUPPORT */
#ifdef DOT11_N_SUPPORT
		{
			INT IdBss = 0;
			UCHAR ht_protect_en = 1;
			BSS_STRUCT *pMbss = NULL;

			for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
				pMbss = &pAd->ApCfg.MBSSID[IdBss];

				if ((pMbss) && (&pMbss->wdev) && (pMbss->wdev.DevInfo.WdevActive)) {
					ht_protect_en = wlan_config_get_ht_protect_en(&pMbss->wdev);
					if (ht_protect_en) {
						ApUpdateCapabilityAndErpIe(pAd, pMbss);
						APUpdateOperationMode(pAd, &pMbss->wdev);
					}
				}
			}
		}
#endif /* DOT11_N_SUPPORT */

#ifdef A_BAND_SUPPORT
	if (bSupport5G && (pAd->CommonCfg.bIEEE80211H == 1)) {
		INT IdBss = 0;
		BOOLEAN BandInCac[DBDC_BAND_NUM];
		UCHAR i;
		BSS_STRUCT *pMbss = NULL;
		struct DOT11_H *pDot11hTest = NULL;
		struct wifi_dev *wdev;
		UCHAR BandIdx;

		for (i = 0; i < DBDC_BAND_NUM; i++)
			BandInCac[i] = FALSE;

		for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
			pMbss = &pAd->ApCfg.MBSSID[IdBss];
			wdev = &pMbss->wdev;
			if ((pMbss == NULL) || (wdev == NULL) || (wdev->pHObj == NULL))
				continue;

			BandIdx = HcGetBandByWdev(wdev);

			pDot11hTest = &pAd->Dot11_H[BandIdx];
			if (pDot11hTest == NULL)
				continue;
#ifdef MT_DFS_SUPPORT
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_TURNKEY_ENABLE(pAd)) {
				if (wdev->map_indicate_channel_change && (wdev->map_radar_detect == 0)) {
					wdev->map_indicate_channel_change = 0;
					wapp_send_ch_change_rsp(pAd, wdev, wdev->channel);
				}
			}
#endif
			if (pDot11hTest->RDMode == RD_SILENCE_MODE) {
				if (BandInCac[BandIdx] == TRUE)
					continue;
				else
					BandInCac[BandIdx] = TRUE;

				if (pDot11hTest->RDCount++ > pDot11hTest->cac_time) {
					pDot11hTest->RDCount = 0;
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
						wapp_send_cac_stop(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), wdev->channel, TRUE);
					}
#endif
					MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
					AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0,  OPMODE_AP);
					pDot11hTest->RDMode = RD_NORMAL_MODE;
				}
			} else
#endif
			{
				pDot11hTest->InServiceMonitorCount++;
			}
		}
	}
#endif /* A_BAND_SUPPORT */

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyCountDown(pAd);
	DfsOutBandCacCountUpdate(pAd);
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	DfsV10W56APDownTimeCountDown(pAd);
#endif
#endif
#ifdef MBO_SUPPORT
	MboCheckBssTermination(pAd);
#endif /* MBO_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_R1KHInfoMaintenance(pAd);
#endif /* DOT11R_FT_SUPPORT */
#ifdef BAND_STEERING
	BndStrgHeartBeatMonitor(pAd);
#endif
}


/*! \brief   To substitute the message type if the message is coming from external
 *  \param  *Fr            The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN APMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType)
{
	USHORT Seq;
#ifdef DOT11_SAE_SUPPORT
	USHORT Alg;
#endif /* DOT11_SAE_SUPPORT */
	UCHAR  EAPType;
	BOOLEAN     Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
	PMAC_TABLE_ENTRY pEntry;
#endif /* WSC_AP_SUPPORT */
	unsigned char hdr_len = LENGTH_802_11;

#ifdef A4_CONN
	if ((pFrame->Hdr.FC.FrDs == 1) && (pFrame->Hdr.FC.ToDs == 1))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif
	/*
		TODO:
		only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid;
		otherwise, ignore this frame
	*/

	/* wpa EAPOL PACKET */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) {
#ifdef WSC_AP_SUPPORT
		WSC_CTRL *wsc_ctrl;
		struct wifi_dev *wdev;

		/*WSC EAPOL PACKET */
		pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);

		if (pEntry) {
			wdev = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev;

			if (pEntry->bWscCapable
				|| IS_AKM_OPEN(wdev->SecConfig.AKMMap)
				|| IS_AKM_SHARED(wdev->SecConfig.AKMMap)
				|| IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap)) {
				/*
					WSC AP only can service one WSC STA in one WPS session.
					Forward this EAP packet to WSC SM if this EAP packets is from
					WSC STA that WSC AP services or WSC AP doesn't service any
					WSC STA now.
				*/
				wsc_ctrl = &wdev->WscControl;

				if ((MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, pEntry->Addr) ||
					 MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, ZERO_MAC_ADDR)) &&
					IS_ENTRY_CLIENT(pEntry) &&
					(wsc_ctrl->WscConfMode != WSC_DISABLE)) {
					*Machine = WSC_STATE_MACHINE;
					EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
					EAPCode = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 4);
					Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
				}
			}
		}

#endif /* WSC_AP_SUPPORT */

		if (!Return) {
			*Machine = WPA_STATE_MACHINE;
			EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
			Return = WpaMsgTypeSubst(EAPType, (INT *) MsgType);
		}

		return Return;
	}

	if (pFrame->Hdr.FC.Type != FC_TYPE_MGMT)
		return FALSE;

	switch (pFrame->Hdr.FC.SubType) {
	case SUBTYPE_ASSOC_REQ:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_ASSOC_REQ;
		break;

	/*
	case SUBTYPE_ASSOC_RSP:
		*Machine = ASSOC_FSM;
		*MsgType = APMT2_PEER_ASSOC_RSP;
		break;
	*/
	case SUBTYPE_REASSOC_REQ:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_REASSOC_REQ;
		break;

	/*
	case SUBTYPE_REASSOC_RSP:
		*Machine = ASSOC_FSM;
		*MsgType = APMT2_PEER_REASSOC_RSP;
		break;
	*/

	case SUBTYPE_BEACON:
		*Machine = SYNC_FSM;
		*MsgType = SYNC_FSM_PEER_BEACON;
		break;

	case SUBTYPE_PROBE_RSP:
		*Machine = SYNC_FSM;
		*MsgType = SYNC_FSM_PEER_PROBE_RSP;
		break;

	case SUBTYPE_PROBE_REQ:
		*Machine = SYNC_FSM;
		*MsgType = SYNC_FSM_PEER_PROBE_REQ;
		break;

	/*
	case SUBTYPE_ATIM:
		*Machine = AP_SYNC_STATE_MACHINE;
		*MsgType = APMT2_PEER_ATIM;
		break;
	*/
	case SUBTYPE_DISASSOC:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_DISASSOC_REQ;
		break;

	case SUBTYPE_AUTH:
		/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm */
#ifdef DOT11_SAE_SUPPORT
		NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
#endif /* DOT11_SAE_SUPPORT */
		NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
		*Machine = AUTH_FSM;

		if (Seq == 1
#ifdef DOT11_SAE_SUPPORT
			|| (Alg == AUTH_MODE_SAE && Seq == 2)
#endif /* DOT11_SAE_SUPPORT */
			)
			*MsgType = AUTH_FSM_PEER_AUTH_REQ;
		else if (Seq == 3)
			*MsgType = AUTH_FSM_PEER_AUTH_CONF;
		else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("wrong AUTH seq=%d Octet=%02x %02x %02x %02x %02x %02x %02x %02x\n",
					 Seq,
					 pFrame->Octet[0], pFrame->Octet[1], pFrame->Octet[2], pFrame->Octet[3],
					 pFrame->Octet[4], pFrame->Octet[5], pFrame->Octet[6], pFrame->Octet[7]));
			return FALSE;
		}

		break;

	case SUBTYPE_DEAUTH:
		*Machine = AUTH_FSM; /*AP_AUTH_RSP_STATE_MACHINE;*/
		*MsgType = AUTH_FSM_PEER_DEAUTH;
		break;

	case SUBTYPE_ACTION:
	case SUBTYPE_ACTION_NO_ACK:
		*Machine = ACTION_STATE_MACHINE;
		/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support */
#ifdef P2P_SUPPORT

		/*	Vendor specific usage. */
		if ((pFrame->Octet[0] & 0x7F) == MT2_ACT_VENDOR) { /*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support */
			UCHAR	P2POUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x9};

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Vendor Action frame OUI= 0x%x\n", *(PULONG)&pFrame->Octet[1]));

			/* Now support WFA P2P */
			if (RTMPEqualMemory(&pFrame->Octet[1], P2POUIBYTE, 4) && (P2P_INF_ON(pAd))) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Vendor Action frame P2P OUI= 0x%x\n", *(PULONG)&pFrame->Octet[1]));
				*Machine = P2P_ACTION_STATE_MACHINE;

				if (pFrame->Octet[5] <= MT2_MAX_PEER_SUPPORT)
					*MsgType = pFrame->Octet[5]; /* subtype.  */
				else
					return FALSE;
			} else
				return FALSE;
		} else
#endif /* P2P_SUPPORT */
			if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG)
				*MsgType = MT2_ACT_INVALID;
			else {
				if (pFrame->Hdr.FC.Order) {
					/* Bypass HTC 4 bytes to get correct CategoryCode */
					*MsgType = (pFrame->Octet[4]&0x7F);
				} else {
					*MsgType = (pFrame->Octet[0]&0x7F);
				}
			}

		break;

	default:
		return FALSE;
	}

	return TRUE;
}


/*
    ========================================================================
    Routine Description:
	Periodic evaluate antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd)
{
	ULONG	TxTotalCnt;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->CommonCfg.CarrierDetect.CD_State == CD_SILENCE)
		return;

#endif /* CARRIER_DETECTION_SUPPORT */
	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);
	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
				 pAd->RalinkCounters.OneSecTxRetryOkCount +
				 pAd->RalinkCounters.OneSecTxFailCount;

	if (TxTotalCnt > 50) {
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 20);
		pAd->Mlme.bLowThroughput = FALSE;
	} else {
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 300);
		pAd->Mlme.bLowThroughput = TRUE;
	}
}

/*
    ========================================================================
    Routine Description:
	After evaluation, check antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd)
{
	CHAR rssi[3], *target_rssi;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	/* if the traffic is low, use average rssi as the criteria */
	if (pAd->Mlme.bLowThroughput == TRUE)
		target_rssi = &pAd->ApCfg.RssiSample.LastRssi[0];
	else
		target_rssi = &pAd->ApCfg.RssiSample.AvgRssi[0];

	NdisMoveMemory(&rssi[0], target_rssi, 3);
	/* Disable the below to fix 1T/2R issue. It's suggested by Rory at 2007/7/11. */
	bbp_set_rxpath(pAd, pAd->Mlme.RealRxPath);
}


/*
    ========================================================================
    Routine Description:
	After evaluation, check antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID	APAsicAntennaAvg(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	              AntSelect,
	IN	SHORT * RssiAvg)
{
	SHORT	realavgrssi;
	LONG         realavgrssi1;
	ULONG	recvPktNum = pAd->RxAnt.RcvPktNum[AntSelect];

	realavgrssi1 = pAd->RxAnt.Pair1AvgRssiGroup1[AntSelect];

	if (realavgrssi1 == 0) {
		*RssiAvg = 0;
		return;
	}

	realavgrssi = (SHORT) (realavgrssi1 / recvPktNum);
	pAd->RxAnt.Pair1AvgRssiGroup1[0] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup2[0] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
	pAd->RxAnt.RcvPktNum[0] = 0;
	pAd->RxAnt.RcvPktNum[1] = 0;
	*RssiAvg = realavgrssi - 256;
}

