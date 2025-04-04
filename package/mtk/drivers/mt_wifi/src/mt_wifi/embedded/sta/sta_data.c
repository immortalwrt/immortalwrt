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

*/

#include "rt_config.h"

INT sta_tx_pkt_allowed(
	RTMP_ADAPTER * pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pPacket)
{
	BOOLEAN allowToSend = FALSE;
	MAC_TABLE_ENTRY *pEntry;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
	UINT16 wcid = WCID_INVALID;
	UCHAR frag_nums;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	pSrcBufVA = RTMP_GET_PKT_SRC_VA(pPacket);
	SrcBufLen = RTMP_GET_PKT_LEN(pPacket);

	if ((!pSrcBufVA) || (SrcBufLen <= 14)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pkt error(%p, %d)\n",
				 pSrcBufVA, SrcBufLen);
		return FALSE;
	}

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return FALSE;

	/* TODO: shiang-usw, fix me about this!! */
	if (!(pStaCfg->BssType == BSS_ADHOC || pStaCfg->BssType == BSS_INFRA))
		return FALSE;

	/* Drop send request since we are in monitor mode */
	/* TODO: shiang-usw, integrate this check to wdev->allow_data_tx = FALSE! */
	if (MONITOR_ON(pAd))
		return FALSE;

	if ((pStaCfg->BssType == BSS_ADHOC) && ADHOC_ON(pAd)) {
		if (MAC_ADDR_IS_GROUP(pSrcBufVA)) {
			RTMP_SET_PACKET_WCID(pPacket, wdev->tr_tb_idx);
			wcid = wdev->tr_tb_idx;
			allowToSend = TRUE;
		} else {
			pEntry = MacTableLookup2(pAd, pSrcBufVA, wdev);

			if (pEntry) {
				allowToSend = TRUE;
				wcid = pEntry->wcid;
			}
		}

		RTMP_SET_PACKET_WCID(pPacket, wcid);
		return allowToSend;
	}

	if ((pStaCfg->BssType == BSS_INFRA) && INFRA_ON(pStaCfg)) {
		pEntry = GetAssociatedAPByWdev(pAd, wdev);

		if (pEntry)
			wcid = pEntry->wcid;
		else
			return FALSE;

		if (0
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			|| IS_TDLS_SUPPORT(pAd)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		   ) {
			pEntry = MacTableLookup2(pAd, pSrcBufVA, wdev);

			if (pEntry && (IS_ENTRY_DLS(pEntry)
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
						   || IS_ENTRY_TDLS(pEntry)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
						  ))
				wcid = pEntry->wcid;
		}
	}

	frag_nums = get_frag_num(pAd, wdev, pPacket);
	RTMP_SET_PACKET_FRAGMENTS(pPacket, frag_nums);

	if (!IS_TR_WCID_VALID(pAd, wcid))
		return FALSE;

	if (!RTMPCheckEtherType(pAd, pPacket, &tr_ctl->tr_entry[wcid], wdev))
		return FALSE;

#ifdef WSC_STA_SUPPORT

	if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
		(wdev->WscControl.bWscTrigger) &&
		(!RTMP_GET_PACKET_EAPOL(pPacket)))
		return FALSE;

#endif /* WSC_STA_SUPPORT */

	/* WPA 802.1x secured port control - drop all non-802.1x frame before port secured */
	if (((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
		 || (pStaCfg->MicErrCnt >= 2))
		&& (!RTMP_GET_PACKET_EAPOL(pPacket))
	   )
		return FALSE;

	RTMP_SET_PACKET_WCID(pPacket, wcid);
	return TRUE;
}

UCHAR sta_mlme_search_wcid(RTMP_ADAPTER *pAd, UCHAR *addr1, UCHAR *addr2, PNDIS_PACKET pkt)
{
	MAC_TABLE_ENTRY *mac_entry = MacTableLookup(pAd, addr1);

	/* TODO: for multi-sta or dbdc, need to correct to right wcid */
	if (mac_entry)
		return mac_entry->wcid;
	else
		return 0;
}

INT sta_send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue)
{
	HEADER_802_11 *pHeader_802_11;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UCHAR *pSrcBufVA;
	INT ret;
	struct qm_ops *ops = pAd->qm_ops;

	RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
	RTMP_SET_PACKET_MGMT_PKT(pkt, 1);
	pSrcBufVA = RTMP_GET_PKT_SRC_VA(pkt);

	if (pSrcBufVA == NULL) {
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	pHeader_802_11 = (HEADER_802_11 *)(pSrcBufVA + tx_hw_hdr_len);
	RTMP_SET_PACKET_WCID(pkt, sta_mlme_search_wcid(pAd, pHeader_802_11->Addr1, pHeader_802_11->Addr2, pkt));

	if (in_altx_filter_list(pHeader_802_11)
		&& (pHeader_802_11->FC.Type == FC_TYPE_MGMT)
		) {
		if (!(RTMP_GET_PACKET_TXTYPE(pkt) == TX_ATE_FRAME))
			RTMP_SET_PACKET_TYPE(pkt, TX_ALTX);
	}

	if  (is_data_queue) {
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
		UAPSD_MR_QOS_NULL_HANDLE(pAd, pHeader_802_11, pkt);
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pkt, 1);
	}

	ret = ops->enq_mgmtq_pkt(pAd, wdev, pkt);
	return ret;
}

INT sta_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	UCHAR user_prio = 0;
	UCHAR q_idx;
	STA_TR_ENTRY *tr_entry = NULL;
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	UINT16 wcid = RTMP_GET_PACKET_WCID(pkt);;

	tr_entry = &pAd->tr_ctl.tr_entry[wcid];
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	user_prio = RTMP_GET_PACKET_UP(pkt);
	q_idx = RTMP_GET_PACKET_QUEIDX(pkt);
	wdev_ops->detect_wmm_traffic(pAd, wdev, user_prio, FLG_IS_OUTPUT);
	RTMP_SET_PACKET_UP(pkt, user_prio);
	qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
	ba_ori_session_start(pAd, tr_entry, user_prio);
	return NDIS_STATUS_SUCCESS;
}

/*
 *--------------------------------------------------------
 *FIND ENCRYPT KEY AND DECIDE CIPHER ALGORITHM
 *	Find the WPA key, either Group or Pairwise Key
 *	LEAP + TKIP also use WPA key.
 *--------------------------------------------------------
 *	Decide WEP bit and cipher suite to be used.
 *	Same cipher suite should be used for whole fragment burst
 *	In Cisco CCX 2.0 Leap Authentication
 *	WepStatus is Ndis802_11WEPEnabled but the key will use PairwiseKey
 *	Instead of the SharedKey, SharedKey Length may be Zero.
 */
VOID sta_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	pTxBlk->CipherAlg = CIPHER_NONE;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)) {
		pTxBlk->pKey =  NULL;
	} else if (pTxBlk->TxFrameType == TX_MCAST_FRAME) {
		pTxBlk->CipherAlg = wdev->SecConfig.GroupCipher;
		pTxBlk->KeyIdx =  wdev->SecConfig.GroupKeyId;
		if (IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) {
			pTxBlk->pKey = (PCIPHER_KEY)&(wdev->SecConfig.WepKey[pTxBlk->KeyIdx]);
		} else {
			pTxBlk->pKey = NULL;
			ASSERT(pTxBlk->pKey);
		}
	} else {
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT)) {
			TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);
			pTxBlk->KeyIdx =  pMacEntry->SecConfig.PairwiseKeyId;

			if (IS_CIPHER_WEP40(pMacEntry->SecConfig.PairwiseCipher)) {
				pTxBlk->pKey = (PCIPHER_KEY)&(pMacEntry->SecConfig.WepKey[pTxBlk->KeyIdx]);
				pTxBlk->CipherAlg = CIPHER_WEP64;
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
			} else if (IS_CIPHER_WEP104(pMacEntry->SecConfig.PairwiseCipher)) {
				pTxBlk->pKey = (PCIPHER_KEY)&(pMacEntry->SecConfig.WepKey[pTxBlk->KeyIdx]);
				pTxBlk->CipherAlg = CIPHER_WEP128;
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
			} else if (IS_AKM_SHA384(pMacEntry->SecConfig.AKMMap)) {
				pTxBlk->pKey = &pMacEntry->SecConfig.SwPairwiseKey;
			} else {
				/* TBD : S/W no support apcli, and didn't assign below yet */
				/* CIPHER_AES , CIPHER_TKIP and others TBD */
				pTxBlk->pKey = &pMacEntry->SecConfig.SwPairwiseKey;
				pTxBlk->CipherAlg = pMacEntry->SecConfig.SwPairwiseKey.CipherAlg;

				if (pTxBlk->CipherAlg == CIPHER_AES)
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
				else if (pTxBlk->CipherAlg == CIPHER_TKIP)
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
			}
		}
	}
#else /* SOFT_ENCRYPT || SW_CONNECT_SUPPORT */
	pTxBlk->CipherAlg = CIPHER_NONE;
#endif /* !SOFT_ENCRYPT &&  !SW_CONNECT_SUPPORT */

}

#ifdef DOT11_N_SUPPORT
VOID sta_build_cache_802_11_header(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR *buf)
{
	STA_TR_ENTRY *tr_entry;
	HEADER_802_11 *wifi_hdr;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	wifi_hdr = (HEADER_802_11 *)buf;
	tr_entry = pTxBlk->tr_entry;
	/*
	 *	Update the cached 802.11 HEADER
	 */
	/* normal wlan header size : 24 octets */
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	/* More Bit */
	wifi_hdr->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);
	/* Sequence */
	wifi_hdr->Sequence = tr_entry->TxSeq[pTxBlk->UserPriority];
	tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
	{
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bTdlsEntry)) {
			COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
			COPY_MAC_ADDR(wifi_hdr->Addr3, pAd->CommonCfg.Bssid);
			wifi_hdr->FC.ToDs = 0;
		} else
#endif /* DOT11Z_TDLS_SUPPORT */
			if (ADHOC_ON(pAd))
				COPY_MAC_ADDR(wifi_hdr->Addr3, pAd->StaCfg[0].Bssid);
			else {
				COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);
#ifdef CLIENT_WDS

				if (!MAC_ADDR_EQUAL((pTxBlk->pSrcBufHeader + MAC_ADDR_LEN), pAd->CurrentAddress)) {
					wifi_hdr->FC.FrDs = 1;
					COPY_MAC_ADDR(&wifi_hdr->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);	/* ADDR4 = SA */
					pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
				}

#endif /* CLIENT_WDS */
			}
	}

	pStaCfg = GetStaCfgByWdev(pAd, tr_entry->wdev);

	if (pAd->CommonCfg.bAPSDForcePowerSave)
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	else
		wifi_hdr->FC.PwrMgmt = (RtmpPktPmBitCheck(pAd, pStaCfg) == TRUE);


	pTxBlk->dot11_type = wifi_hdr->FC.Type;
	pTxBlk->dot11_subtype = wifi_hdr->FC.SubType;

	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;
}
#endif /* DOT11_N_SUPPORT */


