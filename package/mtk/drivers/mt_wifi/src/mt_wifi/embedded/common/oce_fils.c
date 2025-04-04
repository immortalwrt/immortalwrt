#include "rt_config.h"

static INT build_fils_indication_element(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *buf)
{
	FILS_INFORMATION FilsInfo;
	INT len = 0;
	UCHAR *IeLen;
	UCHAR filsIndIe = IE_FILS_INDICATION, filsIndIeLen = 0;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
	OCE_CTRL *oceCtrl = &wdev->OceCtrl;

	NdisZeroMemory(&FilsInfo, sizeof(FILS_INFORMATION));
	MAKE_IE_TO_BUF(buf, &filsIndIe, 1, len);

	IeLen = buf + len;
	MAKE_IE_TO_BUF(buf, &filsIndIeLen, 1, len);

	if (oceCtrl->FilsRealmsHash)
		FilsInfo.NumOfRealmID = 1;

	if (oceCtrl->FilsCacheId)
		FilsInfo.CacheIDIncluded = 1;

	FilsInfo.NumOfPublicKeyID = 0;
	FilsInfo.FilsIPConf = 0;
	FilsInfo.HESSIDIncluded = 0;

	if (IS_AKM_FILS_SHA256(pSecConfig->AKMMap) ||
		IS_AKM_FILS_SHA384(pSecConfig->AKMMap))
		FilsInfo.FilsSKAuthNoPFS = 1;

	FilsInfo.FilsSKAuthPFS = 0;
	FilsInfo.FilsPublicKeyAuth = 0;
	MAKE_IE_TO_BUF(buf, &FilsInfo, 2, len);

	if (FilsInfo.CacheIDIncluded)
		MAKE_IE_TO_BUF(buf, &oceCtrl->FilsCacheId, FILS_CACHE_ID_LEN, len);

	if (FilsInfo.NumOfRealmID)
		MAKE_IE_TO_BUF(buf, &oceCtrl->FilsRealmsHash, FILS_REALMS_HASH_LEN, len);

	*IeLen = (len - 2); /* IE and Len */
	return len;
}

static INT build_fils_discovery_rsn_info(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *buf)
{
	FD_RSN_INFO FdRsnInfo;
	RSN_CAPABILITIES *pRSN_Cap = &FdRsnInfo.RSNCap;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
	INT len = 0;

	NdisZeroMemory(&FdRsnInfo, sizeof(FD_RSN_INFO));
#ifdef DOT1X_SUPPORT
	pRSN_Cap->field.PreAuth = (pSecConfig->PreAuth == TRUE) ? 1 : 0;
#endif /* DOT1X_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
	pRSN_Cap->field.MFPC = (pSecConfig->PmfCfg.MFPC) ? 1 : 0;
	pRSN_Cap->field.MFPR = (pSecConfig->PmfCfg.MFPR) ? 1 : 0;
	MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"[PMF]%s: RSNIE Capability MFPC=%d, MFPR=%d\n",
		__func__, pRSN_Cap->field.MFPC, pRSN_Cap->field.MFPR);
#endif /* DOT11W_PMF_SUPPORT */
	pRSN_Cap->word = cpu2le16(pRSN_Cap->word);

	/* Group cipher */
	if (IS_CIPHER_WEP40(pSecConfig->GroupCipher))
		FdRsnInfo.GroupDataCipher = FD_CIPHER_WEP40;
	else if (IS_CIPHER_WEP104(pSecConfig->GroupCipher))
		FdRsnInfo.GroupDataCipher = FD_CIPHER_WEP104;
	else if (IS_CIPHER_TKIP(pSecConfig->GroupCipher))
		FdRsnInfo.GroupDataCipher = FD_CIPHER_TKIP;
	else if (IS_CIPHER_CCMP128(pSecConfig->GroupCipher))
		FdRsnInfo.GroupDataCipher = FD_CIPHER_CCMP128;
	else if (IS_CIPHER_CCMP256(pSecConfig->GroupCipher))
		FdRsnInfo.GroupDataCipher = FD_CIPHER_CCMP256;
	else if (IS_CIPHER_GCMP128(pSecConfig->GroupCipher))
		FdRsnInfo.GroupDataCipher = FD_CIPHER_GCMP128;
	else if (IS_CIPHER_GCMP256(pSecConfig->GroupCipher))
		FdRsnInfo.GroupDataCipher = FD_CIPHER_GCMP256;
	else {
		FdRsnInfo.GroupDataCipher = FD_CIPHER_NO_SELECTED;
	}

	/* Pairwise cipher */
	if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher))
		FdRsnInfo.PairwiseCipher = FD_CIPHER_TKIP;
	else if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher))
		FdRsnInfo.PairwiseCipher = FD_CIPHER_CCMP128;
	else
		FdRsnInfo.PairwiseCipher = FD_CIPHER_NO_SELECTED;

	/* Group Mgmt cipher */
