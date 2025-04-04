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
 ***************************************************************************

    Module Name:
    fsm_sync.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/

#include "rt_config.h"


struct _auth_api_ops ap_auth_api;


static BOOLEAN ap_peer_auth_sanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	IN AUTH_FRAME_INFO * auth_info)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(auth_info->addr1,  &Fr->Hdr.Addr1);		/* BSSID */
	COPY_MAC_ADDR(auth_info->addr2,  &Fr->Hdr.Addr2);		/* SA */
	/* TODO: shiang-usw, how about the endian issue here?? */
	NdisMoveMemory(&auth_info->auth_alg,    &Fr->Octet[0], 2);
	NdisMoveMemory(&auth_info->auth_seq,    &Fr->Octet[2], 2);
	NdisMoveMemory(&auth_info->auth_status, &Fr->Octet[4], 2);

	if (auth_info->auth_alg == AUTH_MODE_OPEN) {
		if (auth_info->auth_seq == 1 || auth_info->auth_seq == 2)
			return TRUE;
		else {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " fail - wrong Seg# (=%d)\n",
					 auth_info->auth_seq);
			return FALSE;
		}
	} else if (auth_info->auth_alg == AUTH_MODE_KEY) {
		if (auth_info->auth_seq == 1 || auth_info->auth_seq == 4)
			return TRUE;
		else if (auth_info->auth_seq == 2 || auth_info->auth_seq == 3) {
			NdisMoveMemory(auth_info->Chtxt, &Fr->Octet[8], CIPHER_TEXT_LEN);
			return TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " fail - wrong Seg# (=%d)\n",
					 auth_info->auth_seq);
			return FALSE;
		}
	}

#ifdef DOT11R_FT_SUPPORT
	else if (auth_info->auth_alg == AUTH_MODE_FT) {
		PEID_STRUCT eid_ptr;
		UCHAR *Ptr;
		UCHAR WPA2_OUI[3] = {0x00, 0x0F, 0xAC};
		PFT_INFO pFtInfo = &auth_info->FtInfo;

		NdisZeroMemory(pFtInfo, sizeof(FT_INFO));
		Ptr = &Fr->Octet[6];
		eid_ptr = (PEID_STRUCT) Ptr;

		/* get variable fields from payload and advance the pointer */
		while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)Fr + MsgLen)) {
			switch (eid_ptr->Eid) {
			case IE_FT_MDIE:
				if (FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo) == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"wrong IE_FT_MDIE\n");
					return FALSE;
				}
				break;

			case IE_FT_FTIE:
				if (FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo) == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"wrong IE_FT_FTIE\n");
					return FALSE;
				}
				break;

			case IE_FT_RIC_DATA:

				/* record the pointer of first RDIE. */
				if (pFtInfo->RicInfo.pRicInfo == NULL) {
					pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
					pFtInfo->RicInfo.Len = ((UCHAR *)Fr + MsgLen)
										   - (UCHAR *)eid_ptr + 1;
				}

				if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
					NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
								   &eid_ptr->Eid, eid_ptr->Len + 2);
					pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
				} else {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"wrong IE_FT_RIC_DATA\n");
					return FALSE;
				}

				break;

			case IE_FT_RIC_DESCRIPTOR:
				if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
					NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
								   &eid_ptr->Eid, eid_ptr->Len + 2);
					pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
				} else {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"wrong IE_FT_RIC_DESCRIPTOR\n");
					return FALSE;
				}

				break;

			case IE_RSN:
				if (parse_rsn_ie(eid_ptr)) {
					if (NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI))) {
						NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
						pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
					}
				} else {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"wrong IE_RSN\n");
					return FALSE;
				}

				break;

			default:
				break;
			}

			eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
		}
	}

#endif /* DOT11R_FT_SUPPORT */
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef DOT11_SAE_SUPPORT
	else if (auth_info->auth_alg == AUTH_MODE_SAE) {
		if (auth_info->auth_seq != SAE_COMMIT_SEQ && auth_info->auth_seq != SAE_CONFIRM_SEQ)
			return FALSE;
	}
#endif /* DOT11_SAE_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/
#ifdef OCE_FILS_SUPPORT
	else if (auth_info->auth_alg == AUTH_MODE_FILS) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, " Recv FILS Auth from ("MACSTR")\n",
				 MAC2STR(auth_info->addr2));
		return TRUE;
	}
#endif /* OCE_FILS_SUPPORT */
	else {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, " fail - wrong algorithm (=%d)\n",
				 auth_info->auth_alg);
		return FALSE;
	}

	return TRUE;
}

VOID ap_mlme_broadcast_deauth_req_action(
		IN PRTMP_ADAPTER pAd,
		IN MLME_QUEUE_ELEM * Elem)
{
	MLME_BROADCAST_DEAUTH_REQ_STRUCT	*pInfo;
	HEADER_802_11			Hdr;
	PUCHAR					pOutBuffer = NULL;
	NDIS_STATUS				NStatus;
	ULONG					FrameLen = 0;
	MAC_TABLE_ENTRY			*pEntry;
	UCHAR					apidx = 0;
	struct wifi_dev *wdev;
	int wcid, startWcid;

	startWcid = 1;
	pInfo = (PMLME_BROADCAST_DEAUTH_REQ_STRUCT)Elem->Msg;
	if (!MAC_ADDR_EQUAL(pInfo->Addr, BROADCAST_ADDR))
		return;
	wdev = pInfo->wdev;
	apidx = wdev->func_idx;
	for (wcid = startWcid; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = &pAd->MacTab.Content[wcid];
		if (pEntry->wdev != wdev)
			continue;
		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG,
			pEntry->Addr, 0, 0);
		ApLogEvent(pAd, pInfo->Addr, EVENT_DISASSOCIATED);
#ifdef MAP_R2
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
			wapp_handle_sta_disassoc(pAd, wcid, pInfo->Reason);
#endif
		MacTableDeleteEntry(pAd, wcid, pEntry->Addr);
	}
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;
	MgtMacHeaderInit(pAd, &Hdr, SUBTYPE_DEAUTH, 0, pInfo->Addr,
					pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
					pAd->ApCfg.MBSSID[apidx].wdev.bssid);
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(HEADER_802_11), &Hdr,
					  2, &pInfo->Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
}