VOID sta_build_802_11_header(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, tr_entry->wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	/*
	 *	MAKE A COMMON 802.11 HEADER
	 */
	ASSERT((tr_entry != NULL));

	if (!tr_entry)
		return;

	/* Get the header start offset and normal wlan header size : 24 octets */
	pTxBlk->wifi_hdr = &pTxBlk->HeaderBuf[tx_hw_hdr_len];
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	wifi_hdr = (HEADER_802_11 *)pTxBlk->wifi_hdr;
	/* TODO:shiang-usw, is it necessary to do this zero here? */
	NdisZeroMemory(wifi_hdr, sizeof(HEADER_802_11));
	wifi_hdr->FC.FrDs = 0;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM) ? SUBTYPE_QDATA : SUBTYPE_DATA;

	/* TODO: shiang-usw, for BCAST/MCAST, original it's sequence assigned by "pAd->Sequence", how about now? */
	if (tr_entry) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
			wifi_hdr->Sequence = tr_entry->TxSeq[pTxBlk->UserPriority];
			tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
		} else {
			wifi_hdr->Sequence = tr_entry->NonQosDataSeq;
			tr_entry->NonQosDataSeq = (tr_entry->NonQosDataSeq + 1) & MAXSEQ;
		}
	}

	wifi_hdr->Frag = 0;
	wifi_hdr->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);
	{
		if (pStaCfg->BssType == BSS_INFRA) {
			COPY_MAC_ADDR(wifi_hdr->Addr1, tr_entry->Addr);
			COPY_MAC_ADDR(wifi_hdr->Addr2, tr_entry->wdev->if_addr);
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bTdlsEntry)) {
				COPY_MAC_ADDR(wifi_hdr->Addr3, tr_entry->wdev->bssid);
				wifi_hdr->FC.ToDs = 0;
			} else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			{
				COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);
				wifi_hdr->FC.ToDs = 1;
#ifdef CLIENT_WDS

				if (!MAC_ADDR_EQUAL((pTxBlk->pSrcBufHeader + MAC_ADDR_LEN), tr_entry->wdev->if_addr)) {
					wifi_hdr->FC.FrDs = 1;
					COPY_MAC_ADDR(&wifi_hdr->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
					pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
				}

#endif /* CLIENT_WDS */
			}
		} else if (ADHOC_ON(pAd)) {
			COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
#ifdef XLINK_SUPPORT

			if (pStaCfg->PSPXlink)
				/* copy the SA of ether frames to address 2 of 802.11 frame */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
			else
#endif /* XLINK_SUPPORT */
				COPY_MAC_ADDR(wifi_hdr->Addr2, tr_entry->wdev->if_addr);

			COPY_MAC_ADDR(wifi_hdr->Addr3, pStaCfg->Bssid);
			wifi_hdr->FC.ToDs = 0;
		}
	}

	if (pTxBlk->CipherAlg)
		wifi_hdr->FC.Wep = 1;

	if (pAd->CommonCfg.bAPSDForcePowerSave)
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	else
		wifi_hdr->FC.PwrMgmt = (RtmpPktPmBitCheck(pAd, pStaCfg) == TRUE);

	/* build QOS Control bytes */
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		UCHAR *buf = (UCHAR *)(pTxBlk->wifi_hdr + pTxBlk->wifi_hdr_len);

		*buf = ((pTxBlk->UserPriority & 0x0F) | (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] << 5));

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
		UAPSD_MR_EOSP_SET(buf, pTxBlk);
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

		if (pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			*buf |= 0x80;

		*(buf + 1) = 0;
		pTxBlk->wifi_hdr_len += 2;
	}

	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;

	pTxBlk->dot11_type = wifi_hdr->FC.Type;
	pTxBlk->dot11_subtype = wifi_hdr->FC.SubType;
}

INT sta_ieee_802_3_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	UCHAR wdev_idx = BSS0;
	BOOLEAN bFragment = FALSE;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef TR181_SUPPORT
	UCHAR bandIdx = HcGetBandByWdev(wdev);
#endif
	wdev_idx = wdev->wdev_idx;
	MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
			 __func__, pEntry->wcid, wdev->wdev_idx,
			 pRxBlk->Flags,
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC),
			 pFmeCtrl->Type, pFmeCtrl->SubType,
			 pFmeCtrl->FrDs, pFmeCtrl->ToDs);

	/* Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461 */
	/* must be here, before no DATA check */
	if (ops->rx_ps_handle)
		ops->rx_ps_handle(pAd, wdev, pRxBlk);

	pEntry->NoDataIdleCount = 0;
	tr_ctl->tr_entry[pEntry->wcid].NoDataIdleCount = 0;
#ifdef TR181_SUPPORT
	pAd->WlanCounters[bandIdx].RxTotByteCount.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
	pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;

	if (((FRAME_CONTROL *)pRxBlk->FC)->SubType & 0x08) {

		if ((pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID] != 0)
			&& pRxInfo->U2M)
			pRxInfo->BA = 1;
		else
			pRxInfo->BA = 0;

		if (pRxBlk->AmsduState)
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);

		if (pRxInfo->BA)
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
	}

	/*check if duplicate frame, ignore it and then drop*/
	if (rx_chk_duplicate_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): duplicate frame drop it!\n", __func__);
		return FALSE;
	}

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if (((FRAME_CONTROL *)pRxBlk->FC)->SubType & 0x04) { /* bit 2 : no DATA */
		MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): Null/QosNull frame!\n", __func__);
		return FALSE;
	}

	if ((pRxBlk->FN == 0) && (pFmeCtrl->MoreFrag != 0)) {
		bFragment = TRUE;
		de_fragment_data_pkt(pAd, pRxBlk);
	}

	if (pRxInfo->U2M)
		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);

#ifdef MAP_R3
	if (pRxBlk->UserPriority > 0 && pRxBlk->UserPriority < 8)
		RTMP_SET_PACKET_UP(pRxBlk->pRxPacket, pRxBlk->UserPriority);
#endif

	if (pRxBlk->pRxPacket)
		rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, pEntry->wdev);

	return TRUE;
}

INT sta_ieee_802_11_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	BOOLEAN bFragment = FALSE;
	UCHAR wdev_idx = BSS0;
	UCHAR UserPriority = 0;
	INT hdr_len = LENGTH_802_11;
	COUNTER_RALINK *pCounter = &pAd->RalinkCounters;
	UCHAR *pData;
	BOOLEAN drop_err = TRUE;
#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
	NDIS_STATUS status;
#endif /* defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT) */
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef TR181_SUPPORT
	UCHAR bandIdx = HcGetBandByWdev(wdev);
#endif

	pStaCfg = GetStaCfgByWdev(pAd, wdev);

	wdev_idx = wdev->wdev_idx;
	MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
			 __func__, pEntry->wcid, wdev->wdev_idx,
			 pRxBlk->Flags,
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC),
			 pFmeCtrl->Type, pFmeCtrl->SubType,
			 pFmeCtrl->FrDs, pFmeCtrl->ToDs);
	/* Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461 */
	/* must be here, before no DATA check */
	pData = pRxBlk->FC;

	if (ops->rx_ps_handle)
		ops->rx_ps_handle(pAd, wdev, pRxBlk);

	pEntry->NoDataIdleCount = 0;
	tr_ctl->tr_entry[pEntry->wcid].NoDataIdleCount = 0;
	/*
	 *	update RxBlk->pData, DataSize, 802.11 Header, QOS, HTC, Hw Padding
	 */
	pData = pRxBlk->FC;
	/* 1. skip 802.11 HEADER */
	pData += hdr_len;
	pRxBlk->DataSize -= hdr_len;

	/* 2. QOS */
	if (pFmeCtrl->SubType & 0x08) {
		UserPriority = *(pData) & 0x0f;

		if ((pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID] != 0)
			&& pRxInfo->U2M)
			pRxInfo->BA = 1;
		else
			pRxInfo->BA = 0;


		/* bit 7 in QoS Control field signals the HT A-MSDU format */
		if ((*pData) & 0x80) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
			pCounter->RxAMSDUCount.u.LowPart++;
		}

		if (pRxInfo->BA) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
			/* incremented by the number of MPDUs */
			/* received in the A-MPDU when an A-MPDU is received. */
			pCounter->MPDUInReceivedAMPDUCount.u.LowPart++;
		}

		/* skip QOS contorl field */
		pData += 2;
		pRxBlk->DataSize -= 2;
	}

	pRxBlk->UserPriority = UserPriority;
#ifdef MAP_R3
	if (pRxBlk->UserPriority > 0 && pRxBlk->UserPriority < 8)
		RTMP_SET_PACKET_UP(pRxBlk->pRxPacket, pRxBlk->UserPriority);
#endif

	/*check if duplicate frame, ignore it and then drop*/
	if (rx_chk_duplicate_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): duplicate frame drop it!\n", __func__);
		return FALSE;
	}

	/*
	 *	check if need to resend PS Poll when received packet with MoreData = 1
	 *	a).only for unicast packet
	 *	b).for TDLS power save, More Data bit is not used
	 */
	if (IS_ENTRY_PEER_AP(pEntry) && (pRxInfo->U2M)) {
		if ((RtmpPktPmBitCheck(pAd, pStaCfg) == TRUE) && (pFmeCtrl->MoreData == 1)) {
			if ((((UserPriority == 0) || (UserPriority == 3)) && pAd->CommonCfg.bAPSDAC_BE == 0) ||
				(((UserPriority == 1) || (UserPriority == 2)) && pAd->CommonCfg.bAPSDAC_BK == 0) ||
				(((UserPriority == 4) || (UserPriority == 5)) && pAd->CommonCfg.bAPSDAC_VI == 0) ||
				(((UserPriority == 6) || (UserPriority == 7)) && pAd->CommonCfg.bAPSDAC_VO == 0)) {
				/* non-UAPSD delivery-enabled AC */
				RTMP_PS_POLL_ENQUEUE(pAd, pStaCfg);
			}
		}
	}

	/* 3. Order bit: A-Ralink or HTC+ */
	if (pFmeCtrl->Order) {
#ifdef AGGREGATION_SUPPORT

		/* TODO: shiang-MT7603, fix me, because now we don't have rx_rate.field.MODE can refer */
		if ((pRxBlk->rx_rate.field.MODE <= MODE_OFDM) &&
			(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE)))
			RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
		else
#endif /* AGGREGATION_SUPPORT */
		{
			/* skip HTC control field */
			pData += 4;
			pRxBlk->DataSize -= 4;
		}
	}

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if (pFmeCtrl->SubType & 0x04) { /* bit 2 : no DATA */
		MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): Null/QosNull frame!\n", __func__);
		drop_err = FALSE;
		return FALSE;
	}

	/* 4. skip HW padding */
	if (pRxInfo->L2PAD) {
		/* just move pData pointer because DataSize excluding HW padding */
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pData += 2;
	}

	pRxBlk->pData = pData;
#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)

	/* Use software to decrypt the encrypted frame if necessary.
	 *   If a received "encrypted" unicast packet(its WEP bit as 1)
	 *   and it's passed to driver with "Decrypted" marked as 0 in RxInfo.
	 */
	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		if ((pFmeCtrl->Wep == 1) && (pRxInfo->Decrypted == 0)) {
#ifdef HDR_TRANS_SUPPORT

			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
				UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pRxBlk->pRxPacket);

				status = RTMPSoftDecryptionAction(pAd,
												  pRxBlk->FC,
												  UserPriority,
												  &pEntry->PairwiseKey,
												  wdev_idx,
												  pRxBlk->pTransData + 14,
												  &(pRxBlk->TransDataSize));
			} else
