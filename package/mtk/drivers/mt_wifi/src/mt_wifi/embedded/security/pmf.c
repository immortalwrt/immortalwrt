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
 ***************************************************************************/

/****************************************************************************
	Abstract:
	IEEE P802.11w

***************************************************************************/

#ifdef DOT11W_PMF_SUPPORT

#include "rt_config.h"

UCHAR OUI_PMF_BIP_CMAC_128_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x06};
UCHAR OUI_PMF_BIP_CMAC_256_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0d};
UCHAR OUI_PMF_BIP_GMAC_128_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0b};
UCHAR OUI_PMF_BIP_GMAC_256_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0c};

/* The definition in IEEE 802.11w - Table 7-34 AKM suite selectors */
UCHAR OUI_PMF_8021X_AKM[4] = {0x00, 0x0F, 0xAC, 0x05};
UCHAR OUI_PMF_PSK_AKM[4] = {0x00, 0x0F, 0xAC, 0x06};

UCHAR PMF_MMIE_BUFFER[18] = {0x4C, 0x10,
							 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
							};


VOID PMF_PeerAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_WARN, ("[PMF]%s : PMF_PeerAction Action=%d\n", __func__, Action));

	switch (Action) {
	case ACTION_SAQ_REQUEST:
		PMF_PeerSAQueryReqAction(pAd, Elem);
		break;

	case ACTION_SAQ_RESPONSE:
		PMF_PeerSAQueryRspAction(pAd, Elem);
		break;
	}
}


VOID PMF_MlmeSAQueryReq(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY * pEntry)
{
	PUCHAR pOutBuffer = NULL;
	HEADER_802_11 SAQReqHdr;
	ULONG FrameLen = 0;
	UCHAR SACategoryType, SAActionType;
	PPMF_CFG pPmfCfg = NULL;
	UCHAR oci_buf[MAX_OCI_LEN];
	ULONG oci_len = 0;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : Entry is NULL\n");
		return;
	}

	pPmfCfg = &pEntry->SecConfig.PmfCfg;

	if (pPmfCfg) {
		if ((pPmfCfg->UsePMFConnect == FALSE)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : Entry is not PMF capable, STA("MACSTR")\n", MAC2STR(pEntry->Addr));
			return;
		}

		if (pPmfCfg->SAQueryStatus == SAQ_SENDING)
			return;

		/* Send the SA Query Request */
		os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

		if (pOutBuffer == NULL)
			return;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

			if ((pEntry) && IS_ENTRY_PEER_AP(pEntry)) {
				ApCliMgtMacHeaderInit(pAd,
									  &SAQReqHdr,
									  SUBTYPE_ACTION, 0,
									  pEntry->Addr,
									  pEntry->Addr,
									  pEntry->func_tb_idx);
			} else
#endif /* APCLI_SUPPORT */
			{
				MgtMacHeaderInit(pAd, &SAQReqHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
								 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid,
								 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MgtMacHeaderInit(pAd, &SAQReqHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
							 pAd->CurrentAddress,
							 pEntry->Addr);
		}
#endif /* CONFIG_STA_SUPPORT */
		pPmfCfg->TransactionID++;
		SACategoryType = CATEGORY_SA;
		SAActionType = ACTION_SAQ_REQUEST;

		if (pEntry->SecConfig.ocv_support)
			build_oci_ie(pEntry->pAd, pEntry->wdev, oci_buf, &oci_len);

		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &SAQReqHdr,
						  1, &SACategoryType,
						  1, &SAActionType,
						  2, &pPmfCfg->TransactionID,
						  oci_len, oci_buf,
						  END_OF_ARGS);
		if (wpa3_test_ctrl != 12) {
			if (pPmfCfg->SAQueryStatus == SAQ_IDLE) {
				RTMPSetTimer(&pPmfCfg->SAQueryTimer, DEFAULT_SAQUERY_TIMEOUT);
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] -- SAQueryTimer\n");
			}

			pPmfCfg->SAQueryStatus = SAQ_SENDING;
			RTMPSetTimer(&pPmfCfg->SAQueryConfirmTimer, DEFAULT_SAQUERY_CONFIRM_TIMEOUT);
		}
		/* transmit the frame */
		MiniportMMRequest(pAd, QID_MGMT, pOutBuffer, FrameLen);
		os_free_mem(pOutBuffer);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF] - Send SA Query Request to STA("MACSTR")\n",
				  MAC2STR(pEntry->Addr));
	}
}


VOID PMF_PeerSAQueryReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	if (Action == ACTION_SAQ_REQUEST) {
		PMAC_TABLE_ENTRY pEntry = NULL;
		PFRAME_802_11 pHeader;
		USHORT TransactionID;
		PUCHAR pOutBuffer = NULL;
		HEADER_802_11 SAQRspHdr;
		ULONG FrameLen = 0;
		UCHAR SACategoryType, SAActionType;
		PPMF_CFG pPmfCfg = NULL;
		UCHAR *oci_ptr = &Elem->Msg[LENGTH_802_11 + 4];
		UCHAR oci_buf[MAX_OCI_LEN];
		ULONG oci_len = 0;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF] : Receive SA Query Request\n");
		pHeader = (PFRAME_802_11) Elem->Msg;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, pHeader->Hdr.Addr2);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, pHeader->Hdr.Addr2, Elem->wdev);
#endif

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : Entry is not found, STA("MACSTR")\n",
				 MAC2STR(pHeader->Hdr.Addr2));
			return;
		}

		pPmfCfg = &pEntry->SecConfig.PmfCfg;

		if (pPmfCfg->UsePMFConnect == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : Entry is not PMF capable, STA("MACSTR")\n",
				 MAC2STR(pHeader->Hdr.Addr2));
			return;
		}

		if (Elem->MsgLen < LENGTH_802_11 + 4) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : saq req len(%lu) is wrong, STA("MACSTR")\n",
				 Elem->MsgLen, MAC2STR(pHeader->Hdr.Addr2));
			return;
		}

		/* Fix PMF 5.3.3.4 un-protect SA Query Req. Need to ignore. */
		if (pHeader->Hdr.FC.Wep == 0) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : un-Protected SA Query Req.!!! Drop it!!, STA("MACSTR")\n",
				 MAC2STR(pHeader->Hdr.Addr2));
			return;
		}

		if (pEntry->SecConfig.ocv_support) {
			if (parse_oci_ie(pEntry->pAd, pEntry->wdev, oci_ptr, Elem->MsgLen - LENGTH_802_11 - 4) == FALSE) {
				   MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
					"[PMF] : oci check fail, drop it! STA("MACSTR")\n",
					 MAC2STR(pHeader->Hdr.Addr2));
				   return;
			} else if (pEntry->SecConfig.wait_csa_sa_query) {
				struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
				STA_TR_ENTRY *tr_entry = NULL;

				pEntry->SecConfig.wait_csa_sa_query = FALSE;
				tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
				tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
			}
		}

		NdisMoveMemory(&TransactionID, &Elem->Msg[LENGTH_802_11+2], sizeof(USHORT));
		/* Response the SA Query */
		os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

		if (pOutBuffer == NULL)
			return;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

			if (pEntry && IS_ENTRY_PEER_AP(pEntry)) {
				ApCliMgtMacHeaderInit(pAd, &SAQRspHdr,
									  SUBTYPE_ACTION, 0,
									  pHeader->Hdr.Addr2,
									  pHeader->Hdr.Addr2,
									  pEntry->func_tb_idx);
			} else
#endif /* APCLI_SUPPORT */
			{
				MgtMacHeaderInit(pAd, &SAQRspHdr, SUBTYPE_ACTION, 0, pHeader->Hdr.Addr2,
								 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid,
								 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MgtMacHeaderInit(pAd, &SAQRspHdr, SUBTYPE_ACTION, 0, pHeader->Hdr.Addr2,
							 pAd->CurrentAddress,
							 pHeader->Hdr.Addr2);
		}
#endif /* CONFIG_STA_SUPPORT */
		SACategoryType = CATEGORY_SA;
		SAActionType = ACTION_SAQ_RESPONSE;

		if (pEntry->SecConfig.ocv_support)
			build_oci_ie(pEntry->pAd, pEntry->wdev, oci_buf, &oci_len);

		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &SAQRspHdr,
						  1, &SACategoryType,
						  1, &SAActionType,
						  2, &TransactionID,
						  oci_len, oci_buf,
						  END_OF_ARGS);
		/* transmit the frame */
		MiniportMMRequest(pAd, QID_MGMT, pOutBuffer, FrameLen);
		os_free_mem(pOutBuffer);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]: Send SA Query Response to STA("MACSTR")\n", MAC2STR(SAQRspHdr.Addr1));
	}
}