VOID ap_mlme_deauth_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	MLME_DISCONNECT_STRUCT	*pInfo; /* snowpin for cntl mgmt */
	HEADER_802_11			Hdr;
	PUCHAR					pOutBuffer = NULL;
	NDIS_STATUS				NStatus;
	ULONG					FrameLen = 0;
	MAC_TABLE_ENTRY			*pEntry;
	UCHAR					apidx;

	pInfo = (MLME_DISCONNECT_STRUCT *)Elem->Msg; /* snowpin for cntl mgmt */
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, " ---> wcid = %d\n", Elem->Wcid);

	if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Valid unicast entry\n");
		pEntry = &pAd->MacTab.Content[Elem->Wcid];

		if (!pEntry)
			return;

		/* send wireless event - for deauthentication */
		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, pInfo->addr, 0, 0);
		ApLogEvent(pAd, pInfo->addr, EVENT_DISASSOCIATED);
		apidx = pEntry->func_tb_idx;
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH, pInfo->reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				pInfo->reason);
#endif
#ifdef MAP_R2
		if (IS_ENTRY_CLIENT(pEntry) && IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
			wapp_handle_sta_disassoc(pAd, Elem->Wcid, pInfo->reason);
#endif
		/* 1. remove this STA from MAC table */
		MacTableDeleteEntry(pAd, Elem->Wcid, pInfo->addr);
		/* 2. send out DE-AUTH request frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				 "AUTH - Send DE-AUTH req to "MACSTR"\n",
				  MAC2STR(pInfo->addr));
		MgtMacHeaderInit(pAd, &Hdr, SUBTYPE_DEAUTH, 0, pInfo->addr,
						 pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
						 pAd->ApCfg.MBSSID[apidx].wdev.bssid);
		MakeOutgoingFrame(pOutBuffer,				&FrameLen,
						  sizeof(HEADER_802_11),	&Hdr,
						  2,						&pInfo->reason,
						  END_OF_ARGS);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
#ifdef WH_EVENT_NOTIFIER
		{
			EventHdlr pEventHdlrHook = NULL;

			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
			if (pEventHdlrHook && pEntry->wdev)
				pEventHdlrHook(pAd, pEntry->wdev, pInfo->addr, Elem->Channel);
		}
#endif /* WH_EVENT_NOTIFIER */
	} else {
		ap_mlme_broadcast_deauth_req_action(pAd, Elem);
	}

	return;
}

VOID ap_peer_deauth_action(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem)
{
	UCHAR Addr1[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN], Addr3[MAC_ADDR_LEN];
	UINT16 Reason, SeqNum;
	MAC_TABLE_ENTRY *pEntry;
	PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg;
#ifdef WTBL_TDD_SUPPORT
	BOOLEAN isInActiveEntry = FALSE;
#endif /* WTBL_TDD_SUPPORT */

	if (!PeerDeauthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, Addr3, &Reason))
		return;

	SeqNum = Fr->Hdr.Sequence;

	pEntry = NULL;

#ifdef WTBL_TDD_SUPPORT
	if (IS_WTBL_TDD_ENABLED(pAd)) {
		pEntry = MacTableLookup(pAd, Addr2);

		if (pEntry) {
			Elem->Wcid = pEntry->wcid;
		} else {
			pEntry = WtblTdd_InactiveList_Lookup(pAd, Addr2);
			if (pEntry) {
				isInActiveEntry = TRUE;
				Elem->Wcid = pEntry->wcid;
			}
		}
	}
#endif /* WTBL_TDD_SUPPORT */

	/*pEntry = MacTableLookup(pAd, Addr2); */
	if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {

#ifdef WTBL_TDD_SUPPORT
		if (isInActiveEntry == FALSE)
#endif /* WTBL_TDD_SUPPORT */
		{
			pEntry = &pAd->MacTab.Content[Elem->Wcid];
		}

		{
			/*
				Add Hotspot2.0 Rlease 1 Prestested Code
			*/
			BSS_STRUCT	*pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
			PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg;
			unsigned char *tmp = (unsigned char *)pMbss->wdev.bssid;
			unsigned char *tmp2 = (unsigned char *)&Fr->Hdr.Addr1;
#ifdef DOT11W_PMF_SUPPORT
			PMF_CFG        *pPmfCfg = NULL;

			pPmfCfg = &pEntry->SecConfig.PmfCfg;

			if (pPmfCfg->UsePMFConnect == TRUE && Fr->Hdr.FC.Wep == 0) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_WARN, "drop due to unprotect deauth frame\n");
				return;
			}
#endif
			if (IS_ENTRY_WDS(pEntry)) {
				ba_session_tear_down_all(pAd, Elem->Wcid, FALSE);
				wifi_sys_update_wds(pAd, pEntry);
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "receive wds deauth, update starec\n");
			}

			if (memcmp(&Fr->Hdr.Addr1, pMbss->wdev.bssid, 6) != 0) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "da not match bssid,bssid:0x%02x%02x%02x%02x%02x%02x, addr1:0x%02x%02x%02x%02x%02x%02x\n",
						  *tmp, *(tmp + 1), *(tmp + 2), *(tmp + 3), *(tmp + 4), *(tmp + 5), *tmp2, *(tmp2 + 1), *(tmp2 + 2), *(tmp2 + 3), *(tmp2 + 4), *(tmp2 + 5));
				return;
			} else
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "da match,0x%02x%02x%02x%02x%02x%02x\n",
						 *tmp, *(tmp + 1), *(tmp + 2), *(tmp + 3), *(tmp + 4), *(tmp + 5));
		}
#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

		/* Notify 802.1x daemon to clear this sta info */
		if (IS_AKM_1X_Entry(pEntry)
			|| IS_IEEE8021X_Entry(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev))
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
		/* send wireless event - for deauthentication */
		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, Addr2, 0, 0);
		ApLogEvent(pAd, Addr2, EVENT_DISASSOCIATED);

		if (pEntry->CMTimerRunning == TRUE) {
			/*
				If one who initilized Counter Measure deauth itself,
				AP doesn't log the MICFailTime
			*/
			pAd->ApCfg.aMICFailTime = pAd->ApCfg.PrevaMICFailTime;
		}

#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_DEAUTH, REASON_DEAUTH_STA_LEAVING);
#endif
#ifdef CONN_FAIL_EVENT
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_DEAUTH_STA_LEAVING);
#endif
		if (pEntry && !IS_ENTRY_CLIENT(pEntry))
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 " receive not client de-auth ###\n");
		else {
#ifdef MAP_R2
			if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
				pEntry->DisconnectReason = Reason;
#endif
			MacTableDeleteEntry(pAd, Elem->Wcid, Addr2);
		}

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				 "AUTH - receive DE-AUTH(seq-%d) from "MACSTR", reason=%d\n",
				  SeqNum, MAC2STR(Addr2), Reason);
#ifdef WH_EVENT_NOTIFIER
		{
			EventHdlr pEventHdlrHook = NULL;

			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
			if (pEventHdlrHook && pEntry->wdev)
			pEventHdlrHook(pAd, pEntry->wdev, Addr2, Elem->Channel);
		}
#endif /* WH_EVENT_NOTIFIER */
	}
}



static VOID ap_peer_auth_simple_rsp_gen_and_send(
	IN PRTMP_ADAPTER pAd,
	IN PHEADER_802_11 pHdr,
	IN USHORT Alg,
	IN USHORT Seq,
	IN USHORT StatusCode)
{
	HEADER_802_11     AuthHdr;
	ULONG             FrameLen = 0;
	PUCHAR            pOutBuffer = NULL;
	NDIS_STATUS       NStatus;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	if (StatusCode == MLME_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AUTH_RSP - Send AUTH response (SUCCESS)...\n");
	else {
		/* For MAC wireless client(Macintosh), need to send AUTH_RSP with Status Code (fail reason code) to reject it. */
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"AUTH_RSP - Peer AUTH fail (Status = %d)...\n", StatusCode);
	}

	MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, pHdr->Addr2,
					 pHdr->Addr1,
					 pHdr->Addr1);
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(HEADER_802_11),	&AuthHdr,
					  2,						&Alg,
					  2,						&Seq,
					  2,						&StatusCode,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
}