#endif /* HDR_TRANS_SUPPORT */
			{
				CIPHER_KEY *pSwKey = &pEntry->PairwiseKey;

				if (IS_ENTRY_PEER_AP(pEntry)) {
					pSwKey = RTMPSwCipherKeySelection(pAd,
													  pRxBlk->pData, pRxBlk,
													  pEntry);

					/* Cipher key table selection */
					if (!pSwKey) {
						MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "No vaild cipher key for SW decryption!!!\n");
						RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
						return;
					}
				}

				status = RTMPSoftDecryptionAction(pAd,
												  pRxBlk->FC,
												  UserPriority,
												  pSwKey,
												  wdev_idx,
												  pRxBlk->pData,
												  &(pRxBlk->DataSize));
			}

			if (status != NDIS_STATUS_SUCCESS) {
				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}

			/* Record the Decrypted bit as 1 */
			pRxInfo->Decrypted = 1;
		}
	}

#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT
#ifdef TDLS_AUTOLINK_SUPPORT

	if (pAd->StaCfg[0].TdlsInfo.TdlsAutoLink) {
		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_DLS))
			TDLS_AutoSetupByRcvFrame(pAd, pRxBlk->FC);
	}

#endif /* TDLS_AUTOLINK_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef SMART_ANTENNA

	if (RTMP_SA_WORK_ON(pAd))
		sa_pkt_radio_info_update(pAd, pRxBlk, pEntry);

#endif /* SMART_ANTENNA */

	pEntry->NoDataIdleCount = 0;
	/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
	tr_ctl->tr_entry[pEntry->wcid].NoDataIdleCount = 0;
	/* Case I  Process Broadcast & Multicast data frame */
	MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s():pRxInfo->Bcast =%d, pRxInfo->Mcast=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x\n",
			 __func__, pRxInfo->Bcast, pRxInfo->Mcast, pRxBlk->Flags,
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC));

	if (pRxInfo->Bcast || pRxInfo->Mcast) {
		/* Drop Mcast/Bcast frame with fragment bit on */
		if (pFmeCtrl->MoreFrag) {
			MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): MoreFrag!\n", __func__);
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return TRUE;
		}

		/* Filter out Bcast frame which AP relayed for us */
		if (pFmeCtrl->FrDs
			&& MAC_ADDR_EQUAL(pRxBlk->Addr3, wdev->if_addr)) {
			MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): pFmeCtrl->FrDs!\n", __func__);
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return TRUE;
		}

#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters[0].MulticastReceivedFrameCount);
#endif /* STATS_COUNT_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT
#ifdef WFD_SUPPORT

		if ((pAd->StaCfg[0].WfdCfg.bWfdEnable) &&
			pRxInfo->Bcast &&
			TDLS_CheckTDLSframe(pAd, pRxBlk->pData, pRxBlk->DataSize)) {
			UCHAR *pTmpBuf = pRxBlk->pData - LENGTH_802_11;

			NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, LENGTH_802_11);
			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
							pTmpBuf,
							pRxBlk->DataSize + LENGTH_802_11,
							pRxBlk->rx_signal.raw_rssi[0],
							pRxBlk->rx_signal.raw_rssi[1],
							pRxBlk->rx_signal.raw_rssi[2],
							pRxBlk->rx_signal.raw_rssi[3],
							0,
							pRxBlk->channel_freq,
							OPMODE_STA,
							wdev,
							pRxBlk->rx_rate.field.MODE);
			MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "!!! report TDLS Action DATA to MLME (len=%d) !!!\n",
					  pRxBlk->DataSize);
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

#endif /* WFD_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */
		indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
		return TRUE;
	}

	if (pRxInfo->U2M) {
		pAd->RalinkCounters.OneSecRxOkDataCnt++;
#ifdef STA_LP_PHASE_2_SUPPORT

		/* there's packet sent to me, keep awake for 1200ms */
		if (pStaCfg)
			if (pStaCfg->CountDowntoPsm < STAY_2_SECONDS_AWAKE)
				pStaCfg->CountDowntoPsm = STAY_2_SECONDS_AWAKE;

#else
#endif /* STA_LP_PHASE_2_SUPPORT */
		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);
#ifdef TXBF_SUPPORT

		if (pRxBlk->rx_rate.field.ShortGI)
			pEntry->OneSecRxSGICount++;
		else
			pEntry->OneSecRxLGICount++;

#endif /* TXBF_SUPPORT */
#ifdef DBG_DIAGNOSE

		if (pAd->DiagStruct.inited) {
			struct dbg_diag_info *diag_info;

			diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
			diag_info->RxDataCnt++;
		}

#endif /* DBG_DIAGNOSE */
	}

#ifdef XLINK_SUPPORT
	else if (IS_ENTRY_PEER_AP(pEntry)) {
		if (pAd->StaCfg[0].PSPXlink) {
			indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
			return;
		} else
			goto drop;
	}

#endif /* XLINK_SUPPORT */
	wdev->LastSNR0 = (UCHAR)(pRxBlk->rx_signal.raw_snr[0]);
	wdev->LastSNR1 = (UCHAR)(pRxBlk->rx_signal.raw_snr[1]);
	pEntry->freqOffset = (CHAR)(pRxBlk->rx_signal.freq_offset);
	pEntry->freqOffsetValid = TRUE;

	if ((pRxBlk->FN != 0) || (pFmeCtrl->MoreFrag != 0)) {
		bFragment = TRUE;
		de_fragment_data_pkt(pAd, pRxBlk);
	}

	if (pRxBlk->pRxPacket) {
		/*
		 *	process complete frame which encrypted by TKIP,
		 *	Minus MIC length and calculate the MIC value
		 */
		if (bFragment && (pFmeCtrl->Wep) && IS_CIPHER_TKIP_Entry(pEntry)) {
			pRxBlk->DataSize -= 8;

			if (rtmp_chk_tkip_mic(pAd, pEntry, pRxBlk) == FALSE)
				return TRUE;
		}
#ifdef TR181_SUPPORT
		pAd->WlanCounters[bandIdx].RxTotByteCount.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
		pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;
#ifdef MAC_REPEATER_SUPPORT

		if (IS_ENTRY_PEER_AP(pEntry))
			RTMP_SET_PACKET_WCID(pRxBlk->pRxPacket, pRxBlk->wcid);

#endif /* MAC_REPEATER_SUPPORT */

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
		else
			rx_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
	}

	return TRUE;
}

VOID sta_ieee_802_11_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	BOOLEAN bVLANPkt;
	UCHAR *buf_ptr;

	sta_build_802_11_header(pAd, pTxBlk);
#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}

#endif /* SOFT_ENCRYPT */
	/* skip 802.3 header and VLAN tag if have */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen -= LENGTH_802_3;
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

	if (bVLANPkt) {
		pTxBlk->pSrcBufData += LENGTH_802_1Q;
		pTxBlk->SrcBufLen -= LENGTH_802_1Q;
	}

	wifi_hdr = (HEADER_802_11 *)pTxBlk->wifi_hdr;
	/* The remaining content of MPDU header should locate at 4-octets aligment */
	pTxBlk->HdrPadLen = (pTxBlk->wifi_hdr_len & 0x3);

	if (pTxBlk->HdrPadLen)
		pTxBlk->HdrPadLen = 4 - pTxBlk->HdrPadLen;

	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;
	buf_ptr = (UCHAR *)(pTxBlk->wifi_hdr + pTxBlk->wifi_hdr_len + pTxBlk->HdrPadLen);
#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		tx_sw_encrypt(pAd, pTxBlk, buf_ptr, wifi_hdr);
	else
#endif /* SOFT_ENCRYPT */
	{
		/*
		 *	Insert LLC-SNAP encapsulation - 8 octets
		 *	if original Ethernet frame contains no LLC/SNAP,
		 *	then an extra LLC/SNAP encap is required
		 */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
										   pTxBlk->pExtraLlcSnapEncap);

		if (pTxBlk->pExtraLlcSnapEncap) {
			UCHAR vlan_size;

			NdisMoveMemory(buf_ptr, pTxBlk->pExtraLlcSnapEncap, 6);
			buf_ptr += 6;
			/* skip vlan tag */
			vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(buf_ptr,
						   pTxBlk->pSrcBufHeader + 12 + vlan_size,
						   2);
			buf_ptr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H; /* 6 + 2 */
		}
	}
}

VOID sta_ieee_802_3_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	pTxBlk->MpduHeaderLen = 0;
	pTxBlk->HdrPadLen = 0;
	pTxBlk->wifi_hdr_len = 0;
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
}

INT sta_ampdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr = NULL;
	USHORT freeCnt = 0;
	BOOLEAN bVLANPkt;
	MAC_TABLE_ENTRY *pMacEntry;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN bHTCPlus = FALSE;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#ifdef TR181_SUPPORT
	UCHAR bandIdx = HcGetBandByWdev(wdev);
#endif
	if (!fill_tx_blk(pAd, wdev, pTxBlk)) {
		tr_cnt->fill_tx_blk_fail_drop++;
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = pTxBlk->tr_entry;

	if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
		if ((tr_entry->isCached)) {
			/*
			 *	NOTE: Please make sure the size of tr_entry->CachedBuf[]
			 *	is smaller than pTxBlk->HeaderBuf[]!!!!
			 */
#ifndef VENDOR_FEATURE1_SUPPORT
			NdisMoveMemory((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]),
						   (UCHAR *)(&tr_entry->CachedBuf[0]),
						   TXWISize + sizeof(HEADER_802_11));
#else
			pTxBlk->HeaderBuf = (UCHAR *)(tr_entry->HeaderBuf);
#endif /* VENDOR_FEATURE1_SUPPORT */
			pHeaderBufPtr = (UCHAR *)(&pTxBlk->HeaderBuf[tx_hw_hdr_len]);
			sta_build_cache_802_11_header(pAd, pTxBlk, pHeaderBufPtr);
#ifdef SOFT_ENCRYPT
			RTMPUpdateSwCacheCipherInfo(pAd, pTxBlk, pHeaderBufPtr);
#endif /* SOFT_ENCRYPT */
		} else {
			sta_build_802_11_header(pAd, pTxBlk);
			pHeaderBufPtr = &pTxBlk->HeaderBuf[tx_hw_hdr_len];
		}

#ifdef SOFT_ENCRYPT

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
			if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
				RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}
		}

#endif /* SOFT_ENCRYPT */
		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
		wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;
		/* skip common header */
		pHeaderBufPtr += pTxBlk->MpduHeaderLen;
#ifdef VENDOR_FEATURE1_SUPPORT

		if (tr_entry->isCached
			&& (tr_entry->Protocol == (RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket)))
#ifdef SOFT_ENCRYPT
			&& !TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)
#endif /* SOFT_ENCRYPT */
		   ) {
			/* build QOS Control bytes */
			*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
			UAPSD_MR_EOSP_SET(pHeaderBufPtr, pTxBlk);
#endif /* UAPSD_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			pTxBlk->MpduHeaderLen = tr_entry->MpduHeaderLen;
			pTxBlk->wifi_hdr_len = tr_entry->wifi_hdr_len;
			pHeaderBufPtr = ((UCHAR *)wifi_hdr) + pTxBlk->MpduHeaderLen;
			pTxBlk->HdrPadLen = tr_entry->HdrPadLen;
			/* skip 802.3 header */
			pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
			pTxBlk->SrcBufLen -= LENGTH_802_3;

			/* skip vlan tag */
			if (bVLANPkt) {
				pTxBlk->pSrcBufData += LENGTH_802_1Q;
				pTxBlk->SrcBufLen -= LENGTH_802_1Q;
			}
		} else
