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

#include <rt_config.h>
#include "mac/mac_mt/dmac/wf_wtbl.h"
#define WTBL_TDD_WAIT_TIMEOUT RTMPMsecsToJiffies(10000)

static INT MacTableResetTimer(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN Cancelled;
	struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;

	RTMPCancelTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
	RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	RTMPCancelTimer(&pEntry->DisassocTimer, &Cancelled);
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11V_WNM_SUPPORT */

	return TRUE;
}

typedef struct wcid_score {
	UCHAR idx;
	UINT  score;
} WCID_SCORE, *PWCID_SCORE;


static BOOLEAN WtblTdd_InfoNode_HashDel(
	RTMP_ADAPTER *pAd,
	NODE_INFO *pNode)
{

	return TRUE;
}

static BOOLEAN WtblTdd_InfoNode_HashAdd(
	RTMP_ADAPTER *pAd,
	NODE_INFO *pNode)
{
	UINT HashIdx = 0;
	NODE_INFO *pCurrEntry = NULL;
	pNode->pNext = NULL;
	RTMP_SEM_LOCK(&pAd->wtblTddInfo.NodeHashLock);
	HashIdx = MAC_ADDR_HASH_INDEX(pNode->Addr);
	if (pAd->wtblTddInfo.NodeHash[HashIdx] == NULL)
		pAd->wtblTddInfo.NodeHash[HashIdx] = pNode;
	else {
		pCurrEntry = pAd->wtblTddInfo.NodeHash[HashIdx];
		while (pCurrEntry->pNext != NULL)
			pCurrEntry = pCurrEntry->pNext;
		pCurrEntry->pNext = pNode;
	}
	RTMP_SEM_UNLOCK(&pAd->wtblTddInfo.NodeHashLock);
	return TRUE;
}

static BOOLEAN WtblTdd_InactiveList_HashDel(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry)
{
	UINT HashIdx;
	MAC_TABLE_ENTRY  *pPrevEntry, *pProbeEntry;
	RTMP_SEM_LOCK(&pAd->MacTab.HashExtLock);
	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->Addr);
	pPrevEntry = NULL;
	pProbeEntry = pAd->MacTab.HashExt[HashIdx];
	ASSERT(pProbeEntry);
	/* update Hash list*/
	while (pProbeEntry) {
		if (pProbeEntry == pEntry) {
			if (pPrevEntry == NULL)
				pAd->MacTab.HashExt[HashIdx] = pEntry->pNext;
			else
				pPrevEntry->pNext = pEntry->pNext;

			break;
		}
		pPrevEntry = pProbeEntry;
		pProbeEntry = pProbeEntry->pNext;
	};
	RTMP_SEM_UNLOCK(&pAd->MacTab.HashExtLock);
	ASSERT(pProbeEntry != NULL);
	return TRUE;
}

static BOOLEAN WtblTdd_InactiveList_HashAdd(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry)
{
	UINT HashIdx = 0;
	MAC_TABLE_ENTRY *pCurrEntry = NULL;
	pEntry->pNext = NULL;
	RTMP_SEM_LOCK(&pAd->MacTab.HashExtLock);
	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->Addr);
	if (pAd->MacTab.HashExt[HashIdx] == NULL)
		pAd->MacTab.HashExt[HashIdx] = pEntry;
	else {
		pCurrEntry = pAd->MacTab.HashExt[HashIdx];
		while (pCurrEntry->pNext != NULL)
			pCurrEntry = pCurrEntry->pNext;
		pCurrEntry->pNext = pEntry;
	}
	RTMP_SEM_UNLOCK(&pAd->MacTab.HashExtLock);
	return TRUE;
}

static UINT16 WtblTdd_InactiveList_FindSpace(
	RTMP_ADAPTER *pAd,
	UCHAR *SegIdx)
{
	UINT16 segIdx = 0, idx = 0;
	for (segIdx = 0; segIdx < WTBL_TDD_SW_MAC_TAB_SEG_NUM; segIdx++) {
		for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
			if (IS_ENTRY_NONE(&pAd->MacTab.ContentExt[segIdx][idx])) {
				pAd->MacTab.ContentExt[segIdx][idx].wtblTddCtrl.LastExtIdx = idx;
				*SegIdx = segIdx;
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Select SPACE [%d, %d] EntryType=%x--->\n", segIdx, idx, pAd->MacTab.ContentExt[segIdx][idx].EntryType);
				return idx;
			}
		}
	}
	return WCID_INVALID;
}

VOID WtblTdd_InactiveList_Predict(
	RTMP_ADAPTER *pAd,
	ULONG targetTime)
{
	UCHAR segIdx = 0, idx = 0, hit = 0;
	UCHAR windowCnt = 2;
	for (segIdx = 0; segIdx < WTBL_TDD_SW_MAC_TAB_SEG_NUM; segIdx++) {
		for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.ContentExt[segIdx][idx];
			if (IS_ENTRY_CLIENT(pEntry)) {
				/* if (RTMP_TIME_AFTER(targetTime, pEntry->wtblTddCtrl.ConnectTime)) { */
					if (targetTime < pEntry->wtblTddCtrl.ConnectTime) {
					hit++;
					if (pEntry->wtblTddCtrl.state != WTBL_TDD_STA_SWAP_REQ_ING) {
						MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							" SPACE [%d, %d]---> %02X:%02X:%02X:%02X:%02X:%02X (%ld) -->(%ld)\n",
							segIdx, idx, PRINT_MAC(pEntry->Addr), targetTime,
							pEntry->wtblTddCtrl.ConnectTime);
						WtblTdd_Entry_TxPacket(pAd, pEntry->wdev, pEntry);
					}
					if (hit == windowCnt)
						break;
				}
			}
		}
	}
}


static UINT16 WtblTdd_SelectBad_Fifo(
	RTMP_ADAPTER *pAd)
{
	UINT16 maxWcid = GET_MAX_UCAST_NUM(pAd), traversal = 0;
	static UINT16 wcid;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
next:
	wcid++;
	if (wcid >= maxWcid)
		wcid = 1;
	pEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &tr_ctl->tr_entry[wcid];

	if (IS_ENTRY_CLIENT(pEntry) &&
		(pEntry->Sst == SST_ASSOC) &&
		(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RULE 2---> wcid%d\n", wcid);
	} else {
		traversal++;
		if (traversal == maxWcid) {
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "can't select bad, No available Node!\n");
			return WCID_INVALID;
		}
		goto next;
	}
	return wcid;
	return wcid;
}

/* -- PUBLIC ------------------------------------------------------------------------------------------------------------- */

UINT16 WtblTdd_ActiveList_SelectBad(
	RTMP_ADAPTER *pAd)
{
	UINT16 Ret = WCID_INVALID;
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	/* WCID_SCORE wcidScore[MAX_LEN_OF_MAC_TABLE]; */

	Ret = WtblTdd_SelectBad_Fifo(pAd);
	if (Ret == WCID_INVALID)
		return WCID_INVALID;

	pEntry = &pAd->MacTab.Content[Ret];
	tr_entry = &tr_ctl->tr_entry[Ret];

	if (pEntry->wtblTddCtrl.guardTime !=  0)
		Ret = WCID_INVALID;
	MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" ===> %d, %d , %d, %d\n", IS_ENTRY_CLIENT(pEntry), pEntry->Sst,
		tr_entry->PortSecured, pEntry->wtblTddCtrl.guardTime);
	return Ret;
}