VOID ap_peer_auth_req_at_idle_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	INT i;
	USHORT RspReason;
	AUTH_FRAME_INFO *pAuth_info;
	UINT32 apidx;
	BOOLEAN checkAuthSanity = FALSE;
	PHEADER_802_11 pRcvHdr;
	HEADER_802_11 AuthHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	MAC_TABLE_ENTRY *pEntry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR ChTxtIe = 16, ChTxtLen = CIPHER_TEXT_LEN;
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg;
	PFT_INFO pFtInfoBuf;
#endif /* DOT11R_FT_SUPPORT */
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UCHAR WdevBandIdx;
	UCHAR ChBandIdx;
#ifdef WAPP_SUPPORT
	UINT8 wapp_cnnct_stage = WAPP_AUTH;
	UINT16 wapp_auth_fail = NOT_FAILURE;
	UINT16 status_code = MLME_SUCCESS;
#endif /* WAPP_SUPPORT */
#ifdef RADIUS_MAC_AUTH_SUPPORT
	UINT32 freq;
#endif
#ifdef MBO_SUPPORT
	BOOLEAN is_mbo_bndstr_sta = 0;
#endif /* MBO_SUPPORT */

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
	BOOLEAN bACLReject = FALSE;
	BOOLEAN bBlReject = FALSE;
#endif


	os_alloc_mem_suspend(pAd, (UCHAR **)&pAuth_info, sizeof(AUTH_FRAME_INFO));

	if (!pAuth_info) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "auth_info: Mem Alloc fail!\n");
		return;
	}
	os_zero_mem(pAuth_info, sizeof(AUTH_FRAME_INFO));

	checkAuthSanity = ap_peer_auth_sanity(pAd, Elem->Msg, Elem->MsgLen, pAuth_info);
	/* Find which MBSSID to be authenticate */
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s: Recv Auth from "MACSTR"\n",
			__func__, MAC2STR(pAuth_info->addr2));

	apidx = get_apidx_by_addr(pAd, pAuth_info->addr1);
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;


	if (pAd->ApCfg.BANClass3Data == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Disallow new Association\n");
#ifdef WAPP_SUPPORT
		wapp_auth_fail = DISALLOW_NEW_ASSOCI;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
	}

	if (!checkAuthSanity) {
#ifdef WAPP_SUPPORT
		wapp_auth_fail = PEER_REQ_SANITY_FAIL;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "AUTH - Bssid not found\n");
#ifdef WAPP_SUPPORT
		wapp_auth_fail = BSSID_NOT_FOUND;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
	}

	if (!OPSTATUS_TEST_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED) ||
		(WDEV_BSS_STATE(wdev) <= BSS_ACTIVE)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "AP is not ready, disallow new Association (state:%d)\n",
				 WDEV_BSS_STATE(wdev));
#ifdef WAPP_SUPPORT
		wapp_auth_fail = AP_NOT_READY;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
	}

	if ((wdev->if_dev == NULL) || (wdev->bRejectAuth == TRUE) || ((wdev->if_dev != NULL) &&
								   !(RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev)))) {
		if (wdev->bRejectAuth == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Reject Auth from "MACSTR"\n",
				MAC2STR(pAuth_info->addr2));
		}
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "AUTH - Bssid IF didn't up yet.\n");
#ifdef WAPP_SUPPORT
		wapp_auth_fail = BSSID_IF_NOT_READY;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
	}

	ASSERT((wdev->func_idx == apidx));


	pEntry = MacTableLookup(pAd, pAuth_info->addr2);

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	/* Check and remove TWT agrt to FW for unexpectedly leaved STA before. */
	if (pEntry)
		twt_resource_release_at_link_down(pAd, pEntry->wcid);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

#ifdef WTBL_TDD_SUPPORT
	if (IS_WTBL_TDD_ENABLED(pAd)) {
		/* Double check Ext In-Act Table */
		if (!pEntry)
			pEntry = WtblTdd_InactiveList_Lookup(pAd, pAuth_info->addr2);

		/* Force Delete the Entry ether in Active or Ext In-Act Table */
		if (pEntry) {
			MacTableDeleteEntry(pAd, pEntry->wcid, pAuth_info->addr2);
			pEntry = NULL;
		}
	}
#endif /* WTBL_TDD_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
	if (IS_SW_STA_ENABLED(pAd)) {
		/* Force Delete the Entry if found , to make sure dummy obj ref cnt symatric , ex : abnormal disconnect STA */
		if (pEntry) {
			MacTableDeleteEntry(pAd, pEntry->wcid, pAuth_info->addr2);
			pEntry = NULL;
		}
	}
#endif /* SW_CONNECT_SUPPORT */

#ifdef WDS_SUPPORT
	if (pEntry && IS_ENTRY_WDS(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"WDS entry, not need auth!\n");
#ifdef WAPP_SUPPORT
		wapp_auth_fail = MLME_REQ_WITH_INVALID_PARAM;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
	}
#endif

#ifdef AIR_MONITOR
	if (pEntry && IS_ENTRY_MONITOR(pEntry)) {
		MacTableDeleteEntry(pAd, pEntry->wcid, pAuth_info->addr2);
		goto auth_failure;
	}
#endif

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
		/* reset NoDataIdleCount to prevent unexpected STA assoc timeout and kicked by MacTableMaintenance */
		pEntry->NoDataIdleCount = 0;
		/* WPA2-PSK Case : To prevent from auth flood aatack */
		if (!IS_AKM_SHA256(wdev->SecConfig.AKMMap)) {
			if ((pAuth_info->auth_alg == AUTH_MODE_SAE) || (pAuth_info->auth_alg == AUTH_MODE_FT) ||
				(pAuth_info->auth_alg == AUTH_MODE_FILS) || (pAuth_info->auth_alg == AUTH_MODE_FILS_PFS)) {
				goto auth_failure;
			}
		}

#ifdef DOT11W_PMF_SUPPORT

		if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef DOT11_SAE_SUPPORT
			/* WPA3-PSK case : To prevent from auth flood attack  */

			if (IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap) && pEntry->SecConfig.ft_only == FALSE) {
#ifdef DOT11R_FT_SUPPORT
				if (!wdev->FtCfg.FtCapFlag.Dot11rFtEnable) {
#endif
					if ((pAuth_info->auth_alg == AUTH_MODE_FT) ||
						(pAuth_info->auth_alg == AUTH_MODE_FILS) ||
						(pAuth_info->auth_alg == AUTH_MODE_FILS_PFS)) {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
							"SAE_AUTH_FLOOD Attack !\n");
						goto auth_failure;
					}
#ifdef DOT11R_FT_SUPPORT
				}
#endif
			}

#endif //DOT11_SAE_SUPPORT
		}

#endif /* DOT11W_PMF_SUPPORT */

		/* If wdev bandix does not match to pEntry bandidex, remove entry. */
		WdevBandIdx = HcGetBandByWdev(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev);
		ChBandIdx = HcGetBandByWdev(wdev);
		if (WdevBandIdx != ChBandIdx) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wcid%d exist in Band%d but Recv Band%d, CH%d => SKIP\n",
				pEntry->wcid, WdevBandIdx, ChBandIdx, Elem->Channel);
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			pEntry = NULL;
		} else if (!RTMPEqualMemory(pAuth_info->addr1,
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN)) {
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			pEntry = NULL;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "AUTH - Bssid does not match\n");
#ifdef MBO_SUPPORT
			is_mbo_bndstr_sta = 1;
