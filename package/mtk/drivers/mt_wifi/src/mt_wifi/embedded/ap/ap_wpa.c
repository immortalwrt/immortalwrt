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
    wpa.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Jan Lee     03-07-22        Initial
    Rory Chen   04-11-29        Add WPA2PSK
*/
#include "rt_config.h"

extern UCHAR	EAPOL[];

/*
    ==========================================================================
    Description:
	Port Access Control Inquiry function. Return entry's Privacy and Wpastate.
    Return:
	pEntry
    ==========================================================================
*/
MAC_TABLE_ENTRY *PACInquiry(RTMP_ADAPTER *pAd, UINT16 Wcid)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		pEntry = &(pAd->MacTab.Content[Wcid]);

	return pEntry;
}

VOID ap_handle_mic_error_event(RTMP_ADAPTER *ad, MAC_TABLE_ENTRY *entry, RX_BLK *rx_blk)
{
	{
		RTMP_HANDLE_COUNTER_MEASURE(ad, entry);
	}
}

/*
    ==========================================================================
    Description:
	Function to handle countermeasures active attack.  Init 60-sec timer if necessary.
    Return:
    ==========================================================================
*/
VOID HandleCounterMeasure(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{

#ifndef RT_CFG80211_SUPPORT
	INT i;
#endif
	BOOLEAN Cancelled;

	if (!pEntry)
		return;

	/* Todo by AlbertY - Not support currently in ApClient-link */
	if (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
		return;

	/* if entry not set key done, ignore this RX MIC ERROR */
	if ((pEntry->SecConfig.Handshake.WpaState < AS_PTKINITDONE) || (pEntry->SecConfig.Handshake.GTKState != REKEY_ESTABLISHED))
		return;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HandleCounterMeasure ===>\n");
	/* record which entry causes this MIC error, if this entry sends disauth/disassoc, AP doesn't need to log the CM */
	pEntry->CMTimerRunning = TRUE;
	pAd->ApCfg.MICFailureCounter++;
	/* send wireless event - for MIC error */
	RTMPSendWirelessEvent(pAd, IW_MIC_ERROR_EVENT_FLAG, pEntry->Addr, 0, 0);
#ifdef RT_CFG80211_SUPPORT
	{
		const UCHAR tsc[6] = {0, 0, 0, 0, 0, 0};
		PNET_DEV pNetDev = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_dev;
		/* NL80211_KEYTYPE_PAIRWISE = 1, tsc = tsc of frame causing mic failure */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s:calling cfg event to HandleCounterMeasure\n", __FUNCTION__));
		cfg80211_michael_mic_failure(pNetDev, pEntry->Addr, 1, 0, tsc, GFP_KERNEL);
	}
#endif

	if (pAd->ApCfg.CMTimerRunning == TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Receive CM Attack Twice within 60 seconds ====>>>\n"));
		/* send wireless event - for counter measures */
		RTMPSendWirelessEvent(pAd, IW_COUNTER_MEASURES_EVENT_FLAG, pEntry->Addr, 0, 0);
		ApLogEvent(pAd, pEntry->Addr, EVENT_COUNTER_M);
#ifndef RT_CFG80211_SUPPORT
		/* renew GTK */
		GenRandom(pAd, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].GNonce);
#endif
		/* Cancel CounterMeasure Timer */
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;

#ifndef RT_CFG80211_SUPPORT

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			/* happened twice within 60 sec,  AP SENDS disaccociate all associated STAs.  All STA's transition to State 2 */
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i])) {
#ifdef MAP_R2
				if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
					wapp_handle_sta_disassoc(pAd, i, REASON_MIC_FAILURE);
#endif
				MlmeDeAuthAction(pAd, &pAd->MacTab.Content[i], REASON_MIC_FAILURE, FALSE);
			}
		}