BOOLEAN WtblTdd_ActiveList_SwapOut(
	RTMP_ADAPTER *pAd,
	UCHAR SegIdx,
	UINT16 inActWcid,
	UINT16 actWcid)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[actWcid];
	MAC_TABLE_ENTRY *pInActEntry = &pAd->MacTab.ContentExt[SegIdx][inActWcid];
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	MT_WTBL_TDD_TIME_LOG_IN(pAd, pAd->sm_swapout_ent_time);
	MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG,
			("%s(): SegIdx=%d, inActWcid=%d, actWcid=%d,  IN-ACT ConnectID=%u, ACT ConnectID=%u--->\n",
			__func__, SegIdx, inActWcid, actWcid, pInActEntry->wtblTddCtrl.ConnectID, pEntry->wtblTddCtrl.ConnectID));
	/*
	   BackUp WTBL value, need to check the non block behavior!
	   this function can't be sleep due to called by MacTableInsertEntry()
	*/
	if (pEntry) {

		/* also copy to pActEntry for WtblTdd_ActiveList_SwapOut()'s NdisMoveMemory(pInActEntry, pEntry, sizeof(MAC_TABLE_ENTRY)) */
		RT_RW_WTBL_ALL WTBL_ALL;
		WTBL_ALL.WCID = actWcid;
		WTBL_ALL.inActWCID = inActWcid;
		WTBL_ALL.SegIdx = SegIdx;
		WTBL_ALL.Op = 0; /* query */
		WTBL_ALL.ucWtblBegin = 0;
		WTBL_ALL.ucWtblDwCnt = 44;
		os_move_mem((PUINT8)&WTBL_ALL.au4WtblBuffer, (PUINT8)&(pEntry->UWtblRaw.u4UWBLRaw), 20);
		/*
			HW_RW_WTBL_ALL() is non block cmd, MacTableInsertEntry() also call it!
		*/
		HW_RW_WTBL_ALL(pAd, &WTBL_ALL);

	} else {
		ASSERT(0);
	}


	/* Backup the active WCID to inActWcid */
	if (pEntry && !IS_ENTRY_NONE(pEntry)) {
		NODE_INFO *pNode = NULL;
		ULONG ConnectID = pEntry->wtblTddCtrl.ConnectID;
		pNode = &pAd->wtblTddInfo.nodeInfo[ConnectID];
		MacTableResetTimer(pAd, pEntry);
		NdisMoveMemory(pInActEntry, pEntry, sizeof(MAC_TABLE_ENTRY)); /* be care the pInActEntry.UWtblRaw & wtblTddCtrl */
		MT_WTBL_TDD_HEX_DUMP(pAd, "WtblTdd_ActiveList_SwapOut : pInActEntry->UWtblRaw.u4UWBLRaw ==>", (PUCHAR)&(pInActEntry->UWtblRaw.u4UWBLRaw),  (sizeof(pInActEntry->UWtblRaw.u4UWBLRaw)));
		NdisMoveMemory(pInActEntry->Addr, pEntry->Addr, MAC_ADDR_LEN);

		/* be care of the place  of tr_entry is move so tr_ctl */
		NdisMoveMemory(&pAd->MacTab.tr_entryExt[SegIdx][inActWcid],
			&tr_ctl->tr_entry[actWcid], sizeof(STA_TR_ENTRY));

		pInActEntry->EntryType = pEntry->EntryType;
		WtblTdd_InactiveList_HashAdd(pAd, pInActEntry);
		pAd->MacTab.SizeExt[SegIdx]++;

		pNode->tableMode = SW_TABLE;
		pNode->swLastIndex = inActWcid;
		pNode->swLastSegIndex = SegIdx;
		pNode->hwWtblIndex = actWcid;
		pNode->swapOutCnt++;

		pInActEntry->wtblTddCtrl.ConnectTime = pEntry->wtblTddCtrl.ConnectTime;
		pInActEntry->wtblTddCtrl.ConnectID = pEntry->wtblTddCtrl.ConnectID;

		pInActEntry->wtblTddCtrl.SegIdx = SegIdx;
		pInActEntry->wtblTddCtrl.LastExtIdx = inActWcid;

		pEntry->wtblTddCtrl.state = WTBL_TDD_STA_SWAP_OUT_ING;
		pEntry->wtblTddCtrl.LastWtblIdx = actWcid;
	} else {
		ASSERT(0);
	}

	pEntry = MacTableLookup(pAd, pEntry->Addr);
	if (pEntry && pEntry->wdev) {
		RT_RW_WTBL_ALL WTBL_ALL;
		WTBL_ALL.WCID = actWcid;
		WTBL_ALL.inActWCID = inActWcid;
		WTBL_ALL.SegIdx = SegIdx;
		HW_TDD_WCID_SWAP_OUT(pAd, &WTBL_ALL);
		{
			RTMP_OS_TASK *pTask = &pAd->HwCtrl.HwCtrlTask;
			RtmpMLMEUp(pTask);
		}

		MacTableDelEntryFromHash(pAd, pEntry);
		TRTableResetEntry(pAd, pEntry->wcid);
		/* Fix the AID full issue */
		aid_clear(&pAd->MacTab.aid_info, pEntry->Aid);
		SET_ENTRY_NONE(pEntry);
		/* Clear the active WCID Entry */
		pAd->MacTab.Size--;
	} else {
		ASSERT(0);
	}
	MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG, ("%s(): <----\n", __func__));
	MT_WTBL_TDD_TIME_LOG_OUT(pAd, pAd->sm_swapout_exit_time);
	return TRUE;
}