#endif /* MBO_SUPPORT */


		} else {
#ifdef DOT11_N_SUPPORT
			ba_session_tear_down_all(pAd, pEntry->wcid, FALSE);
#endif /* DOT11_N_SUPPORT */
		}
	}

	pRcvHdr = (PHEADER_802_11)(Elem->Msg);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "AUTH - MBSS(%d), Rcv AUTH seq#%d, Alg=%d, Status=%d from [wcid=%d]"MACSTR"\n",
			  apidx, pAuth_info->auth_seq, pAuth_info->auth_alg,
			  pAuth_info->auth_status, Elem->Wcid,
			  MAC2STR(pAuth_info->addr2));
#ifdef WSC_V2_SUPPORT

	/* Do not check ACL when WPS V2 is enabled and ACL policy is positive. */
	/* Fix: WCS Cert #4.2.7, Usage of wdev->WscControl insted of pMbss->WscControl */
	/* now instead of two places, WscControl is maintained at only one place i.e pMbss->wdev.WscControl*/
	if ((pMbss->wdev.WscControl.WscConfMode != WSC_DISABLE) &&
		(pMbss->wdev.WscControl.bWscTrigger) &&
		(pMbss->wdev.WscControl.WscV2Info.bEnableWpsV2) &&
		(pMbss->wdev.WscControl.WscV2Info.bWpsEnable) &&
		(pMbss->AccessControlList.Policy == 1))
		;
	else
#endif /* WSC_V2_SUPPORT */
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
		/* set a flag for sending Assoc-Fail response to unwanted STA later. */
		if (map_is_entry_bl(pAd, pAuth_info->addr2, apidx) == TRUE) {
			bBlReject = TRUE;
		} else
#endif /*  MAP_BL_SUPPORT */

		/* fail in ACL checking => send an AUTH-Fail seq#2. */
		if (!ApCheckAccessControlList(pAd, pAuth_info->addr2, apidx)) {

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
			bACLReject = TRUE;
		}

		if (bBlReject || bACLReject) {
#endif
			ASSERT(pAuth_info->auth_seq == 1);
			ASSERT(pEntry == NULL);
#ifdef WAPP_SUPPORT
			status_code = MLME_UNSPECIFY_FAIL;
#endif
			ap_peer_auth_simple_rsp_gen_and_send(pAd, pRcvHdr,
				pAuth_info->auth_alg, pAuth_info->auth_seq + 1, MLME_UNSPECIFY_FAIL);
#ifdef WIFI_DIAG
			diag_conn_error(pAd, apidx, pAuth_info->addr2, DIAG_CONN_ACL_BLK, 0);
#endif
#ifdef CONN_FAIL_EVENT
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[apidx].Ssid,
				pAd->ApCfg.MBSSID[apidx].SsidLen,
				pAuth_info->addr2,
				REASON_DECLINED);
#endif

			/* If this STA exists, delete it. */
			if (pEntry)
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

			RTMPSendWirelessEvent(pAd, IW_MAC_FILTER_LIST_EVENT_FLAG, pAuth_info->addr2, wdev->wdev_idx, 0);
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Failed in BL/ACL %d/%d checking => send an AUTH seq#2 with Status code = %d\n",
					  bBlReject, bACLReject, MLME_UNSPECIFY_FAIL);
#else
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Failed in ACL checking => send an AUTH seq#2 with Status code = %d\n",
					  MLME_UNSPECIFY_FAIL);
#endif
#ifdef WH_EVENT_NOTIFIER
			{
				EventHdlr pEventHdlrHook = NULL;

				pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_AUTH_REJECT);
				if (pEventHdlrHook && wdev)
				pEventHdlrHook(pAd, wdev, pAuth_info->addr2, Elem);
			}
#endif /* WH_EVENT_NOTIFIER */

#ifdef WAPP_SUPPORT
			wapp_auth_fail = ACL_CHECK_FAIL;
#endif /* WAPP_SUPPORT */
			 goto auth_failure;
		}

#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering

	) {
		BOOLEAN bBndStrgCheck;

		bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev, pAuth_info->addr2, Elem, NULL);
		if (bBndStrgCheck == FALSE) {
#ifdef WAPP_SUPPORT
			status_code = MLME_UNSPECIFY_FAIL;
#endif
			ap_peer_auth_simple_rsp_gen_and_send(pAd, pRcvHdr,
				pAuth_info->auth_alg, pAuth_info->auth_seq + 1, MLME_UNSPECIFY_FAIL);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "AUTH - check failed.\n");
#ifdef WAPP_SUPPORT
			wapp_auth_fail = BND_STRG_CONNECT_CHECK_FAIL;
#endif /* WAPP_SUPPORT */
#ifdef WIFI_DIAG
			diag_conn_error(pAd, apidx, pAuth_info->addr2, DIAG_CONN_BAND_STE, 0);
#endif
#ifdef CONN_FAIL_EVENT
			 ApSendConnFailMsg(pAd,
						 pAd->ApCfg.MBSSID[apidx].Ssid,
						 pAd->ApCfg.MBSSID[apidx].SsidLen,
						 pAuth_info->addr2,
						 REASON_UNSPECIFY);
#endif
			 goto auth_failure;

		}
	}
#endif /* BAND_STEERING */

#ifdef RADIUS_MAC_AUTH_SUPPORT
	MAP_CHANNEL_ID_TO_KHZ(pAd->LatchRfRegs.Channel, freq);
	freq /= 1000;
#endif

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS(wdev->SecConfig.AKMMap)
		&& (pAuth_info->auth_alg == AUTH_MODE_FILS)) {

		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, pAuth_info->addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

#ifdef MBO_SUPPORT
		if (NULL != pEntry)
			pEntry->is_mbo_bndstr_sta = is_mbo_bndstr_sta;
#endif/* MBO_SUPPORT*/

		/* SET FILS First for clean STA if auth fail
		   and adjust AKM with 256/384 in assoc phase */
		if (NULL != pEntry)
			SET_AKM_FILS_SHA256(pEntry->SecConfig.AKMMap);
		if (0) {
		} else {
			struct fils_info *filsInfo = NULL;

			if (pEntry != NULL) {
				filsInfo = &pEntry->filsInfo;

				if (filsInfo->pending_ie) {
					os_free_mem(filsInfo->pending_ie);
					filsInfo->pending_ie_len = 0;
					filsInfo->pending_ie = NULL;
				}

				filsInfo->auth_algo = AUTH_MODE_FILS;
				filsInfo->pending_ie_len = Elem->MsgLen;
				os_alloc_mem(NULL, (UCHAR **)&filsInfo->pending_ie, filsInfo->pending_ie_len);
				NdisMoveMemory(filsInfo->pending_ie, Elem->Msg, filsInfo->pending_ie_len);

				DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_MLME_EVENT);
				filsInfo->is_pending_auth = TRUE;
			}

			os_free_mem(pAuth_info);
			return;
		}
	}
#endif /* OCE_FILS_SUPPORT */