#endif /* VENDOR_FEATURE1_SUPPORT */
		{
			/* build HTC control field after QoS field */
			bHTCPlus = FALSE;

			if ((pAd->CommonCfg.bRdg == TRUE)
				&& (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE))) {
				if (tr_entry->isCached == FALSE) {
					NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
					((PHT_CONTROL)pHeaderBufPtr)->RDG = 1;
				}

				bHTCPlus = TRUE;
			}

			if (bHTCPlus == TRUE) {
				wifi_hdr->FC.Order = 1;
				pHeaderBufPtr += 4;
				pTxBlk->MpduHeaderLen += 4;
				pTxBlk->wifi_hdr_len += 4;
			}

			ASSERT(pTxBlk->MpduHeaderLen >= 24);
			/* skip 802.3 header */
			pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
			pTxBlk->SrcBufLen -= LENGTH_802_3;

			/* skip vlan tag */
			if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket)) {
				pTxBlk->pSrcBufData += LENGTH_802_1Q;
				pTxBlk->SrcBufLen -= LENGTH_802_1Q;
			}

			/*
			 *	padding at front of LLC header
			 *	LLC header should locate at 4-octets aligment
			 *
			 *	@@@ MpduHeaderLen excluding padding @@@
			 */
			pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
			pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
			pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);
#ifdef VENDOR_FEATURE1_SUPPORT
			tr_entry->HdrPadLen = pTxBlk->HdrPadLen;
#endif /* VENDOR_FEATURE1_SUPPORT */
#ifdef SOFT_ENCRYPT

			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
				tx_sw_encrypt(pAd, pTxBlk, pHeaderBufPtr, wifi_hdr);
			else
#endif /* SOFT_ENCRYPT */
			{
				/*
				 *   Insert LLC-SNAP encapsulation - 8 octets
				 *	if original Ethernet frame contains no LLC/SNAP,
				 *	then an extra LLC/SNAP encap is required
				 */
				EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2, pTxBlk->pExtraLlcSnapEncap);

				if (pTxBlk->pExtraLlcSnapEncap) {
					NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
					pHeaderBufPtr += 6;
					/* get 2 octets (TypeofLen) */
					NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData - 2, 2);
					pHeaderBufPtr += 2;
					pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
				}
			}

#ifdef VENDOR_FEATURE1_SUPPORT
			tr_entry->Protocol = RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket);
			tr_entry->MpduHeaderLen = pTxBlk->MpduHeaderLen;
			tr_entry->wifi_hdr_len = pTxBlk->wifi_hdr_len;
#endif /* VENDOR_FEATURE1_SUPPORT */
		}
	} else {
		pTxBlk->MpduHeaderLen = 0;
		pTxBlk->HdrPadLen = 2;
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	}

	if ((tr_entry->isCached))
		asic_write_tmac_info(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);
	else {
		asic_write_tmac_info(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);

		if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket))
			tr_entry->isCached = FALSE;

		NdisZeroMemory((UCHAR *)(&tr_entry->CachedBuf[0]), sizeof(tr_entry->CachedBuf));
		NdisMoveMemory((UCHAR *)(&tr_entry->CachedBuf[0]),
					   (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]),
					   (pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE])));
#ifdef VENDOR_FEATURE1_SUPPORT
		/* use space to get performance enhancement */
		NdisZeroMemory((UCHAR *)(&tr_entry->HeaderBuf[0]), sizeof(tr_entry->HeaderBuf));
		NdisMoveMemory((UCHAR *)(&tr_entry->HeaderBuf[0]),
					   (UCHAR *)(&pTxBlk->HeaderBuf[0]),
					   (pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[0])));
#endif /* VENDOR_FEATURE1_SUPPORT */
	}

#ifdef STATS_COUNT_SUPPORT
	pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart++;
	pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.QuadPart += pTxBlk->SrcBufLen;
#endif /* STATS_COUNT_SUPPORT */
#ifdef TR181_SUPPORT
	pAd->WlanCounters[bandIdx].TxTotByteCount.QuadPart += pTxBlk->SrcBufLen;
#endif
	pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
	asic_write_tx_resource(pAd, pTxBlk, TRUE, &freeCnt);
#ifdef SMART_ANTENNA

	if (pMacEntry)
		pMacEntry->saTxCnt++;

#endif /* SMART_ANTENNA */

	return NDIS_STATUS_SUCCESS;
}

INT sta_amsdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	PQUEUE_ENTRY pQEntry;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT index = 0;

	ASSERT((tx_blk->TxPacketList.Number > 1));

	while (tx_blk->TxPacketList.Head) {
		pQEntry = RemoveHeadQueue(&tx_blk->TxPacketList);
		tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

		if (index == 0)
			tx_blk->amsdu_state = TX_AMSDU_ID_FIRST;
		else if (index == (tx_blk->TotalFrameNum - 1))
			tx_blk->amsdu_state = TX_AMSDU_ID_LAST;
		else
			tx_blk->amsdu_state = TX_AMSDU_ID_MIDDLE;

		if (!fill_tx_blk(pAd, wdev, tx_blk)) {
			tr_cnt->fill_tx_blk_fail_drop++;
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS))
			wdev_ops->ieee_802_3_data_tx(pAd, wdev, tx_blk);
		else
			wdev_ops->ieee_802_11_data_tx(pAd, wdev, tx_blk);

		ret = asic_hw_tx(pAd, tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

		tx_blk->frame_idx++;
		index++;
	}

	return NDIS_STATUS_SUCCESS;
}

INT sta_legacy_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
#ifdef ADHOC_WPA2PSK_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#ifdef TR181_SUPPORT
	UCHAR bandIdx = HcGetBandByWdev(wdev);
#endif
	if (!fill_tx_blk(pAd, wdev, pTxBlk)) {
		tr_cnt->fill_tx_blk_fail_drop++;
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

#ifdef STATS_COUNT_SUPPORT

	if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
		INC_COUNTER64(pAd->WlanCounters[0].MulticastTransmittedFrameCount);

#endif /* STATS_COUNT_SUPPORT */

	/* TODO: shiang-usw, remove this to other place! */
	if (pTxBlk->TxRate < pAd->CommonCfg.MinTxRate)
		pTxBlk->TxRate = pAd->CommonCfg.MinTxRate;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
		ops->ieee_802_3_data_tx(pAd, wdev, pTxBlk);
	else
		ops->ieee_802_11_data_tx(pAd, wdev, pTxBlk);

#ifdef ADHOC_WPA2PSK_SUPPORT

	if (ADHOC_ON(pAd)
		&& (pStaCfg->wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
		&& (pStaCfg->GroupCipher == Ndis802_11AESEnable)
		&& (!pTxBlk->pMacEntry)) {
		/* use Wcid as Hardware Key Index */
		/* GET_GroupKey_WCID(pAd, pTxBlk->Wcid, BSS0); */
		pTxBlk->Wcid = pStaCfg->wdev.bss_info_argument.bmc_wlan_idx;
	}

#endif /* ADHOC_WPA2PSK_SUPPORT */
	ret = asic_hw_tx(pAd, pTxBlk);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

#ifdef SMART_ANTENNA

	if (pTxBlk->pMacEntry)
		pTxBlk->pMacEntry->saTxCnt++;

#endif /* SMART_ANTENNA */
#ifdef TR181_SUPPORT
	pAd->WlanCounters[bandIdx].TxTotByteCount.QuadPart += pTxBlk->SrcBufLen;
#endif
	pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
	return NDIS_STATUS_SUCCESS;
}

INT sta_frag_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *buf_ptr;
	BOOLEAN bVLANPkt;
	PACKET_INFO PacketInfo;
#ifdef SOFT_ENCRYPT
	UCHAR *tmp_ptr = NULL;
	UINT32 buf_offset = 0;
#endif /* SOFT_ENCRYPT */
	HTTRANSMIT_SETTING *pTransmit;
	UCHAR fragNum = 0;
	USHORT EncryptionOverhead = 0;
	UINT32 FreeMpduSize, SrcRemainingBytes;
	USHORT AckDuration;
	UINT NextMpduSize;
	UINT32 ret = NDIS_STATUS_SUCCESS;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#ifdef TR181_SUPPORT
	UCHAR bandIdx = HcGetBandByWdev(wdev);
#endif
	if (!fill_tx_blk(pAd, wdev, pTxBlk)) {
		tr_cnt->fill_tx_blk_fail_drop++;
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	sta_build_802_11_header(pAd, pTxBlk);
#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}

#endif /* SOFT_ENCRYPT */

	if (IS_CIPHER_TKIP(pTxBlk->CipherAlg)) {
		pTxBlk->pPacket = duplicate_pkt_with_TKIP_MIC(pAd, pTxBlk->pPacket);

		if (pTxBlk->pPacket == NULL)
			return NDIS_STATUS_FAILURE;

		RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
	}

	/* skip 802.3 header */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen -= LENGTH_802_3;
	/* skip vlan tag */
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

	if (bVLANPkt) {
		pTxBlk->pSrcBufData += LENGTH_802_1Q;
		pTxBlk->SrcBufLen -= LENGTH_802_1Q;
	}

	buf_ptr = pTxBlk->wifi_hdr;
	wifi_hdr = (HEADER_802_11 *)buf_ptr;
	/* skip common header */
	buf_ptr += pTxBlk->wifi_hdr_len;
	/* The remaining content of MPDU header should locate at 4-octets aligment */
	pTxBlk->HdrPadLen = (ULONG)buf_ptr;
	buf_ptr = (UCHAR *)ROUND_UP(buf_ptr, 4);
	pTxBlk->HdrPadLen = (ULONG)(buf_ptr - pTxBlk->HdrPadLen);
	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;
#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		UCHAR iv_offset = 0;
		/*
		 *	If original Ethernet frame contains no LLC/SNAP,
		 *	then an extra LLC/SNAP encap is required
		 */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
											pTxBlk->pExtraLlcSnapEncap);

		/* Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer */
		if (pTxBlk->pExtraLlcSnapEncap) {
			/* Reserve the front 8 bytes of data for LLC header */
			pTxBlk->pSrcBufData -= LENGTH_802_1_H;
			pTxBlk->SrcBufLen += LENGTH_802_1_H;
			NdisMoveMemory(pTxBlk->pSrcBufData, pTxBlk->pExtraLlcSnapEncap, 6);
		}

		/* Construct and insert specific IV header to MPDU header */
		RTMPSoftConstructIVHdr(pTxBlk->CipherAlg,
							   pTxBlk->KeyIdx,
							   pTxBlk->pKey->TxTsc,
							   buf_ptr, &iv_offset);
		buf_ptr += iv_offset;
		pTxBlk->MpduHeaderLen += iv_offset;
	} else
