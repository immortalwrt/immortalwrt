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
	cmm_data.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */


#include "rt_config.h"
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
#endif

#ifdef IXIA_C50_MODE
#ifdef MT7986
#include "chip/mt7986_cr.h"
#endif
#ifdef MT7916
#include "chip/mt7916_cr.h"
#endif
#ifdef MT7981
#include "chip/mt7981_cr.h"
#endif
#endif

#ifdef MAP_R3
#include "map.h"
#endif

UCHAR	SNAP_802_1H[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
UCHAR	SNAP_BRIDGE_TUNNEL[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};
UCHAR	EAPOL[] = {0x88, 0x8e};
UCHAR   TPID[] = {0x81, 0x00}; /* VLAN related */

UCHAR	IPX[] = {0x81, 0x37};
UCHAR	APPLE_TALK[] = {0x80, 0xf3};

/* UserPriority To AccessCategory mapping */
UCHAR WMM_UP2AC_MAP[8] = {QID_AC_BE, QID_AC_BK,
						  QID_AC_BK, QID_AC_BE,
						  QID_AC_VI, QID_AC_VI,
						  QID_AC_VO, QID_AC_VO
						 };

#if defined(VLAN_SUPPORT) || defined(MAP_TS_TRAFFIC_SUPPORT)
VOID update_rxblk_addr(RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *) pRxBlk->FC;

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		if ((FC->ToDs == 0) && (FC->FrDs == 0)) {
			pRxBlk->Addr1 = pRxBlk->pData;
			pRxBlk->Addr2 = pRxBlk->pData + 6;
		} else if ((FC->ToDs == 0) && (FC->FrDs == 1)) {
			pRxBlk->Addr1 = pRxBlk->pData;
			pRxBlk->Addr3 = pRxBlk->pData + 6;
		} else if ((FC->ToDs == 1) && (FC->FrDs == 0)) {
			pRxBlk->Addr2 = pRxBlk->pData + 6;
			pRxBlk->Addr3 = pRxBlk->pData;
		} else {
			pRxBlk->Addr2 = pRxBlk->FC + 2;
			pRxBlk->Addr3 = pRxBlk->pData;
			pRxBlk->Addr4 = pRxBlk->pData + 6;
		}
	} else {
		pRxBlk->Addr1 = pRxBlk->FC + 4; /*4 byte Frame Control*/
		pRxBlk->Addr2 = pRxBlk->FC + 4 + MAC_ADDR_LEN;
		pRxBlk->Addr3 = pRxBlk->FC + 4 + MAC_ADDR_LEN * 2;
		if ((FC->ToDs == 1) && (FC->FrDs == 1))
			pRxBlk->Addr4 = pRxBlk->FC + 6 + MAC_ADDR_LEN * 3;
	}

}
VOID remove_vlan_hw_padding(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	UCHAR byte0 = 0, byte1 = 0, extra_field_offset = 0;

	/*For IOT, remove unused fields*/
	if (pRxBlk->pRxPacket) {
		extra_field_offset = IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket)) ? (LENGTH_802_3_NO_TYPE+LENGTH_802_1Q) : LENGTH_802_3_NO_TYPE;
		/*Remove the extra field (2 Bytes) which is added by hardware*/
		/*The added info is the length of actual data (without overhead)*/
		byte0 = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket)[extra_field_offset];
		byte1 = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket)[extra_field_offset+1];
		/*If there is the extra field, remove it*/
		if (((byte0<<8) | byte1) == GET_OS_PKT_LEN(pRxBlk->pRxPacket) - extra_field_offset - 2) {/*2 : len of extra field for VLAN packet and untaged VLAN packet*/
			NdisMoveMemory(GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket) + 2, GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), extra_field_offset);
			RtmpOsSkbPullRcsum(pRxBlk->pRxPacket, 2);
			RtmpOsSkbResetNetworkHeader(pRxBlk->pRxPacket);
			RtmpOsSkbResetTransportHeader(pRxBlk->pRxPacket);
			RtmpOsSkbResetMacLen(pRxBlk->pRxPacket);
			pRxBlk->pData = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
			pRxBlk->DataSize -= 2;
			update_rxblk_addr(pRxBlk);
		}
		/*End of remove extra field*/
	}
}

VOID remove_vlan_tag(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UCHAR *pSrcBuf;
	struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	NdisMoveMemory(GET_OS_PKT_DATAPTR(pkt) + LENGTH_802_1Q, GET_OS_PKT_DATAPTR(pkt), LENGTH_802_3_NO_TYPE);
	RtmpOsSkbPullRcsum(RTPKT_TO_OSPKT(pkt), LENGTH_802_1Q);
	RtmpOsSkbResetMacHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetNetworkHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetTransportHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetMacLen(RTPKT_TO_OSPKT(pkt));
	skb->vlan_tci = 0;
}
VOID remove_80211_frame_vlan_tag(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, UCHAR offset)
{
	UCHAR *pSrcBuf;
	struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	NdisMoveMemory(GET_OS_PKT_DATAPTR(pkt) + LENGTH_802_1Q, GET_OS_PKT_DATAPTR(pkt), offset);
	RtmpOsSkbPullRcsum(RTPKT_TO_OSPKT(pkt), LENGTH_802_1Q);
	RtmpOsSkbResetMacHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetNetworkHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetTransportHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetMacLen(RTPKT_TO_OSPKT(pkt));
	skb->vlan_tci = 0;
}

VOID rebuild_802_11_eapol_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{

	HEADER_802_11 buf;
	HEADER_802_11 *pHhdr80211 = &buf;
	UCHAR hdr_len;
	UCHAR *pData;
	char addr4[6];

	NdisZeroMemory(pHhdr80211, sizeof(HEADER_802_11));
	pHhdr80211->FC = *((FRAME_CONTROL *) pRxBlk->FC);
	COPY_MAC_ADDR(pHhdr80211->Addr1, pRxBlk->Addr1);
	COPY_MAC_ADDR(pHhdr80211->Addr2, pRxBlk->Addr2);
	COPY_MAC_ADDR(pHhdr80211->Addr3, pRxBlk->Addr3);

	if (pHhdr80211->FC.FrDs == 1 && pHhdr80211->FC.ToDs == 1) {
		COPY_MAC_ADDR(addr4, pRxBlk->Addr4);
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	} else
		hdr_len = LENGTH_802_11;

	if (pHhdr80211->FC.SubType & 0x08) /*Is QoS Packet*/
		hdr_len += LENGTH_WMMQOS_H + 6; /*QoS Control & SNAP*/
	else
		hdr_len += 6; /*SNAP*/

	skb_push(pRxBlk->pRxPacket, hdr_len - LENGTH_802_3_NO_TYPE);
	RtmpOsSkbResetNetworkHeader(pRxBlk->pRxPacket);
	RtmpOsSkbResetTransportHeader(pRxBlk->pRxPacket);
	RtmpOsSkbResetMacLen(pRxBlk->pRxPacket);

	pRxBlk->pData = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
	pRxBlk->DataSize += (hdr_len - LENGTH_802_3_NO_TYPE);
	pRxBlk->FC = pRxBlk->pData;

	NdisZeroMemory(GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), hdr_len);
	pData = pRxBlk->pData;
	NdisMoveMemory(pData, pHhdr80211, sizeof(HEADER_802_11));
	pData += sizeof(HEADER_802_11);

	if (pHhdr80211->FC.FrDs == 1 && pHhdr80211->FC.ToDs == 1) {
		NdisMoveMemory(pData, addr4, 6);
		pData += 6; /*addr4*/
	}

	/*QoS Control : 2 byte*/
	if (pHhdr80211->FC.SubType & 0x08) {
		*pData = pRxBlk->UserPriority;
		pData += LENGTH_WMMQOS_H;
	}
	NdisMoveMemory(pData, SNAP_802_1H, 6);
	RX_BLK_CLEAR_FLAG(pRxBlk, fRX_HDR_TRANS);
	update_rxblk_addr(pRxBlk);
}

static inline VOID Sniff2BytesFromSrcBuffer(PNDIS_BUFFER buf, UCHAR offset, UCHAR *p0, UCHAR *p1)
{
	UCHAR *ptr = (UCHAR *)(buf + offset);
	*p0 = *ptr;
	*p1 = *(ptr + 1);
}

#ifdef VLAN_SUPPORT
static inline VOID RtmpOsRemoveVLANTag(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UCHAR *pSrcBuf;
	UINT16 VLAN_LEN = 4;
	UCHAR extra_field_offset = 2 * ETH_ALEN;

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	memmove(GET_OS_PKT_DATAPTR(pkt) + VLAN_LEN, GET_OS_PKT_DATAPTR(pkt), extra_field_offset);
	RtmpOsSkbPullRcsum(RTPKT_TO_OSPKT(pkt), 4);
	RtmpOsSkbResetMacHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetNetworkHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetTransportHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetMacLen(RTPKT_TO_OSPKT(pkt));
}

BOOLEAN check_copy_pkt_needed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	UCHAR *pSrcBuf;
	UINT16 TypeLen;
	BOOLEAN ret = FALSE;
	USHORT vlan_id, vlan_pcp;

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

	/*skip the Ethernet Header*/
	pSrcBuf += LENGTH_802_3;

	vlan_id = *(USHORT *)pSrcBuf;

	vlan_id = cpu2be16(vlan_id);
	vlan_pcp = (vlan_id & (~MASK_CLEAR_TCI_PCP)) >> 13;
	vlan_id = vlan_id & MASK_TCI_VID; /* 12 bit */

	if (TypeLen == ETH_TYPE_VLAN) {
		if (wdev->bVLAN_Tag == FALSE) {
			ret = TRUE;
		} else if (wdev->VLAN_VID != 0) {
			if (vlan_id != wdev->VLAN_VID) {
				switch (wdev->VLAN_Policy[TX_VLAN]) {
				case VLAN_TX_REPLACE_VID:
				case VLAN_TX_REPLACE_ALL:
					ret = TRUE;
					break;
				default:
					ret = FALSE;
				}
			} else if (vlan_pcp != wdev->VLAN_Priority) {
				ret = TRUE;
			}
		}
	} else if (wdev->bVLAN_Tag && wdev->VLAN_Policy[TX_VLAN] != VLAN_TX_ALLOW) {
		ret = TRUE;
	}

	return ret;
}

PNDIS_PACKET RtmpVlanPktCopy(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	struct sk_buff *skb = NULL;
	PNDIS_PACKET pkt_copy = NULL;
	struct net_device *net_dev = wdev->if_dev;

	/*why change skb_unshare to skb_copy:*/
	/*if skb_copy allocate skb buffer fail, skb_unshare still kfree_skb(old_pkt) and return NULL.*/
	/*the old pkt may be free double times.*/
	skb = skb_copy(RTPKT_TO_OSPKT(pkt), GFP_ATOMIC);

	if (skb) {
		skb->dev = net_dev;
		pkt_copy = OSPKT_TO_RTPKT(skb);
		RTMP_SET_PACKET_WDEV(pkt_copy, wdev->wdev_idx);
		RTMP_SET_PACKET_WCID(pkt_copy, MAX_LEN_OF_MAC_TABLE);
		/*if copy skb success, should free the old skb.*/
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	}

	return pkt_copy;
}

static int ap_fp_tx_pkt_vlan_tag_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET *pPkt)
{
	UCHAR *pSrcBuf;
	UINT16 TypeLen, DblVlan = 0;
	UCHAR Byte0, Byte1;
	PACKET_INFO pkt_info;
	UCHAR *pkt_va;
	UINT pkt_len;
	PNDIS_PACKET pkt = NULL, pkt_copy = NULL;
	BOOLEAN need_copy;
	struct sk_buff *skb = (struct sk_buff *)(*pPkt);

	pkt = *pPkt;
	RTMP_QueryPacketInfo(pkt, &pkt_info, &pkt_va, &pkt_len);
	if ((!pkt_va) || (pkt_len <= 14))
		return FALSE;

	if (skb_cloned(skb)) {
		need_copy = check_copy_pkt_needed(pAd, wdev, pkt);
		if (need_copy) {
			pkt_copy = RtmpVlanPktCopy(pAd, wdev, pkt);
			if (pkt_copy == NULL) {
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "%s():copy packet fail!!\n", __func__);
				return FALSE;
			}
			/*can't release packet here.*/
			*pPkt = pkt_copy;
			pkt = *pPkt;
			RTMP_QueryPacketInfo(pkt, &pkt_info, &pkt_va, &pkt_len);
			if ((!pkt_va) || (pkt_len <= 14))
				return FALSE;
		}
	}

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	if (TypeLen != ETH_TYPE_VLAN) {
		RTMP_SET_PACKET_VLAN(pkt, FALSE);
		RTMP_SET_PACKET_PROTOCOL(pkt, TypeLen);
	} else
		DblVlan = (pSrcBuf[16] << 8) | pSrcBuf[17];
#if !defined(MBSS_AS_WDS_AP_SUPPORT) && !defined(APCLI_AS_WDS_STA_SUPPORT)
	/*insert 802.1Q tag if required*/
	if (pAd->CommonCfg.bEnableVlan && wdev->bVLAN_Tag && (TypeLen != ETH_TYPE_VLAN)
		&& (wdev->VLAN_Policy[TX_VLAN] != VLAN_TX_ALLOW) && (TypeLen != ETH_TYPE_EAPOL)) {
		UINT16 tci = (wdev->VLAN_Priority<<(CFI_LEN + VID_LEN)) | wdev->VLAN_VID; /*CFI = 0*/

		pkt = RtmpOsVLANInsertTag(pkt, tci);
		if (pkt == NULL) {
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s():vlan insert tag fail!!\n", __func__);
			return FALSE;
		}
		skb = (struct sk_buff *)pkt;
		skb->vlan_tci = tci;
		pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
		ASSERT(pSrcBuf);
		TypeLen = ETH_TYPE_VLAN;
	}
#endif

	/*skip the Ethernet Header*/
	pSrcBuf += LENGTH_802_3;

	if (TypeLen == ETH_TYPE_VLAN) {
#ifdef CONFIG_AP_SUPPORT
		/*
		*	802.3 VLAN packets format:
		*
		*	DstMAC(6B) + SrcMAC(6B)
		*	+ 802.1Q Tag Type (2B = 0x8100) + Tag Control Information (2-bytes)
		*	+ Length/Type(2B)
		*	+ data payload (42-1500 bytes)
		*	+ FCS(4B)
		*
		*	VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID
		*/

		/* If it is not my vlan pkt, no matter unicast or multicast, deal with it according to the policy*/
		if (wdev->VLAN_VID != 0) {

			USHORT vlan_id = *(USHORT *)pSrcBuf;

			vlan_id = cpu2be16(vlan_id);
			vlan_id = vlan_id & MASK_TCI_VID; /* 12 bit */

			if (vlan_id != wdev->VLAN_VID) {
				switch (wdev->VLAN_Policy[TX_VLAN]) {
				case VLAN_TX_DROP:
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "Drop the packet\n");
					wdev->VLANTxdrop++;
					return FALSE;
				case VLAN_TX_ALLOW:
				case VLAN_TX_KEEP_TAG:
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "Allow the packet\n");
					break;
				case VLAN_TX_REPLACE_VID:
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "Replace the packet VLAN ID\n");
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_VID);
					*(USHORT *)pSrcBuf |= be2cpu16(wdev->VLAN_VID);
					break;
				case VLAN_TX_REPLACE_ALL:
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "Replace the packet VLAN Tag\n");
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_PCP);
					*(USHORT *)pSrcBuf |= be2cpu16((wdev->VLAN_Priority)<<(CFI_LEN + VID_LEN));
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_VID);
					*(USHORT *)pSrcBuf |= be2cpu16(wdev->VLAN_VID);
					break;
				default:
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Unexpected checking policy\n");
					return FALSE;
				}
			} else {
				/* align PCP*/
				*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_PCP);
				*(USHORT *)pSrcBuf |= be2cpu16((wdev->VLAN_Priority)<<(CFI_LEN + VID_LEN));
			}
		}
		/*TxPath Efficiency consideration:DoubleVLAN driver untag 1st,SingleVlan MDP untag 1st.*/
		if ((wdev->bVLAN_Tag == FALSE)
			&& (DblVlan == ETH_TYPE_VLAN)) {
			remove_vlan_tag(pAd, pkt);
			RTMP_SET_PACKET_VLAN(pkt, FALSE);
			pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
			ASSERT(pSrcBuf);
			TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
			RTMP_SET_PACKET_PROTOCOL(pkt, TypeLen);
		} else {
			USHORT vlan_pcp = *(USHORT *)pSrcBuf;
			vlan_pcp = cpu2be16(vlan_pcp);
			vlan_pcp = (((vlan_pcp) & (~MASK_CLEAR_TCI_PCP)) >> 13); /* 3 bit */

			RTMP_SET_VLAN_PCP(pkt, vlan_pcp);
			RTMP_SET_PACKET_VLAN(pkt, TRUE);
			Sniff2BytesFromSrcBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			RTMP_SET_PACKET_PROTOCOL(pkt, TypeLen);
		}

#endif /* CONFIG_AP_SUPPORT */
	}

	return TRUE;
}
#endif /*VLAN_SUPPORT*/
#endif /*defined(VLAN_SUPPORT) || defined(MAP_R2) */

#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
UCHAR	P2POUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x9}; /* spec. 1.14 OUI */
#endif /* defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) */

#ifdef DBG_DIAGNOSE
VOID dbg_diag_deque_log(RTMP_ADAPTER *pAd)
{
	struct dbg_diag_info *diag_info;
#ifdef DBG_TXQ_DEPTH
	UCHAR QueIdx = 0;
#endif
	diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
#ifdef DBG_TXQ_DEPTH
	if ((pAd->DiagStruct.diag_cond & DIAG_COND_TXQ_DEPTH) != DIAG_COND_TXQ_DEPTH) {
		if ((pAd->DiagStruct.wcid > 0) && IS_TR_WCID_VALID(pAd, pAd->DiagStruct.wcid)) {
			STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pAd->DiagStruct.wcid];
			UCHAR swq_cnt;

			for (QueIdx = 0; QueIdx < 4; QueIdx++) {
				if (tr_entry->tx_queue[QueIdx].Number <= 7)
					swq_cnt = tr_entry->tx_queue[QueIdx].Number;
				else
					swq_cnt = 8;

				diag_info->TxSWQueCnt[QueIdx][swq_cnt]++;
			}
		}
	}

#endif /* DBG_TXQ_DEPTH */
}
#endif /* DBG_DIAGNOSE */

VOID dump_rxblk(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MTWF_PRINT("Dump RX_BLK Structure:\n");
	MTWF_PRINT("\tHW rx info:\n");
	hex_dump("RawData", &pRxBlk->hw_rx_info[0], RXD_SIZE);
	MTWF_PRINT("\tData Pointer info:\n");

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_PRINT("\t\trmac_info=0x%p\n", pRxBlk->rmac_info);
		asic_dump_rmac_info(pAd, pRxBlk->rmac_info);
	}

	MTWF_PRINT("\t\tpRxPacket=0x%p, MPDUtotalByteCnt=%d\n", pRxBlk->pRxPacket, pRxBlk->MPDUtotalByteCnt);
	MTWF_PRINT("\t\tpData=0x%p\n", pRxBlk->pData);
	MTWF_PRINT("\t\tDataSize=%d\n", pRxBlk->DataSize);
	MTWF_PRINT("\t\tFlags=0x%x\n", pRxBlk->Flags);
	MTWF_PRINT("\t\tUserPriority=%d\n", pRxBlk->UserPriority);
	MTWF_PRINT("\t\tOpMode=%d\n", pRxBlk->OpMode);
	MTWF_PRINT("\tMirror Info from RMAC Info:\n");
	MTWF_PRINT("\t\tWCID=%d\n", pRxBlk->wcid);
	MTWF_PRINT("\t\tTID=%d\n", pRxBlk->TID);
	MTWF_PRINT("\t\tKey_idx=%d\n", pRxBlk->key_idx);
	MTWF_PRINT("\t\tBSS_IDX=%d\n", pRxBlk->bss_idx);
	hex_dump("Dump RxPacket in dump_rxblk", (UCHAR *)pRxBlk->pData, pRxBlk->MPDUtotalByteCnt > 512 ? 512 : pRxBlk->MPDUtotalByteCnt);
}


VOID dump_txblk(RTMP_ADAPTER *pAd, TX_BLK *txblk)
{
	/* hex_dump("TxBlk Raw Data", (UCHAR *)txblk, sizeof(TX_BLK)); */
	MTWF_PRINT("TxBlk Info\n");
	MTWF_PRINT("\twdev=%p\n", txblk->wdev);
	MTWF_PRINT("\tWCID=%d\n", txblk->Wcid);
	MTWF_PRINT("\tWMM_Idx=%d\n", txblk->WmmIdx);
	MTWF_PRINT("\tQueIdx=%d\n", txblk->QueIdx);
	MTWF_PRINT("\tWMM_Set=%d\n", txblk->wmm_set);
	MTWF_PRINT("\tpMacEntry=%p\n", txblk->pMacEntry);

	if (txblk->pMacEntry) {
		MTWF_PRINT("\t\tpMacEntry->wcid=%d\n", txblk->pMacEntry->wcid);
		MTWF_PRINT("\t\tpMacEntry->tr_tb_idx=%d\n", txblk->pMacEntry->tr_tb_idx);
	}

	MTWF_PRINT("\tTR_Entry=%p\n", txblk->tr_entry);

	if (txblk->tr_entry)
		MTWF_PRINT("\t\tTR_Entry->wcid=%d\n", txblk->tr_entry->wcid);

	MTWF_PRINT("\tOpMode=%d\n", txblk->OpMode);
	MTWF_PRINT("\tTxFrameType=%d\n", txblk->TxFrameType);
	MTWF_PRINT("\tTotalFragNum=%d\n", txblk->TotalFragNum);
	MTWF_PRINT("\tUserPriority=%d\n", txblk->UserPriority);
}


#ifdef MT_MAC

static VOID ParseRxVStat_v2(RTMP_ADAPTER *pAd, RX_STATISTIC_RXV *rx_stat, UCHAR *Data)
{
	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = NULL;
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = NULL;
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = NULL;
	RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;
	INT16 foe = 0;
	UINT32 i = 0;
	UINT8 cbw = 0;
	UINT32 foe_const = 0;

	RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
	RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
	RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(Data + 16);
	RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 20);
	RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(Data + 24);
	RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(Data + 28);
	RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
	RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
	RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);

	if (RXV1_1ST_CYCLE->TxMode == MODE_CCK) {
		foe = (RXV1_5TH_CYCLE->MISC1 & 0x7ff);
		foe = (foe * 1000) >> 11;
	} else {
		cbw = RXV1_1ST_CYCLE->FrMode;
		foe_const = ((1 << (cbw + 1)) & 0xf) * 10000;
		foe = (RXV1_5TH_CYCLE->MISC1 & 0xfff);

		if (foe >= 2048)
			foe = foe - 4096;

		foe = (foe * foe_const) >> 15;
	}

	rx_stat->FreqOffsetFromRx[0] = foe;
	rx_stat->RCPI[0] = RXV1_4TH_CYCLE->RCPI0;
	rx_stat->RCPI[1] = RXV1_4TH_CYCLE->RCPI1;
	rx_stat->RCPI[2] = RXV1_4TH_CYCLE->RCPI2;
	rx_stat->RCPI[3] = RXV1_4TH_CYCLE->RCPI3;
	rx_stat->FAGC_RSSI_IB[0] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[0] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->FAGC_RSSI_IB[1] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[1] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->FAGC_RSSI_IB[2] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[2] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->FAGC_RSSI_IB[3] = RXV1_3TH_CYCLE->IBRssiRx;
	rx_stat->FAGC_RSSI_WB[3] = RXV1_3TH_CYCLE->WBRssiRx;
	rx_stat->SNR[0] = (RXV1_5TH_CYCLE->MISC1 >> 19) - 16;

	for (i = 0; i < 4; i++) {
		if (rx_stat->FAGC_RSSI_IB[i] >= 128)
			rx_stat->FAGC_RSSI_IB[i] -= 256;

		if (rx_stat->FAGC_RSSI_WB[i] >= 128)
			rx_stat->FAGC_RSSI_WB[i] -= 256;
	}
}

VOID parse_RXV_packet_v2(RTMP_ADAPTER *pAd, UINT32 Type, RX_BLK *RxBlk, UCHAR *Data)
{
	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
	BOOLEAN parse_rssi = TRUE;

	if (Type == RMAC_RX_PKT_TYPE_RX_NORMAL) {
		RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)Data;
		RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 4);

#if defined(WIFI_DIAG) && defined(MT7663)
		/* get SNR from RXV6 */
		if (VALID_UCAST_ENTRY_WCID(pAd, RxBlk->wcid))
			diag_get_snr(pAd, RxBlk->wcid, Data + 20);
#endif
		if (((FRAME_CONTROL *)RxBlk->FC)->Type == FC_TYPE_DATA) {
			parse_rssi = FALSE;
#ifdef AIR_MONITOR
			if (pAd->MntEnable[RxBlk->band] && IS_WCID_VALID(pAd, RxBlk->wcid)) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[RxBlk->wcid];

				if (IS_ENTRY_MONITOR(pEntry))
					parse_rssi = TRUE;
			}
#endif /* AIR_MONITOR */
		}
	} else if (Type == RMAC_RX_PKT_TYPE_RX_TXRXV) {
		RXV_DWORD0 *DW0 = NULL;
		RXV_DWORD1 *DW1 = NULL;
		RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
		RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
		RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;
		RX_STATISTIC_RXV *rx_stat = &pAd->rx_stat_rxv[DBDC_BAND0];

		DW0 = (RXV_DWORD0 *)Data;
		DW1 = (RXV_DWORD1 *)(Data + 4);
		RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
		RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
		RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
		RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
		RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);
		RxBlk->rxv2_cyc1 = *(UINT32 *)RXV2_1ST_CYCLE;
		RxBlk->rxv2_cyc2 = *(UINT32 *)RXV2_2ND_CYCLE;
		RxBlk->rxv2_cyc3 = *(UINT32 *)RXV2_3TH_CYCLE;
		pAd->rxv2_cyc3[(DW1->RxvSn % 10)] = RxBlk->rxv2_cyc3;

		if (pAd->parse_rxv_stat_enable)
			ParseRxVStat_v2(pAd, rx_stat, Data);

#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			MT_ATEUpdateRxStatistic(pAd, TESTMODE_RXV, Data);

#endif /* CONFIG_ATE */
	} else {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s(): invalid Type %u\n", __func__, Type);
		return; /* return here to avoid dereferencing NULL pointer below */
	}

#ifdef TXRX_STAT_SUPPORT
	if ((parse_rssi == TRUE) || (pAd->TXRX_EnableReadRssi && (parse_rssi == FALSE)))
#else
	if (parse_rssi)
#endif
	{
		RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 12);
		RxBlk->rx_signal.raw_rssi[0] = (RXV1_4TH_CYCLE->RCPI0 - 220) / 2;
		RxBlk->rx_signal.raw_rssi[1] = (RXV1_4TH_CYCLE->RCPI1 - 220) / 2;
		RxBlk->rx_signal.raw_rssi[2] = (RXV1_4TH_CYCLE->RCPI2 - 220) / 2;
		RxBlk->rx_signal.raw_rssi[3] = (RXV1_4TH_CYCLE->RCPI3 - 220) / 2;
	}

#ifdef AUTOMATION
	if (pAd->auto_dvt &&
		pAd->auto_dvt->rxv.enable &&
		(Type == RMAC_RX_PKT_TYPE_RX_NORMAL) &&
		(((FRAME_CONTROL *)RxBlk->FC)->Type == FC_TYPE_DATA) &&
		(((FRAME_CONTROL *)RxBlk->FC)->SubType != SUBTYPE_DATA_NULL) &&
		OS_NTOHS(*((UINT16 *)((PUCHAR)(RxBlk->pData + 12)))) != ETH_P_ARP)
		rxv_correct_test(Data);
#endif /* AUTOMATION */

	RxBlk->rx_rate.field.MODE = RXV1_1ST_CYCLE->TxMode;
	RxBlk->rx_rate.field.MCS = RXV1_1ST_CYCLE->TxRate;
	RxBlk->rx_rate.field.ldpc = RXV1_1ST_CYCLE->HtAdCode;
	RxBlk->rx_rate.field.BW = RXV1_1ST_CYCLE->FrMode;
	RxBlk->rx_rate.field.STBC = RXV1_1ST_CYCLE->HtStbc ? 1 : 0;
	RxBlk->rx_rate.field.ShortGI = RXV1_1ST_CYCLE->HtShortGi;

	if ((RxBlk->rx_rate.field.MODE >= MODE_VHT) && (RxBlk->rx_rate.field.STBC == 0))
		RxBlk->rx_rate.field.MCS |= ((RXV1_2ND_CYCLE->NstsField & 0x3) << 4);


}

/* this function ONLY if not allow pn replay attack and drop packet */
static BOOLEAN check_rx_pkt_pn_allowed(RTMP_ADAPTER *pAd, RX_BLK *rx_blk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	BOOLEAN isAllow = TRUE;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)rx_blk->FC;

	if (pFmeCtrl->Wep == 0)
		return TRUE;

	if (!VALID_UCAST_ENTRY_WCID(pAd, rx_blk->wcid))
		return TRUE;

	pEntry = &pAd->MacTab.Content[rx_blk->wcid];

	if (!pEntry || !pEntry->wdev)
		return TRUE;

	if (rx_blk->key_idx > 3) {
		return TRUE;
	}

	if (rx_blk->pRxInfo->Mcast || rx_blk->pRxInfo->Bcast) {
		UINT32 GroupCipher = pEntry->SecConfig.GroupCipher;
		UCHAR key_idx = rx_blk->key_idx;

		if (IS_CIPHER_CCMP128(GroupCipher) || IS_CIPHER_CCMP256(GroupCipher) ||
			IS_CIPHER_GCMP128(GroupCipher) || IS_CIPHER_GCMP256(GroupCipher) ||
			IS_CIPHER_TKIP(GroupCipher)) {

			if (pEntry->Init_CCMP_BC_PN_Passed[key_idx] == FALSE) {
					if (rx_blk->CCMP_PN >= pEntry->CCMP_BC_PN[key_idx]) {
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BC, %s (%d)(%d) OK: 1st come-in the %llu and now is %llu\n",
							__FUNCTION__, pEntry->wcid, key_idx, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[key_idx]);
						pEntry->CCMP_BC_PN[key_idx] = rx_blk->CCMP_PN;
						pEntry->Init_CCMP_BC_PN_Passed[key_idx] = TRUE;
					} else {
					        MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "BC, %s (%d)(%d) Reject: 1st come-in the %llu and now is %llu\n",
						__FUNCTION__, pEntry->wcid, key_idx, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[key_idx]);
						isAllow = FALSE;
					}
			} else {
				if (rx_blk->CCMP_PN <= pEntry->CCMP_BC_PN[key_idx]) {
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "BC, %s (%d)(%d) Reject: come-in the %llu and now is %llu\n",
						__FUNCTION__, pEntry->wcid, key_idx, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[key_idx]);
					isAllow = FALSE;
				} else {
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BC, %s (%d)(%d) OK: come-in the %llu and now is %llu\n",
						__FUNCTION__, pEntry->wcid, key_idx, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[key_idx]);
					pEntry->CCMP_BC_PN[key_idx] = rx_blk->CCMP_PN;
				}
			}
		}
	}
#ifdef PN_UC_REPLAY_DETECTION_SUPPORT
	else {
		UCHAR TID = rx_blk->TID;
		if (rx_blk->CCMP_PN < pEntry->CCMP_UC_PN[TID]) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"UC, %s (%d) Reject: come-in the %llu and now is %llu\n",
				__FUNCTION__, pEntry->wcid, rx_blk->CCMP_PN, pEntry->CCMP_UC_PN[TID]);
			isAllow = FALSE;
		} else {
			pEntry->CCMP_UC_PN[TID] = rx_blk->CCMP_PN;
		}
	}
#endif /* PN_UC_REPLAY_DETECTION_SUPPORT */

	return isAllow;
}

#define NUM_TYPE_STRING 8
UCHAR *rx_pkt_type_string[NUM_TYPE_STRING] = {
	"RMAC_RX_PKT_TYPE_RX_TXS", "RMAC_RX_PKT_TYPE_RX_TXRXV", "RMAC_RX_PKT_TYPE_RX_NORMAL",
	"RMAC_RX_PKT_TYPE_RX_DUP_RFB", "RMAC_RX_PKT_TYPE_RX_TMR", "Undefine Type 0x5",
	"Undefine Type 0x6", "RMAC_RX_PKT_TYPE_RX_EVENT"
};

static UINT32 rx_data_handler(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet)
{

	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
		RELEASE_NDIS_PACKET(pAd, rx_packet, NDIS_STATUS_SUCCESS);
		return NDIS_STATUS_FAILURE;
	}
#ifdef SNIFFER_RADIOTAP_SUPPORT
		if (MONITOR_ON(pAd)) {
			if (asic_trans_rxd_into_radiotap(pAd, rx_packet, rx_blk) == NDIS_STATUS_SUCCESS)
				announce_802_11_radiotap_packet(pAd, rx_packet, rx_blk);
			else
				RELEASE_NDIS_PACKET(pAd, rx_packet, NDIS_STATUS_SUCCESS);

			return NDIS_STATUS_SUCCESS;
		}
#endif

	if (asic_trans_rxd_into_rxblk(pAd, rx_blk, rx_packet) == 0) {
		RELEASE_NDIS_PACKET(pAd, rx_packet, NDIS_STATUS_SUCCESS);
		return NDIS_STATUS_FAILURE;
	}


#ifdef CONFIG_CSO_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_CSO)) {
		NdisCopyMemory(&(rx_blk->rCso), &(pAd->rCso), sizeof(RX_CSO_STRUCT));
	}

#endif

#ifdef IXIA_C50_MODE
	if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(rx_packet))) {
		pAd->rx_cnt.rx_from_hw[smp_processor_id()]++;

		if (pAd->rx_cnt.rx_pkt_len != GET_OS_PKT_LEN(rx_packet))
			pAd->rx_cnt.rx_pkt_len = GET_OS_PKT_LEN(rx_packet);

		pAd->rx_cnt.rxpktdetect++;
	}
#endif

	if (header_packet_process(pAd, rx_packet, rx_blk) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	rx_packet_process(pAd, rx_packet, rx_blk);
	return NDIS_STATUS_SUCCESS;
}

BOOLEAN tmr_handler(RTMP_ADAPTER *ad, RX_BLK *rx_blk, VOID *rx_packet)
{
	TMR_FRM_STRUC *tmr = (TMR_FRM_STRUC *)(rx_packet);
	/*Tmr pkt comes to Host directly, there are some minor calculation need to do.*/
	TmrReportParser(ad, tmr, FALSE, 0); /* TMRv1.0 TOAE calibration result need to leverage EXT_EVENT_ID_TMR_CAL */
	return TRUE;
}

UINT32 pkt_alloc_fail_handle(RTMP_ADAPTER *ad, PNDIS_PACKET rx_packet, UINT8 resource_idx)
{
	UINT32 rx_pkt_type;

	rx_pkt_type = asic_get_packet_type(ad, rx_packet);

	switch (rx_pkt_type) {
#ifdef CUT_THROUGH

	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
		asic_txdone_handle(ad, rx_packet, resource_idx);
		break;
#endif

	default:
		break;
	}

	return 0;
}

/*
*
*/
INT call_traffic_notifieriers(INT val, struct _RTMP_ADAPTER *ad, void *v)
{
	struct tx_rx_ctl *tr_ctrl = &ad->tr_ctl;
	struct traffic_notify_info info;
	INT ret;

	/*fill wsys notify info*/
	info.ad = ad;
	info.v = v;
	/*traversal caller for traffic notify chain*/
	ret = mt_notify_call_chain(&tr_ctrl->traffic_notify_head, val, &info);
	return ret;
}

/*
*
*/
static INT call_traffic_rx_notifierieres(struct _RTMP_ADAPTER *ad, UINT rx_pkt_type, NDIS_PACKET *rx_packet)
{
	switch (rx_pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_NORMAL:
	case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
#ifdef TRAFFIC_NOTIFY
		call_traffic_notifieriers(TRAFFIC_NOTIFY_RX_DATA, ad, rx_packet);
#endif /*TRAFFIC_NOTIFY*/
		break;
	case RMAC_RX_PKT_TYPE_RX_TXRXV:
		call_traffic_notifieriers(TRAFFIC_NOTIFY_RX_TXRXV, ad, rx_packet);
		break;
	case RMAC_RX_PKT_TYPE_RX_TXS:
		call_traffic_notifieriers(TRAFFIC_NOTIFY_RX_TXS, ad, rx_packet);
		break;
	case RMAC_RX_PKT_TYPE_RX_EVENT:
		call_traffic_notifieriers(TRAFFIC_NOTIFY_RX_EVENT, ad, rx_packet);
		break;
	case RMAC_RX_PKT_TYPE_RX_TMR:
		call_traffic_notifieriers(TRAFFIC_NOTIFY_RX_TMR, ad, rx_packet);
		break;
#ifdef CUT_THROUGH
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
		call_traffic_notifieriers(TRAFFIC_NOTIFY_RX_TXRX_NOTIFY, ad, rx_packet);
		break;
#endif /* CUT_THROUGH */
	default:
		break;
	}
	return 0;
}

/*export function*/
/*
*
*/
INT register_traffic_notifier(struct _RTMP_ADAPTER *ad, struct notify_entry *ne)
{
	INT ret;
	struct tx_rx_ctl *tr_ctrl = &ad->tr_ctl;

	ret = mt_notify_chain_register(&tr_ctrl->traffic_notify_head, ne);

	return ret;
}

/*
*
*/
INT unregister_traffic_notifier(struct _RTMP_ADAPTER *ad, struct notify_entry *ne)
{
	INT ret;
	struct tx_rx_ctl *tr_ctrl = &ad->tr_ctl;

	ret = mt_notify_chain_unregister(&tr_ctrl->traffic_notify_head, ne);
	return ret;
}