#ifdef RADIUS_MAC_ACL_SUPPORT

	if (IS_IEEE8021X_Entry(wdev) &&
		(wdev->SecConfig.RadiusMacAuthCache.Policy == RADIUS_MAC_AUTH_ENABLE)) {
#define	RADIUS_ACL_REJECT  0
#define	RADIUS_ACL_ACCEPT  1
#define	RADIUS_ACL_PENDING  2
#define	RADIUS_ACL_ACCEPT_TIMEOUT  3
		PRT_802_11_RADIUS_ACL_ENTRY pAclEntry = NULL;

		pAclEntry = RadiusFindAclEntry(&wdev->SecConfig.RadiusMacAuthCache.cacheList, pAuth_info->addr2);

		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, pAuth_info->addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

#ifdef MBO_SUPPORT
		if (NULL != pEntry)
			pEntry->is_mbo_bndstr_sta = is_mbo_bndstr_sta;
#endif/* MBO_SUPPORT*/

		if (pAclEntry) {
			if (pAclEntry->result == RADIUS_ACL_REJECT) {
#ifdef WAPP_SUPPORT
				status_code = MLME_UNSPECIFY_FAIL;
#endif
				ap_peer_auth_simple_rsp_gen_and_send(pAd, pRcvHdr,
					pAuth_info->auth_alg, pAuth_info->auth_seq + 1, MLME_UNSPECIFY_FAIL);
#ifdef WIFI_DIAG
				diag_conn_error(pAd, apidx, pAuth_info->addr2, DIAG_CONN_ACL_BLK, 0);
#endif
#ifdef CONN_FAIL_EVENT
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[apidx].Ssid,
					pAd->ApCfg.MBSSID[apidx].SsidLen,
					pAuth_info->addr2,
					REASON_DECLINED);
#endif
				if (pEntry)
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

				RTMPSendWirelessEvent(pAd, IW_MAC_FILTER_LIST_EVENT_FLAG, Addr2, apidx, 0);
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						""MACSTR" RADIUS ACL checking => Reject.\n",
							MAC2STR(pAuth_info->addr2));
			} else if (pAclEntry->result == RADIUS_ACL_ACCEPT) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ""MACSTR" RADIUS ACL checking => OK.\n",
						 MAC2STR(pAuth_info->addr2));
			} else if (pAclEntry->result == RADIUS_ACL_ACCEPT_TIMEOUT) {
				/* with SESSION-TIMEOUT from Radius Server */
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ""MACSTR" RADIUS ACL checking => OK. (TIMEOUT)\n",
						 MAC2STR(pAuth_info->addr2));
				DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_ACL_ENTRY);
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ""MACSTR" RADIUS ACL checking => Unkown.\n",
						 MAC2STR(pAuth_info->addr2));
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ""MACSTR" Not Found in RADIUS ACL & go to Check.\n",
					 MAC2STR(pAuth_info->addr2));
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_ACL_ENTRY);
#ifdef WAPP_SUPPORT
			wapp_auth_fail = NOT_FOUND_IN_RADIUS_ACL;
#endif /* WAPP_SUPPORT */
#ifdef WIFI_DIAG
			diag_conn_error(pAd, apidx, pAuth_info->addr2, DIAG_CONN_ACL_BLK, 0);
#endif
#ifdef CONN_FAIL_EVENT
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[apidx].Ssid,
				pAd->ApCfg.MBSSID[apidx].SsidLen,
				pAuth_info->addr2,
				REASON_DECLINED);
#endif

			goto auth_failure;
		}
	}

#endif /* RADIUS_MAC_ACL_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	pFtCfg = &wdev->FtCfg;

	if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
		&& (pAuth_info->auth_alg == AUTH_MODE_FT)) {
		USHORT result;

		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, pAuth_info->addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

		if (pEntry != NULL) {
#ifdef MBO_SUPPORT
			pEntry->is_mbo_bndstr_sta = is_mbo_bndstr_sta;
#endif/* MBO_SUPPORT*/
			os_alloc_mem(pAd, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO));

			if (pFtInfoBuf) {
				result = FT_AuthReqHandler(pAd, pEntry, &pAuth_info->FtInfo, pFtInfoBuf);

				if (result == MLME_SUCCESS) {
					NdisMoveMemory(&pEntry->MdIeInfo, &pAuth_info->FtInfo.MdIeInfo,
						sizeof(FT_MDIE_INFO));
					pEntry->AuthState = AS_AUTH_OPEN;
					/*According to specific, if it already in SST_ASSOC, it can not go back */
					if (pEntry->Sst != SST_ASSOC)
						pEntry->Sst = SST_AUTH;
				} else if (result == MLME_FAIL_NO_RESOURCE) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"- give up this AUTH pkt ======================> Query R1KH from backbone (Wcid%d, %d)\n",
						 pEntry->wcid, pEntry->FT_R1kh_CacheMiss_Times);
					os_free_mem(pFtInfoBuf);
#ifdef WAPP_SUPPORT
					wapp_auth_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
#ifdef WIFI_DIAG
					diag_conn_error(pAd, apidx, pAuth_info->addr2,
						DIAG_CONN_AUTH_FAIL, REASON_NO_RESOURCE);
#endif
#ifdef CONN_FAIL_EVENT
					ApSendConnFailMsg(pAd,
								pAd->ApCfg.MBSSID[apidx].Ssid,
								pAd->ApCfg.MBSSID[apidx].SsidLen,
								pAuth_info->addr2,
								REASON_UNSPECIFY);
#endif

					goto auth_failure;
				}


				FT_EnqueueAuthReply(pAd, pRcvHdr, pAuth_info->auth_alg, 2, result,
									&pFtInfoBuf->MdIeInfo, &pFtInfoBuf->FtIeInfo, NULL,
									pFtInfoBuf->RSN_IE, pFtInfoBuf->RSNIE_Len);
				os_free_mem(pFtInfoBuf);

				if (result == MLME_SUCCESS) {
				struct _ASIC_SEC_INFO *info = NULL;

				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));

				if (info) {
					struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;

					os_zero_mem(info, sizeof(ASIC_SEC_INFO));
					NdisCopyMemory(pSecConfig->PTK, pEntry->FT_PTK, LEN_MAX_PTK);
					info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
					info->Direction = SEC_ASIC_KEY_BOTH;
					info->Wcid = pEntry->wcid;
					info->BssIndex = pEntry->func_tb_idx;
					info->Cipher = pSecConfig->PairwiseCipher;
					info->KeyIdx = pSecConfig->PairwiseKeyId;
					os_move_mem(&info->PeerAddr[0],
								pEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(info->Key.Key,
								&pEntry->FT_PTK[LEN_PTK_KCK + LEN_PTK_KEK],
								(LEN_TK + LEN_TK2));
					WPAInstallKey(pAd, info, TRUE, TRUE);
					/* Update status and set Port as Secured */
					pSecConfig->Handshake.WpaState = AS_PTKINITDONE;
					os_free_mem(info);
				} else {
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL,
							 DBG_LVL_ERROR, "struct alloc fail\n");
				}
			}
			}
		}

		else  {/* MAC table full*/
#ifdef WIFI_DIAG

			diag_conn_error(pAd, apidx, pAuth_info->addr2, DIAG_CONN_STA_LIM, 0);
#endif
#ifdef CONN_FAIL_EVENT
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[apidx].Ssid,
				pAd->ApCfg.MBSSID[apidx].SsidLen,
				pAuth_info->addr2,
				REASON_DISASSPC_AP_UNABLE);
#endif
		}
		os_free_mem(pAuth_info);
		return;
	} else