#ifdef DOT11W_PMF_SUPPORT
	FdRsnInfo.GroupMgmtCipher = FD_CIPHER_BIPCMAC128;
#else /* DOT11W_PMF_SUPPORT */
	FdRsnInfo.GroupMgmtCipher = FD_CIPHER_NO_SELECTED;
#endif

	/* AKM */
	if (IS_AKM_FILS_SHA256(pSecConfig->AKMMap) &&
		IS_AKM_FILS_SHA384(pSecConfig->AKMMap))
		FdRsnInfo.AKMSuiteSelector = FD_AKM_FILS_SHA256_SHA384;
	else if (IS_AKM_FILS_SHA256(pSecConfig->AKMMap))
		FdRsnInfo.AKMSuiteSelector = FD_AKM_FILS_SHA256;
	else if (IS_AKM_FILS_SHA384(pSecConfig->AKMMap))
		FdRsnInfo.AKMSuiteSelector = FD_AKM_FILS_SHA384;
	else
		FdRsnInfo.AKMSuiteSelector = FD_AKM_USE_FROM_BCN_PROBERSP;

	MAKE_IE_TO_BUF(buf, &FdRsnInfo, sizeof(FD_RSN_INFO), len);

	return len;
}

static INT build_fils_discovery_info_field(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *buf)
{
	FILS_DIS_FRAME_CTRL FilsDisFrameCtrl;
	UINT64 TimeStamp = 0;
	UINT16 BcnInterval = 0;
	INT len = 0;
	UCHAR vht_bw = wlan_operate_get_vht_bw(wdev);
	UCHAR *IeLenPos = 0, IeLen = 0;
	BSS_STRUCT *pMbss = wdev->func_dev;
	OCE_CTRL *oceCtrl = &wdev->OceCtrl;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

	/* FILS Discovery Frame control subfield */
	NdisZeroMemory(&FilsDisFrameCtrl, sizeof(FILS_DIS_FRAME_CTRL));

	if (oceCtrl->ShortSSIDEnabled) {
		FilsDisFrameCtrl.ShortSsidInd = TRUE;
		FilsDisFrameCtrl.SsidLength = SHORT_SSID_LEN - 1;
	} else {
		FilsDisFrameCtrl.ShortSsidInd = FALSE;
		FilsDisFrameCtrl.SsidLength = pMbss->SsidLen - 1;
	}

	FilsDisFrameCtrl.LengthPresenceInd = TRUE;

	if (vht_bw == VHT_BW_8080)
		FilsDisFrameCtrl.PriChPresenceInd = TRUE;

	if (IS_AKM_WPA_CAPABILITY(pSecConfig->AKMMap))
		FilsDisFrameCtrl.RSNInfoPresenceInd = TRUE;

	if (OCE_GET_CONTROL_FIELD(oceCtrl->OceCapIndication, OCE_NONOCE_PRESENT_MASK, OCE_NONOCE_PRESENT_OFFSET))
		FilsDisFrameCtrl.NonOceAPPresent = TRUE;
	else
		FilsDisFrameCtrl.NonOceAPPresent = FALSE;

	if (OCE_GET_CONTROL_FIELD(oceCtrl->OceCapIndication, OCE_11B_ONLY_PRESENT_MASK, OCE_11B_ONLY_PRESENT_OFFSET))
		FilsDisFrameCtrl.AP11bPresent = TRUE;
	else
		FilsDisFrameCtrl.AP11bPresent = FALSE;

	MAKE_IE_TO_BUF(buf, &FilsDisFrameCtrl, 2, len);

	/* Timestamp */
	MAKE_IE_TO_BUF(buf, &TimeStamp, 8, len);

	/* Beacon Interval */
	BcnInterval = pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)];
	MAKE_IE_TO_BUF(buf, &BcnInterval, 2, len);

	/* SSID / Short SSID */
	if (FilsDisFrameCtrl.ShortSsidInd == TRUE) {
		MAKE_IE_TO_BUF(buf, &pMbss->ShortSSID,
			sizeof(pMbss->ShortSSID), len);
	} else {
		MAKE_IE_TO_BUF(buf, &pMbss->Ssid, pMbss->SsidLen, len);
	}

	/* Length */
	if (FilsDisFrameCtrl.LengthPresenceInd == TRUE) {
		UCHAR Length = 0;

		IeLenPos = buf + len;
		IeLen = len;
		MAKE_IE_TO_BUF(buf, &Length, 1, len);
	}

	/* FD Capability */
	if (FilsDisFrameCtrl.CapPresenceInd == TRUE) {
		FD_CAP_SUB_FIELD FdCap;

		NdisZeroMemory(&FdCap, sizeof(FD_CAP_SUB_FIELD));
		MAKE_IE_TO_BUF(buf, &FdCap, 2, len);
	}

	/* Operating Class */
	/* Primary Channel */
	if (FilsDisFrameCtrl.PriChPresenceInd == TRUE) {
		UCHAR cent_ch_1 = wlan_operate_get_prim_ch(wdev);
		UCHAR opClass = get_regulatory_class(pAd, cent_ch_1,
			wdev->PhyMode, wdev);

		MAKE_IE_TO_BUF(buf, &opClass, 1, len);
		MAKE_IE_TO_BUF(buf, &cent_ch_1, 1, len);
	}

	/* FD RSN Information */
	if (FilsDisFrameCtrl.RSNInfoPresenceInd == TRUE) {
		len += build_fils_discovery_rsn_info(pAd, wdev, (buf+len));
	}

	if (FilsDisFrameCtrl.LengthPresenceInd == TRUE) {
		*IeLenPos = len - IeLen - 1;
	}

	return len;
}