VOID PMF_PeerSAQueryRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	if (Action == ACTION_SAQ_RESPONSE) {
		PMAC_TABLE_ENTRY pEntry = NULL;
		PFRAME_802_11 pHeader;
		USHORT TransactionID;
		BOOLEAN Cancelled;
		PPMF_CFG pPmfCfg = NULL;
		UCHAR *oci_ptr = &Elem->Msg[LENGTH_802_11 + 4];

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]: Receive SA Query Response\n");
		pHeader = (PFRAME_802_11) Elem->Msg;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, pHeader->Hdr.Addr2);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, pHeader->Hdr.Addr2, Elem->wdev);
#endif

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : Entry is not found, STA("MACSTR")\n",
				 MAC2STR(pHeader->Hdr.Addr2));
			return;
		}

		pPmfCfg = &pEntry->SecConfig.PmfCfg;

		if (pPmfCfg->UsePMFConnect == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : Entry is not PMF capable, STA("MACSTR")\n",
				 MAC2STR(pHeader->Hdr.Addr2));
			return;
		}

		if (Elem->MsgLen < LENGTH_802_11 + 4) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF]: saq rsp len(%lu) is wrong, STA("MACSTR")\n",
				 Elem->MsgLen, MAC2STR(pHeader->Hdr.Addr2));
			return;
		}

		if (pEntry->SecConfig.ocv_support &&
			parse_oci_ie(pEntry->pAd, pEntry->wdev, oci_ptr, Elem->MsgLen - LENGTH_802_11 - 4) == FALSE) {
			   MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : oci check fail, drop it! STA("MACSTR")\n",
				 MAC2STR(pHeader->Hdr.Addr2));
			   return;
		}

		NdisMoveMemory(&TransactionID, &Elem->Msg[LENGTH_802_11+2], sizeof(USHORT));

		if (pPmfCfg->TransactionID == TransactionID) {
			pPmfCfg->SAQueryStatus = SAQ_IDLE;
			RTMPCancelTimer(&pPmfCfg->SAQueryTimer, &Cancelled);
			RTMPCancelTimer(&pPmfCfg->SAQueryConfirmTimer, &Cancelled);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF] - Compare TransactionID correctly, STA("MACSTR")\n", MAC2STR(pHeader->Hdr.Addr2));
		} else
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
				"[PMF] - Compare TransactionID wrong, STA("MACSTR"), AP TransactionID =%d, STA TransactionID =%d\n",
				MAC2STR(pHeader->Hdr.Addr2), pPmfCfg->TransactionID, TransactionID);
	}
}


VOID PMF_SAQueryTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;

	if (pEntry) {
		RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pEntry->pAd;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF] - STA("MACSTR")\n", MAC2STR(pEntry->Addr));
#ifdef CONFIG_STA_SUPPORT

		if (IS_ENTRY_PEER_AP(pEntry)) {
			PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
			BOOLEAN Cancelled;

			RTMPCancelTimer(&pEntry->SecConfig.PmfCfg.SAQueryTimer, &Cancelled);
			RTMPCancelTimer(&pEntry->SecConfig.PmfCfg.SAQueryConfirmTimer, &Cancelled);
			cntl_disconnect_request(pEntry->wdev, CNTL_DISASSOC, pStaCfg->Bssid, REASON_DISASSOC_STA_LEAVING);
		} else
#endif /* CONFIG_STA_SUPPORT */
		{
#ifdef CONFIG_AP_SUPPORT

#ifdef MAC_REPEATER_SUPPORT
			if (IS_ENTRY_REPEATER(pEntry)
				&& IS_REPT_LINK_UP(pEntry->pReptCli))
			{
				RepeaterDisconnectRootAP(pAd, pEntry->pReptCli, APCLI_DISCONNECT_SUB_REASON_AP_PEER_DISASSOC_REQ);
			} else
#endif /* MAC_REPEATER_SUPPORT */
				mac_entry_delete(pAd, pEntry);
#endif /* CONFIG_AP_SUPPORT */
		}
	}
}



VOID PMF_SAQueryConfirmTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;

	if (pEntry) {
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF] - STA("MACSTR")\n", MAC2STR(pEntry->Addr));
		pEntry->SecConfig.PmfCfg.SAQueryStatus = SAQ_RETRY;
		PMF_MlmeSAQueryReq(pAd, pEntry);
	}
}


VOID PMF_ConstructBIPAad(
	IN PUCHAR pHdr,
	OUT UCHAR *aad_hdr)
{
	UINT8 aad_len = 0;
	/* Frame control -
		Retry bit (bit 11) masked to 0
		PwrMgt bit (bit 12) masked to 0
		MoreData bit (bit 13) masked to 0 */
	aad_hdr[0] = (*pHdr);
	aad_hdr[1] = (*(pHdr + 1)) & 0xc7;
	aad_len = 2;
	/* Append Addr 1, 2 & 3 */
	NdisMoveMemory(&aad_hdr[aad_len], pHdr + 4, 3 * MAC_ADDR_LEN);
	aad_len += (3 * MAC_ADDR_LEN);
}


BOOLEAN PMF_CalculateBIPMIC(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAadHdr,
	IN PUCHAR pFrameBuf,
	IN UINT32 FrameLen,
	IN PUCHAR pKey,
	OUT PUCHAR pBipMic)
{
	UCHAR *m_buf;
	UINT32 total_len;
	UCHAR cmac_output[16] = {0};
	UINT mlen = AES_KEY128_LENGTH;
	/* Allocate memory for MIC calculation */
	os_alloc_mem(NULL, (PUCHAR *)&m_buf, MAX_MGMT_PKT_LEN);

	if (m_buf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "out of resource.\n");
		return FALSE;
	}

	/* Initialize the buffer */
	NdisZeroMemory(m_buf, MAX_MGMT_PKT_LEN);
	/* Construct the concatenation */
	NdisMoveMemory(m_buf, pAadHdr, LEN_PMF_BIP_AAD_HDR);
	total_len = LEN_PMF_BIP_AAD_HDR;
	/* Append the Mgmt frame into the concatenation */
	NdisMoveMemory(&m_buf[total_len], pFrameBuf, FrameLen);
	total_len += FrameLen;
	/* Compute AES-128-CMAC over the concatenation */
	AES_CMAC(m_buf, total_len, pKey, 16, cmac_output, &mlen);
	/* Truncate the first 64-bits */
	NdisMoveMemory(pBipMic, cmac_output, LEN_PMF_BIP_MIC);
	os_free_mem(m_buf);
	return TRUE;
}


/*
	========================================================================

	Routine Description:
		Derive IGTK randomly
		IGTK, a hierarchy consisting of a single key to provide integrity
		protection for broadcast and multicast Robust Management frames

	Arguments:

	Return Value:

	Note:
		It's defined in IEEE 802.11w 8.5.1.3a

	========================================================================
*/
VOID PMF_DeriveIGTK(
	IN PRTMP_ADAPTER pAd,
	OUT UCHAR *output)
{
	INT i;

	for (i = 0; i < LEN_MAX_IGTK; i++)
		output[i] = RandomByte(pAd);
}

static VOID insert_igtk_kde(
	IN PRTMP_ADAPTER pAd,
	IN INT apidx,
	IN UCHAR kde_type,
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	PPMF_IGTK_KDE igtk_kde_ptr;
	UINT8 idx = 0;
	PPMF_CFG pPmfCfg = NULL;
#ifdef BCN_PROTECTION_SUPPORT
	struct bcn_protection_cfg *bcn_prot_cfg = NULL;
#endif
	UINT8 igtk_len = 0;
	UINT32 cipher = 0;
	UINT8 key_idx = 0;
	UCHAR *pn = NULL;
	UCHAR *igtk = NULL;

	if (kde_type != KDE_IGTK
#ifdef BCN_PROTECTION_SUPPORT
		&& kde_type != KDE_BIGTK
#endif
		)
		return;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (apidx < 0 || apidx >= pAd->ApCfg.BssidNum) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"apidx is invalid parameter!\n");
			return;
		}
		pPmfCfg = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg;
#ifdef BCN_PROTECTION_SUPPORT
		bcn_prot_cfg = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.bcn_prot_cfg;
#endif
	}
#endif /* CONFIG_AP_SUPPORT */

	if (!pPmfCfg)
		return;

	if (kde_type == KDE_IGTK) {
		cipher = pPmfCfg->igtk_cipher;
		key_idx = pPmfCfg->IGTK_KeyIdx;
		idx = (key_idx == 4) ? 0 : 1;
		pn = &pPmfCfg->IPN[idx][0];
		igtk = &pPmfCfg->IGTK[idx][0];
	}