#endif /* DOT11R_FT_SUPPORT */
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef DOT11_SAE_SUPPORT

	if ((pAuth_info->auth_alg == AUTH_MODE_SAE) &&
		(IS_AKM_SAE_SHA256(pMbss->wdev.SecConfig.AKMMap))) {
		UCHAR sae_conn_type;
		UCHAR *pmk;
#ifdef DOT11W_PMF_SUPPORT

		if (pEntry) {
			tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
			map_a4_peer_disable(pAd, pEntry, TRUE);
#endif

			if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
				&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef WAPP_SUPPORT
				status_code = MLME_ASSOC_REJ_TEMPORARILY;
#endif
				if (pEntry->SecConfig.PmfCfg.SAQueryStatus == SAQ_IDLE) {
					ap_peer_auth_simple_rsp_gen_and_send(pAd,
								    pRcvHdr,
								    pAuth_info->auth_alg,
								    pAuth_info->auth_seq,
								    MLME_ASSOC_REJ_TEMPORARILY);
					PMF_MlmeSAQueryReq(pAd, pEntry);
				}
#ifdef WAPP_SUPPORT
				wapp_auth_fail = MLME_ASSOC_REJ_TEMP;
#endif /* WAPP_SUPPORT */
#ifdef WIFI_DIAG
				diag_conn_error(pAd, apidx, pAuth_info->addr2,
					DIAG_CONN_AUTH_FAIL, REASON_REJ_TEMPORARILY);
#endif
#ifdef CONN_FAIL_EVENT
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[apidx].Ssid,
					pAd->ApCfg.MBSSID[apidx].SsidLen,
					pAuth_info->addr2,
					REASON_CIPHER_SUITE_REJECTED);
#endif

				goto auth_failure;
			}
		}

#endif /* DOT11W_PMF_SUPPORT */
		/* ap is passive, so do not consider the retrun value of sae_handle_auth */
		sae_handle_auth(pAd, &pAd->SaeCfg, Elem->Msg, Elem->MsgLen,
				pMbss->wdev.SecConfig.PSK,
				pMbss->wdev.SecConfig.pt_list,
				&pMbss->wdev.SecConfig.sae_pk,
				&pMbss->wdev.SecConfig.sae_cap,
				&pMbss->wdev.SecConfig.pwd_id_list_head,
				pAuth_info->auth_seq, pAuth_info->auth_status, &pmk, &sae_conn_type);

		if (pmk) {
			if (!pEntry)
				pEntry = MacTableInsertEntry(pAd, pAuth_info->addr2,
							wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

			if (pEntry) {
				UCHAR pmkid[80];

#ifdef MBO_SUPPORT
				pEntry->is_mbo_bndstr_sta = is_mbo_bndstr_sta;
#endif/* MBO_SUPPORT*/
				NdisMoveMemory(pEntry->SecConfig.PMK, pmk, LEN_PMK);
				pEntry->AuthState = AS_AUTH_OPEN;
				/*According to specific, if it already in SST_ASSOC, it can not go back */
				if (pEntry->Sst != SST_ASSOC)
					pEntry->Sst = SST_AUTH;
				pEntry->SecConfig.sae_conn_type = sae_conn_type;
				pEntry->SecConfig.sae_cap.gen_pwe_method = pMbss->wdev.SecConfig.sae_cap.gen_pwe_method;
				if (sae_get_pmk_cache(&pAd->SaeCfg, pAuth_info->addr1,
							pAuth_info->addr2, pmkid, NULL)) {
					RTMPAddPMKIDCache(&pAd->ApCfg.PMKIDCache,
							  apidx,
							  pEntry->Addr,
							  pmkid,
							  pmk,
							  FALSE,
							  LEN_PMK);
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
						 "WPA3PSK(SAE):("MACSTR")Calc PMKID="MACSTR"\n",
						 MAC2STR(pEntry->Addr), MAC2STR(pmkid));
				}
			} else {/* MAC table full*/
#ifdef WIFI_DIAG
				diag_conn_error(pAd, apidx, pAuth_info->addr2, DIAG_CONN_STA_LIM, 0);
#endif
#ifdef CONN_FAIL_EVENT
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[apidx].Ssid,
					pAd->ApCfg.MBSSID[apidx].SsidLen,
					pAuth_info->addr2,
					REASON_DISASSPC_AP_UNABLE);
#endif
			}

		}
	} else
#endif /* DOT11_SAE_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/

		if ((pAuth_info->auth_alg == AUTH_MODE_OPEN) &&
			(!IS_AKM_SHARED(pMbss->wdev.SecConfig.AKMMap))) {
			if (!pEntry)
				pEntry = MacTableInsertEntry(pAd, pAuth_info->addr2,
					wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

			if (pEntry) {
#ifdef MBO_SUPPORT
				pEntry->is_mbo_bndstr_sta = is_mbo_bndstr_sta;
#endif/* MBO_SUPPORT*/

				tr_entry = &tr_ctl->tr_entry[pEntry->wcid];

				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%d-%d-%d\n",
					pEntry->SecConfig.PmfCfg.UsePMFConnect, tr_entry->PortSecured,
					IS_AKM_SAE_SHA256(pMbss->wdev.SecConfig.AKMMap));

#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
				map_a4_peer_disable(pAd, pEntry, TRUE);
#endif
#ifdef DOT11W_PMF_SUPPORT

				if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
					&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
					&& (IS_AKM_SAE_SHA256(pMbss->wdev.SecConfig.AKMMap))) {
#ifdef WAPP_SUPPORT
					status_code = MLME_ASSOC_REJ_TEMPORARILY;
#endif
					if (pEntry->SecConfig.PmfCfg.SAQueryStatus == SAQ_IDLE) {
						ap_peer_auth_simple_rsp_gen_and_send(pAd,
											pRcvHdr,
											pAuth_info->auth_alg,
											pAuth_info->auth_seq,
											MLME_ASSOC_REJ_TEMPORARILY);
						PMF_MlmeSAQueryReq(pAd, pEntry);
					}
#ifdef WAPP_SUPPORT
					wapp_auth_fail = MLME_ASSOC_REJ_TEMP;
#endif /* WAPP_SUPPORT */
#ifdef WIFI_DIAG
					diag_conn_error(pAd, apidx, pAuth_info->addr2,
						DIAG_CONN_AUTH_FAIL, REASON_REJ_TEMPORARILY);
#endif
#ifdef CONN_FAIL_EVENT
					ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[apidx].Ssid,
						pAd->ApCfg.MBSSID[apidx].SsidLen,
						pAuth_info->addr2,
						REASON_CIPHER_SUITE_REJECTED);
#endif

					goto auth_failure;
				}

				if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE)
					|| (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED))
#endif /* DOT11W_PMF_SUPPORT */
				{
					pEntry->AuthState = AS_AUTH_OPEN;
					/*According to specific, if it already in SST_ASSOC, it can not go back */
					if (pEntry->Sst != SST_ASSOC)
						pEntry->Sst = SST_AUTH;
				}

				ap_peer_auth_simple_rsp_gen_and_send(pAd, pRcvHdr,
					pAuth_info->auth_alg, pAuth_info->auth_seq + 1, MLME_SUCCESS);