/*
*
*/
UINT32 mt_rx_pkt_process(RTMP_ADAPTER *pAd, UINT8 resource_idx, RX_BLK *rx_blk, PNDIS_PACKET rx_packet)
{
	UINT32 rx_pkt_type;
	enum resource_attr res_attr = hif_get_resource_type(pAd->hdev_ctrl, resource_idx);

	if (res_attr == HIF_RX_DATA)
		rx_pkt_type = asic_get_packet_type(pAd, GET_OS_PKT_DATAPTR(rx_packet));
	else {
		rx_pkt_type = asic_get_packet_type(pAd, rx_packet);

		/* rx data is from event ring, convert it to skb */
		if (rx_pkt_type == RMAC_RX_PKT_TYPE_RX_NORMAL || rx_pkt_type == RMAC_RX_PKT_TYPE_RX_DUP_RFB) {
			PNDIS_PACKET pkt = NULL;

			DEV_ALLOC_SKB(pkt, RX1_BUFFER_SIZE);

			if (pkt) {
				NdisCopyMemory(OS_PKT_TAIL_BUF_EXTEND(pkt, RX1_BUFFER_SIZE), rx_packet, RX1_BUFFER_SIZE);
				rx_packet = pkt;
				hif_free_rx_buf(pAd->hdev_ctrl, resource_idx);
			} else {
				hif_free_rx_buf(pAd->hdev_ctrl, resource_idx);
				return NDIS_STATUS_SUCCESS;

			}
		}
	}

	/*notify to interesting feature, only enable when DVT mode enable*/
	call_traffic_rx_notifierieres(pAd, rx_pkt_type, rx_packet);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s, rx_pkt_type(%d)\n", __func__, rx_pkt_type);

	switch (rx_pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_NORMAL:
	case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
		rx_data_handler(pAd, rx_blk, rx_packet);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
#ifdef CONFIG_ICS_FRAME_HANDLE
		if (pAd->rxvIcs == 0)
			asic_rxv_handler(pAd, rx_blk, rx_packet);
#else
		asic_rxv_handler(pAd, rx_blk, rx_packet);
#endif /* CONFIG_ICS_FRAME_HANDLE */
		hif_free_rx_buf(pAd->hdev_ctrl, resource_idx);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXS:
		chip_txs_handler(pAd, rx_packet);
		hif_free_rx_buf(pAd->hdev_ctrl, resource_idx);
		break;

	case RMAC_RX_PKT_TYPE_RX_EVENT:
		asic_rx_event_handler(pAd, rx_packet);
		RX_BLK_SET_FLAG(rx_blk, fRX_CMD_RSP);
		hif_free_rx_buf(pAd->hdev_ctrl, resource_idx);
		break;

	case RMAC_RX_PKT_TYPE_RX_TMR:
		/* tmr_handler(pAd, rx_blk, rx_packet); */
		hif_free_rx_buf(pAd->hdev_ctrl, resource_idx);
		break;

#ifdef CUT_THROUGH
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
#ifdef WFDMA_WED_COMPATIBLE
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY_V0:
#endif /*WFDMA_WED_COMPATIBLE*/
		asic_txdone_handle(pAd, rx_packet, resource_idx);
		hif_free_rx_buf(pAd->hdev_ctrl, resource_idx);
		break;
#endif /* CUT_THROUGH */

	case RMAC_RX_PKT_TYPE_RX_ICS:
#ifdef CONFIG_ICS_FRAME_HANDLE
		if (pAd->rxvIcs)
			chip_rx_ics_handler(pAd, DBG_LOG_PKT_TYPE_ICS, rx_packet, 0);
#endif /* CONFIG_ICS_FRAME_HANDLE */
#ifdef FW_LOG_DUMP
		dbg_log_wrapper(pAd, DBG_LOG_PKT_TYPE_ICS, rx_packet, 0);
#endif /* FW_LOG_DUMP */
		RELEASE_NDIS_PACKET(pAd, rx_packet, NDIS_STATUS_SUCCESS);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s():Invalid PktType:%d (res_attr:%d)\n", __func__, rx_pkt_type, res_attr);
		if (res_attr != HIF_RX_DATA)
			hif_free_rx_buf(pAd->hdev_ctrl, resource_idx);
		else
			RELEASE_NDIS_PACKET(pAd, rx_packet, NDIS_STATUS_FAILURE);
		break;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* MT_MAC */

/*
	========================================================================

	Routine Description:
		API for MLME to transmit management frame to AP (BSS Mode)
	or station (IBSS Mode)

	Arguments:
		pAd Pointer to our adapter
		pData		Pointer to the outgoing 802.11 frame
		Length		Size of outgoing management frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS MiniportMMRequest(RTMP_ADAPTER *pAd, UCHAR QueIdx, UCHAR *pData, UINT Length)
{
	PNDIS_PACKET pkt;
	NDIS_STATUS Status;
	BOOLEAN bUseDataQ = FALSE, FlgDataQForce = FALSE;
	HEADER_802_11 *pHead = (HEADER_802_11 *)pData;
	struct wifi_dev *wdev = NULL;
	struct wifi_dev_ops *ops = NULL;
	INT hw_len;
	UCHAR hw_hdr[40];
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

#ifdef WF_RESET_SUPPORT
	if (pAd->wf_reset_in_progress == TRUE)
		return NDIS_STATUS_SUCCESS;
#endif

	hw_len = cap->tx_hw_hdr_len;
	ASSERT((sizeof(hw_hdr) > hw_len));
	NdisZeroMemory(&hw_hdr, hw_len);

	ASSERT(Length <= MAX_MGMT_PKT_LEN);

	Status = RTMPAllocateNdisPacket(pAd, &pkt, (UCHAR *)&hw_hdr[0], hw_len, pData, Length);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MiniportMMRequest (error:: can't allocate NDIS PACKET)\n");
		return NDIS_STATUS_FAILURE;
	}

	wdev = wdev_search_by_address(pAd, pHead->Addr2);

	if (wdev) {
		ops = wdev->wdev_ops;
	} else {
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		tr_cnt->tx_invalid_wdev++;
		return NDIS_STATUS_FAILURE;
	}


	if ((QueIdx & MGMT_USE_QUEUE_FLAG) == MGMT_USE_QUEUE_FLAG) {
		bUseDataQ = TRUE;
		QueIdx &= (~MGMT_USE_QUEUE_FLAG);
	}

	if (bUseDataQ)
		FlgDataQForce = TRUE;

#ifdef WIFI_DIAG
	diag_miniport_mm_request(pAd, pData, Length);
#endif

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
	if (wdev->wdev_type == WDEV_TYPE_STA)
		RTMPWakeUpWdev(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT || APCLI_SUPPORT */

	RTMP_SET_PACKET_TYPE(pkt, TX_MGMT);
	Status = send_mlme_pkt(pAd, pkt, wdev, QueIdx, bUseDataQ);

	return Status;
}
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
VOID send_dummy_data_packet(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry)
{
	UCHAR Header802_3[LENGTH_802_3];
	UCHAR EtherType[] = {0x08, 0x00};
	UCHAR Dummy_Data[14] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

	NdisZeroMemory(Header802_3, sizeof(UCHAR) * LENGTH_802_3);
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pEntry->wdev->bssid, EtherType);

	RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3, (PCHAR)&Dummy_Data, 14, TRUE);
	return;
}
#endif

#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function

	Arguments:
		pAd Pointer to our adapter
		pBuffer	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame
		FlgIsDeltsFrame 1: the frame is a DELTS frame

	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
void AP_QueuePsActionPacket(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN PNDIS_PACKET pPacket,
	IN BOOLEAN FlgIsDeltsFrame,
	IN BOOLEAN FlgIsLocked,
	IN UCHAR MgmtQid)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef UAPSD_SUPPORT
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
	PNDIS_PACKET DuplicatePkt = NULL;
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
#endif /* UAPSD_SUPPORT */
	STA_TR_ENTRY *tr_entry = &tr_ctl->tr_entry[pMacEntry->wcid];
	/* Note: for original mode of 4 AC are UAPSD, if station want to change
			the mode of a AC to legacy PS, we dont know where to put the
			response;
			1. send the response;
			2. but the station is in ps mode, so queue the response;
			3. we should queue the reponse to UAPSD queue because the station
				is not yet change its mode to legacy ps AC;
			4. so AP should change its mode to legacy ps AC only when the station
				sends a trigger frame and we send out the reponse;
			5. the mechanism is too complicate; */
#ifdef UAPSD_SUPPORT

	/*
		If the frame is action frame and the VO is UAPSD, we can not send the
		frame to VO queue, we need to send to legacy PS queue; or the frame
		maybe not got from QSTA.
	*/
	/*    if ((pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid]) &&*/
	/*		(FlgIsDeltsFrame == 0))*/
	if (pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid]) {
		/* queue the management frame to VO queue if VO is deliver-enabled */
		MTWF_DBG(pAd, DBG_CAT_PS, CATPS_UAPSD, DBG_LVL_INFO, "ps> mgmt to UAPSD queue %d ... (IsDelts: %d)\n",
				 MgmtQid, FlgIsDeltsFrame);
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY

		if (!pMacEntry->bAPSDAllAC) {
			/* duplicate one packet to legacy PS queue */
			RTMP_SET_PACKET_UAPSD(pPacket, 0, MgmtQid);
			DuplicatePkt = DuplicatePacket(wdev->if_dev, pPacket);
		} else
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		{
			RTMP_SET_PACKET_UAPSD(pPacket, 1, MgmtQid);
		}

		UAPSD_PacketEnqueue(pAd, pMacEntry, pPacket, MgmtQid, FALSE);

		if (pMacEntry->bAPSDAllAC) {
			/* mark corresponding TIM bit in outgoing BEACON frame*/
			WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);
		} else {
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
			/* duplicate one packet to legacy PS queue */

			/*
				Sometimes AP will send DELTS frame to STA but STA will not
				send any trigger frame to get the DELTS frame.
				We must force to send it so put another one in legacy PS
				queue.
			*/
			if (DuplicatePkt != NULL) {
				pPacket = DuplicatePkt;
				goto Label_Legacy_PS;
			}

#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		}
	} else
#endif /* UAPSD_SUPPORT */
	{
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
Label_Legacy_PS:
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ps> mgmt to legacy ps queue... (%d)\n", FlgIsDeltsFrame);

		if (tr_entry->ps_queue.Number >= MAX_PACKETS_IN_PS_QUEUE ||
			ge_enq_req(pAd, pPacket, MgmtQid, tr_entry, NULL) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "%s(%d): WLAN_TX_DROP, pPacket=%p, QueIdx=%d, ps_queue_num=%d, wcid=%d\n",
				 __func__, __LINE__, pPacket, MgmtQid, tr_entry->ps_queue.Number, tr_entry->wcid);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
			return;
		} else {
			/* mark corresponding TIM bit in outgoing BEACON frame*/
			WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);
		}
	}
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef PER_PKT_CTRL_FOR_CTMR
static BOOLEAN do_pkt_tmac_ctrl(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt,  TX_BLK *tx_blk)
{
	BOOLEAN ret = FALSE;

	/*get packet type by RTMP_GET_PACKET_xxx(pkt)
	*for example:
	*RTMP_GET_PACKET_DHCP(pkt)
	*RTMP_GET_PACKET_ARP(pkt)
	*RTMP_GET_PACKET_EAPOL(pkt)
	*RTMP_GET_PACKET_PING(pkt)
	*RTMP_GET_PACKET_WAI(pkt)
	*RTMP_GET_PACKET_TDLS_MMPDU(pkt)
	*RTMP_GET_PACKET_QOS_NULL(pkt)
	*RTMP_GET_PACKET_MGMT_PKT(pkt)
	*RTMP_GET_TDLS_SPECIFIC_PACKET(pkt)
	*.....
	*/

	if (0) {
		/*Fix rate for percific packets*/
		TRANSMIT_SETTING_HE *transmit;

		transmit = &tx_blk->TransmitHeByHost;
		tx_blk->ApplyFixRate = TRUE;

		/*
		*#define MODE_CCK 0
		*#define MODE_OFDM 1
		*#define MODE_HTMIX 2
		*#define MODE_HTGREENFIELD 3
		*#define MODE_VHT 4
		*#define MODE_HE 5
		*#define MODE_HE_SU	8
		*#define MODE_HE_24G 7
		*#define MODE_HE_5G 6
		*#define MODE_HE_EXT_SU	9
		*#define MODE_HE_TRIG	10
		*#define MODE_HE_MU	11
		*/
		transmit->field.MODE = MODE_OFDM;
		transmit->field.STBC = 0;/* 0:OFF; 1:ON */
		/*
		*	00: 1x HE_LTF + 0.8us GI
		*	01: 2x HT_LTF + 0.8us GI
		*	10: 2x HE_LTF + 1.6us GI
		*	11: 4x HE_LTF + 0.8us GI / 4x HE_LTF + 3.2us GI
		*/
		transmit->field.GILTF = 0;/*0~3*/
		/*
		*	BW_20	0
		*	BW_40	1
		*	BW_80	2
		*	BW_160	3
		*/
		transmit->field.BW = BW_20;/*0~3*/
		transmit->field.ldpc = 0;/*0~1, must be 1 in HE*/
		transmit->field.nss = 0;/*0~3, 1stream to 4 stream*/
		/*
		*MODE_CCK:
		*	TMI_TX_RATE_CCK_1M_LP	0,
		*	TMI_TX_RATE_CCK_2M_LP	1,
		*	TMI_TX_RATE_CCK_5M_LP	2,
		*	TMI_TX_RATE_CCK_11M_LP	3,
		*
		*MODE_CCK:
		*	TMI_TX_RATE_OFDM_6M		0,
		*	TMI_TX_RATE_OFDM_9M		1,
		*	TMI_TX_RATE_OFDM_12M	2,
		*	TMI_TX_RATE_OFDM_18M	3,
		*	TMI_TX_RATE_OFDM_24M	4,
		*	TMI_TX_RATE_OFDM_36M	5,
		*	TMI_TX_RATE_OFDM_48M	6,
		*	TMI_TX_RATE_OFDM_54M	7,
		*
		*MODE_HTMIX:
		*	MCS0~15;
		*
		*MODE_VHT:
		*	MCS0~9;
		*
		*MODE_HE:
		*	MCS0~11;
		*/
		transmit->field.MCS = 0;
		ret = TRUE;
	}


	if(0) {
		/*Modify retry limit or Time for percific packets*/
		tx_blk->ApplyRetryLimit = TRUE;

		tx_blk->RetryLimitByHost= 30;/*1~30*/
		tx_blk->MaxTxTimeByHost= 0;/*0~255; 0:neglect, unit: 1024us*/

		ret = TRUE;
	}

	if(0) {
		/*Modify TID for percific packets*/
		tx_blk->ApplyTid = TRUE;

		tx_blk->TidByHost= 5;/*0~7*/

		ret = TRUE;
	}

	if(0) {
		/*Modify BA/Power/Tx status for percific packets*/
		tx_blk->ApplyBaPowerTxs= TRUE;

		tx_blk->BaDisableByHost= 1;/*0~1, if disable BA*/

		/*
		*0~63, -8~+7.75dB; (Signed 6bit)
		*0 ~31 : 0~31
		*32~63 : -1~-32 (31 - pTxBlk->power_offset)
		*step is 0.25dB;
		*eg: pTxBlk->power_offset = 40, means that power offset is:
		*	(31-40) * 0.25 = -9 * 0.25 = -2.25dB
		*/
		tx_blk->PowerOffsetByHost = 0;

		tx_blk->TxS2HostByHost = 0;/* 0:OFF; 1:ON */
		/*Pid tag for packet need TX status;
		*Tx status can be checked in chip_txs_handler()
		*/
		tx_blk->PidByHost = 0x34;

	}

	if(ret)
		TX_BLK_SET_FLAG(tx_blk, fTX_CT_WithTxD);

	return ret;
}

static BOOLEAN is_amsdu_frame(RTMP_ADAPTER *pAd, NDIS_PACKET *pkt, TX_BLK *pTxBlk)
{
	int minLen = LENGTH_802_3;
	int wcid = RTMP_GET_PACKET_WCID(pkt);

	if (IS_ENTRY_MCAST(&pAd->MacTab.Content[wcid]))
		return FALSE;

	if (RTMP_GET_PACKET_DHCP(pkt) ||
		RTMP_GET_PACKET_ARP(pkt) ||
		RTMP_GET_PACKET_EAPOL(pkt) ||
		RTMP_GET_PACKET_PING(pkt) ||
		RTMP_GET_PACKET_WAI(pkt) ||
		RTMP_GET_PACKET_TDLS_MMPDU(pkt)
	   )
		return FALSE;

	/* Make sure the first packet has non-zero-length data payload */
	if (RTMP_GET_PACKET_VLAN(pkt))
		minLen += LENGTH_802_1Q; /* VLAN tag */
	else if (RTMP_GET_PACKET_LLCSNAP(pkt))
		minLen += 8; /* SNAP hdr Len*/

	if (minLen >= GET_OS_PKT_LEN(pkt))
		return FALSE;

	return TRUE;
}
#endif

UINT16 tx_pkt_classification(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, TX_BLK *tx_blk)
{
	UINT16 wcid;
	UCHAR TxFrameType = TX_LEGACY_FRAME;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR up = RTMP_GET_PACKET_UP(pkt);
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

#ifdef CONFIG_ATE
	if (RTMP_GET_PACKET_TXTYPE(pkt) == TX_ATE_FRAME)
		return TX_ATE_FRAME;
#endif /* CONFIG_ATE */

#ifdef VERIFICATION_MODE
	if (RTMP_GET_PACKET_TXTYPE(pkt) == TX_VERIFY_FRAME)
		return TX_VERIFY_FRAME;
#endif

	if (RTMP_GET_PACKET_TXTYPE(pkt) == TX_MCAST_FRAME)
		TxFrameType = TX_MCAST_FRAME;

	wcid = RTMP_GET_PACKET_WCID(pkt);
	tr_entry = &tr_ctl->tr_entry[wcid];

	if (VALID_UCAST_ENTRY_WCID(pAd, wcid))
		pMacEntry = &pAd->MacTab.Content[wcid];
	else {
		if (wcid > 0)
			pMacEntry = &pAd->MacTab.Content[wcid];
		else
			pMacEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];
	}

	tx_blk->pMacEntry = pMacEntry;

	if (RTMP_GET_PACKET_MGMT_PKT(pkt)) {
		if (RTMP_GET_PACKET_MGMT_PKT_DATA_QUE(pkt))
			return TX_MLME_DATAQ_FRAME;
		else
			return TX_MLME_MGMTQ_FRAME;
	}

	if ((RTMP_GET_PACKET_FRAGMENTS(pkt) > 1)
		&& ((pMacEntry->TXBAbitmap & (1 << up)) == 0))
		return TX_FRAG_FRAME;

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD) &&
		IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if (TxFrameType != TX_FRAG_FRAME)
			TX_BLK_SET_FLAG(tx_blk, fTX_MCU_OFFLOAD);
	}
#ifdef PER_PKT_CTRL_FOR_CTMR
	if (pAd->PerPktCtrlEnable)
		if(do_pkt_tmac_ctrl(pAd, pkt, tx_blk))
			TX_BLK_CLEAR_FLAG(tx_blk, fTX_MCU_OFFLOAD);
#endif

	/* Panther's IGMP snooping that implemented in driver
		need to fill full TXD. */
#ifdef IGMP_SNOOPING_NON_OFFLOAD
	if (RTMP_GET_PACKET_MCAST_CLONE(pkt)) {
		TX_BLK_CLEAR_FLAG(tx_blk, fTX_MCU_OFFLOAD);
	}
#endif

#ifdef SW_CONNECT_SUPPORT
	if (RTMP_GET_PACKET_SW(pkt)) {
		TX_BLK_CLEAR_FLAG(tx_blk, fTX_MCU_OFFLOAD);
	}
#endif /* SW_CONNECT_SUPPORT */

	if (!TX_BLK_TEST_FLAG(tx_blk, fTX_MCU_OFFLOAD)) {
		if (pMacEntry->tx_amsdu_bitmap & (1 << up)) {
#ifdef HW_TX_AMSDU_SUPPORT
			if (tr_ctl->amsdu_type == TX_HW_AMSDU) {
#ifdef PER_PKT_CTRL_FOR_CTMR
				if (!is_amsdu_frame(pAd, pkt, tx_blk))
					TX_BLK_CLEAR_FLAG(tx_blk, fTX_HW_AMSDU);
				else
#endif
					TX_BLK_SET_FLAG(tx_blk, fTX_HW_AMSDU);
			} else
#endif /* HW_TX_AMSDU_SUPPORT */
				TxFrameType = TX_AMSDU_FRAME;
		}
	}

	return TxFrameType;
}

inline BOOLEAN fill_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	BOOLEAN ret;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_MCU_OFFLOAD))
		ret = ops->fill_offload_tx_blk(pAd, wdev, pTxBlk);
	else
		ret = ops->fill_non_offload_tx_blk(pAd, wdev, pTxBlk);

	return ret;
}

INT send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	INT ret = NDIS_STATUS_SUCCESS;
	BOOLEAN allowed;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;

#ifdef VLAN_SUPPORT
	if (pAd->CommonCfg.bEnableVlan && IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		allowed = ap_fp_tx_pkt_vlan_tag_handle(pAd, wdev, &pkt);
		if (!allowed) {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			ret = NDIS_STATUS_FAILURE;
			return ret;
		}
	}
#endif /*VLAN_SUPPORT*/

	MEM_DBG_PKT_RECORD(pkt, 1<<1);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		allowed = ops->fp_tx_pkt_allowed(pAd, wdev, pkt);

		if (allowed) {
			ret = ops->fp_send_data_pkt(pAd, wdev, pkt);
		} else {
#ifdef WTBL_TDD_SUPPORT
			if (IS_WTBL_TDD_ENABLED(pAd) && (RTMP_GET_PACKET_PENDING(pkt) == 1)) {
				RTMP_SEM_LOCK(&pAd->wtblTddInfo.txPendingListLock);
				InsertTailQueue(&pAd->wtblTddInfo.txPendingList, PACKET_TO_QUEUE_ENTRY(pkt));
				 MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s(): STORE with Data packt CNT(%d)\n", __func__, pAd->wtblTddInfo.txPendingList.Number);
				RTMP_SEM_UNLOCK(&pAd->wtblTddInfo.txPendingListLock);
				return NDIS_STATUS_FAILURE;
			} else
#endif /* WTBL_TDD_SUPPORT */
			{
				tr_cnt->tx_not_allowed_drop++;
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
				ret = NDIS_STATUS_FAILURE;
				return ret;
			}
		}
	} else {
		allowed = ops->tx_pkt_allowed(pAd, wdev, pkt);

		if (allowed) {
			ret = ops->send_data_pkt(pAd, wdev, pkt);
		} else {
			tr_cnt->tx_not_allowed_drop++;
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			ret = NDIS_STATUS_FAILURE;
			return ret;
		}
	}

	return ret;
}

INT send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue)
{
	INT ret;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	ret = ops->send_mlme_pkt(pAd, pkt, wdev, q_idx, is_data_queue);
	return ret;
}

/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		pAd	Pointer to our adapter
		Rate			Transmit rate
		Size			Frame size in units of byte

	Return Value:
		Duration number in units of usec

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
USHORT RTMPCalcDuration(RTMP_ADAPTER *pAd, UCHAR Rate, ULONG Size)
{
	ULONG	Duration = 0;

	if (Rate < RATE_FIRST_OFDM_RATE) { /* CCK*/
		if ((Rate > RATE_1) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
			Duration = 96;	/* 72+24 preamble+plcp*/
		else
			Duration = 192; /* 144+48 preamble+plcp*/

		Duration += (USHORT)((Size << 4) / RateIdTo500Kbps[Rate]);

		if ((Size << 4) % RateIdTo500Kbps[Rate])
			Duration++;
	} else if (Rate <= RATE_LAST_OFDM_RATE) { /* OFDM rates*/
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
		Duration += 4 * (USHORT)((11 + Size * 4) / RateIdTo500Kbps[Rate]);

		if ((11 + Size * 4) % RateIdTo500Kbps[Rate])
			Duration += 4;
	} else {	/*mimo rate*/
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
	}

	return (USHORT)Duration;
}


/*
	NOTE: we do have an assumption here, that Byte0 and Byte1
		always reasid at the same scatter gather buffer
 */
static inline VOID Sniff2BytesFromNdisBuffer(
	IN PNDIS_BUFFER buf,
	IN UCHAR offset,
	OUT UCHAR *p0,
	OUT UCHAR *p1)
{
	UCHAR *ptr = (UCHAR *)(buf + offset);
	*p0 = *ptr;
	*p1 = *(ptr + 1);
}

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
BOOLEAN is_gratuitous_arp(UCHAR *pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UCHAR *SenderIP;
	UCHAR *TargetIP;

	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_ARP) {
		/*
		 * Check if Gratuitous ARP, Sender IP equal Target IP
		 */
		SenderIP = Pos + 14;
		TargetIP = Pos + 24;

		if (NdisCmpMemory(SenderIP, TargetIP, 4) == 0) {
			MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "The Packet is GratuitousARP\n");
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN is_dad_packet(RTMP_ADAPTER *pAd, UCHAR *pData)
{
	UCHAR *pSenderIP = pData + 16;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR *pSourceMac = pData + 10;
#endif /* MAC_REPEATER_SUPPORT */
	UCHAR *pDestMac = pData + 20;
	UCHAR ZERO_IP_ADDR[4] = {0x00, 0x00, 0x00, 0x00};
	UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	/*
	* Check if DAD packet
	*/
	if (
#ifdef MAC_REPEATER_SUPPORT
		(RTMPLookupRepeaterCliEntry(pAd, FALSE, pSourceMac, TRUE) != NULL) &&
#endif /* MAC_REPEATER_SUPPORT */
		((MAC_ADDR_EQUAL(pDestMac, BROADCAST_ADDR) == TRUE) ||
		 (MAC_ADDR_EQUAL(pDestMac, ZERO_MAC_ADDR) == TRUE)) &&
		(RTMPEqualMemory(pSenderIP, ZERO_IP_ADDR, 4) == TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "DAD found, and do not send this packet\n");
		return TRUE;
	}

	return FALSE;
}

BOOLEAN is_looping_packet(RTMP_ADAPTER *pAd, NDIS_PACKET *pPacket)
{
	UCHAR *pSrcBuf;
	UINT16 TypeLen;
	UCHAR Byte0, Byte1;

	if (!pAd)
		return FALSE;

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	ASSERT(pSrcBuf);
	/* get Ethernet protocol field and skip the Ethernet Header */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	pSrcBuf += LENGTH_802_3;

	if (TypeLen <= 1500) {
		/*
			802.3, 802.3 LLC:
				DestMAC(6) + SrcMAC(6) + Lenght(2) +
				DSAP(1) + SSAP(1) + Control(1) +
			if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
				=> + SNAP (5, OriginationID(3) + etherType(2))
			else
				=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
		*/
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03) {
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += LENGTH_802_1_H; /* Skip this LLC/SNAP header*/
		} else {
			return FALSE;
		}
	}

	if (TypeLen == ETH_TYPE_ARP) {
		/* AP's tx shall check DAD.*/
		if (is_dad_packet(pAd, pSrcBuf - 2) || is_gratuitous_arp(pSrcBuf - 2))
			return TRUE;
	}

	return FALSE;
}

void set_wf_fwd_cb(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, struct wifi_dev *wdev)
{
	struct sk_buff *pOsRxPkt = RTPKT_TO_OSPKT(pPacket);

	RTMP_CLEAN_PACKET_BAND(pOsRxPkt);
	RTMP_CLEAN_PACKET_RECV_FROM(pOsRxPkt);

	if (wdev->channel >= H_CHANNEL_BIGGER_THAN) {
		RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_5G_H);

		if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_WDS))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_H_AP);
		else if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_H_CLIENT);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "No Setting RTMP_SET_PACKET_RECV_FROM(5G_H) wdev->channel(%d) wdev->wdev_type(%d)\n", wdev->channel, wdev->wdev_type);
		}
	} else if (wdev->channel > 14) {
		RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_5G);

		if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_WDS))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_AP);
		else if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_CLIENT);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "No Setting RTMP_SET_PACKET_RECV_FROM(5G) wdev->channel(%d) wdev->wdev_type(%d)\n", wdev->channel, wdev->wdev_type);
		}
	} else {
		RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_2G);

		if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_WDS))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_AP);
		else if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA))
			RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_CLIENT);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "No Setting RTMP_SET_PACKET_RECV_FROM(2G) wdev->channel(%d) wdev->wdev_type(%d)\n", wdev->channel, wdev->wdev_type);
		}
	}

	/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("set_wf_fwd_cb: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x=> BandFrom:0x%x, RecvdFrom: 0x%x\n", */
	/* wdev->wdev_idx,wdev->wdev_type,wdev->func_idx, */
	/* RTMP_GET_PACKET_BAND(pOsRxPkt),RTMP_GET_PACKET_RECV_FROM(pOsRxPkt))); */

}

#endif /* CONFIG_WIFI_PKT_FWD */

VOID CheckQosMapUP(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry, UCHAR DSCP, PUCHAR pUserPriority)
{
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2)
	UCHAR i = 0, find_up = 0, dscpL = 0, dscpH = 0;
	BSS_STRUCT *pMbss = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (pEntry == NULL || pEntry->wdev == NULL)
		return;
	else {
		/* pAd = (RTMP_ADAPTER *)pEntry->pAd; */
		tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
	}

	if (IS_ENTRY_CLIENT(tr_entry))
		pMbss = (BSS_STRUCT *)pEntry->wdev->func_dev;
	else
		return;

	if (pEntry->QosMapSupport && pMbss->HotSpotCtrl.QosMapEnable && pMbss->HotSpotCtrl.HotSpotEnable) {
		for (i = 0; i < (pEntry->DscpExceptionCount / 2); i++) {
			if ((pEntry->DscpException[i] & 0xff) == DSCP) {
				*pUserPriority = (pEntry->DscpException[i] >> 8) & 0xff;
				find_up = 1;
				break;
			}
		}

		if (!find_up) {
			for (i = 0; i < 8; i++) {
				dscpL = pEntry->DscpRange[i] & 0xff;
				dscpH = (pEntry->DscpRange[i] >> 8) & 0xff;

				if ((dscpH >= DSCP) && (dscpL <= DSCP)) {
					*pUserPriority = i;
					break;
				}
			}
		}
	}

#endif /* defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2) */
}

#ifdef DSCP_PRI_SUPPORT
VOID check_dscp_mapped_up(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry, UCHAR DSCP, PUCHAR pUserPriority)
{
	INT8 pri = -1;

	if (pEntry == NULL || pEntry->wdev == NULL || IS_ENTRY_APCLI(pEntry))
		return;

	if ((pAd->ApCfg.DscpPriMapSupport) && DSCP <= 63) {
		pri = pAd->ApCfg.MBSSID[pEntry->wdev->func_idx].dscp_pri_map[DSCP];
	}

		if (pri >= 0  && pri <= 7)
			*pUserPriority = pri;
}
#endif /*DSCP_PRI_SUPPORT*/

#ifdef CONFIG_HOTSPOT_R2
static BOOLEAN is_hotspot_disabled_for_wdev(IN RTMP_ADAPTER * pAd, IN struct wifi_dev *wdev)
{
	BSS_STRUCT *pMbss;

	ASSERT(wdev->func_idx < pAd->ApCfg.BssidNum);
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

	if (pMbss->HotSpotCtrl.HotSpotEnable)
		return false;
	else
		return true;

}
#else
static inline BOOLEAN is_hotspot_disabled_for_wdev(IN RTMP_ADAPTER * pAd, IN struct wifi_dev *wdev)
{
	return true;
}
#endif

static UINT8  ac_queue_to_up[4] = {
	1 /* AC_BK */, 0 /* AC_BE */, 5 /* AC_VI */, 7 /* AC_VO */
};

/*
	Check the Ethernet Frame type, and set RTMP_SET_PACKET_SPECIFIC flags
	Here we set the PACKET_SPECIFIC flags(LLC, VLAN, DHCP/ARP, EAPOL).
*/
BOOLEAN RTMPCheckEtherType(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN STA_TR_ENTRY *tr_entry,
	IN struct wifi_dev *wdev)
{
	UINT16 TypeLen;
	UCHAR Byte0, Byte1, *pSrcBuf, up = 0;
	UCHAR q_idx = QID_AC_BE;
	UCHAR final_user_prio = 0;
#ifdef RT_CFG80211_SUPPORT
	BOOLEAN bClearFrame;
#endif
	MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[tr_entry->wcid];

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	ASSERT(pSrcBuf);
	if (!pSrcBuf) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s: pSrcBuf is null!\n", __func__);
		return FALSE;
	}
	RTMP_SET_PACKET_SPECIFIC(pPacket, 0);
	/* get Ethernet protocol field */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

#ifdef VLAN_SUPPORT
	/*insert 802.1Q tag if required*/
	if (pAd->CommonCfg.bEnableVlan && wdev->bVLAN_Tag && TypeLen != ETH_TYPE_VLAN
		&& wdev->VLAN_Policy[TX_VLAN] != VLAN_TX_ALLOW) {
		UINT16 tci = (wdev->VLAN_Priority<<(CFI_LEN + VID_LEN)) | wdev->VLAN_VID; /*CFI = 0*/

		pPacket = RtmpOsVLANInsertTag(pPacket, tci);
		if (pPacket == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Insert VLAN Tag fail!\n");
			return FALSE;
		}

		pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
		ASSERT(pSrcBuf);

		TypeLen = ETH_TYPE_VLAN;
	}
#endif /*VLAN_SUPPORT*/

	/*skip the Ethernet Header*/
	pSrcBuf += LENGTH_802_3;

	if (TypeLen <= 1500) {
		/*
			802.3, 802.3 LLC:
				DestMAC(6) + SrcMAC(6) + Lenght(2) +
				DSAP(1) + SSAP(1) + Control(1) +
			if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
				=> + SNAP (5, OriginationID(3) + etherType(2))
			else
				=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
		*/
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03) {
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			RTMP_SET_PACKET_LLCSNAP(pPacket, 1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += LENGTH_802_1_H; /* Skip this LLC/SNAP header*/
		} else
			return FALSE;
	}

	/* If it's a VLAN packet, get the real Type/Length field.*/
	if (TypeLen == ETH_TYPE_VLAN) {
#ifdef CONFIG_AP_SUPPORT
		/*
			802.3 VLAN packets format:

			DstMAC(6B) + SrcMAC(6B)
			+ 802.1Q Tag Type (2B = 0x8100) + Tag Control Information (2-bytes)
			+ Length/Type(2B)
			+ data payload (42-1500 bytes)
			+ FCS(4B)

			VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID
		*/

		/* If it is not my vlan pkt, no matter unicast or multicast, deal with it according to the policy*/
		if (wdev->VLAN_VID != 0) {
			USHORT vlan_id = *(USHORT *)pSrcBuf;

			vlan_id = cpu2be16(vlan_id);
			vlan_id = vlan_id & MASK_TCI_VID; /* 12 bit */

			if (vlan_id != wdev->VLAN_VID) {
#ifdef VLAN_SUPPORT
				if (pAd->CommonCfg.bEnableVlan) {
					switch (wdev->VLAN_Policy[TX_VLAN]) {
					case VLAN_TX_DROP:
						MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
							 "Drop the packet\n");
						wdev->VLANTxdrop++;
						return FALSE;
					case VLAN_TX_ALLOW:
					case VLAN_TX_KEEP_TAG:
						MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
							 "Allow the packet\n");
						break;
					case VLAN_TX_REPLACE_VID:
						MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
							 "Replace the packet VLAN ID\n");
						*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_VID);
						*(USHORT *)pSrcBuf |= be2cpu16(wdev->VLAN_VID);
						break;
					case VLAN_TX_REPLACE_ALL:
						MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
							 "Replace the packet VLAN Tag\n");
						*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_PCP);
						*(USHORT *)pSrcBuf |= be2cpu16((wdev->VLAN_Priority)<<(CFI_LEN + VID_LEN));
						*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_VID);
						*(USHORT *)pSrcBuf |= be2cpu16(wdev->VLAN_VID);
						break;
					default:
						MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Unexpected checking policy\n");
						return FALSE;
					}
				}
#else
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "failed for VLAN_ID(vlan_id=%d, VLAN_VID=%d)\n",
						vlan_id, wdev->VLAN_VID);
				return FALSE;
#endif /*VLAN_SUPPORT*/
			}
#ifdef VLAN_SUPPORT
			else {
				if (pAd->CommonCfg.bEnableVlan) {
					/* align PCP*/
					*(USHORT *)pSrcBuf &= be2cpu16(MASK_CLEAR_TCI_PCP);
					*(USHORT *)pSrcBuf |= be2cpu16((wdev->VLAN_Priority)<<(CFI_LEN + VID_LEN));
				}
			}
#endif /*VLAN_SUPPORT*/
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef MAP_TS_TRAFFIC_SUPPORT
		if (!pAd->bTSEnable)
#endif
		RTMP_SET_PACKET_VLAN(pPacket, 1);
		Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
		TypeLen = (USHORT)((Byte0 << 8) + Byte1);
#ifdef AUTOMATION
		if ((pAd->fpga_ctl.txrx_dbg_type == 1) ||
			(pAd->fpga_ctl.txrx_dbg_type == 5)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: vlan, Eth type=0x%x\n",
					 __func__, TypeLen);
		}
#endif /* AUTOMATION */
		/* only use VLAN tag as UserPriority setting */
		up = (*pSrcBuf & 0xe0) >> 5;
		CheckQosMapUP(pAd, pMacEntry, (*pSrcBuf & 0xfc) >> 2, &up);
#ifdef DSCP_PRI_SUPPORT
		if (pAd->ApCfg.DscpPriMapSupport) {
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
			if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
				/*handle DSCP-UP Mapping in CR4*/
			} else
#endif
				check_dscp_mapped_up(pAd, pMacEntry, (*pSrcBuf & 0xfc) >> 2, &up);
		}
#endif /*DSCP_PRI_SUPPORT*/

		pSrcBuf += LENGTH_802_1Q; /* Skip the VLAN Header.*/
	} else if (TypeLen == ETH_TYPE_IPv4) {
		/*
			0       4       8          14  15                      31(Bits)
			+---+----+-----+----+---------------+
			|Ver |  IHL |DSCP |ECN |    TotalLen           |
			+---+----+-----+----+---------------+
			Ver    - 4bit Internet Protocol version number.
			IHL    - 4bit Internet Header Length
			DSCP - 6bit Differentiated Services Code Point(TOS)
			ECN   - 2bit Explicit Congestion Notification
			TotalLen - 16bit IP entire packet length(include IP hdr)
		*/
		up = (*(pSrcBuf + 1) & 0xe0) >> 5;
		CheckQosMapUP(pAd, pMacEntry, (*(pSrcBuf + 1) & 0xfc) >> 2, &up);

#ifdef DSCP_PRI_SUPPORT
		if (pAd->ApCfg.DscpPriMapSupport) {
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
			if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
				/*handle DSCP-UP Mapping in CR4*/
			} else
#endif
				check_dscp_mapped_up(pAd, pMacEntry, (*(pSrcBuf + 1) & 0xfc) >> 2, &up);
		}
#endif /*DSCP_PRI_SUPPORT*/

	} else if (TypeLen == ETH_TYPE_IPv6) {
		/*
			0       4       8        12     16                      31(Bits)
			+---+----+----+----+---------------+
			|Ver | TrafficClas |  Flow Label                   |
			+---+----+----+--------------------+
			Ver           - 4bit Internet Protocol version number.
			TrafficClas - 8bit traffic class field, the 6 most-significant bits used for DSCP
		*/
		up = ((*pSrcBuf) & 0x0e) >> 1;
		CheckQosMapUP(pAd, pMacEntry, ((*pSrcBuf & 0x0f) << 2) | ((*(pSrcBuf + 1)) & 0xc0) >> 6, &up);
#ifdef DSCP_PRI_SUPPORT
		if (pAd->ApCfg.DscpPriMapSupport) {
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
			if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
				/*handle DSCP-UP Mapping in CR4*/
			} else
#endif
				check_dscp_mapped_up(pAd, pMacEntry,
					((*pSrcBuf & 0x0f) << 2) | ((*(pSrcBuf + 1)) & 0xc0) >> 6, &up);
		}