#ifdef BCN_PROTECTION_SUPPORT
	else if (kde_type == KDE_BIGTK) {
		cipher = bcn_prot_cfg->bigtk_cipher;
		key_idx = bcn_prot_cfg->bigtk_key_idx;
		idx = get_bigtk_table_idx(bcn_prot_cfg);
		pn = &bcn_prot_cfg->bipn[idx][0];
		igtk = &bcn_prot_cfg->bigtk[idx][0];
	}
#endif

	/* Decide the IGTK length */
	if (IS_CIPHER_BIP_CMAC128(cipher)
		|| IS_CIPHER_BIP_GMAC128(cipher))
		igtk_len = LEN_BIP128_IGTK;
	else if (IS_CIPHER_BIP_CMAC256(cipher)
		|| IS_CIPHER_BIP_GMAC256(cipher))
		igtk_len = LEN_BIP256_IGTK;
	else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
			"unknown cipher %x.\n", cipher);
		return;
	}

	/* Construct the common KDE format */
	WPA_ConstructKdeHdr(kde_type, 8 + igtk_len, pFrameBuf);
	/* Prepare the IGTK KDE */
	igtk_kde_ptr = (PPMF_IGTK_KDE)(pFrameBuf + LEN_KDE_HDR);
	NdisZeroMemory(igtk_kde_ptr, 8 + igtk_len);
	/* Bits 0-11 define a value in the range 0-4095.
	   Bits 12 - 15 are reserved and set to 0 on transmission and ignored on reception.
	   The IGTK Key ID is either 4 or 5. The BIGTK Key ID is either 6 or 7. The remaining Key IDs are reserved. */
	igtk_kde_ptr->KeyID[0] = key_idx;
	/* Fill in the IPN field */
	NdisMoveMemory(igtk_kde_ptr->IPN, pn, LEN_WPA_TSC);
	/* Fill uin the IGTK field */
	NdisMoveMemory(igtk_kde_ptr->IGTK, igtk, igtk_len);
	/* Update the total output length */
	*pFrameLen = *pFrameLen + LEN_KDE_HDR + 8 + igtk_len;
	return;
}


/*
	========================================================================

	Routine Description:
		Insert IGTK KDE. The field shall be included in pair-Msg3-WPA2 and
		group-Msg1-WPA2.

	Arguments:

	Return Value:

	Note:

	========================================================================
*/
VOID PMF_InsertIGTKKDE(
	IN PRTMP_ADAPTER pAd,
	IN INT apidx,
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	insert_igtk_kde(pAd, apidx, KDE_IGTK, pFrameBuf, pFrameLen);
}


/*
	========================================================================

	Routine Description:
		Extract IGTK KDE.

	Arguments:

	Return Value:

	Note:

	========================================================================
*/
BOOLEAN PMF_ExtractIGTKKDE(
	IN PUCHAR pBuf,
	IN INT buf_len,
	OUT PUCHAR IGTK,
	OUT UCHAR *IGTKLEN,
	OUT PUCHAR IPN,
	OUT UINT8 *IGTK_KeyIdx)
{
	PPMF_IGTK_KDE igtk_kde_ptr;
	UINT8 offset = 0;

	igtk_kde_ptr = (PPMF_IGTK_KDE) pBuf;
	*IGTK_KeyIdx = igtk_kde_ptr->KeyID[0];
	offset += 2;
	NdisMoveMemory(IPN, igtk_kde_ptr->IPN, LEN_WPA_TSC);
	offset += LEN_WPA_TSC;

	if ((buf_len - offset) == LEN_BIP128_IGTK) {
		NdisMoveMemory(IGTK, igtk_kde_ptr->IGTK, LEN_BIP128_IGTK);
		*IGTKLEN = LEN_BIP128_IGTK;
	} else if ((buf_len - offset) == LEN_BIP256_IGTK) {
		NdisMoveMemory(IGTK, igtk_kde_ptr->IGTK, LEN_BIP256_IGTK);
		*IGTKLEN = LEN_BIP256_IGTK;
	} else {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "the IGTK length(%d) is invalid\n",
				  (buf_len - offset));
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]%s : IGTK_Key_ID=%d, IGTK length=%d\n",
			 __func__, *IGTK_KeyIdx, *IGTKLEN);
	return TRUE;
}


/*
	========================================================================

	Routine Description:
		Build Group Management Cipher in RSN-IE.
		It only shall be called by RTMPMakeRSNIE.

	Arguments:
		pAd - pointer to our pAdapter context
		ElementID - indicate the WPA1 or WPA2
		apidx - indicate the interface index

	Return Value:

	Note:

	========================================================================
*/
BOOLEAN PMF_MakeRsnIeGMgmtCipher(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{
	UINT8 *pBuf = (&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len));

	if (pSecConfig->RSNE_Type[ie_idx] == SEC_RSNIE_WPA2_IE) {
		/* default group management cipher suite in an RSNA with
		      Management Frame Protection enabled. */
		if (pSecConfig->PmfCfg.MFPC == TRUE) {
#ifdef CONFIG_HOTSPOT_R3
			if (pSecConfig->bIsWPA2EntOSEN) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO,
					"[PMF]: Return as RSN OSEN is enabled \n");
				return TRUE;
			}
#endif
			if (IS_CIPHER_BIP_CMAC128(pSecConfig->PmfCfg.igtk_cipher))
				NdisMoveMemory(pBuf, OUI_PMF_BIP_CMAC_128_CIPHER, LEN_OUI_SUITE);
			else if (IS_CIPHER_BIP_CMAC256(pSecConfig->PmfCfg.igtk_cipher))
				NdisMoveMemory(pBuf, OUI_PMF_BIP_CMAC_256_CIPHER, LEN_OUI_SUITE);
			else if (IS_CIPHER_BIP_GMAC128(pSecConfig->PmfCfg.igtk_cipher))
				NdisMoveMemory(pBuf, OUI_PMF_BIP_GMAC_128_CIPHER, LEN_OUI_SUITE);
			else if (IS_CIPHER_BIP_GMAC256(pSecConfig->PmfCfg.igtk_cipher))
				NdisMoveMemory(pBuf, OUI_PMF_BIP_GMAC_256_CIPHER, LEN_OUI_SUITE);
			else {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
					"[PMF] : insert fail, IGTK cipher is wrong\n");
				return FALSE;
			}
			(*rsn_len) += sizeof(LEN_OUI_SUITE);
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]: Insert BIP to the group management cipher of RSNIE\n");
		}
	}

	return TRUE;
}