VOID OceSendFilsDiscoveryAction(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	FRAME_ACTION_HDR Frame;
	ULONG FrameLen;
	struct _RTMP_CHIP_CAP *cap;
	struct DOT11_H *pDot11h = NULL;

	pDot11h = wdev->pDot11_H;
	if ((WMODE_CAP_5G(wdev->PhyMode) && RadarChannelCheck(pAd, wdev->channel)) &&
			pDot11h->RDMode != RD_NORMAL_MODE) {
		return ;
	}

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"allocate memory failed\n");
		return;
	}

	ActHeaderInit(pAd, &Frame.Hdr, BROADCAST_ADDR, wdev->if_addr, wdev->bssid);
	Frame.Category = CATEGORY_PUBLIC;
	Frame.Action = ACTION_FILS_DISCOVERY;
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(FRAME_ACTION_HDR), &Frame,
					  END_OF_ARGS);

	/* FILS Discovery Information field*/
	FrameLen += build_fils_discovery_info_field(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));

	/* Reduced Neighbor Report Element */

	/* FILS Indication Elemnet */
	FrameLen += build_fils_indication_element(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));

	if (IS_FD_FRAME_FW_MODE(cap)) {
		HW_SET_FD_FRAME_OFFLOAD(pAd, wdev->wdev_idx, FrameLen, TRUE,
			0 /* todo: timestamp pos */, pOutBuffer);

		hex_dump_with_lvl("FD_FRAME OceSendFilsDiscoveryAction", pOutBuffer, FrameLen, DBG_LVL_INFO);
	} else {
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	}
	MlmeFreeMemory(pOutBuffer);
}