BOOLEAN pEntry_Clone_SEC(
	IN MAC_TABLE_ENTRY *pEntry,
	IN MAC_TABLE_ENTRY *pTmpEntry)
{
	if (!IS_SECURITY(&(pTmpEntry->SecConfig))) {
		/*
			no need to restore the sctructure;
			only restore AKMMap, PairwiseCipher
		*/
		pEntry->SecConfig.AKMMap = pTmpEntry->SecConfig.AKMMap;
		pEntry->SecConfig.PairwiseCipher = pTmpEntry->SecConfig.PairwiseCipher;
		return FALSE;
	}
	pEntry->SecConfig.AKMMap = pTmpEntry->SecConfig.AKMMap;
	os_move_mem(&(pEntry->SecConfig.WepKey), &(pTmpEntry->SecConfig.WepKey), sizeof(SEC_KEY_INFO));
	pEntry->SecConfig.PairwiseCipher = pTmpEntry->SecConfig.PairwiseCipher;
	pEntry->SecConfig.PairwiseKeyId = pTmpEntry->SecConfig.PairwiseKeyId;
	os_move_mem(&(pEntry->SecConfig.PSK), &(pTmpEntry->SecConfig.PSK), (LEN_PSK + 1));
	os_move_mem(&(pEntry->SecConfig.PMK), &(pTmpEntry->SecConfig.PMK), LEN_PMK);
	os_move_mem(&(pEntry->SecConfig.PTK), &(pTmpEntry->SecConfig.PTK), LEN_MAX_PTK);
	pEntry->SecConfig.GroupCipher = pTmpEntry->SecConfig.GroupCipher;
	pEntry->SecConfig.GroupKeyId = pTmpEntry->SecConfig.GroupKeyId;
	os_move_mem(&(pEntry->SecConfig.GMK), &(pTmpEntry->SecConfig.GMK), LEN_GMK);
	os_move_mem(&(pEntry->SecConfig.GTK), &(pTmpEntry->SecConfig.GTK), LEN_MAX_GTK);
	pEntry->SecConfig.GroupReKeyMethod = pTmpEntry->SecConfig.GroupReKeyMethod;
	pEntry->SecConfig.GroupReKeyInterval = pTmpEntry->SecConfig.GroupReKeyInterval;
	pEntry->SecConfig.GroupPacketCounter = pTmpEntry->SecConfig.GroupPacketCounter;
	pEntry->SecConfig.GroupReKeyInstallCountDown = pTmpEntry->SecConfig.GroupReKeyInstallCountDown;
	pEntry->SecConfig.PMKCachePeriod = pTmpEntry->SecConfig.PMKCachePeriod;
	/* useless */
	/* pEntry->SecConfig.STARec_Bssid = pTmpEntry->SecConfig.STARec_Bssid; */
	pEntry->SecConfig.key_deri_alg = pEntry->SecConfig.key_deri_alg;
#ifdef DOT11W_PMF_SUPPORT
	pEntry->SecConfig.PmfCfg.MFPC = pTmpEntry->SecConfig.PmfCfg.MFPC;
	pEntry->SecConfig.PmfCfg.Desired_MFPC = pTmpEntry->SecConfig.PmfCfg.Desired_MFPC;
	pEntry->SecConfig.PmfCfg.MFPR = pTmpEntry->SecConfig.PmfCfg.MFPR;
	pEntry->SecConfig.PmfCfg.Desired_MFPR = pTmpEntry->SecConfig.PmfCfg.Desired_MFPR;
	pEntry->SecConfig.PmfCfg.PMFSHA256 = pTmpEntry->SecConfig.PmfCfg.PMFSHA256;
	pEntry->SecConfig.PmfCfg.Desired_PMFSHA256 = pTmpEntry->SecConfig.PmfCfg.Desired_PMFSHA256;
	pEntry->SecConfig.PmfCfg.UsePMFConnect = pTmpEntry->SecConfig.PmfCfg.UsePMFConnect;
	/* pEntry->SecConfig.PmfCfg.UseSHA256Connect = pTmpEntry->SecConfig.PmfCfg.UseSHA256Connect; */
	pEntry->SecConfig.PmfCfg.IGTK_KeyIdx = pTmpEntry->SecConfig.PmfCfg.IGTK_KeyIdx;
	os_move_mem(&(pEntry->SecConfig.PmfCfg.IGTK[0][0]), &(pTmpEntry->SecConfig.PmfCfg.IGTK[0][0]), LEN_AES_GTK);
	os_move_mem(&(pEntry->SecConfig.PmfCfg.IGTK[1][0]), &(pTmpEntry->SecConfig.PmfCfg.IGTK[1][0]), LEN_AES_GTK);
	os_move_mem(&(pEntry->SecConfig.PmfCfg.IPN[0][0]), &(pTmpEntry->SecConfig.PmfCfg.IPN[0][0]), LEN_WPA_TSC);
	os_move_mem(&(pEntry->SecConfig.PmfCfg.IPN[1][0]), &(pTmpEntry->SecConfig.PmfCfg.IPN[1][0]), LEN_WPA_TSC);
	os_move_mem(&(pEntry->SecConfig.PmfCfg.PmfTxTsc), &(pTmpEntry->SecConfig.PmfCfg.PmfTxTsc), LEN_WPA_TSC);
	os_move_mem(&(pEntry->SecConfig.PmfCfg.PmfRxTsc), &(pTmpEntry->SecConfig.PmfCfg.PmfRxTsc), LEN_WPA_TSC);
	pEntry->SecConfig.PmfCfg.SAQueryStatus = pTmpEntry->SecConfig.PmfCfg.SAQueryStatus;
	pEntry->SecConfig.PmfCfg.TransactionID = pTmpEntry->SecConfig.PmfCfg.TransactionID;
#endif /* DOT11W_PMF_SUPPORT */

#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
	os_move_mem(&(pEntry->SecConfig.HsUniGTK), &(pTmpEntry->SecConfig.HsUniGTK), LEN_MAX_GTK);
#endif /* defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT) */


	/* 802.1x */
#ifdef DOT1X_SUPPORT
	pEntry->SecConfig.own_ip_addr = pTmpEntry->SecConfig.own_ip_addr;
	pEntry->SecConfig.own_radius_port = pTmpEntry->SecConfig.own_radius_port;
	pEntry->SecConfig.retry_interval = pTmpEntry->SecConfig.retry_interval;
	pEntry->SecConfig.session_timeout_interval = pTmpEntry->SecConfig.session_timeout_interval;
	pEntry->SecConfig.quiet_interval = pTmpEntry->SecConfig.quiet_interval;
	os_move_mem(&(pEntry->SecConfig.EAPifname), &(pTmpEntry->SecConfig.EAPifname), pTmpEntry->SecConfig.EAPifname_len);
	pEntry->SecConfig.EAPifname_len = pTmpEntry->SecConfig.EAPifname_len;
	os_move_mem(&(pEntry->SecConfig.PreAuthifname), &(pTmpEntry->SecConfig.PreAuthifname), pTmpEntry->SecConfig.PreAuthifname_len);
	pEntry->SecConfig.PreAuthifname_len = pTmpEntry->SecConfig.PreAuthifname_len;
	pEntry->SecConfig.PreAuth = pTmpEntry->SecConfig.PreAuth;
	os_move_mem(&(pEntry->SecConfig.NasId), &(pTmpEntry->SecConfig.NasId), pTmpEntry->SecConfig.NasIdLen);
	pEntry->SecConfig.NasIdLen = pTmpEntry->SecConfig.NasIdLen;
	pEntry->SecConfig.radius_srv_num = pTmpEntry->SecConfig.radius_srv_num;
	os_move_mem(&(pEntry->SecConfig.radius_srv_info), &(pTmpEntry->SecConfig.radius_srv_info), (sizeof(RADIUS_SRV_INFO)*MAX_RADIUS_SRV_NUM));

#ifdef RADIUS_ACCOUNTING_SUPPORT
	pEntry->SecConfig.radius_acct_srv_num = pTmpEntry->SecConfig.radius_acct_srv_num;
	os_move_mem(&(pEntry->SecConfig.radius_acct_srv_info), &(pTmpEntry->SecConfig.radius_acct_srv_info), (sizeof(RADIUS_SRV_INFO)*MAX_RADIUS_SRV_NUM));
	/* int radius_request_cui; */
	pEntry->SecConfig.radius_acct_authentic = pTmpEntry->SecConfig.radius_acct_authentic;
	pEntry->SecConfig.acct_interim_interval = pTmpEntry->SecConfig.acct_interim_interval;
	pEntry->SecConfig.acct_enable = pTmpEntry->SecConfig.acct_enable;
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
/* TBD */

#endif /* DOT1X_SUPPORT */
	/* pEntry->SecConfig.PMKID_CacheIdx = pTmpEntry->SecConfig.PMKID_CacheIdx; */
#if defined(DOT1X_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
	pEntry->SecConfig.IEEE8021X = pTmpEntry->SecConfig.IEEE8021X; /* Only indicate if we are running in dynamic WEP mode (WEP+802.1x) */
#endif
	/* IE for WPA1/WPA2/WAPI */
	os_move_mem(&(pEntry->SecConfig.RSNE_Type), &(pTmpEntry->SecConfig.RSNE_Type), (sizeof(SEC_RSNIE_TYPE)*SEC_RSNIE_NUM));
	os_move_mem(&(pEntry->SecConfig.RSNE_EID), &(pTmpEntry->SecConfig.RSNE_EID), SEC_RSNIE_NUM);
	os_move_mem(&(pEntry->SecConfig.RSNE_Len), &(pTmpEntry->SecConfig.RSNE_Len), SEC_RSNIE_NUM);
	os_move_mem(&(pEntry->SecConfig.RSNE_Content), &(pTmpEntry->SecConfig.RSNE_Content), (SEC_RSNIE_NUM * MAX_LEN_OF_RSNIE));
	pEntry->SecConfig.LastGroupKeyId = pTmpEntry->SecConfig.LastGroupKeyId;
	os_move_mem(&(pEntry->SecConfig.LastGTK), &(pTmpEntry->SecConfig.LastGTK), LEN_MAX_GTK);

	/* os_move_mem(&(pEntry->PMKID), &(pTmpEntry->PMKID), LEN_PMKID); */
	pEntry->RSNIE_Len = pTmpEntry->RSNIE_Len;
	os_move_mem(&(pEntry->RSN_IE), &(pTmpEntry->RSN_IE), MAX_LEN_OF_RSNIE);
	pEntry->CMTimerRunning = pTmpEntry->CMTimerRunning;
	/* pEntry->PMKID_CacheIdx = pTmpEntry->PMKID_CacheIdx; */
	pEntry->PrivacyFilter = pTmpEntry->PrivacyFilter;	/* PrivacyFilter enum for 802.1X */
	return TRUE;
}

BOOLEAN pEntry_Clone(
	IN MAC_TABLE_ENTRY *pEntry,
	IN MAC_TABLE_ENTRY *pTmpEntry)