#endif /*DSCP_PRI_SUPPORT*/

	}

	switch (TypeLen) {
	case ETH_TYPE_IPv4: {
		UINT32 pktLen = GET_OS_PKT_LEN(pPacket);
#ifdef CSO_TEST_SUPPORT
		if (CsCtrl & BIT0) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
			*(pSrcBuf + 10) = 0xFF;
			*(pSrcBuf + 11) = 0xFF;
#else /* AUTOMATION */
			*(pSrcBuf + 10) = ~(*(pSrcBuf + 10));
			*(pSrcBuf + 11) = ~(*(pSrcBuf + 11));
#endif /* !AUTOMATION */
		}
#endif

		ASSERT((pktLen > (ETH_HDR_LEN + IP_HDR_LEN)));	/* 14 for ethernet header, 20 for IP header*/
		RTMP_SET_PACKET_IPV4(pPacket, 1);

		switch (*(pSrcBuf + 9)) {
		case IP_PROTO_UDP: {
			UINT16 srcPort, dstPort;

#ifdef CSO_TEST_SUPPORT
			if (CsCtrl & BIT2) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
				*(pSrcBuf + 26) = 0xFF;
				*(pSrcBuf + 27) = 0xFF;
#else /* AUTOMATION */
				*(pSrcBuf + 26) = ~(*(pSrcBuf + 26));
				*(pSrcBuf + 27) = ~(*(pSrcBuf + 27));
#endif /* !AUTOMATION */
			}
			if (CsCtrl & BIT3)
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Ipv4 udp pkt\n");
#endif
			pSrcBuf += IP_HDR_LEN;
			srcPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf)));
			dstPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf + 2)));

			if ((srcPort == 0x44 && dstPort == 0x43) || (srcPort == 0x43 && dstPort == 0x44)) {
				/*It's a BOOTP/DHCP packet*/
				RTMP_SET_PACKET_DHCP(pPacket, 1);
				RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
				RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
			}

#ifdef CONFIG_AP_SUPPORT
#if defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP)
			WNMIPv4ProxyARPCheck(pAd, pPacket, srcPort, dstPort, pSrcBuf, 0);
#endif
#ifdef CONFIG_HOTSPOT
			{
				USHORT Wcid = RTMP_GET_PACKET_WCID(pPacket);

				if (!HSIPv4Check(pAd, &Wcid, pPacket, pSrcBuf, srcPort, dstPort))
					return FALSE;
			}
#endif
#endif
			}
			break;
		case IP_PROTOCOL_ICMP:
#ifdef AUTOMATION
		if (pAd->auto_dvt && pAd->auto_dvt->apps.block_packet & ICMP_PACKET) {
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Automation Block ICMP Packet\n");
			return FALSE;
		}
#endif /* !AUTOMATION */
			pSrcBuf += IP_HDR_LEN;
			RTMP_SET_PACKET_PING(pPacket, 1);
			RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
			RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);

			break;
#ifdef CSO_TEST_SUPPORT
		case IP_PROTOCOL_TCP:
			if (CsCtrl & BIT1) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
				*(pSrcBuf + 36) = 0xFF;
				*(pSrcBuf + 37) = 0xFF;
#else /* AUTOMATION */
				*(pSrcBuf + 36) = ~(*(pSrcBuf + 36));
				*(pSrcBuf + 37) = ~(*(pSrcBuf + 37));
#endif /* !AUTOMATION */
			}
			if (CsCtrl & BIT3)
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Ipv4 tcp pkt\n");
			break;
#endif
		}
	}
	break;

	case ETH_TYPE_ARP: {
#ifdef CONFIG_AP_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP) {
#if defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP)
			BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

			if (pMbss->WNMCtrl.ProxyARPEnable) {
				/* Check if IPv4 Proxy ARP Candidate from DS */
				if (IsIPv4ProxyARPCandidate(pAd, pSrcBuf - 2)) {
					BOOLEAN FoundProxyARPEntry;

					FoundProxyARPEntry = IPv4ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE, 0);

					if (!FoundProxyARPEntry)
						MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Can not find proxy entry\n");

					return FALSE;
				}
			}

#endif

#ifdef CONFIG_HOTSPOT

			if (pMbss->HotSpotCtrl.HotSpotEnable) {
				if (!pMbss->HotSpotCtrl.DGAFDisable) {
					if (IsGratuitousARP(pAd, pSrcBuf - 2, pSrcBuf - 14, pMbss, 0))
						return FALSE;
				}
			}
#endif

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
			/* AP's tx shall check DAD.*/
			if (is_dad_packet(pAd, pSrcBuf - 2) || is_gratuitous_arp(pSrcBuf - 2))
				return FALSE;
#endif /* CONFIG_WIFI_PKT_FWD */
		}
#endif
#ifdef AUTOMATION
		if (pAd->auto_dvt && pAd->auto_dvt->apps.block_packet & ARP_PACKET) {
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Automation Block ARP Packet\n");
			return FALSE;
		}
#endif /* !AUTOMATION */

		RTMP_SET_PACKET_ARP(pPacket, 1);
		RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
	}
	break;

	case ETH_P_IPV6: {
#ifdef CONFIG_AP_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP) {
#if defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP)
			BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

			WNMIPv6ProxyARPCheck(pAd, pPacket, pSrcBuf, 0);

			if (pMbss->WNMCtrl.ProxyARPEnable) {
				/* Check if IPv6 Proxy ARP Candidate from DS */
				if (IsIPv6ProxyARPCandidate(pAd, pSrcBuf - 2)) {
					BOOLEAN FoundProxyARPEntry;

					FoundProxyARPEntry = IPv6ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE, 0);

					if (!FoundProxyARPEntry)
						MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Can not find IPv6 proxy entry\n");

					return FALSE;
				}
			}

#endif
#ifdef CONFIG_HOTSPOT
			if (pMbss->HotSpotCtrl.HotSpotEnable) {
				if (!pMbss->HotSpotCtrl.DGAFDisable) {
					if (IsUnsolicitedNeighborAdver(pAd, pSrcBuf - 2))
						return FALSE;
				}
			}
#endif
		}
		/*
			Check if DHCPv6 Packet, and Convert group-address DHCP
			packets to individually-addressed 802.11 frames
		 */
#endif

		/* return AC_BE if packet is not IPv6 */
		if ((*pSrcBuf & 0xf0) != 0x60)
			up = 0;

#ifdef CSO_TEST_SUPPORT
		switch (*(pSrcBuf + 6)) {
		case IP_PROTO_UDP:
			if (CsCtrl & BIT2) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
				*(pSrcBuf + 46) = 0xFF;
				*(pSrcBuf + 47) = 0xFF;
#else /* AUTOMATION */
				*(pSrcBuf + 46) = ~(*(pSrcBuf + 46));
				*(pSrcBuf + 47) = ~(*(pSrcBuf + 47));
#endif /* !AUTOMATION */
			}
			if (CsCtrl & BIT3)
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Ipv6 udp pkt\n");
			break;
		case IP_PROTOCOL_TCP:
			if (CsCtrl & BIT1) {
#ifdef AUTOMATION
/* for MSP check always 0xFF for Rx log easily check PASS / FAIL */
				*(pSrcBuf + 56) = 0xFF;
				*(pSrcBuf + 57) = 0xFF;
#else /* AUTOMATION */
				*(pSrcBuf + 56) = ~(*(pSrcBuf + 56));
				*(pSrcBuf + 57) = ~(*(pSrcBuf + 57));
#endif /* !AUTOMATION */
			}
			if (CsCtrl & BIT3)
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Ipv6 tcp pkt\n");
			break;
		}
#endif
	}
	break;

	case ETH_TYPE_EAPOL:
		RTMP_SET_PACKET_EAPOL(pPacket, 1);
		RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
#ifdef RT_CFG80211_SUPPORT
		bClearFrame = (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE;
		RTMP_SET_PACKET_CLEAR_EAP_FRAME(pPacket, (bClearFrame ? 1 : 0));
#endif
		break;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

	case ETHER_TYPE_TDLS_MMPDU: {
		RTMP_SET_PACKET_TDLS_MMPDU(pPacket, 1);
		RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
		up = 5;
	}
	break;
#endif /* DOT11Z_TDLS_SUPPORT */
	case ETH_TYPE_1905: {
		RTMP_SET_PACKET_HIGH_PRIO(pPacket, 1);
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_LEGACY_FRAME);
	}
	break;

	default:
		break;
	}

#ifdef VENDOR_FEATURE1_SUPPORT
	RTMP_SET_PACKET_PROTOCOL(pPacket, TypeLen);
#endif /* VENDOR_FEATURE1_SUPPORT */

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (pAd->vow_dvt_en) {
		UINT8 band = 0;
		UINT16 wcid = tr_entry->wcid;
		if (pAd->CommonCfg.dbdc_mode)
			band = WMODE_CAP_5G(wdev->PhyMode) ? 1 : 0;

		if ((!RTMP_GET_PACKET_MGMT_PKT(pPacket)) &&
			(RTMP_GET_PACKET_TYPE(pPacket) != TX_ALTX)) {
			if ((wcid == pAd->vow_cloned_wtbl_from[band]) ||
				((wcid >= pAd->vow_cloned_wtbl_start[band]) &&
				(wcid <= pAd->vow_cloned_wtbl_max[band]))) {
				q_idx = pAd->vow_sta_ac[tr_entry->wcid];
				up = ac_queue_to_up[pAd->vow_sta_ac[tr_entry->wcid]];
				final_user_prio = up;
			}
		}
	}
#endif
	/*
	 * Set WMM when
	 * 1. wdev->bWmmCapable == TRUE
	 * 2. Receiver's capability
	 *	a). bc/mc packets
	 *		->Need to get UP for IGMP use
	 *	b). unicast packets
	 *		-> CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)
	 *	3. has VLAN tag or DSCP fields in IPv4/IPv6 hdr
	 */
	if ((wdev->bWmmCapable == TRUE) && (up <= 7)) {
		if (RTMP_GET_PACKET_HIGH_PRIO(pPacket)) {
			if ((up == 0) || (up == 3)) {
				if (is_hotspot_disabled_for_wdev(pAd, wdev)
#ifdef DSCP_PRI_SUPPORT
					&& (!pAd->ApCfg.DscpPriMapSupport)
#endif /*DSCP_PRI_SUPPORT*/
					)
						up = ac_queue_to_up[pAd->cp_support];
			}
		}
	}

	/* have to check ACM bit. downgrade UP & QueIdx before passing ACM*/
	/* NOTE: AP doesn't have to negotiate TSPEC. ACM is controlled purely via user setup, not protocol handshaking*/
	/*
		Under WMM ACM control, we dont need to check the bit;
		Or when a TSPEC is built for VO but we will change priority to
		BE here and when we issue a BA session, the BA session will
		be BE session, not VO session.
	*/
	if ((up <= 7) && ((pAd->CommonCfg.APEdcaParm[0].bACM[WMM_UP2AC_MAP[up]])
		|| (((wdev->wdev_type == WDEV_TYPE_REPEATER)
			|| (wdev->wdev_type == WDEV_TYPE_STA))
			&& (pMacEntry->bACMBit[WMM_UP2AC_MAP[up]]))))

		up = 0;



	if ((wdev->bWmmCapable == TRUE) && (up <= 7)) {
		final_user_prio = up;
		q_idx = WMM_UP2AC_MAP[up];
	}

	RTMP_SET_PACKET_UP(pPacket, final_user_prio);
	RTMP_SET_PACKET_QUEIDX(pPacket, q_idx);
	return TRUE;
}

VOID RTMPCheckDhcpArp(IN PNDIS_PACKET pkt)
{
	UCHAR *pSrcBuf, Byte0, Byte1;
	UINT16 TypeLen;
	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	pSrcBuf += LENGTH_802_3;
	if (TypeLen == ETH_TYPE_VLAN) {
		Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
		TypeLen = (USHORT)((Byte0 << 8) + Byte1);
		pSrcBuf += LENGTH_802_1Q;
	}
	switch (TypeLen) {
	case ETH_TYPE_IPv4:
		if (*(pSrcBuf + 9) == IP_PROTO_UDP) {
			UINT16 srcPort, dstPort;

			pSrcBuf += IP_HDR_LEN;
			srcPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf)));
			dstPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf + 2)));
			if ((srcPort == 0x44 && dstPort == 0x43) || (srcPort == 0x43 && dstPort == 0x44)) {
				/*It's a BOOTP/DHCP packet*/
				RTMP_SET_PACKET_DHCP(pkt, 1);
			}
		}
		break;

	case ETH_TYPE_ARP:
		RTMP_SET_PACKET_ARP(pkt, 1);
		break;
	default:
		break;
	}
}

#ifdef SOFT_ENCRYPT
VOID RTMPUpdateSwCacheCipherInfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pHdr)
{
	HEADER_802_11 *pHeader_802_11;
	MAC_TABLE_ENTRY *pMacEntry;

	pHeader_802_11 = (HEADER_802_11 *) pHdr;
	pMacEntry = pTxBlk->pMacEntry;

	if (pMacEntry && pHeader_802_11->FC.Wep &&
		CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT)) {
		PCIPHER_KEY pKey = &pMacEntry->PairwiseKey;

		TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);
		pTxBlk->CipherAlg = pKey->CipherAlg;
		pTxBlk->pKey = pKey;
			if ((pKey->CipherAlg == CIPHER_WEP64) || (pKey->CipherAlg == CIPHER_WEP128))
				inc_iv_byte(pKey->TxTsc, LEN_WEP_TSC, 1);
			else if ((pKey->CipherAlg == CIPHER_TKIP) || (pKey->CipherAlg == CIPHER_AES))
				inc_iv_byte(pKey->TxTsc, LEN_WPA_TSC, 1);
	}
}
#endif

#if defined (SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
BOOLEAN RTMPExpandPacketForSwEncrypt(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UINT32	ex_head = 0, ex_tail = 0;
	UCHAR	NumberOfFrag = RTMP_GET_PACKET_FRAGMENTS(pTxBlk->pPacket);
		if (pTxBlk->CipherAlg == CIPHER_AES)
			ex_tail = LEN_CCMP_MIC;

	ex_tail = (NumberOfFrag * ex_tail);
	pTxBlk->pPacket = ExpandPacket(pAd, pTxBlk->pPacket, ex_head, ex_tail);

	if (pTxBlk->pPacket == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: out of resource.\n", __func__);
		return FALSE;
	}

	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pTxBlk->pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pTxBlk->pPacket);
	return TRUE;
}

INT tx_sw_encrypt(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR *pHeaderBufPtr, HEADER_802_11 *wifi_hdr)
{
	UCHAR iv_offset = 0, ext_offset = 0;
	/*
		If original Ethernet frame contains no LLC/SNAP,
		then an extra LLC/SNAP encap is required
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
						   pHeaderBufPtr, &iv_offset);
	pHeaderBufPtr += iv_offset;
	/* TODO: shiang-MT7603, for header Len, shall we take care that?? */
	pTxBlk->MpduHeaderLen += iv_offset;
	/* Encrypt the MPDU data by software */
	RTMPSoftEncryptionAction(pAd,
							 pTxBlk->CipherAlg,
							 (UCHAR *)wifi_hdr,
							 pTxBlk->pSrcBufData,
							 pTxBlk->SrcBufLen,
							 pTxBlk->KeyIdx,
							 pTxBlk->pKey, &ext_offset);
	pTxBlk->SrcBufLen += ext_offset;
	pTxBlk->TotalFrameLen += ext_offset;
	return TRUE;
}
#endif /* SOFT_ENCRYPT */

#ifdef IP_ASSEMBLY
/*for cache usage to improve throughput*/
static INT rtmp_IpAssembleDataCreate(RTMP_ADAPTER *pAd, UCHAR queId, IP_ASSEMBLE_DATA **ppIpAsmbData, UINT id, UINT fragSize)
{
	ULONG now = 0;
	IP_ASSEMBLE_DATA *pIpAsmbData = NULL;
	DL_LIST *pAssHead = &pAd->assebQueue[queId];

	os_alloc_mem(NULL, (UCHAR **)&pIpAsmbData, sizeof(IP_ASSEMBLE_DATA));
	*ppIpAsmbData = pIpAsmbData;

	if (pIpAsmbData == NULL)
		return NDIS_STATUS_FAILURE;

	InitializeQueueHeader(&pIpAsmbData->queue);
	NdisGetSystemUpTime(&now);
	pIpAsmbData->identify = id;
	pIpAsmbData->fragSize = fragSize;
	pIpAsmbData->createTime = now;
	DlListAdd(pAssHead, &pIpAsmbData->list);
	return NDIS_STATUS_SUCCESS;
}


static VOID rtmp_IpAssembleDataDestory(RTMP_ADAPTER *pAd, IP_ASSEMBLE_DATA *pIpAsmbData)
{
	PQUEUE_ENTRY pPktEntry;
	PNDIS_PACKET pPkt;

	/*free queue packet*/
	while (1) {
		pPktEntry = RemoveHeadQueue(&pIpAsmbData->queue);

		if (pPktEntry == NULL)
			break;

		pPkt = QUEUE_ENTRY_TO_PACKET(pPktEntry);
		RELEASE_NDIS_PACKET(pAd, pPkt, NDIS_STATUS_FAILURE);
	}

	/*remove from list*/
	DlListDel(&pIpAsmbData->list);
	/*free data*/
	os_free_mem(pIpAsmbData);
}


static IP_ASSEMBLE_DATA *rtmp_IpAssembleDataSearch(RTMP_ADAPTER *pAd, UCHAR queIdx, UINT identify)
{
	DL_LIST *pAssHead = &pAd->assebQueue[queIdx];
	IP_ASSEMBLE_DATA *pAssData = NULL;

	DlListForEach(pAssData, pAssHead, struct ip_assemble_data, list) {
		if (pAssData->identify == identify)
			return pAssData;
	}
	return NULL;
}


static VOID rtmp_IpAssembleDataUpdate(RTMP_ADAPTER *pAd)
{
	DL_LIST *pAssHead = NULL;
	IP_ASSEMBLE_DATA *pAssData = NULL, *pNextAssData = NULL;
	INT i = 0;
	ULONG now = 0;
	QUEUE_HEADER *pAcQueue = NULL;
	struct ip_assemble_data **cur_ip_assem_data = (struct ip_assemble_data **) pAd->cur_ip_assem_data;

	NdisGetSystemUpTime(&now);

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAssHead = &pAd->assebQueue[i];
		DlListForEachSafe(pAssData, pNextAssData, pAssHead, struct ip_assemble_data, list) {
			pAcQueue = &pAssData->queue;

			if ((pAcQueue->Number != 0) && (RTMP_TIME_AFTER(now, (pAssData->createTime) + (1000 * OS_HZ)))) {
				if (cur_ip_assem_data[i] == pAssData)
					cur_ip_assem_data[i] = NULL;

				rtmp_IpAssembleDataDestory(pAd, pAssData);
			}
		}
	}
}


INT rtmp_IpAssembleHandle(RTMP_ADAPTER *pAd, STA_TR_ENTRY *pTrEntry, PNDIS_PACKET pPacket, UCHAR queIdx, PACKET_INFO packetInfo)
{
	IP_ASSEMBLE_DATA *pIpAsmData = NULL;
	/*define local variable*/
	IP_V4_HDR *pIpv4Hdr, Ipv4Hdr;
	IP_FLAGS_FRAG_OFFSET *pFlagsFragOffset, flagsFragOffset;
	UINT fragSize = 0;
	QUEUE_HEADER *pAcQueue = NULL;
	UINT32 fragCount = 0;
	struct ip_assemble_data **cur_ip_assem_data = (struct ip_assemble_data **) pAd->cur_ip_assem_data;
	/*check all timer of assemble for ageout */
	rtmp_IpAssembleDataUpdate(pAd);

	/*is not ipv4 packet*/
	if (!RTMP_GET_PACKET_IPV4(pPacket)) {
		/*continue to do normal path*/
		return NDIS_STATUS_INVALID_DATA;
	}

	pFlagsFragOffset = (IP_FLAGS_FRAG_OFFSET *) (packetInfo.pFirstBuffer + LENGTH_802_3 + 6);
	flagsFragOffset.word = ntohs(pFlagsFragOffset->word);

	/*is not fragment packet*/
	if (flagsFragOffset.field.flags_more_frag == 0 && flagsFragOffset.field.frag_offset == 0) {
		/*continue to do normal path*/
		return NDIS_STATUS_INVALID_DATA;
	}

	/*get ipv4 */
	pIpv4Hdr = (IP_V4_HDR *) (packetInfo.pFirstBuffer + LENGTH_802_3);
	Ipv4Hdr.identifier = ntohs(pIpv4Hdr->identifier);
	Ipv4Hdr.tot_len = ntohs(pIpv4Hdr->tot_len);
	Ipv4Hdr.ihl = pIpv4Hdr->ihl;
	fragSize = Ipv4Hdr.tot_len - (Ipv4Hdr.ihl * 4);

	/* check if 1st fragment */
	if ((flagsFragOffset.field.flags_more_frag == 1) && (flagsFragOffset.field.frag_offset == 0)) {
		/*check current queue is exist this id packet or not*/
		pIpAsmData = rtmp_IpAssembleDataSearch(pAd, queIdx, Ipv4Hdr.identifier);

		/*if not exist, create it*/
		if (!pIpAsmData) {
			rtmp_IpAssembleDataCreate(pAd, queIdx, &pIpAsmData, Ipv4Hdr.identifier, fragSize);

			if (!pIpAsmData) {
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}
		}

		/*store to  cache */
		cur_ip_assem_data[queIdx] = pIpAsmData;
		/*insert packet*/
		pAcQueue = &pIpAsmData->queue;
		InsertTailQueue(pAcQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
	} else {
		/*search assemble data from identify and cache first*/
		if (cur_ip_assem_data[queIdx] && (cur_ip_assem_data[queIdx]->identify == Ipv4Hdr.identifier))
			pIpAsmData = cur_ip_assem_data[queIdx];
		else {
			pIpAsmData = rtmp_IpAssembleDataSearch(pAd, queIdx, Ipv4Hdr.identifier);

			/*not create assemble, should drop*/
			if (!pIpAsmData) {
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}

			/*update cache*/
			cur_ip_assem_data[queIdx] = pIpAsmData;
		}

		pAcQueue = &pIpAsmData->queue;
		InsertTailQueue(pAcQueue, PACKET_TO_QUEUE_ENTRY(pPacket));

		/* check if last fragment */
		if (pIpAsmData && (flagsFragOffset.field.flags_more_frag == 0) && (flagsFragOffset.field.frag_offset != 0)) {
			/*fragment packets gatter and check*/
			fragCount = ((flagsFragOffset.field.frag_offset * 8) / (pIpAsmData->fragSize)) + 1;

			if (pAcQueue->Number != fragCount) {
				rtmp_IpAssembleDataDestory(pAd, pIpAsmData);
				cur_ip_assem_data[queIdx] = NULL;
				return NDIS_STATUS_FAILURE;
			}

			/* move backup fragments to software queue */
			if (ge_enq_req(pAd, NULL, queIdx, pTrEntry, pAcQueue) == FALSE) {
				rtmp_IpAssembleDataDestory(pAd, pIpAsmData);
				cur_ip_assem_data[queIdx] = NULL;
				return NDIS_STATUS_FAILURE;
			}

			rtmp_IpAssembleDataDestory(pAd, pIpAsmData);
			cur_ip_assem_data[queIdx] = NULL;
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif


VOID enable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio, UINT16 level)
{
	if (wdev == NULL)
		return;

#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "%s, prio=%d, level=0x%x, <caller: %pS>\n",
				  __func__, prio, level,
				  __builtin_return_address(0));
		HW_SET_TX_BURST(pAd, wdev, ac_type, prio, level, 1);
	}

#endif /* MT_MAC */
}

VOID disable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					  UINT8 ac_type, UINT8 prio, UINT16 level)
{
	if (wdev == NULL)
		return;

#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "%s, prio=%d, level=0x%x, <caller: %pS>\n",
				  __func__, prio, level,
				  __builtin_return_address(0));
		HW_SET_TX_BURST(pAd, wdev, ac_type, prio, level, 0);
	}

#endif /* MT_MAC */
}

UINT8 query_tx_burst_prio(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UINT8 i, prio = PRIO_DEFAULT;
	UINT32 prio_bitmap = 0;

	if (wdev == NULL)
		return prio;
#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		prio_bitmap = wdev->bss_info_argument.prio_bitmap;
		for (i = 0; i < MAX_PRIO_NUM; i++) {
			if (prio_bitmap & (1 << i))
				prio = i;
		}
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s, curr: prio=%d, txop=0x%x, <caller: %pS>\n",
				__func__, prio, wdev->bss_info_argument.txop_level[prio],
				__builtin_return_address(0));
	}
#endif /* MT_MAC */
	return prio;
}

INT TxOPUpdatingAlgo(RTMP_ADAPTER *pAd)
{
	UCHAR UpdateTxOP = 0xFF;
	UINT32 TxTotalByteCnt = pAd->TxTotalByteCnt;
	UINT32 RxTotalByteCnt = pAd->RxTotalByteCnt;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if ((TxTotalByteCnt == 0) || (RxTotalByteCnt == 0)) {
		/* Avoid to divide 0, when doing the traffic calculating */
		/* MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Not expected one of them is 0, TxTotalByteCnt = %lu, RxTotalByteCnt = %lu\n", (ULONG)TxTotalByteCnt, (ULONG)RxTotalByteCnt); */
	} else if ((pAd->MacTab.Size == 1)
			   && (pAd->CommonCfg.ManualTxop == 0)
			   && pAd->CommonCfg.bEnableTxBurst
			   && (cap->qos.TxOPScenario == 1)) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE) == TRUE) {
			UpdateTxOP = 0x80; /* Tx/Rx/Bi */
		} else if ((((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20) > pAd->CommonCfg.ManualTxopThreshold) {
			/* TxopThreshold unit is Mbps */
			if (TxTotalByteCnt > RxTotalByteCnt) {
				if ((TxTotalByteCnt / RxTotalByteCnt) >= pAd->CommonCfg.ManualTxopUpBound) {
					/* Boundary unit is Ratio */
					UpdateTxOP = 0x60; /* Tx */
				} else if ((TxTotalByteCnt / RxTotalByteCnt) <= pAd->CommonCfg.ManualTxopLowBound) {
					/* UpdateTxOP = 0x0; // Bi */
					UpdateTxOP = (pAd->MacTab.fCurrentStaBw40) ? 0x0 : 0x60;
				} else {
					/* No change TxOP */
				}
			} else {
				if ((RxTotalByteCnt / TxTotalByteCnt) >= pAd->CommonCfg.ManualTxopUpBound) {
					/* Boundary unit is Ratio */
					UpdateTxOP = 0x0; /* Rx */
				} else if ((RxTotalByteCnt / TxTotalByteCnt) <= pAd->CommonCfg.ManualTxopLowBound) {
					/* UpdateTxOP = 0x0; // Bi */
					UpdateTxOP = (pAd->MacTab.fCurrentStaBw40) ? 0x0 : 0x60;
				} else {
					/* No change TxOP */
				}
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Current TP=%lu < Threshold(%lu), turn-off TxOP\n",
					 (ULONG)(((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20), (ULONG)pAd->CommonCfg.ManualTxopThreshold);
			UpdateTxOP = 0x0;
		}
	} else if (pAd->MacTab.Size > 1)
		UpdateTxOP = 0x0;

	if (UpdateTxOP != 0xFF &&
		UpdateTxOP != pAd->CurrEdcaParam[WMM_PARAM_AC_1].u2Txop)
		AsicUpdateTxOP(pAd, NULL, WMM_PARAM_AC_1, UpdateTxOP);

	pAd->TxTotalByteCnt = 0;
	pAd->RxTotalByteCnt = 0;
	return TRUE;
}


VOID ComposeNullFrame(
	RTMP_ADAPTER *pAd,
	PHEADER_802_11 pNullFrame,
	UCHAR *pAddr1,
	UCHAR *pAddr2,
	UCHAR *pAddr3)
{
	NdisZeroMemory(pNullFrame, sizeof(HEADER_802_11));
	pNullFrame->FC.Type = FC_TYPE_DATA;
	pNullFrame->FC.SubType = SUBTYPE_DATA_NULL;
	pNullFrame->FC.ToDs = 1;
	COPY_MAC_ADDR(pNullFrame->Addr1, pAddr1);
	COPY_MAC_ADDR(pNullFrame->Addr2, pAddr2);
	COPY_MAC_ADDR(pNullFrame->Addr3, pAddr3);
}

VOID mt_detect_wmm_traffic(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR usr_prior, UCHAR FlgIsOutput)
{
	/*Check WMM Rx traffic : Count Non-BE pkt in 1 sec*/
	/* RX WMM detetion, it only enable in cert. test */
	if ((usr_prior != AC_BE_ee) && (usr_prior != AC_BE_be)) {
		if (FlgIsOutput == FLG_IS_OUTPUT)
			pAd->tx_OneSecondnonBEpackets++;
		else if (FlgIsOutput == FLAG_IS_INPUT && pAd->CommonCfg.wifi_cert)
			pAd->rx_OneSecondnonBEpackets++;
	}
}

VOID mt_dynamic_wmm_be_tx_op(
	IN RTMP_ADAPTER *pAd,
	IN ULONG nonBEpackets)
{
	struct wifi_dev *wdev;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	INT idx;
#endif /* APCLI_SUPPORT */

	if (pAd->tx_one_second_ac_counter >= WMM_PBC_CTL_COUNT ||
		pAd->rx_OneSecondnonBEpackets > nonBEpackets) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE)) {
			if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD) && cap->txd_flow_ctl)
				MtCmdCr4Set(pAd, WA_SET_OPTION_CONFIG_WMM_MODE, 1, 0);

#ifdef APCLI_SUPPORT
			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;

					if ((wdev) && (pAd->StaCfg[idx].ApcliInfStat.Valid == TRUE))
					enable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
			}

#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

			if (wdev)
				enable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);

#ifdef PKT_BUDGET_CTRL_SUPPORT
			if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
				HW_SET_PBC_CTRL(pAd, NULL, NULL, PBC_TYPE_WMM);
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT

			for (idx = 0; idx < pAd->MSTANum; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;

				if ((wdev) && (pAd->StaCfg[idx].wdev.DevInfo.WdevActive))
					enable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
			}

#endif /* CONFIG_STA_SUPPORT */
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);
		}
	} else {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) == 0) {
			if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD) && cap->txd_flow_ctl)
				MtCmdCr4Set(pAd, WA_SET_OPTION_CONFIG_WMM_MODE, 0, 0);

#ifdef APCLI_SUPPORT
			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;

					if ((wdev) && (pAd->StaCfg[idx].ApcliInfStat.Valid == TRUE))
					disable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
			}

#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

			if (wdev)
				disable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);

#ifdef PKT_BUDGET_CTRL_SUPPORT
			if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
				HW_SET_PBC_CTRL(pAd, NULL, NULL, PBC_TYPE_NORMAL);
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
			for (idx = 0; idx < pAd->MSTANum; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;

				if ((wdev) && (pAd->StaCfg[idx].wdev.DevInfo.WdevActive))
					disable_tx_burst(pAd, wdev, AC_BE, PRIO_WMM, TXOP_0);
			}
#endif /* CONFIG_STA_SUPPORT */
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);
		}
	}
	pAd->tx_one_second_ac_counter = 0;
	pAd->rx_OneSecondnonBEpackets = 0;
}

UINT32 Get_OBSS_AirTime(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                   BandIdx)
{
	return pAd->OneSecMibBucket.OBSSAirtime[BandIdx];
}

VOID Reset_OBSS_AirTime(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                   BandIdx)
{
	/* Policy. CR access MUST implement at FW.
	     It will move to FW in the future. */
	UINT32  CrValue = 0;

	HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME0, &CrValue);
	CrValue |= 1 << RX_MIBTIME_CLR_OFFSET;
	CrValue |= 1 << RX_MIBTIME_EN_OFFSET;
	HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MIBTIME0, CrValue);
}

UINT32 Get_My_Tx_AirTime(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                 BandIdx)
{
	return pAd->OneSecMibBucket.MyTxAirtime[BandIdx];
}

UINT32 Get_My_Rx_AirTime(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                   BandIdx)
{
	return pAd->OneSecMibBucket.MyRxAirtime[BandIdx];
}

UINT32 Get_EDCCA_Time(
	IN	PRTMP_ADAPTER	pAd,
	IN      UCHAR                   BandIdx)
{
	return pAd->OneSecMibBucket.EDCCAtime[BandIdx];
}


VOID CCI_ACI_scenario_maintain(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR   i = 0;
	UCHAR   j = 0;
	UINT32  ObssAirTime[DBDC_BAND_NUM] = {0};
	UINT32  MyTxAirTime[DBDC_BAND_NUM] = {0};
	UINT32  MyRxAirTime[DBDC_BAND_NUM] = {0};
	UINT32  EDCCATime[DBDC_BAND_NUM] = {0};
	UCHAR   ObssAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR   MyAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR   EdccaOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR   MyTxAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);
	struct wifi_dev *pWdev = NULL;
	UCHAR BandIdx;

	if (concurrent_bands > DBDC_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "It should not happen!!!!!");
		concurrent_bands = DBDC_BAND_NUM;
	}

	for (i = 0; i < concurrent_bands; i++) {
		ObssAirTime[i] = Get_OBSS_AirTime(pAd, i);
		MyTxAirTime[i] = Get_My_Tx_AirTime(pAd, i);
		MyRxAirTime[i] = Get_My_Rx_AirTime(pAd, i);
		EDCCATime[i] = Get_EDCCA_Time(pAd, i);

		if (ObssAirTime[i] != 0)
			ObssAirOccupyPercentage[i] = (ObssAirTime[i] * 100) / ONE_SEC_2_US;

		if (MyTxAirTime[i] != 0 || MyRxAirTime[i] != 0)
			MyAirOccupyPercentage[i] = ((MyTxAirTime[i] + MyRxAirTime[i]) * 100) / ONE_SEC_2_US;

		if (EDCCATime[i] != 0)
			EdccaOccupyPercentage[i] = (EDCCATime[i] * 100) / ONE_SEC_2_US;

		if (MyTxAirTime[i] + MyRxAirTime[i] != 0)
			MyTxAirOccupyPercentage[i] = (MyTxAirTime[i] * 100) / (MyTxAirTime[i] + MyRxAirTime[i]);

		/* MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, */
		/* "Band%d OBSSAirtime=%d, MyAirtime=%d, EdccaTime=%d, MyTxAirtime=%d, MyRxAirtime=%d\n", */
		/* i, ObssAirOccupyPercentage[i], MyAirOccupyPercentage[i], EdccaOccupyPercentage[i], MyTxAirTime[i], MyRxAirTime[i]); */

		if ((ObssAirOccupyPercentage[i] > OBSS_OCCUPY_PERCENT_HIGH_TH)
			&& (ObssAirOccupyPercentage[i] + MyAirOccupyPercentage[i] > ALL_AIR_OCCUPY_PERCENT)
			&& MyTxAirOccupyPercentage[i] > TX_RATIO_TH
			&& MyAirOccupyPercentage[i] > My_OCCUPY_PERCENT) {
			if (pAd->CCI_ACI_TxOP_Value[i] == 0) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "CCI detected !!!!! Apply TxOP=FE\n");

				for (j = 0; j < WDEV_NUM_MAX; j++) {
					pWdev = pAd->wdev_list[j];

					if ((!pWdev) || (!HcIsRadioAcq(pWdev)))
						continue;

					BandIdx = HcGetBandByWdev(pWdev);

					if (BandIdx == i) {
						pAd->CCI_ACI_TxOP_Value[i] = TXOP_FE;
						enable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_FE);
						break;
					}
				}
			}
		} else if ((EdccaOccupyPercentage[i] > OBSS_OCCUPY_PERCENT_HIGH_TH)
				   && (EdccaOccupyPercentage[i] + MyAirOccupyPercentage[i] > ALL_AIR_OCCUPY_PERCENT)
				   && MyTxAirOccupyPercentage[i] > TX_RATIO_TH
				   && MyAirOccupyPercentage[i] > My_OCCUPY_PERCENT) {
			if (pAd->CCI_ACI_TxOP_Value[i] == 0) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "ACI detected !!!!! Apply TxOP=FE\n");

				for (j = 0; j < WDEV_NUM_MAX; j++) {
					pWdev = pAd->wdev_list[j];

					if ((!pWdev) || (!HcIsRadioAcq(pWdev)))
						continue;

					BandIdx = HcGetBandByWdev(pWdev);

					if (BandIdx == i) {
						pAd->CCI_ACI_TxOP_Value[i] = TXOP_FE;
						enable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_FE);
						break;
					}
				}
			}
		} else {
			if (pAd->CCI_ACI_TxOP_Value[i] != 0) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "NO CCI /ACI detected !!!!! Apply TxOP=0\n");

				for (j = 0; j < WDEV_NUM_MAX; j++) {
					pWdev = pAd->wdev_list[j];

					if ((!pWdev) || (!HcIsRadioAcq(pWdev)))
						continue;

					BandIdx = HcGetBandByWdev(pWdev);

					if (BandIdx == i) {
						pAd->CCI_ACI_TxOP_Value[i] = TXOP_0;
						disable_tx_burst(pAd, pWdev, AC_BE, PRIO_CCI, TXOP_0);
						break;
					}
				}
			}
		}

		/* Reset_OBSS_AirTime(pAd,i); */
	}
}

/*
	detect AC Category of trasmitting packets
	to turn AC0(BE) TX_OP (MAC reg 0x1300)
*/
/* TODO: shiang-usw, this function should move to other place!! */
VOID detect_wmm_traffic(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	IN UCHAR UserPriority,
	IN UCHAR FlgIsOutput)
{
	if (!pAd)
		return;

	/* For BE & BK case and TxBurst function is disabled */
	if ((pAd->CommonCfg.bEnableTxBurst == FALSE)
#ifdef DOT11_N_SUPPORT
		&& (pAd->CommonCfg.bRdg == FALSE)
		&& (pAd->CommonCfg.bRalinkBurstMode == FALSE)
#endif /* DOT11_N_SUPPORT */
		&& (FlgIsOutput == 1)
	   ) {
		if (WMM_UP2AC_MAP[UserPriority] == QID_AC_BK) {
			/* has any BK traffic */
			if (pAd->flg_be_adjust == 0) {
				/* yet adjust */
				/* TODO: here need YF check! */
				/* RTMP_SET_TX_BURST(pAd, wdev); */
				pAd->flg_be_adjust = 1;
				NdisGetSystemUpTime(&pAd->be_adjust_last_time);
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wmm> adjust be!\n");
			}
		} else {
			if (pAd->flg_be_adjust != 0) {
				QUEUE_HEADER *pQueue;
				/* has adjusted */
				pQueue = &pAd->TxSwQueue[QID_AC_BK];

				if ((pQueue == NULL) ||
					((pQueue != NULL) && (pQueue->Head == NULL))) {
					ULONG	now;

					NdisGetSystemUpTime(&now);

					if ((now - pAd->be_adjust_last_time) > TIME_ONE_SECOND) {
						/* no any BK traffic */
						/* TODO: here need YF check! */
						/* RTMP_SET_TX_BURST(pAd, wdev); */
						pAd->flg_be_adjust = 0;
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wmm> recover be!\n");
					}
				} else
					NdisGetSystemUpTime(&pAd->be_adjust_last_time);
			}
		}
	}

	/* count packets which priority is more than BE */
	if (UserPriority > 3) {
		pAd->tx_OneSecondnonBEpackets++;

#ifdef CONFIG_AP_SUPPORT
		if (pAd->tx_OneSecondnonBEpackets > 100
#ifdef DOT11_N_SUPPORT
			&& pAd->MacTab.fAnyStationMIMOPSDynamic
#endif /* DOT11_N_SUPPORT */
		   ) {
			if (!pAd->is_on) {
				RTMP_AP_ADJUST_EXP_ACK_TIME(pAd);
				pAd->is_on = 1;
				/*notify for wmm detect on*/
				call_traffic_notifieriers(TRAFFIC_NOTIFY_WMM_DETECT, pAd, &pAd->is_on);
			}
		} else {
			if (pAd->is_on) {
				RTMP_AP_RECOVER_EXP_ACK_TIME(pAd);
				pAd->is_on = 0;
				/*notify for wmm detect off*/
				call_traffic_notifieriers(TRAFFIC_NOTIFY_WMM_DETECT, pAd, &pAd->is_on);
			}
		}
#endif /*CONFIG_AP_SUPPORT*/
	}
}

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
extern BOOLEAN hotspot_rx_handler(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk);
#ifdef CONFIG_HOTSPOT_R3
extern BOOLEAN hotspot_osu_data_handler(RTMP_ADAPTER * pAd, MAC_TABLE_ENTRY * pEntry, RX_BLK * pRxBlk, struct wifi_dev *wdev);
#endif
#endif /* CONFIG_HOTSPOT */