INT oce_build_ies(RTMP_ADAPTER *pAd, struct _build_ie_info *info, BOOLEAN is_oce_sta)
{
	INT len = 0;

	if ((info->frame_subtype == SUBTYPE_BEACON) ||
		(info->frame_subtype == SUBTYPE_PROBE_RSP) ||
		(info->frame_subtype == SUBTYPE_ASSOC_RSP) ||
		(info->frame_subtype == SUBTYPE_REASSOC_RSP)) {
		len += build_fils_indication_element(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	}

	if (info->frame_subtype == SUBTYPE_BEACON)
		len += build_rnr_element(pAd, info->wdev, (UCHAR *)(info->frame_buf + len), info->pos, info->frame_subtype);
	if (is_oce_sta && (info->frame_subtype == SUBTYPE_PROBE_RSP))
		len += build_rnr_element(pAd, info->wdev, (UCHAR *)(info->frame_buf + len), info->pos, info->frame_subtype);
	if ((info->frame_subtype == SUBTYPE_BEACON) ||
		(info->frame_subtype == SUBTYPE_PROBE_RSP)) {
		len += build_esp_element(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	}

	return len;
}


VOID ap_eapol_pairwise_3_send_at_pending_action(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	PRT_802_11_STA_MLME_EVENT mlmeEvent)
{
	UCHAR Header802_3[14];
	STA_TR_ENTRY *tr_entry;
	PHANDSHAKE_PROFILE pHandshake4Way = NULL;
	struct fils_info *filsInfo = &pEntry->filsInfo;
	BOOLEAN Cancelled;

	if (filsInfo->is_pending_encrypt == FALSE)
		return;

	filsInfo->is_pending_encrypt = FALSE;

	if (!pEntry->wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"no wdev\n");
		return;
	}

	pHandshake4Way = &pEntry->SecConfig.Handshake;
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

	/* Make outgoing frame: Authenticator send to Supplicant */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->SAddr, pHandshake4Way->AAddr, EAPOL);

	RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3,
					  mlmeEvent->ie,
					  (mlmeEvent->len > MAX_OPT_IE) ? MAX_OPT_IE : (mlmeEvent->len),
					  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

	RTMPCancelTimer(&pHandshake4Way->MsgRetryTimer, &Cancelled);
	RTMPSetTimer(&pHandshake4Way->MsgRetryTimer, PEER_MSG3_RETRY_EXEC_INTV * 2);
}

VOID ap_eapol_pairwise_2_process_at_pending_action(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	PRT_802_11_STA_MLME_EVENT mlmeEvent)
{
	struct fils_info *filsInfo = &pEntry->filsInfo;
	struct wifi_dev *wdev = NULL;

	if (filsInfo->is_pending_decrypt == FALSE)
		return;

	if (!pEntry->wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"no wdev\n");
		return;
	}

	wdev = pEntry->wdev;
	filsInfo->status = mlmeEvent->status;

	if (filsInfo->pending_decrypt) {
		PMLME_QUEUE_ELEM pElem;
		UINT offset = filsInfo->pending_ie_len - mlmeEvent->len;

		ASSERT(filsInfo->pending_ie_len >= mlmeEvent->len);

		os_alloc_mem_suspend(pAd, (UCHAR **)&pElem, sizeof(MLME_QUEUE_ELEM));

		if (!pElem) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Mem alloc fail!\n");
			return;
		}

		os_zero_mem(pElem, sizeof(MLME_QUEUE_ELEM));

		pElem->Wcid = pEntry->wcid;
		NdisMoveMemory(&pElem->rssi_info, &filsInfo->rssi_info, sizeof(struct raw_rssi_info));
		pElem->MsgLen = filsInfo->pending_ie_len;
		NdisMoveMemory(pElem->Msg, filsInfo->pending_ie, offset);
		NdisMoveMemory(&pElem->Msg[offset], mlmeEvent->ie, mlmeEvent->len);

		pElem->OpMode = OPMODE_AP;
		pElem->wdev = wdev;

		filsInfo->pending_decrypt(pAd, pEntry, &pEntry->SecConfig, pElem);

		os_free_mem(pElem);

	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"no CB pending_decrypt\n");
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 "STA - "MACSTR" FILS decrypt back with %d\n",
		  MAC2STR(pEntry->Addr), filsInfo->status);
}

VOID ap_assoc_extra_ie_at_pending_action(
	RTMP_ADAPTER *pAd,
	RTMP_IOCTL_INPUT_STRUCT *wrq,
	MAC_TABLE_ENTRY *pEntry,
	PRT_802_11_STA_MLME_EVENT mlmeEvent)
{
	struct fils_info *filsInfo = &pEntry->filsInfo;

	if (!pEntry->wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"no wdev\n");
		return;
	}

	mlmeEvent->len = filsInfo->extra_ie_len;
	NdisMoveMemory(mlmeEvent->ie, filsInfo->extra_ie, mlmeEvent->len);

	if (copy_to_user(wrq->u.data.pointer, mlmeEvent, sizeof(RT_802_11_STA_MLME_EVENT))) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");
		return;
	}


}