{

	pEntry->ClientStatusFlags = pTmpEntry->ClientStatusFlags;
	pEntry->cli_cap_flags = pTmpEntry->cli_cap_flags;

	pEntry->HTPhyMode.word = pTmpEntry->HTPhyMode.word;
	pEntry->MaxHTPhyMode.word = pTmpEntry->MaxHTPhyMode.word;
	pEntry->MinHTPhyMode.word = pTmpEntry->MinHTPhyMode.word;


	pEntry->isRalink = pTmpEntry->isRalink;
	pEntry->bIAmBadAtheros = pTmpEntry->bIAmBadAtheros;	/* Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection. */

#ifdef MWDS
	pEntry->MWDSEntry = pTmpEntry->MWDSEntry;		/* Determine if this entry act which MWDS role */
	pEntry->bSupportMWDS = pTmpEntry->bSupportMWDS;	/* Determine If own MWDS capability */
	pEntry->bEnableMWDS = pTmpEntry->bEnableMWDS;	/* Determine If do 3-address to 4-address */
#endif /* MWDS */

	/*
		STATE MACHINE Status
	*/
	pEntry->Sst = pTmpEntry->Sst;
	pEntry->AuthState = pTmpEntry->AuthState;	/* for SHARED KEY authentication state machine used only */


	/* WPA/WPA2 4-way database */
	/* ?? do we have chance that 4-way not complete and swap out ?? */
	pEntry->EnqueueEapolStartTimerRunning = pTmpEntry->EnqueueEapolStartTimerRunning;	/* Enqueue EAPoL-Start for triggering EAP SM */

	pEntry_Clone_SEC(pEntry, pTmpEntry);

	pEntry->RateLen = pTmpEntry->RateLen;
	pEntry->MaxSupportedRate = pTmpEntry->MaxSupportedRate;

	pEntry->bAutoTxRateSwitch = pTmpEntry->bAutoTxRateSwitch;

	pEntry->fgGband256QAMSupport = pTmpEntry->fgGband256QAMSupport;
	pEntry->SupportRateMode = pTmpEntry->SupportRateMode; /* 1: CCK 2:OFDM 4: HT, 8:VHT */
	pEntry->SupportCCKMCS = pTmpEntry->SupportCCKMCS;
	pEntry->SupportOFDMMCS = pTmpEntry->SupportOFDMMCS;
#ifdef DOT11_N_SUPPORT
	pEntry->SupportHTMCS = pTmpEntry->SupportHTMCS;
#ifdef DOT11_VHT_AC
	pEntry->SupportVHTMCS1SS = pTmpEntry->SupportVHTMCS1SS;
	pEntry->SupportVHTMCS2SS = pTmpEntry->SupportVHTMCS2SS;
	pEntry->SupportVHTMCS3SS = pTmpEntry->SupportVHTMCS3SS;
	pEntry->SupportVHTMCS4SS = pTmpEntry->SupportVHTMCS4SS;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */


#ifdef DOT11_N_SUPPORT
	os_move_mem((UINT8 *)&(pEntry->HTCapability), (UINT8 *)&(pTmpEntry->HTCapability), sizeof(HT_CAPABILITY_IE));
	pEntry->RXBAbitmap = pTmpEntry->RXBAbitmap;
	pEntry->TXBAbitmap = pTmpEntry->TXBAbitmap;
	pEntry->tx_amsdu_bitmap = pTmpEntry->tx_amsdu_bitmap;
	pEntry->TXAutoBAbitmap = pTmpEntry->TXAutoBAbitmap;
	pEntry->BADeclineBitmap = pTmpEntry->BADeclineBitmap;

	os_move_mem((UINT8 *)&(pEntry->BARecWcidArray), (UINT8 *)&(pTmpEntry->BARecWcidArray), sizeof(USHORT)*NUM_OF_TID);
	os_move_mem((UINT8 *)&(pEntry->BAOriWcidArray), (UINT8 *)&(pTmpEntry->BAOriWcidArray), sizeof(USHORT)*NUM_OF_TID);
	os_move_mem((UINT8 *)&(pEntry->BAOriSequence), (UINT8 *)&(pTmpEntry->BAOriSequence), sizeof(USHORT)*NUM_OF_TID);

	pEntry->MpduDensity = pTmpEntry->MpduDensity;
	pEntry->MaxRAmpduFactor = pTmpEntry->MaxRAmpduFactor;
	pEntry->AMsduSize = pTmpEntry->AMsduSize;
	pEntry->amsdu_limit_len = pTmpEntry->amsdu_limit_len;
	pEntry->amsdu_limit_len_adjust = pTmpEntry->amsdu_limit_len_adjust;
	pEntry->MmpsMode = pTmpEntry->MmpsMode;

#ifdef DOT11N_DRAFT3
	pEntry->BSS2040CoexistenceMgmtSupport = pTmpEntry->BSS2040CoexistenceMgmtSupport;
	pEntry->bForty_Mhz_Intolerant = pTmpEntry->bForty_Mhz_Intolerant;
#endif /* DOT11N_DRAFT3 */

#ifdef DOT11_VHT_AC
	os_move_mem((UINT8 *)&(pEntry->vht_cap_ie), (UINT8 *)&(pTmpEntry->vht_cap_ie), sizeof(VHT_CAP_IE));

	/* only take effect if ext_cap.operating_mode_notification = 1 */
	pEntry->force_op_mode = pTmpEntry->force_op_mode;
	pEntry->operating_mode = pTmpEntry->operating_mode;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */



#ifdef CONFIG_DOT11V_WNM
	pEntry->BssTransitionManmtSupport = pTmpEntry->BssTransitionManmtSupport;
#endif /* CONFIG_DOT11V_WNM */

	pEntry->bWscCapable = pTmpEntry->bWscCapable;

	pEntry->Receive_EapolStart_EapRspId = pTmpEntry->Receive_EapolStart_EapRspId;

#ifdef CONFIG_STA_SUPPORT
	pEntry->LastBeaconRxTime = pTmpEntry->LastBeaconRxTime;
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	os_move_mem(&pEntry->MdIeInfo, &pTmpEntry->MdIeInfo, sizeof(FT_MDIE_INFO));
	os_move_mem(&pEntry->FtIeInfo, &pTmpEntry->FtIeInfo, sizeof(FT_FTIE_INFO));
	os_move_mem(&pEntry->InitialMDIE, &pTmpEntry->InitialMDIE, 5);
	os_move_mem(&pEntry->InitialFTIE, &pTmpEntry->InitialFTIE, 256);
	pEntry->InitialFTIE_Len = pTmpEntry->InitialFTIE_Len;
	os_move_mem(&pEntry->FT_PMK_R0, &pTmpEntry->FT_PMK_R0, 32);
	os_move_mem(&pEntry->FT_PMK_R0_NAME, &pTmpEntry->FT_PMK_R0_NAME, 16);
	os_move_mem(&pEntry->FT_PMK_R1, &pTmpEntry->FT_PMK_R1, 32);
	os_move_mem(&pEntry->FT_PMK_R1_NAME, &pTmpEntry->FT_PMK_R1_NAME, 16);
	os_move_mem(&pEntry->PTK_NAME, &pTmpEntry->PTK_NAME, 16);
	os_move_mem(&pEntry->FT_UCipher, &pTmpEntry->FT_UCipher, 4);
	os_move_mem(&pEntry->FT_Akm, &pTmpEntry->FT_Akm, 4);
	os_move_mem(&pEntry->FT_PTK, &pTmpEntry->FT_PTK, LEN_MAX_PTK);
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	os_move_mem(&pEntry->RrmEnCap, &pTmpEntry->RrmEnCap, sizeof(RRM_EN_CAP_IE));
#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	pEntry->bBSSMantSTASupport = pEntry->bBSSMantSTASupport;
	pEntry->bDMSSTASupport = pEntry->bDMSSTASupport;
	pEntry->BTMQueryDialogToken = pEntry->BTMQueryDialogToken;
	pEntry->DisassociationImminent = pEntry->DisassociationImminent;
	pEntry->BSSTerminationIncluded = pEntry->BSSTerminationIncluded;
	#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11V_WNM_SUPPORT */


	os_move_mem((UINT8 *)&(pEntry->RaEntry), (UINT8 *)&(pTmpEntry->RaEntry), sizeof(RA_ENTRY_INFO_T));
	os_move_mem((UINT8 *)&(pEntry->RaInternal), (UINT8 *)&(pTmpEntry->RaInternal), sizeof(RA_INTERNAL_INFO_T));


	/* Restore the WTBL RAW from in-band cmd */
	os_move_mem((UINT8 *)&(pEntry->UWtblRaw), (UINT8 *)&(pTmpEntry->UWtblRaw), sizeof(CMD_STAREC_UWTBL_RAW_T));
	return TRUE;
}