#ifdef AIR_MONITOR
extern VOID Air_Monitor_Pkt_Report_Action(PRTMP_ADAPTER pAd, UINT16 wcid, RX_BLK *pRxBlk);
#endif /* AIR_MONITOR */

/* TODO: shiang-usw, temporary put this function here, should remove to other place or re-write! */
VOID Update_Snr_Sample(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE * pRssi,
	IN struct rx_signal_info *signal,
	IN UCHAR phy_mode,
	IN UCHAR bw)
{
	BOOLEAN bInitial = FALSE;
	INT ant_idx, ant_max = 4;

	if (!(pRssi->AvgRssi[0] | pRssi->AvgRssiX8[0] | pRssi->LastRssi[0]))
		bInitial = TRUE;

	/* TODO: shiang-usw, shall we check this here to reduce the for loop count? */
	if (ant_max > pAd->Antenna.field.RxPath)
		ant_max = pAd->Antenna.field.RxPath;

	for (ant_idx = 0; ant_idx < ant_max; ant_idx++) {
		if (signal->raw_snr[ant_idx] != 0 && phy_mode != MODE_CCK) {
			pRssi->LastSnr[ant_idx] = ConvertToSnr(pAd, signal->raw_snr[ant_idx]);

			if (bInitial) {
				pRssi->AvgSnrX8[ant_idx] = pRssi->LastSnr[ant_idx] * 8;
				pRssi->AvgSnr[ant_idx] = pRssi->LastSnr[ant_idx];
			} else
				pRssi->AvgSnrX8[ant_idx] = (pRssi->AvgSnrX8[ant_idx] - pRssi->AvgSnr[ant_idx]) + pRssi->LastSnr[ant_idx];

			pRssi->AvgSnr[ant_idx] = pRssi->AvgSnrX8[ant_idx] / 8;
		}
	}
}

VOID Update_Rssi_Sample(
	IN RTMP_ADAPTER * pAd,
	IN CHAR * rssi_last,
	IN CHAR * rssi_avg,
	IN UINT8 ant_num,
	IN struct rx_signal_info *signal)
{
	INT ant_idx;
	INT16 rssi_tmp = 0;

	for (ant_idx = 0; ant_idx < ant_num; ant_idx++) {
		*(rssi_last + ant_idx) = signal->raw_rssi[ant_idx];
		rssi_tmp = ((*(rssi_avg + ant_idx) * 7) + (*(rssi_last + ant_idx))) / 8;
		*(rssi_avg + ant_idx) = (CHAR) rssi_tmp;
	}
}

UINT deaggregate_amsdu_announce(
	IN RTMP_ADAPTER *pAd,
	PNDIS_PACKET pPacket,
	IN UCHAR *pData,
	IN ULONG DataSize,
	IN UCHAR OpMode)
{
	USHORT PayloadSize;
	USHORT SubFrameSize;
	HEADER_802_3 *pAMSDUsubheader;
	UINT nMSDU;
	UCHAR Header802_3[14];
	UCHAR *pPayload, *pDA, *pSA, *pRemovedLLCSNAP;
	PNDIS_PACKET pClonePacket;
	struct wifi_dev *wdev;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
	UCHAR VLAN_Size;
#ifdef CONFIG_AP_SUPPORT
	USHORT VLAN_VID = 0;
	USHORT VLAN_Priority = 0;
#endif

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s():invalud wdev_idx(%d)\n", __func__, wdev_idx);
		tr_cnt->rx_invalid_wdev++;
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		return 0;
	}

	wdev = pAd->wdev_list[wdev_idx];
	/* only MBssid support VLAN.*/
	VLAN_Size = (wdev->VLAN_VID != 0) ? LENGTH_802_1Q : 0;
	nMSDU = 0;

	while (DataSize > LENGTH_802_3) {
		nMSDU++;
		/*hex_dump("subheader", pData, 64);*/
		pAMSDUsubheader = (PHEADER_802_3)pData;
		/*pData += LENGTH_802_3;*/
		PayloadSize = pAMSDUsubheader->Octet[1] + (pAMSDUsubheader->Octet[0] << 8);
		SubFrameSize = PayloadSize + LENGTH_802_3;

		if ((DataSize < SubFrameSize) || (PayloadSize > 1518))
			break;

		pPayload = pData + LENGTH_802_3;
		pDA = pData;
		pSA = pData + MAC_ADDR_LEN;
		/* convert to 802.3 header*/
		CONVERT_TO_802_3(Header802_3, pDA, pSA, pPayload, PayloadSize, pRemovedLLCSNAP);

#ifdef CONFIG_STA_SUPPORT
		if ((Header802_3[12] == 0x88) && (Header802_3[13] == 0x8E)
		   ) {
			MLME_QUEUE_ELEM *Elem;

			os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

			if (Elem != NULL) {
				struct wifi_dev_ops *ops = wdev->wdev_ops;
				MAC_TABLE_ENTRY *pEntry;

				ops->mac_entry_lookup(pAd, pSA, wdev, &pEntry);

				ASSERT(pEntry);

				if (pEntry) {
					memmove(Elem->Msg + (LENGTH_802_11 + LENGTH_802_1_H), pPayload, PayloadSize);
					Elem->MsgLen = LENGTH_802_11 + LENGTH_802_1_H + PayloadSize;
					REPORT_MGMT_FRAME_TO_MLME(pAd, pEntry->wcid, Elem->Msg,
						Elem->MsgLen, 0, 0, 0, 0, 0, 0,
						OPMODE_STA, wdev, pEntry->HTPhyMode.field.MODE);
				}

				os_free_mem(Elem);
			}
		}

#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pRemovedLLCSNAP) {
				pPayload -= (LENGTH_802_3 + VLAN_Size);
				PayloadSize += (LENGTH_802_3 + VLAN_Size);
			} else {
				pPayload -= VLAN_Size;
				PayloadSize += VLAN_Size;
			}

			WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);
			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									 Header802_3, LENGTH_802_3, pPayload,
									 TPID);
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (pRemovedLLCSNAP) {
				pPayload -= LENGTH_802_3;
				PayloadSize += LENGTH_802_3;
				NdisMoveMemory(pPayload, &Header802_3[0], LENGTH_802_3);
			}
		}
#endif /* CONFIG_STA_SUPPORT */

		pClonePacket = ClonePacket(MONITOR_ON(pAd), wdev->if_dev, pPacket, pPayload, PayloadSize);

		if (pClonePacket) {
			UCHAR opmode = pAd->OpMode;
			announce_or_forward_802_3_pkt(pAd, pClonePacket, wdev, opmode);
		}

		/* A-MSDU has padding to multiple of 4 including subframe header.*/
		/* align SubFrameSize up to multiple of 4*/
		SubFrameSize = (SubFrameSize + 3) & (~0x3);

		if (SubFrameSize > 1528 || SubFrameSize < 32)
			break;

		if (DataSize > SubFrameSize) {
			pData += SubFrameSize;
			DataSize -= SubFrameSize;
		} else {
			/* end of A-MSDU*/
			DataSize = 0;
		}
	}

	/* finally release original rx packet*/
	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
	return nMSDU;
}

VOID indicate_amsdu_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	UINT nMSDU;
	struct _RXD_BASE_STRUCT *rx_base;
	tr_cnt->rx_sw_amsdu_call++;
	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:drop packet by PN mismatch!\n", __FUNCTION__);
		tr_cnt->rx_pn_mismatch++;
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
	RTMP_UPDATE_OS_PACKET_INFO(pAd, pRxBlk, wdev_idx);
	RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);
	rx_base = (struct _RXD_BASE_STRUCT *)pRxBlk->rmac_info;
	tr_cnt->rx_sw_amsdu_last_len = pRxBlk->DataSize;
#if defined(MT_DMAC)
	if (IS_MT7663(pAd) || IS_MT7626(pAd) || IS_MT7615(pAd)  || IS_MT7622(pAd)) {
	/* this is for mt_dmac define , and mtf_trans_rxd_into_rxblk() of mt_fmac already skip the HO (Header Offset) */
		if ((rx_base->RxD1.HdrOffset == 1) && (rx_base->RxD1.PayloadFmt != 0) && (rx_base->RxD1.HdrTranslation == 0)) {
			pRxBlk->pData += 2;
			pRxBlk->DataSize -= 2;
		}
	}
#endif
	nMSDU = deaggregate_amsdu_announce(pAd, pRxBlk->pRxPacket, pRxBlk->pData, pRxBlk->DataSize, pRxBlk->OpMode);
	tr_cnt->rx_sw_amsdu_num = nMSDU;
}

VOID announce_or_forward_802_3_pkt(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pPacket,
	struct wifi_dev *wdev,
	UCHAR op_mode)
{
	BOOLEAN to_os = FALSE;
	struct wifi_dev_ops *ops = NULL;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#ifdef MAP_TS_TRAFFIC_SUPPORT
	MAC_TABLE_ENTRY *peer_entry = NULL;
	UINT16 Wcid = RTMP_GET_PACKET_WCID(pPacket);
#endif
#ifdef MAP_R3
	UCHAR up = 0;
#endif

	if (!wdev) {
		tr_cnt->rx_to_os_drop++;
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return;
	}

	ops = wdev->wdev_ops;

#ifdef MAP_TS_TRAFFIC_SUPPORT
	if (pAd->bTSEnable) {
		if (VALID_UCAST_ENTRY_WCID(pAd, Wcid))
			peer_entry = &pAd->MacTab.Content[Wcid];

		if (wdev && peer_entry && !map_ts_rx_process(pAd, wdev, pPacket, peer_entry)) {
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
#endif

#ifdef VLAN_SUPPORT
	if (pAd->CommonCfg.bEnableVlan) {
		if (wdev && IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pPacket))) {
			UINT16 tci = (GET_OS_PKT_DATAPTR(pPacket)[14]<<8) | (GET_OS_PKT_DATAPTR(pPacket)[15]);
			UINT16 vid = tci & MASK_TCI_VID;
			RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			if (wdev->VLAN_VID != 0) {
				if (vid != 0 && vid != wdev->VLAN_VID) {
					switch (wdev->VLAN_Policy[RX_VLAN]) {
					case VLAN_RX_DROP:
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
						wdev->VLANRxdrop++;
						return;
					case VLAN_RX_REPLACE_VID:
						if (cap->vlan_rx_tag_mode == VLAN_RX_TAG_SW_MODE) {
							tci &= MASK_CLEAR_TCI_VID;
							tci |= wdev->VLAN_VID;
							GET_OS_PKT_DATAPTR(pPacket)[14] = tci >> 8;
							GET_OS_PKT_DATAPTR(pPacket)[15] = tci & 0xff;
						}
						break;
					case VLAN_RX_REPLACE_ALL:
						if (cap->vlan_rx_tag_mode == VLAN_RX_TAG_SW_MODE) {
							tci &= MASK_CLEAR_TCI_VID;
							tci &= MASK_CLEAR_TCI_PCP;
							tci |= wdev->VLAN_VID;
							tci |= wdev->VLAN_Priority << (CFI_LEN + VID_LEN);
							GET_OS_PKT_DATAPTR(pPacket)[14] = tci >> 8;
							GET_OS_PKT_DATAPTR(pPacket)[15] = tci & 0xff;
						}
						break;
					case VLAN_RX_ALLOW:
					default:
						break;
					}
				}
			}
		} else if (wdev && wdev->VLAN_VID != 0 &&
				(wdev->VLAN_Policy[RX_VLAN] == VLAN_RX_REPLACE_VID
				 || wdev->VLAN_Policy[RX_VLAN] == VLAN_RX_REPLACE_ALL)) {
			RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
			UINT16 tci;

			if (cap->vlan_rx_tag_mode == VLAN_RX_TAG_SW_MODE) {
				tci = (wdev->VLAN_Priority<<(CFI_LEN + VID_LEN)) | wdev->VLAN_VID;
				pPacket = RtmpOsVLANInsertTag(pPacket, tci);
				if (!pPacket) {
					tr_cnt->rx_to_os_drop++;
					return;
				}
			}
		}
	}
#endif /*VLAN_SUPPORT*/

	if (ops->rx_pkt_foward)
		to_os = ops->rx_pkt_foward(pAd, wdev, pPacket);

	if (to_os == TRUE) {
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

		if (wf_drv_tbl.wf_fwd_needed_hook != NULL && wf_drv_tbl.wf_fwd_needed_hook() == TRUE)
			set_wf_fwd_cb(pAd, pPacket, wdev);

#endif /* CONFIG_WIFI_PKT_FWD */
#ifdef DABS_QOS
		if (pAd->dabs_qos_op & DABS_SET_QOS_PARAM) {
			dabs_active_qos_by_ipaddr(pAd, pPacket);
		}
#endif

#ifdef MAP_R3
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd)) {
			if (VALID_UCAST_ENTRY_WCID(pAd, Wcid))
				peer_entry = &pAd->MacTab.Content[Wcid];
			if (peer_entry) {
				up = RTMP_GET_PACKET_UP(pPacket);
				up |= peer_entry->profile << 4;
				RTMP_SET_PACKET_UP(pPacket, up);
			}
		}
#endif
		announce_802_3_packet(pAd, pPacket, op_mode);
	} else {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): No need to send to OS!\n", __func__);
		tr_cnt->rx_to_os_drop++;
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}


VOID indicate_802_3_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#if defined(CONFIG_AP_SUPPORT) && defined(MAC_REPEATER_SUPPORT)
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
#endif

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:drop packet by PN mismatch!\n", __FUNCTION__);
		tr_cnt->rx_pn_mismatch++;
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */

		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR)) {
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
#ifdef VLAN_SUPPORT
			if ((pAd->CommonCfg.bEnableVlan) && IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pRxPacket)))
				pLayerHdr = (pPktHdr + MAT_VLAN_ETH_HDR_LEN);
#endif /*VLAN_SUPPORT*/

			/*For UDP packet, we need to check about the DHCP packet. */
			if (*(pLayerHdr + 9) == 0x11) {
				PUCHAR pUdpHdr;
				UINT16 srcPort, dstPort;
				BOOLEAN bHdrChanged = FALSE;

				pUdpHdr = pLayerHdr + 20;
				srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
				dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr + 2)));

				if (srcPort == 67 && dstPort == 68) { /*It's a DHCP packet */
					PUCHAR bootpHdr, dhcpHdr, pCliHwAddr;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					bootpHdr = pUdpHdr + 8;
					dhcpHdr = bootpHdr + 236;
					pCliHwAddr = (bootpHdr + 28);
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr, TRUE);

					if (pReptEntry) {
						if (pReptEntry->CliValid != TRUE) {
							tr_cnt->rx_to_os_drop++;
							RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
							return;
						}
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
					}

					bHdrChanged = TRUE;
				}

				if (bHdrChanged == TRUE) {
					NdisZeroMemory((pUdpHdr + 6), 2); /*modify the UDP chksum as zero */
#ifdef DHCP_UC_SUPPORT
					*(UINT16 *)(pUdpHdr + 6) = RTMP_UDP_Checksum(pRxPacket);
#endif /* DHCP_UC_SUPPORT */
				}
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
	pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);
	if (pOSPkt->dev == NULL) {
		tr_cnt->rx_to_os_drop++;
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	} else {
		SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
		announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev_search_by_idx(pAd, wdev_idx), pAd->OpMode);
	}
}

VOID indicate_802_11_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	UCHAR Header802_3[LENGTH_802_3];
	USHORT VLAN_VID = 0, VLAN_Priority = 0;
	UINT max_pkt_len = MAX_RX_PKT_LEN;
	UCHAR *pData = pRxBlk->pData;
	INT data_len = pRxBlk->DataSize;
	struct wifi_dev *wdev;
	UCHAR opmode = pAd->OpMode;

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s():invalid wdev_idx(%d)!\n", __func__, wdev_idx);
		tr_cnt->rx_invalid_wdev++;
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:drop packet by PN mismatch!\n", __FUNCTION__);
		tr_cnt->rx_pn_mismatch++;
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	wdev = pAd->wdev_list[wdev_idx];

	if (0)
		hex_dump("indicate_802_11_pkt", pData, data_len);

	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
#ifdef HDR_TRANS_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		max_pkt_len = 1514;
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	} else
#endif /* HDR_TRANS_SUPPORT */
	{
		if (pRxBlk->AmsduState == 0) {
			RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);

		} else {
			/* Update the DA and SA from AMSDU Header instead of 80211 HDR */
			PUCHAR _pRemovedLLCSNAP = NULL, _pDA = NULL, _pSA = NULL;

			_pDA = pRxBlk->pData - LENGTH_802_3;
			_pSA = pRxBlk->pData - 8; /* MAC_ADDR_LEN + PAD */

			CONVERT_TO_802_3(Header802_3, _pDA, _pSA, pRxBlk->pData,
				pRxBlk->DataSize, _pRemovedLLCSNAP);
		}
	}
	/* hex_dump("802_3_hdr", (UCHAR *)Header802_3, LENGTH_802_3); */
	pData = pRxBlk->pData;
	data_len = pRxBlk->DataSize;

	if (data_len > max_pkt_len) {
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

		/* santiy the dump condition only if inf on or linkup cases */
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
			BOOLEAN dump_msg  = FALSE;
			if (wdev) {
				/* STA may up but link down */
				if (wdev->wdev_type == WDEV_TYPE_STA) {
					PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
					if (pStaCfg) {
						if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
							wdev->rx_drop_long_len++;
							dump_msg = TRUE;
						}
					}
				} else {
					if (wdev->if_up_down_state == TRUE) {
						wdev->rx_drop_long_len++;
						dump_msg = TRUE;
					}
				}

				if (dump_msg == TRUE &&
					(wdev->rx_drop_long_len < 1)) { /* dump fewer msg to prevent host busy */
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s(): wdev_type=%x, func_idx=%d, data_len(%d) > max_pkt_len(%d)!\n",
						__func__, wdev->wdev_type, wdev->func_idx, data_len, max_pkt_len);
				}
			}
		}
		return;
	}

	STATS_INC_RX_PACKETS(pAd, wdev_idx);
#ifdef CONFIG_AP_SUPPORT
	WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);
#endif /* CONFIG_AP_SUPPORT */

	/* +++Add by shiang for debug */
	if (0) {
		hex_dump("Before80211_2_8023", pData, data_len);
		hex_dump("header802_3", &Header802_3[0], LENGTH_802_3);
	}

	/* ---Add by shiang for debug */
#ifdef HDR_TRANS_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxPacket);

		pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);
		pOSPkt->data = pRxBlk->pTransData;
		pOSPkt->len = pRxBlk->TransDataSize;
		SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
		/* printk("%s: rx trans ...%d\n", __func__, __LINE__); */
	} else
#endif /* HDR_TRANS_SUPPORT */
	{
		RTMP_SET_PACKET_WCID(pRxPacket, pRxBlk->wcid);
		RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
								pRxBlk, Header802_3, wdev_idx, TPID);
	}

#ifndef APCLI_AS_WDS_STA_SUPPORT
	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */

		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR)) {
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);

			/*For UDP packet, we need to check about the DHCP packet. */
			if (*(pLayerHdr + 9) == 0x11) {
				PUCHAR pUdpHdr;
				UINT16 srcPort, dstPort;
				BOOLEAN bHdrChanged = FALSE;

				pUdpHdr = pLayerHdr + 20;
				srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
				dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr + 2)));

				if (srcPort == 67 && dstPort == 68) { /*It's a DHCP packet */
					PUCHAR bootpHdr, dhcpHdr, pCliHwAddr;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					bootpHdr = pUdpHdr + 8;
					dhcpHdr = bootpHdr + 236;
					pCliHwAddr = (bootpHdr + 28);
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr, TRUE);

					if (pReptEntry) {
						ASSERT(pReptEntry->CliValid == TRUE);
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
					}

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
					else {
						VOID *opp_band_tbl = NULL;
						VOID *band_tbl = NULL;
						VOID *other_band_tbl = NULL;

						if (wf_drv_tbl.wf_fwd_feedback_map_table)
							wf_drv_tbl.wf_fwd_feedback_map_table(pAd,
												&band_tbl,
												&opp_band_tbl,
												&other_band_tbl);

						if (opp_band_tbl != NULL) {
							/*
								check the ReptTable of the opposite band due to dhcp packet (BC)
									may come-in 2/5G band when STA send dhcp broadcast to Root AP
							*/
							pReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pCliHwAddr, FALSE);

							if (pReptEntry)
								NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
						} else
							MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "cannot find the adapter of the oppsite band\n");

						if (other_band_tbl != NULL) {
							pReptEntry = RTMPLookupRepeaterCliEntry(other_band_tbl, FALSE, pCliHwAddr, FALSE);

							if (pReptEntry)
								NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
						} else
							MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "cannot find the adapter of the othersite band\n");
					}

#endif /* CONFIG_WIFI_PKT_FWD */
					bHdrChanged = TRUE;
				}

				if (bHdrChanged == TRUE)
					NdisZeroMemory((pUdpHdr + 6), 2); /*modify the UDP chksum as zero */
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* APCLI_AS_WDS_STA_SUPPORT */

#ifdef P2P_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA))
		WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);

	opmode = pRxBlk->OpMode;
#endif /* P2P_SUPPORT */

	if (0)
		hex_dump("After80211_2_8023", GET_OS_PKT_DATAPTR(pRxPacket), GET_OS_PKT_LEN(pRxPacket));

	announce_or_forward_802_3_pkt(pAd, pRxPacket, wdev, opmode);
}

VOID indicate_agg_ralink_pkt(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	UCHAR Header802_3[LENGTH_802_3];
	UINT16 Msdu2Size;
	UINT16 Payload1Size, Payload2Size;
	PUCHAR pData2;
	PNDIS_PACKET pPacket2 = NULL;
	USHORT VLAN_VID = 0, VLAN_Priority = 0;
	UCHAR opmode = pAd->OpMode;
	struct wifi_dev *wdev;

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s():Invalid wdev_idx(%d)\n", __func__, wdev_idx);
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	wdev = pAd->wdev_list[wdev_idx];
	Msdu2Size = *(pRxBlk->pData) + (*(pRxBlk->pData + 1) << 8);

	if ((Msdu2Size <= 1536) && (Msdu2Size < pRxBlk->DataSize)) {
		/* skip two byte MSDU2 len */
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	} else {
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* get 802.3 Header and  remove LLC*/
	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
	ASSERT(pRxBlk->pRxPacket);

	if (!pRxBlk->pRxPacket) {
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pAd->RalinkCounters.OneSecRxARalinkCnt++;
	Payload1Size = pRxBlk->DataSize - Msdu2Size;
	Payload2Size = Msdu2Size - LENGTH_802_3;
	pData2 = pRxBlk->pData + Payload1Size + LENGTH_802_3;
	pPacket2 = duplicate_pkt_vlan(wdev->if_dev,
								  wdev->VLAN_VID, wdev->VLAN_Priority,
								  (pData2 - LENGTH_802_3), LENGTH_802_3,
								  pData2, Payload2Size, TPID);

	if (!pPacket2) {
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* update payload size of 1st packet*/
	pRxBlk->DataSize = Payload1Size;
	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, wdev_idx, TPID);
#ifdef P2P_SUPPORT
	opmode = pRxBlk->OpMode;
#endif /* P2P_SUPPORT */
	announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev, opmode);

	if (pPacket2)
		announce_or_forward_802_3_pkt(pAd, pPacket2, wdev, opmode);
}

VOID indicate_ampdu_pkt(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, UCHAR wdev_idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;

	if (!RX_BLK_TEST_FLAG(rx_blk, fRX_AMSDU) &&
			(rx_blk->DataSize > MAX_RX_PKT_LEN)) {
		static int err_size;

		err_size++;

		if (err_size > 20) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
				"AMPDU DataSize = %d\n", rx_blk->DataSize);
			hex_dump("802.11 Header", rx_blk->FC, 24);
			hex_dump("Payload", rx_blk->pData, 64);
			err_size = 0;
		}

		tr_cnt->rx_invalid_pkt_len++;
		RELEASE_NDIS_PACKET(pAd, rx_blk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

		ba_reorder(pAd, rx_blk, wdev_idx);
}

VOID de_fragment_data_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	PNDIS_PACKET pRetPacket = NULL;
	UCHAR *pFragBuffer = NULL;
	BOOLEAN bReassDone = FALSE;
	UCHAR HeaderRoom = 0;
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 RXWISize = cap->RXWISize;
	/* TODO: shiang-MT7603, fix me for this function work in MT series chips */
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		RXWISize = 0;

#endif /* MT_MAC */
	HeaderRoom = pData - pRxBlk->FC;

	MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "de_fragment_data_pkt: SN:%d,"
		"FN:%d,PN:%llu, sec_mode: %d, cm:%d \n",
		pRxBlk->SN, pRxBlk->FN, pRxBlk->CCMP_PN, pRxBlk->sec_mode,
		(RX_BLK_TEST_FLAG(pRxBlk, fRX_CM) ? 1 : 0));

	/* Re-assemble the fragmented packets*/
	if (pRxBlk->FN == 0) {
		/* Frag. Number is 0 : First frag or only one pkt*/
		/* the first pkt of fragment, record it.*/
		if (FC->MoreFrag && pAd->FragFrame.pFragPacket) {
			pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);
#ifdef HDR_TRANS_RX_SUPPORT
			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
				pAd->FragFrame.RxSize = DataSize + RXWISize;
				NdisMoveMemory(pFragBuffer, pRxWI, RXWISize);
				NdisMoveMemory(pFragBuffer + RXWISize, pData, pAd->FragFrame.RxSize);
				pAd->FragFrame.Header_802_3 = TRUE;
			} else
#endif /* HDR_TRANS_RX_SUPPORT */
			{
				pAd->FragFrame.RxSize = DataSize + HeaderRoom + RXWISize;
				NdisMoveMemory(pFragBuffer, pRxWI, RXWISize);
				NdisMoveMemory(pFragBuffer + RXWISize, FC, pAd->FragFrame.RxSize - RXWISize);
				pAd->FragFrame.Header_802_3 = FALSE;
			}

			pAd->FragFrame.Sequence = pRxBlk->SN;
			pAd->FragFrame.LastFrag = pRxBlk->FN;	   /* Should be 0*/
			pAd->FragFrame.wcid = pRxBlk->wcid;	   /* to tell from the fragment Buffer from which STA */
			if ((pRxBlk->sec_mode == WTBL_CIPHER_TKIP_MIC) ||
				(pRxBlk->sec_mode == WTBL_CIPHER_TKIP_NO_MIC) ||
				(pRxBlk->sec_mode == WTBL_CIPHER_CCMP_128_PMF) ||
				(pRxBlk->sec_mode == WTBL_CIPHER_CCMP_256) ||
				(pRxBlk->sec_mode == WTBL_CIPHER_GCMP_128) ||
				(pRxBlk->sec_mode == WTBL_CIPHER_GCMP_256)) {
				pAd->FragFrame.sec_on = TRUE;
				pAd->FragFrame.sec_mode = pRxBlk->sec_mode;
				pAd->FragFrame.LastPN = pRxBlk->CCMP_PN;
			}
#ifdef FRAG_ATTACK_SUPPORT
			if (IS_WCID_VALID(pAd, pRxBlk->wcid)) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				NdisCopyMemory(pAd->FragFrame.key, &pEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], 4);
			}
#endif
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Fragment PACKET INIT, %d,%d,%llu - %d,%d,%llu\n",
				pRxBlk->SN, pRxBlk->FN, pRxBlk->CCMP_PN,
				pAd->FragFrame.Sequence, pAd->FragFrame.LastFrag, pAd->FragFrame.LastPN);

			ASSERT(pAd->FragFrame.LastFrag == 0);
			goto done;	/* end of processing this frame*/
		} else if (!pAd->FragFrame.pFragPacket)
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ERR: pAd->FragFrame.pFragPacket is NULL.\n");
		} else {
#ifdef FRAG_ATTACK_SUPPORT
			MAC_TABLE_ENTRY *pEntry = NULL;
			UCHAR *frag_key = NULL;

			if (IS_WCID_VALID(pAd, pRxBlk->wcid)) {
				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
				if (pEntry)
					frag_key = &pEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK];
			}
#endif
		/*Middle & End of fragment*/
		if ((pRxBlk->SN != pAd->FragFrame.Sequence) ||
			(pRxBlk->FN != (pAd->FragFrame.LastFrag + 1)) ||
			((pAd->FragFrame.sec_on) && (pAd->FragFrame.sec_mode == pRxBlk->sec_mode) &&
			((pRxBlk->CCMP_PN != (pAd->FragFrame.LastPN + 1))
#ifdef FRAG_ATTACK_SUPPORT
			|| (pEntry && NdisCmpMemory(pAd->FragFrame.key, frag_key, 4))
#endif
			))) {
			/* Fragment is not the same sequence or out of fragment number order*/
			/* Reset Fragment control blk*/
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Fragment is not the same SN or out of FN order, %d,%d,%llu - %d,%d,%llu\n",
				pRxBlk->SN, pRxBlk->FN, pRxBlk->CCMP_PN, pAd->FragFrame.Sequence,
				pAd->FragFrame.LastFrag, pAd->FragFrame.LastPN);
#ifdef FRAG_ATTACK_SUPPORT
			if (frag_key) {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Fragment is not the same may be diff key, %02x %02x %02x %02x - %02x %02x %02x %02x\n",
					 pAd->FragFrame.key[0], pAd->FragFrame.key[1], pAd->FragFrame.key[2], pAd->FragFrame.key[3],
					 frag_key[0], frag_key[1], frag_key[2], frag_key[3]);
			}
#endif
			RESET_FRAGFRAME(pAd->FragFrame);

			goto done;
		}
		/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
		/* else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE) */
		else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE + RXWISize) {
			/* Fragment frame is too large, it exeeds the maximum frame size.*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Fragment frame is too large, it exeeds the maximum frame size.\n");
			goto done;
		}

		/* Broadcom AP(BCM94704AGR) will send out LLC in fragment's packet, LLC only can accpet at first fragment.*/
		/* In this case, we will drop it.*/
		if (NdisEqualMemory(pData, SNAP_802_1H, sizeof(SNAP_802_1H))) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Find another LLC at Middle or End fragment(SN=%d, Frag=%d)\n", pRxBlk->SN, pRxBlk->FN);
			goto done;
		}

		pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);
		/* concatenate this fragment into the re-assembly buffer*/
		NdisMoveMemory((pFragBuffer + pAd->FragFrame.RxSize), pData, DataSize);
		pAd->FragFrame.RxSize  += DataSize;
		pAd->FragFrame.LastFrag = pRxBlk->FN;	   /* Update fragment number*/
		pAd->FragFrame.LastPN = pRxBlk->CCMP_PN;

		/* Last fragment*/
		if (FC->MoreFrag == FALSE)
			bReassDone = TRUE;
	}

done:
	/* always release rx fragmented packet*/
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	pRxBlk->pRxPacket = NULL;

	/* return defragmented packet if packet is reassembled completely*/
	/* otherwise return NULL*/
	if (bReassDone) {
		PNDIS_PACKET pNewFragPacket;
		/* allocate a new packet buffer for fragment*/
		pNewFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);

		if (pNewFragPacket) {
			/* update RxBlk*/
			pRetPacket = pAd->FragFrame.pFragPacket;
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
			/* pRxBlk->pHeader = (PHEADER_802_11) GET_OS_PKT_DATAPTR(pRetPacket); */
			/* pRxBlk->pData = (UCHAR *)pRxBlk->pHeader + HeaderRoom; */
			/* pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom; */
			/* pRxBlk->pRxPacket = pRetPacket; */
			pRxBlk->pRxWI = (RXWI_STRUC *) GET_OS_PKT_DATAPTR(pRetPacket);
#ifdef HDR_TRANS_RX_SUPPORT

			if (pAd->FragFrame.Header_802_3) {
				pRxBlk->pData =	GET_OS_PKT_DATAPTR(pRetPacket) + RXWISize;
				pRxBlk->DataSize = pAd->FragFrame.RxSize - RXWISize;
			} else
#endif /* HDR_TRANS_RX_SUPPORT */
			{
				pRxBlk->pData =	GET_OS_PKT_DATAPTR(pRetPacket) + HeaderRoom + RXWISize;
				pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom - RXWISize;
			}

			pRxBlk->pRxPacket = pRetPacket;
#ifdef HDR_TRANS_RX_SUPPORT

			if (pAd->FragFrame.Header_802_3) {
				struct sk_buff *pOSPkt;

				pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);
				pOSPkt->data = pRxBlk->pData;
				pOSPkt->len = pRxBlk->DataSize;
				RX_BLK_SET_FLAG(pRxBlk, fRX_HDR_TRANS);
				pAd->FragFrame.Header_802_3 = FALSE;
			}

#endif /* HDR_TRANS_RX_SUPPORT */
			pAd->FragFrame.pFragPacket = pNewFragPacket;
		} else
			RESET_FRAGFRAME(pAd->FragFrame);
	}
}


VOID rx_eapol_frm_handle(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	UCHAR *pTmpBuf;
	BOOLEAN to_mlme = TRUE, to_daemon = FALSE;
	struct wifi_dev *wdev;
	unsigned char hdr_len = LENGTH_802_11;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	if (pEntry->wdev->wdev_type == WDEV_TYPE_STA) {
		pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
		ASSERT(pStaCfg);
	}
#endif

	if (wdev_idx >= WDEV_NUM_MAX)
		goto done;

	wdev = pAd->wdev_list[wdev_idx];

	if (pRxBlk->DataSize < (LENGTH_802_1_H + LENGTH_EAPOL_H)) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pkts size too small\n");
		goto done;
	} else if (!RTMPEqualMemory(SNAP_802_1H, pRxBlk->pData, 6)) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "no SNAP_802_1H parameter\n");
		goto done;
	} else if (!RTMPEqualMemory(EAPOL, pRxBlk->pData + 6, 2)) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"no EAPOL parameter\n");
		goto done;
	} else if (*(pRxBlk->pData + 9) > EAPOLASFAlert) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown EAP type(%d)\n", *(pRxBlk->pData + 9));
		goto done;
	}

#ifdef A4_CONN
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

#ifdef CONFIG_AP_SUPPORT

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef RT_CFG80211_SUPPORT

	if ((pEntry)
#ifndef APCLI_CFG80211_SUPPORT
		&& !(pStaCfg)
#endif
	) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG80211 EAPOL indicate_802_11_pkt\n");
#ifdef CONFIG_STA_SUPPORT

		/*
			Only decide to en-queue EAP-Request packet or not for Infra STA. @20140610
		*/
		if (pStaCfg && (pEntry->wdev == &pStaCfg->wdev)) {
			if (pStaCfg->wdev.bLinkUpDone)
				indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
			else {
				RX_BLK *pEapBlk = pStaCfg->wdev.pEapolPktFromAP;

				if (pEapBlk) {
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG80211(%s) bLinkUpDone = %d, Q this eapol packet\n",
							 __func__, pStaCfg->wdev.bLinkUpDone);
					pStaCfg->wdev.bGotEapolPkt = TRUE;
					NdisZeroMemory(pEapBlk, sizeof(RX_BLK));
					NdisMoveMemory(pEapBlk, pRxBlk, sizeof(RX_BLK));
					pEapBlk->pData = pRxBlk->pData;
					pEapBlk->FC = pRxBlk->FC;
					pEapBlk->pRxInfo = pRxBlk->pRxInfo;
					pEapBlk->pRxPacket = pRxBlk->pRxPacket;
					pEapBlk->pRxWI = pRxBlk->pRxWI;
					pEapBlk->rmac_info = pRxBlk->rmac_info;
#ifdef HDR_TRANS_SUPPORT
					pEapBlk->pTransData = pRxBlk->pTransData;
#endif /* HDR_TRANS_SUPPORT */
				} else
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"CFG80211(%s) pEapolPktFromAP is NULL\n", __func__);
			}
		} else
#endif /* CONFIG_STA_SUPPORT */
			indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);

		return;
	}