#endif /* SOFT_ENCRYPT */
	{
		/*
		 *   Insert LLC-SNAP encapsulation - 8 octets
		 *	if original Ethernet frame contains no LLC/SNAP,
		 *	then an extra LLC/SNAP encap is required
		 */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
										   pTxBlk->pExtraLlcSnapEncap);

		if (pTxBlk->pExtraLlcSnapEncap) {
			UCHAR vlan_size;

			NdisMoveMemory(buf_ptr, pTxBlk->pExtraLlcSnapEncap, 6);
			buf_ptr += 6;
			/* skip vlan tag */
			vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(buf_ptr, pTxBlk->pSrcBufHeader + 12 + vlan_size, 2);
			buf_ptr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}
	}

	/*
	 *1. If TKIP is used and fragmentation is required. Driver has to
	 *	   append TKIP MIC at tail of the scatter buffer
	 *	2. When TXWI->FRAG is set as 1 in TKIP mode,
	 *	   MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC
	 */
	/*  TKIP appends the computed MIC to the MSDU data prior to fragmentation into MPDUs. */
	if (IS_CIPHER_TKIP(pTxBlk->CipherAlg)) {
#ifdef ETH_CONVERT_SUPPORT
		/*
		 *   When enable dongle mode for EthernetConverter, we cannot directly calculate the
		 *   MIC value base on original 802.3 packet, we need use our MAC address as the
		 *   src MAC of 802.3 packet to calculate the MIC, so we use the "bDonglePkt" to
		 *   indicate if the function should calculate this packet base on origianl paylaod
		 *   or need to change the srcMAC as our MAC address.
		 */
		RTMPCalculateMICValue(pAd, pTxBlk->pPacket, pTxBlk->pExtraLlcSnapEncap, pTxBlk->pKey->Key, &pTxBlk->pKey->Key[LEN_TK],
							  (TX_BLK_TEST_FLAG(pTxBlk, fTX_bDonglePkt)));
#else /* ETH_CONVERT_SUPPORT */
		RTMPCalculateMICValue(pAd, pTxBlk->pPacket, pTxBlk->pExtraLlcSnapEncap, pTxBlk->pKey->Key, &pTxBlk->pKey->Key[LEN_TK], 0);

#endif /* !ETH_CONVERT_SUPPORT */
		/*
		 *   NOTE: DON'T refer the skb->len directly after following copy. Becasue the length is not adjust
		 *		to correct lenght, refer to pTxBlk->SrcBufLen for the packet length in following progress.
		 */
		NdisMoveMemory(pTxBlk->pSrcBufData + pTxBlk->SrcBufLen, &pAd->PrivateInfo.Tx.MIC[0], 8);
		pTxBlk->SrcBufLen += 8;
		pTxBlk->TotalFrameLen += 8;
	}

	/*
	 *   calcuate the overhead bytes that encryption algorithm may add. This
	 *   affects the calculate of "duration" field
	 */
	if (IS_CIPHER_WEP(pTxBlk->CipherAlg))
		EncryptionOverhead = 8; /* WEP: IV[4] + ICV[4]; */
	else if (IS_CIPHER_TKIP(pTxBlk->CipherAlg))
		EncryptionOverhead = 12; /* TKIP: IV[4] + EIV[4] + ICV[4], MIC will be added to TotalPacketLength */
	else if (IS_CIPHER_CCMP128(pTxBlk->CipherAlg)
			 || IS_CIPHER_CCMP256(pTxBlk->CipherAlg)
			 || IS_CIPHER_GCMP128(pTxBlk->CipherAlg)
			 || IS_CIPHER_GCMP256(pTxBlk->CipherAlg))
		EncryptionOverhead = 16;	/* AES: IV[4] + EIV[4] + MIC[8] */

	else
		EncryptionOverhead = 0;

	pTransmit = pTxBlk->pTransmit;

	/* Decide the TX rate */
	if (pTransmit->field.MODE == MODE_CCK)
		pTxBlk->TxRate = pTransmit->field.MCS;
	else if (pTransmit->field.MODE == MODE_OFDM)
		pTxBlk->TxRate = pTransmit->field.MCS + RATE_FIRST_OFDM_RATE;
	else
		pTxBlk->TxRate = RATE_6_5;

	/* decide how much time an ACK/CTS frame will consume in the air */
	if (pTxBlk->TxRate <= RATE_LAST_OFDM_RATE)
		AckDuration = RTMPCalcDuration(pAd, pAd->CommonCfg.ExpectedACKRate[pTxBlk->TxRate], 14);
	else
		AckDuration = RTMPCalcDuration(pAd, RATE_6_5, 14);

	/*MTWF_DBG(NULL, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "!!!Fragment AckDuration(%d), TxRate(%d)!!!\n", AckDuration, pTxBlk->TxRate); */
#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		/* store the outgoing frame for calculating MIC per fragmented frame */
		os_alloc_mem(pAd, (PUCHAR *)&tmp_ptr, pTxBlk->SrcBufLen);

		if (tmp_ptr == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "no memory for MIC calculation!\n");
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}

		NdisMoveMemory(tmp_ptr, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	}

#endif /* SOFT_ENCRYPT */
	/* Init the total payload length of this frame. */
	SrcRemainingBytes = pTxBlk->SrcBufLen;
	pTxBlk->TotalFragNum = 0xff;

	do {
		FreeMpduSize = wlan_operate_get_frag_thld(wdev);
		FreeMpduSize -= LENGTH_CRC;
		FreeMpduSize -= pTxBlk->MpduHeaderLen;

		if (SrcRemainingBytes <= FreeMpduSize) {
			/* This is the last or only fragment */
			pTxBlk->SrcBufLen = SrcRemainingBytes;
			wifi_hdr->FC.MoreFrag = 0;
			wifi_hdr->Duration = pAd->CommonCfg.Dsifs + AckDuration;
			/* Indicate the lower layer that this's the last fragment. */
			pTxBlk->TotalFragNum = fragNum;
#ifdef MT_MAC
			pTxBlk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_NO : TX_FRAG_ID_LAST);
#endif /* MT_MAC */
		} else {
			/* more fragment is required */
			pTxBlk->SrcBufLen = FreeMpduSize;
			NextMpduSize = min(((UINT)SrcRemainingBytes - pTxBlk->SrcBufLen),
							   ((UINT)wlan_operate_get_frag_thld(wdev)));
			wifi_hdr->FC.MoreFrag = 1;
			wifi_hdr->Duration = (3 * pAd->CommonCfg.Dsifs) + (2 * AckDuration) +
								 RTMPCalcDuration(pAd, pTxBlk->TxRate, NextMpduSize + EncryptionOverhead);
#ifdef MT_MAC
			pTxBlk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_FIRST : TX_FRAG_ID_MIDDLE);
#endif /* MT_MAC */
		}

		SrcRemainingBytes -= pTxBlk->SrcBufLen;

		if (fragNum == 0)
			pTxBlk->FrameGap = IFS_HTTXOP;
		else
			pTxBlk->FrameGap = IFS_SIFS;

#ifdef SOFT_ENCRYPT

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
			UCHAR ext_offset = 0;

			NdisMoveMemory(pTxBlk->pSrcBufData, tmp_ptr + buf_offset, pTxBlk->SrcBufLen);
			buf_offset += pTxBlk->SrcBufLen;
			/* Encrypt the MPDU data by software */
			RTMPSoftEncryptionAction(pAd,
									 pTxBlk->CipherAlg,
									 (UCHAR *)wifi_hdr,
									 pTxBlk->pSrcBufData,
									 pTxBlk->SrcBufLen,
									 pTxBlk->KeyIdx,
									 pTxBlk->pKey,
									 &ext_offset);
			pTxBlk->SrcBufLen += ext_offset;
			pTxBlk->TotalFrameLen += ext_offset;
		}

#endif /* SOFT_ENCRYPT */
		ret = asic_hw_tx(pAd, pTxBlk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

#ifdef SMART_ANTENNA

		if (pTxBlk->pMacEntry)
			pTxBlk->pMacEntry->saTxCnt++;

#endif /* SMART_ANTENNA */
#ifdef TR181_SUPPORT
		pAd->WlanCounters[bandIdx].TxTotByteCount.QuadPart += pTxBlk->SrcBufLen;
#endif
		pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
#ifdef SOFT_ENCRYPT

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
				if ((pTxBlk->CipherAlg == CIPHER_WEP64) || (pTxBlk->CipherAlg == CIPHER_WEP128)) {
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
					/* Construct and insert 4-bytes WEP IV header to MPDU header */
					RTMPConstructWEPIVHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
										  buf_ptr - (LEN_WEP_IV_HDR));
				} else if (pTxBlk->CipherAlg == CIPHER_TKIP)
					;
				else if (pTxBlk->CipherAlg == CIPHER_AES) {
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
					/* Construct and insert 8-bytes CCMP header to MPDU header */
					RTMPConstructCCMPHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
										 buf_ptr - (LEN_CCMP_HDR));
				}
		} else
#endif /* SOFT_ENCRYPT */
		{
			/* Update the frame number, remaining size of the NDIS packet payload. */
			if (fragNum == 0 && pTxBlk->pExtraLlcSnapEncap)
				pTxBlk->MpduHeaderLen -= LENGTH_802_1_H;	/* space for 802.11 header. */
		}

		fragNum++;
		/* SrcRemainingBytes -= pTxBlk->SrcBufLen; */
		pTxBlk->pSrcBufData += pTxBlk->SrcBufLen;
		wifi_hdr->Frag++;	/* increase Frag # */
	} while (SrcRemainingBytes > 0);

#ifdef SOFT_ENCRYPT

	if (tmp_ptr != NULL)
		os_free_mem(tmp_ptr);

#endif /* SOFT_ENCRYPT */
	return NDIS_STATUS_SUCCESS;
}

#define RELEASE_FRAMES_OF_TXBLK(_pAd, _pTxBlk, _pQEntry, _Status)										\
	do {																					\
		while (_pTxBlk->TxPacketList.Head) {														\
			_pQEntry = RemoveHeadQueue(&_pTxBlk->TxPacketList);									\
			RELEASE_NDIS_PACKET(_pAd, QUEUE_ENTRY_TO_PACKET(_pQEntry), _Status);	\
		}																			\
	} while (0)

BOOLEAN sta_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
#ifdef ETH_CONVERT_SUPPORT
	PUCHAR pSrcBufVA = NULL;
#endif
	pPacket = pTxBlk->pPacket;
	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->FrameGap = IFS_HTTXOP;
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_MCAST_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);


	if (pTxBlk->tr_entry->EntryType == ENTRY_CAT_MCAST) {
		pTxBlk->pMacEntry = NULL;
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);
		{
			{
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;

				pTxBlk->pTransmit->field.MODE = MODE_OFDM;
				pTxBlk->pTransmit->field.MCS = MCS_RATE_6;
			}
		}
		/* AckRequired = FALSE, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, (fTX_bAckRequired | fTX_bAllowFrag | fTX_bWMM));

		if (RTMP_GET_PACKET_MOREDATA(pPacket))
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);

#ifdef ETH_CONVERT_SUPPORT

		if (ADHOC_ON(pAd)) {
			/* If we are running as Ethernet Converter, update MAT DataBase and duplicant the packet if necessary.*/
			if ((pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE) &&
				((pTxBlk->TxFrameType != TX_MCAST_FRAME) &&
				 (pTxBlk->TxFrameType != TX_MLME_DATAQ_FRAME) &&
				 (pTxBlk->TxFrameType != TX_MLME_MGMTQ_FRAME))) {
				PNDIS_PACKET donglePkt = NULL;

				if (!NdisEqualMemory(pAd->CurrentAddress, (GET_OS_PKT_DATAPTR(pPacket) + 6), MAC_ADDR_LEN)) {
					TX_BLK_SET_FLAG(pTxBlk, fTX_bDonglePkt);
					MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "@@ %s: Dongle Packet)\n", __func__);
				}

				/* For each tx packet, update our MAT convert engine databases.*/
				if (pTxBlk->pMacEntry)
					donglePkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, 0,
										 pTxBlk->pMacEntry->EntryType);
				else
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"pTxBlk->pMacEntry is NULL\n");

				if (donglePkt) {
					pPacket = donglePkt;
					pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
					pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
					pTxBlk->pPacket = donglePkt;
				}
			}
		}