UINT16 WtblTdd_ActiveList_SwapIn(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pInActEntry,
	STA_TR_ENTRY *pInActTrEntry)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *pTrEntry =  NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR segIdx, LastExtIdx;
	STA_TR_ENTRY *tr_entry = NULL;
	MAC_TABLE_ENTRY *pTmpEntry = NULL;
	INT retval = 0;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	NODE_INFO *pNode = NULL;
	UINT ConnectID = pInActEntry->wtblTddCtrl.ConnectID;

	MT_WTBL_TDD_TIME_LOG_IN(pAd, pAd->sm_swapin_ent_time);

	pNode = &pAd->wtblTddInfo.nodeInfo[ConnectID];
	LastExtIdx = pInActEntry->wtblTddCtrl.LastExtIdx;
	segIdx = pInActEntry->wtblTddCtrl.SegIdx;

	MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG,
			("%s(): segIdx=%u, LastExtIdx=%u, ConnectID=%u, pNode=%p , pInActEntry=%p --->\n",
			__func__, segIdx, LastExtIdx, ConnectID, pNode, pInActEntry));
	/*
		need to discuss , backup sequence, in MacTableInsertEntry() will clear the buffer again,
		but wtbl buffer is in pEntry backup.

		Prepare the buffer to prevent MacTableInsertEntry() --> WtblTdd_AcquireUcastWcid() -->
		WtblTdd_ActiveList_SwapOut() clean the buffer in inAct entry!
	*/
	retval = os_alloc_mem(pAd, (UCHAR **)&pTmpEntry, sizeof(MAC_TABLE_ENTRY));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Not enough memory for dynamic allocating pTmpEntry!!\n");
	} else {
		os_move_mem((UINT8 *)pTmpEntry, (UINT8 *)pInActEntry, sizeof(MAC_TABLE_ENTRY));
	}

	MT_WTBL_TDD_HEX_DUMP(pAd, "WtblTdd_ActiveList_SwapIn : pInActEntry->UWtblRaw.u4UWBLRaw ==>", (PUCHAR)&(pInActEntry->UWtblRaw.u4UWBLRaw),  (sizeof(pInActEntry->UWtblRaw.u4UWBLRaw)));

	pEntry = MacTableInsertEntry(pAd, pInActEntry->Addr, pInActEntry->wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" ---> alloc entry fail\n");
		os_free_mem(pTmpEntry);
		return WCID_INVALID;
	}

	/*
		need to discuss, backup sequence, in MacTableInsertEntry() will clear the buffer again,
		but wtbl buffer is in pEntry restore.
	*/
	if (retval == NDIS_STATUS_SUCCESS) {
		pEntry_Clone(pEntry, pTmpEntry);
		os_free_mem(pTmpEntry);
	} else {
		ASSERT(0);
	}

	pTrEntry = &tr_ctl->tr_entry[pEntry->wcid];

	wdev = pEntry->wdev;
	/* restore the pEntry -- START */
	pEntry->wtblTddCtrl.LastExtIdx = LastExtIdx;
	pEntry->wtblTddCtrl.SegIdx = segIdx;
	pEntry->wtblTddCtrl.state = WTBL_TDD_STA_SWAP_IN_ING;
	pTrEntry->wdev = wdev;



	HW_TDD_WCID_SWAP_IN(pAd, pEntry);
	{
	    RTMP_OS_TASK *pTask = &pAd->HwCtrl.HwCtrlTask;
	    RtmpMLMEUp(pTask);
	}

	pNode->tableMode = HW_TABLE;
	pNode->swLastIndex = LastExtIdx;
	pNode->swLastSegIndex = segIdx;
	pNode->hwWtblIndex = pEntry->wcid;
	pNode->swapInCnt++;
	pEntry->wtblTddCtrl.ConnectTime = pInActEntry->wtblTddCtrl.ConnectTime;
	pEntry->wtblTddCtrl.ConnectID = pInActEntry->wtblTddCtrl.ConnectID;

	/* unicast key cmd, we expect the AES/TKIP only */
	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

	/* restore the pEntry -- END */

	/* why need this , tmp off */
	
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->wtblTddInfo.acquireComplete, WTBL_TDD_WAIT_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"WAIT the SWAP IN timeout!\n");
	}

	/* release the backup zone */
	WtblTdd_InAct_MacTableDeleteEntry(pAd, pInActEntry->wtblTddCtrl.SegIdx, pInActEntry->wtblTddCtrl.LastExtIdx, pInActEntry->Addr);

	WtblTdd_ActiveList_ScoreDel(pAd, pEntry->wcid);


	MT_WTBL_TDD_TIME_LOG_OUT(pAd, pAd->sm_swapin_exit_time);

	return pEntry->wcid;
}


/* For Ext Non Active Entry Insert */
BOOLEAN WtblTdd_InAct_MacTableInsertEntry(
	RTMP_ADAPTER *pAd,
	UCHAR SegIdx,
	UINT16 inActWcid,
	UINT16 actWcid)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[actWcid];
	MAC_TABLE_ENTRY *pInActEntry = &pAd->MacTab.ContentExt[SegIdx][inActWcid];

	if (pEntry && !IS_ENTRY_NONE(pEntry)) {
		NODE_INFO *pNode = NULL;
		ULONG ConnectID = pEntry->wtblTddCtrl.ConnectID;
		pNode = &pAd->wtblTddInfo.nodeInfo[ConnectID];
		MacTableResetTimer(pAd, pEntry);
		NdisMoveMemory(pInActEntry, pEntry, sizeof(MAC_TABLE_ENTRY));
		MT_WTBL_TDD_HEX_DUMP(pAd, "WtblTdd_InAct_MacTableInsertEntry : pInActEntry->UWtblRaw.u4UWBLRaw ==>", (PUCHAR)&(pInActEntry->UWtblRaw.u4UWBLRaw),  (sizeof(pInActEntry->UWtblRaw.u4UWBLRaw)));
		NdisMoveMemory(pInActEntry->Addr, pEntry->Addr, MAC_ADDR_LEN);
		NdisMoveMemory(&pAd->MacTab.tr_entryExt[SegIdx][inActWcid],
			&tr_ctl->tr_entry[actWcid], sizeof(STA_TR_ENTRY));

		pInActEntry->EntryType = pEntry->EntryType;
		WtblTdd_InactiveList_HashAdd(pAd, pInActEntry);
		pAd->MacTab.SizeExt[SegIdx]++;

		pNode->tableMode = SW_TABLE;
		pNode->swLastIndex = inActWcid;
		pNode->swLastSegIndex = SegIdx;
		pNode->hwWtblIndex = actWcid;
		pNode->swapOutCnt++;

		pInActEntry->wtblTddCtrl.ConnectTime = pEntry->wtblTddCtrl.ConnectTime;
		pInActEntry->wtblTddCtrl.ConnectID = pEntry->wtblTddCtrl.ConnectID;

		pInActEntry->wtblTddCtrl.SegIdx = SegIdx;
		pInActEntry->wtblTddCtrl.LastExtIdx = inActWcid;

		pEntry->wtblTddCtrl.state = WTBL_TDD_STA_SWAP_OUT_ING;
		pEntry->wtblTddCtrl.LastWtblIdx = actWcid;
	} else {
		ASSERT(0);
	}

	return TRUE;

}

/* For Ext Non Active Entry Delete */
BOOLEAN WtblTdd_InAct_MacTableDeleteEntry(RTMP_ADAPTER *pAd, USHORT SegIdx, USHORT inAct_wcid, UCHAR *pAddr)
{
	MAC_TABLE_ENTRY *pInActEntry = &pAd->MacTab.ContentExt[SegIdx][inAct_wcid];
	STA_TR_ENTRY *pInActTrEntry = &pAd->MacTab.tr_entryExt[SegIdx][inAct_wcid];

	if (pInActEntry && !IS_ENTRY_NONE(pInActEntry) /*&& MAC_ADDR_EQUAL(pInActEntry->Addr, pAddr)*/) {
		/* release the backup zone */
		WtblTdd_InactiveList_HashDel(pAd, pInActEntry);
		pAd->MacTab.SizeExt[pInActEntry->wtblTddCtrl.SegIdx]--;
		RTMP_SEM_LOCK(&pInActEntry->wtblTddCtrl.enqMlmeCountLock);
		pInActEntry->wtblTddCtrl.enqMlmeCount = 0;
		RTMP_SEM_UNLOCK(&pInActEntry->wtblTddCtrl.enqMlmeCountLock);

		NdisZeroMemory(pInActEntry, sizeof(MAC_TABLE_ENTRY));
		SET_ENTRY_NONE(pInActEntry);
		NdisZeroMemory(pInActTrEntry, sizeof(STA_TR_ENTRY));
	}

	return TRUE;
}



VOID WtblTdd_ActiveList_ScoreDel(
	RTMP_ADAPTER *pAd,
	UCHAR wcid)
{
	NdisAcquireSpinLock(&pAd->wtblTddInfo.updateScoreLock);
	pAd->wtblTddInfo.wcidScore[wcid] = 0;
	pAd->wtblTddInfo.wcidScoreUsed[wcid] = FALSE;
	NdisReleaseSpinLock(&pAd->wtblTddInfo.updateScoreLock);
}

VOID WtblTdd_ActiveList_MlmeCal(
	RTMP_ADAPTER *pAd)
{
	/*
		pEntry->Factor:
		HTTRANSMIT_SETTING HTPhyMode
		RSSI_SAMPLE RssiSample;
		UINT32 LastTxRate;
		UINT32 LastRxRate;

		ULONG NoDataIdleCount;
		UINT16 StationKeepAliveCount;
		UINT32 StaConnectTime;
		UINT32 StaIdleTimeout;

		UINT32 OneSecTxNoRetryOkCount;
		UINT32 OneSecTxRetryOkCount;
		UINT32 OneSecTxFailCount;
		UINT32 OneSecRxLGICount;
		UINT32 OneSecRxSGICount;
		UINT32 ContinueTxFailCnt;

		USHORT NoBADataCountDown;
	*/
	UINT16 maxWcid = GET_MAX_UCAST_NUM(pAd);
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	UINT16 wcid = 1;
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;

	NdisAcquireSpinLock(&pAd->wtblTddInfo.updateScoreLock);
	/*
	  Weight(StaConnectTime, NoDataIdleCount)[0.5, 0.5]
	 */
	for (wcid = 1; wcid <= maxWcid; wcid++) {
		pEntry = &pAd->MacTab.Content[wcid];
		tr_entry = &tr_ctl->tr_entry[wcid];
		pAd->wtblTddInfo.wcidScore[wcid] = 0;

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) &&
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {

			pAd->wtblTddInfo.wcidScore[wcid] = (pEntry->StaConnectTime / 2) +
				(pEntry->NoDataIdleCount / 2);
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "WCID: %d ===================\n", pEntry->wcid);
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "RssiSample: %d\n", pEntry->RssiSample.AvgRssi[0]);
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "NoDataIdleCount: %lu\n", pEntry->NoDataIdleCount);
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "StaConnectTime: %d\n", pEntry->StaConnectTime);

			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"wcid: %d, state: %d, time: %d\n", wcid, pEntry->wtblTddCtrl.state,
				pEntry->wtblTddCtrl.guardTime);

			if ((pEntry->wtblTddCtrl.state == WTBL_TDD_STA_IDLE)
				&& pEntry->wtblTddCtrl.guardTime > 0)
				pEntry->wtblTddCtrl.guardTime--;
		}
	}

	NdisReleaseSpinLock(&pAd->wtblTddInfo.updateScoreLock);
}

