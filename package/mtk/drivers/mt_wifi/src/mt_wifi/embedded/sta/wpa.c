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
	Who			When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"


/*
 * inc_byte_array - Increment arbitrary length byte array by one
 * @counter: Pointer to byte array
 * @len: Length of the counter in bytes
 *
 * This function increments the last byte of the counter by one and continues
 * rolling over to more significant bytes if the byte was incremented from
 * 0xff to 0x00.
 */
void inc_byte_array(UCHAR *counter, int len)
{
	int pos = len - 1;

	while (pos >= 0) {
		counter[pos]++;

		if (counter[pos] != 0)
			break;

		pos--;
	}
}


/*
 *	========================================================================
 *
 *	Routine Description:
 *		Process MIC error indication and record MIC error timer.
 *
 *	Arguments:
 *		pAd	Pointer to our adapter
 *		pWpaKey		Pointer to the WPA key structure
 *
 *	Return Value:
 *		None
 *
 *	IRQL = DISPATCH_LEVEL
 *
 *	Note:
 *
 *	========================================================================
 */
VOID RTMPReportMicError(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg, PCIPHER_KEY pWpaKey)
{
	ULONG Now;
	UCHAR unicastKey = (pWpaKey->Type == PAIRWISE_KEY ? 1 : 0);
	struct wifi_dev *wdev = &pStaCfg->wdev;
	MAC_TABLE_ENTRY *pEntry = NULL;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	/* Record Last MIC error time and count */
	NdisGetSystemUpTime(&Now);

	if (pStaCfg->MicErrCnt == 0) {
		pStaCfg->MicErrCnt++;
		pStaCfg->LastMicErrorTime = Now;
		NdisZeroMemory(pStaCfg->ReplayCounter, 8);
	} else if (pStaCfg->MicErrCnt == 1) {
		if ((pStaCfg->LastMicErrorTime + (60 * OS_HZ)) < Now) {
			/* Update Last MIC error time, this did not violate two MIC errors within 60 seconds */
			pStaCfg->LastMicErrorTime = Now;
		} else {
			RTMPSendWirelessEvent(pAd, IW_COUNTER_MEASURES_EVENT_FLAG, pEntry->Addr, BSS0, 0);
			pStaCfg->LastMicErrorTime = Now;
			/* Violate MIC error counts, MIC countermeasures kicks in */
			pStaCfg->MicErrCnt++;
			/*
			 * We shall block all reception
			 * We shall clean all Tx ring and disassoicate from AP after next EAPOL frame
			 *
			 * No necessary to clean all Tx ring, on HardTransmit will stop sending non-802.1X EAPOL packets
			 * if pStaCfg->MicErrCnt greater than 2.
			 */
		}
	} else {
		/* MIC error count >= 2 */
		/* This should not happen */
		;
	}

	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						OID_802_11_MIC_FAILURE_REPORT_FRAME,
						1,
						&unicastKey, 0, wdev);

	if (pStaCfg->MicErrCnt == 2)
		RTMPSetTimer(&pStaCfg->MlmeAux.WpaDisassocAndBlockAssocTimer, 100);

}


VOID WpaMicFailureReportFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PUCHAR              pOutBuffer = NULL;
	UCHAR               Header802_3[14];
	ULONG               FrameLen = 0;
	UCHAR				*mpool;
	PEAPOL_PACKET       pPacket;
	UCHAR               Mic[16];
	BOOLEAN             bUnicast;
	struct wifi_dev     *wdev = Elem->wdev;
	MAC_TABLE_ENTRY     *pEntry = NULL;
	PSTA_ADMIN_CONFIG   pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	UINT datalen;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WpaMicFailureReportFrame ----->\n");
	bUnicast = (Elem->Msg[0] == 1 ? TRUE : FALSE);
	pAd->Sequence = ((pAd->Sequence) + 1) & (MAX_SEQ_NUMBER);
	/* init 802.3 header and Fill Packet */
	MAKE_802_3_HEADER(Header802_3, pStaCfg->Bssid, wdev->if_addr, EAPOL);
	/* Allocate memory for output */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "!!!no memory!!!\n");
		return;
	}

	pPacket = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pPacket, TX_EAPOL_BUFFER);
	pPacket->ProVer	= EAPOL_VER;
	pPacket->ProType	= EAPOLKey;
	pPacket->KeyDesc.Type = WPA1_KEY_DESC;
	/* Request field presented */
	pPacket->KeyDesc.KeyInfo.Request = 1;

	if (IS_CIPHER_CCMP128(pStaCfg->wdev.SecConfig.PairwiseCipher))
		pPacket->KeyDesc.KeyInfo.KeyDescVer = 2;
	else	  /* TKIP */
		pPacket->KeyDesc.KeyInfo.KeyDescVer = 1;

	pPacket->KeyDesc.KeyInfo.KeyType = (bUnicast ? PAIRWISEKEY : GROUPKEY);
	/* KeyMic field presented */
	pPacket->KeyDesc.KeyInfo.KeyMic  = 1;
	/* Error field presented */
	pPacket->KeyDesc.KeyInfo.Error  = 1;
	/* Update packet length after decide Key data payload */
	SET_UINT16_TO_ARRARY(pPacket->Body_Len, MIN_LEN_OF_EAPOL_KEY_MSG)
	/* Key Replay Count */
	NdisMoveMemory(pPacket->KeyDesc.ReplayCounter, pStaCfg->ReplayCounter, LEN_KEY_DESC_REPLAY);
	inc_byte_array(pStaCfg->ReplayCounter, 8);
	/* Convert to little-endian format. */
	*((USHORT *)&pPacket->KeyDesc.KeyInfo) = cpu2le16(*((USHORT *)&pPacket->KeyDesc.KeyInfo));
	MlmeAllocateMemory(pAd, (PUCHAR *)&pOutBuffer);  /* allocate memory */

	if (pOutBuffer == NULL) {
		os_free_mem(mpool);
		return;
	}

	/*
	 *   Prepare EAPOL frame for MIC calculation
	 *   Be careful, only EAPOL frame is counted for MIC calculation
	 */
	MakeOutgoingFrame(pOutBuffer,               &FrameLen,
					  CONV_ARRARY_TO_UINT16(pPacket->Body_Len) + 4,   pPacket,
					  END_OF_ARGS);
	/* Prepare and Fill MIC value */
	NdisZeroMemory(Mic, sizeof(Mic));

	if (IS_CIPHER_CCMP128(pStaCfg->wdev.SecConfig.PairwiseCipher)) {
		/* AES */
		UCHAR digest[20] = {0};

		RT_HMAC_SHA1(pStaCfg->PTK, LEN_PTK_KCK, pOutBuffer, FrameLen, digest, SHA1_DIGEST_SIZE);
		NdisMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
	} else {
		/* TKIP */
		RT_HMAC_MD5(pStaCfg->PTK, LEN_PTK_KCK, pOutBuffer, FrameLen, Mic, MD5_DIGEST_SIZE);
	}

	NdisMoveMemory(pPacket->KeyDesc.KeyMicAndData, Mic, LEN_KEY_DESC_MIC);
	/* copy frame to Tx ring and send MIC failure report frame to authenticator */
	datalen = CONV_ARRARY_TO_UINT16(pPacket->Body_Len);
	if (datalen < 65535) {
		RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, LENGTH_802_3,
					  (PUCHAR)pPacket,
					  datalen + 4, FALSE);
	}
	MlmeFreeMemory((PUCHAR)pOutBuffer);
	os_free_mem(mpool);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WpaMicFailureReportFrame <-----\n");
}