#if defined(RADIUS_MAC_AUTH_SUPPORT) && defined(RT_CFG80211_SUPPORT)
				if (wdev->radius_mac_auth_enable)
					CFG80211OS_RxMgmt(wdev->if_dev, freq, Elem->Msg, Elem->MsgLen);
#endif
		} else {/* MAC table full*/
#ifdef WIFI_DIAG
				diag_conn_error(pAd, apidx, pAuth_info->addr2, DIAG_CONN_STA_LIM, 0);
#endif
#ifdef CONN_FAIL_EVENT
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[apidx].Ssid,
					pAd->ApCfg.MBSSID[apidx].SsidLen,
					pAuth_info->addr2,
					REASON_DISASSPC_AP_UNABLE);
#endif
			}

		} else if ((pAuth_info->auth_alg == AUTH_MODE_KEY) &&
				   (IS_AKM_SHARED(pMbss->wdev.SecConfig.AKMMap)
					|| IS_AKM_AUTOSWITCH(pMbss->wdev.SecConfig.AKMMap))) {
			if (!pEntry)
				pEntry = MacTableInsertEntry(pAd, pAuth_info->addr2,
							wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

			if (pEntry) {
#ifdef MBO_SUPPORT
				pEntry->is_mbo_bndstr_sta = is_mbo_bndstr_sta;
#endif/* MBO_SUPPORT*/

				pEntry->AuthState = AS_AUTHENTICATING;
				pEntry->Sst = SST_NOT_AUTH; /* what if it already in SST_ASSOC ??????? */
				/* log this STA in AuthRspAux machine, only one STA is stored. If two STAs using */
				/* SHARED_KEY authentication mingled together, then the late comer will win. */
				COPY_MAC_ADDR(&pAd->ApMlmeAux.Addr, pAuth_info->addr2);

				for (i = 0; i < CIPHER_TEXT_LEN; i++)
					pAd->ApMlmeAux.Challenge[i] = RandomByte(pAd);

				RspReason = 0;
				pAuth_info->auth_seq++;
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

				if (NStatus != NDIS_STATUS_SUCCESS) {
#ifdef WAPP_SUPPORT
					wapp_auth_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
#ifdef WIFI_DIAG
				diag_conn_error(pAd, apidx, pAuth_info->addr2,
					DIAG_CONN_AUTH_FAIL, REASON_NO_RESOURCE);
#endif
#ifdef CONN_FAIL_EVENT
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[apidx].Ssid,
					pAd->ApCfg.MBSSID[apidx].SsidLen,
					pAuth_info->addr2,
					REASON_UNSPECIFY);
#endif

					goto auth_failure; /* if no memory, can't do anything */
				}

				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AUTH - Send AUTH seq#2 (Challenge)\n");
				MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0,	pAuth_info->addr2,
								 wdev->if_addr,
								 wdev->bssid);
				MakeOutgoingFrame(pOutBuffer,			 &FrameLen,
								  sizeof(HEADER_802_11), &AuthHdr,
								  2, &pAuth_info->auth_alg,
								  2, &pAuth_info->auth_seq,
								  2, &RspReason,
								  1, &ChTxtIe,
								  1, &ChTxtLen,
								  CIPHER_TEXT_LEN,	   pAd->ApMlmeAux.Challenge,
								  END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pOutBuffer);

#if defined(RADIUS_MAC_AUTH_SUPPORT) && defined(RT_CFG80211_SUPPORT)
				if (wdev->radius_mac_auth_enable)
					CFG80211OS_RxMgmt(wdev->if_dev, freq, Elem->Msg, Elem->MsgLen);
#endif
		} else {/* MAC table full */
#ifdef WIFI_DIAG
				diag_conn_error(pAd, apidx, pAuth_info->addr2, DIAG_CONN_STA_LIM, 0);
#endif
#ifdef CONN_FAIL_EVENT
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[apidx].Ssid,
					pAd->ApCfg.MBSSID[apidx].SsidLen,
					pAuth_info->addr2,
					REASON_DISASSPC_AP_UNABLE);
#endif
			}

		}
		else {
			/* wrong algorithm */
			ap_peer_auth_simple_rsp_gen_and_send(pAd, pRcvHdr,
				pAuth_info->auth_alg, pAuth_info->auth_seq + 1, MLME_ALG_NOT_SUPPORT);
#ifdef WIFI_DIAG
			diag_conn_error(pAd, apidx, pAuth_info->addr2,
				DIAG_CONN_AUTH_FAIL, REASON_AUTH_WRONG_ALGORITHM);
#endif
#ifdef CONN_FAIL_EVENT
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[apidx].Ssid,
				pAd->ApCfg.MBSSID[apidx].SsidLen,
				pAuth_info->addr2,
				REASON_AKMP_NOT_VALID);
#endif
			/* If this STA exists, delete it. */
			if (pEntry)
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "AUTH -Wrong algorithm Alg=%d, Seq=%d\n",
					 pAuth_info->auth_alg, pAuth_info->auth_seq);
		}
	os_free_mem(pAuth_info);
	return;
auth_failure:
#ifdef WAPP_SUPPORT
	wapp_send_sta_connect_rejected(pAd, wdev, pAuth_info->addr2,
					pAuth_info->addr1, wapp_cnnct_stage,
					wapp_auth_fail, status_code, 0);
#endif /* WAPP_SUPPORT */
	os_free_mem(pAuth_info);
	return;
}


VOID ap_peer_auth_confirm_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	AUTH_FRAME_INFO *auth_info = NULL;
	PHEADER_802_11	pRcvHdr;
	MAC_TABLE_ENTRY *pEntry;
	UINT32 apidx;
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg;
	PFT_INFO pFtInfoBuf;
#endif /* DOT11R_FT_SUPPORT */

	if (pAd == NULL)
		return;

	os_alloc_mem(pAd, (UCHAR **)&auth_info, sizeof(AUTH_FRAME_INFO));
	if (!auth_info)
		goto error;

	if (!ap_peer_auth_sanity(pAd, Elem->Msg, Elem->MsgLen, auth_info))
		goto error;

	apidx = get_apidx_by_addr(pAd, auth_info->addr1);

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "AUTH - Bssid not found\n");
		goto error;
	}

	if (!VALID_MBSS(pAd, apidx)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Index out of bound\n");
		goto error;
	}

	if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
		!(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "AUTH - Bssid IF didn't up yet.\n");
		goto error;
	} /* End of if */

	if (!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "AUTH - Invalid wcid (%d).\n", Elem->Wcid);
		goto error;
	}

	pEntry = &pAd->MacTab.Content[Elem->Wcid];

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		if (!RTMPEqualMemory(auth_info->addr1, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN)) {
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			pEntry = NULL;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "AUTH - Bssid does not match\n");
		} else
			ba_session_tear_down_all(pAd, pEntry->wcid, FALSE);
	}

	pRcvHdr = (PHEADER_802_11)(Elem->Msg);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "AUTH - MBSS(%d), Rcv AUTH seq#%d, Alg=%d, Status=%d from [wcid=%d]"MACSTR"\n",
			  apidx, auth_info->auth_seq, auth_info->auth_alg,
			  auth_info->auth_status, Elem->Wcid,
			  MAC2STR(auth_info->addr2));

	if (pEntry && MAC_ADDR_EQUAL(auth_info->addr2, pAd->ApMlmeAux.Addr)) {
#ifdef DOT11R_FT_SUPPORT
		pFtCfg = &pAd->ApCfg.MBSSID[apidx].wdev.FtCfg;

		if ((pFtCfg->FtCapFlag.Dot11rFtEnable) && (auth_info->auth_alg == AUTH_MODE_FT)) {
			USHORT result;

			os_alloc_mem(pAd, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO));

			if (pFtInfoBuf) {
				NdisZeroMemory(pFtInfoBuf, sizeof(FT_INFO));
				os_alloc_mem(pAd, (UCHAR **) &(pFtInfoBuf->RicInfo.pRicInfo), 512);

				if (pFtInfoBuf->RicInfo.pRicInfo != NULL) {
					result = FT_AuthConfirmHandler(pAd, pEntry, &auth_info->FtInfo, pFtInfoBuf);
					FT_EnqueueAuthReply(pAd, pRcvHdr, auth_info->auth_alg, 4, result,
										&pFtInfoBuf->MdIeInfo, &pFtInfoBuf->FtIeInfo,
										&pFtInfoBuf->RicInfo, pFtInfoBuf->RSN_IE, pFtInfoBuf->RSNIE_Len);
					os_free_mem(pFtInfoBuf->RicInfo.pRicInfo);
				}

				os_free_mem(pFtInfoBuf);
			}
		} else