BOOLEAN WtblTdd_InactiveList_EntryDel(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry)
{
	/* STA_TR_ENTRY *pTrEntry = &pAd->MacTab.tr_entry[pEntry->wcid]; */
	/* dont' zero the tr_entry due to tr_entry is reused by multiple STA, tr_entry->StaRec.list.Next, tr_entry->StaRec.list.Prev  */
	WtblTdd_InactiveList_HashDel(pAd, pEntry);
	pAd->MacTab.SizeExt[pEntry->wtblTddCtrl.SegIdx]--;
	NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
	/* NdisZeroMemory(pTrEntry , sizeof(STA_TR_ENTRY)); */
	SET_ENTRY_NONE(pEntry);
	return TRUE;
}

MAC_TABLE_ENTRY *WtblTdd_InactiveList_Lookup(
	RTMP_ADAPTER *pAd,
	UCHAR *pAddr)
{
	UINT HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.HashExt[HashIdx];

	while (pEntry && !IS_ENTRY_NONE(pEntry)) {
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
			break;
		else
			pEntry = pEntry->pNext;
	}

	return pEntry;

}

NODE_INFO *WtblTdd_InfoNode_Lookup(
	RTMP_ADAPTER *pAd,
	UCHAR *pAddr)
{
	UINT HashIdx;
	NODE_INFO *pNode = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pNode = pAd->wtblTddInfo.NodeHash[HashIdx];

	while (pNode && pNode->isValid) {
		if (MAC_ADDR_EQUAL(pNode->Addr, pAddr))
			break;
		else
			pNode = pNode->pNext;
	}

	return pNode;
}

BOOLEAN WtblTdd_RxPacket_BlockList(
	RTMP_ADAPTER *pAd,
	RX_BLK *rx_blk)
{
	FRAME_CONTROL *fc = (FRAME_CONTROL *) rx_blk->FC;

	/* skip mgmt / ctrl */
	if (((fc->Type == FC_TYPE_DATA) && (fc->SubType == SUBTYPE_QOS_NULL)) ||
	   ((fc->Type == FC_TYPE_DATA) && (fc->SubType == SUBTYPE_DATA_NULL)) ||
	   (fc->Type == FC_TYPE_MGMT) ||
	   (fc->Type == FC_TYPE_CNTL))
	   return TRUE;

	return FALSE;
}

UCHAR WtblTdd_Entry_RxPacket(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pInActEntry)
{
	PWTBL_TDD_MSG msg;

	MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG,
			("%s(): ---> (%02x:%02x:%02x:%02x:%02x:%02x) state=%d\n",
			__func__, PRINT_MAC(pInActEntry->Addr),
			pInActEntry->wtblTddCtrl.state));

	if ((pInActEntry->wtblTddCtrl.state == WTBL_TDD_STA_IDLE) ||
		(pInActEntry->wtblTddCtrl.state == WTBL_TDD_STA_SWAP_IN_DONE)) {
		pInActEntry->wtblTddCtrl.state = WTBL_TDD_STA_SWAP_REQ_ING;
		os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(WTBL_TDD_MSG));
		NdisMoveMemory(msg->addr, pInActEntry->Addr, MAC_ADDR_LEN);
		msg->action = WTBL_TDD_ACTION_RX;
		if (pInActEntry->wtblTddCtrl.enqMlmeCount == 0) {
			if (MlmeEnqueueWithWdev(pAd, WTBL_TDD_FSM, WTBL_TDD_FSM_SWAP_REQ,
				sizeof(WTBL_TDD_MSG), msg, 0, wdev)) {
				RTMP_SEM_LOCK(&pInActEntry->wtblTddCtrl.enqMlmeCountLock);
				pInActEntry->wtblTddCtrl.enqMlmeCount++;
				RTMP_SEM_UNLOCK(&pInActEntry->wtblTddCtrl.enqMlmeCountLock);
				RTMP_MLME_HANDLER(pAd);
			} else {
					MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"mlmeQ full \n");
				pAd->wtblTddInfo.MlmeQFullCounter++;
			}
			RTMP_SEM_LOCK(&pAd->wtblTddInfo.swapReqEventQueLock);
			InsertTailQueue(&pAd->wtblTddInfo.swapReqEventQue, &msg->queueEntry);
			RTMP_SEM_UNLOCK(&pAd->wtblTddInfo.swapReqEventQueLock);
		} else {
			pAd->wtblTddInfo.MlmeQInCounter++;
		}

		/* log the first trigger pkt time */
		if (pAd->rx_trigger_log) {
			pAd->rx_trigger_log = FALSE;
			pAd->rx_trigger_time = jiffies;

			/* also forbid tx's log */
			pAd->tx_trigger_log = FALSE;
			pAd->tx_trigger_time = 0;

			/* log the mlme handle time */
			pAd->mlme_exec_log = TRUE;
			pAd->mlme_exec_time = 0;
		}
	}

	MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG,
			("%s(): <---\n", __func__));

	return TRUE;
}

UCHAR WtblTdd_Entry_TxPacket(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pInActEntry)
{
	PWTBL_TDD_MSG msg;

	MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG,
			("%s(): ---> (%02x:%02x:%02x:%02x:%02x:%02x), state=%d, enqMlmeCount=%d\n",
			__func__, PRINT_MAC(pInActEntry->Addr), pInActEntry->wtblTddCtrl.state, pInActEntry->wtblTddCtrl.enqMlmeCount));

	if ((pInActEntry->wtblTddCtrl.state == WTBL_TDD_STA_IDLE) ||
		(pInActEntry->wtblTddCtrl.state == WTBL_TDD_STA_SWAP_IN_DONE)) {

		pInActEntry->wtblTddCtrl.state = WTBL_TDD_STA_SWAP_REQ_ING;

		os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(WTBL_TDD_MSG));
		NdisMoveMemory(msg->addr, pInActEntry->Addr, MAC_ADDR_LEN);
		msg->action = WTBL_TDD_ACTION_TX;

		if (pInActEntry->wtblTddCtrl.enqMlmeCount == 0) {
			if (MlmeEnqueueWithWdev(pAd, WTBL_TDD_FSM, WTBL_TDD_FSM_SWAP_REQ,
				sizeof(WTBL_TDD_MSG), msg, 0, wdev)) {
				RTMP_SEM_LOCK(&pInActEntry->wtblTddCtrl.enqMlmeCountLock);
				pInActEntry->wtblTddCtrl.enqMlmeCount++;
				RTMP_SEM_UNLOCK(&pInActEntry->wtblTddCtrl.enqMlmeCountLock);
				RTMP_MLME_HANDLER(pAd);
			} else {
					MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"mlmeQ full\n");
				pAd->wtblTddInfo.MlmeQFullCounter++;
			}
			RTMP_SEM_LOCK(&pAd->wtblTddInfo.swapReqEventQueLock);
			InsertTailQueue(&pAd->wtblTddInfo.swapReqEventQue, &msg->queueEntry);
			RTMP_SEM_UNLOCK(&pAd->wtblTddInfo.swapReqEventQueLock);
		} else {
			pAd->wtblTddInfo.MlmeQInCounter++;
		}

		/* log the first trigger pkt time */
		if (pAd->tx_trigger_log) {
			pAd->tx_trigger_log = FALSE;
			pAd->tx_trigger_time = jiffies;

			/* also forbid rx's log */
			pAd->rx_trigger_log = FALSE;
			pAd->rx_trigger_time = 0;

			/* log the mlme handle time */
			pAd->mlme_exec_log = TRUE;
			pAd->mlme_exec_time = 0;
		}
	}

	MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG,
			("%s(): <---\n", __func__));

	return TRUE;
}