VOID ap_assoc_reply_at_pending_action(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	PRT_802_11_STA_MLME_EVENT mlmeEvent)
{
	struct fils_info *filsInfo = &pEntry->filsInfo;
	struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
	struct wifi_dev *wdev = NULL;

	if (filsInfo->is_pending_assoc == FALSE)
		return;

	if (!pEntry->wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"no wdev\n");
		return;
	}

	wdev = pEntry->wdev;
	filsInfo->status = mlmeEvent->status;

	MiniportMMRequest(pAd, 0, mlmeEvent->ie, mlmeEvent->len);
	filsInfo->is_pending_assoc = FALSE;

	if (filsInfo->pending_action) {
		MLME_QUEUE_ELEM *elem;

		os_alloc_mem(pAd, (UCHAR **)&elem, sizeof(MLME_QUEUE_ELEM));
		if (elem == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "os_alloc_mem fail.\n");
			return;
			}
		else
		os_zero_mem(elem, sizeof(MLME_QUEUE_ELEM));

		filsInfo->is_post_assoc = TRUE;
		elem->Wcid = pEntry->wcid;
		NdisMoveMemory(&elem->rssi_info, &filsInfo->rssi_info, sizeof(struct raw_rssi_info));
		elem->MsgLen = filsInfo->pending_ie_len;
		NdisMoveMemory(elem->Msg, filsInfo->pending_ie, elem->MsgLen);
		elem->OpMode = OPMODE_AP;
		elem->wdev = wdev;

		filsInfo->pending_action(pAd, elem);
		os_free_mem(elem);
	}

	if (filsInfo->pending_ie) {
		os_free_mem(filsInfo->pending_ie);
		filsInfo->pending_ie_len = 0;
		filsInfo->pending_ie = NULL;
	}

	if (filsInfo->extra_ie) {
		os_free_mem(filsInfo->extra_ie);
		filsInfo->extra_ie_len = 0;
		filsInfo->extra_ie = NULL;
	}

	if (filsInfo->status == MLME_SUCCESS) {
		struct _ASIC_SEC_INFO *info = NULL;
		struct _STA_REC_CTRL_T *org = NULL;

		os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));

		if (info) {
			STA_TR_ENTRY *tr_entry = NULL;

			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
			org = &tr_entry->StaRec;

			os_zero_mem(info, sizeof(ASIC_SEC_INFO));
			/* NdisCopyMemory(pSecConfig->PTK, pEntry->FT_PTK, LEN_MAX_PTK); */
			info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
			info->Direction = SEC_ASIC_KEY_BOTH;
			info->Wcid = pEntry->wcid;
			info->BssIndex = pEntry->func_tb_idx;
			SET_CIPHER_CCMP128(pSecConfig->PairwiseCipher);
			info->Cipher = pSecConfig->PairwiseCipher;
			info->KeyIdx = pSecConfig->PairwiseKeyId;
			os_move_mem(&info->PeerAddr[0],
						pEntry->Addr, MAC_ADDR_LEN);

			info->Key.KeyLen = 16;
			os_move_mem(info->Key.Key, pSecConfig->PTK,
						info->Key.KeyLen);

			/* Update status and set Port as Secured */
			pSecConfig->Handshake.WpaState = AS_PTKINITDONE;
			pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;
			pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

			WifiSysUpdatePortSecur(pAd, pEntry, info);
			os_free_mem(info);

			hex_dump("TK IN DRIVER", pSecConfig->PTK,
						16);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 "STA - "MACSTR" FILS assoc back with %d len(%d)\n",
		  MAC2STR(pEntry->Addr), filsInfo->status, mlmeEvent->len);

}