#endif /*RT_CFG80211_SUPPORT*/

	if (pEntry && IS_ENTRY_PEER_AP(pEntry)) {
#ifdef WPA_SUPPLICANT_SUPPORT
		WPA_SUPPLICANT_INFO *sup_info = NULL;
		CIPHER_KEY *share_key = NULL;
		int BssIdx;
#ifdef CONFIG_STA_SUPPORT
		/* ap/sta concurrent
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)*/
		if (pStaCfg) {
			sup_info = &pStaCfg->wpa_supplicant_info;
			share_key = &pAd->SharedKey[BSS0][0];
			BssIdx = BSS0;
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

		if (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
			STA_ADMIN_CONFIG *apcli_entry = &pAd->StaCfg[pEntry->func_tb_idx];

			sup_info = &apcli_entry->wpa_supplicant_info;
			share_key = &apcli_entry->SharedKey[0];
			BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + pEntry->func_tb_idx;
		}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		if ((sup_info != NULL) && (sup_info->WpaSupplicantUP)) {
			INT eapcode = WpaCheckEapCode(pAd, pRxBlk->pData, pRxBlk->DataSize, LENGTH_802_1_H);
			struct wifi_dev *wdev = pEntry->wdev;
			/*
				All EAPoL frames have to pass to upper layer (ex. WPA_SUPPLICANT daemon)

				For dynamic WEP(802.1x+WEP) mode, if the received frame is EAP-SUCCESS packet, turn
				on the PortSecured variable
			*/
			to_mlme = FALSE;
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "eapcode=%d\n", eapcode);

			if ((wdev->SecConfig.IEEE8021X == TRUE) && IS_CIPHER_WEP(wdev->SecConfig.PairwiseCipher) &&
				(eapcode == EAP_CODE_SUCCESS)) {
				UCHAR *Key;
				UCHAR CipherAlg;
				int idx = 0;

				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Receive EAP-SUCCESS Packet\n");
#ifdef CONFIG_AP_SUPPORT

				if (IS_ENTRY_PEER_AP(pEntry)) {
					STA_TR_ENTRY *tr_entry;

					tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
					STA_PORT_SECURED_BY_WDEV(pAd, pEntry->wdev);
				}
#endif /* CONFIG_STA_SUPPORT */

				if (sup_info->IEEE8021x_required_keys == FALSE) {
					idx = sup_info->DesireSharedKeyId;
					CipherAlg = sup_info->DesireSharedKey[idx].CipherAlg;
					Key = sup_info->DesireSharedKey[idx].Key;

					if (sup_info->DesireSharedKey[idx].KeyLen > 0) {
						USHORT Wcid = 0;
						ASIC_SEC_INFO Info = {0};

						GET_GroupKey_WCID(wdev, Wcid);
						/* Set key material to Asic */
						os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
						Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
						Info.Direction = SEC_ASIC_KEY_RX;
						Info.Wcid = Wcid;
						Info.BssIndex = BssIdx;
						Info.Cipher = SecEncryModeOldToNew(CipherAlg);
						Info.KeyIdx = idx;
						os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
						Info.Key.KeyLen = sup_info->DesireSharedKey[idx].KeyLen;
						os_move_mem(&Info.Key, &sup_info->DesireSharedKey[idx], 16);
						HW_ADDREMOVE_KEYTABLE(pAd, &Info);
#ifdef CONFIG_STA_SUPPORT
						IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
							RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
							pAd->ExtraInfo = GENERAL_LINK_UP;
						}
#endif /* CONFIG_STA_SUPPORT */
						/* For Preventing ShardKey Table is cleared by remove key procedure. */
						share_key[idx].CipherAlg = CipherAlg;
						share_key[idx].KeyLen = sup_info->DesireSharedKey[idx].KeyLen;
						NdisMoveMemory(share_key[idx].Key,
									   sup_info->DesireSharedKey[idx].Key,
									   sup_info->DesireSharedKey[idx].KeyLen);
					}
				}
			}

#ifdef CONFIG_STA_SUPPORT
			/* ap/sta concurrent
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)*/
			if (pStaCfg) {
#ifdef WSC_STA_SUPPORT

				/* report EAP packets to MLME to check this packet is WPS packet or not */
				if (pStaCfg->WscControl.WscState >= WSC_STATE_LINK_UP)
					to_mlme = TRUE;

#endif /* WSC_STA_SUPPORT */
			}
#endif /* CONFIG_STA_SUPPORT */

			if (IS_AKM_WPA1(pEntry->SecConfig.AKMMap) ||
				IS_AKM_WPA2(pEntry->SecConfig.AKMMap) ||
				IS_AKM_WPA3_192BIT(pEntry->SecConfig.AKMMap) ||
				(wdev->SecConfig.IEEE8021X == TRUE))
				to_daemon = TRUE;
		} else
#endif /* WPA_SUPPLICANT_SUPPORT */
		{
			to_mlme = TRUE;
			to_daemon = FALSE;
		}
	}

#ifdef CONFIG_AP_SUPPORT

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
#ifdef DOT1X_SUPPORT

		/* sent this frame to upper layer TCPIP */
		if ((pEntry->SecConfig.Handshake.WpaState < AS_INITPMK) &&
			(IS_AKM_WPA1(pEntry->SecConfig.AKMMap) ||
			 (IS_AKM_WPA2(pEntry->SecConfig.AKMMap) && (!is_pmkid_cache_in_sec_config(&pEntry->SecConfig))) ||
			 (IS_AKM_WPA3_192BIT(pEntry->SecConfig.AKMMap) && (!is_pmkid_cache_in_sec_config(&pEntry->SecConfig))) ||
			 IS_IEEE8021X(&pEntry->SecConfig))) {
			to_daemon = TRUE;
			to_mlme = FALSE;
#ifdef WSC_AP_SUPPORT

			/* report EAP packets to MLME to check this packet is WPS packet or not */
			if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscConfMode != WSC_DISABLE) &&
				(!MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.EntryAddr, ZERO_MAC_ADDR))) {
				to_mlme = TRUE;
				pTmpBuf = pRxBlk->pData - hdr_len;
				/* TODO: shiang-usw, why we need to change pHeader here?? */
				pRxBlk->FC = pTmpBuf;
			}

#endif /* WSC_AP_SUPPORT */
		} else
#endif /* DOT1X_SUPPORT */
		{
			/* sent this frame to WPA state machine */

			/*
				Check Addr3 (DA) is AP or not.
				If Addr3 is AP, forward this EAP packets to MLME
				If Addr3 is NOT AP, forward this EAP packets to upper layer or STA.
			*/
			UCHAR ap_idx = wdev->func_idx;

			if (wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_GO) {
				ASSERT(VALID_MBSS(pAd, ap_idx));
				ASSERT(wdev == (&pAd->ApCfg.MBSSID[ap_idx].wdev));
			}

			if (VALID_MBSS(pAd, ap_idx)) {
				/* TODO: shiang-usw, why we check this here?? */
				if ((wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_GO) &&
					(NdisEqualMemory(pRxBlk->Addr3, pAd->ApCfg.MBSSID[ap_idx].wdev.bssid, MAC_ADDR_LEN) == FALSE))
					to_daemon = TRUE;
				else
					to_mlme = TRUE;
			}
			else {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s() invalid mbss entry id(%d)\n", __func__, ap_idx);
				goto done;
			}
		}
	}

#endif /* CONFIG_AP_SUPPORT */

	/*
	   Special DATA frame that has to pass to MLME
	   1. Cisco Aironet frames for CCX2. We need pass it to MLME for special process
	   2. EAPOL handshaking frames when driver supplicant enabled, pass to MLME for special process
	 */
	if (to_mlme) {
		struct sk_buff *ptmpSkb = NULL;
		int length = pRxBlk->DataSize + hdr_len;

		DEV_ALLOC_SKB(ptmpSkb, length);
		if (ptmpSkb) {
			 NdisZeroMemory(ptmpSkb->head, length);

			 /* copy 802.11 header without Qos Contrl */
			 NdisCopyMemory(skb_put(ptmpSkb, hdr_len),
					(void *)(pRxBlk->FC), hdr_len);

			 /* copy frame body */
			 NdisCopyMemory(skb_put(ptmpSkb, pRxBlk->DataSize),
					(void *)(pRxBlk->pData), pRxBlk->DataSize);

			{
				REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
							  ptmpSkb->data,
							  ptmpSkb->len,
							  pRxBlk->rx_signal.raw_rssi[0],
							  pRxBlk->rx_signal.raw_rssi[1],
							  pRxBlk->rx_signal.raw_rssi[2],
							  pRxBlk->rx_signal.raw_rssi[3],
							  0,
							  pRxBlk->channel_freq,
							  pRxBlk->OpMode,
							  wdev,
							  pRxBlk->rx_rate.field.MODE);
			}
			 MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "!!! report EAPOL DATA to MLME (len=%d) !!!\n",
					  pRxBlk->DataSize);
			/* free skb */
			consume_skb(ptmpSkb);
		} else {
			 MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "!!! report EAPOL DATA to MLME FAILED !!!\n");
		}
	}

	if (to_daemon == TRUE) {
		indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
		return;
	}

done:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}

VOID indicate_eapol_pkt(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "indicate_eapol_pkt: invalid pAd.\n");
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "indicate_eapol_pkt: invalid wcid.\n");
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:drop packet by PN mismatch!\n", __FUNCTION__);
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "indicate_eapol_pkt: drop and release the invalid packet.\n");
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	rx_eapol_frm_handle(pAd, pEntry, pRxBlk, wdev_idx);
	return;
}

VOID indicate_rx_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU) && (tr_ctl->damsdu_type == RX_SW_AMSDU))
		indicate_amsdu_pkt(pAd, pRxBlk, wdev_idx);
	else if (RX_BLK_TEST_FLAG(pRxBlk, fRX_EAP))
		indicate_eapol_pkt(pAd, pRxBlk, wdev_idx);
	else if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
		indicate_802_3_pkt(pAd, pRxBlk, wdev_idx);
	else
		indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
}

static MAC_TABLE_ENTRY *research_entry(RTMP_ADAPTER *pAd, RX_BLK *rx_blk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pEntry = MacTableLookup(pAd, rx_blk->Addr2);
#endif

#ifdef MAC_REPEATER_SUPPORT

	if (pEntry) {
		if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
			(IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_PEER_AP(pEntry))) {
			REPEATER_CLIENT_ENTRY	*pReptEntry = NULL;

			pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &rx_blk->Addr1[0], TRUE);

			if (pReptEntry) {
				if (pReptEntry->pMacEntry && IS_WCID_VALID(pAd, pReptEntry->pMacEntry->wcid)) {
					rx_blk->wcid = pReptEntry->pMacEntry->wcid;
					pEntry = pReptEntry->pMacEntry;
				}
			}
		}
	}

#endif

#ifdef CONFIG_STA_SUPPORT
	/* TODO: broadcast frame match 1st network interface only */
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	pEntry = MacTableLookup2(pAd, &rx_blk->Addr2[0], NULL);
#endif

#ifdef WTBL_TDD_SUPPORT
	if (IS_WTBL_TDD_ENABLED(pAd)) {
		if (!WtblTdd_RxPacket_BlockList(pAd, rx_blk)) {
			MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG, ("%s(): rx_blk->wcid=%d ---> (%02x:%02x:%02x:%02x:%02x:%02x) \n",
									__func__, rx_blk->wcid, PRINT_MAC(rx_blk->Addr2)));
			if ((rx_blk->wcid == WCID_NO_MATCHED(pAd)) && (pEntry == NULL)) {
				pEntry = WtblTdd_InactiveList_Lookup(pAd, rx_blk->Addr2);
				if (pEntry) {
					WtblTdd_Entry_RxPacket(pAd, pEntry->wdev, pEntry);
					/* pEntry = NULL; */
				} else {
					pAd->wtblTddInfo.rxMissCounter++;
					MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"%s(): ---> Rx Miss ! (%02x:%02x:%02x:%02x:%02x:%02x) \n",
						__func__, PRINT_MAC(rx_blk->Addr2));
				}
			}
		}
	}
#endif /* WTBL_TDD_SUPPORT */

	if (pEntry)
		rx_blk->wcid = pEntry->wcid;

	return pEntry;
}

VOID dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
#ifdef CONFIG_AP_SUPPORT
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
#endif
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef RT_CFG80211_SUPPORT
	INT op_mode = pRxBlk->OpMode;
#endif
	MTWF_DBG(pAd, DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "-->%s()\n", __func__);

	if (pRxBlk->DataSize > MAX_MGMT_PKT_LEN) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DataSize=%d\n", pRxBlk->DataSize);
		hex_dump("MGMT ???", pRxBlk->FC, pRxBlk->DataSize);
		goto done;
	}

#ifdef SW_CONNECT_SUPPORT
	/* in Rx : pRxBlk->wcid : means the HW WCID , not the SW wcid
			 so need to check the HW boundary here.
	     TBD : maybe call HcGetSwWcid() instead of call research_entry() is fast, but need  to protect
	*/
	if (!VALID_UCAST_ENTRY_WCID_HW(pAd, pRxBlk->wcid)) {
		pEntry = research_entry(pAd, pRxBlk);
		if (pEntry)
			pRxBlk->wcid = pEntry->wcid;
	}
#endif /* !SW_CONNECT_SUPPORT */


	/* check if duplicate frame, ignore it and then drop */
	if (rx_chk_duplicate_mgmt_frame(pAd, pRxBlk) == NDIS_STATUS_FAILURE)
		goto done;

#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_HOTSPOT_R2
	if ((FC->SubType == SUBTYPE_ACTION)) {
		UINT8 *ptr = pRxBlk->FC;

		ptr += sizeof(HEADER_802_11);

		if (*ptr == CATEGORY_DLS) {
			hotspot_send_dls_resp(pAd, pRxBlk);
		}
	}
#endif
#endif
#ifdef RT_CFG80211_SUPPORT
#ifdef CFG_TDLS_SUPPORT

	if (CFG80211_HandleTdlsDiscoverRespFrame(pAd, pRxBlk, op_mode))
		goto done;

#endif /* CFG_TDLS_SUPPORT */

	if (CFG80211_HandleP2pMgmtFrame(pAd, pRxBlk, op_mode))
		goto done;

#endif /* RT_CFG80211_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
	if (PMF_PerformRxFrameAction(pAd, pRxBlk) == FALSE)
		goto done;
#endif /* DOT11W_PMF_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
/* pRxBlk->wcid already translate to sw wcid, so call VALID_UCAST_ENTRY_WCID() for sw boundary check */
#endif /* SW_CONNECT_SUPPORT */
	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	else
		pEntry = research_entry(pAd, pRxBlk);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (!ap_dev_rx_mgmt_frm(pAd, pRxBlk, pEntry))
			goto done;

		/* BssInfo not ready, drop frame */
		if ((pEntry) && (pEntry->wdev) && ((FC->SubType == SUBTYPE_AUTH) || (FC->SubType == SUBTYPE_ASSOC_REQ) ||
						 (FC->SubType == SUBTYPE_DEAUTH) || (FC->SubType == SUBTYPE_DISASSOC))) {
			if (WDEV_BSS_STATE(pEntry->wdev) != BSS_READY && pEntry->wdev->wdev_type != WDEV_TYPE_STA) {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR: BSS idx (%d) wdev %s state %d not ready. (subtype %d)\n",
						 pEntry->wdev->bss_info_argument.ucBssIndex,
						 pEntry->wdev->if_dev->name,
						 WDEV_BSS_STATE(pEntry->wdev),
						 FC->SubType);
				goto done;
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (!sta_dev_rx_mgmt_frm(pAd, pRxBlk, pEntry))
			goto done;
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef WIFI_DIAG
	diag_dev_rx_mgmt_frm(pAd, pRxBlk);
#endif

done:
	MTWF_DBG(pAd, DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "<--%s()\n", __func__);

	if (pRxPacket)
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}


VOID dev_rx_ctrl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s - subtype:%d\n", __func__, FC->SubType);

	switch (FC->SubType) {
#ifdef DOT11_N_SUPPORT

	case SUBTYPE_BLOCK_ACK_REQ: {
		FRAME_BA_REQ *bar = (FRAME_BA_REQ *)FC;
#ifdef MT_MAC

		if ((IS_HIF_TYPE(pAd, HIF_MT)) &&
			(pRxBlk->wcid == WCID_NO_MATCHED(pAd))) {
#ifdef MAC_REPEATER_SUPPORT
			REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

			MAC_TABLE_ENTRY *pEntry;

			pEntry = MacTableLookup(pAd, &pRxBlk->Addr2[0]);

			if (pEntry) {
				pRxBlk->wcid = pEntry->wcid;
#ifdef MAC_REPEATER_SUPPORT

				if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && (IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_PEER_AP(pEntry))) {
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &pRxBlk->Addr1[0], TRUE);

					if (pReptEntry && (pReptEntry->CliValid == TRUE)) {
						pEntry = pReptEntry->pMacEntry;
						pRxBlk->wcid = pEntry->wcid;
					} else if ((pReptEntry == NULL) && IS_ENTRY_PEER_AP(pEntry)) {/* this packet is for APCLI */
						INT apcli_idx = pEntry->func_tb_idx;

						pEntry = &pAd->MacTab.Content[pAd->StaCfg[apcli_idx].MacTabWCID];
						pRxBlk->wcid = pEntry->wcid;
					} else {
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                                                         "%s():Cannot found WCID of BAR packet!. A1:"MACSTR",A2:"MACSTR"\n\r",
                                                         __func__, MAC2STR(pRxBlk->Addr1), MAC2STR(pRxBlk->Addr2));
						break;
					}

					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ""MACSTR" recv BAR\n\r", MAC2STR(pRxBlk->Addr1));
				}

#endif
			} else {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                                         "%s():pEntry(NULL),Cannot found WCID of BAR packet!A1:"MACSTR",A2:"MACSTR"\n",
					 __func__, MAC2STR(pRxBlk->Addr1), MAC2STR(pRxBlk->Addr2));
			}
		}

#endif /* MT_MAC */
		bar_process(pAd, pRxBlk->wcid, (pRxBlk->MPDUtotalByteCnt),
					(PFRAME_BA_REQ)FC);

		if (bar->BARControl.Compressed == 0) {
			UCHAR tid = bar->BARControl.TID;

			ba_rec_session_tear_down(pAd, pRxBlk->wcid, tid, FALSE);
		}
	}
	break;
#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	case SUBTYPE_PS_POLL:
		/*
			    This marco is not suitable for P2P GO.
			    It is OK to remove this marco here. @20140728
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		*/
	{
		USHORT Aid = pRxBlk->Duration & 0x3fff;
		PUCHAR pAddr = pRxBlk->Addr2;
		MAC_TABLE_ENTRY *pEntry;
#ifdef MT_MAC

		if ((IS_HIF_TYPE(pAd, HIF_MT)) &&
			(pRxBlk->wcid == WCID_NO_MATCHED(pAd))) {

			UCHAR wdev_idx;
			struct wifi_dev *wdev;
			struct wifi_dev_ops *ops;

			wdev_idx = RTMP_GET_PACKET_WDEV(pRxPacket);
			wdev = pAd->wdev_list[wdev_idx];
			ops = wdev->wdev_ops;
			ops->mac_entry_lookup(pAd, &pRxBlk->Addr2[0], wdev, &pEntry);

			if (pEntry)
				pRxBlk->wcid = pEntry->wcid;
			else {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                                         "%s(): Cannot found WCID of PS-Poll packet!\n", __func__);
			}
		}

#endif /* MT_MAC */

		/* printk("dev_rx_ctrl_frm0 SUBTYPE_PS_POLL pRxBlk->wcid: %x pEntry->wcid:%x\n",pRxBlk->wcid,pEntry->wcid); */
		if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
			/* printk("dev_rx_ctrl_frm1 SUBTYPE_PS_POLL\n"); */
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if (pEntry->Aid == Aid)
				RtmpHandleRxPsPoll(pAd, pAddr, pRxBlk->wcid, FALSE);
			else {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s(): Aid mismatch(pkt:%d, Entry:%d)!\n",
						 __func__, Aid, pEntry->Aid);
			}
		}
	}
	break;
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT

	case SUBTYPE_BLOCK_ACK:
		/* +++Add by shiang for debug */
		/* TODO: shiang-MT7603, remove this! */
	{
		UCHAR *ptr, *ra, *ta;
		BA_CONTROL *ba_ctrl;

#ifdef SNIFFER_SUPPORT
		if (!MONITOR_ON(pAd))
#endif /* SNIFFER_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s():BlockAck From WCID:%d\n", __func__, pRxBlk->wcid);

		ptr = (UCHAR *)pRxBlk->FC;
		ptr += 4;
		ra = ptr;
		ptr += 6;
		ta = ptr;
		ptr += 6;
		ba_ctrl = (BA_CONTROL *)ptr;
		ptr += sizeof(BA_CONTROL);

#ifdef SNIFFER_SUPPORT
		if (!MONITOR_ON(pAd))
#endif /* SNIFFER_SUPPORT */
		{
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\tRA="MACSTR", TA="MACSTR"\n",
					MAC2STR(ra), MAC2STR(ta));
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\tBA Control: AckPolicy=%d, MTID=%d, Compressed=%d, TID_INFO=0x%x\n",
					ba_ctrl->ACKPolicy, ba_ctrl->MTID, ba_ctrl->Compressed, ba_ctrl->TID);
		}

		if (ba_ctrl->ACKPolicy == 0 && ba_ctrl->Compressed == 1) {
			BASEQ_CONTROL *ba_seq;

			ba_seq = (BASEQ_CONTROL *)ptr;

#ifdef SNIFFER_SUPPORT
			if (!MONITOR_ON(pAd))
#endif /* SNIFFER_SUPPORT */
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"\tBA StartingSeqCtrl:StartSeq=%d, FragNum=%d\n",
						ba_seq->field.StartSeq, ba_seq->field.FragNum);

			ptr += sizeof(BASEQ_CONTROL);

#ifdef SNIFFER_SUPPORT
			if (!MONITOR_ON(pAd))
#endif /* SNIFFER_SUPPORT */
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"\tBA Bitmap:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
						*ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3),
						*(ptr + 4), *(ptr + 5), *(ptr + 6), *(ptr + 7));

		}
	}
	break;
		/* ---Add by shiang for debug */
#endif /* DOT11_N_SUPPORT */
	case SUBTYPE_TRIGGER:
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
                         "%s():Recive TRIG frame . A1:"MACSTR",A2:"MACSTR"\n\r", __func__,
		         MAC2STR(pRxBlk->Addr1), MAC2STR(pRxBlk->Addr2));
#ifdef FW_LOG_DUMP
		dbg_log_wrapper(pAd, DBG_LOG_PKT_TYPE_TRIG_FRAME, (UCHAR *)FC, pRxBlk->MPDUtotalByteCnt);
#endif
		break;


	case SUBTYPE_ACK:
	default:
		break;
	}

#ifdef WIFI_DIAG
	diag_dev_rx_cntl_frm(pAd, pRxBlk);
#endif
	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}

VOID rtmp_handle_mic_error_event(
	RTMP_ADAPTER *ad,
	RX_BLK *rx_blk)
{
	MAC_TABLE_ENTRY *entry;
	RXINFO_STRUC *rx_info = rx_blk->pRxInfo;

	entry = MacTableLookup(ad, rx_blk->Addr2);

	if (!entry || RX_BLK_TEST_FLAG(rx_blk, fRX_WCID_MISMATCH))
		return;

	if ((rx_info->U2M) && VALID_UCAST_ENTRY_WCID(ad, rx_blk->wcid) && (IS_CIPHER_TKIP_Entry(entry))) {
		/* unicast case */
		if (IS_ENTRY_PEER_AP(entry) || IS_ENTRY_REPEATER(entry)) {
			/* sta rx mic error */
#ifdef CONFIG_STA_SUPPORT
			sta_handle_mic_error_event(ad, entry, rx_blk);
#endif /* CONFIG_STA_SUPPORT */
		} else {
			/* ap rx mic error */
#ifdef CONFIG_AP_SUPPORT
			ap_handle_mic_error_event(ad, entry, rx_blk);
#endif /* CONFIG_AP_SUPPORT */
		}
	} else if (rx_info->Mcast || rx_info->Bcast) {
		/* bmc case */
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Rx bc/mc Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\n",
				 rx_blk->MPDUtotalByteCnt, rx_blk->wcid, rx_info->CipherErr);

		if (IS_ENTRY_PEER_AP(entry) || IS_ENTRY_REPEATER(entry)) {
#ifdef CONFIG_STA_SUPPORT
			sta_handle_mic_error_event(ad, entry, rx_blk);

#endif /* CONFIG_STA_SUPPORT */
		}
	}
}

/*
	========================================================================
	Routine Description:
		Check Rx descriptor, return NDIS_STATUS_FAILURE if any error found
	========================================================================
*/
static INT rtmp_chk_rx_err(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo;
	FRAME_CONTROL *FC;
#if defined(OUI_CHECK_SUPPORT) || defined(HTC_DECRYPT_IOT) || defined(MWDS)
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif /* defined (OUI_CHECK_SUPPORT) || defined (HTC_DECRYPT_IOT)  || defined (MWDS) */
	int LogDbgLvl = DBG_LVL_INFO;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;

	if ((pRxBlk == NULL) || (pRxBlk->pRxInfo == NULL)) {
		/* +++Add by shiang for debug */
		if (pRxBlk == NULL)
			printk("%s(): pRxBlk is NULL\n", __func__);
		else
			printk("%s(): pRxBlk->pRxInfo is NULL\n", __func__);

		/* ---Add by shiang for debug */
		return NDIS_STATUS_FAILURE;
	}

	FC = (FRAME_CONTROL *)pRxBlk->FC;
	pRxInfo = pRxBlk->pRxInfo;


#ifdef MT_MAC

	/* TODO: shiang-MT7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		/* +++Add by shiang for work-around, should remove it once we correctly configure the BSSID! */
		/* TODO: shiang-MT7603 work around!! */
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_ICV_ERR)) {
#ifdef WIFI_DIAG
			/* WEP + open, wrong passowrd can association success, but rx data error */
			if (IS_WCID_VALID(pAd, pRxBlk->wcid) && (pRxBlk->pRxInfo->U2M)) {
				MAC_TABLE_ENTRY *pEntry = NULL;

				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
				if (pEntry && IS_ENTRY_CLIENT(pEntry)
					&& (IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher)))
					diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
						DIAG_CONN_AUTH_FAIL, REASON_DECRYPTION_FAIL);
			}
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_WCID_VALID(pAd, pRxBlk->wcid) && (pRxBlk->pRxInfo->U2M)) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if (pEntry && IS_ENTRY_CLIENT(pEntry)
					&& (IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher)))
					ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
						pEntry->Addr,
						REASON_MIC_FAILURE);
			}
#endif

#ifdef OUI_CHECK_SUPPORT
			UCHAR band_idx = (pRxBlk->channel_freq > 14) ? 1 : 0;

			if (band_idx >= DBDC_BAND_NUM)
				return NDIS_STATUS_FAILURE;

			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
			if (pEntry->wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s():WDEV is NULL!\n", __func__);
				return NDIS_STATUS_FAILURE;
			}

			if ((pAd->MacTab.oui_mgroup_cnt > 0)
#ifdef MAC_REPEATER_SUPPORT
				|| (pAd->ApCfg.RepeaterCliSize[HcGetBandByWdev(pEntry->wdev)] != 0)
#endif
			   ) {
				if (NdisCmpMemory(pEntry->Addr, pRxBlk->Addr2, MAC_ADDR_LEN)) {
					INC_COUNTER64(pAd->WlanCounters[band_idx].RxHWLookupWcidErrCount);

					if (DebugLevel < DBG_LVL_INFO)
						return NDIS_STATUS_FAILURE;
				}

#ifdef MAC_REPEATER_SUPPORT

				if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
					BOOLEAN bwcid_error = FALSE;

					if (IS_ENTRY_REPEATER(pEntry)) {
						UINT16	orig_wcid = pRxBlk->wcid;
						REPEATER_CLIENT_ENTRY	*pReptEntry = NULL;

						pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &pRxBlk->Addr1[0], TRUE);

						if (pReptEntry && (pReptEntry->CliValid == TRUE)) {
							if (orig_wcid != pReptEntry->pMacEntry->wcid)
								bwcid_error = TRUE;
						} else
							bwcid_error = TRUE;
					} else if (IS_ENTRY_PEER_AP(pEntry)) {
						INT apcli_idx = pEntry->func_tb_idx;

						if (NdisCmpMemory(pAd->StaCfg[apcli_idx].wdev.if_addr, pRxBlk->Addr1, MAC_ADDR_LEN))
							bwcid_error = TRUE;
					}

					if (bwcid_error) {
							INC_COUNTER64(pAd->WlanCounters[band_idx].RxHWLookupWcidErrCount);

						if (DebugLevel < DBG_LVL_INFO)
							return NDIS_STATUS_FAILURE;
					}
				}

#endif /* MAC_REPEATER_SUPPORT */
			}

#endif

#ifdef A4_CONN
			if (pRxBlk->Addr1 != NULL) {
				if (IS_BM_MAC_ADDR(pRxBlk->Addr1)) {
					pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

					if (pEntry && IS_ENTRY_PEER_AP(pEntry))
						return NDIS_STATUS_FAILURE;
				}
			}
#endif /* A4_CONN */


#ifdef HTC_DECRYPT_IOT
			if (pRxInfo->HTC == 1) {
				if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
					/* Rx HTC and FAIL decryp case! */
					if ((pEntry->HTC_AAD_OM_CountDown == 0) &&
						(pEntry->HTC_AAD_OM_Freeze == 0)) {
						if (pEntry->HTC_ICVErrCnt++ > pAd->HTC_ICV_Err_TH
							&& pEntry->HTC_AAD_OM_Force == 0) {
							pEntry->HTC_ICVErrCnt = 0; /* reset the history */
							pEntry->HTC_AAD_OM_CountDown = 3;

							pEntry->HTC_AAD_OM_Force = 1;
							MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, "@AAD_OM Trigger ! wcid=%u\n", pRxBlk->wcid);
							HW_SET_ASIC_WCID_AAD_OM(pAd, pRxBlk->wcid, 1);
						}
					}
				}

				if (pEntry && pEntry->HTC_AAD_OM_Freeze) {
					/*
						Wokradound if the ICV Error happened again!
					*/
					if (pEntry->HTC_AAD_OM_Force) {
						HW_SET_ASIC_WCID_AAD_OM(pAd, pRxBlk->wcid, 1);
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, "@ICV Error, HTC_AAD_OM_Force already 1, wcid=%u", pRxBlk->wcid);
					} else
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, "ICV Error");

					if (pRxBlk->Addr1 != NULL)
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
								 ", Addr1 = "MACSTR"", MAC2STR(pRxBlk->Addr1));

					if (pRxBlk->Addr2 != NULL)
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
								 ", Addr2 = "MACSTR"", MAC2STR(pRxBlk->Addr2));

					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, "\n");
					asic_dump_rmac_info_for_ICVERR(pAd, pRxBlk->rmac_info);
				} else if (pEntry && pEntry->HTC_AAD_OM_Force) {
					if (pEntry->HTC_AAD_OM_CountDown == 0) {
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
							"@ICV Error, AAD_OM has been triggered, wcid=%u", pRxBlk->wcid);
						if (pRxBlk->Addr1 != NULL)
							MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
									 ", Addr1 = "MACSTR"", MAC2STR(pRxBlk->Addr1));

						if (pRxBlk->Addr2 != NULL)
							MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
									 ", Addr2 = "MACSTR"", MAC2STR(pRxBlk->Addr2));
						if (pEntry->HTC_ICVErrCnt == pAd->HTC_ICV_Err_TH)
							asic_dump_wtbl_info(pAd, pRxBlk->wcid);
					}
				} else
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, "AAD_OM detection in progress!\n");
			} else
#endif /* HTC_DECRYPT_IOT */
			{
#ifdef TXRX_STAT_SUPPORT
				{
					if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
						struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
						MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
						UCHAR bandidx = 0;

						if (pEntry->wdev == NULL) {
							MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s():WDEV is NULL!\n", __func__);
							return NDIS_STATUS_FAILURE;
						}
						bandidx = HcGetBandByWdev(pEntry->wdev);
						if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry)) {
							INC_COUNTER64(pEntry->RxDecryptionErrorCount);
							INC_COUNTER64(pEntry->pMbss->stat_bss.RxDecryptionErrorCount);
							INC_COUNTER64(ctrl->rdev[bandidx].pRadioCtrl->RxDecryptionErrorCount);
						}
					}
				}
#endif

				/* If receive ICV error packet, add counter and show in tpinfo */
				tr_cnt->rx_icv_err_cnt++;
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, "ICV Error");

				if (pRxBlk->Addr1 != NULL)
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
							 ", Addr1 = "MACSTR"", MAC2STR(pRxBlk->Addr1));

				if (pRxBlk->Addr2 != NULL)
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
							 ", Addr2 = "MACSTR"", MAC2STR(pRxBlk->Addr2));

				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, "\n");
				asic_dump_rmac_info_for_ICVERR(pAd, pRxBlk->rmac_info);
			}

			if (DebugLevel >= DBG_LVL_INFO)
				dump_rxblk(pAd, pRxBlk);

			return NDIS_STATUS_FAILURE;
		}

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_CLM)) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "CM Length Error\n, WlanIndex = %d\n", pRxBlk->wcid);
			return NDIS_STATUS_FAILURE;
		}

		if (pRxBlk->DeAmsduFail && pRxBlk->wcid != WLAN_IDX_MISMATCH) {
			/* wlan_idx mismatch will release pkt and send deauth in ap_chk_cl2_cl3_err */
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Deamsdu Fail\n, WlanIndex = %d\n", pRxBlk->wcid);
			if (DebugLevel >= DBG_LVL_INFO)
				dump_rxblk(pAd, pRxBlk);
			return NDIS_STATUS_FAILURE;
		}

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_TKIP_MIC_ERR)) {
#ifdef OUI_CHECK_SUPPORT
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if (pAd->MacTab.oui_mgroup_cnt > 0) {
				if (NdisCmpMemory(pEntry->Addr, pRxBlk->Addr2, MAC_ADDR_LEN))
					RX_BLK_SET_FLAG(pRxBlk, fRX_WCID_MISMATCH);
			}

#endif
#ifdef TXRX_STAT_SUPPORT
		{
			if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
				struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
				UCHAR bandidx = 0;

				if (pEntry->wdev == NULL) {
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s():WDEV is NULL!\n", __func__);
					return NDIS_STATUS_FAILURE;
				}
				bandidx = HcGetBandByWdev(pEntry->wdev);
				if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry)) {
					INC_COUNTER64(pEntry->RxMICErrorCount);
					INC_COUNTER64(pEntry->pMbss->stat_bss.RxMICErrorCount);
					INC_COUNTER64(ctrl->rdev[bandidx].pRadioCtrl->RxMICErrorCount);
				 }
			 }
		}
#endif

			if (!(RX_BLK_TEST_FLAG(pRxBlk, fRX_WCID_MISMATCH)))
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "TKIP MIC Error\n, WlanIndex = %d\n", pRxBlk->wcid);

#ifdef CONFIG_STA_SUPPORT
			rtmp_handle_mic_error_event(pAd, pRxBlk);
			return NDIS_STATUS_FAILURE;
#endif /* CONFIG_STA_SUPPORT */
		}

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pRxBlk->pRxInfo->MyBss = 1;
#endif /* CONFIG_STA_SUPPORT */
#ifdef HTC_DECRYPT_IOT
		if (pRxInfo->HTC == 1) { /* focus HTC pkt only! */
			if (pRxBlk->sec_mode != 0) {
				if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid) && (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE))
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
					if ((pEntry->HTC_AAD_OM_CountDown == 0) &&
						(pEntry->HTC_AAD_OM_Freeze == 0)) {
						/* Rx decrypt OK  of HTC */
						if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_ICV_ERR)) {
							if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_CM) &&
								!RX_BLK_TEST_FLAG(pRxBlk, fRX_CLM) &&
								!RX_BLK_TEST_FLAG(pRxBlk, fRX_TKIP_MIC_ERR)) {
								pEntry->HTC_ICVErrCnt = 0; /* reset counter! */
								pEntry->HTC_AAD_OM_Freeze = 1; /* decode ok, we treat the entry don't need to count pEntry->HTC_ICVErrCnt any more! */
							}
						}
					}
				}
			}
		}

#endif /* HTC_DECRYPT_IOT */
	}

#endif /* MT_MAC */
#ifdef CONFIG_CSO_SUPPORT
#ifdef CSO_TEST_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_CSO)
		&& CsCtrl & BIT4) {
		if (pRxBlk->MPDUtotalByteCnt > 500) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "cs_status: %x cs_type: %x,",
						  pRxBlk->rCso.ChksumStatus, pRxBlk->rCso.ChksumType);
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "ip_len_mismatch: %x,",
						  pRxBlk->rCso.IpLenMismatch);
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "un_next_hdr: %x, IpFrag = %x\n",
						  pRxBlk->rCso.UnknownNextHdr, pRxBlk->rCso.IpFrag);
		}
	} else
#endif
	if (IS_ASIC_CAP(pAd, fASIC_CAP_CSO)) {
		/*CSO error*/
		if (pRxBlk->rCso.ChksumStatus & 0xd) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "rtmp_chk_rx_err packet error cs_status: %x cs_type: %x\n",
					  pRxBlk->rCso.ChksumStatus, pRxBlk->rCso.ChksumType);
			return NDIS_STATUS_FAILURE;
		}

		if (pRxBlk->rCso.IpLenMismatch) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "rtmp_chk_rx_err ip_len_mismatch: %x\n",
					  pRxBlk->rCso.IpLenMismatch);
			return NDIS_STATUS_FAILURE;
		}

		if (pRxBlk->rCso.UnknownNextHdr) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "rtmp_chk_rx_err un_next_hdr: %x\n",
					  pRxBlk->rCso.UnknownNextHdr);
			return NDIS_STATUS_FAILURE;
		}
	}

#endif /* CONFIG_CSO_SUPPORT */

	/* Phy errors & CRC errors*/
	if (pRxInfo->Crc) {
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			INT dBm = (pRxBlk->rx_signal.raw_rssi[0]) - pAd->BbpRssiToDbmDelta;

			/* Check RSSI for Noise Hist statistic collection.*/
			if (dBm <= -87)
				pAd->StaCfg[0].RPIDensity[0] += 1;
			else if (dBm <= -82)
				pAd->StaCfg[0].RPIDensity[1] += 1;
			else if (dBm <= -77)
				pAd->StaCfg[0].RPIDensity[2] += 1;
			else if (dBm <= -72)
				pAd->StaCfg[0].RPIDensity[3] += 1;
			else if (dBm <= -67)
				pAd->StaCfg[0].RPIDensity[4] += 1;
			else if (dBm <= -62)
				pAd->StaCfg[0].RPIDensity[5] += 1;
			else if (dBm <= -57)
				pAd->StaCfg[0].RPIDensity[6] += 1;
			else if (dBm > -57)
				pAd->StaCfg[0].RPIDensity[7] += 1;
		}
#endif /* CONFIG_STA_SUPPORT */
		return NDIS_STATUS_FAILURE;
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* Drop ToDs promiscous frame, it is opened due to CCX 2 channel load statistics*/
		if ((pRxBlk->DataSize < 14) || (pRxBlk->MPDUtotalByteCnt > MAX_AGGREGATION_SIZE)) {
			/*
				min_len: CTS/ACK frame len = 10, but usually we filter it in HW,
						so here we drop packet with length < 14 Bytes.

				max_len:  Paul 04-03 for OFDM Rx length issue
			*/
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "rx pkt len err(%d, %d)\n",
					 pRxBlk->DataSize, pRxBlk->MPDUtotalByteCnt);
			return NDIS_STATUS_FAILURE;
		}

		if (pRxBlk->FC) {
#ifndef CLIENT_WDS

			if (FC->ToDs
#ifdef RT_CFG80211_P2P_SUPPORT
				/* CFG TODO */
				/*&& !IS_ENTRY_CLIENT(pEntry)*/ && 0
#endif /* RT_CFG80211_P2P_SUPPORT */
			   ) {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: Line(%d) (ToDs Packet not allow in STA Mode)\n", __func__, __LINE__);
				return NDIS_STATUS_FAILURE;
			}

#endif /* !CLIENT_WDS */
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	/* drop decyption fail frame*/
	if (pRxInfo->Decrypted && pRxInfo->CipherErr) {
		if (pRxInfo->CipherErr == 2)
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RxErr: ICV ok but MICErr");
		else if (pRxInfo->CipherErr == 1)
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RxErr: ICV Err");
		else if (pRxInfo->CipherErr == 3)
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RxErr: Key not valid");
		else
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RxErr: CipherErr 0x%x", pRxInfo->CipherErr);

		INC_COUNTER64(pAd->WlanCounters[0].WEPUndecryptableCount);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*
				WCID equal to 255 mean MAC couldn't find any matched entry in Asic-MAC table.
				The incoming packet mays come from WDS or AP-Client link.
				We need them for further process. Can't drop the packet here.
			*/
			if ((pRxInfo->U2M) && (pRxBlk->wcid == 255)
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode[pRxBlk->band] == WDS_LAZY_MODE)
#endif /* WDS_SUPPORT */
			   )
				return NDIS_STATUS_SUCCESS;

			/* Increase received error packet counter per BSS */
			if (FC->FrDs == 0 &&
				pRxInfo->U2M &&
				pRxBlk->bss_idx < pAd->ApCfg.BssidNum) {
				BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pRxBlk->bss_idx];

				pMbss->RxDropCount++;
				pMbss->RxErrorCount++;
			}