/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
UINT PMF_RsnCapableValidation(
	IN PUINT8 pRsnie,
	IN UINT rsnie_len,
	IN BOOLEAN self_MFPC,
	IN BOOLEAN self_MFPR,
	IN UINT32 self_igtk_cipher,
	IN UCHAR end_field,
	IN struct _SECURITY_CONFIG *pSecConfigEntry)
{
	UINT8 count;
	PUINT8 pBuf = NULL;
	BOOLEAN	peer_MFPC = FALSE, peer_MFPR = FALSE;
	/* Check the peer's MPFC and MPFR -
	   Refer to Table 8-1a, IEEE 802.11W to check the PMF policy */
	pBuf = WPA_ExtractSuiteFromRSNIE(pRsnie, rsnie_len, RSN_CAP_INFO, &count);

	if (pBuf == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : Peer's MPFC isn't used.\n");

		if (self_MFPR) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : PMF policy violation.\n");
			return MLME_ROBUST_MGMT_POLICY_VIOLATION;
		}
	} else {
		RSN_CAPABILITIES RsnCap;

		NdisMoveMemory(&RsnCap, pBuf, sizeof(RSN_CAPABILITIES));
		RsnCap.word = cpu2le16(RsnCap.word);
		peer_MFPC = RsnCap.field.MFPC;
		peer_MFPR = RsnCap.field.MFPR;



		if ((self_MFPC == TRUE) && (peer_MFPC == FALSE)) {
			if ((self_MFPR == TRUE) && (peer_MFPR == FALSE)) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : PMF policy violation for case 4\n");
				return MLME_ROBUST_MGMT_POLICY_VIOLATION;
			}

			if (peer_MFPR == TRUE) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : PMF policy violation for case 7\n");
				return MLME_ROBUST_MGMT_POLICY_VIOLATION;
			}
		}

		if ((self_MFPC == TRUE) && (peer_MFPC == TRUE)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]: PMF Connection.\n");
			pSecConfigEntry->PmfCfg.UsePMFConnect = TRUE;
		}

		if (IS_AKM_OWE(pSecConfigEntry->AKMMap)) {
			pSecConfigEntry->PmfCfg.MFPC = 1;
			pSecConfigEntry->PmfCfg.MFPR = 1;
		}

		if (IS_AKM_SAE(pSecConfigEntry->AKMMap) && (peer_MFPC == FALSE)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : SAE connection fail due to not PMF connection(peer MFPR = %d, MFPC = %d)\n", peer_MFPR, peer_MFPC);
			return MLME_ROBUST_MGMT_POLICY_VIOLATION;
		}
	}

	/* SHA1 or SHA256 */
	pBuf = WPA_ExtractSuiteFromRSNIE(pRsnie, rsnie_len, AKM_SUITE, &count);

	if ((self_MFPC == TRUE)
		&& (pBuf != NULL)) {
		UCHAR OUI_WPA2_1X_SHA256[4] = {0x00, 0x0F, 0xAC, 0x05};
		UCHAR OUI_WPA2_PSK_SHA256[4] = {0x00, 0x0F, 0xAC, 0x06};
		UCHAR OUI_WPA2_SAE_SHA256[4] = {0x00, 0x0F, 0xAC, 0x08};

		while (count > 0) {
			if (RTMPEqualMemory(pBuf, OUI_WPA2_1X_SHA256, 4)
				|| RTMPEqualMemory(pBuf, OUI_WPA2_PSK_SHA256, 4)
				|| RTMPEqualMemory(pBuf, OUI_WPA2_SAE_SHA256, 4)) {
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF] : SHA256 Support\n");
			}

			pBuf += 4;
			count--;
		}
	}

	/* Group Management Cipher Suite */
	if (pSecConfigEntry->PmfCfg.UsePMFConnect == TRUE) {
		if (end_field < RSN_FIELD_GROUP_MGMT_CIPHER
			&& IS_CIPHER_BIP_CMAC128(self_igtk_cipher)) {
			pSecConfigEntry->PmfCfg.igtk_cipher = self_igtk_cipher;
			return MLME_SUCCESS;
		} else if (end_field < RSN_FIELD_GROUP_MGMT_CIPHER) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : The peer Group_mgmt_cipher_suite(default) is mismatch\n");
			return MLME_INVALID_SECURITY_POLICY;
		}

		pBuf = WPA_ExtractSuiteFromRSNIE(pRsnie, rsnie_len, G_MGMT_SUITE, &count);

		if (pBuf == NULL) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : The peer RSNIE doesn't include Group_mgmt_cipher_suite\n");
			return MLME_INVALID_SECURITY_POLICY;
		}

		if (RTMPEqualMemory(pBuf, OUI_PMF_BIP_CMAC_128_CIPHER, LEN_OUI_SUITE))
			SET_CIPHER_BIP_CMAC128(pSecConfigEntry->PmfCfg.igtk_cipher);
		else if (RTMPEqualMemory(pBuf, OUI_PMF_BIP_CMAC_256_CIPHER, LEN_OUI_SUITE))
			SET_CIPHER_BIP_CMAC256(pSecConfigEntry->PmfCfg.igtk_cipher);
		else if (RTMPEqualMemory(pBuf, OUI_PMF_BIP_GMAC_128_CIPHER, LEN_OUI_SUITE))
			SET_CIPHER_BIP_GMAC128(pSecConfigEntry->PmfCfg.igtk_cipher);
		else if (RTMPEqualMemory(pBuf, OUI_PMF_BIP_GMAC_256_CIPHER, LEN_OUI_SUITE))
			SET_CIPHER_BIP_GMAC256(pSecConfigEntry->PmfCfg.igtk_cipher);
		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : unknown peer Group_mgmt_cipher_suite\n");
			hex_dump("peer Group_mgmt_cipher_suite", pBuf, LEN_OUI_SUITE);
			return MLME_INVALID_SECURITY_POLICY;
		}

		if ((pSecConfigEntry->PmfCfg.igtk_cipher & self_igtk_cipher) == 0) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"[PMF] : peer Group_mgmt_cipher_suite(%s) is mismatch\n",
				 GetEncryModeStr(pSecConfigEntry->PmfCfg.igtk_cipher));
			CLEAR_CIPHER(pSecConfigEntry->PmfCfg.igtk_cipher);
			return MLME_INVALID_SECURITY_POLICY;
		}
	}

	return MLME_SUCCESS;
}


/*
========================================================================
Routine Description:
	Decide if the frame is PMF Robust frame

Arguments:
	pHdr		:	pointer to the 802.11 header
	pFrame		:	point to frame body. It exclude the 802.11 header
	frame_len	:	the frame length without 802.11 header

Return Value:
	NOT_ROBUST_FRAME
	UNICAST_ROBUST_FRAME
	GROUP_ROBUST_FRAME

Note:

========================================================================
*/
INT PMF_RobustFrameClassify(
	IN PHEADER_802_11 pHdr,
	IN PUCHAR pFrame,
	IN UINT	frame_len,
	IN PUCHAR pData,
	IN BOOLEAN IsRx)
{
	PMAC_TABLE_ENTRY pEntry = (PMAC_TABLE_ENTRY) pData;

	if ((pHdr->FC.Type != FC_TYPE_MGMT) || (frame_len <= 0))
		return NORMAL_FRAME;

	/* Classify the frame */
	switch (pHdr->FC.SubType) {
	case SUBTYPE_DISASSOC:
	case SUBTYPE_DEAUTH:
		break;

	case SUBTYPE_ACTION: {
		if  ((IsRx == FALSE)
			 || (IsRx && (pHdr->FC.Wep == 0))) {
			UCHAR Category = (UCHAR) (pHdr->Octet[(pHdr->FC.Order ? 4 : 0)]);

			switch (Category) {
			/* Refer to IEEE 802.11w Table7-24 */
			case CATEGORY_SPECTRUM:
			case CATEGORY_QOS:
			case CATEGORY_DLS:
			case CATEGORY_BA:
			case CATEGORY_RM:
			case CATEGORY_FT:
			case CATEGORY_SA:
			case CATEGORY_PD:
			case CATEGORY_VSP:
			case CATEGORY_WNM:
			case CATEGORY_MESH:
			case CATEGORY_MULTIHOP:
			case CATEGORY_DMG:
			case CATEGORY_FST:
			case CATEGORY_RAVS:
			case CATEGORY_PROTECTED_HE:
				break;

			default:
				return NORMAL_FRAME;
			}
		}

		break;
	}

	default:
		return NORMAL_FRAME;
	}

	if (pHdr->Addr1[0] & 0x01) { /* Broadcast frame */
		UINT8 offset_mmie;

		if (frame_len <= (LEN_PMF_MMIE + 2))
			return NOT_ROBUST_GROUP_FRAME;

		/* The offset of MMIE */
		offset_mmie = frame_len - (LEN_PMF_MMIE + 2);

		/* check if this is a group Robust frame */
		if (((*(pFrame + offset_mmie)) == IE_PMF_MMIE) &&
			((*(pFrame + offset_mmie + 1)) == LEN_PMF_MMIE))
			return GROUP_ROBUST_FRAME;
		else
			return NOT_ROBUST_GROUP_FRAME;
	}
	/* Unicast frame */
	else if (pEntry == NULL)
		return NORMAL_FRAME;
	else if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE)
		return NORMAL_FRAME;
	else if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) && (pHdr->FC.Wep == 0) && (IsRx == TRUE))
		return NOT_ROBUST_UNICAST_FRAME;
	else
		return UNICAST_ROBUST_FRAME;
}

#ifdef SOFT_ENCRYPT
INT PMF_EncryptUniRobustFrameAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMgmtFrame,
	IN UINT mgmt_len)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	PHEADER_802_11 pHdr = (PHEADER_802_11)pMgmtFrame;
	INT data_len;
	PUCHAR pBuf;
	INT Status;
	/* Check if the length is valid */
	data_len = mgmt_len - (LENGTH_802_11 + LEN_CCMP_HDR + LEN_CCMP_MIC);

	if (data_len <= 0) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "The payload length(%d) is invalid\n",
				  data_len);
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* Look up the entry through Address 1 of 802.11 header */
	pEntry = MacTableLookup(pAd, pHdr->Addr1);

	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "The entry doesn't exist\n");
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* check the PMF capable for this entry */
	if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "the entry no PMF capable !\n");
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* Allocate a buffer for building PMF packet */
	Status = MlmeAllocateMemory(pAd, &pBuf);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "allocate PMF buffer fail!\n");
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* Construct and insert 8-bytes CCMP header to MPDU header */
	RTMPConstructCCMPHdr(0, pEntry->PmfTxTsc, pBuf);
	NdisMoveMemory(pBuf + LEN_CCMP_HDR,
				   &pHdr->Octet[0],
				   data_len);
	/* Encrypt the MPDU data by software */
	RTMPSoftEncryptCCMP(pAd,
						(PUCHAR)pHdr,
						pEntry->PmfTxTsc,
						pEntry->PairwiseKey.Key,
						pBuf + LEN_CCMP_HDR,
						data_len);
	data_len += (LEN_CCMP_HDR + LEN_CCMP_MIC);
	NdisMoveMemory(&pHdr->Octet[0], pBuf, data_len);
	/* TSC increment for next transmittion */
	INC_TX_TSC(pEntry->PmfTxTsc, LEN_WPA_TSC);
	MlmeFreeMemory(pBuf);
	return PMF_STATUS_SUCCESS;
}