#endif

		/*
			Further,  ban all Class 3 DATA transportation for a period 0f 60 sec
			disallow new association , too
		*/
		pAd->ApCfg.BANClass3Data = TRUE;
		/* check how many entry left...  should be zero */
		/*pAd->ApCfg.MBSSID[pEntry->func_tb_idx].GKeyDoneStations = pAd->MacTab.Size; */
		/*MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "GKeyDoneStations=%d\n", pAd->ApCfg.MBSSID[pEntry->func_tb_idx].GKeyDoneStations); */
	}

	RTMPSetTimer(&pAd->ApCfg.CounterMeasureTimer, 60 * MLME_TASK_EXEC_INTV * MLME_TASK_EXEC_MULTIPLE);
	pAd->ApCfg.CMTimerRunning = TRUE;
	pAd->ApCfg.PrevaMICFailTime = pAd->ApCfg.aMICFailTime;
	RTMP_GetCurrentSystemTime(&pAd->ApCfg.aMICFailTime);
}


/*
    ==========================================================================
    Description:
	countermeasures active attack timer execution
    Return:
    ==========================================================================
*/
VOID CMTimerExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	UINT            i, j = 0;
	PRTMP_ADAPTER   pAd = (PRTMP_ADAPTER)FunctionContext;

	pAd->ApCfg.BANClass3Data = FALSE;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i])
			&& (pAd->MacTab.Content[i].CMTimerRunning == TRUE)) {
			pAd->MacTab.Content[i].CMTimerRunning = FALSE;
			j++;
		}
	}

	if (j > 1)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Find more than one entry which generated MIC Fail ..\n"));

	pAd->ApCfg.CMTimerRunning = FALSE;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Counter measure timer expired, resume connection access.\n"));
}




#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
VOID    ApcliWpaSendEapolStart(
	IN	PRTMP_ADAPTER	pAd,
	IN  PUCHAR          pBssid,
	IN  PMAC_TABLE_ENTRY pMacEntry,
	IN	PSTA_ADMIN_CONFIG pApCliEntry)
{
	IEEE8021X_FRAME		Packet;
	UCHAR               Header802_3[14];

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "-----> ApCliWpaSendEapolStart\n");
	NdisZeroMemory(Header802_3, sizeof(UCHAR)*14);
	MAKE_802_3_HEADER(Header802_3, pBssid, &pApCliEntry->wdev.if_addr[0], EAPOL);
	/* Zero message 2 body */
	NdisZeroMemory(&Packet, sizeof(Packet));
	Packet.Version = EAPOL_VER;
	Packet.Type    = EAPOLStart;
	Packet.Length  = cpu2be16(0);
	/* Copy frame to Tx ring */
	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					  Header802_3, LENGTH_802_3, (PUCHAR)&Packet, 4, TRUE);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "<----- ApcliWpaSendEapolStart\n");
}
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif/*APCLI_SUPPORT*/

INT	Set_PtkRekey_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT16 wcid = 0;
	MAC_TABLE_ENTRY *pEntry  = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry	= NULL;
	PSECURITY_CONFIG pSecConfig  = NULL;

	wcid = (CHAR)os_str_tol(arg, 0, 10);
	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return FALSE;

	pEntry = &pAd->MacTab.Content[wcid];
	pSecConfig = &pEntry->SecConfig;
	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];

	if (!IS_ENTRY_CLIENT(pEntry))
		return FALSE;

	if ((IS_AKM_PSK(pSecConfig->AKMMap)
		|| (IS_AKM_WPA2(pSecConfig->AKMMap) && (is_pmkid_cache_in_sec_config(pSecConfig)))
		|| (IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) && (is_pmkid_cache_in_sec_config(pSecConfig)))
		|| (IS_AKM_OWE(pSecConfig->AKMMap) && (is_pmkid_cache_in_sec_config(pSecConfig))))) {

		PHANDSHAKE_PROFILE pHandshake4Way  = NULL;

		pHandshake4Way = &pSecConfig->Handshake;
		pHandshake4Way->WpaState = AS_PTKSTART;
		pHandshake4Way->MsgRetryCounter = 0;
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;

		WPABuildPairMsg1(pAd, &pEntry->SecConfig, pEntry);

		MTWF_PRINT("(WCID=%d)\n", wcid);
	}

	return TRUE;
}