#ifdef TXRX_STAT_SUPPORT
			if (FC->FrDs == 0 && pRxInfo->U2M && (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
				if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry))
				INC_COUNTER64(pEntry->pMbss->stat_bss.RxPacketDroppedCount);
			}
#endif

#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT

			if ((FC->FrDs == 1) && (FC->ToDs == 1) &&
				(VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid) && (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE))) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if (IS_ENTRY_WDS(pEntry) && (pEntry->func_tb_idx < MAX_WDS_ENTRY))
					pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].WdsCounter.RxErrorCount++;
			}

#endif /* STATS_COUNT_SUPPORT */
#endif /* WDS_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (INFRA_ON(&pAd->StaCfg[0]) && pRxInfo->MyBss) {
				if ((pRxInfo->CipherErr & 1) == 1) {
					RTMPSendWirelessEvent(pAd, IW_ICV_ERROR_EVENT_FLAG,
										  pRxBlk->Addr2,
										  BSS0, 0);
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */
		rtmp_handle_mic_error_event(pAd, pRxBlk);
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): %d (len=%d, Mcast=%d, MyBss=%d, Wcid=%d, KeyId=%d)\n",
				 __func__, pRxInfo->CipherErr, pRxBlk->MPDUtotalByteCnt,
				 pRxInfo->Mcast | pRxInfo->Bcast, pRxInfo->MyBss, pRxBlk->wcid,
				 pRxBlk->key_idx);
#ifdef DBG
		asic_dump_rxinfo(pAd, (UCHAR *) pRxInfo);
		asic_dump_rmac_info(pAd, (UCHAR *)pRxBlk->pRxWI);
		if (pRxBlk->FC != NULL) {
			hex_dump("ErrorPkt",  (UCHAR *)pRxBlk->FC, pRxBlk->MPDUtotalByteCnt);
		}
#endif /* DBG */

		if (pRxBlk->FC == NULL)
			return NDIS_STATUS_SUCCESS;

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


BOOLEAN dev_rx_no_foward(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	return TRUE;
}


#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT hdr_len = LENGTH_802_11;
#ifdef CONFIG_MAP_3ADDR_SUPPORT
	MAC_TABLE_ENTRY *ptEntry = NULL;
	struct wifi_dev *twdev = NULL;
	UCHAR idx;
	UINT16 wcid;
#endif
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "-->%s():pRxBlk->wcid=%d\n", __func__, pRxBlk->wcid);

#ifdef AUTOMATION
	sta_rx_packet_check(pAd, pRxBlk);
#endif /* AUTOMATION */

	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s(): pAd is null", __func__);
		return FALSE;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                         "%s(): pEntry is null", __func__);
		return FALSE;
	}

#ifdef CONFIG_STA_SUPPORT
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	if (pStaCfg == NULL)
		return FALSE;
#endif /* CONFIG_STA_SUPPORT */


	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)) {
#ifdef APCLI_AS_WDS_STA_SUPPORT
		if (IS_ENTRY_PEER_AP(pEntry)) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			hdr_len = LENGTH_802_11_WITH_ADDR4;
		}
#endif /* APCLI_AS_WDS_STA_SUPPORT */
#ifdef CLIENT_WDS
		if ((VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
			&& IS_ENTRY_CLIENT(pEntry)) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			hdr_len = LENGTH_802_11_WITH_ADDR4;
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
		}
#endif /* CLIENT_WDS */

#ifdef APCLI_SUPPORT
#ifdef A4_CONN
		if (IS_ENTRY_A4(pEntry)) {

			/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("sta_rx_pkt_allow: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x\n ApCli recvd a MWDS pkt\n", */
			/* wdev->wdev_idx,wdev->wdev_type,wdev->func_idx)); */
#ifdef MWDS
			pEntry->MWDSInfo.Addr4PktNum++;
#endif
			NdisGetSystemUpTime(&pStaCfg->ApcliInfStat.ApCliRcvBeaconTime);
			if (MAC_ADDR_EQUAL(pRxBlk->Addr4, pStaCfg->wdev.if_addr)) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					"ApCli receive a looping packet!\n");
					return FALSE;
			}

			if ((pAd->ApCfg.ApCliInfRunned == 1)) {
				BOOLEAN bTAMatchSA = MAC_ADDR_EQUAL(pEntry->Addr, pRxBlk->Addr4);

				/*
				For ApCli, we have to avoid to delete the bridge MAC(SA) and AP MAC(TA) is the same case.
				*/

				if (!bTAMatchSA) {
						MAC_TABLE_ENTRY *pMovedEntry = MacTableLookup(pAd, pRxBlk->Addr4);

						if (pMovedEntry
#ifdef AIR_MONITOR
							&& !IS_ENTRY_MONITOR(pMovedEntry)
#endif /* AIR_MONITOR */
							&& IS_ENTRY_CLIENT(pMovedEntry)
						) {

						/*
						It means this source entry has moved to another one and hidden behind it.
						So delete this source entry!
						*/
						/*
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("ApCli found a entry("MACSTR") who has moved to another side! Delete it from MAC table.\n",
						MAC2STR(pMovedEntry->Addr)));
						*/
#ifdef WH_EVENT_NOTIFIER
						{
							EventHdlr pEventHdlrHook = NULL;

							pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
							if (pEventHdlrHook && pMovedEntry->wdev)
								pEventHdlrHook(pAd, pMovedEntry->wdev,
									pMovedEntry->Addr, pMovedEntry->wdev->channel);
						}
#endif /* WH_EVENT_NOTIFIER */
						if (pMovedEntry->NoDataIdleCount > 1) {
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"ApCli found a entry(%02X:%02X:%02X:%02X:%02X:%02X) moved to another dev! Delete it from MAC table.\n",
								 PRINT_MAC(pMovedEntry->Addr));
							mac_entry_delete(pAd, pMovedEntry);
						}
					}
				}
			}
		}
#endif /* A4_CONN */
#endif /* APCLI_SUPPORT */

		RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	}


	/* Drop not my BSS frames */
	if (pRxInfo->MyBss == 0) {
		/* CFG_TODO: NEED CHECK for MT_MAC */
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
		/* When the p2p-IF up, the STA own address would be set as my_bssid address.
		   If receiving an "encrypted" broadcast packet(its WEP bit as 1) and doesn't match my BSSID,
		   Asic pass to driver with "Decrypted" marked as 0 in pRxInfo.
		   The condition is below,
		   1. p2p IF is ON,
		   2. the addr2 of the received packet is STA's BSSID,
		   3. broadcast packet,
		   4. from DS packet,
		   5. Asic pass this packet to driver with "pRxInfo->Decrypted=0"
		 */
		if (
#ifdef RT_CFG80211_P2P_SUPPORT
			TRUE /* The dummy device always present for CFG80211 application*/
#else
			(P2P_INF_ON(pAd))
#endif /* RT_CFG80211_P2P_SUPPORT */
			&& (MAC_ADDR_EQUAL(pAd->CommonCfg.Bssid, pHeader->Addr2)) &&
			(pRxInfo->Bcast || pRxInfo->Mcast) &&
			(pFmeCtrl->FrDs == 1) &&
			(pFmeCtrl->ToDs == 0) &&
			(pRxInfo->Decrypted == 0)) {
			/* set this m-cast frame is my-bss. */
			pRxInfo->MyBss = 1;
		} else
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
		{
#ifdef A4_CONN
			if (IS_ENTRY_A4(pEntry))
		pRxInfo->MyBss = 1;
	    else
#endif /* A4_CONN */

#ifdef APCLI_AS_WDS_STA_SUPPORT
		if (wdev->wds_enable == 1)
			pRxInfo->MyBss = 1;
		else
#endif

#ifdef APCLI_SUPPORT

			if (pEntry &&
				(IS_ENTRY_PEER_AP(pEntry) || (IS_ENTRY_REPEATER(pEntry)))
			   ) {
				/* Focus the EAP PKT only! */
				if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0) &&
					(pFmeCtrl->Type == FC_TYPE_DATA) && (MAC_ADDR_EQUAL(pEntry->wdev->bssid, pRxBlk->Addr3))) {
					/*
						update RxBlk->pData, DataSize, 802.11 Header, QOS, HTC, Hw Padding
					*/
					UCHAR *pData = (UCHAR *)pFmeCtrl;
					/* 1. skip 802.11 HEADER */
					pData += LENGTH_802_11;
					/* pRxBlk->DataSize -= hdr_len; */

					/* 2. QOS */
					if (pFmeCtrl->SubType & 0x08) {
						/* skip QOS contorl field */
						pData += 2;
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
#ifdef DOT11_N_SUPPORT
							/* skip HTC control field */
							pData += 4;
#endif /* DOT11_N_SUPPORT */
						}
					}

					/* 4. skip HW padding */
					if (pRxInfo->L2PAD)
						pData += 2;

					if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
						/* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
						NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
						pData += 6;

					if (NdisEqualMemory(EAPOL, pData, 2)) {
						pRxInfo->MyBss = 1; /* correct this */
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): Hit EAP!\n", __func__);
					} else {
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s():  Not my bss! pRxInfo->MyBss=%d\n", __func__, pRxInfo->MyBss);
						return FALSE;
					}
				}
			} else
#endif /* APCLI_SUPPORT */
			{
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s():  Not my bss! pRxInfo->MyBss=%d\n", __func__, pRxInfo->MyBss);
				return FALSE;
			}
		}
	}

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
	if (pEntry &&
		IS_ENTRY_PEER_AP(pEntry) &&
		wdev->wdev_type == WDEV_TYPE_STA &&
		pRxInfo->MyBss &&
		pRxInfo->U2M) {
		RTMPWakeUpWdev(pAd, wdev);
	}
#endif /* CONFIG_STA_SUPPORT || APCLI_SUPPORT */

	pAd->RalinkCounters.RxCountSinceLastNULL++;
#ifdef UAPSD_SUPPORT

	if (wdev->UapsdInfo.bAPSDCapable
		&& pAd->CommonCfg.APEdcaParm[0].bAPSDCapable
		&& (pFmeCtrl->SubType & 0x08)) {
		UCHAR *pData;
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "bAPSDCapable\n");
		/* Qos bit 4 */
		pData = pRxBlk->FC + LENGTH_802_11;

		if ((*pData >> 4) & 0x01) {
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

			/* ccv EOSP frame so the peer can sleep */
			if (pEntry != NULL)
				RTMP_PS_VIRTUAL_SLEEP(pEntry);

#ifdef CONFIG_STA_SUPPORT

			if (pStaCfg->FlgPsmCanNotSleep == TRUE)
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "tdls uapsd> Rcv EOSP frame but we can not sleep!\n");
			else
#endif /* CONFIG_STA_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			{
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "RxDone- Rcv EOSP frame, driver may fall into sleep\n");
				pAd->CommonCfg.bInServicePeriod = FALSE;
#ifdef CONFIG_STA_SUPPORT

				/* Force driver to fall into sleep mode when rcv EOSP frame */
				if (!pStaCfg->PwrMgmt.bDoze)
					RTMP_SLEEP_FORCE_AUTO_WAKEUP(pAd, pStaCfg);

#endif /* CONFIG_STA_SUPPORT */
			}
		}

		if ((pFmeCtrl->MoreData) && (pAd->CommonCfg.bInServicePeriod))
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MoreData bit=1, Sending trigger frm again\n");
	}

#endif /* UAPSD_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

	/* 1: PWR_SAVE, 0: PWR_ACTIVE */
	if (pEntry != NULL) {
		UCHAR OldPwrMgmt;

		OldPwrMgmt = RtmpPsIndicate(pAd, pRxBlk->Addr2, pEntry->wcid, pFmeCtrl->PwrMgmt);
#ifdef UAPSD_SUPPORT
		RTMP_PS_VIRTUAL_TIMEOUT_RESET(pEntry);

		if (pFmeCtrl->PwrMgmt) {
			if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
				(pFmeCtrl->SubType & 0x08)) {
				/*
					In IEEE802.11e, 11.2.1.4 Power management with APSD,
					If there is no unscheduled SP in progress, the unscheduled SP begins
					when the QAP receives a trigger frame from a non-AP QSTA, which is a
					QoS data or QoS Null frame associated with an AC the STA has
					configured to be trigger-enabled.

					In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
					to/from Power Save Mode is not considered to be a Trigger Frame and
					the AP shall not respond with a QoS Null frame.
				*/
				/* Trigger frame must be QoS data or QoS Null frame */
				UCHAR  OldUP;

				if ((*(pRxBlk->pData + LENGTH_802_11) & 0x10) == 0) {
					/* this is not a EOSP frame */
					OldUP = (*(pRxBlk->pData + LENGTH_802_11) & 0x07);

					if (OldPwrMgmt == PWR_SAVE) {
						/* hex_dump("trigger frame", pRxBlk->pData, 26); */
						UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
					}
				} else
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "This is a EOSP frame, not a trigger frame!\n");
			}
		}

#endif /* UAPSD_SUPPORT */
	}

#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if ((pFmeCtrl->SubType & 0x04)) { /* bit 2 : no DATA */
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s():  No DATA!\n", __func__);
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

	if (pEntry &&
		(IS_ENTRY_PEER_AP(pEntry) || (IS_ENTRY_REPEATER(pEntry)))
	   ) {


		if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0)) {
			ULONG Now32;
			UCHAR ifidx = pStaCfg->wdev.func_idx;

			if (ifidx < pAd->MSTANum) {
				if (!(pEntry && APCLI_IF_UP_CHECK(pAd, ifidx)))
					return FALSE;
			}
			else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s() invalid sta entry id(%d)\n", __func__, ifidx);
				return FALSE;
			}

			NdisGetSystemUpTime(&Now32);
			pStaCfg->ApcliInfStat.ApCliRcvBeaconTime = Now32;

#ifdef MWDS
		if (GET_ENTRY_A4(pEntry) == A4_TYPE_MWDS) {
			/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, */
			/* ("wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x : Non MWDS Pkt not allowed\n", */
				/* wdev->wdev_idx,wdev->wdev_type,wdev->func_idx)); */
				pEntry->MWDSInfo.Addr3PktNum++;
				if ((pFmeCtrl->SubType == SUBTYPE_DATA_NULL) || (pFmeCtrl->SubType == SUBTYPE_QOS_NULL))
					pEntry->MWDSInfo.NullPktNum++;
				if (pRxInfo->Mcast || pRxInfo->Bcast)
					pEntry->MWDSInfo.bcPktNum++;
				return FALSE;
			}
#endif /* MWDS */

#ifdef CONFIG_MAP_SUPPORT
			/* do not receive 3-address broadcast/multicast packet, */
			/* because the broadcast/multicast packet woulld be send using 4-address, */
			/* 1905 message is an exception, need to receive 3-address 1905 multicast, */
			/* because some vendor send only one 3-address 1905 multicast packet */
			/* 1905 daemon would filter and drop duplicate packet */
#ifdef CONFIG_MAP_3ADDR_SUPPORT
			if (pAd->MapAccept3Addr) {
				if ((GET_ENTRY_A4(pEntry) == A4_TYPE_MAP) &&
					(pRxInfo->Mcast || pRxInfo->Bcast)) {
					ptEntry = MacTableLookup(pAd, pRxBlk->Addr3);
					if (ptEntry) {
						if (IS_ENTRY_CLIENT(ptEntry)) {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"%s[%d]Drop: SA is from my CLI=%d\n\r",
								__func__, __LINE__, ptEntry->wdev->func_idx);
							return FALSE;
						}
					}

					for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
						twdev = &pAd->ApCfg.MBSSID[idx].wdev;
						if (RoutingTabLookup(pAd, twdev->func_idx, pRxBlk->Addr3, FALSE, &wcid) != NULL) {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"%s[%d]Drop: SA is from connected Agent=%d\n\r",
								__func__, __LINE__, twdev->func_idx);
							return FALSE;
						}
					}
					if (eth_lookup_entry_by_addr(pAd, wdev->func_idx, pRxBlk->Addr3, FALSE)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"%s[%d]Drop: SA is from connected eth CLI=%d\n\r",
								__func__, __LINE__, wdev->func_idx);
							return FALSE;
					}
				}
			} else {
				if (GET_ENTRY_A4(pEntry) == A4_TYPE_MAP &&
					(pRxInfo->Mcast || pRxInfo->Bcast) &&
					(memcmp(pRxBlk->Addr1, multicast_mac_1905, MAC_ADDR_LEN) != 0))
					return FALSE;

			}
#else
			if (GET_ENTRY_A4(pEntry) == A4_TYPE_MAP &&
				(pRxInfo->Mcast || pRxInfo->Bcast) &&
				(memcmp(pRxBlk->Addr1, multicast_mac_1905, MAC_ADDR_LEN) != 0))
				return FALSE;
#endif
#endif

			pStaCfg->StaStatistic.ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
			pStaCfg->StaStatistic.RxCount++;

			/* Process broadcast packets */
			if (pRxInfo->Mcast || pRxInfo->Bcast) {
				/* Process the received broadcast frame for AP-Client. */
				if (!ApCliHandleRxBroadcastFrame(pAd, pRxBlk, pEntry))
					return FALSE;
			}

			/* drop received packet which come from apcli */
			/* check if sa is to apcli */
			if (MAC_ADDR_EQUAL(pEntry->wdev->if_addr, pRxBlk->Addr3)) {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s[%d]Drop: SA is from my CLI=%d\n\r",
					__func__, __LINE__, pEntry->wdev->func_idx);
				return FALSE;	/* give up this frame */
			}
		}
				RX_BLK_SET_FLAG(pRxBlk, fRX_AP);
			goto ret;
		}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (pStaCfg->BssType == BSS_INFRA) {
		/* Infrastructure mode, check address 2 for BSSID */
		if (1
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			&& (!IS_TDLS_SUPPORT(pAd))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		   ) {
			if (!RTMPEqualMemory(pRxBlk->Addr2, pStaCfg->MlmeAux.Bssid, 6)) {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                                         "%s():  Infra-No my BSSID(Peer=>"MACSTR", My=>"MACSTR")!\n",
					 __func__, MAC2STR(pRxBlk->Addr2), MAC2STR(pStaCfg->MlmeAux.Bssid));
				return FALSE; /* Receive frame not my BSSID */
			}
		}
	} else {
		/* Ad-Hoc mode or Not associated */

		/* Ad-Hoc mode, check address 3 for BSSID */
		if (!RTMPEqualMemory(pRxBlk->Addr3, pAd->StaCfg[0].Bssid, 6)) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                                 "%s():  AdHoc-No my BSSID(Peer=>"MACSTR", My=>"MACSTR")!\n",
				 __func__, MAC2STR(pRxBlk->Addr3), MAC2STR(pStaCfg->Bssid));
			return FALSE; /* Receive frame not my BSSID */
		}
	}

#endif /*CONFIG_STA_SUPPORT */

	if (pEntry) {
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (pStaCfg->BssType == BSS_INFRA) {
			/* infra mode */
			RX_BLK_SET_FLAG(pRxBlk, fRX_AP);
#if defined(DOT11Z_TDLS_SUPPORT)  || defined(CFG_TDLS_SUPPORT)

			if ((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 0))
				RX_BLK_SET_FLAG(pRxBlk, fRX_DLS);
			else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
				ASSERT(pRxBlk->wcid == pEntry->wcid);
		} else {
			/* ad-hoc mode */
			if ((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 0))
				RX_BLK_SET_FLAG(pRxBlk, fRX_ADHOC);
		}
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
ret:
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	return hdr_len;
}

INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	/*
		For STA, direct to OS and no need to forwad the packet to WM
	*/
	return TRUE; /* need annouce to upper layer */
}
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */



VOID rx_data_frm_announce(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN struct wifi_dev *wdev)
{
	BOOLEAN eth_frame = FALSE;
	UCHAR *pData = pRxBlk->pData;
	UINT data_len = pRxBlk->DataSize;
	UCHAR wdev_idx = wdev->wdev_idx;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s():Invalid wdev_idx(%d)\n", __func__, wdev_idx);
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

#ifdef HDR_TRANS_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		eth_frame = TRUE;
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	}

#endif /* HDR_TRANS_SUPPORT */


	/* non-EAP frame */
	if (pEntry) {
		if (!RTMPCheckWPAframe(pAd, pEntry, pData, data_len, wdev_idx, eth_frame)) {
			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_CM) && FC && (FC->Type == FC_TYPE_DATA) && (pRxBlk->DataSize) && (FC->Wep)) {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: CM, wcid=%d\n",
							__func__, pRxBlk->wcid);
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Addr1="MACSTR"\t",
							MAC2STR(pRxBlk->Addr1));
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Addr2="MACSTR"\n",
							MAC2STR(pRxBlk->Addr2));
				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
#ifdef MWDS
			if (pEntry && GET_ENTRY_A4(pEntry) == A4_TYPE_MWDS) {
				if (!(FC && (FC->FrDs == 1) && (FC->ToDs == 1))) {
					/* release packet */
					RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
					return;
				}
			}
#endif /* MWDS */

#ifdef CONFIG_STA_SUPPORT

			/* TODO: revise for APCLI about this checking */
			if (pEntry->wdev->wdev_type == WDEV_TYPE_STA
					|| pEntry->wdev->wdev_type == WDEV_TYPE_GC) {
				PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);

				if (IS_ENTRY_PEER_AP(pEntry)) {
					/* printk("%s: RX\n", __func__); */
				} else /* drop all non-EAP DATA frame before peer's Port-Access-Control is secured */
					if (!STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
						RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
						return;
					}

#ifdef DOT11Z_TDLS_SUPPORT

				if (TDLS_CheckTDLSframe(pAd, pRxBlk->pData, pRxBlk->DataSize)) {
					UCHAR *pTmpBuf;

					pTmpBuf = pRxBlk->pData - LENGTH_802_11;
					NdisMoveMemory(pTmpBuf, pRxBlk->FC, LENGTH_802_11);
					REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
							pTmpBuf,
							pRxBlk->DataSize + LENGTH_802_11,
							pRxBlk->rx_signal.raw_rssi[0],
							pRxBlk->rx_signal.raw_rssi[1],
							pRxBlk->rx_signal.raw_rssi[2],
							pRxBlk->rx_signal.raw_rssi[3],
							0,
							(rxd_base != NULL) ? rxd_base->RxD1.ChFreq : 0,
							OPMODE_STA,
							wdev,
							pRxBlk->rx_rate.field.MODE);
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"!!! report TDLS Action DATA to MLME (len=%d) !!!\n",
							 pRxBlk->DataSize);
					RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket,
							NDIS_STATUS_FAILURE);
					return;
				}

#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef CFG_TDLS_SUPPORT
				cfg_tdls_rx_parsing(pAd, pRxBlk);
				/* return; */
#endif /* CFG_TDLS_SUPPORT */
			}

#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

			/* drop all non-EAP DATA frame before peer's Port-Access-Control is secured */
			if ((pEntry->wdev->wdev_type == WDEV_TYPE_AP || pEntry->wdev->wdev_type == WDEV_TYPE_GO) &&
					IS_ENTRY_CLIENT(pEntry) && (pEntry->PrivacyFilter == Ndis802_11PrivFilter8021xWEP)) {
				/*
				   If	1) no any EAP frame is received within 5 sec and
				   2) an encrypted non-EAP frame from peer associated STA is received,
				   AP would send de-authentication to this STA.
				 */
				if (FC != NULL && FC->Wep &&
						pEntry->StaConnectTime > 5 && pEntry->SecConfig.Handshake.WpaState < AS_AUTHENTICATION2) {
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, "==> De-Auth this STA("MACSTR")\n",
								MAC2STR(pEntry->Addr));
					MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				}

				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				BOOLEAN PortSecured = wdev->PortSecured;
				/* CFG TODO: ApCli on OPMODE_STA */
				/* ASSERT(wdev == &pAd->StaCfg[0].wdev); */

				/* drop all non-EAP DATA frame before peer's Port-Access-Control is secured */
				if (FC && FC->Wep) {
					/* unsupported cipher suite */
					if (IS_NO_SECURITY_Entry(wdev)) {
						RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
						return;
					}
				} else {
					/* encryption in-use but receive a non-EAPOL clear text frame, drop it */
					if (IS_SECURITY_Entry(wdev)
							&& (PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
						RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
						return;
					}
				}
			}
#endif /* CONFIG_STA_SUPPORT */
#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->pMbss) && ((pEntry->pMbss->HotSpotCtrl.HotSpotEnable) || (pEntry->pMbss->WNMCtrl.ProxyARPEnable))) {
#ifdef CONFIG_HOTSPOT_R3
			if ((pEntry->pMbss->HotSpotCtrl.HotSpotEnable) && (hotspot_osu_data_handler(pAd, pEntry, pRxBlk, wdev) == TRUE))
				return;
#endif
#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
				if (hotspot_rx_handler(pAd, pEntry, pRxBlk) == TRUE)
					return;
			}
#endif

#endif /* CONFIG_HOTSPOT */

#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
			APCLI_ROAMING_ENHANCE_CHECK(pAd, pEntry, pRxBlk, wdev);
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef STATS_COUNT_SUPPORT

			if ((IS_ENTRY_CLIENT(pEntry)) && (pEntry->pMbss)) {
				BSS_STRUCT *pMbss = pEntry->pMbss;
				UCHAR *pDA = pRxBlk->Addr3;
#ifdef TR181_SUPPORT
			UCHAR bandIdx = HcGetBandByWdev(wdev);
#endif
			if (((*pDA) & 0x1) == 0x01) {
				if (IS_BROADCAST_MAC_ADDR(pDA)) {
					pMbss->bcPktsRx++;
#ifdef TR181_SUPPORT
					pAd->WlanCounters[bandIdx].bcPktsRx.QuadPart++;
#endif
				} else {
					pMbss->mcPktsRx++;
#ifdef TR181_SUPPORT
					pAd->WlanCounters[bandIdx].mcPktsRx.QuadPart++;
#endif
				}
			} else {
				pMbss->ucPktsRx++;
#ifdef TR181_SUPPORT
				pAd->WlanCounters[bandIdx].ucPktsRx.QuadPart++;
#endif
			}
		}

#endif /* STATS_COUNT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT

			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) /*&& (pAd->CommonCfg.bDisableReordering == 0)*/)
				indicate_ampdu_pkt(pAd, pRxBlk, wdev_idx);
			else
#endif /* DOT11_N_SUPPORT */
				if (RX_BLK_TEST_FLAG(pRxBlk, fRX_ARALINK))
					indicate_agg_ralink_pkt(pAd, pEntry, pRxBlk, wdev->wdev_idx);
				else
					indicate_802_11_pkt(pAd, pRxBlk, wdev_idx);
		} else {
			RX_BLK_SET_FLAG(pRxBlk, fRX_EAP);
#ifdef CONFIG_AP_SUPPORT

			/* Update the WPA STATE to indicate the EAP handshaking is started */
			if (IS_ENTRY_CLIENT(pEntry)) {
				if (pEntry->SecConfig.Handshake.WpaState == AS_AUTHENTICATION)
					pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION2;
			}

#endif /* CONFIG_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT

			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU)
					/*&& (pAd->CommonCfg.bDisableReordering == 0)*/)
				indicate_ampdu_pkt(pAd, pRxBlk, wdev_idx);
			else
#endif /* DOT11_N_SUPPORT */
			{
#ifndef HOSTAPD_HS_R2_SUPPORT
#ifdef CONFIG_HOTSPOT_R2
				UCHAR *pData = (UCHAR *)pRxBlk->pData;

				if (pEntry) {
					BSS_STRUCT *pMbss = pEntry->pMbss;

					if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
							/* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
							NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
						pData += 6;

					if (NdisEqualMemory(EAPOL, pData, 2))
						pData += 2;
					if (IS_ENTRY_CLIENT(pEntry) && pEntry->pMbss) {
						if (((*(pData + 1) == EAPOLStart) || (*(pData + 1) == EAPPacket)) && (pMbss->HotSpotCtrl.HotSpotEnable == 1) &&  IS_AKM_WPA2(pMbss->wdev.SecConfig.AKMMap) && (pEntry->hs_info.ppsmo_exist == 1)) {
							UCHAR HS2_Header[4] = {0x50, 0x6f, 0x9a, 0x12};

							memcpy(&pRxBlk->pData[pRxBlk->DataSize], HS2_Header, 4);
							memcpy(&pRxBlk->pData[pRxBlk->DataSize + 4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
							MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: hotspot rcv eapol start, %x:%x:%x:%x\n",
										__func__, pRxBlk->pData[pRxBlk->DataSize + 4], pRxBlk->pData[pRxBlk->DataSize + 5],
										pRxBlk->pData[pRxBlk->DataSize + 6], pRxBlk->pData[pRxBlk->DataSize + 7]);
							pRxBlk->DataSize += 8;
						}
					}
				}
#endif
#endif /*HOSTAPD_HS_R2_SUPPORT*/
				/* Determin the destination of the EAP frame */
				/*  to WPA state machine or upper layer */
				rx_eapol_frm_handle(pAd, pEntry, pRxBlk, wdev_idx);
			}
		}
	}
}

static BOOLEAN amsdu_non_ampdu_sanity(RTMP_ADAPTER *pAd, UINT16 cur_sn, UINT8 cur_amsdu_state,
					UINT16 previous_sn, UINT8 previous_amsdu_state)
{
	BOOLEAN amsdu_miss = FALSE;

	if (cur_sn != previous_sn) {
		if ((previous_amsdu_state == FIRST_AMSDU_FORMAT) ||
				(previous_amsdu_state == MIDDLE_AMSDU_FORMAT)) {
			amsdu_miss = TRUE;
		}
	} else {
		if (((previous_amsdu_state == FIRST_AMSDU_FORMAT) ||
			(previous_amsdu_state == MIDDLE_AMSDU_FORMAT)) &&
				(cur_amsdu_state == FIRST_AMSDU_FORMAT)) {
			amsdu_miss = TRUE;
		}


	}

	return amsdu_miss;
}

static BOOLEAN rx_time_pri_mgmt_frame(PHEADER_802_11 pHdr)
{
	BOOLEAN time_pri_mgmt = FALSE;

	if (pHdr->FC.SubType == SUBTYPE_ACTION) {
		UCHAR category = (UCHAR) (pHdr->Octet[(pHdr->FC.Order ? 4 : 0)]);
		UCHAR ht_action = (UCHAR) (pHdr->Octet[(pHdr->FC.Order ? 4 : 0) + 1]);
		if (category == CATEGORY_HT) {
			switch (ht_action) {
				case PSMP_ACTION:
				case MIMO_CHA_MEASURE_ACTION:
				case MIMO_N_BEACONFORM:
				case MIMO_BEACONFORM:
				case ANTENNA_SELECT:
					time_pri_mgmt = TRUE;
					break;
				default:
					break;
			}
		}
	}

	return time_pri_mgmt;
}

INT rx_chk_amsdu_invalid_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, struct wifi_dev *wdev)
{
	UINT16 wcid = pRxBlk->wcid;
	STA_TR_ENTRY *trEntry = NULL;
	INT sn = pRxBlk->SN;
	UINT8 AmsduState = pRxBlk->AmsduState;
	UCHAR up = 0;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	/*For AMSDU Only Packet */
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU))
		return NDIS_STATUS_SUCCESS;

	/*check is valid sta entry*/
	if (!IS_TR_WCID_VALID(pAd, wcid))
		return NDIS_STATUS_SUCCESS;

	/*check sta tr entry is exist*/
	trEntry = &tr_ctl->tr_entry[wcid];

	if (!trEntry)
		return NDIS_STATUS_SUCCESS;

	up = pRxBlk->UserPriority;

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU) && (pRxBlk->AMSDU_ADDR)) {
		UCHAR *CmpAddr = pRxBlk->Addr2;

		if ((((FRAME_CONTROL *)pRxBlk->FC)->ToDs == 0) && (((FRAME_CONTROL *)pRxBlk->FC)->FrDs == 1))
			CmpAddr = pRxBlk->Addr1;

		if ((AmsduState == FIRST_AMSDU_FORMAT) &&
			NdisCmpMemory(pRxBlk->AMSDU_ADDR, CmpAddr, MAC_ADDR_LEN)) {

			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: AMSDU ATTACK,	wcid=%d SN-AN(%d,%d)\n",
					__func__, pRxBlk->wcid, pRxBlk->SN, AmsduState);
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "80211 Addr=%02x:%02x:%02x:%02x:%02x:%02x\t",
					PRINT_MAC(pRxBlk->AMSDU_ADDR));
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "8023 Addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(CmpAddr));

			trEntry->is_amsdu_invalid[up] = TRUE;
			trEntry->previous_amsdu_invalid_sn[up] = sn;
			trEntry->previous_amsdu_invalid_state[up] = AmsduState;

			return NDIS_STATUS_FAILURE;
		} else {
			if ((trEntry->is_amsdu_invalid[up]) &&
					(trEntry->previous_amsdu_invalid_sn[up] == sn)) {

					trEntry->previous_amsdu_invalid_state[up] = AmsduState;
					if (AmsduState == FINAL_AMSDU_FORMAT) {
						trEntry->is_amsdu_invalid[up] = FALSE;
						trEntry->previous_amsdu_invalid_sn[up] = 0;
						trEntry->previous_amsdu_invalid_state[up] = 0;
					}

					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s: AMSDU ATTACK, wcid=%d SN-AN(%d,%d)\n",
						__func__, pRxBlk->wcid, pRxBlk->SN, AmsduState);
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"80211 Addr=%02x:%02x:%02x:%02x:%02x:%02x\t",
						PRINT_MAC(pRxBlk->AMSDU_ADDR));
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"8023 Addr=%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(CmpAddr));

					return NDIS_STATUS_FAILURE;
			} else {
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"%s: NON-(AMSDU ATTACK), wcid=%d SN-AN(%d,%d)\n",
						__func__, pRxBlk->wcid, pRxBlk->SN, AmsduState);
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"80211 Addr=%02x:%02x:%02x:%02x:%02x:%02x\t",
						PRINT_MAC(pRxBlk->AMSDU_ADDR));
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"8023 Addr=%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(CmpAddr));
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
}

INT rx_chk_duplicate_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, struct wifi_dev *wdev)
{
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	UINT16 wcid = pRxBlk->wcid;
	STA_TR_ENTRY *trEntry = NULL;
	INT sn = pRxBlk->SN;
	UINT32 WmmIndex = HcGetWmmIdx(pAd, wdev);
	UCHAR up = 0;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	/*check if AMPDU frame ignore it, since AMPDU wil handle reorder */
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU))
		return NDIS_STATUS_SUCCESS;

	/*check is vaild sta entry*/
	if (!IS_TR_WCID_VALID(pAd, wcid))
		return NDIS_STATUS_SUCCESS;

	/*check sta tr entry is exist*/
	trEntry = &tr_ctl->tr_entry[wcid];

	if (!trEntry)
		return NDIS_STATUS_SUCCESS;

	if ((pFmeCtrl->Type == FC_TYPE_DATA && pFmeCtrl->SubType == SUBTYPE_DATA_NULL) ||
		(pFmeCtrl->Type == FC_TYPE_DATA && pFmeCtrl->SubType == SUBTYPE_QOS_NULL))
		return NDIS_STATUS_SUCCESS;

	up = (WmmIndex * 4) + pRxBlk->UserPriority;

	/*check frame is QoS or Non-QoS frame*/
	if (!(pFmeCtrl->SubType & 0x08) || up >= NUM_OF_UP)
		up = (NUM_OF_UP - 1);

#ifdef SW_CONNECT_SUPPORT
	trEntry->RxSeq[pRxBlk->UserPriority] = sn;
#endif /* SW_CONNECT_SUPPORT */

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU)) {
		if ((amsdu_non_ampdu_sanity(pAd, sn, pRxBlk->AmsduState,
			trEntry->previous_sn[up], trEntry->previous_amsdu_state[up]))
			|| (pRxBlk->AmsduState == FINAL_AMSDU_FORMAT)) {
			trEntry->cacheSn[up] =  trEntry->previous_sn[up];
		}

		trEntry->previous_amsdu_state[up] = pRxBlk->AmsduState;
		trEntry->previous_sn[up] = sn;

		if (!pFmeCtrl->Retry || trEntry->cacheSn[up] != sn)
			return NDIS_STATUS_SUCCESS;
	} else {
		if (!pFmeCtrl->Retry || trEntry->cacheSn[up] != sn) {
			trEntry->cacheSn[up] = sn;
			return NDIS_STATUS_SUCCESS;
		}
	}

	/* Middle/End of fragment */
	if (pRxBlk->FN && pRxBlk->FN != pAd->FragFrame.LastFrag)
		return NDIS_STATUS_SUCCESS;

	MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s(): pFrameCtrl->Retry=%d, trEntry->cacheSn[%d]=%d, pkt->sn=%d\n",
			 __func__, pFmeCtrl->Retry, up, trEntry->cacheSn[up], sn);
	/*is duplicate frame, should return failed*/
	return NDIS_STATUS_FAILURE;
}

INT rx_chk_duplicate_mgmt_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	UINT16 wcid = pRxBlk->wcid;
	STA_TR_ENTRY *trEntry = NULL;
	INT sn = pRxBlk->SN;
	UINT8 sn_cat = 0;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	/* check if AMPDU frame ignore it, since AMPDU wil handle reorder */
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU))
		return NDIS_STATUS_SUCCESS;

	/* check is vaild sta entry */
	if (!IS_TR_WCID_VALID(pAd, wcid))
		return NDIS_STATUS_SUCCESS;

	/*check sta tr entry is exist*/
	trEntry = &tr_ctl->tr_entry[wcid];

	if (!trEntry)
		return NDIS_STATUS_SUCCESS;

	/* check sn category */
	sn_cat = (rx_time_pri_mgmt_frame((HEADER_802_11 *)pRxBlk->FC) ? TIME_PRI_MGMT : NOT_TIME_PRI_MGMT);

	if (!pFmeCtrl->Retry || trEntry->cacheMgmtSn[sn_cat] != sn) {
		trEntry->cacheMgmtSn[sn_cat] = sn;
		return NDIS_STATUS_SUCCESS;
	}

	/* Middle/End of fragment */
	if (pRxBlk->FN && pRxBlk->FN != pAd->FragFrame.LastFrag)
		return NDIS_STATUS_SUCCESS;

	MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s(): pFrameCtrl->Retry=%d, trEntry->cacheMgmtSn[%d]=%d, pkt->sn=%d\n",
			 __func__, pFmeCtrl->Retry, sn_cat, trEntry->cacheMgmtSn[sn_cat], sn);
	/* is duplicate frame, should return failed */
	return NDIS_STATUS_FAILURE;
}