#endif /* ETH_CONVERT_SUPPORT */
	} else {
		pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;
		pMacEntry = pTxBlk->pMacEntry;
#ifdef MULTI_WMM_SUPPORT

		if (IS_ENTRY_PEER_AP(pMacEntry))
			pTxBlk->QueIdx = EDCA_WMM1_AC0_PIPE;

#endif /* MULTI_WMM_SUPPORT */
		/* For all unicast packets, need Ack unless the Ack Policy is not set as NORMAL_ACK.*/
#ifdef MULTI_WMM_SUPPORT

		if (pTxBlk->QueIdx >= EDCA_WMM1_AC0_PIPE) {
			if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx - EDCA_WMM1_AC0_PIPE] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		} else
#endif /* MULTI_WMM_SUPPORT */
		{
			if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		}

#ifdef XLINK_SUPPORT

		if ((pAd->OpMode == OPMODE_STA) &&
			(ADHOC_ON(pAd)) /*&& (RX_FILTER_TEST_FLAG(pAd, fRX_FILTER_ACCEPT_PROMISCUOUS))*/) {
			if (pAd->StaCfg[0].PSPXlink)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
		}

#endif /* XLINK_SUPPORT */
		{
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)

			if (pTxBlk->OpMode == OPMODE_STA)
#else
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
			{
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

				if (IS_ENTRY_TDLS(pMacEntry))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bTdlsEntry);

#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef ETH_CONVERT_SUPPORT

				/* If we are running as Ethernet Converter, update MAT DataBase and duplicant the packet if necessary.*/
				if ((pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE) &&
					((pTxBlk->TxFrameType != TX_MCAST_FRAME) &&
					 (pTxBlk->TxFrameType != TX_MLME_DATAQ_FRAME) &&
					 (pTxBlk->TxFrameType != TX_MLME_MGMTQ_FRAME))) {
					PNDIS_PACKET donglePkt = NULL;

					if (!NdisEqualMemory(pAd->CurrentAddress, (GET_OS_PKT_DATAPTR(pPacket) + 6), MAC_ADDR_LEN)) {
						TX_BLK_SET_FLAG(pTxBlk, fTX_bDonglePkt);
						MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): Dongle Packet)\n", __func__);
					}

					/* For each tx packet, update our MAT convert engine databases.*/
					donglePkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, 0, pTxBlk->pMacEntry->EntryType);

					if (donglePkt) {
						pPacket = donglePkt;
						pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
						pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
						pTxBlk->pPacket = donglePkt;
					}

					if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
						pSrcBufVA = GET_OS_PKT_DATAPTR(pTxBlk->pPacket);
						NdisMoveMemory(pSrcBufVA + 6, wdev->if_addr, MAC_ADDR_LEN);
					}
				}

#endif /* ETH_CONVERT_SUPPORT */

				/* If support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) &&
					CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME) {
			if (((RTMP_GET_PACKET_LOWRATE(pPacket))
#ifdef UAPSD_SUPPORT
				 && (!(pMacEntry && (pMacEntry->bAPSDFlagSPStart)))
#endif /* UAPSD_SUPPORT */
				) ||
				((pAd->OpMode == OPMODE_AP) && (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			   ) {
				/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate. */
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;
				TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);

				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (!pTxBlk->pMacEntry)
					MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s():Err!! pTxBlk->pMacEntry is NULL!!\n", __func__);
				else if (IS_HT_STA(pTxBlk->pMacEntry) &&
						 (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) &&
						 ((pAd->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)))
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
			}

			if (!pMacEntry)
				MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s():Err!! pMacEntry is NULL!!\n", __func__);
			else if ((IS_HT_RATE(pMacEntry) == FALSE) &&
					 (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE))) {
				/* Currently piggy-back only support when peer is operate in b/g mode.*/
				TX_BLK_SET_FLAG(pTxBlk, fTX_bPiggyBack);
			}

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);

#ifdef UAPSD_SUPPORT

			if (RTMP_GET_PACKET_EOSP(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP);

#endif /* UAPSD_SUPPORT */
		} else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);

		if (!pMacEntry)
			MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s():Err!! pMacEntry is NULL!!\n", __func__);
		else
			pMacEntry->DebugTxCount++;
	}

	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;
	ops->find_cipher_algorithm(pAd, wdev, pTxBlk);
	return TRUE;
}

BOOLEAN sta_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;
#ifdef ETH_CONVERT_SUPPORT
	PUCHAR pSrcBufVA = NULL;
	PNDIS_PACKET convertPkt = NULL;
#endif

	pPacket = pTxBlk->pPacket;
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);

	if (RTMP_GET_PACKET_MGMT_PKT(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_MCAST_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

#ifdef ETH_CONVERT_SUPPORT

	/* If we are running as Ethernet Converter, update MAT DataBase and duplicant the packet if necessary.*/
	if ((pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE) &&
		((pTxBlk->TxFrameType != TX_MCAST_FRAME) &&
		 (pTxBlk->TxFrameType != TX_MLME_DATAQ_FRAME) &&
		 (pTxBlk->TxFrameType != TX_MLME_MGMTQ_FRAME))) {
		if (!NdisEqualMemory(pAd->CurrentAddress, (GET_OS_PKT_DATAPTR(pPacket) + 6), MAC_ADDR_LEN)) {
			TX_BLK_SET_FLAG(pTxBlk, fTX_bDonglePkt);
			MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): Dongle Packet)\n", __func__);
		}

		/* For each tx packet, update our MAT convert engine databases.*/
		convertPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, 0, pTxBlk->pMacEntry->EntryType);

		if (convertPkt) {
			pPacket = convertPkt;
			pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
			pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
			pTxBlk->pPacket = convertPkt;
		}

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
			pSrcBufVA = GET_OS_PKT_DATAPTR(pTxBlk->pPacket);
			NdisMoveMemory(pSrcBufVA + 6, wdev->if_addr, MAC_ADDR_LEN);
		}
	}

#endif /* ETH_CONVERT_SUPPORT */
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	return TRUE;
}

INT sta_mlme_mgmtq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	HTTRANSMIT_SETTING *transmit;
	UCHAR MlmeRate;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	PSTA_ADMIN_CONFIG sta_cfg = NULL;
	struct dev_rate_info *rate;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR PID, tx_rate;
	UINT16 wcid;
	UCHAR prot = 0;
	UCHAR apidx = 0;
	MAC_TX_INFO mac_info;
	UCHAR *tmac_info;
	PHEADER_802_11 pHeader_802_11;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev is NULL!\n");
		return NDIS_STATUS_FAILURE;
	}

	sta_fill_offload_tx_blk(pAd, wdev, tx_blk);
	tmac_info = tx_blk->pSrcBufHeader;
	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

	/* Verify Mlme rate for a / g bands.*/
	if ((wdev->channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		MlmeRate = RATE_6;

	rate = &wdev->rate;

	/* Fixed W52 with Activity scan issue in ABG_MIXED and ABGN_MIXED mode.*/
	/* TODO: shiang-6590, why we need this condition check here? */
	if (WMODE_EQUAL(wdev->PhyMode, WMODE_A | WMODE_B | WMODE_G)
		|| WMODE_CAP(wdev->PhyMode, WMODE_A | WMODE_B | WMODE_G | WMODE_AN | WMODE_GN)
#ifdef DOT11_VHT_AC
		|| WMODE_CAP(wdev->PhyMode, WMODE_AC)
#endif /* DOT11_VHT_AC*/
	   ) {
		if (pAd->LatchRfRegs.Channel > 14) {
			rate->MlmeTransmit.field.MODE = MODE_OFDM;
			rate->MlmeTransmit.field.MCS = MCS_RATE_6;
		} else {
			rate->MlmeTransmit.field.MODE = MODE_CCK;
			rate->MlmeTransmit.field.MCS = MCS_0;
		}
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;

	/*
	 *	In WMM-UAPSD, mlme frame should be set psm as power saving but probe
	 *	request frame, Data-Null packets alse pass through MMRequest in RT2860,
	 *	however, we hope control the psm bit to pass APSD
	 */
	sta_cfg = GetStaCfgByWdev(pAd, wdev);

	if ((pHeader_802_11->FC.SubType == SUBTYPE_ACTION) ||
		(pHeader_802_11->FC.SubType == SUBTYPE_PS_POLL) ||
		((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
		 ((pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL) ||
		  (pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL)))) {
		if (RtmpPktPmBitCheck(pAd, sta_cfg) == TRUE)
			pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
		else if (STA_STATUS_TEST_FLAG(sta_cfg, fSTA_STATUS_MEDIA_STATE_CONNECTED) &&
				 INFRA_ON(sta_cfg) &&
				 RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
			/* We are in scan progress, just let the PwrMgmt bit keep as it orginally should be */
		} else
			pHeader_802_11->FC.PwrMgmt = pAd->CommonCfg.bAPSDForcePowerSave;
	}

	bInsertTimestamp = FALSE;
	pMacEntry = MacTableLookup2(pAd, pHeader_802_11->Addr1, wdev);

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) { /* must be PS-POLL*/
		/*Set PM bit in ps-poll, to fix WLK 1.2  PowerSaveMode_ext failure issue.*/
		if ((pAd->OpMode == OPMODE_STA) && (pHeader_802_11->FC.SubType == SUBTYPE_PS_POLL))
			pHeader_802_11->FC.PwrMgmt = PWR_SAVE;

		bAckRequired = FALSE;
#ifdef VHT_TXBF_SUPPORT

		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA) {
			pHeader_802_11->Duration = 100;
		}

#endif /* VHT_TXBF_SUPPORT */
	} else /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/ {
		if (pHeader_802_11->Addr1[0] & 0x01) { /* MULTICAST, BROADCAST*/
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		} else {
#ifdef SOFT_SOUNDING

			if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
				&& pMacEntry && (pMacEntry->snd_reqired == TRUE)) {
				bAckRequired = FALSE;
				pHeader_802_11->Duration = 0;
			} else
#endif /* SOFT_SOUNDING */
			{
				bAckRequired = TRUE;
				pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);

				if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
					bInsertTimestamp = TRUE;
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Response*/
				} else if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Request*/
				} else if ((pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH) &&
						   (pMacEntry == NULL)) {
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Deauth */
				}
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;

	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	PID = PID_MGMT;
#ifdef DOT11W_PMF_SUPPORT
	PMF_PerformTxFrameAction(pAd, pHeader_802_11, tx_blk->SrcBufLen, tx_hw_hdr_len, &prot);
#endif

	if (pMacEntry == NULL) {
		wcid = 0;
		tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
		transmit = &rate->MlmeTransmit;
#ifdef VHT_TXBF_SUPPORT

		if (pAd->NDPA_Request) {
			transmit->field.MODE = MODE_VHT;
			transmit->field.MCS = MCS_RATE_6;
		}

#endif
	} else {
		wcid = pMacEntry->wcid;
		tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
		transmit = &rate->MlmeTransmit;
	}

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	if (prot)
		mac_info.prot = prot;

	if (prot == 2 || prot == 3)
		mac_info.bss_idx = apidx;

	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = bInsertTimestamp;
	mac_info.AMPDU = FALSE;
	mac_info.Ack = bAckRequired;
	mac_info.BM = IS_BM_MAC_ADDR(pHeader_802_11->Addr1);
	mac_info.NSeq = FALSE;
	mac_info.BASize = 0;
	mac_info.WCID = wcid;
	mac_info.Type = pHeader_802_11->FC.Type;
	mac_info.SubType = pHeader_802_11->FC.SubType;
	mac_info.txpwr_offset = 0;
#ifdef CONFIG_PM_BIT_HW_MODE
	/* For  MT STA LP control, use H/W control mode for PM bit */
	mac_info.PsmBySw = 0;
#else
	mac_info.PsmBySw = 1;
#endif
	/* check if the pkt is Tmr frame. */
	mac_info.Length = (tx_blk->SrcBufLen - tx_hw_hdr_len);

	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;

		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;

		mac_info.txpwr_offset = wdev->mgmt_txd_txpwr_offset;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_DATA_NULL:
			mac_info.hdr_len = 24;
			tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
			transmit = &rate->MlmeTransmit;
			break;

		case SUBTYPE_QOS_NULL:
			mac_info.hdr_len = 26;
			tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
			transmit = &rate->MlmeTransmit;
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType);
			hex_dump("DataFrame", (char *)pHeader_802_11, 24);
			break;
		}

		if (pMacEntry && tr_ctl->tr_entry[wcid].PsDeQWaitCnt)
			PID = PID_PS_DATA;

		mac_info.WCID = wcid;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_PS_POLL:
			mac_info.hdr_len = sizeof(PSPOLL_FRAME);
			tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
			transmit = &rate->MlmeTransmit;
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType);
			break;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "FIXME!!!Unexpected frame send to MgmtRing, need to assign the length!\n");
	}

	mac_info.PID = PID;
	mac_info.TID = 0;
	mac_info.TxRate = tx_rate;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
	mac_info.wmm_set = HcGetWmmIdx(pAd, wdev);
	mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
	mac_info.OmacIdx = wdev->OmacIdx;

	MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s(): %d, WMMSET=%d,QId=%d\n",
			  __func__, __LINE__, mac_info.wmm_set, mac_info.q_idx);

	/* PCI use Miniport to send NULL frame and need to add NULL frame TxS control here to enter PSM */