VOID ap_auth_reply_at_pending_action(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	PRT_802_11_STA_MLME_EVENT mlmeEvent)
{
	struct fils_info *filsInfo = &pEntry->filsInfo;
	HEADER_802_11     AuthHdr;
	ULONG             FrameLen = 0;
	PUCHAR            pOutBuffer = NULL;
	NDIS_STATUS       NStatus;
	struct wifi_dev *wdev = NULL;

	if (filsInfo->is_pending_auth == FALSE)
		return;

	if (!pEntry->wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"no wdev\n");
		return;
	}

	wdev = pEntry->wdev;

	hex_dump("FILS: ANonce", mlmeEvent->fils_anonce, FILS_NONCE_LEN);
	hex_dump("FILS: SNonce", mlmeEvent->fils_snonce, FILS_NONCE_LEN);
	hex_dump("FILS: KEK", mlmeEvent->fils_kek,
		(mlmeEvent->fils_kek_len > WPA_KEK_MAX_LEN) ? WPA_KEK_MAX_LEN : mlmeEvent->fils_kek_len);

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	if (mlmeEvent->status == MLME_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"AUTH_RSP - Send AUTH response (SUCCESS)...\n");

		pEntry->AuthState = AS_AUTH_OPEN;
		/*According to specific, if it already in SST_ASSOC, it can not go back */
		if (pEntry->Sst != SST_ASSOC)
			pEntry->Sst = SST_AUTH;

	} else {
		/* For MAC wireless client(Macintosh), need to send AUTH_RSP with Status Code (fail reason code) to reject it. */
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"AUTH_RSP - Peer AUTH fail (Status = %d)...\n", mlmeEvent->status);
	}

	MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, pEntry->Addr,
					 wdev->if_addr, wdev->bssid);
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(HEADER_802_11),	&AuthHdr,
					  2,						&mlmeEvent->auth_algo,
					  2,						&mlmeEvent->seq,
					  2,						&mlmeEvent->status,
					  END_OF_ARGS);

	if (mlmeEvent->len > 0) {
		ULONG TmpLen;

		MakeOutgoingFrame(pOutBuffer  + FrameLen,				&TmpLen,
						  mlmeEvent->len,			&mlmeEvent->ie,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);

	filsInfo->is_pending_auth = FALSE;
}

VOID RTMPIoctlStaMlmeEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	PRT_802_11_STA_MLME_EVENT pMlmeEvent;
	MAC_TABLE_ENTRY *pEntry = NULL;

	os_alloc_mem_suspend(pAd, (UCHAR **)&pMlmeEvent, sizeof(RT_802_11_STA_MLME_EVENT));

	if (!pMlmeEvent) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Mem alloc fail!\n");
		return;
	}

	os_zero_mem(pMlmeEvent, sizeof(RT_802_11_STA_MLME_EVENT));

	if (wrq->u.data.length != sizeof(RT_802_11_STA_MLME_EVENT)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length is mis-match\n");
		os_free_mem(pMlmeEvent);
		return;
	}

	if (copy_from_user(pMlmeEvent, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");
		os_free_mem(pMlmeEvent);
		return;
	}

	pEntry = MacTableLookup(pAd, pMlmeEvent->addr);

	if (pEntry != NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "%s:("MACSTR", action: %d, algo: %d)\n",
				  __func__, MAC2STR(pMlmeEvent->addr),
				  pMlmeEvent->mgmt_subtype, pMlmeEvent->auth_algo);

		if (pMlmeEvent->mgmt_subtype == SUBTYPE_AUTH) {
			ap_auth_reply_at_pending_action(pAd, pEntry, pMlmeEvent);
		} else if ((pMlmeEvent->mgmt_subtype == SUBTYPE_ASSOC_REQ) ||
			     (pMlmeEvent->mgmt_subtype == SUBTYPE_REASSOC_REQ)) {
			ap_assoc_reply_at_pending_action(pAd, pEntry, pMlmeEvent);
		} else if ((pMlmeEvent->mgmt_subtype == SUBTYPE_ACTION) &&
			     (pMlmeEvent->auth_algo == AUTH_MODE_OPEN)) {
			ap_eapol_pairwise_2_process_at_pending_action(pAd, pEntry, pMlmeEvent);
		} else if ((pMlmeEvent->mgmt_subtype == SUBTYPE_ACTION) &&
			     (pMlmeEvent->auth_algo == AUTH_MODE_KEY)) {
			ap_eapol_pairwise_3_send_at_pending_action(pAd, pEntry, pMlmeEvent);
		} else if ((pMlmeEvent->mgmt_subtype == SUBTYPE_ASSOC_RSP) ||
			     (pMlmeEvent->mgmt_subtype == SUBTYPE_REASSOC_RSP)) {
			ap_assoc_extra_ie_at_pending_action(pAd, wrq, pEntry, pMlmeEvent);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "("MACSTR", Not Found)\n",
				  MAC2STR(pMlmeEvent->addr));
	}

	if (pMlmeEvent) {
		os_free_mem(pMlmeEvent);
		return;
	}

}

VOID RTMPIoctlRsneSyncEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	PRT_802_11_SEC_INFO_SYNC_EVENT pSyncEvent;
	struct wifi_dev *wdev = NULL;
	OCE_CTRL *oceCtrl = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	UCHAR rsne_idx = 0;

	os_alloc_mem_suspend(pAd, (UCHAR **)&pSyncEvent, sizeof(RT_802_11_SEC_INFO_SYNC_EVENT));

	if (!pSyncEvent) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Mem alloc fail!\n");
		return;
	}

	os_zero_mem(pSyncEvent, sizeof(RT_802_11_SEC_INFO_SYNC_EVENT));

	if (wrq->u.data.length != sizeof(RT_802_11_SEC_INFO_SYNC_EVENT)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length is mis-match\n");
		os_free_mem(pSyncEvent);
		return;
	}

	if (copy_from_user(pSyncEvent, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_from_user() fail\n");
		os_free_mem(pSyncEvent);
		return;
	}

	if (pSyncEvent->apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pSyncEvent->apidx:%d error\n", pSyncEvent->apidx);
		os_free_mem(pSyncEvent);
		return;
	}

	wdev = &pAd->ApCfg.MBSSID[pSyncEvent->apidx].wdev;
	pSecConfig = &wdev->SecConfig;
	oceCtrl = &wdev->OceCtrl;

	pSyncEvent->wpa_key_mgmt = pSecConfig->AKMMap;
	pSyncEvent->CapabilityInfo = pAd->ApCfg.MBSSID[pSyncEvent->apidx].CapabilityInfo;

	pSyncEvent->GN = pSecConfig->GroupKeyId;
	pSyncEvent->GTK_len = sec_get_cipher_key_len(pSecConfig->GroupCipher);
	NdisMoveMemory(pSyncEvent->GTK, pSecConfig->GTK, pSyncEvent->GTK_len);

	pSyncEvent->IGN = pSecConfig->PmfCfg.IGTK_KeyIdx;
	pSyncEvent->IGTK_len = LEN_TK;
	NdisMoveMemory(pSyncEvent->IGTK, pSecConfig->PmfCfg.IGTK[pSyncEvent->IGN-4], pSyncEvent->IGTK_len);

	pSyncEvent->FilsCacheId = oceCtrl->FilsCacheId;
	pSyncEvent->FilsDhcpServerIp = oceCtrl->FilsDhcpServerIp;

	for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
		if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
			continue;
		if (pSyncEvent->rsne_len + 3 < MAX_OPT_IE) {
			NdisMoveMemory(&pSyncEvent->rsne[pSyncEvent->rsne_len], pSecConfig->RSNE_EID[rsne_idx], 1);
			NdisMoveMemory(&pSyncEvent->rsne[pSyncEvent->rsne_len + 1], &pSecConfig->RSNE_Len[rsne_idx], 1);
			NdisMoveMemory(&pSyncEvent->rsne[pSyncEvent->rsne_len + 2], pSecConfig->RSNE_Content[rsne_idx],
				pSecConfig->RSNE_Len[rsne_idx]);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "syncEvent.rsne_len:%d\n", pSyncEvent->rsne_len);
		}
		pSyncEvent->rsne_len += pSecConfig->RSNE_Len[rsne_idx] + 2;
	}

	if (pSyncEvent->rsne_len > MAX_OPT_IE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "syncEvent.rsne_len > MAX_OPT_IE) fail\n");
		os_free_mem(pSyncEvent);
		return;
	}

	if (copy_to_user(wrq->u.data.pointer, pSyncEvent, sizeof(RT_802_11_SEC_INFO_SYNC_EVENT))) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");
		os_free_mem(pSyncEvent);
		return;
	}

	hex_dump("RSNE INFO", pSyncEvent->rsne, pSyncEvent->rsne_len);

	if (pSyncEvent) {
		os_free_mem(pSyncEvent);
		return;
	}
}
VOID RTMPIoctlKeyEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	RT_802_11_KEY_EVENT *KeyEvent = NULL;
	PNDIS_FILS_802_11_KEY keyInfo = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;

	os_alloc_mem(pAd, (UCHAR **)&KeyEvent, sizeof(RT_802_11_KEY_EVENT));
	if (KeyEvent == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "os_alloc_mem fail.\n");
		return;
		}
	else
		os_zero_mem(KeyEvent, sizeof(RT_802_11_KEY_EVENT));

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s: wrq->u.data.length = %d, sizeof(RT_802_11_KEY_EVENT) = %d\n"
		, __func__, wrq->u.data.length, (UINT)(sizeof(RT_802_11_KEY_EVENT)));

	if (wrq->u.data.length != sizeof(RT_802_11_KEY_EVENT)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length is mis-match\n");
		os_free_mem(KeyEvent);
		return;
	}

	if (copy_from_user(KeyEvent, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_from_user() fail\n");
		os_free_mem(KeyEvent);
		return;
	}

	keyInfo = &KeyEvent->keyInfo;
	NdisMoveMemory(keyInfo, &KeyEvent->keyInfo, sizeof(NDIS_FILS_802_11_KEY));

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s:("MACSTR" action: %d)\n",
			  __func__, MAC2STR(keyInfo->addr), KeyEvent->action);

	pEntry = MacTableLookup(pAd, keyInfo->addr);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Entry not found\n");
		os_free_mem(KeyEvent);
		return;
	}


	if (KeyEvent->action == FILS_KEY_INSTALL_PTK) {
		struct _ASIC_SEC_INFO *info = NULL;

		os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));

		if (info) {
			struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
			STA_TR_ENTRY *tr_entry = NULL;
			os_zero_mem(info, sizeof(ASIC_SEC_INFO));

			tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];

			if (keyInfo->KeyLength <= LEN_PSK)
				os_move_mem(pSecConfig->PTK, &keyInfo->KeyMaterial,
						keyInfo->KeyLength);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"keylen:%d\n", keyInfo->KeyLength);

			/* Update status and set Port as Secured */
			pSecConfig->Handshake.WpaState = AS_PTKINITDONE;
			pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;
			pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

		} else {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL,
					 DBG_LVL_ERROR, "struct alloc fail\n");
		}
		os_free_mem(info);
	} else if (KeyEvent->action == FILS_KEY_GET_TSC) {
		if (!pEntry->wdev) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev not found\n");
			os_free_mem(KeyEvent);
			return;
		}

		if ((keyInfo->KeyIndex == 4) || (keyInfo->KeyIndex == 5))
			KeyEvent->keytsc = 0; /* TODO */
		else
			AsicGetTxTsc(pAd, pEntry->wdev, TSC_TYPE_GTK_PN, (UCHAR *)&(KeyEvent->keytsc));

		if (copy_to_user(wrq->u.data.pointer, KeyEvent, sizeof(RT_802_11_KEY_EVENT))) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user fail\n");
			os_free_mem(KeyEvent);
		return;
		}
	}
	os_free_mem(KeyEvent);
}

VOID RTMPIoctlPmkCacheEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RT_802_11_PMK_CACHE_SYNC_EVENT PmkCacheEvent;
	MAC_TABLE_ENTRY *pEntry = NULL;
	PAP_BSSID_INFO pkeyInfo = NULL;

	if (wrq->u.data.length != sizeof(RT_802_11_PMK_CACHE_SYNC_EVENT)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length is mis-match\n");
		return;
	}

	if (copy_from_user(&PmkCacheEvent, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_from_user() fail\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "("MACSTR" action: %d)\n",
			  MAC2STR(PmkCacheEvent.addr), PmkCacheEvent.res);

	pEntry = MacTableLookup(pAd, PmkCacheEvent.addr);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Entry not found\n");
		return;
	}

	if (PmkCacheEvent.res == PMK_CACHE_QUERY) {
		INT		CacheIdx;
		/* Search PMK Cache */
		CacheIdx = RTMPSearchPMKIDCacheByPmkId(&pAd->ApCfg.PMKIDCache,
			pEntry->func_tb_idx, pEntry->Addr, PmkCacheEvent.pmkid);

		if (CacheIdx == INVALID_PMKID_IDX) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR,
				"The PMK Cache doesn't exist for "MACSTR"\n",
					 MAC2STR(pEntry->Addr));
			PmkCacheEvent.res = PMK_CACHE_STATUS_FAIL;
			goto reply;
		}

		pkeyInfo = &pAd->ApCfg.PMKIDCache.BSSIDInfo[CacheIdx];

		PmkCacheEvent.res = PMK_CACHE_STATUS_OK;
		os_move_mem(PmkCacheEvent.pmkid, pkeyInfo->PMKID, LEN_PMKID);
		PmkCacheEvent.pmk_len = LEN_MAX_PMK;
		os_move_mem(PmkCacheEvent.pmk, pkeyInfo->PMK, LEN_MAX_PMK);
	}

reply:
	if (copy_to_user(wrq->u.data.pointer, &PmkCacheEvent, sizeof(RT_802_11_PMK_CACHE_SYNC_EVENT))) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");
		return;
	}
}