VOID rx_802_3_data_frm_announce(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk, struct wifi_dev *wdev)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_CM) && FC   &&	(FC->Type == FC_TYPE_DATA)) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: CM,	wcid=%d\n", __func__, pRxBlk->wcid);
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Addr1="MACSTR"\t", MAC2STR(pRxBlk->Addr1));
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Addr2="MACSTR"\n", MAC2STR(pRxBlk->Addr2));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
	if (IS_ENTRY_CLIENT(pEntry) && (pEntry->pMbss) && ((pEntry->pMbss->HotSpotCtrl.HotSpotEnable) || (pEntry->pMbss->WNMCtrl.ProxyARPEnable))) {
#ifdef CONFIG_HOTSPOT_R3
		if ((pEntry->pMbss->HotSpotCtrl.HotSpotEnable) && (hotspot_osu_data_handler(pAd, pEntry, pRxBlk, wdev) == TRUE))
			return;
#endif
#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
		if (hotspot_rx_handler(pAd, pEntry, pRxBlk) == TRUE)
			return;
#endif
	}

#endif /* CONFIG_HOTSPOT */

#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	APCLI_ROAMING_ENHANCE_CHECK(pAd, pEntry, pRxBlk, wdev);
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU))
		indicate_ampdu_pkt(pAd, pRxBlk, wdev->wdev_idx);
	else
		indicate_802_3_pkt(pAd, pRxBlk, wdev->wdev_idx);
}

static MAC_TABLE_ENTRY *check_rx_pkt_allowed(RTMP_ADAPTER *pAd, RX_BLK *rx_blk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_AP_SUPPORT
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)rx_blk->FC;
#endif

#ifdef SW_CONNECT_SUPPORT
	/* in Rx : pRxBlk->wcid : means the HW WCID , not the SW wcid
			 so need to check the HW boundary here.
			 if not unicast, call research_entry() to translate rx_blk->wcid into sw wcid
	     TBD : maybe call HcGetSwWcid() instead of call research_entry() is fast, but need  to protect
	*/
	if (VALID_UCAST_ENTRY_WCID_HW(pAd, rx_blk->wcid))
#else /* SW_CONNECT_SUPPORT */
	if (VALID_UCAST_ENTRY_WCID(pAd, rx_blk->wcid))
#endif /* !SW_CONNECT_SUPPORT */
	{
#if defined(OUI_CHECK_SUPPORT) && defined(MAC_REPEATER_SUPPORT)
		BOOLEAN	bCheck_ApCli = FALSE;
		UINT16	orig_wcid = rx_blk->wcid;
#endif
		pEntry = &pAd->MacTab.Content[rx_blk->wcid];
#ifdef OUI_CHECK_SUPPORT

		if (
			(pAd->MacTab.oui_mgroup_cnt > 0)
#ifdef MAC_REPEATER_SUPPORT
			|| (pAd->ApCfg.RepeaterCliSize[HcGetBandByWdev(pEntry->wdev)] != 0)
#endif
		) {
			if (NdisCmpMemory(pEntry->Addr, rx_blk->Addr2, MAC_ADDR_LEN))
				pEntry = research_entry(pAd, rx_blk);

#ifdef MAC_REPEATER_SUPPORT

			if (pEntry) {
				if (IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_PEER_AP(pEntry))
					bCheck_ApCli = TRUE;
			}

#endif
		}

#ifdef MAC_REPEATER_SUPPORT

		if (bCheck_ApCli == TRUE) {
			if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
				REPEATER_CLIENT_ENTRY	*pReptEntry = NULL;
				BOOLEAN					entry_found = FALSE;

				pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &rx_blk->Addr1[0], TRUE);

				if (pReptEntry && (pReptEntry->CliValid == TRUE)) {
					pEntry = pReptEntry->pMacEntry;
					rx_blk->wcid = pEntry->wcid;
					entry_found = TRUE;
				} else if ((pReptEntry == NULL) && IS_ENTRY_PEER_AP(pEntry)) {/* this packet is for APCLI */
					INT apcli_idx = pEntry->func_tb_idx;

					pEntry = &pAd->MacTab.Content[pAd->StaCfg[apcli_idx].MacTabWCID];
					rx_blk->wcid = pEntry->wcid;
					entry_found = TRUE;
				}

				if (entry_found) {
					if (orig_wcid != rx_blk->wcid) {
						pAd->MacTab.repeater_wcid_error_cnt++;
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"orig_wcid=%d,pRxBlk->wcid=%d\n\r", orig_wcid, rx_blk->wcid);
					}

					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s()[%d]:"MACSTR" recv data\n\r",
						__func__, __LINE__, MAC2STR(rx_blk->Addr1));
				} else {
					if ((rx_blk->Addr1[0] & 0x01) == 0x01) {/* BM packet,find apcli entry */
						if (IS_ENTRY_PEER_AP(pEntry)) {
							rx_blk->wcid = pEntry->wcid;
							pAd->MacTab.repeater_bm_wcid_error_cnt++;
						} else if (IS_ENTRY_REPEATER(pEntry)) {
							UINT16 apcli_wcid = 0;

							if (pEntry->wdev) {
								PSTA_ADMIN_CONFIG apcli = GetStaCfgByWdev(pAd, pEntry->wdev);
								apcli_wcid = apcli->MacTabWCID;
							}
							else   /* use default apcli0 */
								apcli_wcid = pAd->StaCfg[0].MacTabWCID;

							pEntry = &pAd->MacTab.Content[apcli_wcid];
							rx_blk->wcid = pEntry->wcid;
							pAd->MacTab.repeater_bm_wcid_error_cnt++;
						}
					} else {
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"%s()[%d]:Cannot found WCID of data packet!\n",
							__func__, __LINE__);
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"A1:"MACSTR", A2:"MACSTR"\n",
							MAC2STR(rx_blk->Addr1),
							MAC2STR(rx_blk->Addr2));
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"wcid=%d,orig_wcid=%d,M/B/U=%d/%d/%d\r\n",
							pEntry->wcid, orig_wcid,
							rx_blk->pRxInfo->Mcast,
							rx_blk->pRxInfo->Bcast,
							rx_blk->pRxInfo->U2M);
					}
				}
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
#endif /*OUI_CHECK_SUPPORT*/
	} else {
#ifdef AIR_MONITOR
		if (pAd->MntEnable[rx_blk->band]) {
			UCHAR count;
			MNT_STA_ENTRY *pMntTable;

			for (count = 0; count < MAX_NUM_OF_MONITOR_STA; count++) {
				pMntTable = &pAd->MntTable[rx_blk->band][count];

				if (pMntTable->bValid &&
					 (MAC_ADDR_EQUAL(rx_blk->Addr1, pMntTable->addr) ||
					 MAC_ADDR_EQUAL(rx_blk->Addr2, pMntTable->addr))) {
					pEntry = pMntTable->pMacEntry;
					break;
				}
			}
		}
#endif
		if (!pEntry)
			pEntry = research_entry(pAd, rx_blk);

		if (pEntry) {
			rx_blk->wcid = pEntry->wcid;
#ifdef MAC_REPEATER_SUPPORT

			if (IS_ENTRY_PEER_AP(pEntry))
				rx_blk->wcid = pEntry->wcid;
			else if (IS_ENTRY_REPEATER(pEntry)) {
				UINT16 apcli_wcid = 0;

				if (pEntry->wdev) {
					UCHAR ifidx = pEntry->wdev->func_idx;

					if (ifidx < pAd->ApCfg.ApCliNum)
						apcli_wcid = pAd->StaCfg[ifidx].MacTabWCID;
					/* use default apcli0 */
					else if (ifidx < pAd->MSTANum)
						apcli_wcid = pAd->StaCfg[0].MacTabWCID;
					else {
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "%s() invalid sta entry id(%d)\n", __func__, ifidx);
						return NULL;
					}

					if (apcli_wcid >= MAX_LEN_OF_MAC_TABLE) {
						MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"%s(): apcli_wcid >= MAX_LEN_OF_MAC_TABLE!\n", __func__);
						return NULL;
					}
					pEntry = &pAd->MacTab.Content[apcli_wcid];
					rx_blk->wcid = pEntry->wcid;
				}
				else {
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"%s() get wdev fail.\n", __func__);
					return NULL;
				}
			}
#endif
		}
	}

	if (pEntry && pEntry->wdev) {
		struct wifi_dev_ops *ops = pEntry->wdev->wdev_ops;

		if (ops->rx_pkt_allowed) {
			if (!ops->rx_pkt_allowed(pAd, pEntry->wdev, rx_blk)) {
				MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s(): rx_pkt_allowed drop this paket!\n", __func__);
				return NULL;
			}
		}
	} else {
		if (pEntry) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"invalid hdr_len, wdev=%p!\n", pEntry->wdev);
		} else {
#ifdef CONFIG_AP_SUPPORT
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
			if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)) {
				if (pAd->WdsTab.Mode[rx_blk->band] && IS_UCAST_MAC(rx_blk->Addr1)) {
					struct wifi_dev *main_bss_wdev = NULL;

					if (rx_blk->band == DBDC_BAND0)
						main_bss_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
					else
						main_bss_wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;

					if (MAC_ADDR_EQUAL(rx_blk->Addr1, main_bss_wdev->if_addr))
						FindWdsEntry(pAd, rx_blk);
					else
						MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
							 "%s(): WDS for TA("MACSTR") is not enabled\n",
							  __func__, MAC2STR(rx_blk->Addr1));
				}
			}

#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) */

			/* check if Class2 or 3 error */
			if ((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))
				ap_chk_cl2_cl3_err(pAd, rx_blk);
#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT)
			else if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)) {
				ap_chk_cl2_cl3_err(pAd, rx_blk);
			}
#endif

#endif /* CONFIG_AP_SUPPORT */
		}
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s(): drop this packet as pEntry NULL OR !rx_pkt_allowed !!\n", __func__);

		return NULL;
	}

	return pEntry;
}


VOID dev_rx_802_3_data_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
#endif
	struct wifi_dev_ops *ops;

	pEntry = check_rx_pkt_allowed(pAd, pRxBlk);

	if (!pEntry)
		goto drop;

	wdev = pEntry->wdev;
	ops = wdev->wdev_ops;

	if (ops->ieee_802_3_data_rx(pAd, wdev, pRxBlk, pEntry))
		return;

drop:
#ifdef CONFIG_AP_SUPPORT

	/* Increase received error packet counter per BSS */
	if (pFmeCtrl->FrDs == 0 &&
		pRxInfo->U2M &&
		pRxBlk->bss_idx < pAd->ApCfg.BssidNum) {
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxDropCount++;
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxErrorCount++;
	}
#ifdef TXRX_STAT_SUPPORT
	if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
		INC_COUNTER64(pEntry->pMbss->stat_bss.RxPacketDroppedCount);
	}
#endif

#endif
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
}


/*
 All Rx routines use RX_BLK structure to hande rx events
 It is very important to build pRxBlk attributes
  1. pHeader pointer to 802.11 Header
  2. pData pointer to payload including LLC (just skip Header)
  3. set payload size including LLC to DataSize
  4. set some flags with RX_BLK_SET_FLAG()
*/
VOID dev_rx_802_11_data_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
#ifdef CONFIG_AP_SUPPORT
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
#endif
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *ops;

	pEntry = check_rx_pkt_allowed(pAd, pRxBlk);

	if (!pEntry)
		goto drop;

	wdev = pEntry->wdev;
	ops = wdev->wdev_ops;

	if (ops->ieee_802_11_data_rx(pAd, wdev, pRxBlk, pEntry))
		return;

drop:
#ifdef CONFIG_AP_SUPPORT

	/* Increase received error packet counter per BSS */
	if (pFmeCtrl->FrDs == 0 &&
		pRxInfo->U2M &&
		(pRxBlk->bss_idx < pAd->ApCfg.BssidNum) &&
		!(pFmeCtrl->SubType & 0x04)) {
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxDropCount++;
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxErrorCount++;
	}
#ifdef TXRX_STAT_SUPPORT
	if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && !(pFmeCtrl->SubType & 0x04)) {
		INC_COUNTER64(pEntry->pMbss->stat_bss.RxPacketDroppedCount);
	}
#endif
#endif /* CONFIG_AP_SUPPORT */
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}

/*
		========================================================================
		Routine Description:
			Process RxDone interrupt, running in DPC level

		Arguments:
			pAd    Pointer to our adapter

		Return Value:
			None

		Note:
			This routine has to maintain Rx ring read pointer.
	========================================================================
*/
NDIS_STATUS header_packet_process(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pRxPacket,
	RX_BLK *pRxBlk)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if ((pRxBlk->DataSize == 0) && (pRxPacket)) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s():Packet Length is zero!\n", __func__);
			return NDIS_STATUS_INVALID_DATA;
		}
	}

#endif /* MT_MAC */
#ifdef CONFIG_STA_SUPPORT
#ifdef SNIFFER_SUPPORT

	if (MONITOR_ON(pAd) && pAd->monitor_ctrl.CurrentMonitorMode == MONITOR_MODE_REGULAR_RX)
		return NDIS_STATUS_SUCCESS;

#endif /* SNIFFER_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE

	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_RXWI) {
		dbQueueEnqueueRxFrame(GET_OS_PKT_DATAPTR(pRxPacket),
							  (UCHAR *)pHeader,
							  pAd->CommonCfg.DebugFlags);
	}

#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

#ifdef RT_BIG_ENDIAN
   	 RTMPFrameEndianChange(pAd, pRxBlk->FC, DIR_READ, TRUE);
#endif /* RT_BIG_ENDIAN */


#ifdef AIR_MONITOR
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (!pRxBlk->pRxInfo->CipherErr && !pRxBlk->pRxInfo->Crc) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			if (IS_WCID_VALID(pAd, pRxBlk->wcid))
				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if (pEntry && IS_ENTRY_MONITOR(pEntry)) {
				FRAME_CONTROL *fc = (FRAME_CONTROL *)pRxBlk->FC;

				if (pAd->MntEnable[pRxBlk->band])
					Air_Monitor_Pkt_Report_Action(pAd, pRxBlk->wcid, pRxBlk);

				if (!((fc->Type == FC_TYPE_MGMT) && (fc->SubType == SUBTYPE_PROBE_REQ))) {
					RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
					return NDIS_STATUS_INVALID_DATA;
				}
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* AIR_MONITOR */

#ifdef RT_CFG80211_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd) &&
		(NdisEqualMemory(pAd->cfg80211_ctrl.P2PCurrentAddress, pHeader->Addr1, MAC_ADDR_LEN) ||
		 (pHeader->FC.SubType == SUBTYPE_PROBE_REQ)))
		SET_PKT_OPMODE_AP(pRxBlk);
	else if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) &&
			 (((pHeader->FC.SubType == SUBTYPE_BEACON || pHeader->FC.SubType == SUBTYPE_PROBE_RSP) &&
			   NdisEqualMemory(pAd->StaCfg[MAIN_MBSSID].CfgApCliBssid, pHeader->Addr2, MAC_ADDR_LEN)) ||
			  (pHeader->FC.SubType == SUBTYPE_PROBE_REQ) ||
			  NdisEqualMemory(pAd->StaCfg[MAIN_MBSSID].MlmeAux.Bssid, pHeader->Addr2, MAC_ADDR_LEN))) {
		/*
		   1. Beacon & ProbeRsp for Connecting & Tracking
			   2. ProbeReq for P2P Search
			   3. Any Packet's Addr2 Equals MlmeAux.Bssid when connected
			 */
		SET_PKT_OPMODE_AP(pRxBlk);
	} else
#endif /* RT_CFG80211_P2P_SUPPORT */
	{
		/* todo: Bind to pAd->net_dev */
		if (RTMP_CFG80211_HOSTAPD_ON(pAd))
			SET_PKT_OPMODE_AP(pRxBlk);
		else
			SET_PKT_OPMODE_STA(pRxBlk);
	}

#endif /* RT_CFG80211_SUPPORT */
#ifdef CFG80211_MULTI_STA

	if (RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev) &&
		(((FC->SubType == SUBTYPE_BEACON || FC->SubType == SUBTYPE_PROBE_RSP) &&
		  NdisEqualMemory(pAd->StaCfg[MAIN_MBSSID].CfgApCliBssid, pRxBlk->Addr2, MAC_ADDR_LEN)) ||
		 (pHeader->FC.SubType == SUBTYPE_PROBE_REQ) ||
		 NdisEqualMemory(pAd->StaCfg[MAIN_MBSSID].MlmeAux.Bssid, pRxBlk->Addr2, MAC_ADDR_LEN)))
		SET_PKT_OPMODE_AP(pRxBlk);
	else
		SET_PKT_OPMODE_STA(pRxBlk);

#endif /* CFG80211_MULTI_STA */
	/* Increase Total receive byte counter after real data received no mater any error or not */
	pAd->RalinkCounters.ReceivedByteCount += pRxBlk->DataSize;
	pAd->RalinkCounters.OneSecReceivedByteCount += pRxBlk->DataSize;
	pAd->RalinkCounters.RxCount++;
	pAd->RalinkCounters.OneSecRxCount++;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd)) {
		/* Note: temp for wlan_service/original coexistence */
#ifdef CONFIG_WLAN_SERVICE
		struct service_test *serv_test;
		serv_test = (struct service_test *)(pAd->serv.serv_handle);

		net_ad_rx_done_handle(serv_test->test_winfo, (VOID *)pRxBlk);
#else
		MT_ATERxDoneHandle(pAd, pRxBlk);
#endif /* CONFIG_WLAN_SERVICE */
	return NDIS_STATUS_SUCCESS;
	}
#endif /* CONFIG_ATE */

#ifdef STATS_COUNT_SUPPORT
	INC_COUNTER64(pAd->WlanCounters[0].ReceivedFragmentCount);
#endif /* STATS_COUNT_SUPPORT */

	/* Check for all RxD errors */
	if (rtmp_chk_rx_err(pAd, pRxBlk) != NDIS_STATUS_SUCCESS) {
		pAd->Counters8023.RxErrors++;
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s(): CheckRxError!\n", __func__);
		return NDIS_STATUS_INVALID_DATA;
	}

	/* TODO: shiang-usw, for P2P, we original has following code, need to check it and merge to correct place!!! */
	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rx_packet_process(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pRxPacket,
	RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
#ifdef RATE_PRIOR_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif/*RATE_PRIOR_SUPPORT*/
#ifdef VLAN_SUPPORT
	UCHAR eapol_80211_frame_offset = 0;
#endif

#ifdef SNIFFER_SUPPORT
	struct wifi_dev *wdev;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pRxPacket);
#endif /* SNIFFER_SUPPORT */
	enum pkt_type {
		_802_3_data,
		_802_3_eapol,
		_802_3_vlan_data,
		_802_3_vlan_eapol,
	} type;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd)) {
		/* TODO::Check if Rx cutthrough stable */
		if (pRxPacket)
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

		return NDIS_STATUS_SUCCESS;
	}

#endif /* CONFIG_ATE */
#ifdef SNIFFER_SUPPORT

	if (MONITOR_ON(pAd) && pAd->monitor_ctrl.CurrentMonitorMode == MONITOR_MODE_FULL) {
		PNDIS_PACKET	pClonePacket;
		PNDIS_PACKET    pTmpRxPacket;

		if (pAd->monitor_ctrl.FrameType == FC_TYPE_RSVED ||
			pAd->monitor_ctrl.FrameType == FC->Type) {
			if (!pAd->monitor_ctrl.MacFilterOn || (pAd->monitor_ctrl.MacFilterOn &&
				((pRxBlk->Addr1 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr1)) ||
				(pRxBlk->Addr2 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr2)) ||
				(pRxBlk->Addr3 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr3)) ||
				(pRxBlk->Addr4 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr4))))) {
				wdev = pAd->wdev_list[wdev_idx];
				pTmpRxPacket = pRxBlk->pRxPacket;
				pClonePacket = ClonePacket(MONITOR_ON(pAd), wdev->if_dev, pRxBlk->pRxPacket,
							pRxBlk->pData, pRxBlk->DataSize);
				pRxBlk->pRxPacket = pClonePacket;
				STA_MonPktSend(pAd, pRxBlk);
				pRxBlk->pRxPacket = pTmpRxPacket;
			}
		}
	}
	if (MONITOR_ON(pAd) && pAd->monitor_ctrl.CurrentMonitorMode == MONITOR_MODE_REGULAR_RX) {
		if (pAd->monitor_ctrl.FrameType == FC_TYPE_RSVED ||
				pAd->monitor_ctrl.FrameType == FC->Type) {
			if (!pAd->monitor_ctrl.MacFilterOn || (pAd->monitor_ctrl.MacFilterOn &&
				((pRxBlk->Addr1 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr1)) ||
				(pRxBlk->Addr2 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr2)) ||
				(pRxBlk->Addr3 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr3)) ||
				(pRxBlk->Addr4 && MAC_ADDR_EQUAL(pAd->monitor_ctrl.MacFilterAddr, pRxBlk->Addr4))))) {
				STA_MonPktSend(pAd, pRxBlk);
				return NDIS_STATUS_SUCCESS;
			}
		}
	}

#endif /* SNIFFER_SUPPORT */

	if (FC == NULL) {
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_SUCCESS;
	}
#ifdef RATE_PRIOR_SUPPORT
	if (pAd->LowRateCtrl.RatePrior == 1) {
		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
		if (pEntry && FC->SubType != SUBTYPE_DATA_NULL && FC->SubType != SUBTYPE_QOS_NULL) {
			pEntry->McsTotalRxCount++;
			if (pRxBlk->rx_rate.field.MCS <= RATE_PRIOR_LOW_RATE_THRESHOLD)
				pEntry->McsLowRateRxCount++;
		}
	}
#endif/*RATE_PRIOR_SUPPORT*/

	switch (FC->Type) {
	case FC_TYPE_DATA:
		/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("rx_packet_process: wcid=0x%x\n",pRxBlk->wcid)); */
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
			if (RTMPEqualMemory(TPID, pRxBlk->pData + LENGTH_802_3_NO_TYPE, 2))
				type = _802_3_vlan_data;
			else if (RTMPEqualMemory(EAPOL, pRxBlk->pData + LENGTH_802_3_NO_TYPE, 2))
				type = _802_3_eapol;
			else
				type = _802_3_data;
#ifdef VLAN_SUPPORT
			if (pAd->tr_ctl.vlan2ethctrl == FALSE) {
			/*VLAN header transfer fail, more 2 bytes tag&untag*/
				if (pRxBlk->is_htf == 1)
					remove_vlan_hw_padding(pAd, pRxBlk);
			}
#endif
			if (type == _802_3_data) {
				dev_rx_802_3_data_frm(pAd, pRxBlk);
			}
#if defined(VLAN_SUPPORT) || defined(MAP_TS_TRAFFIC_SUPPORT)
			else {
					if (type == _802_3_vlan_data &&
						RTMPEqualMemory(EAPOL, pRxBlk->pData + LENGTH_802_3_NO_TYPE + LENGTH_802_1Q, 2)) {
						type = _802_3_vlan_eapol;
					}
					if (type == _802_3_vlan_data) {
						dev_rx_802_3_data_frm(pAd, pRxBlk);
					} else if (type == _802_3_vlan_eapol) {
						/* It is VLAN EAPOL Packet*/
						remove_vlan_tag(pAd, pRxBlk->pRxPacket);
						pRxBlk->pData = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
						pRxBlk->DataSize -= LENGTH_802_1Q;
						update_rxblk_addr(pRxBlk);

						rebuild_802_11_eapol_frm(pAd, pRxBlk);
						dev_rx_802_11_data_frm(pAd, pRxBlk);
					} else if (type == _802_3_eapol) {
						/* It is a HW-VLAN-untagged EAPOL Packet*/
						rebuild_802_11_eapol_frm(pAd, pRxBlk);
						dev_rx_802_11_data_frm(pAd, pRxBlk);
					} else {
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"%s[%d]impossible!!! pkt has no type!!!",
							__func__, __LINE__);
					}
			}
#endif /*VLAN_SUPPORT*/
		} else {
#ifdef VLAN_SUPPORT
			/*Fix Connection issue for EAPOL Vlan under EAPOL NOT RX_HDR_TRANS*/
			/*6 is 80211 LLC/SNAP Length*/
			eapol_80211_frame_offset = LENGTH_802_11 + LENGTH_WMMQOS_H + 6;
			if (RTMPEqualMemory(TPID, pRxBlk->pData + eapol_80211_frame_offset, 2)) {
				if (RTMPEqualMemory(EAPOL, pRxBlk->pData + eapol_80211_frame_offset + LENGTH_802_1Q, 2)) {
					remove_80211_frame_vlan_tag(pAd, pRxBlk->pRxPacket, eapol_80211_frame_offset);
					pRxBlk->pData = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
					pRxBlk->DataSize -= LENGTH_802_1Q;
					pRxBlk->FC = pRxBlk->pData;
					update_rxblk_addr(pRxBlk);
				}
			}
#endif
			dev_rx_802_11_data_frm(pAd, pRxBlk);
		}
		break;

	case FC_TYPE_MGMT:
		dev_rx_mgmt_frm(pAd, pRxBlk);
		break;

	case FC_TYPE_CNTL:
		dev_rx_ctrl_frm(pAd, pRxBlk);
		break;

	default:
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		break;
	}

	return NDIS_STATUS_SUCCESS;
}

/*
	STEP 1. Decide number of fragments required to deliver this MSDU.
		The estimation here is not very accurate because difficult to
		take encryption overhead into consideration here. The result
		"NumberOfFrag" is then just used to pre-check if enough free
		TXD are available to hold this MSDU.

		The calculated "NumberOfFrag" is a rough estimation because of various
		encryption/encapsulation overhead not taken into consideration. This number is just
		used to make sure enough free TXD are available before fragmentation takes place.
		In case the actual required number of fragments of an NDIS packet
		excceeds "NumberOfFrag"caculated here and not enough free TXD available, the
		last fragment (i.e. last MPDU) will be dropped in RTMPHardTransmit() due to out of
		resource, and the NDIS packet will be indicated NDIS_STATUS_FAILURE. This should
		rarely happen and the penalty is just like a TX RETRY fail. Affordable.

		exception:
			a). fragmentation not allowed on multicast & broadcast
			b). Aggregation overwhelms fragmentation (fCLIENT_STATUS_AGGREGATION_CAPABLE)
			c). TSO/CSO not do fragmentation
*/

/* TODO: shiang-usw. we need to modify the TxPktClassificatio to adjust the NumberOfFrag! */
UCHAR get_frag_num(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	PACKET_INFO pkt_info;
	UCHAR *src_buf_va;
	UINT32 src_buf_len, frag_sz, pkt_len;
	UCHAR frag_num;

	RTMP_QueryPacketInfo(pPacket, &pkt_info, &src_buf_va, &src_buf_len);
	pkt_len = pkt_info.TotalPacketLength - LENGTH_802_3 + LENGTH_802_1_H;
	frag_sz = wlan_operate_get_frag_thld(wdev);
	if ((frag_sz < MIN_FRAG_THRESHOLD) || (frag_sz > MAX_FRAG_THRESHOLD))
		frag_sz = MAX_FRAG_THRESHOLD;
	frag_sz = frag_sz - LENGTH_802_11 - LENGTH_CRC;

	if (*src_buf_va & 0x01) {
		/* fragmentation not allowed on multicast & broadcast */
		frag_num = 1;
	} else {
		frag_num = (pkt_len / frag_sz) + 1;

		/* To get accurate number of fragmentation, Minus 1 if the size just match to allowable fragment size */
		if ((pkt_len % frag_sz) == 0)
			frag_num--;
	}
#ifdef AUTOMATION
	if (pAd->fpga_ctl.txrx_dbg_type == 3) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s[%d]pkt_len=%d,frag_sz=%d,frag_num=%d\n\r",
				__func__, __LINE__, pkt_len, frag_sz, frag_num);
	}
#endif /* AUTOMATION */
	return frag_num;
}

INT32 fp_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	UCHAR q_idx = QID_AC_BE;
	UINT16 wcid = RTMP_GET_PACKET_WCID(pkt);
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = &tr_ctl->tr_entry[wcid];
	struct qm_ops *ops = pAd->qm_ops;
	UCHAR user_prio = RTMP_GET_PACKET_UP(pkt);

	if (wcid != 0 && tr_entry->EntryType == ENTRY_CAT_MCAST) {

#ifdef IGMP_SNOOPING_NON_OFFLOAD
		/* For Panther's IGMP snooping implemented in driver. */
		if (wdev->IgmpSnoopEnable) {
			NDIS_STATUS ret_val = igmp_snoop_non_offload(pAd, wdev, pkt);

			if (ret_val != NDIS_STATUS_MORE_PROCESSING_REQUIRED)
				return ret_val;
		}
#endif /* IGMP_SNOOP_NON_OFFLOAD */

		RTMP_SET_PACKET_TXTYPE(pkt, TX_MCAST_FRAME);
	}
	MEM_DBG_PKT_RECORD(pkt, 1<<2);

	q_idx = RTMP_GET_PACKET_QUEIDX(pkt);
	if (RTMP_GET_PACKET_EAPOL(pkt) || RTMP_GET_PACKET_DHCP(pkt))
		ops->enq_mgmtq_pkt(pAd, wdev, pkt);
	else {
#ifdef SW_CONNECT_SUPPORT
		tr_entry->tx_send_data_cnt++;
#endif /* SW_CONNECT_SUPPORT */
		ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
	}

	ba_ori_session_start(pAd, tr_entry, user_prio);

	return NDIS_STATUS_SUCCESS;
}

INT wdev_tx_pkts(NDIS_HANDLE dev_hnd, PPNDIS_PACKET pkt_list, UINT pkt_cnt, struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)dev_hnd;
	PNDIS_PACKET pPacket;
	UINT16 wcid = WCID_INVALID;
	UINT Index;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#ifdef REDUCE_TCP_ACK_SUPPORT
	PMAC_TABLE_ENTRY pEntry = NULL;
	PUCHAR pDA = NULL;
#endif
#ifdef CONFIG_MAP_3ADDR_SUPPORT
	PMAC_TABLE_ENTRY ptEntry = NULL;
	PUCHAR pSA = NULL;
	struct wifi_dev *twdev = NULL;
	UCHAR idx;
	BOOLEAN add_entry = FALSE;
#endif

	for (Index = 0; Index < pkt_cnt; Index++) {
		pPacket = pkt_list[Index];

		MEM_DBG_PKT_RECORD(pPacket, 1<<0);
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)
			|| !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY)
			|| IsHcRadioCurStatOffByWdev(wdev)) {
			tr_cnt->sys_not_ready_drop++;
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
			tr_cnt->err_recovery_drop++;
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			continue;
		}
#endif /* ERR_RECOVERY */

#ifdef PKTLOSS_CHK
		if (pAd->pktloss_chk.enable)
			pAd->pktloss_chk.pktloss_chk_handler(pAd, GET_OS_PKT_DATAPTR(pPacket), MAT_ETHER_HDR_LEN, 0, FALSE);
#endif
		if (wdev->forbid_data_tx & (0x1 << MSDU_FORBID_CONNECTION_NOT_READY)) {
			tr_cnt->tx_forbid_drop++;
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		RTMP_SET_PACKET_WDEV(pPacket, wdev->wdev_idx);
#ifdef ANDLINK_FEATURE_SUPPORT
#ifdef ANDLINK_HOSTNAME_IP
        if (HcGetBandByWdev(wdev) < DBDC_BAND_NUM &&
			TRUE == pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)] &&
			TRUE == pAd->CommonCfg.andlink_ip_hostname_en[HcGetBandByWdev(wdev)] &&
			NDIS_STATUS_SUCCESS == update_sta_ip(pAd, pPacket)){
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(%d): get sta ip success\n", __FUNCTION__, __LINE__);
        }

        if (HcGetBandByWdev(wdev) < DBDC_BAND_NUM &&
			TRUE == pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)] &&
			TRUE == pAd->CommonCfg.andlink_ip_hostname_en[HcGetBandByWdev(wdev)] &&
			NDIS_STATUS_SUCCESS == update_sta_hostname(pAd, pPacket)){
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(%d): get sta hostname success\n", __FUNCTION__, __LINE__);
        }
#endif/*ANDLINK_HOSTNAME_IP*/
#endif /*ANDLINK_FEATURE_SUPPORT*/
		/*
			WIFI HNAT need to learn packets going to which interface from skb cb setting.
			@20150325
		*/
#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)

		if (ra_sw_nat_hook_tx != NULL) {
#ifdef TCSUPPORT_MT7510_FE

			if (ra_sw_nat_hook_tx(pPacket, NULL, FOE_MAGIC_WLAN) == 0)
#else
			if (ra_sw_nat_hook_tx(pPacket, 1) == 0)
#endif
			{
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				continue;
			}
		}

#endif
#ifdef CONFIG_FAST_NAT_SUPPORT
		if ((ra_sw_nat_hook_tx != NULL)
#ifdef WHNAT_SUPPORT
			/*if not support pure hw nat, then goto sw nat fast path*/
			&& (!pAd->CommonCfg.whnat_en)
#endif /*WHNAT_SUPPORT*/
		) {
			ra_sw_nat_hook_tx(pPacket, 0);
		}


#endif /*CONFIG_FAST_NAT_SUPPORT*/
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
		if (wdev->wdev_type == WDEV_TYPE_STA)
			RTMPWakeUpWdev(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT || APCLI_SUPPORT */
		RTMP_SET_PACKET_WCID(pPacket, wcid);

#ifdef REDUCE_TCP_ACK_SUPPORT
		pDA = GET_OS_PKT_DATAPTR(pPacket);
		pEntry = MacTableLookup(pAd, pDA);
		if (pEntry)
			RTMP_SET_PACKET_WCID(pPacket, pEntry->wcid);

		if (!ReduceTcpAck(pAd, pPacket))
#endif /* REDUCE_TCP_ACK_SUPPORT */
		{
#ifdef HOSTAPD_MAP_SUPPORT
			/* Send Eapol packets Recieved from Hostapd */
			if (RTMP_CFG80211_HOSTAPD_ON(pAd)) {
				UCHAR *pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
				UINT16 TypeLen = 0;

				if (pSrcBuf) {
					TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
					if (TypeLen == ETH_TYPE_EAPOL) {
						UINT16 Len = GET_OS_PKT_LEN(pPacket);
						RTMP_SET_PACKET_EAPOL(pPacket, 1);
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL,
						DBG_LVL_INFO,
						"%s, send EAPOL from hostapd length %d\n", __func__, Len);
					}
				}

			}

#endif
#ifndef A4_CONN
#ifdef RT_CFG80211_SUPPORT
			if (RTMP_CFG80211_HOSTAPD_ON(pAd)) {
			UCHAR *pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
			UINT16 TypeLen = 0;
				if (pSrcBuf) {
					TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
					if (TypeLen == ETH_TYPE_EAPOL) {
						UINT16 Len = GET_OS_PKT_LEN(pPacket);
						RTMP_SET_PACKET_EAPOL(pPacket, 1);
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL,
						DBG_LVL_INFO,
						"%s, send EAPOL from hostapd length %d\n", __func__, Len);
					}
				}

			}
#endif /* RT_CFG80211_SUPPORT */
#else
			UCHAR *pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				if (MAC_ADDR_IS_GROUP(pSrcBufVA))
					a4_send_clone_pkt(pAd, wdev->func_idx, pPacket, NULL);
			}
#ifdef CONFIG_MAP_3ADDR_SUPPORT
			if (pAd->MapAccept3Addr) {
				if (MAC_ADDR_IS_GROUP(pSrcBufVA)) {
					PSTA_ADMIN_CONFIG apcli_entry;
					apcli_entry = &pAd->StaCfg[wdev->func_idx];
					if (apcli_entry->eth_list_init) {
						if (wdev->wdev_type == WDEV_TYPE_STA) {
							pSA = pSrcBufVA + MAC_ADDR_LEN;
							ptEntry = MacTableLookup(pAd, pSA);
							if (!ptEntry)
								add_entry = TRUE;

							for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
								twdev = &pAd->ApCfg.MBSSID[idx].wdev;
								if (RoutingTabLookup(pAd, twdev->func_idx, pSA, FALSE, &wcid) == NULL)
									add_entry = TRUE;
							}

							if (add_entry)
								eth_add_entry(pAd, wdev->func_idx, pSA);
						}
					}
				}
			}
#endif
#endif /* A4_CONN */
			send_data_pkt(pAd, wdev, pPacket);
		}
	}

	return 0;
}

#ifdef TX_AGG_ADJUST_WKR
BOOLEAN tx_check_for_agg_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN check_result = FALSE;
	BOOLEAN support_four_stream = FALSE;

	if (pAd->TxAggAdjsut == FALSE)
		return FALSE;

	if (!pEntry)
		return FALSE;

	if (pEntry->vendor_ie.is_rlt == TRUE ||
		pEntry->vendor_ie.is_mtk == TRUE)
		return FALSE;

	if ((pEntry->SupportHTMCS & 0xff000000) != 0)
		support_four_stream = TRUE;

	if (pEntry->SupportVHTMCS4SS != 0)
		support_four_stream = TRUE;

	if (support_four_stream == FALSE)
		check_result = TRUE;

	return check_result;
}
#endif /* TX_AGG_ADJUST_WKR */

VOID tx_bytes_calculate(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
		(tx_blk->amsdu_state == TX_AMSDU_ID_FIRST))
		tx_blk->tx_bytes_len = sizeof(TMAC_TXD_L);

	tx_blk->tx_bytes_len += tx_blk->MpduHeaderLen +
					tx_blk->HdrPadLen +
					tx_blk->SrcBufLen;

	if (tx_blk->amsdu_state != TX_AMSDU_ID_NO) {

		if (((RTMP_GET_PACKET_PROTOCOL(tx_blk->pPacket) <= 1500) ? 0 : 1))
			tx_blk->tx_bytes_len += 8;

		/* TODO: need to profile to decide if remove vlan or not  */
		if (RTMP_GET_PACKET_VLAN(tx_blk->pPacket)) {
			if (!(tx_blk->wdev->bVLAN_Tag))/*is_rmvl */
				tx_blk->tx_bytes_len -= 4;
			else
				tx_blk->tx_bytes_len += 6;
		}

		if (tx_blk->amsdu_state != TX_AMSDU_ID_LAST) {
			if (tx_blk->tx_bytes_len & 3)
				tx_blk->tx_bytes_len += (4 - (tx_blk->tx_bytes_len & 3));
		}
	}
}