#ifdef SOFT_SOUNDING

	if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
		&& pMacEntry && (pMacEntry->snd_reqired == TRUE)) {
		tx_rate = (UCHAR)pMacEntry->snd_rate.field.MCS;
		transmit = &pMacEntry->snd_rate;
		mac_info.Txopmode = IFS_PIFS;
		pMacEntry->snd_reqired = FALSE;
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Kick Sounding to "MACSTR", dataRate(PhyMode:%s, BW:%sHz, %dSS, MCS%d)\n",
				 MAC2STR(pMacEntry->Addr),
				 get_phymode_str(transmit->field.MODE),
				 get_bw_str(transmit->field.BW),
				 (transmit->field.MCS >> 4) + 1, (transmit->field.MCS & 0xf));
	} else
#endif /* SOFT_SOUNDING */
	{
		mac_info.Txopmode = IFS_BACKOFF;
	}

	/* if we are going to send out FTM action. enable CR to report TMR report.*/
	if ((pAd->pTmrCtrlStruct != NULL) && (pAd->pTmrCtrlStruct->TmrEnable != TMR_DISABLE)) {
		if (mac_info.IsTmr == TRUE) {
			pAd->pTmrCtrlStruct->TmrState = SEND_OUT;
		}
	}

	if (wdev) {
		if ((!WMODE_CAP_2G(wdev->PhyMode)) || (wdev->channel > 14)) {
			if (transmit->field.MODE == MODE_CCK) {
				/*
				    something wrong with rate->MlmeTransmit
					correct with OFDM mode
				*/
				transmit->field.MODE = MODE_OFDM;
				MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "@@@@ FIXME!! frame(Type=%x, SubType=%x) use the CCK RATE but wdev support A band only, mac_info.Length=%lu, mac_info.wmm_set=%d, mac_info.q_idx=%d, mac_info.OmacIdx=%d\n",
						 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType, mac_info.Length, mac_info.wmm_set, mac_info.q_idx, mac_info.OmacIdx);
			}
		}
	}
	return asic_mlme_hw_tx(pAd, tmac_info, &mac_info, transmit, tx_blk);
}

INT sta_mlme_dataq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	UCHAR *tmac_info, *frm_buf;
	UINT frm_len;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PHEADER_802_11 pHeader_802_11;
	PFRAME_BAR pBar = NULL;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR MlmeRate, tx_rate;
	UINT16 hw_wcid, wcid;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	HTTRANSMIT_SETTING *transmit, tmp_transmit, TransmitSetting;
	MAC_TX_INFO mac_info;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct DOT11_H *pDot11h = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev is NULL!\n");
		return NDIS_STATUS_FAILURE;
	}

	sta_fill_offload_tx_blk(pAd, wdev, tx_blk);
	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
	pMacEntry = MacTableLookup2(pAd, pHeader_802_11->Addr1, wdev);
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(pStaCfg);

	/*
		copy to local var first to prevernt the dev->rate.MlmeTransmit is change this moment
	*/
	NdisMoveMemory(&tmp_transmit, &wdev->rate.MlmeTransmit, sizeof(HTTRANSMIT_SETTING));
	transmit = &tmp_transmit;
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return FALSE;

	frm_buf = tx_blk->pSrcBufHeader;
	frm_len = tx_blk->SrcBufLen;
	tmac_info = tx_blk->pSrcBufHeader;

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

	/* Verify Mlme rate for a/g bands.*/
	if ((wdev->channel > 14) && (MlmeRate < RATE_6)) { /* 11A band*/
		MlmeRate = RATE_6;
		transmit->field.MCS = MCS_RATE_6;
		transmit->field.MODE = MODE_OFDM;
	}

	/*
	 *	Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
	 *	Snice it's been set to 0 while on MgtMacHeaderInit
	 *	By the way this will cause frame to be send on PWR_SAVE failed.
	 */

	/* In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame */
	/* Data-Null packets alse pass through MMRequest in RT2860, however, we hope control the psm bit to pass APSD*/
	if (pHeader_802_11->FC.Type != FC_TYPE_DATA) {
		if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) ||
			!(pStaCfg->wdev.UapsdInfo.bAPSDCapable && pAd->CommonCfg.APEdcaParm[0].bAPSDCapable))
			pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;
		else
			pHeader_802_11->FC.PwrMgmt = pAd->CommonCfg.bAPSDForcePowerSave;
	}

	bInsertTimestamp = FALSE;

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		if (pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ) {
			pBar = (PFRAME_BAR)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
			bAckRequired = TRUE;
		} else
			bAckRequired = FALSE;

#ifdef VHT_TXBF_SUPPORT

		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA) {
			UINT len = tx_blk->SrcBufLen - TXINFO_SIZE - cap->TXWISize - TSO_SIZE;

			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, len);
		}
#endif /* VHT_TXBF_SUPPORT*/
	} else { /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
		if (pHeader_802_11->Addr1[0] & 0x01) { /* MULTICAST, BROADCAST */
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		} else {
			bAckRequired = TRUE;
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);

			if (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) {
				bInsertTimestamp = TRUE;
				bAckRequired = FALSE;
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;

	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	/* Before radar detection done, mgmt frame can not be sent but probe req*/
	/* Because we need to use probe req to trigger driver to send probe req in passive scan*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pDot11h->RDMode != RD_NORMAL_MODE)) {
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "MlmeHardTransmit --> radar detect not in normal mode !!!\n");
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	/*
	 *	Fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
	 *	should always has only one ohysical buffer, and the whole frame size equals
	 *	to the first scatter buffer size
	 *
	 *	Initialize TX Descriptor
	 *	For inter-frame gap, the number is for this frame and next frame
	 *	For MLME rate, we will fix as 2Mb to match other vendor's implement
	 */

	/* management frame doesn't need encryption. so use WCID_NO_MATCHED no matter u are sending to specific wcid or not */
	/* Only beacon use Nseq=TRUE. So here we use Nseq=FALSE.*/
	if (pMacEntry == NULL) {
		if (((pHeader_802_11->FC.Type == FC_TYPE_CNTL) && (pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ)) ||
			((pHeader_802_11->FC.Type == FC_TYPE_MGMT) && (pHeader_802_11->FC.SubType == SUBTYPE_ACTION))) {
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}

		wcid = WCID_INVALID;

		if (wdev) {
#ifdef SW_CONNECT_SUPPORT
			GET_GroupKey_WCID(wdev, hw_wcid);
			wcid = wdev->tr_tb_idx;
#else /* SW_CONNECT_SUPPORT */
			GET_GroupKey_WCID(wdev, wcid);
			hw_wcid = wcid;
#endif /* !SW_CONNECT_SUPPORT */
			if (!IS_WCID_VALID(pAd, wcid)) {
				if ((wdev->wdev_type == WDEV_TYPE_AP) ||
					(wdev->wdev_type == WDEV_TYPE_GO) ||
					(wdev->wdev_type == WDEV_TYPE_ADHOC)) {
					RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
					return NDIS_STATUS_FAILURE;
				} else if ((wdev->wdev_type == WDEV_TYPE_STA) ||
						   (wdev->wdev_type == WDEV_TYPE_GC)) {
					/*because sta role has no bssinfo before linkup with rootap. use a temp idx here.*/
					wcid = 0;
				}
			}
		}
	} else {
		wcid = pMacEntry->wcid;
		hw_wcid = pMacEntry->wcid;
	}

	tx_rate = (UCHAR)transmit->field.MCS;
	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = bInsertTimestamp;
	mac_info.AMPDU = FALSE;
	mac_info.BM = IS_BM_MAC_ADDR(pHeader_802_11->Addr1);
	mac_info.Ack = bAckRequired;
	mac_info.NSeq = FALSE;
	mac_info.BASize = 0;
	mac_info.WCID = hw_wcid;
	mac_info.TID = 0;
	mac_info.wmm_set = HcGetWmmIdx(pAd, wdev);
	mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
	mac_info.txpwr_offset = 0;
	mac_info.OmacIdx = wdev->OmacIdx;

	mac_info.Type = pHeader_802_11->FC.Type;
	mac_info.SubType = pHeader_802_11->FC.SubType;
	mac_info.Length = (tx_blk->SrcBufLen - tx_hw_hdr_len);

	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;

		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;

		mac_info.PID = PID_MGMT;
#ifdef DOT11W_PMF_SUPPORT
		PMF_PerformTxFrameAction(pAd, pHeader_802_11, tx_blk->SrcBufLen, tx_hw_hdr_len, &mac_info.prot);
#endif
		if (IS_ASIC_CAP(pAd, fASIC_CAP_ADDBA_HW_SSN)) {
			if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) {
				PFRAME_ACTION_HDR act_hdr = (PFRAME_ACTION_HDR)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

				if (act_hdr->Category == CATEGORY_BA && act_hdr->Action == ADDBA_REQ) {
					PFRAME_ADDBA_REQ addba_frame = (PFRAME_ADDBA_REQ)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
#ifdef RT_BIG_ENDIAN
					BA_PARM tempBaParm;
					NdisMoveMemory((PUCHAR)(&tempBaParm), (PUCHAR)(&addba_frame->BaParm),
						sizeof(BA_PARM));
					*(USHORT *)(&tempBaParm) = le2cpu16(*(USHORT *)(&tempBaParm));
					mac_info.TID = tempBaParm.TID;
					mac_info.q_idx = WMM_UP2AC_MAP[tempBaParm.TID];
#else
					mac_info.TID = addba_frame->BaParm.TID;
					mac_info.q_idx = WMM_UP2AC_MAP[addba_frame->BaParm.TID];
#endif
					mac_info.addba = TRUE;
				}
			}
		}
		mac_info.txpwr_offset = wdev->mgmt_txd_txpwr_offset;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_DATA_NULL:
			mac_info.hdr_len = 24;
			tx_rate = (UCHAR)transmit->field.MCS;
			break;

		case SUBTYPE_QOS_NULL:
			mac_info.hdr_len = 26;
			tx_rate = (UCHAR)transmit->field.MCS;
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType);
			hex_dump("DataFrame", frm_buf, frm_len);
			break;
		}

		mac_info.WCID = hw_wcid;

		if (pMacEntry && tr_ctl->tr_entry[wcid].PsDeQWaitCnt)
			mac_info.PID = PID_PS_DATA;
		else
			mac_info.PID = PID_MGMT;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_BLOCK_ACK_REQ:
			mac_info.PID = PID_CTL_BAR;
			mac_info.hdr_len = 16;
			mac_info.SpeEn = 0;
			mac_info.TID = pBar->BarControl.TID;

			if (wdev->channel > 14) {
				/* 5G */
				TransmitSetting.field.MODE = MODE_OFDM;
			} else {
				/* 2.4G */
				TransmitSetting.field.MODE = MODE_CCK;
			}

			TransmitSetting.field.BW = BW_20;
			TransmitSetting.field.STBC = 0;
			TransmitSetting.field.ShortGI = 0;
			TransmitSetting.field.MCS = 0;
			TransmitSetting.field.ldpc = 0;
			transmit = &TransmitSetting;
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					 "FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					  pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType);
			hex_dump("Control Frame", frm_buf, frm_len);
			break;
		}
	}

	mac_info.TxRate = tx_rate;
	mac_info.Txopmode = IFS_BACKOFF;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;

	/* PCI use Miniport to send NULL frame and need to add NULL frame TxS control here to enter PSM */
	if (wdev) {
		if ((!WMODE_CAP_2G(wdev->PhyMode)) || (wdev->channel > 14)) {
			if (transmit->field.MODE == MODE_CCK) {
				/*
				    something wrong with rate->MlmeTransmit
					correct with OFDM mode
				*/
				transmit->field.MODE = MODE_OFDM;
				MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "@@@@ FIXME!! frame(Type=%x, SubType=%x) use the CCK RATE but wdev support A band only, mac_info.Length=%lu, mac_info.wmm_set=%d, mac_info.q_idx=%d, mac_info.OmacIdx=%d\n",
						 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType, mac_info.Length, mac_info.wmm_set, mac_info.q_idx, mac_info.OmacIdx);
			}
		}
	}

	return asic_mlme_hw_tx(pAd, tmac_info, &mac_info, transmit, tx_blk);
}