UCHAR WtblTdd_Entry_Init(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	UCHAR SegIdx)
{
	UINT idx = 0;
	NODE_INFO *pNode = NULL;

	pEntry->wtblTddCtrl.SegIdx = SegIdx;
	NdisGetSystemUpTime(&pEntry->wtblTddCtrl.ConnectTime);

	/* Search the Entry in NodeList first, else find the space to store the info. */
	pNode = WtblTdd_InfoNode_Lookup(pAd, pEntry->Addr);
	if (pNode) {
		/* Already in the node table means in swap-in path */
		return TRUE;
	} else {
		for (idx = 0; idx < pAd->wtblTddInfo.nodeInfoTabSize; idx++) {
			pNode = &pAd->wtblTddInfo.nodeInfo[idx];

			if (pNode->isValid == FALSE) {
				WtblTdd_InfoNode_HashAdd(pAd, pNode);
				break;
			}
		}
	}

	/* MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): ConnectID=%u <---\n", __func__, idx)); */
	pEntry->wtblTddCtrl.ConnectID = idx;
	NdisAcquireSpinLock(&pAd->wtblTddInfo.nodeInfoLock);
	pNode->isValid = TRUE;
	NdisReleaseSpinLock(&pAd->wtblTddInfo.nodeInfoLock);

	return TRUE;
}

TABLE_MODE WtblTdd_Entry_DeInit(
	RTMP_ADAPTER *pAd,
	USHORT wcid,
	UCHAR *pAddr)

{
	TABLE_MODE tableMode = HW_TABLE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	NODE_INFO *pNode = NULL;
	UINT idx = 0;

	pEntry = WtblTdd_InactiveList_Lookup(pAd, pAddr);
	if (pEntry) {
		idx = pEntry->wtblTddCtrl.ConnectID;
		WtblTdd_InactiveList_EntryDel(pAd, pEntry);
		tableMode = SW_TABLE;
	} else {
		if (VALID_UCAST_ENTRY_WCID(pAd, wcid)) {
			pEntry = &pAd->MacTab.Content[wcid];

			idx = pEntry->wtblTddCtrl.ConnectID;
			pEntry->wtblTddCtrl.guardTime = 0;
			WtblTdd_ActiveList_ScoreDel(pAd, pEntry->wcid);
		} else {
			ASSERT(0);
			return SW_TABLE;
		}
	}

	pNode = &pAd->wtblTddInfo.nodeInfo[idx];


	/* MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): ConnectID=%u<---\n", __func__, idx)); */
	WtblTdd_InfoNode_HashDel(pAd, pNode);

	NdisAcquireSpinLock(&pAd->wtblTddInfo.nodeInfoLock);
	pNode->isValid = FALSE;
	pNode->tableMode = SW_TABLE;
	pNode->swLastIndex = 0;
	pNode->swLastSegIndex = 0;
	pNode->hwWtblIndex = 0;
	pNode->pktCnt = 0;
	pNode->swapInCnt = 0;
	pNode->swapOutCnt = 0;

	NdisZeroMemory(pNode, sizeof(NODE_INFO));
	NdisReleaseSpinLock(&pAd->wtblTddInfo.nodeInfoLock);

	MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"---> (%02x:%02x:%02x:%02x:%02x:%02x) ==> (%d), TBL Mode(%s)\n",
			PRINT_MAC(pAddr), wcid, (tableMode ? "HW" : "SW")));
	return tableMode;
}

UINT16 WtblTdd_AcquireUcastWcid(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR  *useExt,
	UCHAR *SegIdx)
{
	UINT16 i = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR state = WTBL_TDD_STA_IDLE;

	*useExt = 0;

	i = HcAcquireUcastWcid(pAd, wdev);

	/* Normal Segment full goto extend part. */
	if (i == WCID_INVALID) {
		UINT16 srcWcid = WCID_INVALID, destWcid = WCID_INVALID;
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Active List Entry full!\n";
		srcWcid = WtblTdd_InactiveList_FindSpace(pAd, SegIdx);
		if (srcWcid != WCID_INVALID) {
			/* select one from active list and move to inactive */
			destWcid = WtblTdd_ActiveList_SelectBad(pAd);
			if (destWcid == WCID_INVALID) {
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"<--- can't select one from ActiveList\n";
				return WCID_INVALID;
			}

			pEntry = &pAd->MacTab.Content[destWcid];
			state = pEntry->wtblTddCtrl.state;

			MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG,
				("%s(): <--- state %d\n", __func__, state));

			WtblTdd_ActiveList_SwapOut(pAd, *SegIdx, srcWcid, destWcid);

			/* Clear the WCID Score to prevent double choosen */
			WtblTdd_ActiveList_ScoreDel(pAd, destWcid);

			*useExt = 1;
			return destWcid;
		} else {
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					" <--- inActiveList Full, No More STA!\n");
		}
		return WCID_INVALID;
	}

	return i;
}

INT WtblTdd_DumpTab(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	NODE_INFO *pNode = NULL;
	UINT segIdx = 0, idx = 0;
	USHORT act;

	act = (USHORT) os_str_tol(arg, 0, 10);

	switch (act) {
	case 1: /* dump */
	{
		MTWF_PRINT("enabled: %d\n", pAd->wtblTddInfo.enabled);

		MTWF_PRINT("nodeInfoTabSize: %d\n", pAd->wtblTddInfo.nodeInfoTabSize);
		MTWF_PRINT("GET_MAX_UCAST_NUM: %d\n", GET_MAX_UCAST_NUM(pAd));


		for (segIdx = 0; segIdx < WTBL_TDD_SW_MAC_TAB_SEG_NUM; segIdx++) {
			if (pAd->MacTab.SizeExt[segIdx]) {
				MTWF_PRINT("==> SegPart%d: \n", segIdx);
				for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
					pEntry = &pAd->MacTab.ContentExt[segIdx][idx];
					if (IS_ENTRY_CLIENT(pEntry))
						MTWF_PRINT("%d-%d:%d=%d (%02x:%02x:%02x:%02x:%02x:%02x), EntryType=%x\n",
							 segIdx, idx, pEntry->wcid,
							 pEntry->wtblTddCtrl.LastWtblIdx, PRINT_MAC(pEntry->Addr), pEntry->EntryType);
				}

				MTWF_PRINT(	"<== SegPart%d: Num(%d)\n", segIdx,
						pAd->MacTab.SizeExt[segIdx]);
			}
		}

		for (idx = 0; idx < MAX_LEN_OF_NODE_TABLE; idx++) {
			pNode = &pAd->wtblTddInfo.nodeInfo[idx];
			if (pNode->isValid == TRUE) {
				MTWF_PRINT("== Node %d (TBL Mode %s) HW(%d) SW(%d,%d) ==> pktCnt %ld (SwapIn %ld, SwapOut %ld)\n", idx, (pNode->tableMode ? "H/W" : "S/W"),
						pNode->hwWtblIndex, pNode->swLastIndex, pNode->swLastSegIndex, pNode->pktCnt, pNode->swapInCnt, pNode->swapOutCnt);
			}
		}

		MTWF_PRINT("txMissCounter ==> %d\n", pAd->wtblTddInfo.txMissCounter);
		MTWF_PRINT("rxMissCounter ==> %d\n", pAd->wtblTddInfo.rxMissCounter);
		MTWF_PRINT("MlmeQFullCounter ==> %d\n", pAd->wtblTddInfo.MlmeQFullCounter);
	}
	break;
	case 0: /* clean Ext Table */
	{
		for (segIdx = 0; segIdx < WTBL_TDD_SW_MAC_TAB_SEG_NUM; segIdx++) {
			for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
				pEntry = &pAd->MacTab.ContentExt[segIdx][idx];
				WtblTdd_InAct_MacTableDeleteEntry(pAd, segIdx, idx, pEntry->Addr);
			}
			pAd->MacTab.SizeExt[segIdx] = 0;
		}
	}
	break;

	case 2: /* dump all for debug */
	{
		for (segIdx = 0; segIdx < WTBL_TDD_SW_MAC_TAB_SEG_NUM; segIdx++) {
				MTWF_PRINT("==> SegPart%d: \n", segIdx);
				for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
					pEntry = &pAd->MacTab.ContentExt[segIdx][idx];
					/* if (IS_ENTRY_CLIENT(pEntry)) */
						MTWF_PRINT("%d-%d:%d=%d (%02x:%02x:%02x:%02x:%02x:%02x), EntryType=%x\n",
							 segIdx, idx, pEntry->wcid, pEntry->wtblTddCtrl.LastWtblIdx, PRINT_MAC(pEntry->Addr), pEntry->EntryType);
				}

				MTWF_PRINT("<== SegPart%d: Num(%d)\n", segIdx, pAd->MacTab.SizeExt[segIdx]);
		}
	}
	break;

	case 3: /* dump hash for debug */
	{
		MAC_TABLE_ENTRY *pEntry = NULL;
		UINT sta_cnt = 0;
		for (idx = 0; idx < HASH_TABLE_SIZE; idx++) {
			pEntry = pAd->MacTab.HashExt[idx];

			MTWF_PRINT("HashIdx[%d] : \n", idx);
			while (pEntry) {
					sta_cnt++;
					MTWF_PRINT("\t sta_cnt : %d, wcid : %d, (%02x:%02x:%02x:%02x:%02x:%02x), EntryType=%x\n",
						 sta_cnt, pEntry->wcid, PRINT_MAC(pEntry->Addr), pEntry->EntryType);
				pEntry = pEntry->pNext;
			}
		}
	}
	break;

	default:
		MTWF_PRINT("usage : [0] clean, [1] dump valid , [2] dump all dbg, [3] dump hash dbg !\n");
		break;
	}

	return TRUE;
}