INT PMF_DecryptUniRobustFrameAction(
	IN PRTMP_ADAPTER pAd,
	INOUT PUCHAR pMgmtFrame,
	IN UINT	mgmt_len)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	PHEADER_802_11 pHeader = (PHEADER_802_11)pMgmtFrame;
	PUCHAR pDate = pMgmtFrame + LENGTH_802_11;
	UINT16 data_len = mgmt_len - LENGTH_802_11;

	/* Check if the length is valid */
	if (data_len <= LEN_CCMP_HDR + LEN_CCMP_MIC) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "The payload length(%d) is invalid\n",
				  data_len);
		return PMF_UNICAST_DECRYPT_FAILURE;
	}

	/* Look up the entry through Address 2 of 802.11 header */
	pEntry = MacTableLookup(pAd, pHeader->Addr2);

	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "the entry doesn't exist !\n");
		return PMF_STATUS_SUCCESS;
	}

	/* check the PMF capable for this entry */
	if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "the entry no PMF capable !\n");
		return PMF_UNICAST_DECRYPT_FAILURE;
	}

#ifdef RT_BIG_ENDIAN

	if ((pHeader->FC.SubType == SUBTYPE_DISASSOC) || (pHeader->FC.SubType == SUBTYPE_DEAUTH))
		*(USHORT *)pDate = cpu2le16(*(USHORT *)pDate); /* swap reason code */

#endif /* RT_BIG_ENDIAN */

	if (RTMPSoftDecryptCCMP(pAd,
							pMgmtFrame,
							&pEntry->PairwiseKey,
							pDate,
							&data_len) == FALSE)
		return PMF_UNICAST_DECRYPT_FAILURE;

	return PMF_STATUS_SUCCESS;
}


INT PMF_EncapBIPAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMgmtFrame,
	IN UINT	mgmt_len)
{
	PHEADER_802_11 pHdr = (PHEADER_802_11)pMgmtFrame;
	PPMF_CFG pPmfCfg = NULL;
	PPMF_MMIE pMMIE;
	INT idx = 0;
	PUCHAR pKey = NULL;
	UCHAR aad_hdr[LEN_PMF_BIP_AAD_HDR];
	UCHAR BIP_MIC[LEN_PMF_BIP_MIC];
	PUCHAR pFrameBody = &pHdr->Octet[0];
	UINT32 body_len = mgmt_len - LENGTH_802_11;
#ifdef RT_BIG_ENDIAN
	PUCHAR pMacHdr = NULL;
	BOOLEAN bSwaped = FALSE;
#endif

	/* Sanity check the total frame body length */
	if (body_len <= (2 + LEN_PMF_MMIE)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : the total length(%d) is too short\n",
				 body_len);
		return PMF_ENCAP_BIP_FAILURE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pPmfCfg = &pAd->ApCfg.MBSSID[MAIN_MBSSID].PmfCfg;
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Sanity check */
	if (pPmfCfg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : No related PMF configuation\n");
		return PMF_ENCAP_BIP_FAILURE;
	}

	if (pPmfCfg && pPmfCfg->MFPC == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : PMF is disabled\n");
		return PMF_ENCAP_BIP_FAILURE;
	}

	/* Pointer to the position of MMIE */
	pMMIE = (PPMF_MMIE)(pMgmtFrame + (mgmt_len - LEN_PMF_MMIE));

	/*  Select the IGTK currently active for transmission of frames to
	    the intended group of recipients and construct the MMIE (see 7.3.2.55)
	    with the MIC field masked to zero and the KeyID field set to the
	    corresponding IGTK KeyID value. */
	if (pPmfCfg->IGTK_KeyIdx == 5)
		idx = 1;

	pKey = &pPmfCfg->IGTK[idx][0];
	NdisZeroMemory(pMMIE, LEN_PMF_MMIE);
	/* Bits 0-11 define a value in the range 0-4095.
	   Bits 12 - 15 are reserved and set to 0 on transmission and ignored on reception.
	   The IGTK Key ID is either 4 or 5. The remaining Key IDs are reserved. */
	pMMIE->KeyID[0] = pPmfCfg->IGTK_KeyIdx;
	NdisMoveMemory(pMMIE->IPN, &pPmfCfg->IPN[idx][0], LEN_WPA_TSC);
	/* The transmitter shall insert a monotonically increasing non-neg-
	   ative integer into the MMIE IPN field. */
	INC_TX_TSC(pPmfCfg->IPN[idx], LEN_WPA_TSC);
	/* Compute AAD  */
#ifdef RT_BIG_ENDIAN

	if (pHdr->FC.SubType == SUBTYPE_DISASSOC || pHdr->FC.SubType == SUBTYPE_DEAUTH) {
		pMacHdr = (PUCHAR) pHdr;
		*(USHORT *)pMacHdr = cpu2le16(*(USHORT *)pMacHdr); /* swap frame-control */
		pMacHdr += sizeof(HEADER_802_11);
		*(USHORT *)pMacHdr = cpu2le16(*(USHORT *)pMacHdr);	/* swap reason code */
		bSwaped = TRUE;
	}

#endif
	PMF_ConstructBIPAad((PUCHAR)pHdr, aad_hdr);
	/* Calculate BIP MIC */
	PMF_CalculateBIPMIC(pAd, aad_hdr, pFrameBody, body_len, pKey, BIP_MIC);
	/* Fill into the MMIE MIC field */
	NdisMoveMemory(pMMIE->MIC, BIP_MIC, LEN_PMF_BIP_MIC);
#ifdef RT_BIG_ENDIAN

	if (bSwaped) {
		pMacHdr = (PUCHAR) pHdr;
		*(USHORT *)pMacHdr = cpu2le16(*(USHORT *)pMacHdr);
		pMacHdr += sizeof(HEADER_802_11);
		*(USHORT *)pMacHdr = cpu2le16(*(USHORT *)pMacHdr);
		bSwaped = TRUE;
	}

#endif
	/* BIP doesn't need encrypt frame */
	pHdr->FC.Wep = 0;
	return PMF_STATUS_SUCCESS;
}

INT PMF_ExtractBIPAction(
	IN PRTMP_ADAPTER pAd,
	INOUT PUCHAR pMgmtFrame,
	IN UINT	mgmt_len)
{
	PPMF_CFG pPmfCfg = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PHEADER_802_11 pHeader = (PHEADER_802_11)pMgmtFrame;
	PPMF_MMIE pMMIE;
	INT idx = 0;
	PUCHAR pKey = NULL;
	UCHAR aad_hdr[LEN_PMF_BIP_AAD_HDR];
	UCHAR rcvd_mic[LEN_PMF_BIP_MIC];
	UCHAR cal_mic[LEN_PMF_BIP_MIC];
	UINT32 body_len = mgmt_len - LENGTH_802_11;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif

	/* Sanity check the total frame body length */
	if (body_len <= (2 + LEN_PMF_MMIE)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "the total length(%d) is too short\n",
				  body_len));
		return PMF_EXTRACT_BIP_FAILURE;
	}

	/* Look up the entry through Address 2 of 802.11 header */
	pEntry = MacTableLookup(pAd, pHeader->Addr2);
#ifdef CONFIG_STA_SUPPORT
	pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
	ASSERT(pStaCfg);
#endif

	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "the entry doesn't exist !\n");
		return PMF_STATUS_SUCCESS;
	}

	/* check the PMF capable for this entry */
	if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "the entry no PMF capable !\n");
		return PMF_EXTRACT_BIP_FAILURE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/* MT7615 only need H/W  Decrypt, SOFT_ENCRYPT of BIP is not need */
#ifdef APCLI_SUPPORT
		if (IS_ENTRY_PEER_AP(pEntry))
			pPmfCfg = &pAd->StaCfg[pEntry->func_tb_idx].wdev.SecConfig.PmfCfg;
		else
#endif /* APCLI_SUPPORT */
			pPmfCfg = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PmfCfg;
	}
#endif /* CONFIG_AP_SUPPORT // */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pPmfCfg = &pStaCfg.wdev.SecConfig.PmfCfg;
	}