INT sta_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PQUEUE_ENTRY pQEntry;
	PNDIS_PACKET pPacket;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (!pTxBlk->pPacket) {
		return NDIS_STATUS_FAILURE;
	}

	if (!pStaCfg) {
		if (pTxBlk->TxFrameType == TX_AMSDU_FRAME) {
			while (pTxBlk->TxPacketList.Head) {
				pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
				pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

				if (pPacket)
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}
		} else {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		}

		return NDIS_STATUS_FAILURE;
	}

#ifdef CFG_TDLS_SUPPORT

	if (IS_ENTRY_TDLS(pTxBlk->pMacEntry))
		pTxBlk->Pid = PID_TDLS;

#endif /* CFG_TDLS_SUPPORT */

	switch (pTxBlk->TxFrameType) {
	case TX_AMPDU_FRAME:
		ret = ops->ampdu_tx(pAd, wdev, pTxBlk);
		break;

	case TX_LEGACY_FRAME:
		ret = ops->legacy_tx(pAd, wdev, pTxBlk);
		break;

	case TX_MCAST_FRAME:
		ret = ops->legacy_tx(pAd, wdev, pTxBlk);
		break;

	case TX_AMSDU_FRAME:
		ret = ops->amsdu_tx(pAd, wdev, pTxBlk);
		break;

	case TX_FRAG_FRAME:
		ret = ops->frag_tx(pAd, wdev, pTxBlk);
		break;

	case TX_MLME_MGMTQ_FRAME:
		ret = ops->mlme_mgmtq_tx(pAd, wdev, pTxBlk);
		break;

	case TX_MLME_DATAQ_FRAME:
		ret = ops->mlme_dataq_tx(pAd, wdev, pTxBlk);
		break;
#ifdef CONFIG_ATE
	case TX_ATE_FRAME:
		ret = ops->ate_tx(pAd, wdev, pTxBlk);
		break;
#endif
#ifdef VERIFICATION_MODE
	case TX_VERIFY_FRAME:
		ret = ops->verify_tx(pAd, wdev, pTxBlk);
		break;
#endif

	default:
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Send a pacekt was not classified!!\n");
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);

		return NDIS_STATUS_FAILURE;
	}

	return ret;
}

/*
 *	========================================================================
 *	Routine Description:
 *	Arguments:
 *		pAd	Pointer to our adapter
 *
 *	IRQL = DISPATCH_LEVEL
 *
 *	========================================================================
 */
VOID RTMPHandleTwakeupInterrupt(
	IN PRTMP_ADAPTER pAd)
{
	AsicWakeup(pAd, FALSE, &pAd->StaCfg[0]);
}

BOOLEAN sta_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	INT i, ii;
	INT op_mode = OPMODE_STA;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
#if defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	/* CFG_TODO */
	op_mode = pRxBlk->OpMode;
#endif /* RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */

	if (pEntry) {
		struct wifi_dev *wdev = NULL;
		PSTA_ADMIN_CONFIG pStaCfg = NULL;

		wdev = wdev_search_by_wcid(pAd, pEntry->wcid);

		if (wdev) {
			pStaCfg = GetStaCfgByWdev(pAd, wdev);

			if (pStaCfg) {
				if (INFRA_ON(pStaCfg)) {
					/* check if need to resend PS Poll when received packet with MoreData = 1 */
					if ((RtmpPktPmBitCheck(pAd, pStaCfg) == TRUE) && (FC->MoreData == 1)) {
						/* for UAPSD, all management frames will be VO priority */
						if (pAd->CommonCfg.bAPSDAC_VO == 0) {
							/* non-UAPSD delivery-enabled AC */
							RTMP_PS_POLL_ENQUEUE(pAd, pStaCfg);
						}
					}
				}
			}
		}
	}

	/* TODO: if MoreData == 0, station can go to sleep */

	/* We should collect RSSI not only U2M data but also my beacon */
	for (i = 0; i < pAd->MSTANum; i++) {
		if (!pAd->StaCfg[i].wdev.DevInfo.WdevActive)
			continue;

		if ((FC->SubType == SUBTYPE_BEACON)
			&& (MAC_ADDR_EQUAL(&pAd->StaCfg[i].Bssid, &pRxBlk->Addr2))) {
			if (pAd->RxAnt.EvaluatePeriod == 0) {
				pAd->StaCfg[i].wdev.LastSNR0 = (UCHAR) (pRxBlk->rx_signal.raw_snr[0]);
				pAd->StaCfg[i].wdev.LastSNR1 = (UCHAR) (pRxBlk->rx_signal.raw_snr[1]);
			}
		}
	}

	if (pEntry && (FC->SubType == SUBTYPE_ACTION)) {
		/* only PM bit of ACTION frame can be set */
		if (((op_mode == OPMODE_AP) && IS_ENTRY_CLIENT(pEntry)) ||
			((op_mode == OPMODE_STA) && (IS_ENTRY_TDLS(pEntry))))
			RtmpPsIndicate(pAd, pRxBlk->Addr2, pRxBlk->wcid, FC->PwrMgmt);

		/*
		 *	In IEEE802.11, 11.2.1.1 STA Power Management modes,
		 *	The Power Managment bit shall not be set in any management
		 *	frame, except an Action frame.
		 *
		 *	In IEEE802.11e, 11.2.1.4 Power management with APSD,
		 *	If there is no unscheduled SP in progress, the unscheduled SP
		 *	begins when the QAP receives a trigger frame from a non-AP QSTA,
		 *	which is a QoS data or QoS Null frame associated with an AC the
		 *	STA has configured to be trigger-enabled.
		 *	So a management action frame is not trigger frame.
		 */
	}

	if (pEntry) {
		if (pEntry->wdev)
			/* Signal in MLME_QUEUE isn't used, therefore take this item to save min SNR. */
			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
							FC,
							pRxBlk->DataSize,
							pRxBlk->rx_signal.raw_rssi[0],
							pRxBlk->rx_signal.raw_rssi[1],
							pRxBlk->rx_signal.raw_rssi[2],
							pRxBlk->rx_signal.raw_rssi[3],
							min(pRxBlk->rx_signal.raw_snr[0],
							pRxBlk->rx_signal.raw_snr[1]),
							pRxBlk->channel_freq,
							op_mode,
							pEntry->wdev,
							pRxBlk->rx_rate.field.MODE);
		else
			ASSERT(0);
	} else {
		/* Pat: TODO. Dispatch managment frame by its frequency */
		for (ii = 0; ii < pAd->MSTANum; ii++) {
			if (pAd->StaCfg[ii].wdev.DevInfo.WdevActive)
				/* Signal in MLME_QUEUE isn't used, therefore take this item to save min SNR. */
				REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
								FC,
								pRxBlk->DataSize,
								pRxBlk->rx_signal.raw_rssi[0],
								pRxBlk->rx_signal.raw_rssi[1],
								pRxBlk->rx_signal.raw_rssi[2],
								pRxBlk->rx_signal.raw_rssi[3],
								min(pRxBlk->rx_signal.raw_snr[0],
								pRxBlk->rx_signal.raw_snr[1]),
								pRxBlk->channel_freq,
								op_mode,
								&pAd->StaCfg[ii].wdev,
								pRxBlk->rx_rate.field.MODE);
		}
	}

	return TRUE;
}

struct wifi_dev_ops sta_wdev_ops = {
	.tx_pkt_allowed = sta_tx_pkt_allowed,
	.fp_tx_pkt_allowed = sta_tx_pkt_allowed,
	.send_data_pkt = sta_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = sta_send_mlme_pkt,
	.tx_pkt_handle = sta_tx_pkt_handle,
	.legacy_tx = sta_legacy_tx,
	.ampdu_tx = sta_ampdu_tx,
	.frag_tx = sta_frag_tx,
	.amsdu_tx = sta_amsdu_tx,
	.fill_non_offload_tx_blk = sta_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = sta_fill_offload_tx_blk,
	.mlme_mgmtq_tx = sta_mlme_mgmtq_tx,
	.mlme_dataq_tx = sta_mlme_dataq_tx,
#ifdef CONFIG_ATE
	.ate_tx = mt_ate_tx,
#endif
#ifdef VERIFICATION_MODE
	.verify_tx = verify_pkt_tx,
#endif
	.ieee_802_11_data_tx = sta_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = sta_ieee_802_3_data_tx,
	.rx_pkt_foward = sta_rx_fwd_hnd,
	.rx_pkt_allowed = sta_rx_pkt_allow,
	.ieee_802_3_data_rx = sta_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = sta_ieee_802_11_data_rx,
	.find_cipher_algorithm = sta_find_cipher_algorithm,
	.mac_entry_lookup = sta_mac_entry_lookup,
	.media_state_connected = sta_media_state_connected,
	.ioctl = rt28xx_sta_ioctl,
	.open = sta_inf_open,
	.close = sta_inf_close,
	.linkup = wifi_sys_linkup,
	.linkdown = wifi_sys_linkdown,
	.conn_act = wifi_sys_conn_act,
	.disconn_act = wifi_sys_disconn_act,
};