INT WtblTdd_TestAction(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.ContentExt[1][0];
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	UINT16 destWcid = 0;
	USHORT act;

	act = (USHORT) os_str_tol(arg, 0, 10);

	if (act == WTBL_TDD_ACTION_TX) {
		/* swap out */
		UINT16 maxWcid = GET_MAX_UCAST_NUM(pAd);
		UINT16 wcid = 1;
		MAC_TABLE_ENTRY *pEntry = NULL;
		STA_TR_ENTRY *tr_entry = NULL;
		for (wcid = maxWcid; wcid > 0; wcid--) {
			pEntry = &pAd->MacTab.Content[wcid];
			tr_entry = &tr_ctl->tr_entry[wcid];

			if (IS_ENTRY_CLIENT(pEntry)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					" ---> wcid%d\n", wcid);
				destWcid = wcid;
				break;
			}
		}

		WtblTdd_ActiveList_SwapOut(pAd, 0, 0, destWcid);

		pEntry->wtblTddCtrl.guardTime = 0;

		/* log the first trigger pkt time */
		pAd->rx_trigger_log = TRUE;
		pAd->rx_trigger_time = 0;
		pAd->tx_trigger_log = TRUE;
		pAd->tx_trigger_time = 0;
		pAd->mlme_exec_log = FALSE;
		pAd->mlme_exec_time = 0;
	} else {
		/* swap in */
		if (pEntry) {
			WtblTdd_Entry_RxPacket(pAd, pEntry->wdev, pEntry);
		}
	}

	return TRUE;
}

VOID WtblTdd_Init(RTMP_ADAPTER *pAd)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT16 idx = 0, segIdx = 0;

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> WtblTdd_Init()\n");

	/* let profile to enable / disable */
	SET_WTBL_TDD_DISABLED(pAd);

	pAd->wtbl_dbg_level = 0; /* WTBL_TDD_DBG_STATE_FLAG; */

	RTMP_OS_INIT_COMPLETION(&pAd->wtblTddInfo.acquireComplete);
	RTMP_OS_INIT_COMPLETION(&pAd->wtblTddInfo.swapOutComplete);


	for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
		pEntry = &pAd->MacTab.Content[idx];

		pEntry->wtblTddCtrl.state = WTBL_TDD_STA_IDLE;
	}

	for (segIdx = 0; segIdx < WTBL_TDD_SW_MAC_TAB_SEG_NUM; segIdx++) {
		for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
			pEntry = &pAd->MacTab.ContentExt[segIdx][idx];
			SET_ENTRY_NONE(pEntry);
			NdisAllocateSpinLock(pAd, &pEntry->wtblTddCtrl.enqMlmeCountLock);
		}
	}

	NdisAllocateSpinLock(pAd, &pAd->MacTab.HashExtLock);
	NdisAllocateSpinLock(pAd, &pAd->wtblTddInfo.updateScoreLock);
	NdisAllocateSpinLock(pAd, &pAd->wtblTddInfo.nodeInfoLock);

	NdisAllocateSpinLock(pAd, &pAd->wtblTddInfo.txPendingListLock);
	InitializeQueueHeader(&pAd->wtblTddInfo.txPendingList);

	InitializeQueueHeader(&pAd->wtblTddInfo.swapReqEventQue);
	NdisAllocateSpinLock(pAd, &pAd->wtblTddInfo.swapReqEventQueLock);

	pAd->wtblTddInfo.nodeInfoTabSize = GET_MAX_UCAST_NUM(pAd) +
		(MAX_LEN_OF_MAC_TABLE * WTBL_TDD_SW_MAC_TAB_SEG_NUM);

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"<-- WtblTdd_Init()\n");
}


VOID WtblTdd_DeInit(RTMP_ADAPTER *pAd)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT idx = 0, segIdx = 0, node_idx = 0, count = 0;
	NODE_INFO *pNode = NULL;
	PNDIS_PACKET pPacket = NULL;
	QUEUE_HEADER *pQue = NULL;
	QUEUE_HEADER *pEventQue = NULL;
	QUEUE_ENTRY *pQEntry = NULL;
	PWTBL_TDD_MSG msg = NULL;
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> WtblTdd_DeInit()\n");

	/* free pending pkts */
       RTMP_SEM_LOCK(&pAd->wtblTddInfo.txPendingListLock);
	pQue = &pAd->wtblTddInfo.txPendingList;
	count = pAd->wtblTddInfo.txPendingList.Number;
	while (pQue->Head) {
		pQEntry = RemoveHeadQueue(pQue);

		pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

		if (pPacket) {
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}

		idx++;
		if (idx == count)
			break;
       }
       RTMP_SEM_UNLOCK(&pAd->wtblTddInfo.txPendingListLock);

	/* free pending events */
	RTMP_SEM_LOCK(&pAd->wtblTddInfo.swapReqEventQueLock);
	pEventQue = &pAd->wtblTddInfo.swapReqEventQue;
	count = pAd->wtblTddInfo.swapReqEventQue.Number;
	while (pEventQue->Head) {
		pQEntry = RemoveHeadQueue(pEventQue);
		msg  = (PWTBL_TDD_MSG)pQEntry;

		if (msg)
			os_free_mem(msg);

		idx++;
		if (idx == count)
			break;
	}
	RTMP_SEM_UNLOCK(&pAd->wtblTddInfo.swapReqEventQueLock);

	/* re-init MacTab */
	for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
		pEntry = &pAd->MacTab.Content[idx];
		pEntry->wtblTddCtrl.state = WTBL_TDD_STA_IDLE;
	}

	/* re-init MacTab.ContentExt */
	/* some STA may not de-init via WtblTdd_Entry_DeInit() if abnormal disconnect */
	for (segIdx = 0; segIdx < WTBL_TDD_SW_MAC_TAB_SEG_NUM; segIdx++) {
		for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
			pEntry = &pAd->MacTab.ContentExt[segIdx][idx];
			if (pEntry) {
				node_idx = pEntry->wtblTddCtrl.ConnectID;
				pEntry->wtblTddCtrl.guardTime = 0;
				WtblTdd_ActiveList_ScoreDel(pAd, pEntry->wcid);

				pNode = &pAd->wtblTddInfo.nodeInfo[node_idx];
				WtblTdd_InfoNode_HashDel(pAd, pNode);
				WtblTdd_InactiveList_EntryDel(pAd, pEntry);
			}

			NdisFreeSpinLock(&pEntry->wtblTddCtrl.enqMlmeCountLock);
		}
	}
	NdisFreeSpinLock(&pAd->MacTab.HashExtLock);
	NdisFreeSpinLock(&pAd->wtblTddInfo.updateScoreLock);
	NdisFreeSpinLock(&pAd->wtblTddInfo.nodeInfoLock);

	NdisFreeSpinLock(&pAd->wtblTddInfo.txPendingListLock);
	InitializeQueueHeader(&pAd->wtblTddInfo.txPendingList);

	InitializeQueueHeader(&pAd->wtblTddInfo.swapReqEventQue);
	NdisFreeSpinLock(&pAd->wtblTddInfo.swapReqEventQueLock);

	pAd->wtblTddInfo.nodeInfoTabSize = GET_MAX_UCAST_NUM(pAd) +
		(MAX_LEN_OF_MAC_TABLE * WTBL_TDD_SW_MAC_TAB_SEG_NUM);


	/* let profile to enable / disable */
	SET_WTBL_TDD_DISABLED(pAd);

	RTMP_OS_EXIT_COMPLETION(&pAd->wtblTddInfo.acquireComplete);
	RTMP_OS_EXIT_COMPLETION(&pAd->wtblTddInfo.swapOutComplete);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<-- WtblTdd_DeInit()\n");
}