#endif /* CONFIG_STA_SUPPORT // */
	/* Pointer to the position of MMIE */
	pMMIE = (PPMF_MMIE)(pMgmtFrame + (mgmt_len - LEN_PMF_MMIE));

	/*  Select the IGTK currently active for transmission of frames to
	    the intended group of recipients and construct the MMIE (see 7.3.2.55)
	    with the MIC field masked to zero and the KeyID field set to the
	    corresponding IGTK KeyID value. */
	if (pMMIE->KeyID[0] == 5)
		idx = 1;

	pKey = &pPmfCfg->IGTK[idx][0];
	/* store the MIC value of the received frame */
	NdisMoveMemory(rcvd_mic, pMMIE->MIC, LEN_PMF_BIP_MIC);
	NdisZeroMemory(pMMIE->MIC, LEN_PMF_BIP_MIC);
	/* Compute AAD  */
	PMF_ConstructBIPAad((PUCHAR)pMgmtFrame, aad_hdr);
	/* Calculate BIP MIC */
	PMF_CalculateBIPMIC(pAd, aad_hdr,
						pMgmtFrame + LENGTH_802_11,
						body_len, pKey, cal_mic);

	if (!NdisEqualMemory(rcvd_mic, cal_mic, LEN_PMF_BIP_MIC)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "MIC Different !\n");
		return PMF_EXTRACT_BIP_FAILURE;
	}

	return PMF_STATUS_SUCCESS;
}
#endif /* SOFT_ENCRYPT */


VOID PMF_AddMMIE(
	IN PMF_CFG * pPmfCfg,
	IN PUCHAR pMgmtFrame,
	IN UINT	mgmt_len)
{
	PHEADER_802_11 pHdr = (PHEADER_802_11)pMgmtFrame;
	PPMF_MMIE pMMIE;
	UINT8 idx = 0;
	UINT32 body_len = mgmt_len - LENGTH_802_11;

	/* Sanity check the total frame body length */
	if (body_len <= (2 + LEN_PMF_MMIE)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF]: the total length(%d) is too short\n",
				  body_len);
		return;
	}

	/* Sanity check */
	if (pPmfCfg == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : No related PMF configuation\n");
		return;
	}

	if (pPmfCfg && pPmfCfg->MFPC == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : PMF is disabled\n");
		return;
	}

	/* Pointer to the position of MMIE */
	pMMIE = (PPMF_MMIE)(pMgmtFrame + (mgmt_len - LEN_PMF_MMIE));

	/*  Select the IGTK currently active for transmission of frames to
	    the intended group of recipients and construct the MMIE (see 7.3.2.55)
	    with the MIC field masked to zero and the KeyID field set to the
	    corresponding IGTK KeyID value. */
	if (pPmfCfg->IGTK_KeyIdx == 5)
		idx = 1;

	os_zero_mem((char *) pMMIE, LEN_PMF_MMIE);
	/* Bits 0-11 define a value in the range 0-4095.
	   Bits 12 - 15 are reserved and set to 0 on transmission and ignored on reception.
	   The IGTK Key ID is either 4 or 5. The remaining Key IDs are reserved. */
	pMMIE->KeyID[0] = pPmfCfg->IGTK_KeyIdx;
	os_move_mem(pMMIE->IPN, &pPmfCfg->IPN[idx][0], LEN_WPA_TSC);
	/* The transmitter shall insert a monotonically increasing non-neg-
	   ative integer into the MMIE IPN field. */
	INC_TX_TSC(pPmfCfg->IPN[idx], LEN_WPA_TSC);
	/* BIP doesn't need encrypt frame */
	pHdr->FC.Wep = 0;
	return;
}

BOOLEAN PMF_PerformTxFrameAction(
	IN PRTMP_ADAPTER pAd,
	IN PHEADER_802_11 pHeader_802_11,
	IN UINT SrcBufLen,
	IN UINT8 tx_hw_hdr_len,
	OUT UCHAR *prot)
{
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		UINT32 ret = 0;
		MAC_TABLE_ENTRY *pEntry = NULL;
		struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
		struct _STA_TR_ENTRY *tr_entry = NULL;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, pHeader_802_11->Addr1, NULL);
#endif
		*prot = 0;

		if (pEntry) {
			tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
			if (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED &&
				!pEntry->SecConfig.wait_csa_sa_query)
				return TRUE;
		}

		/*6G PMF Test, AP Testbed, one shut only then recover*/
#ifdef DOT11W_PMF_SUPPORT
		if (wpa3_test_ctrl == 10) {
			wpa3_test_ctrl = 0;
			return TRUE;
		}
#endif /*DOT11W_PMF_SUPPORT*/
		ret = PMF_RobustFrameClassify(
				  (PHEADER_802_11)pHeader_802_11,
				  (PUCHAR)(((PUCHAR)pHeader_802_11)+LENGTH_802_11),
				  (SrcBufLen - LENGTH_802_11 - tx_hw_hdr_len),
				  (PUCHAR) pEntry,
				  FALSE);

		if (ret == UNICAST_ROBUST_FRAME) {
			*prot = 1;
			pHeader_802_11->FC.Wep = 1;
		} else if (ret == GROUP_ROBUST_FRAME) {
#ifdef SOFT_ENCRYPT
			ret = PMF_EncapBIPAction(pAd,
									 (UCHAR *) pHeader_802_11,
									 (SrcBufLen - tx_hw_hdr_len));

			if (ret == PMF_STATUS_SUCCESS)
				*prot = 3;
			else
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " PMF GROUP ROBUST Encap fail, ret=%d\n",
						 ret);

#else
			PMF_CFG *pPmfCfg = NULL;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				struct wifi_dev *wdev = NULL;

				if (pHeader_802_11->Addr1[0] & 0x01) /* Broadcast frame */
				  wdev = wdev_search_by_address(pAd, pHeader_802_11->Addr2);
				else if (pEntry != NULL)
				  wdev = &pAd->ApCfg.MBSSID[pEntry->apidx].wdev;

				if (wdev)
				  pPmfCfg = &wdev->SecConfig.PmfCfg;
				else
				  pPmfCfg = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.SecConfig.PmfCfg;
			}
#endif /* CONFIG_AP_SUPPORT */
			PMF_AddMMIE(pPmfCfg,
						(UCHAR *) pHeader_802_11,
						(SrcBufLen - tx_hw_hdr_len));
			if (pPmfCfg == NULL || IS_CIPHER_BIP_CMAC128(pPmfCfg->igtk_cipher))
				*prot = 2;
			else
				*prot = 3;
#endif /* SOFT_ENCRYPT */
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, " PMF GROUP ROBUST\n");
		}
	}

	return TRUE;
}