BOOLEAN is_udp_packet(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UINT16 TypeLen;
	UCHAR Byte0, Byte1, *pSrcBuf;

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	if (!pSrcBuf) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s: pSrcBuf is null!\n", __func__);
		return FALSE;
	}
	/* get Ethernet protocol field and skip the Ethernet Header */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

	pSrcBuf += LENGTH_802_3;

	if (TypeLen <= 1500) {
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03) {
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += LENGTH_802_1_H;
		} else {
			return FALSE;
		}
	}

	if (TypeLen == ETH_TYPE_IPv4) {
		UINT32 pktLen = GET_OS_PKT_LEN(pkt);

		ASSERT((pktLen > (ETH_HDR_LEN + IP_HDR_LEN)));

		if (*(pSrcBuf + 9) == IP_PROTO_UDP)
			return TRUE;
		else
			return FALSE;
	} else  {
		return FALSE;
	}
}
#ifdef TXRX_STAT_SUPPORT
BOOLEAN RTMPGetUserPriority(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN struct wifi_dev *wdev,
	OUT UCHAR *pUserPriority,
	OUT UCHAR *pQueIdx)
{
		UINT16 TypeLen;
		UCHAR Byte0, Byte1, *pSrcBuf, up = 0;
		MAC_TABLE_ENTRY *pMacEntry = NULL;

		*pUserPriority = 0;
		*pQueIdx = 0;

		pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
		ASSERT(pSrcBuf);

		pMacEntry = MacTableLookup(pAd, pSrcBuf);

		/* get Ethernet protocol field and skip the Ethernet Header */
		TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
		pSrcBuf += LENGTH_802_3;

		if (TypeLen <= 1500) {
			/*
				802.3, 802.3 LLC:
					DestMAC(6) + SrcMAC(6) + Lenght(2) +
					DSAP(1) + SSAP(1) + Control(1) +
				if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
					=> + SNAP (5, OriginationID(3) + etherType(2))
				else
					=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
			*/
			if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03) {
				Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);

				TypeLen = (USHORT)((Byte0 << 8) + Byte1);
				pSrcBuf += LENGTH_802_1_H; /* Skip this LLC/SNAP header*/
			} else
				return FALSE;
		}


		switch (TypeLen) {
		case ETH_TYPE_VLAN: {
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
#ifdef RTMP_UDMA_SUPPORT
			/*patch for vlan_udma TOS*/

			if (pAd->CommonCfg.bUdmaFlag  == TRUE) {
				if (TypeLen == ETH_TYPE_IPv4) {
					/*For IPV4 packet*/
					up = (*(pSrcBuf + 5) & 0xe0) >> 5;
				} else if (TypeLen == ETH_TYPE_IPv6) {
					/*For IPV6 Packet*/
					up = ((*(pSrcBuf + 4)) & 0x0e) >> 1;
				}
			} else
#endif	/* RTMP_UDMA_SUPPORT */
			{
				/* only use VLAN tag as UserPriority setting */
				up = (*pSrcBuf & 0xe0) >> 5;
			}
			pSrcBuf += LENGTH_802_1Q; /* Skip the VLAN Header.*/
		}
		break;

		case ETH_TYPE_IPv4: {
			/* If it's a VLAN packet, get the real Type/Length field.*/
			/*
			0   4	   8		  14  15		  31(Bits)
			+---+----+-----+----+---------------+
			|Ver |  IHL |DSCP |ECN |    TotalLen 		  |
			+---+----+-----+----+---------------+
			Ver	  - 4bit Internet Protocol version number.
			IHL	  - 4bit Internet Header Length
			DSCP - 6bit Differentiated Services Code Point(TOS)
			ECN	 - 2bit Explicit Congestion Notification
			TotalLen - 16bit IP entire packet length(include IP hdr)
			*/
			up = (*(pSrcBuf + 1) & 0xe0) >> 5;

		}
		break;

		case ETH_TYPE_IPv6: {
			/*
			0	4	8	 12 	16		31(Bits)
			+---+----+----+----+---------------+
			|Ver | TrafficClas |  Flow Label		   |
			+---+----+----+--------------------+
			Ver   - 4bit Internet Protocol version number.
			TrafficClas - 8bit traffic class field, the 6 most-significant bits used for DSCP
			*/
			up = ((*pSrcBuf) & 0x0e) >> 1;

			/* return AC_BE if packet is not IPv6 */
			if ((*pSrcBuf & 0xf0) != 0x60)
				up = 0;
		}
		break;

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		case ETHER_TYPE_TDLS_MMPDU: {
			up = 5;
		}
		break;
#endif /* DOT11Z_TDLS_SUPPORT */

		default:
			break;
		}

		if (up > 7)
			return FALSE;

		if ((pAd->CommonCfg.APEdcaParm[0].bACM[WMM_UP2AC_MAP[up]])
			|| (((wdev->wdev_type == WDEV_TYPE_APCLI)
				|| (wdev->wdev_type == WDEV_TYPE_REPEATER)
				|| (wdev->wdev_type == WDEV_TYPE_STA))
				&& (pMacEntry && pMacEntry->bACMBit[WMM_UP2AC_MAP[up]])))

			up = 0;

		/*
			Set WMM when
			1. wdev->bWmmCapable == TRUE
			2. Receiver's capability
				a). bc/mc packets
					->Need to get UP for IGMP use
				b). unicast packets
					-> CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)
			3. has VLAN tag or DSCP fields in IPv4/IPv6 hdr
		*/
		if ((wdev->bWmmCapable == TRUE) && (up <= 7)) {
			*pUserPriority = up;
			*pQueIdx = WMM_UP2AC_MAP[up];
		}

		return TRUE;
}

/* Get the status of the current Streaming stataus ( BE, BK, VI, VO) which is getting transmitted throught the AP */
#endif

DECLARE_TIMER_FUNCTION(rx_delay_ctl_algo);

VOID rx_delay_ctl_algo(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
}

BUILD_TIMER_FUNCTION(rx_delay_ctl_algo);

#ifdef CONFIG_TP_DBG
DECLARE_TIMER_FUNCTION(tp_dbg_history);

VOID tp_dbg_history(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct tp_debug *tp_dbg = (struct tp_debug *)FunctionContext;

	tp_dbg->IsrTxCntRec[tp_dbg->time_slot] = tp_dbg->IsrTxCnt;
	tp_dbg->IsrTxCnt = 0;
	tp_dbg->IsrRxCntRec[tp_dbg->time_slot] = tp_dbg->IsrRxCnt;
	tp_dbg->IsrRxCnt = 0;
	tp_dbg->IsrRx1CntRec[tp_dbg->time_slot] = tp_dbg->IsrRx1Cnt;
	tp_dbg->IsrRx1Cnt = 0;
	tp_dbg->IsrRxDlyCntRec[tp_dbg->time_slot] = tp_dbg->IsrRxDlyCnt;
	tp_dbg->IsrRxDlyCnt = 0;
	tp_dbg->IoReadTxRec[tp_dbg->time_slot] = tp_dbg->IoReadTx;
	tp_dbg->IoReadTx = 0;
	tp_dbg->IoWriteTxRec[tp_dbg->time_slot] = tp_dbg->IoWriteTx;
	tp_dbg->IoWriteTx = 0;
	tp_dbg->IoReadRxRec[tp_dbg->time_slot] = tp_dbg->IoReadRx;
	tp_dbg->IoReadRx = 0;
	tp_dbg->IoWriteRxRec[tp_dbg->time_slot] = tp_dbg->IoWriteRx;
	tp_dbg->IoWriteRx = 0;
	tp_dbg->IoReadRx1Rec[tp_dbg->time_slot] = tp_dbg->IoReadRx1;
	tp_dbg->IoReadRx1 = 0;
	tp_dbg->IoWriteRx1Rec[tp_dbg->time_slot] = tp_dbg->IoWriteRx1;
	tp_dbg->IoWriteRx1 = 0;
	tp_dbg->MaxProcessCntRxRecA[tp_dbg->time_slot] = tp_dbg->RxMaxProcessCntA;
	tp_dbg->RxMaxProcessCntA = 0;
	tp_dbg->MaxProcessCntRxRecB[tp_dbg->time_slot] = tp_dbg->RxMaxProcessCntB;
	tp_dbg->RxMaxProcessCntB = 0;
	tp_dbg->MaxProcessCntRxRecC[tp_dbg->time_slot] = tp_dbg->RxMaxProcessCntC;
	tp_dbg->RxMaxProcessCntC = 0;
	tp_dbg->MaxProcessCntRxRecD[tp_dbg->time_slot] = tp_dbg->RxMaxProcessCntD;
	tp_dbg->RxMaxProcessCntD = 0;
	tp_dbg->MaxProcessCntRx1RecA[tp_dbg->time_slot] = tp_dbg->Rx1MaxProcessCntA;
	tp_dbg->Rx1MaxProcessCntA = 0;
	tp_dbg->MaxProcessCntRx1RecB[tp_dbg->time_slot] = tp_dbg->Rx1MaxProcessCntB;
	tp_dbg->Rx1MaxProcessCntB = 0;
	tp_dbg->MaxProcessCntRx1RecC[tp_dbg->time_slot] = tp_dbg->Rx1MaxProcessCntC;
	tp_dbg->Rx1MaxProcessCntC = 0;
	tp_dbg->MaxProcessCntRx1RecD[tp_dbg->time_slot] = tp_dbg->Rx1MaxProcessCntD;
	tp_dbg->Rx1MaxProcessCntD = 0;
	tp_dbg->TRDoneTimesRec[tp_dbg->time_slot] = tp_dbg->TRDoneTimes;
	tp_dbg->TRDoneTimes = 0;
	tp_dbg->time_slot++;
	tp_dbg->time_slot = tp_dbg->time_slot % TP_DBG_TIME_SLOT_NUMS;
}
BUILD_TIMER_FUNCTION(tp_dbg_history);
#endif /* CONFIG_TP_DBG */

INT32 tr_ctl_init(RTMP_ADAPTER *pAd)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_flow_control *tr_flow_ctl = NULL;
	UINT8 *pTxFlowBlockState = NULL;
	NDIS_SPIN_LOCK *pTxBlockLock = NULL;
	DL_LIST *pTxBlockDevList = NULL;
	UINT8 num_of_tx_ring = hif_get_tx_res_num(pAd->hdev_ctrl);
	UINT32 Index;
	ULONG Flags;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = NULL;
#endif

	tr_flow_ctl = &tr_ctl->tr_flow_ctl;

#ifdef CONFIG_TP_DBG
	tp_dbg = &tr_ctl->tp_dbg;
#endif

	ba_ctl_init(pAd, &pAd->tr_ctl.ba_ctl);

	/*  Allocate BA Reordering memory */
	if (ba_reordering_resource_init(pAd, MAX_REORDERING_MPDU_NUM) != TRUE)
		goto error0;

	if (IS_ASIC_CAP(pAd, fASIC_CAP_HW_TX_AMSDU)) {
		tr_ctl->amsdu_type = TX_HW_AMSDU;
	} else {
		tr_ctl->amsdu_type = TX_SW_AMSDU;
	}

	if (IS_ASIC_CAP(pAd, fASIC_CAP_HW_DAMSDU)) {
		tr_ctl->damsdu_type = RX_HW_AMSDU;
	} else {
		tr_ctl->damsdu_type = RX_SW_AMSDU;
	}

	os_alloc_mem(pAd, (UCHAR **)&pTxFlowBlockState, (num_of_tx_ring) * sizeof(UINT8));

	if (!pTxFlowBlockState) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s os_alloc_mem fail\n", __func__);
		goto error0;
	}

	NdisZeroMemory(pTxFlowBlockState, (num_of_tx_ring) * sizeof(UINT8));
	tr_flow_ctl->TxFlowBlockState = pTxFlowBlockState;
	os_alloc_mem(pAd, (UCHAR **)&pTxBlockLock, (num_of_tx_ring) * sizeof(NDIS_SPIN_LOCK));

	if (!pTxBlockLock) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s os_alloc_mem fail\n", __func__);
		goto error1;
	}

	NdisZeroMemory(pTxBlockLock, (num_of_tx_ring) * sizeof(NDIS_SPIN_LOCK));
	tr_flow_ctl->TxBlockLock = pTxBlockLock;
	os_alloc_mem(pAd, (UCHAR **)&pTxBlockDevList, (num_of_tx_ring) * sizeof(DL_LIST));

	if (!pTxBlockDevList) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                         "%s os_alloc_mem fail\n", __func__);
		goto error2;
	}

	NdisZeroMemory(pTxBlockDevList, (num_of_tx_ring) * sizeof(DL_LIST));
	tr_flow_ctl->TxBlockDevList = pTxBlockDevList;

	tr_flow_ctl->RxFlowBlockState = 0;

	for (Index = 0; Index < num_of_tx_ring; Index++) {
		tr_flow_ctl->TxFlowBlockState[Index] = 0;
		NdisAllocateSpinLock(pAd, &tr_flow_ctl->TxBlockLock[Index]);
		RTMP_SPIN_LOCK_IRQSAVE(&tr_flow_ctl->TxBlockLock[Index], &Flags);
		DlListInit(&tr_flow_ctl->TxBlockDevList[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&tr_flow_ctl->TxBlockLock[Index], &Flags);
	}

	tr_flow_ctl->IsTxBlocked = FALSE;

#ifdef CONFIG_TP_DBG
	NdisZeroMemory(tp_dbg, sizeof(struct tp_debug));
	tp_dbg->debug_flag = (TP_DEBUG_ISR);
	RTMPInitTimer(pAd, &tp_dbg->tp_dbg_history_timer, GET_TIMER_FUNCTION(tp_dbg_history), tp_dbg, TRUE);
	RTMPSetTimer(&tp_dbg->tp_dbg_history_timer, 1000);
#endif

	NdisZeroMemory(tr_cnt, sizeof(*tr_cnt));

	return NDIS_STATUS_SUCCESS;

error2:
	os_free_mem(pTxBlockLock);
error1:
	os_free_mem(pTxFlowBlockState);
error0:
	ba_reordering_resource_release(pAd);

	return NDIS_STATUS_FAILURE;
}

INT32 tr_ctl_exit(RTMP_ADAPTER *pAd)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_flow_control *tr_flow_ctl = &tr_ctl->tr_flow_ctl;
	TX_BLOCK_DEV *TxBlockDev = NULL;
	UINT32 Index;
#ifdef CONFIG_TP_DBG
	BOOLEAN cancelled;
	struct tp_debug *tp_dbg = &tr_ctl->tp_dbg;
#endif
	UINT8 tx_res_num = hif_get_tx_res_num(pAd->hdev_ctrl);

	ba_reordering_resource_release(pAd);
	ba_ctl_exit(&tr_ctl->ba_ctl);

	for (Index = 0; Index < tx_res_num; Index++) {
		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[Index]);

		while (1) {
			TxBlockDev = DlListFirst(&tr_flow_ctl->TxBlockDevList[Index], TX_BLOCK_DEV, list);

			if (!TxBlockDev)
				break;

			DlListDel(&TxBlockDev->list);
			RTMP_OS_NETDEV_WAKE_QUEUE(TxBlockDev->NetDev);
			os_free_mem(TxBlockDev);
		}

		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Index]);
		NdisFreeSpinLock(&tr_flow_ctl->TxBlockLock[Index]);
	}

	os_free_mem(tr_flow_ctl->TxFlowBlockState);
	os_free_mem(tr_flow_ctl->TxBlockLock);
	os_free_mem(tr_flow_ctl->TxBlockDevList);

#ifdef CONFIG_TP_DBG
	RTMPReleaseTimer(&tp_dbg->tp_dbg_history_timer, &cancelled);
#endif

	return 0;
}

INT32 tx_flow_block(RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx)
{
	INT32 Ret = 0;
	struct tr_flow_control *tr_flow_ctl = &pAd->tr_ctl.tr_flow_ctl;
	UINT8 num_of_tx_ring = hif_get_tx_res_num(pAd->hdev_ctrl);

	if (Block == TRUE) {
		TX_BLOCK_DEV *TxBlockDev = NULL;

		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		tr_flow_ctl->TxFlowBlockState[RingIdx] |= State;
		DlListForEach(TxBlockDev, &tr_flow_ctl->TxBlockDevList[RingIdx], TX_BLOCK_DEV, list) {
			if (TxBlockDev->NetDev == NetDev) {
				RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
				return Ret;
			}
		}

		os_alloc_mem(NULL, (PUCHAR *)&TxBlockDev, sizeof(*TxBlockDev));

		if (!TxBlockDev) {
			MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
				 "can not allocate TX_BLOCK_DEV\n");
			RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
			return -1;
		}

		TxBlockDev->NetDev = NetDev;
		DlListAddTail(&tr_flow_ctl->TxBlockDevList[RingIdx], &TxBlockDev->list);
		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		RTMP_OS_NETDEV_STOP_QUEUE(NetDev);
	} else {
		TX_BLOCK_DEV *TxBlockDev = NULL;

		if (RingIdx == num_of_tx_ring) {
			UINT8 Idx = 0;

			for (Idx = 0; Idx < num_of_tx_ring; Idx++) {
				RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[Idx]);
				tr_flow_ctl->TxFlowBlockState[Idx] &= ~State;

				if (tr_flow_ctl->TxFlowBlockState[Idx] != 0) {
					RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Idx]);
					continue;
				}

				while (1) {
					TxBlockDev = DlListFirst(&tr_flow_ctl->TxBlockDevList[Idx], TX_BLOCK_DEV, list);

					if (!TxBlockDev)
						break;

					DlListDel(&TxBlockDev->list);
					RTMP_OS_NETDEV_WAKE_QUEUE(TxBlockDev->NetDev);
					os_free_mem(TxBlockDev);
				}

				RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Idx]);
			}
		} else {
			RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
			tr_flow_ctl->TxFlowBlockState[RingIdx] &= ~State;

			if (tr_flow_ctl->TxFlowBlockState[RingIdx] != 0) {
				RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
				return Ret;
			}

			while (1) {
				TxBlockDev = DlListFirst(&tr_flow_ctl->TxBlockDevList[RingIdx], TX_BLOCK_DEV, list);

				if (!TxBlockDev)
					break;

				DlListDel(&TxBlockDev->list);
				RTMP_OS_NETDEV_WAKE_QUEUE(TxBlockDev->NetDev);
				os_free_mem(TxBlockDev);
			}

			RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		}
	}

	return Ret;
}

static inline void tx_flow_block_all_netdev(RTMP_ADAPTER *pAd, BOOLEAN Block)
{
	INT32 wdev_idx = 0;
	struct wifi_dev *wdev = NULL;

	if (Block) {
		for (wdev_idx = 0; wdev_idx < WDEV_NUM_MAX; wdev_idx++) {
			wdev = pAd->wdev_list[wdev_idx];
			if (wdev && (wdev->if_dev))
				RTMP_OS_NETDEV_STOP_QUEUE(wdev->if_dev);
		}
	} else {
		for (wdev_idx = 0; wdev_idx < WDEV_NUM_MAX; wdev_idx++) {
			wdev = pAd->wdev_list[wdev_idx];
			if (wdev && (wdev->if_dev))
				RTMP_OS_NETDEV_WAKE_QUEUE(wdev->if_dev);
		}
	}
}

INT32 tx_flow_set_state_block(RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx)
{
	INT32 Ret = 0;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	struct tr_flow_control *tr_flow_ctl = &pAd->tr_ctl.tr_flow_ctl;

	if (Block == TRUE) {
		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		tr_flow_ctl->TxFlowBlockState[RingIdx] |= State;
		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		/* Stop device because this queue got congestion */
		/* stop all bss if no Netdev input */
		if (NetDev)
			RTMP_OS_NETDEV_STOP_QUEUE(NetDev);
		else
			tx_flow_block_all_netdev(pAd, true);

		tr_flow_ctl->IsTxBlocked = TRUE;
		tr_cnt->net_if_stop_cnt++;
	} else {
		UINT8 num_of_tx_ring = hif_get_tx_res_num(pAd->hdev_ctrl);

		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		tr_flow_ctl->TxFlowBlockState[RingIdx] &= ~State;
		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
		/* Wake up device if all queue are available */
		if (!tx_flow_check_state(pAd, State, num_of_tx_ring)) {
			/* Wake up all bss if no Netdev input */
			if (NetDev)
				RTMP_OS_NETDEV_WAKE_QUEUE(NetDev);
			else
				tx_flow_block_all_netdev(pAd, false);
		} /* !tx_flow_check_state */

		tr_flow_ctl->IsTxBlocked = FALSE;
	}

	return Ret;
}

inline BOOLEAN tx_flow_check_if_blocked(RTMP_ADAPTER *pAd)
{
	return pAd->tr_ctl.tr_flow_ctl.IsTxBlocked;
}

BOOLEAN tx_flow_check_state(RTMP_ADAPTER *pAd, UINT8 State, UINT8 RingIdx)
{
	UINT8 Idx;
	struct tr_flow_control *tr_flow_ctl = &pAd->tr_ctl.tr_flow_ctl;
	UINT8 num_of_tx_ring = hif_get_tx_res_num(pAd->hdev_ctrl);

	if (RingIdx == num_of_tx_ring) {
		for (Idx = 0; Idx < num_of_tx_ring; Idx++) {
			RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[Idx]);

			if ((tr_flow_ctl->TxFlowBlockState[Idx] & State) == State) {
				RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Idx]);
				return TRUE;
			}

			RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[Idx]);
		}
	} else {
		RTMP_SEM_LOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);

		if ((tr_flow_ctl->TxFlowBlockState[RingIdx] & State) == State) {
			RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
			return TRUE;
		}

		RTMP_SEM_UNLOCK(&tr_flow_ctl->TxBlockLock[RingIdx]);
	}

	return FALSE;
}


#if defined(MT_MAC) && defined(VHT_TXBF_SUPPORT)
VOID Mumimo_scenario_maintain(
	IN PRTMP_ADAPTER pAd)
{
#ifdef CFG_SUPPORT_MU_MIMO
	UINT16              u2WlanIdx;
	UCHAR               ucWdevice = 0;
	UCHAR               ucMUNum = 0;
	struct wifi_dev     *pWdev = NULL;
	PMAC_TABLE_ENTRY    pEntry;
	BOOL                fgHitMUTxOPCondition = FALSE;
	for (u2WlanIdx = 1; u2WlanIdx <= pAd->MacTab.Size; u2WlanIdx++) {

	pEntry = &pAd->MacTab.Content[u2WlanIdx];

	if (pEntry->rStaRecBf.fgSU_MU) {
		ucMUNum++;
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "at 1\n");
	}

	if (ucMUNum >= 2) {
		fgHitMUTxOPCondition = TRUE;
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "at 2\n");
		break;
		}
	}

	/* If Hit Trigger Condition, assign TxOP=0xC0 for 3/4 MU-MIMO Peak */
	if (fgHitMUTxOPCondition) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "value of pAd->MUMIMO_TxOP_Value = %d\n", pAd->MUMIMO_TxOP_Value);
		if (pAd->MUMIMO_TxOP_Value != TXOP_C0) {
		for (ucWdevice = 0; ucWdevice < WDEV_NUM_MAX; ucWdevice++) {
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "at 3\n");
			pWdev = pAd->wdev_list[ucWdevice];
			if ((!pWdev) || (!HcIsRadioAcq(pWdev)))
				continue;
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "at 4\n");
			pAd->MUMIMO_TxOP_Value = TXOP_C0;
			enable_tx_burst(pAd, pWdev, AC_BE, PRIO_MU_MIMO, TXOP_C0);
			}
		}
	} else {
		if (pAd->MUMIMO_TxOP_Value != TXOP_0) {
			for (ucWdevice = 0; ucWdevice < WDEV_NUM_MAX; ucWdevice++) {
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "at 5\n");
				pWdev = pAd->wdev_list[ucWdevice];
				if ((!pWdev) || (!HcIsRadioAcq(pWdev)))
					continue;
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "at 6\n");
				pAd->MUMIMO_TxOP_Value = TXOP_0;
				disable_tx_burst(pAd, pWdev, AC_BE, PRIO_MU_MIMO, TXOP_0);
			}
		}
	}
/*MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,*/
/*"MU TxOP = %d, MU STA = %d, MU CW Min %d\n", pAd->MUMIMO_TxOP_Value, ucMUNum, u4EdcaCWmin);*/
	return;
#else
	return;
#endif
}

#endif /* MT_MAC && VHT_TXBF_SUPPORT */
#ifdef PKTLOSS_CHK
static void pktloss_chk_hex_dump(PUINT_8 pData, UINT_32 dump_len)
{
	PUINT_8 ptr = pData;
	UINT_32 len = 0;
	UINT_8 buf[256] = {0};
	UINT32 buf_len = 256;
	INT_32 i;

	for (i = 0; i < dump_len; i++) {
		len += snprintf(buf + len, buf_len - strlen(buf), "%02X,", *ptr);
		ptr++;
		if ((i%16 == 15) || (len >= (dump_len - 1))) {
			len += snprintf(buf + len, buf_len - strlen(buf), "%s", "\n");
			MTWF_PRINT("%s", buf);
			len = 0;
		}
	}
}

void pktloss_chk_dump(PRTMP_ADAPTER pAd, BOOLEAN dump_all)
{
	INT_32 i, j, idx;
	pktloss_check_var_type *pktloss_chk = &pAd->pktloss_chk;

	for (i = 0; i < MAX_PKTLOSS_CHK_PT; i++) {
		if (pktloss_chk->tot_cnt[i] > 0) {
			MTWF_PRINT("PKTLOSS[%d]times:%u,cnt:%u[tot_tx:%u,drop_tx:%u,seq:%u]\n",
				i, pktloss_chk->loss[i],
				pktloss_chk->loss_cnt[i],
				pktloss_chk->tot_cnt[i],
				pktloss_chk->tot_drop_cnt[i],
				pktloss_chk->seq[i]);
		}
	}

	for (i = 0; i < MAX_PKTLOSS_CHK_PT; i++) {
		if (pktloss_chk->tot_cnt[i] > 0) {
			MTWF_PRINT("PKTLOSS[%d]:dlycnt:%u,maxdly:%u\n",
				i, pktloss_chk->ts_delay[i],
				pktloss_chk->max_ts_delay[i]);
		}
	}


	if (!dump_all)
		return;

	for (i = 0; i < MAX_PKTLOSS_CHK_PT; i++) {
		idx = pktloss_chk->loss_seq_idx[i];
		for (j = 0; j < idx; j++) {
			MTWF_PRINT("PKTLOSS[%d] lost seq[%u]:%u\n",
				i, j, pktloss_chk->loss_seq[i][j]);
		}
	}

	for (i = 0; i < MAX_PKTLOSS_CHK_PT; i++) {
		idx = pktloss_chk->dup_seq_idx[i];
		for (j = 0; j < idx; j++) {
			MTWF_PRINT("PKTLOSS[%d] dup seq[%u]:%u\n",
				i, j, pktloss_chk->dup_seq[i][j]);
		}
	}

	return;
}

BOOLEAN pktloss_chk_func(PRTMP_ADAPTER pAd,
			PUINT_8 aucPktContent,
			UINT_8 ip_offset,
			UINT_8 check_point,
			BOOLEAN drop)
{
	UINT_32 src_ip, dest_ip, ts;
	UINT_32 packet_seq = 0;
	UINT_8 temp[4] = {0};
	UINT_8 proto;
	UINT_16 port;
	UINT_8 idx, offset = 0;
	pktloss_check_var_type *pktloss_chk = &pAd->pktloss_chk;
	UINT_32 my_seq = pktloss_chk->seq[check_point];
	BOOLEAN ret = FALSE;

	src_ip = (aucPktContent[ip_offset + 12] << 24) | (aucPktContent[ip_offset + 13] << 16) |
		(aucPktContent[ip_offset + 14] << 8) | (aucPktContent[ip_offset + 15] << 0);
	dest_ip = (aucPktContent[ip_offset + 16] << 24) | (aucPktContent[ip_offset + 17] << 16) |
		(aucPktContent[ip_offset + 18] << 8) | (aucPktContent[ip_offset + 19] << 0);
	proto = aucPktContent[ip_offset + 9];
	port = (aucPktContent[ip_offset + 20 + 2] << 8) | aucPktContent[ip_offset + 20 + 3];

	if (pktloss_chk->ctrl_flag & (1 << PKTLOSS_CHK_HEX_DUMP | (1 << PKTLOSS_CHK_CONTINUE_HEX_DUMP))) {
		pktloss_chk->ctrl_flag &= ~(1 << PKTLOSS_CHK_HEX_DUMP);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"[%d]proto:0x%02X,port:0x%04X, src_ip:%08X,dest_ip:%08X\n",
			 check_point, proto, port, src_ip, dest_ip);
		pktloss_chk_hex_dump(aucPktContent, 96);
	}

	if ((port != pktloss_chk->dest_port) ||
		(proto != pktloss_chk->proto) ||
		((src_ip & pktloss_chk->src_ip_mask) != src_ip) ||
		((dest_ip & pktloss_chk->dest_ip_mask) != dest_ip)) {
		return FALSE;
	}

	if (drop) {
		pktloss_chk->tot_drop_cnt[check_point]++;
		return TRUE;
	} else
		pktloss_chk->tot_cnt[check_point]++;

	ret = TRUE;
	offset = ip_offset + pktloss_chk->byte_offset;
	temp[0]  = aucPktContent[offset + 20 + 8 + 0];
	temp[1]  = aucPktContent[offset + 20 + 8 + 1];
	temp[2]  = aucPktContent[offset + 20 + 8 + 2];
	temp[3]  = aucPktContent[offset + 20 + 8 + 3];

	packet_seq = ((temp[0]<<24) | (temp[1]<<16) | (temp[2]<<8) | (temp[3]<<0)) & pktloss_chk->seq_mask;

	if ((packet_seq <= (pktloss_chk->seq_mask >> 1)) || (pktloss_chk->is_seq_signed == 0)) {
		if (my_seq == 0)
			my_seq = packet_seq;

		if (my_seq > packet_seq) {
			/* warp around loss or duplicate */
			if ((my_seq - packet_seq) < (pktloss_chk->seq_mask >> (pktloss_chk->is_seq_signed + 1))) {
				/* duplicate case*/
				idx = pktloss_chk->dup_seq_idx[check_point];
				if (idx < MAX_PKTLOSS_CHK_LOG) {
					pktloss_chk->dup_seq[check_point][idx] = packet_seq;
					idx = idx + 1;
					pktloss_chk->dup_seq_idx[check_point] = idx;
				}
			} else {
				idx = pktloss_chk->loss_seq_idx[check_point];
				if (idx < MAX_PKTLOSS_CHK_LOG) {
					pktloss_chk->loss_seq[check_point][idx] = my_seq;
					idx = idx + 1;
					pktloss_chk->loss_seq_idx[check_point] = idx;
				}
				pktloss_chk->loss[check_point]++;
				pktloss_chk->loss_cnt[check_point] +=
					((pktloss_chk->seq_mask >> pktloss_chk->is_seq_signed) + 1) -
					(my_seq - packet_seq);
					my_seq = packet_seq;
			}
		}
		if (my_seq < packet_seq) {
			idx = pktloss_chk->loss_seq_idx[check_point];
			if (idx < MAX_PKTLOSS_CHK_LOG) {
				pktloss_chk->loss_seq[check_point][idx] = my_seq;
				idx = idx + 1;
				pktloss_chk->loss_seq_idx[check_point] = idx;
			}
			pktloss_chk->loss[check_point]++;
			pktloss_chk->loss_cnt[check_point] += (packet_seq - my_seq);
			my_seq = packet_seq;
		}

		if (pktloss_chk->ctrl_flag & (1 << PKTLOSS_CHK_BY_TS)) {
			/*
			temp[0]  = aucPktContent[offset + 20 + 8 + 0 + 4];
			temp[1]  = aucPktContent[offset + 20 + 8 + 1 + 4];
			temp[2]  = aucPktContent[offset + 20 + 8 + 2 + 4];
			temp[3]  = aucPktContent[offset + 20 + 8 + 3 + 4];

			ts = ((temp[0]<<24) | (temp[1]<<16) | (temp[2]<<8) | (temp[3]<<0));
			*/
			ts = jiffies;
			if ((my_seq == packet_seq) && (pktloss_chk->ts[check_point] > 0)) {
				if (((ts - pktloss_chk->ts[check_point]) > pktloss_chk->ts_threshold)) {
					pktloss_chk->ts_delay[check_point]++;
				}
				if ((ts - pktloss_chk->ts[check_point]) > pktloss_chk->max_ts_delay[check_point])
					pktloss_chk->max_ts_delay[check_point] = ts - pktloss_chk->ts[check_point];
			}
			pktloss_chk->ts[check_point] = ts;
		}

		my_seq = ((my_seq == (pktloss_chk->seq_mask >> pktloss_chk->is_seq_signed)) ?
			(pktloss_chk->is_seq_cross_zero ? 0 : 1) : (my_seq + 1));

		pktloss_chk->seq[check_point] = my_seq;
	}

	return ret;
}
#endif /* PKTLOSS_CHK */
#ifdef IXIA_C50_MODE
char *txType[TX_TYPE_MAX] = {
	"APD",
	"ASD",
	"LGY",
	"FRG",
};

/*this function is for ixia or c50 counter debug*/
void wifi_txrx_parmtrs_dump(RTMP_ADAPTER *pAd)
{
	UINT32 idx = 0;
	UINT32 cpu = 0;
	UINT16 i = 0;
	UINT32 tx_enq_total = 0;
	UINT32 tx_type_total = 0;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 rx_from_hw_total = 0;
	UINT32 rx_to_os_total = 0;

	UINT32 rx_dup_seq_total = 0;
	UINT32 rx_old_seq_total = 0;
	UINT32 rx_flush_seq_total = 0;
	UINT32 rx_surpass_seq_total = 0;

	UINT32 cr_val = 0;

	/********************************************** TX COUNTER********************************************/
	if (pAd->tx_cnt.tx_pkt_from_os >= 500) {
		pAd->ixia_ctl.tx_test_round++;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\n************ wifi TX counter(%d) ************\n",
		pAd->ixia_ctl.tx_test_round);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Online Stations: %d .\n", pAd->MacTab.Size);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"tx_pkt_len    : %d .\n", pAd->tx_cnt.tx_pkt_len);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"tx_pkt_from_os: %d .\n", pAd->tx_cnt.tx_pkt_from_os);

		/**********************************TX ENQ & DEQ ******************************************/
		for (cpu = 0; cpu < NR_CPUS; cpu++) {
			if (pAd->tx_cnt.tx_pkt_enq_cnt[cpu] != 0) {
				if (pAd->ixia_ctl.debug_lvl >= IXIA_ERROR)
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"*enq_cpu[%d]: %d .\n", cpu, pAd->tx_cnt.tx_pkt_enq_cnt[cpu]);
				tx_enq_total += pAd->tx_cnt.tx_pkt_enq_cnt[cpu];
			}
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"tx_pkt_enq_cnt: %d .\n", tx_enq_total);

		/**************************************************TX TO HW*******************************************************/
		for (idx = 0; idx < TX_TYPE_MAX; idx++) {
			for (cpu = 0; cpu < NR_CPUS; cpu++) {
				if (pAd->tx_cnt.tx_pkt_to_hw[idx][cpu] != 0) {
					tx_type_total += pAd->tx_cnt.tx_pkt_to_hw[idx][cpu];
					if (pAd->ixia_ctl.debug_lvl >= IXIA_ERROR)
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "*%s-to_hw_cpu[%d]: %d .\n",
						txType[idx], cpu, pAd->tx_cnt.tx_pkt_to_hw[idx][cpu]);
				}
			}

			if (tx_type_total != 0)
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s-tx_pkt_to_hw  : %d .\n",
				txType[idx], tx_type_total);
			tx_type_total = 0;
		}

		/**************************************************TX RETRY LIMIT COUNTER*******************************************************/
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DROPSR00_MPDU_RETRY_DROP_COUNT_ADDR, &cr_val);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"BAND0 MPDU_retry_limit_cnt = %d, rts_retry_limit_cnt = %d\n",
		(cr_val & BN0_WF_MIB_TOP_M0DROPSR00_MPDU_RETRY_DROP_COUNT_MASK) >> 16,
		(cr_val & BN0_WF_MIB_TOP_M0DROPSR00_RTS_DROP_COUNT_MASK));

		RTMP_IO_READ32(pAd->hdev_ctrl, BN1_WF_MIB_TOP_M0DROPSR00_MPDU_RETRY_DROP_COUNT_ADDR, &cr_val);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"BAND1 MPDU_retry_limit_cnt = %d, rts_retry_limit_cnt = %d\n",
		(cr_val & BN1_WF_MIB_TOP_M0DROPSR00_MPDU_RETRY_DROP_COUNT_MASK) >> 16,
		(cr_val & BN1_WF_MIB_TOP_M0DROPSR00_RTS_DROP_COUNT_MASK));
		/*************************************************TX COUNTER CLEAR**************************************************/
		NdisZeroMemory(&pAd->tx_cnt, sizeof(struct ixia_tx_cnt));
	}

	/********************************************** RX COUNTER********************************************/
	for (cpu = 0; cpu < NR_CPUS; cpu++)
		rx_from_hw_total += pAd->rx_cnt.rx_from_hw[cpu];

	if (rx_from_hw_total > 500) {
		pAd->ixia_ctl.rx_test_round++;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\n************ wifi RX counter(%d) ************\n",
		pAd->ixia_ctl.rx_test_round);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Online Stations: %d .\n", pAd->MacTab.Size);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"rx_pkt_len    : %d .\n", pAd->rx_cnt.rx_pkt_len);

		/********************************** RX FROM HW******************************************/
		if (pAd->ixia_ctl.debug_lvl >= IXIA_ERROR) {
			for (cpu = 0; cpu < NR_CPUS; cpu++) {
				if (pAd->rx_cnt.rx_from_hw[cpu] != 0) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"*from_hw_cpu[%d]: %d .\n", cpu, pAd->rx_cnt.rx_from_hw[cpu]);
				}
			}
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"rx_pkt_from_hw_total: %d .\n", rx_from_hw_total);

		/********************************** RX BA COUNTER******************************************/
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		">>>>>>>>>BA Related Counter start\n");

		for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) {
			pEntry = &pAd->MacTab.Content[i];

			if (!(IS_ENTRY_CLIENT(pEntry) &&
				(pEntry->Sst == SST_ASSOC)))
				continue;

			if (pAd->ixia_ctl.debug_lvl >= IXIA_ERROR) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"wcid(%d): dup(%d),old(%d),flush(%d),surpass(%d).\n",
				pEntry->wcid,
				pAd->rx_cnt.rx_dup_drop[pEntry->wcid],
				pAd->rx_cnt.rx_old_drop[pEntry->wcid],
				pAd->rx_cnt.rx_flush_drop[pEntry->wcid],
				pAd->rx_cnt.rx_surpass_drop[pEntry->wcid]);
			}

			rx_dup_seq_total += pAd->rx_cnt.rx_dup_drop[pEntry->wcid];
			rx_old_seq_total += pAd->rx_cnt.rx_old_drop[pEntry->wcid];
			rx_flush_seq_total += pAd->rx_cnt.rx_flush_drop[pEntry->wcid];
			rx_surpass_seq_total += pAd->rx_cnt.rx_surpass_drop[pEntry->wcid];
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"rx_dup_seq_total  : %d .\n", rx_dup_seq_total);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"rx_old_seq_total  : %d .\n", rx_old_seq_total);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"rx_flush_seq_total  : %d .\n", rx_flush_seq_total);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"rx_surpass_seq_total  : %d .\n", rx_surpass_seq_total);

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"BA Related Counter end<<<<<<<<<\n");
		/*************************************** RX TO OS ************************************************/
		for (cpu = 0; cpu < NR_CPUS; cpu++) {
			if (pAd->rx_cnt.rx_pkt_to_os[cpu] != 0) {
				rx_to_os_total += pAd->rx_cnt.rx_pkt_to_os[cpu];
				if (pAd->ixia_ctl.debug_lvl >= IXIA_ERROR)
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"*to_os_cpu[%d]: %d .\n", cpu, pAd->rx_cnt.rx_pkt_to_os[cpu]);
			}
		}
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"rx_pkt_to_os_total  : %d .\n", rx_to_os_total);
		/*************************************************RX COUNTER CLEAR**************************************************/
		NdisZeroMemory(&pAd->rx_cnt, sizeof(struct ixia_rx_cnt));
	}
}

#endif