VOID WpaDisassocApAndBlockAssoc(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	pStaCfg->bBlockAssoc = TRUE;
	pStaCfg->MicErrCnt = 0;

	cntl_disconnect_request(wdev, CNTL_DISASSOC, pStaCfg->Bssid, REASON_MIC_FAILURE);
}


#ifdef DOT11R_FT_SUPPORT
VOID WpaStaPairwiseKeySetting(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASIC_SEC_INFO Info = {0};

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	NdisMoveMemory(pStaCfg->PTK, pEntry->SecConfig.PTK, LEN_PTK);
	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = pEntry->wcid;
	Info.BssIndex = BSS0;
	Info.Cipher = pStaCfg->PairwiseCipher;
	Info.KeyIdx = 0;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
	os_move_mem(&Info.Key, &pEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], (LEN_TK + LEN_TK2));
	WPAInstallKey(pAd, &Info, FALSE, TRUE);
    /*RTMP_SET_PORT_SECURED(pAd, wdev);*/
    RTEnqueueInternalCmd(pAd, CMDTHREAD_SET_PORT_SECURED, wdev, sizeof(struct wifi_dev));
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s : AID(%d) port secured\n", __func__, pEntry->Aid);
}


VOID WpaStaGroupKeySetting(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASIC_SEC_INFO Info = {0};
	USHORT Wcid;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	if (wdev->SecConfig.GroupKeyId > 3) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "!!! DefaultKeyId > 3 !!!\n");
		return;
	}

	/* Get a specific WCID to record this MBSS key attribute */
	GET_GroupKey_WCID(wdev, Wcid);
	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
	Info.Direction = SEC_ASIC_KEY_RX;
	Info.Wcid = Wcid;
	Info.BssIndex = BSS0;
	Info.Cipher = wdev->SecConfig.GroupCipher;
	Info.KeyIdx = wdev->SecConfig.GroupKeyId;
}
#endif /* DOT11R_FT_SUPPORT */


/*
 *	========================================================================
 *
 *	Routine Description:
 *		Send EAPoL-Start packet to AP.
 *
 *	Arguments:
 *		pAd         - NIC Adapter pointer
 *
 *	Return Value:
 *		None
 *
 *	IRQL = DISPATCH_LEVEL
 *
 *	Note:
 *		Actions after link up
 *		1. Change the correct parameters
 *		2. Send EAPOL - START
 *
 *	========================================================================
 */
VOID WpaSendEapolStart(RTMP_ADAPTER *pAd, UCHAR *pBssid, struct wifi_dev *wdev)
{
	IEEE8021X_FRAME Packet;
	UCHAR Header802_3[14];
	MAC_TABLE_ENTRY *pEntry = MacTableLookup2(pAd, pBssid, wdev);

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-----> WpaSendEapolStart\n");
	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
	MAKE_802_3_HEADER(Header802_3, pBssid, &wdev->if_addr[0], EAPOL);
	/* Zero message 2 body */
	NdisZeroMemory(&Packet, sizeof(Packet));
	Packet.Version = EAPOL_VER;
	Packet.Type    = EAPOLStart;
	Packet.Length  = cpu2be16(0);
	/* Copy frame to Tx ring */
	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pEntry,
					  Header802_3, LENGTH_802_3, (PUCHAR)&Packet, 4, TRUE);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<----- WpaSendEapolStart\n");
}