/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
BOOLEAN PMF_PerformRxFrameAction(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	INT FrameType = NORMAL_FRAME;
	PUCHAR pMgmtFrame;
	UINT mgmt_len;
	PMAC_TABLE_ENTRY pEntry = NULL;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;

	pMgmtFrame = (PUCHAR)FC;
	mgmt_len = pRxBlk->MPDUtotalByteCnt;

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

	if ((pRxBlk->Addr1[0] & 0x01) &&
		(FC->Type == FC_TYPE_MGMT) &&
		((FC->SubType == SUBTYPE_DISASSOC) || (FC->SubType == SUBTYPE_DEAUTH))) {
#ifdef CONFIG_AP_SUPPORT
		/* MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF]: Bcast  FRAME, FC->SubType=%d, pRxBlk->pRxInfo->U2M=%d, pRxBlk->pRxInfo->disasso=%d, pRxBlk->pRxInfo->Decrypted=%d, pRxBlk->wcid=%d\n", */
		/* FC->SubType, pRxBlk->pRxInfo->U2M, pRxBlk->pRxInfo->disasso, pRxBlk->pRxInfo->Decrypted,pRxBlk->wcid); */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pEntry = MacTableLookup2(pAd, pRxBlk->Addr2, NULL);
#endif

		if (!pEntry)
			return TRUE;
		else {
			if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE)
				return TRUE;
		}
	}

	FrameType = PMF_RobustFrameClassify((HEADER_802_11 *)pRxBlk->FC,
							(PUCHAR)(pMgmtFrame + LENGTH_802_11),
							(mgmt_len - LENGTH_802_11),
							(PUCHAR) pEntry,
							TRUE);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		switch (FrameType) {
		case NORMAL_FRAME:
			break;

		case NOT_ROBUST_GROUP_FRAME:
#ifdef APCLI_SUPPORT

			/* H/W Decrypt case won't fall into below case */
			if ((pEntry) && IS_ENTRY_PEER_AP(pEntry)) {
				if (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
					return FALSE;
			}

#endif /* APCLI_SUPPORT */
			break;

		case NOT_ROBUST_UNICAST_FRAME:
#ifdef APCLI_SUPPORT
			if ((pEntry) && IS_ENTRY_PEER_AP(pEntry)) {
				if (((FC->SubType == SUBTYPE_DISASSOC) || (FC->SubType == SUBTYPE_DEAUTH))
					&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)) {
					PMF_MlmeSAQueryReq(pAd, pEntry);
					return FALSE;
				}
			}

#endif /* APCLI_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
					 "[PMF] : NOT_ROBUST_UNICAST_FRAME, FC->SubType=%d (wcid=%d)\n",
					  FC->SubType, pRxBlk->wcid);
			return FALSE;

		case UNICAST_ROBUST_FRAME: {
#ifdef SOFT_ENCRYPT

			if (PMF_DecryptUniRobustFrameAction(pAd,
												pMgmtFrame,
												mgmt_len) != PMF_STATUS_SUCCESS)
				return FALSE;

			pRxBlk->MPDUtotalByteCnt -= (LEN_CCMP_HDR + LEN_CCMP_MIC);
#endif /* SOFT_ENCRYPT */
			break;
		}

		case GROUP_ROBUST_FRAME: {
#ifdef SOFT_ENCRYPT

			if (PMF_ExtractBIPAction(pAd,
									 pMgmtFrame,
									 mgmt_len) != PMF_STATUS_SUCCESS)
				return FALSE;

#endif /* SOFT_ENCRYPT */
			/* MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF]: GROUP_ROBUST_FRAME, FC->SubType=%d\n",  FC->SubType); */
			pRxBlk->MPDUtotalByteCnt -= (2 + LEN_PMF_MMIE);
			break;
		}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		switch (FrameType) {
		case NORMAL_FRAME:
			break;

		case NOT_ROBUST_UNICAST_FRAME:
			if (((FC->SubType == SUBTYPE_DISASSOC) || (FC->SubType == SUBTYPE_DEAUTH))
				&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)) {
				PMF_MlmeSAQueryReq(pAd, pEntry);
				return FALSE;
			}

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] : NOT_ROBUST_UNICAST_FRAME, FC->SubType=%d\n", FC->SubType);
			return FALSE;

		case NOT_ROBUST_GROUP_FRAME:
			if ((pEntry) && (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE))
				return FALSE;
			else
				break;

		case UNICAST_ROBUST_FRAME: {
#ifdef SOFT_ENCRYPT

			if (!IS_HIF_TYPE(pAd, HIF_MT)) {
				if (PMF_DecryptUniRobustFrameAction(pAd,
													pMgmtFrame,
													mgmt_len) != PMF_STATUS_SUCCESS)
					return FALSE;

				pRxBlk->MPDUtotalByteCnt -= (LEN_CCMP_HDR + LEN_CCMP_MIC);
			}

#endif /* SOFT_ENCRYPT */
			break;
		}

		case GROUP_ROBUST_FRAME: {
#ifdef SOFT_ENCRYPT

			if (!IS_HIF_TYPE(pAd, HIF_MT)) {
				if (PMF_ExtractBIPAction(pAd,
										 pMgmtFrame,
										 mgmt_len) != PMF_STATUS_SUCCESS)
					return FALSE;

				pRxBlk->MPDUtotalByteCnt -= (2 + LEN_PMF_MMIE);
			}

#endif /* SOFT_ENCRYPT */
			break;
		}
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}


/*
========================================================================
Routine Description:
    Protection Management Frame Capable
    Protection Management Frame Required

Arguments:

Return Value:

Note:
RSNA policy selection in a ESS: IEEE P802.11w Table 8-1a
RSNA policy selection in an IBSS: IEEE P802.11w Table 8-1b
========================================================================
*/
void rtmp_read_pmf_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		INT apidx;
		POS_COOKIE pObj;
		RTMP_STRING *macptr;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;
		}

		/* Protection Management Frame Capable */
		if (RTMPGetKeyParameter("PMFMFPC", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pObj->ioctl_if = apidx;
				Set_PMFMFPC_Proc(pAd, macptr);
			}
		}

		/* Protection Management Frame Required */
		if (RTMPGetKeyParameter("PMFMFPR", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pObj->ioctl_if = apidx;
				Set_PMFMFPR_Proc(pAd, macptr);
			}
		}

		if (RTMPGetKeyParameter("PMFSHA256", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pObj->ioctl_if = apidx;
				Set_PMFSHA256_Proc(pAd, macptr);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		INT staidx;
		POS_COOKIE pObj;
		RTMP_STRING *macptr;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;
		}

		/* Protection Management Frame Capable */
		if (RTMPGetKeyParameter("PMFMFPC", tmpbuf, 32, pBuffer, TRUE)) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pObj->ioctl_if = staidx;
				Set_PMFMFPC_Proc(pAd, macptr);
			}
		}

		/* Protection Management Frame Required */
		if (RTMPGetKeyParameter("PMFMFPR", tmpbuf, 32, pBuffer, TRUE)) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pObj->ioctl_if = staidx;
				Set_PMFMFPR_Proc(pAd, macptr);
			}
		}

		if (RTMPGetKeyParameter("PMFSHA256", tmpbuf, 32, pBuffer, TRUE)) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pObj->ioctl_if = staidx;
				Set_PMFSHA256_Proc(pAd, macptr);
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */
}

/*
========================================================================
Routine Description: Protection Management Frame Capable

Arguments:

Return Value:

Note:
RSNA policy selection in a ESS: IEEE P802.11w Table 8-1a
RSNA policy selection in an IBSS: IEEE P802.11w Table 8-1b
========================================================================
*/
INT Set_PMFMFPC_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (strlen(arg) == 0)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj;
		UINT32 apidx = 0;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->ApCfg.BssidNum) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"pObj->ioctl_if is invalid value\n");
			return FALSE;
		}

		apidx = pObj->ioctl_if;
		if (os_str_tol(arg, 0, 10))
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPC = TRUE;
		else
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, "[PMF] :: apidx=%d, Desired MFPC=%d\n"
				 , apidx, pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPC);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		POS_COOKIE pObj;
		UINT32 staidx = 0;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"pObj->ioctl_if is invalid value\n");
			return FALSE;
		}

		staidx = pObj->ioctl_if;
		if (os_str_tol(arg, 0, 10))
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPC = TRUE;
		else
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]: staidx=%d, Desired MFPC=%d\n",
				 staidx, pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPC);
	}
#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}


/*
========================================================================
Routine Description: Protection Management Frame Required

Arguments:

Return Value:

Note:
RSNA policy selection in a ESS: IEEE P802.11w Table 8-1a
RSNA policy selection in an IBSS: IEEE P802.11w Table 8-1b
========================================================================
*/
INT Set_PMFMFPR_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (strlen(arg) == 0)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj;
		UINT32 apidx = 0;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->ApCfg.BssidNum) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"pObj->ioctl_if is invalid value\n");
			return FALSE;
		}

		apidx = pObj->ioctl_if;
		if (os_str_tol(arg, 0, 10))
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPR = TRUE;
		else
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]: apidx=%d, Desired MFPR=%d\n"
				 , apidx, pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPR);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		POS_COOKIE pObj;
		UINT32 staidx = 0;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				 "pObj->ioctl_if is invalid value\n");
			return FALSE;
		}

		staidx = pObj->ioctl_if;
		if (os_str_tol(arg, 0, 10))
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPR = TRUE;
		else
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]: staidx=%d, Desired MFPR=%d\n"
				 , staidx, pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_MFPR);
	}
#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}


INT Set_PMFSHA256_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (strlen(arg) == 0)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj;
		UINT32 apidx = 0;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->ApCfg.BssidNum) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"pObj->ioctl_if is invalid value\n");
			return FALSE;
		}

		apidx = pObj->ioctl_if;
		if (os_str_tol(arg, 0, 10))
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = TRUE;
		else
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]: apidx=%d, Desired PMFSHA256=%d\n"
				 , apidx, pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		POS_COOKIE pObj;
		UINT32 staidx = 0;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				"pObj->ioctl_if is invalid value\n");
			return FALSE;
		}

		staidx = pObj->ioctl_if;
		if (os_str_tol(arg, 0, 10))
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = TRUE;
		else
			pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF]: staidx=%d, Desired PMFSHA256=%d\n"
				 , staidx, pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256);
	}
#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