#endif /* DOT11R_FT_SUPPORT */
			if ((pRcvHdr->FC.Wep == 1) &&
				NdisEqualMemory(auth_info->Chtxt, pAd->ApMlmeAux.Challenge, CIPHER_TEXT_LEN)) {
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				/* Copy security config from AP to STA's entry */
				pEntry->SecConfig.AKMMap = wdev->SecConfig.AKMMap;
				pEntry->SecConfig.PairwiseCipher = wdev->SecConfig.PairwiseCipher;
				pEntry->SecConfig.PairwiseKeyId = wdev->SecConfig.PairwiseKeyId;
				pEntry->SecConfig.GroupCipher = wdev->SecConfig.GroupCipher;
				pEntry->SecConfig.GroupKeyId = wdev->SecConfig.GroupKeyId;
				os_move_mem(pEntry->SecConfig.WepKey, wdev->SecConfig.WepKey, sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
				pEntry->SecConfig.GroupKeyId = wdev->SecConfig.GroupKeyId;
				/* Successful */
				ap_peer_auth_simple_rsp_gen_and_send(pAd, pRcvHdr, auth_info->auth_alg, auth_info->auth_seq + 1, MLME_SUCCESS);
				pEntry->AuthState = AS_AUTH_KEY;
				/*According to specific, if it already in SST_ASSOC, it can not go back */
				if (pEntry->Sst != SST_ASSOC)
					pEntry->Sst = SST_AUTH;
			} else {
				/* send wireless event - Authentication rejected because of challenge failure */
				RTMPSendWirelessEvent(pAd, IW_AUTH_REJECT_CHALLENGE_FAILURE, pEntry->Addr, 0, 0);
				/* fail - wep bit is not set or challenge text is not equal */
				ap_peer_auth_simple_rsp_gen_and_send(pAd, pRcvHdr, auth_info->auth_alg,
													 auth_info->auth_seq + 1,
													 MLME_REJ_CHALLENGE_FAILURE);
#ifdef WIFI_DIAG
				if (pEntry && IS_ENTRY_CLIENT(pEntry))
					diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
						DIAG_CONN_AUTH_FAIL, REASON_CHALLENGE_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
				if (IS_ENTRY_CLIENT(pEntry))
					ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
						pEntry->Addr,
						REASON_MIC_FAILURE);
#endif

				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
				/*Chtxt[127]='\0'; */
				/*pAd->ApMlmeAux.Challenge[127]='\0'; */
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "%s\n",
						 ((pRcvHdr->FC.Wep == 1) ? "challenge text is not equal" : "wep bit is not set"));
				/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Sent Challenge = %s\n",&pAd->ApMlmeAux.Challenge[100])); */
				/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rcv Challenge = %s\n",&Chtxt[100])); */
			}
	} else {
		/* fail for unknown reason. most likely is AuthRspAux machine be overwritten by another */
		/* STA also using SHARED_KEY authentication */
		ap_peer_auth_simple_rsp_gen_and_send(pAd, pRcvHdr, auth_info->auth_alg, auth_info->auth_seq + 1, MLME_UNSPECIFY_FAIL);
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_UNKNOWN);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_MIC_FAILURE);
#endif

		/* If this STA exists, delete it. */
		if (pEntry)
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
	}

error:
	os_free_mem(auth_info);
}

void ap_send_broadcast_deauth(void *ad_obj, struct wifi_dev *wdev)
{
	MLME_BROADCAST_DEAUTH_REQ_STRUCT  *pInfo = NULL;
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd = ad_obj;

	os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));
	if (Elem == NULL)
		return;
	if (Elem) {
		pInfo = (MLME_BROADCAST_DEAUTH_REQ_STRUCT *)Elem->Msg;
		pInfo->wdev = wdev;
		Elem->Wcid = WCID_ALL;
#ifdef MAP_R4
		if ((IS_MAP_ENABLE(pAd)
			&& IS_MAP_CERT_ENABLE(pAd)
			&& IS_MAP_R4_ENABLE(pAd)))
			pInfo->Reason = REASON_CLS2ERR;
		else
#endif
			pInfo->Reason = MLME_UNSPECIFY_FAIL;
		NdisCopyMemory(pInfo->Addr, BROADCAST_ADDR, MAC_ADDR_LEN);
		ap_mlme_deauth_req_action(pAd, Elem);
		MlmeFreeMemory(Elem);
	}
}

void ap_send_unicast_deauth(void *ad_obj, struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, bool protected)
{
	struct _RTMP_ADAPTER *ad = ad_obj;
	HEADER_802_11	hdr;
	PUCHAR	out_buffer = NULL;
	ULONG	frame_len = 0;
	USHORT reason = MLME_UNSPECIFY_FAIL;

	/*6G PMF test*/
#ifdef DOT11W_PMF_SUPPORT
	wpa3_test_ctrl = (protected) ? 0 : 10;
#endif

	MlmeAllocateMemory(ad, &out_buffer);
	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 "AUTH - Send DE-AUTH req to %02x:%02x:%02x:%02x:%02x:%02x\n",
			  PRINT_MAC(entry->Addr));
	MgtMacHeaderInit(ad, &hdr, SUBTYPE_DEAUTH, 0, entry->Addr,
					 wdev->if_addr,
					 wdev->bssid);
	MakeOutgoingFrame(out_buffer,				&frame_len,
					  sizeof(HEADER_802_11),	&hdr,
					  2,						&reason,
					  END_OF_ARGS);
	MiniportMMRequest(ad, 0, out_buffer, frame_len);
	MlmeFreeMemory(out_buffer);
}

VOID ap_auth_init(struct wifi_dev *wdev)
{
	ap_auth_api.mlme_deauth_req_action		=	ap_mlme_deauth_req_action;
	ap_auth_api.peer_deauth_action			=	ap_peer_deauth_action;
	ap_auth_api.peer_auth_req_action		=	ap_peer_auth_req_at_idle_action;
	ap_auth_api.peer_auth_confirm_action	=	ap_peer_auth_confirm_action;
	wdev->auth_api = &ap_auth_api;
}