INT Set_PMFSA_Q_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;
	BOOLEAN search_by_mac = FALSE;
	UINT8 mac_addr[MAC_ADDR_LEN];
	UINT16 idx = 0, wcid = 0;
	PMAC_TABLE_ENTRY pEntry = NULL;
	RTMP_STRING *value = NULL;

	if (strlen(arg) <= 17 && strlen(arg) >= 11) {
		UCHAR i = 0;

		for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
			if ((strlen(value) == 1 && !isxdigit(*value)) ||
				(strlen(value) == 2 && (!isxdigit(*value) || !isxdigit(*(value + 1)))) ||
				(strlen(value) != 1 && strlen(value) != 2))
				return FALSE;  /*Invalid */

			AtoH(value, (UCHAR *)&mac_addr[i], 1);
		}
		search_by_mac = TRUE;
	} else
		wcid = (UINT16)os_str_tol(arg, 0, 10);

	if (search_by_mac)
		pEntry = MacTableLookup(pAd, mac_addr);
	else {
		for (idx = 0; idx < HcGetMaxStaNum(pAd); idx++) {
			pEntry = &pAd->MacTab.Content[idx];

			if (pEntry && !IS_ENTRY_NONE(pEntry) && pEntry->wcid == wcid)
				break;
		}
	}

	if (!pEntry)
		return FALSE;

	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
	if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) &&
		(tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				 "PMF Connection IGNORE THIS PKT DUE TO NOT IN PORTSECURED(wcid = %d)\n", pEntry->wcid);
		return FALSE;
	}

	if (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				 "PMF CONNECTION BUT RECV WEP=0 ACTION, ACTIVE THE SA QUERY(wcid = %d)\n", pEntry->wcid);
		PMF_MlmeSAQueryReq(pAd, pEntry);
		return TRUE;
	}
#endif /* CONFIG_AP_SUPPORT */
	return FALSE;

}

#ifdef APCLI_SUPPORT
/* chane the cmd depend on security mode first, and update to run time flag*/
INT Set_ApCliPMFMFPC_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	PMF_CFG *pPmfCfg = NULL;
	struct wifi_dev *wdev = NULL;
	UINT32 staidx = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;
	pPmfCfg = &pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg;
	wdev = &pAd->StaCfg[staidx].wdev;

	if (!pPmfCfg || !wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PMF] : pPmfCfg=%p, wdev=%p\n",
				  pPmfCfg, wdev);
		return FALSE;
	}

	if (os_str_tol(arg, 0, 10))
		pPmfCfg->Desired_MFPC = TRUE;
	else {
		pPmfCfg->Desired_MFPC = FALSE;
		pPmfCfg->MFPC = FALSE;
		pPmfCfg->MFPR = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PMF]: Desired MFPC=%d\n", pPmfCfg->Desired_MFPC);

	if ((IS_AKM_WPA2_Entry(wdev) || IS_AKM_WPA2PSK_Entry(wdev)
		|| IS_AKM_WPA3PSK_Entry(wdev) || IS_AKM_OWE_Entry(wdev)
#ifdef DPP_SUPPORT
		|| IS_AKM_DPP_Entry(wdev)
#endif /* DPP_SUPPORT */
		) && IS_CIPHER_AES_Entry(wdev)) {
		pPmfCfg->PMFSHA256 = pPmfCfg->Desired_PMFSHA256;

		if (pPmfCfg->Desired_MFPC) {
			pPmfCfg->MFPC = TRUE;
			pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;

			if (pPmfCfg->MFPR)
				pPmfCfg->PMFSHA256 = TRUE;
		}
	} else if (pPmfCfg->Desired_MFPC)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[PMF]: Security is not WPA2/WPA2PSK AES\n");

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PMF]: MFPC=%d, MFPR=%d, SHA256=%d\n",
			 pPmfCfg->MFPC,
			 pPmfCfg->MFPR,
			 pPmfCfg->PMFSHA256);
	return TRUE;
}
/* chane the cmd depend on security mode first, and update to run time flag*/
INT Set_ApCliPMFMFPR_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	PMF_CFG *pPmfCfg = NULL;
	struct wifi_dev *wdev = NULL;
	UINT32 staidx = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;
	pPmfCfg = &pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg;
	wdev = &pAd->StaCfg[staidx].wdev;

	if (!pPmfCfg || !wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PMF]: pPmfCfg=%p, wdev=%p\n",
				  pPmfCfg, wdev);
		return FALSE;
	}

	if (os_str_tol(arg, 0, 10))
		pPmfCfg->Desired_MFPR = TRUE;
	else {
		pPmfCfg->Desired_MFPR = FALSE;
		/* only close the MFPR */
		pPmfCfg->MFPR = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PMF]: Desired MFPR=%d\n", pPmfCfg->Desired_MFPR);

	if ((IS_AKM_WPA2_Entry(wdev) || IS_AKM_WPA2PSK_Entry(wdev)
		|| IS_AKM_WPA3PSK_Entry(wdev)
		|| IS_AKM_OWE_Entry(wdev)
		) && IS_CIPHER_AES_Entry(wdev)) {
		pPmfCfg->PMFSHA256 = pPmfCfg->Desired_PMFSHA256;

		if (pPmfCfg->Desired_MFPC) {
			pPmfCfg->MFPC = TRUE;
			pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;

			if (pPmfCfg->MFPR)
				pPmfCfg->PMFSHA256 = TRUE;
		}
	} else if (pPmfCfg->Desired_MFPC)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[PMF]: Security is not WPA2/WPA2PSK AES\n");

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PMF]: MFPC=%d, MFPR=%d, SHA256=%d\n",
			 pPmfCfg->MFPC,
			 pPmfCfg->MFPR,
			 pPmfCfg->PMFSHA256);
	return TRUE;
}
INT Set_ApCliPMFSHA256_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	PMF_CFG *pPmfCfg = NULL;
	UINT32 staidx = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;
	pPmfCfg = &pAd->StaCfg[staidx].wdev.SecConfig.PmfCfg;

	if (!pPmfCfg) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PMF]: pPmfCfg=%p\n",
				  pPmfCfg);
		return FALSE;
	}

	if (os_str_tol(arg, 0, 10))
		pPmfCfg->Desired_PMFSHA256 = TRUE;
	else
		pPmfCfg->Desired_PMFSHA256 = FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PMF]: Desired PMFSHA256=%d\n",
			  pPmfCfg->Desired_PMFSHA256);
	return TRUE;
}
#endif /* APCLI_SUPPORT */

#ifdef BCN_PROTECTION_SUPPORT
VOID read_bcn_prot_parma_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	INT i = 0;
	struct _SECURITY_CONFIG *sec_cfg = NULL;
	RTMP_STRING *macptr;
#endif

	if (RTMPGetKeyParameter("BcnProt", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR bcn_prot = 0;
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					bcn_prot = os_str_tol(macptr, 0, 10);
				sec_cfg->bcn_prot_cfg.desired_bcn_prot_en = (bcn_prot) ? TRUE : FALSE;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.bcn_prot_cfg.desired_bcn_prot_en = FALSE;
			}
#endif /* CONFIG_AP_SUPPORT */
		}
}

INT build_bcn_mmie(
	IN struct bcn_protection_cfg *bcn_prot_cfg,
	IN UCHAR *buf)
{
	UCHAR ie = IE_MME;
	UCHAR ie_len = LEN_PMF_MMIE;

	if (!bcn_prot_cfg->bcn_prot_en)
		return 0;

	os_zero_mem(buf, LEN_PMF_MMIE + 2);
	os_move_mem(buf, &ie, 1);
	os_move_mem(buf + 1, &ie_len, 1);
	os_move_mem(buf + 2, &bcn_prot_cfg->bigtk_key_idx, 1);
	os_move_mem(buf + 4, &bcn_prot_cfg->bipn[get_bigtk_table_idx(bcn_prot_cfg)][0], LEN_WPA_TSC);

	hex_dump_with_cat_and_lvl("bcn_mmie", buf, LEN_PMF_MMIE + 2, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_INFO);

	return LEN_PMF_MMIE + 2;
}


VOID insert_bigtk_kde(
	IN PRTMP_ADAPTER pAd,
	IN INT apidx,
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	insert_igtk_kde(pAd, apidx, KDE_BIGTK, pFrameBuf, pFrameLen);
	hex_dump_with_cat_and_lvl("bigtk kde", pFrameBuf,
		LEN_KDE_HDR + 8 + LEN_BIP128_IGTK, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_INFO); /* todo: change to trace*/
}

UCHAR get_bigtk_table_idx(struct bcn_protection_cfg *bcn_prot_cfg)
{
	return (bcn_prot_cfg->bigtk_key_idx == 6) ? 0 : 1;
}

/* please update bipn in wdev->SecConfig.bcn_prot_cfg.bipn first */
VOID bcn_prot_update_bipn(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev)
{
	struct _BSS_INFO_ARGUMENT_T bss;
	UCHAR idx = get_bigtk_table_idx(&wdev->SecConfig.bcn_prot_cfg);

	memcpy(&bss, &wdev->bss_info_argument, sizeof(bss));
	bss.u4BssInfoFeature = BSS_INFO_BCN_PROT_FEATURE;
	if (wpa3_test_ctrl == 7)
		os_zero_mem(&bss.bcn_prot_cfg.bipn[idx][0], LEN_WPA_TSC);

	AsicBssInfoUpdate(ad, &bss);
}
#endif
#endif /* DOT11W_PMF_SUPPORT */

