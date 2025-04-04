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
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
#endif

static VOID ap_tx_drop_update(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, TX_BLK *txblk)
{
	struct tr_counter *tr_cnt = &ad->tr_ctl.tr_cnt;

#ifdef STATS_COUNT_SUPPORT
	BSS_STRUCT *mbss = txblk->pMbss;
#ifdef TXRX_STAT_SUPPORT
	struct hdev_obj *hdev = (struct hdev_obj *)wdev->pHObj;
#endif

	if (mbss != NULL) {
#ifdef TXRX_STAT_SUPPORT
		INC_COUNTER64(mbss->stat_bss.TxPacketDroppedCount);
		INC_COUNTER64(hdev->rdev->pRadioCtrl->TxPacketDroppedCount);
#endif
		mbss->TxDropCount++;
	}
#ifdef APCLI_SUPPORT
	else {
		if (txblk->pApCliEntry != NULL)
			txblk->pApCliEntry->StaStatistic.TxDropCount++;
	}
#endif
#endif /* STATS_COUNT_SUPPORT */

	tr_cnt->fill_tx_blk_fail_drop++;

}

static VOID ap_tx_ok_update(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, TX_BLK *txblk)
{
#ifdef STATS_COUNT_SUPPORT
	MAC_TABLE_ENTRY *entry = txblk->pMacEntry;
#ifdef TXRX_STAT_SUPPORT
	struct hdev_obj *hdev = (struct hdev_obj *)(wdev->pHObj);
	UCHAR  pUserPriority, QIdx;
#endif

	if (!entry) {
#ifdef TXRX_STAT_SUPPORT
		BSS_STRUCT *txrx_mbss = NULL;
		if (txblk->wdev) {
			UCHAR band_idx = HcGetBandByWdev(txblk->wdev);
			struct hdev_ctrl *ctrl = (struct hdev_ctrl *)ad->hdev_ctrl;
			txrx_mbss = &ad->ApCfg.MBSSID[txblk->wdev->wdev_idx];
			if (IS_MULTICAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				INC_COUNTER64(txrx_mbss->stat_bss.TxMulticastDataPacket);
				INC_COUNTER64(ctrl->rdev[band_idx].pRadioCtrl->TxMulticastDataPacket);
			} else if (IS_BROADCAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				INC_COUNTER64(txrx_mbss->stat_bss.TxBroadcastDataPacket);
				INC_COUNTER64(ctrl->rdev[band_idx].pRadioCtrl->TxBroadcastDataPacket);
			}
		}
#endif /* TXRX_STAT_SUPPORT */
		return ;
	}

#ifdef WHNAT_SUPPORT
#ifdef MAP_R2
/*if WHNAT enable, query from CR4 and then update it, but before returning update uc and mc counts*/
	if (IS_MAP_ENABLE(ad) && IS_MAP_R2_ENABLE(ad)) {
		if ((ad->CommonCfg.whnat_en) && (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD))) {
			BSS_STRUCT *mbss = txblk->pMbss;
#ifdef TR181_SUPPORT
			UCHAR band_idx = DBDC_BAND0;

			if (txblk->wdev)
				band_idx = HcGetBandByWdev(txblk->wdev);
#endif
			if (mbss != NULL) {
				mbss->TransmittedByteCount += txblk->SrcBufLen;
				mbss->TxCount++;
				if (IS_MULTICAST_MAC_ADDR(txblk->pSrcBufHeader)) {
					mbss->mcPktsTx++;
					mbss->mcBytesTx += txblk->SrcBufLen;
#ifdef TR181_SUPPORT
					ad->WlanCounters[band_idx].mcPktsTx.QuadPart++;
					ad->WlanCounters[band_idx].mcBytesTx.QuadPart += txblk->SrcBufLen;
#endif
				} else if (IS_BROADCAST_MAC_ADDR(txblk->pSrcBufHeader)) {
					mbss->bcPktsTx++;
					mbss->bcBytesTx += txblk->SrcBufLen;
#ifdef TR181_SUPPORT
					ad->WlanCounters[band_idx].bcPktsTx.QuadPart++;
					ad->WlanCounters[band_idx].bcBytesTx.QuadPart += txblk->SrcBufLen;
#endif /*TR181_SUPPORT*/
				}
			}
			return;
		}
	} else {
		/*if WHNAT enable, query from CR4 and then update it*/
		if ((ad->CommonCfg.whnat_en) && (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD)))
			return;
	}
#else
	/*if WHNAT enable, query from CR4 and then update it*/
	if ((ad->CommonCfg.whnat_en) && (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD)))
		return;
#endif
#endif /*WHNAT_SUPPORT*/

	/* calculate Tx count and ByteCount per BSS */
	{
		BSS_STRUCT *mbss = txblk->pMbss;
#ifdef TXRX_STAT_SUPPORT
		BSS_STRUCT *txrx_mbss = NULL;
#endif
#ifdef TR181_SUPPORT
		UCHAR band_idx = DBDC_BAND0;

		if (txblk->wdev)
			band_idx = HcGetBandByWdev(txblk->wdev);
#endif

		if (mbss != NULL) {
			mbss->TransmittedByteCount += txblk->SrcBufLen;
			mbss->TxCount++;

			if (IS_MULTICAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				mbss->mcPktsTx++;
#ifdef TR181_SUPPORT
				ad->WlanCounters[band_idx].mcPktsTx.QuadPart++;
#endif
#ifdef MAP_R2
				if (IS_MAP_ENABLE(ad) && IS_MAP_R2_ENABLE(ad)) {
					mbss->mcBytesTx += txblk->SrcBufLen;
#ifdef TR181_SUPPORT
					ad->WlanCounters[band_idx].mcBytesTx.QuadPart += txblk->SrcBufLen;
#endif
				}
#endif
			} else if (IS_BROADCAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				mbss->bcPktsTx++;
#ifdef TR181_SUPPORT
				ad->WlanCounters[band_idx].bcPktsTx.QuadPart++;
#endif
#ifdef MAP_R2
				if (IS_MAP_ENABLE(ad) && IS_MAP_R2_ENABLE(ad)) {
					mbss->bcBytesTx += txblk->SrcBufLen;
#ifdef TR181_SUPPORT
					ad->WlanCounters[band_idx].bcBytesTx.QuadPart += txblk->SrcBufLen;
#endif
				}
#endif
			} else {
				mbss->ucPktsTx++;
#ifdef TR181_SUPPORT
				ad->WlanCounters[band_idx].ucPktsTx.QuadPart++;
#endif
#ifdef MAP_R2
				if (IS_MAP_ENABLE(ad) && IS_MAP_R2_ENABLE(ad)) {
					mbss->ucBytesTx += txblk->SrcBufLen;
#ifdef TR181_SUPPORT
					ad->WlanCounters[band_idx].ucBytesTx.QuadPart += txblk->SrcBufLen;
#endif
				}
#endif
			}
		}
#ifdef TXRX_STAT_SUPPORT
		if ((mbss != NULL) || (mbss == NULL && txblk->wdev)) {
			UCHAR band_idx = HcGetBandByWdev(txblk->wdev);
			struct hdev_ctrl *ctrl = (struct hdev_ctrl *)ad->hdev_ctrl;
			txrx_mbss = &ad->ApCfg.MBSSID[txblk->wdev->wdev_idx];
			if (IS_MULTICAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				INC_COUNTER64(txrx_mbss->stat_bss.TxMulticastDataPacket);
				INC_COUNTER64(ctrl->rdev[band_idx].pRadioCtrl->TxMulticastDataPacket);
			} else if (IS_BROADCAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				INC_COUNTER64(txrx_mbss->stat_bss.TxBroadcastDataPacket);
				INC_COUNTER64(ctrl->rdev[band_idx].pRadioCtrl->TxBroadcastDataPacket);
			} else {
				INC_COUNTER64(txrx_mbss->stat_bss.TxUnicastDataPacket);
				INC_COUNTER64(ctrl->rdev[band_idx].pRadioCtrl->TxUnicastDataPacket);
			}
		}
#endif

		if (entry->Sst == SST_ASSOC) {
			INC_COUNTER64(entry->TxPackets);
			entry->TxBytes += txblk->SrcBufLen;
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_ENABLE(ad))
				entry->TxBytesMAP += txblk->SrcBufLen;
#endif
#ifdef TR181_SUPPORT
			if (txblk->wdev) {
				UCHAR band_idx = HcGetBandByWdev(txblk->wdev);
				ad->WlanCounters[band_idx].TxTotByteCount.QuadPart += txblk->SrcBufLen;
			}
#endif
			ad->TxTotalByteCnt += txblk->SrcBufLen;
		}
	}
#ifdef TXRX_STAT_SUPPORT
		RTMPGetUserPriority(ad, txblk->pPacket, wdev, &pUserPriority, &QIdx);
		if ((entry && (IS_ENTRY_CLIENT(entry) || IS_ENTRY_PEER_AP(entry))) && (entry->Sst == SST_ASSOC))
		{
			/*increase unicast packet count per station*/
			INC_COUNTER64(entry->TxDataPacketCount);
			INC_COUNTER64(entry->TxDataPacketCountPerAC[QIdx]);
			if (entry->pMbss) {
				INC_COUNTER64(entry->pMbss->stat_bss.TxDataPacketCount);
				INC_COUNTER64(entry->pMbss->stat_bss.TxDataPacketCountPerAC[QIdx]);
				entry->pMbss->stat_bss.TxDataPacketByte.QuadPart += txblk->SrcBufLen;
				entry->pMbss->stat_bss.TxDataPayloadByte.QuadPart += (txblk->SrcBufLen - 14);
				entry->pMbss->stat_bss.LastPktStaWcid = txblk->Wcid;
			}
			INC_COUNTER64(hdev->rdev->pRadioCtrl->TxDataPacketCount);
			INC_COUNTER64(hdev->rdev->pRadioCtrl->TxDataPacketCountPerAC[QIdx]);
			entry->TxDataPacketByte.QuadPart += txblk->SrcBufLen;
			hdev->rdev->pRadioCtrl->TxDataPacketByte.QuadPart += txblk->SrcBufLen;
		} else
		if (entry && (IS_ENTRY_MCAST(entry))) {
		/*increase mcast packet count per mbss*/
		}
#endif

#ifdef APCLI_SUPPORT
	if (IS_ENTRY_PEER_AP(entry)) {
		struct _STA_ADMIN_CONFIG *apcli = GetStaCfgByWdev(ad, entry->wdev);

		if (apcli != NULL) {
			apcli->StaStatistic.TxCount++;
			apcli->StaStatistic.TransmittedByteCount += txblk->SrcBufLen;
		}
	}
#endif

#ifdef WDS_SUPPORT

	if (IS_ENTRY_WDS(entry)) {
		INC_COUNTER64(ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
		ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedByteCount += txblk->SrcBufLen;
	}

#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */

}


INT ap_fp_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	UCHAR *pkt_va;
	UINT pkt_len;
	MAC_TABLE_ENTRY *entry = NULL;
	UINT16 wcid = WCID_INVALID;
	STA_TR_ENTRY *tr_entry = NULL;
	MAC_TABLE_ENTRY *pTmpEntry = NULL;
	UCHAR frag_nums;
#ifdef MAP_TS_TRAFFIC_SUPPORT
	MAC_TABLE_ENTRY *peer_entry = NULL;
#endif

	pkt_va = RTMP_GET_PKT_SRC_VA(pkt);
	pkt_len = RTMP_GET_PKT_LEN(pkt);

	if ((!pkt_va) || (pkt_len <= 14))
		return FALSE;

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE)) {
		if (is_looping_packet(pAd, pkt))
			return FALSE;
	}

#endif /* CONFIG_WIFI_PKT_FWD */

	if (MAC_ADDR_IS_GROUP(pkt_va)) {
#ifdef CONFIG_VLAN_GTK_SUPPORT
		struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);
		INT16 vlan_id;
		struct vlan_gtk_info *vg_info;
#endif
#ifdef A4_CONN
		/* If we check an ethernet source move to this device, we should remove it. */
		if (!RTMP_GET_PACKET_A4_FWDDATA(pkt))
		a4_proxy_delete(pAd, wdev->func_idx, (pkt_va + MAC_ADDR_LEN));
#endif /* A4_CONN */
		if (wdev->PortSecured != WPA_802_1X_PORT_SECURED)
			return FALSE;
		wcid = wdev->tr_tb_idx;
#ifdef CONFIG_VLAN_GTK_SUPPORT
		vlan_id = CFG80211_IsVlanPkt(pkt);
		vg_info = CFG80211_GetVlanInfoByVlanid(wdev, vlan_id);
		if (vlan_id > 0 && vg_info) {
			/* substitute wcid with vlan bmc_wcid */
			wcid = vg_info->vlan_tr_tb_idx;

			/* remove vlan tag and set procotol */
			memmove(skb->data + VLAN_HLEN, skb->data, MAC_ADDR_LEN*2);
			RtmpOsSkbPullRcsum(skb, LENGTH_802_1Q);
			RtmpOsSkbResetMacHeader(skb);
			RtmpOsSkbResetNetworkHeader(skb);
			RtmpOsSkbResetTransportHeader(skb);
			RtmpOsSkbResetMacLen(skb);
			skb->protocol = skb->data[12] + (skb->data[13] << 8);
			RTMP_SET_PACKET_VLANGTK(pkt, vlan_id);
			RTMP_SET_PACKET_VLAN(pkt, FALSE);
			RTMP_SET_PACKET_PROTOCOL(pkt, ntohs(skb->protocol));
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s() tx bmc pkt, proto=0x%x, bmc_wcid=%d vlan_id=%d\n", __func__, ntohs(skb->protocol), wcid, vlan_id);
		}
#endif
	} else {
		entry = MacTableLookup(pAd, pkt_va);

		if (entry && (entry->Sst == SST_ASSOC)) {
#ifdef WH_EVENT_NOTIFIER
			if (IS_ENTRY_CLIENT(entry)
#ifdef A4_CONN
				&& !IS_ENTRY_A4(entry)
#endif /* A4_CONN */
			)
				entry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */

#if defined(RT_CFG80211_SUPPORT) || defined(DYNAMIC_VLAN_SUPPORT)
			{
				UCHAR *pSrcBuf;
				UINT16 TypeLen;

				pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
				TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

#ifdef DYNAMIC_VLAN_SUPPORT
				pSrcBuf += LENGTH_802_3;
				if (TypeLen == ETH_TYPE_VLAN && entry->vlan_id) {
					USHORT vlan_id = *(USHORT *)pSrcBuf;

					vlan_id = cpu2be16(vlan_id);
					vlan_id = vlan_id & 0x0FFF; /* 12 bit */
					if (vlan_id != entry->vlan_id)
						return FALSE;
					pSrcBuf -= LENGTH_802_3;
					memmove(pSrcBuf + 4, pSrcBuf, 12);
					skb_pull(pkt, 4);
				}
#endif
#ifdef RT_CFG80211_SUPPORT

				if (TypeLen == ETH_TYPE_EAPOL) {
					struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
					struct _STA_TR_ENTRY *tr_entry = &tr_ctl->tr_entry[entry->wcid];

					if(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
						RTMP_SET_PACKET_CLEAR_EAP_FRAME(pkt,0);
					else
						RTMP_SET_PACKET_CLEAR_EAP_FRAME(pkt,1);

					if (pAd->FragFrame.wcid == entry->wcid) {
						MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"\nClear Wcid = %d FragBuffer !!!!!\n", entry->wcid);
						RESET_FRAGFRAME(pAd->FragFrame);
					}
				}
#endif
			}
#endif

			wcid = entry->wcid;
		}

#ifdef A4_CONN
		if ((entry == NULL)
#ifdef AIR_MONITOR
			|| (entry && IS_ENTRY_MONITOR(entry))
#endif
		){
			UINT16 main_wcid;

			/* If we check an ethernet source move to this device, we should remove it. */
			if (!RTMP_GET_PACKET_A4_FWDDATA(pkt))
			a4_proxy_delete(pAd, wdev->func_idx, (pkt_va + MAC_ADDR_LEN));
			if (a4_proxy_lookup(pAd, wdev->func_idx, pkt_va, FALSE, FALSE, &main_wcid))
				wcid = main_wcid;
		}
#endif /* A4_CONN */
#ifdef CLIENT_WDS
		if (entry == NULL) {
			PUCHAR pEntryAddr = CliWds_ProxyLookup(pAd, pkt_va);

			if (pEntryAddr != NULL) {
				entry = MacTableLookup(pAd, pEntryAddr);
				if (entry && (entry->Sst == SST_ASSOC))
					wcid = (UCHAR)entry->wcid;
			}
		}
#endif /* CLIENT_WDS */

#ifdef WTBL_TDD_SUPPORT
		if (IS_WTBL_TDD_ENABLED(pAd)) {
				if (entry == NULL) {
					MAC_TABLE_ENTRY *pEntry = NULL;
					pEntry = WtblTdd_InactiveList_Lookup(pAd, pkt_va);

					MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG, ("%s(): ---> (%02x:%02x:%02x:%02x:%02x:%02x) %d\n",
								__func__, PRINT_MAC(pkt_va), MAC_ADDR_HASH_INDEX(pkt_va)));
					if (pEntry) {
						WtblTdd_Entry_TxPacket(pAd, pEntry->wdev, pEntry);
						RTMP_SET_PACKET_PENDING(pkt, 1);

						/* WtblTdd_InactiveList_Predict(pAd, pEntry->wtblTddCtrl.ConnectTime); */
					} else {
						pAd->wtblTddInfo.txMissCounter++;
						MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"---> (%02x:%02x:%02x:%02x:%02x:%02x) %d\n",
							PRINT_MAC(pkt_va), MAC_ADDR_HASH_INDEX(pkt_va));
					}
					return FALSE;
				}
		}
#endif /* WTBL_TDD_SUPPORT */

	}
	if (!IS_TR_WCID_VALID(pAd, wcid))
			return FALSE;
		tr_entry = &tr_ctl->tr_entry[wcid];

	if (!IS_VALID_ENTRY(tr_entry))
			return FALSE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
#ifdef SW_CONNECT_SUPPORT
	if (tr_entry->bSw == TRUE)
		RTMP_SET_PACKET_SW(pkt, 1);
	else
		RTMP_SET_PACKET_SW(pkt, 0);
#endif /* SW_CONNECT_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef CFG80211_SUPPORT

	/* CFG_TODO: POS NO GOOD */
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
		RTMP_SET_PACKET_OPMODE(pkt, OPMODE_AP);

#endif /* CFG80211_SUPPORT */
#endif
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);
#ifdef MAP_TS_TRAFFIC_SUPPORT
	if (pAd->bTSEnable) {
		peer_entry = &pAd->MacTab.Content[wcid];

		if (!map_ts_tx_process(pAd, wdev, pkt, peer_entry))
			return FALSE;
	}
#endif

	/*  ethertype check is not offload to mcu for fragment frame*/
	if ((frag_nums > 1)
#ifdef PER_PKT_CTRL_FOR_CTMR
		|| pAd->PerPktCtrlEnable
#endif
#ifdef IGMP_SNOOPING_NON_OFFLOAD
		|| (wdev->IgmpSnoopEnable && IS_MULTICAST_MAC_ADDR(GET_OS_PKT_DATAPTR(pkt)))
#endif
#ifdef SW_CONNECT_SUPPORT
		|| (tr_entry->bSw == TRUE)
#endif /* SW_CONNECT_SUPPORT */
	) {
		if (!RTMPCheckEtherType(pAd, pkt, tr_entry, wdev))
			return FALSE;
	} else {
		RTMPCheckDhcpArp(pkt);
	}

	if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED) {
		if (!((IS_AKM_WPA_CAPABILITY_Entry(wdev) || (entry && entry->bWscCapable)
#ifdef DOT1X_SUPPORT
			   || (IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */
#ifdef HOSTAPD_11R_SUPPORT
			   || (IS_AKM_PSK_Entry(wdev))
#endif
			  ) && ((RTMP_GET_PACKET_EAPOL(pkt) || RTMP_GET_PACKET_WAI(pkt))))
		   ) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Drop PKT before 4-Way Handshake done! wcid = %d.\n", wcid);
				return FALSE;
		}
	}

	/* if sta rec isn't valid, don't allow pkt tx */
	pTmpEntry = &pAd->MacTab.Content[wcid];
	if (!(pTmpEntry && pTmpEntry->sta_rec_valid)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Drop PKT before StaRec ready! wcid = %d.\n",  wcid);
		return FALSE;
	}

#ifdef SW_CONNECT_SUPPORT
	tr_entry->tx_fp_allow_cnt++;
#endif /* SW_CONNECT_SUPPORT */

	return TRUE;
}

INT ap_tx_pkt_allowed(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pkt)
{
	UCHAR *pkt_va;
	UINT pkt_len;
	MAC_TABLE_ENTRY *entry = NULL;
	UINT16 wcid = WCID_INVALID;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;
	MAC_TABLE_ENTRY *pTmpEntry = NULL;
	UCHAR frag_nums;
#ifdef MAP_TS_TRAFFIC_SUPPORT
	MAC_TABLE_ENTRY *peer_entry = NULL;
#endif

	pkt_va = RTMP_GET_PKT_SRC_VA(pkt);
	pkt_len = RTMP_GET_PKT_LEN(pkt);

	if ((!pkt_va) || (pkt_len <= 14))
		return FALSE;

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE)) {
		if (is_looping_packet(pAd, pkt))
			return FALSE;
	}

#endif /* CONFIG_WIFI_PKT_FWD */

	if (MAC_ADDR_IS_GROUP(pkt_va)) {
#ifdef A4_CONN
		/* If we check an ethernet source move to this device, we should remove it. */
		if (!RTMP_GET_PACKET_A4_FWDDATA(pkt))
		a4_proxy_delete(pAd, wdev->func_idx, (pkt_va + MAC_ADDR_LEN));
#endif /* A4_CONN */
		if (wdev->PortSecured != WPA_802_1X_PORT_SECURED)
			return FALSE;

		wcid = wdev->tr_tb_idx;
	} else {
		entry = MacTableLookup(pAd, pkt_va);

		if (entry && (entry->Sst == SST_ASSOC)) {
#ifdef WH_EVENT_NOTIFIER
			if (IS_ENTRY_CLIENT(entry)
#ifdef A4_CONN
				&& !IS_ENTRY_A4(entry)
#endif /* A4_CONN */
			)
				entry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */

#ifdef RADIUS_MAC_AUTH_SUPPORT
			if (wdev->radius_mac_auth_enable) {
				if (!entry->bAllowTraffic)
					return FALSE;
			}
#endif

#ifdef DYNAMIC_VLAN_SUPPORT
		{
			UCHAR *pSrcBuf;
			UINT16 TypeLen;

			if (entry->vlan_id) {
				pSrcBuf = GET_OS_PKT_DATAPTR (pkt);
				TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
				pSrcBuf += LENGTH_802_3;
				if (TypeLen == ETH_TYPE_VLAN) {
					USHORT vlan_id = *(USHORT *)pSrcBuf;

					vlan_id = cpu2be16(vlan_id);
					vlan_id = vlan_id & 0x0FFF; /* 12 bit */
					if (vlan_id != entry->vlan_id)
						return FALSE;
					pSrcBuf -= LENGTH_802_3;
					memmove(pSrcBuf + 4, pSrcBuf, 12);
					skb_pull(pkt, 4);
				}
			}
		}
#endif

			wcid = entry->wcid;
		}
#ifdef A4_CONN
		if ((entry == NULL)
#ifdef AIR_MONITOR
			|| (entry && IS_ENTRY_MONITOR(entry))
#endif
		) {
			UINT16 main_wcid;

			/* If we check an ethernet source move to this device, we should remove it. */
			if (!RTMP_GET_PACKET_A4_FWDDATA(pkt))
				a4_proxy_delete(pAd, wdev->func_idx, (pkt_va + MAC_ADDR_LEN));
			if (a4_proxy_lookup(pAd, wdev->func_idx, pkt_va, FALSE, FALSE, &main_wcid))
				wcid = main_wcid;
		}
#endif /* A4_CONN */
	}

	if (!IS_TR_WCID_VALID(pAd, wcid))
		return FALSE;

	tr_entry = &tr_ctl->tr_entry[wcid];

	if (!IS_VALID_ENTRY(tr_entry))
		return FALSE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
#ifdef SW_CONNECT_SUPPORT
	if (tr_entry->bSw == TRUE)
		RTMP_SET_PACKET_SW(pkt, 1);
	else
		RTMP_SET_PACKET_SW(pkt, 0);
#endif /* SW_CONNECT_SUPPORT */
#ifdef CONFIG_HOTSPOT

	/* Drop broadcast/multicast packet if disable dgaf */
	if (IS_ENTRY_CLIENT(tr_entry)) {
		BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

		if ((wcid == wdev->bss_info_argument.bmc_wlan_idx) &&
			(pMbss->HotSpotCtrl.HotSpotEnable || pMbss->HotSpotCtrl.bASANEnable) &&
			pMbss->HotSpotCtrl.DGAFDisable) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Drop broadcast/multicast packet when dgaf disable\n");
			return FALSE;
		}
	}

#endif
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);
#ifdef MAP_TS_TRAFFIC_SUPPORT
	if (pAd->bTSEnable) {
		peer_entry = &pAd->MacTab.Content[tr_entry->wcid];

		if (!map_ts_tx_process(pAd, wdev, pkt, peer_entry))
			return FALSE;
	}
#endif

	if (!RTMPCheckEtherType(pAd, pkt, tr_entry, wdev))
		return FALSE;

	if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED) {
		if (!((IS_AKM_WPA_CAPABILITY_Entry(wdev)
#ifdef DOT1X_SUPPORT
			   || (IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */

			  ) && ((RTMP_GET_PACKET_EAPOL(pkt) ||
					 RTMP_GET_PACKET_WAI(pkt))))
		) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Drop PKT before 4-Way Handshake done! wcid = %d.\n", wcid);
			return FALSE;
		}
	}

	/* if sta rec isn't valid, don't allow pkt tx */
	pTmpEntry = &pAd->MacTab.Content[wcid];
	if (!(pTmpEntry && pTmpEntry->sta_rec_valid)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Drop PKT before StaRec ready! wcid = %d.\n", wcid);
		return FALSE;
	}

#ifdef CFG80211_SUPPORT

	/* CFG_TODO: POS NO GOOD */
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
		RTMP_SET_PACKET_OPMODE(pkt, OPMODE_AP);

#endif /* CFG80211_SUPPORT */
	return TRUE;
}

UINT16 ap_mlme_search_wcid(RTMP_ADAPTER *pAd, UCHAR *addr1, UCHAR *addr2, PNDIS_PACKET pkt, struct wifi_dev *wdev)
{
	MAC_TABLE_ENTRY *mac_entry;

	if (wdev->tr_tb_idx == WCID_INVALID) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wdev not initailized\n");
		return 0;
	}


	if ((wdev->wdev_type == WDEV_TYPE_AP) && MAC_ADDR_IS_GROUP(addr1)) {
		mac_entry = &pAd->MacTab.Content[wdev->tr_tb_idx];
	} else {
		mac_entry = MacTableLookup(pAd, addr1);
	}

#ifdef MAC_REPEATER_SUPPORT
	if ((mac_entry != NULL) && (IS_ENTRY_PEER_AP(mac_entry) || IS_ENTRY_REPEATER(mac_entry))) {

		REPEATER_CLIENT_ENTRY *rept_entry = lookup_rept_entry(pAd, addr2);

		if (rept_entry) { /*repeater case*/
			if ((rept_entry->CliEnable == TRUE) && (rept_entry->CliValid == TRUE))
				mac_entry = rept_entry->pMacEntry;
		} else { /*apcli case*/
			UINT16 apcli_wcid = 0;

			if (mac_entry->wdev && (mac_entry->wdev->func_idx < pAd->ApCfg.ApCliNum))
				apcli_wcid = pAd->StaCfg[mac_entry->wdev->func_idx].MacTabWCID;
			else   /* use default apcli0 */
				apcli_wcid = pAd->StaCfg[0].MacTabWCID;

			mac_entry = &pAd->MacTab.Content[apcli_wcid];
		}
	}

#endif
	if (mac_entry)
		return mac_entry->wcid;
	else
		return 0;
}

INT ap_send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue)
{
	HEADER_802_11 *pHeader_802_11;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UCHAR *pSrcBufVA;
	INT ret;
	struct qm_ops *ops = pAd->qm_ops;
	 MAC_TABLE_ENTRY *pEntry = NULL;

	RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
	RTMP_SET_PACKET_MGMT_PKT(pkt, 1);
	pSrcBufVA = RTMP_GET_PKT_SRC_VA(pkt);

	if (pSrcBufVA == NULL) {
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	pHeader_802_11 = (HEADER_802_11 *)(pSrcBufVA + tx_hw_hdr_len);
	RTMP_SET_PACKET_WCID(pkt, ap_mlme_search_wcid(pAd, pHeader_802_11->Addr1, pHeader_802_11->Addr2, pkt, wdev));

	if (in_altx_filter_list(pHeader_802_11)
		&& (pHeader_802_11->FC.Type == FC_TYPE_MGMT)
		) {
		if (!(RTMP_GET_PACKET_TXTYPE(pkt) == TX_ATE_FRAME))
			RTMP_SET_PACKET_TYPE(pkt, TX_ALTX);
	}

	if  (!is_data_queue) {
	} else {
#ifdef UAPSD_SUPPORT
#ifdef P2P_SUPPORT

		if (P2P_GO_ON(pAd))
#else
#ifdef RT_CFG80211_P2P_SUPPORT
		if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
#endif /* RT_CFG80211_P2P_SUPPORT */
#endif /* P2P_SUPPORT */
		{
			UAPSD_MR_QOS_NULL_HANDLE(pAd, pHeader_802_11, pkt);
		}

#endif /* UAPSD_SUPPORT */
		RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pkt, 1);
	}

	/*Mgmt frame which is not in TX_ALTX cannot send when entry&starc is not valid*/
	pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	if ((!pEntry  || !(pEntry->sta_rec_valid)) && (RTMP_GET_PACKET_TYPE(pkt) != TX_ALTX)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_CAT_PS, DBG_LVL_ERROR,
			"pkt from non-connected sta, type=%d, sub_type=%d to ACQ, drop\n",
			pHeader_802_11->FC.Type,
			pHeader_802_11->FC.SubType);
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	ret = ops->enq_mgmtq_pkt(pAd, wdev, pkt);

	return ret;
}

static INT ap_ps_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STA_TR_ENTRY *tr_entry,
					PNDIS_PACKET pkt, UCHAR q_idx)
{
	struct qm_ctl *qm_ctl = &pAd->qm_ctl;
	struct qm_ops *qm_ops = pAd->qm_ops;
	UINT16 occupy_cnt = (tr_entry->token_cnt + tr_entry->enqCount);

	if (occupy_cnt >= SQ_ENQ_PS_MAX) {
		if ((tr_entry->ps_queue.Number < SQ_ENQ_PSQ_MAX) &&
		    (qm_ctl->total_psq_cnt < SQ_ENQ_PSQ_TOTAL_MAX)) {
			if (qm_ops->enq_psq_pkt) {
				qm_ops->enq_psq_pkt(pAd, wdev, tr_entry, pkt);
				qm_ctl->total_psq_cnt++;
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_CAT_PS, DBG_LVL_ERROR,
					"no enq_psq_pkt handler\n");
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			}
		} else {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
		}
	} else {
		if (tr_entry->ps_queue.Number != 0) {
			NDIS_PACKET *ps_pkt = NULL;
			UINT16 quota = (SQ_ENQ_PS_MAX - occupy_cnt);

			do {
				ps_pkt = qm_ops->get_psq_pkt(pAd, tr_entry);

				if (ps_pkt) {
					quota--;
					qm_ctl->total_psq_cnt--;
					qm_ops->enq_dataq_pkt(pAd, wdev, ps_pkt, q_idx);
				}
			} while (ps_pkt && (quota > 0));

			if (quota > 0) {
				qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
			} else {
				qm_ops->enq_psq_pkt(pAd, wdev, tr_entry, pkt);
				qm_ctl->total_psq_cnt++;
			}

		} else {
			qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

INT ap_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	UINT16 wcid = RTMP_GET_PACKET_WCID(pkt);
#ifdef IGMP_SNOOP_SUPPORT
	INT InIgmpGroup = IGMP_NONE;
	MULTICAST_FILTER_TABLE_ENTRY *pGroupEntry = NULL;
#endif /* IGMP_SNOOP_SUPPORT */
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef IGMP_SNOOP_SUPPORT
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#endif /* IGMP_SNOOP_SUPPORT */
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR user_prio = 0;
	UCHAR q_idx;
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;
	UCHAR *pkt_va;

	pkt_va = RTMP_GET_PKT_SRC_VA(pkt);
	tr_entry = &tr_ctl->tr_entry[wcid];
	user_prio = RTMP_GET_PACKET_UP(pkt);
	q_idx = RTMP_GET_PACKET_QUEIDX(pkt);

	if (tr_entry->EntryType != ENTRY_CAT_MCAST)
		wdev_ops->detect_wmm_traffic(pAd, wdev, user_prio, FLG_IS_OUTPUT);
	else {
#ifdef IGMP_SNOOP_SUPPORT
		if (wdev->IgmpSnoopEnable) {
			if (IgmpPktInfoQuery(pAd, pkt_va, pkt, wdev,
						&InIgmpGroup, &pGroupEntry) != NDIS_STATUS_SUCCESS) {
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}

			/* if it's a mcast packet in igmp gourp. ucast clone it for all members in the gourp. */
			if ((InIgmpGroup == IGMP_IN_GROUP)
				 && pGroupEntry
				 && (IgmpMemberCnt(&pGroupEntry->MemberList) > 0)) {
				NDIS_STATUS PktCloneResult = IgmpPktClone(pAd, wdev, pkt, InIgmpGroup, pGroupEntry,
								q_idx, user_prio, GET_OS_PKT_NETDEV(pkt));
#ifdef IGMP_TVM_SUPPORT
				if (PktCloneResult != NDIS_STATUS_MORE_PROCESSING_REQUIRED)
#endif /* IGMP_TVM_SUPPORT */
				{
					tr_cnt->igmp_clone_fail_drop++;
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
					return PktCloneResult;
				}
			}

			RTMP_SET_PACKET_TXTYPE(pkt, TX_MCAST_FRAME);
		} else
#endif /* IGMP_SNOOP_SUPPORT */
			RTMP_SET_PACKET_TXTYPE(pkt, TX_MCAST_FRAME);
	}

	RTMP_SET_PACKET_UP(pkt, user_prio);

	RTMP_SEM_LOCK(&tr_entry->ps_sync_lock);

	if (tr_entry->ps_state == PWR_ACTIVE)
		qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
	else
		ap_ps_handle(pAd, wdev, tr_entry, pkt, q_idx);

	RTMP_SEM_UNLOCK(&tr_entry->ps_sync_lock);

	ba_ori_session_start(pAd, tr_entry, user_prio);
	return NDIS_STATUS_SUCCESS;
}

/*
	--------------------------------------------------------
	FIND ENCRYPT KEY AND DECIDE CIPHER ALGORITHM
		Find the WPA key, either Group or Pairwise Key
		LEAP + TKIP also use WPA key.
	--------------------------------------------------------
	Decide WEP bit and cipher suite to be used.
	Same cipher suite should be used for whole fragment burst
	In Cisco CCX 2.0 Leap Authentication
		WepStatus is Ndis802_11WEPEnabled but the key will use PairwiseKey
		Instead of the SharedKey, SharedKey Length may be Zero.
*/
VOID ap_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	pTxBlk->CipherAlg = CIPHER_NONE;

	/* TODO:Eddy, Confirm MESH/Apcli.WAPI */
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)
		) {
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
	} else if (pMacEntry) {
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
#endif /* !SOFT_ENCRYPT && !SW_CONNECT_SUPPORT */

}

static inline VOID ap_build_cache_802_11_header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pHeader)
{
	STA_TR_ENTRY *tr_entry;
	HEADER_802_11 *pHeader80211;
	MAC_TABLE_ENTRY *pMacEntry;

	pHeader80211 = (PHEADER_802_11)pHeader;
	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = pTxBlk->tr_entry;
	/*
		Update the cached 802.11 HEADER
	*/
	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	/* More Bit */
	pHeader80211->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);
	/* Sequence */
	pHeader80211->Sequence = tr_entry->TxSeq[pTxBlk->UserPriority];
	tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
	/* SA */
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
		if (FALSE
#ifdef WDS_SUPPORT
			|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
			|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif /* CLIENT_WDS */
		   ) {
			/* The addr3 of WDS packet is Destination Mac address and Addr4 is the Source Mac address. */
			COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
			COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
			pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
		} else
#endif /* WDS_SUPPORT || CLIENT_WDS */
#ifdef A4_CONN
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bA4Frame)) {
		pHeader80211->FC.ToDs = 1;
		pHeader80211->FC.FrDs = 1;
		if (pTxBlk->pMacEntry) {
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_PEER_AP(pTxBlk->pMacEntry)) {
				COPY_MAC_ADDR(pHeader80211->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid)); /* to AP2 */
				COPY_MAC_ADDR(pHeader80211->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);
			} else
#endif /* APCLI_SUPPORT */
			if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry)) {
				COPY_MAC_ADDR(pHeader80211->Addr1, pTxBlk->pMacEntry->Addr);/* to AP2 */
				COPY_MAC_ADDR(pHeader80211->Addr2, pAd->CurrentAddress); /* from AP1 */
			}
			COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);	/* DA */
			COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
			pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
		} else
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTxBlk->pMacEntry == NULL!\n");
	} else
#endif /* A4_CONN */
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) {
				/* The addr3 of Ap-client packet is Destination Mac address. */
				COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
			} else
#endif /* APCLI_SUPPORT */
			{	/* The addr3 of normal packet send from DS is Src Mac address. */
				COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
			}
}

static inline VOID ap_build_802_11_header(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	struct wifi_dev *wdev = pTxBlk->wdev;
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;
	/*
		MAKE A COMMON 802.11 HEADER
	*/
	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	/* TODO: shiang-7603 */
	pTxBlk->wifi_hdr = &pTxBlk->HeaderBuf[tx_hw_hdr_len];
	wifi_hdr = (HEADER_802_11 *)pTxBlk->wifi_hdr;
	NdisZeroMemory(wifi_hdr, sizeof(HEADER_802_11));
	wifi_hdr->FC.FrDs = 1;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = ((TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? SUBTYPE_QDATA : SUBTYPE_DATA);

	/* TODO: shiang-usw, for BCAST/MCAST, original it's sequence assigned by "pAd->Sequence", how about now? */
	if (tr_entry) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
			wifi_hdr->Sequence = tr_entry->TxSeq[pTxBlk->UserPriority];
			tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
		} else {
			wifi_hdr->Sequence = tr_entry->NonQosDataSeq;
			tr_entry->NonQosDataSeq = (tr_entry->NonQosDataSeq + 1) & MAXSEQ;
		}
	} else {
		wifi_hdr->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ; /* next sequence */
	}

	wifi_hdr->Frag = 0;
	wifi_hdr->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

#ifdef A4_CONN
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bA4Frame)) {
		wifi_hdr->FC.ToDs = 1;
		wifi_hdr->FC.FrDs = 1;
		if (pTxBlk->pMacEntry) {
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_PEER_AP(pTxBlk->pMacEntry)) {
				COPY_MAC_ADDR(wifi_hdr->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid)); /* to AP2 */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);
			} else
#endif /* APCLI_SUPPORT */
			if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry)) {
				COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr);/* to AP2 */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress); /* from AP1 */
			}
			COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);	/* DA */
			COPY_MAC_ADDR(wifi_hdr->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
			pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
		} else
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTxBlk->pMacEntry == NULL!\n");
	} else
#endif /* A4_CONN*/

#ifdef APCLI_SUPPORT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket)) {
			wifi_hdr->FC.ToDs = 1;
			wifi_hdr->FC.FrDs = 0;
			COPY_MAC_ADDR(wifi_hdr->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid));	/* to AP2 */
#ifdef MAC_REPEATER_SUPPORT

			if (pTxBlk->pMacEntry && IS_REPT_LINK_UP(pTxBlk->pMacEntry->pReptCli))
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pMacEntry->pReptCli->CurrentAddress);
			else
#endif /* MAC_REPEATER_SUPPORT */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);		/* from AP1 */

			COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);					/* DA */
		} else
#endif /* APCLI_SUPPORT */
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
			if (FALSE
#ifdef WDS_SUPPORT
				|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
				|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif /* CLIENT_WDS */
			   ) {
				wifi_hdr->FC.ToDs = 1;

				if (pTxBlk->pMacEntry == NULL)
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTxBlk->pMacEntry == NULL!\n");
				else
					COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr);				/* to AP2 */

				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress);						/* from AP1 */
				COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);					/* DA */
				COPY_MAC_ADDR(&wifi_hdr->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
				pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
				pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
			} else
#endif /* WDS_SUPPORT || CLIENT_WDS */
			{
				/* TODO: how about "MoreData" bit? AP need to set this bit especially for PS-POLL response */
#if defined(IGMP_SNOOP_SUPPORT)
				if (pTxBlk->tr_entry->EntryType != ENTRY_CAT_MCAST) {
					COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr); /* DA */
				} else
#endif /* defined(IGMP_SNOOP_SUPPORT) */
				{
					COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
				}

				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->ApCfg.MBSSID[wdev->func_idx].wdev.bssid);		/* BSSID */
				COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);			/* SA */
			}

#ifdef P2P_SUPPORT

	/* To not disturb the Opps test, set psm bit if I use power save mode.  */
	/* P2P Test case 7.1.3 */
	if (P2P_INF_ON(pAd) && P2P_CLI_ON(pAd) &&
		(P2P_TEST_BIT(pAd->P2pCfg.CTWindows, P2P_OPPS_BIT))
		&& (pAd->P2pCfg.bP2pCliPmEnable))
		wifi_hdr->FC.PwrMgmt = 1;

#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT

	/* To not disturb the Opps test, set psm bit if I use power save mode.	*/
	/* P2P Test case 7.1.3 */
	if (CFG_P2PCLI_ON(pAd) && pAd->cfg80211_ctrl.bP2pCliPmEnable &&
		CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.CTWindows, P2P_OPPS_BIT))
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;

#endif /* P2P_SUPPORT */

	if (pTxBlk->CipherAlg) /* reference enum MT_SEC_CIPHER_SUITS_T , not bitwise */
		wifi_hdr->FC.Wep = 1;

	pTxBlk->dot11_type = wifi_hdr->FC.Type;
	pTxBlk->dot11_subtype = wifi_hdr->FC.SubType;
}

BOOLEAN ap_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	pPacket = pTxBlk->pPacket;
	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
#ifdef MT7626_REDUCE_TX_OVERHEAD
	pTxBlk->wmm_set = wdev->WmmIdx;
#else
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
#endif /* MT7626_REDUCE_TX_OVERHEAD */
	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->FrameGap = IFS_HTTXOP;
	pTxBlk->pMbss = NULL;
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
#ifdef MCAST_RATE_SPECIFIC
			PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);

			if (((*pDA & 0x01) == 0x01) && (*pDA != 0xff)) {
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
				pTxBlk->pTransmit = (wdev->channel > 14) ?
					(&wdev->rate.MCastPhyMode_5G) :
					(&wdev->rate.MCastPhyMode);
#else
				pTxBlk->pTransmit = &wdev->rate.mcastphymode;
#endif
			} else
#endif /* MCAST_RATE_SPECIFIC */
			{
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;

				if (WMODE_CAP_5G(pTxBlk->wdev->PhyMode)) {
					pTxBlk->pTransmit->field.MODE = MODE_OFDM;
					pTxBlk->pTransmit->field.MCS = MCS_RATE_6;
				}
			}
		}
		/* AckRequired = FALSE, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, (fTX_bAckRequired | fTX_bAllowFrag | fTX_bWMM));

		if (RTMP_GET_PACKET_MOREDATA(pPacket))
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
	} else {
		pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;
		pMacEntry = pTxBlk->pMacEntry;

		if (!pMacEntry)
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Err!! pMacEntry is NULL!!\n");
		else
			pTxBlk->pMbss = pMacEntry->pMbss;

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

		{
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)

			if (pTxBlk->OpMode == OPMODE_AP)
#else
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT*/
			{
#ifdef WDS_SUPPORT

				if (IS_ENTRY_WDS(pMacEntry))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWDSEntry);
				else
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
					if (IS_ENTRY_CLIWDS(pMacEntry)) {
						PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
						PUCHAR pSA = GET_OS_PKT_DATAPTR(pPacket) + MAC_ADDR_LEN;
					UCHAR idx = pMacEntry->func_tb_idx;

					if (((idx < MAX_MBSSID_NUM(pAd))
						 && !MAC_ADDR_EQUAL(pSA, pAd->ApCfg.MBSSID[idx].wdev.bssid))
							|| !MAC_ADDR_EQUAL(pDA, pMacEntry->Addr)
						   )
							TX_BLK_SET_FLAG(pTxBlk, fTX_bClientWDSFrame);
					} else
#endif /* CLIENT_WDS */
						if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)) {
#ifdef A4_CONN
					if (IS_ENTRY_A4(pMacEntry)
					&& !RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))
						TX_BLK_SET_FLAG(pTxBlk, fTX_bA4Frame);
#endif /* A4_CONN */
						} else
							return FALSE;

				/* If both of peer and us support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME) {
			if (((RTMP_GET_PACKET_LOWRATE(pPacket))
#ifdef UAPSD_SUPPORT
				  && (!(pMacEntry && (pMacEntry->bAPSDFlagSPStart)))
#endif /* UAPSD_SUPPORT */
				 )  ||
#ifdef SW_CONNECT_SUPPORT
				(RTMP_GET_PACKET_SW(pPacket)) ||
#endif /* SW_CONNECT_SUPPORT */
				 ((pAd->OpMode == OPMODE_AP) && (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			   ) {
				/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate. */
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;
#ifdef SW_CONNECT_SUPPORT
				if (RTMP_GET_PACKET_LOWRATE(pPacket)) {
					pTxBlk->pTransmit = &(wdev->rate.MlmeTransmit);
				} else {
					if (RTMP_GET_PACKET_SW(pPacket)) {/* use dummy's STA's max rate */
						if ((pTxBlk->pMacEntry) && (wdev->pDummy_obj)) {
							if (wdev->pDummy_obj->bFixedRateSet == TRUE) /* use fixed rate */
								pTxBlk->pTransmit->word = wdev->pDummy_obj->HTPhyMode.word;
						}
					}
				}

#endif /* SW_CONNECT_SUPPORT */
				TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);

				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (!pTxBlk->pMacEntry)
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Err!! pTxBlk->pMacEntry is NULL!!\n");
				else if (IS_HT_STA(pTxBlk->pMacEntry) &&
						 (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) &&
						 ((pAd->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)))
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
			}

			if (!pMacEntry)
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Err!! pMacEntry is NULL!!\n");
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
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Err!! pMacEntry is NULL!!\n");
		else
			pMacEntry->DebugTxCount++;

#ifdef IGMP_SNOOP_SUPPORT
		if (RTMP_GET_PACKET_MCAST_CLONE(pPacket)) {
			TX_BLK_SET_FLAG(pTxBlk, fTX_MCAST_CLONE);
#ifdef IGMP_SNOOPING_NON_OFFLOAD
			TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);
#endif
		}
#endif

#ifdef SW_CONNECT_SUPPORT
		/* correct to H/W WCID before fill TxD */
		if (RTMP_GET_PACKET_SW(pPacket)) {
			/* only Unicast entry will bet set as S/W */
			TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);
			TX_BLK_CLEAR_FLAG(pTxBlk, fTX_HDR_TRANS);
			if (pTxBlk->tr_entry->EntryType != ENTRY_CAT_MCAST)
				pTxBlk->Wcid = pTxBlk->tr_entry->hw_wcid;
		} else {
			if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
				pTxBlk->Wcid = wdev->hw_bmc_wcid;
		}
#endif /* SW_CONNECT_SUPPORT */
	}

	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;
	ops->find_cipher_algorithm(pAd, wdev, pTxBlk);
	return TRUE;
}

BOOLEAN ap_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;

	pPacket = pTxBlk->pPacket;
#ifdef SW_CONNECT_SUPPORT
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
		pTxBlk->Wcid = wdev->hw_bmc_wcid;
#else /* SW_CONNECT_SUPPORT */
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
#endif /* !SW_CONNECT_SUPPORT */
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
#ifdef MT7626_REDUCE_TX_OVERHEAD
	pTxBlk->wmm_set = wdev->WmmIdx;
#else
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
#endif /* MT7626_REDUCE_TX_OVERHEAD */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	/*get MBSS for tx counter usage*/
	if (pTxBlk->TxFrameType != TX_MCAST_FRAME)
		pTxBlk->pMbss = pTxBlk->pMacEntry->pMbss;
	return TRUE;
}

INT ap_mlme_mgmtq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	UCHAR *tmac_info;
	HTTRANSMIT_SETTING *transmit, tmp_transmit;
	UCHAR MlmeRate;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	PHEADER_802_11 pHeader_802_11;
	MAC_TABLE_ENTRY *pMacEntry = tx_blk->pMacEntry;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR PID, tx_rate;
	UINT16 wcid, hw_wcid;
	UCHAR prot = 0;
	UCHAR apidx = 0;
	MAC_TX_INFO mac_info;
	struct DOT11_H *pDot11h = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef	CONFIG_RCSA_SUPPORT
	UINT8 BandIdx = 0;
#ifdef MT_DFS_SUPPORT
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#endif
#endif
#ifdef DPP_SUPPORT
	UINT16 orig_sn;
#endif /* DPP_SUPPORT */

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return NDIS_STATUS_FAILURE;

	ap_fill_offload_tx_blk(pAd, wdev, tx_blk);
	tmac_info = tx_blk->pSrcBufHeader;
	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

	/*
		copy to local var first to prevernt the dev->rate.MlmeTransmit is change this moment
	*/
	NdisMoveMemory(&tmp_transmit, &wdev->rate.MlmeTransmit, sizeof(HTTRANSMIT_SETTING));
	transmit = &tmp_transmit;
	/* Verify Mlme rate for a / g bands.*/
	if (WMODE_CAP_5G(wdev->PhyMode) && (MlmeRate < RATE_6)) {
		MlmeRate = RATE_6;
		transmit->field.MCS = MCS_RATE_6;
		transmit->field.MODE = MODE_OFDM;
	}

	pHeader_802_11->FC.MoreData = RTMP_GET_PACKET_MOREDATA(tx_blk->pPacket);
	bInsertTimestamp = FALSE;

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) { /* must be PS-POLL*/
		bAckRequired = FALSE;
#ifdef VHT_TXBF_SUPPORT

		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA) {
			pHeader_802_11->Duration = 100;
		}

#endif /* VHT_TXBF_SUPPORT */
	} else { /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
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
#ifdef SPECIFIC_TX_POWER_SUPPORT
					/* Find which MBSSID to be send this probeRsp */
					UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

					if (!(apidx >= pAd->ApCfg.BssidNum) &&
						 (pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
						 (transmit->field.MODE == MODE_CCK) &&
						 (transmit->field.MCS == RATE_1))
						TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;

#endif /* SPECIFIC_TX_POWER_SUPPORT */
				} else if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Request*/
				} else if ((pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH) &&
						   (pMacEntry == NULL)) {
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Deauth */
				}
			}
		}
	}

#ifdef DPP_SUPPORT
	orig_sn = pHeader_802_11->Sequence;
#endif /* DPP_SUPPORT */
	pHeader_802_11->Sequence = pAd->Sequence++;

	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	/*
		Before radar detection done, mgmt frame can not be sent but probe req
		Because we need to use probe req to trigger driver to send probe req in passive scan
	*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
#ifdef CONFIG_RCSA_SUPPORT
		&& (pHeader_802_11->FC.SubType != SUBTYPE_ACTION)
#endif
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pDot11h->RDMode != RD_NORMAL_MODE)) {
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	/*
		fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one physical buffer, and the whole frame size equals
		to the first scatter buffer size

		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement

		management frame doesn't need encryption.
		so use WCID_NO_MATCHED no matter u are sending to specific wcid or not
	*/
	PID = PID_MGMT;
#ifdef DOT11W_PMF_SUPPORT
	PMF_PerformTxFrameAction(pAd, pHeader_802_11, tx_blk->SrcBufLen, tx_hw_hdr_len, &prot);
#endif

	if (pMacEntry) {
#ifdef SW_CONNECT_SUPPORT
		wcid = pMacEntry->wcid;
		hw_wcid = pMacEntry->hw_wcid;
#else /* SW_CONNECT_SUPPORT */
		hw_wcid = wcid = pMacEntry->wcid;
#endif /* !SW_CONNECT_SUPPORT */
	} else {
		hw_wcid = wcid = 0;
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pMacEntry is null !!\n");
	}

	tx_rate = (UCHAR)transmit->field.MCS;
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
	mac_info.WCID = hw_wcid;
	mac_info.Type = pHeader_802_11->FC.Type;
	mac_info.SubType = pHeader_802_11->FC.SubType;
	mac_info.PsmBySw = 1;
	mac_info.txpwr_offset = 0;

	if (pAd->CommonCfg.bSeOff != TRUE) {
		if (HcGetBandByWdev(wdev) == BAND0) {
			if ((pAd->CommonCfg.CCKTxStream[BAND0] == 1) &&
				(transmit->field.MODE == MODE_CCK))
				mac_info.AntPri = 0x0;
			else
				mac_info.AntPri = BAND0_SPE_IDX;
		}
		else if (HcGetBandByWdev(wdev) == BAND1) {
			mac_info.AntPri = BAND1_SPE_IDX;
		}
	}
	/* check if the pkt is Tmr frame. */
	mac_info.Length = (tx_blk->SrcBufLen - tx_hw_hdr_len);

	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;

		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
/*	if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) || (pHeader_802_11->FC.SubType == SUBTYPE_BEACON))*/
	if (wdev->eap.eap_mgmrate_en == TRUE) {
		tx_rate = wdev->eap.mgmphymode.field.MCS;
		transmit = &wdev->eap.mgmphymode;
	}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


#ifdef DPP_SUPPORT
		/* Check whether it is a DPP frame */
		if (!mac_info.BM && (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) &&
			((tx_blk->SrcBufLen - tx_hw_hdr_len) > sizeof(HEADER_802_11))) {
			GAS_FRAME *GASFrame = (GAS_FRAME *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
			UCHAR *pkt_buf = (tx_blk->pSrcBufHeader + tx_hw_hdr_len + sizeof(HEADER_802_11));
			/* Public action frame with WFA DPP */
			if ((pkt_buf[0] == CATEGORY_PUBLIC) &&
					(pkt_buf[1] == ACTION_WIFI_DIRECT) && /*vendor specific*/
					(memcmp(&pkt_buf[2], DPP_OUI, OUI_LEN) == 0) &&
					(pkt_buf[2 + OUI_LEN] == WFA_DPP_SUBTYPE) &&
					(!MAC_ADDR_IS_GROUP(pHeader_802_11->Addr1))) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"sending out a dpp frame on interface:%s da:%02x:%02x:%02x:%02x:%02x:%02x\n",
					wdev->if_dev->name, PRINT_MAC(pHeader_802_11->Addr1));
				PID = PID_MGMT_DPP_FRAME;
				mac_info.seq_no = orig_sn;
			}
			/* Gas DPP frame */
			if (((GASFrame->u.GAS_INIT_RSP.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
				NdisEqualMemory(&GASFrame->u.GAS_INIT_RSP.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
				GASFrame->u.GAS_INIT_RSP.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE)
				||
				((GASFrame->u.GAS_CB_RSP.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
				NdisEqualMemory(&GASFrame->u.GAS_CB_RSP.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
				GASFrame->u.GAS_CB_RSP.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"sending out a dpp frame on interface %s\n",
					wdev->if_dev->name);
				PID = PID_MGMT_DPP_FRAME;
				mac_info.seq_no = orig_sn;
				transmit = &tmp_transmit;
				transmit->field.MODE = MODE_OFDM;
				transmit->field.BW = BW_20;
				transmit->field.MCS = MCS_RATE_6;
				tx_rate = transmit->field.MCS;
			}
		}
#endif /* DPP_SUPPORT */
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
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType);
			hex_dump("DataFrame", (char *)pHeader_802_11, 24);
			break;
		}

		if (pMacEntry && tr_ctl->tr_entry[wcid].PsDeQWaitCnt)
			PID = PID_PS_DATA;

		mac_info.WCID = hw_wcid;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_PS_POLL:
			mac_info.hdr_len = sizeof(PSPOLL_FRAME);
			tx_rate = (UCHAR)transmit->field.MCS;
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

	MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%d, WMMSET=%d,QId=%d\n",
			  __LINE__, mac_info.wmm_set, mac_info.q_idx);
#ifdef CONFIG_MULTI_CHANNEL

	if (pAd->Mlme.bStartMcc == TRUE) {
		if ((NdisEqualMemory(pAd->cfg80211_ctrl.P2PCurrentAddress, pHeader_802_11->Addr2, MAC_ADDR_LEN))
			|| (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP))
			mac_info.q_idx = Q_IDX_AC10;
		else
			mac_info.q_idx = Q_IDX_AC0;
	}

#endif /* CONFIG_MULTI_CHANNEL */
#ifdef APCLI_SUPPORT

	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA)
		&& ((pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL) || (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))) {
		if ((pMacEntry != NULL) && (IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry))) {
			/* CURRENT_BW_TX_CNT/CURRENT_BW_FAIL_CNT only count for aute rate */
			if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd) || IS_AXE(pAd) || IS_MT7626(pAd) || IS_MT7915(pAd))
				mac_info.IsAutoRate = TRUE;
			if (IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
				PSTA_ADMIN_CONFIG sta_cfg = GetStaCfgByWdev(pAd, pMacEntry->wdev);

				if (sta_cfg && twtPlannerIsRunning(pAd, sta_cfg))
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
					mac_info.Ack = 0;
			}
		}
	}

#ifdef	CONFIG_RCSA_SUPPORT
	if ((pDot11h->RDMode == RD_SWITCHING_MODE) && (pDfsParam->bRCSAEn == TRUE)
		&& (pHeader_802_11->FC.Type == FC_TYPE_MGMT) && (pHeader_802_11->FC.SubType == SUBTYPE_ACTION)) {
		BandIdx = HcGetBandByWdev(wdev);
		mac_info.q_idx = TxQ_IDX_ALTX0;
		if (BandIdx == BAND1)
			mac_info.q_idx = TxQ_IDX_ALTX1;

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set q_idx to %d\n",
					 mac_info.q_idx);

		mac_info.Type = FC_TYPE_MGMT;
		mac_info.SubType = SUBTYPE_ACTION;
	}
#endif
#endif /* APCLI_SUPPORT */
#ifdef SOFT_SOUNDING

	if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
		&& pMacEntry && (pMacEntry->snd_reqired == TRUE)) {
		tx_rate = (UCHAR)pMacEntry->snd_rate.field.MCS;
		NdisMoveMemory(&tmp_transmit, &pMacEntry->snd_rate, sizeof(HTTRANSMIT_SETTING));
		mac_info.Txopmode = IFS_PIFS;
		pMacEntry->snd_reqired = FALSE;
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Kick Sounding to "MACSTR", dataRate(PhyMode:%s, BW:%sHz, %dSS, MCS%d)\n",
				 MAC2STR(pMacEntry->Addr),
				 get_phymode_str(transmit->field.MODE),
				 get_bw_str(transmit->field.BW),
				 (transmit->field.MCS>>4) + 1, (transmit->field.MCS & 0xf));
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

#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev) && WMODE_CAP_2G(wdev->PhyMode) &&
		pHeader_802_11->FC.Type == FC_TYPE_MGMT && pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) {
		transmit->field.MODE = (transmit->field.MODE >= MODE_OFDM) ? transmit->field.MODE : MODE_OFDM;
	}
#endif /* OCE_SUPPORT */
	if (wdev) {
		if (WMODE_CAP_5G(wdev->PhyMode)) {
			if (transmit->field.MODE == MODE_CCK) {
				/*
				    something wrong with rate->MlmeTransmit
					correct with OFDM mode
				*/
				transmit->field.MODE = MODE_OFDM;
				MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					"@@@@ FIXME!!frame(Type=%x, SubType=%x) use the CCK RATE but wdev support A band only, mac_info.Length=%lu, mac_info.wmm_set=%d, mac_info.q_idx=%d, mac_info.OmacIdx=%d\n",
						 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType, mac_info.Length, mac_info.wmm_set, mac_info.q_idx, mac_info.OmacIdx);
			}
		}
	}

#ifdef CONFIG_RCSA_SUPPORT
	/* Before sending RCSA using ALTx0 first flush it then enable as there might be pending pkts */
	if ((pDot11h->RDMode == RD_SWITCHING_MODE) && (pDfsParam->bRCSAEn == TRUE) && (pDfsParam->ChSwMode == 1)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Enable ALTX for BandIdx:%d for RCSA Tx\n", BandIdx);
		mtRddControl(pAd, RDD_ALTX_CTRL, BandIdx, 0, 1);
		pAd->CommonCfg.DfsParameter.fCheckRcsaTxDone = TRUE;
	}
#endif
#ifdef TXRX_STAT_SUPPORT
	{
		UINT32 bandidx = HcGetBandByWdev(wdev);
		if (!(scan_in_run_state(pAd, wdev)) && pMacEntry && IS_ENTRY_CLIENT(pMacEntry) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
			UCHAR band_idx = HcGetBandByWdev(pMacEntry->wdev);
			struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
			INC_COUNTER64(pMacEntry->TxMgmtPacketCount);
			INC_COUNTER64(pMacEntry->pMbss->stat_bss.TxMgmtPacketCount);
			INC_COUNTER64(ctrl->rdev[band_idx].pRadioCtrl->TxMgmtPacketCount);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, " On Channel ::  ap_mlme_mgmtq_tx ::	Tx Mgmt Subtype : %d\n", pHeader_802_11->FC.SubType);
		}
		if ((scan_in_run_state(pAd, wdev)) && (pAd->ScanCtrl[bandidx].ScanType == SCAN_ACTIVE) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT) && wdev) {
			BSS_STRUCT *mbss = NULL;
			mbss = &pAd->ApCfg.MBSSID[wdev->wdev_idx];
			INC_COUNTER64(mbss->stat_bss.TxMgmtOffChPktCount);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, " Off Channel :: ap_mlme_mgmtq_tx ::	Tx Mgmt Subtype : %d\n", pHeader_802_11->FC.SubType);
		}
	}
#endif
#ifdef MGMT_TXPWR_CTRL
		if ((wdev->bPwrCtrlEn == TRUE) && (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) &&
			(pHeader_802_11->FC.Type == FC_TYPE_MGMT) && (wdev->tr_tb_idx != WCID_INVALID)) {
			mac_info.WCID = wdev->tr_tb_idx;
			mac_info.IsAutoRate = TRUE;
		}
#endif

	if ((RTMP_GET_PACKET_TYPE(tx_blk->pPacket) != TX_ALTX) && (mac_info.WCID == 0)) {
		enum PACKET_TYPE pkt_type_old = RTMP_GET_PACKET_TYPE(tx_blk->pPacket);
		UCHAR q_idx_old = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
		struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
		struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;

		RTMP_SET_PACKET_TYPE(tx_blk->pPacket, TX_ALTX);
		mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
		tr_cnt->pkt_invalid_wcid++;

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pkt_t(%d),type(%d),sub_type(%d),q_idx(%d,%d),cnt(%d)\n",
			pkt_type_old, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType,
			q_idx_old, mac_info.q_idx, tr_cnt->pkt_invalid_wcid);

		if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) {
			PFRAME_ACTION_HDR Frame = (PFRAME_ACTION_HDR)pHeader_802_11;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cat(%d),act(%d)\n",
				Frame->Category, Frame->Action);
		}
	}

	return asic_mlme_hw_tx(pAd, tmac_info, &mac_info, transmit, tx_blk);
}

INT ap_mlme_dataq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
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
	UINT16 wcid, hw_wcid;
	MAC_TABLE_ENTRY *pMacEntry = tx_blk->pMacEntry;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	HTTRANSMIT_SETTING *transmit, tmp_transmit, TransmitSetting;
	MAC_TX_INFO mac_info;
#ifdef SPECIFIC_TX_POWER_SUPPORT
	UCHAR TxPwrAdj = 0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
	struct dev_rate_info *rate;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return NDIS_STATUS_FAILURE;

	ap_fill_offload_tx_blk(pAd, wdev, tx_blk);

	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

	rate = &wdev->rate;
	frm_buf = tx_blk->pSrcBufHeader;
	frm_len = tx_blk->SrcBufLen;
	tmac_info = tx_blk->pSrcBufHeader;

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;


	/*
		copy to local var first to prevernt the dev->rate.MlmeTransmit is change this moment
	*/
	NdisMoveMemory(&tmp_transmit, &wdev->rate.MlmeTransmit, sizeof(HTTRANSMIT_SETTING));
	transmit = &tmp_transmit;

	/* Verify Mlme rate for a/g bands.*/
	if (WMODE_CAP_5G(wdev->PhyMode) && (MlmeRate < RATE_6)) { /* 11A band*/
		MlmeRate = RATE_6;
		transmit->field.MCS = MCS_RATE_6;
		transmit->field.MODE = MODE_OFDM;
	}

	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/
	/* In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame */
	bInsertTimestamp = FALSE;

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		if (pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ) {
			pBar = (PFRAME_BAR)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
			bAckRequired = TRUE;
		} else
			bAckRequired = FALSE;

#ifdef VHT_TXBF_SUPPORT

		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA) {
			pHeader_802_11->Duration =
				RTMPCalcDuration(pAd, MlmeRate, (tx_blk->SrcBufLen - TXINFO_SIZE - cap->TXWISize - TSO_SIZE));
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
#ifdef SPECIFIC_TX_POWER_SUPPORT
				/* Find which MBSSID to be send this probeRsp */
				UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

				if (!(apidx >= pAd->ApCfg.BssidNum) &&
					(pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
					(transmit->field.MODE == MODE_CCK) &&
					(transmit->field.field.MCS == RATE_1))
					TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;

#endif /* SPECIFIC_TX_POWER_SUPPORT */
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

	if (pMacEntry) {
#ifdef SW_CONNECT_SUPPORT
		wcid = pMacEntry->wcid;
		hw_wcid = pMacEntry->hw_wcid;
#else /* SW_CONNECT_SUPPORT */
		hw_wcid = wcid = pMacEntry->wcid;
#endif /* !SW_CONNECT_SUPPORT */
	} else {
		hw_wcid = wcid = 0;
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pMacEntry is null !!\n");
	}

	tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;

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
	if (pAd->CommonCfg.bSeOff != TRUE) {
		if (HcGetBandByWdev(wdev) == BAND0)
			mac_info.AntPri = BAND0_SPE_IDX;
		else if (HcGetBandByWdev(wdev) == BAND1)
			mac_info.AntPri = BAND1_SPE_IDX;
	}

	mac_info.OmacIdx = wdev->OmacIdx;
	mac_info.Type = pHeader_802_11->FC.Type;
	mac_info.SubType = pHeader_802_11->FC.SubType;
	mac_info.Length = (tx_blk->SrcBufLen - tx_hw_hdr_len);

	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;

		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;

		mac_info.PID = PID_MGMT;

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

#ifdef DOT11W_PMF_SUPPORT
		PMF_PerformTxFrameAction(pAd, pHeader_802_11, tx_blk->SrcBufLen, tx_hw_hdr_len, &mac_info.prot);
#endif
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
			mac_info.q_idx = WMM_UP2AC_MAP[pBar->BarControl.TID];
			if (
				(WMODE_CAP_5G(wdev->PhyMode))
#ifdef GN_MIXMODE_SUPPORT
				|| (pAd->CommonCfg.GNMixMode
				    && (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
					|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
					|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G))))
#endif /*GN_MIXMODE_SUPPORT*/
			) {
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
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
			if (wdev->eap.eap_mgmrate_en == TRUE)
				transmit = &wdev->eap.mgmphymode;
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					 " FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
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
#ifdef APCLI_SUPPORT

	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA)
		&& ((pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL) || (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))) {
		if ((pMacEntry != NULL) && (IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry))) {
			/* CURRENT_BW_TX_CNT/CURRENT_BW_FAIL_CNT only count for aute rate */
			mac_info.IsAutoRate = TRUE;
		}
	}

#endif /* APCLI_SUPPORT */

	if (wdev) {
		if (WMODE_CAP_5G(wdev->PhyMode)) {
			if (transmit->field.MODE == MODE_CCK) {
				/*
				    something wrong with rate->MlmeTransmit
					correct with OFDM mode
				*/
				transmit->field.MODE = MODE_OFDM;
				MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "@@@@ FIXME!!frame(Type=%d, SubType=%d) use the CCK RATE but wdev support A band only, mac_info.Length=%lu, mac_info.wmm_set=%d, mac_info.q_idx=%d, mac_info.OmacIdx=%d\n",
						 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType, mac_info.Length, mac_info.wmm_set, mac_info.q_idx, mac_info.OmacIdx);
			}
		}
	}
#ifdef TXRX_STAT_SUPPORT
	{
		UINT32 bandidx = HcGetBandByWdev(wdev);
		if (!(scan_in_run_state(pAd, wdev)) && pMacEntry && IS_ENTRY_CLIENT(pMacEntry) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
			UCHAR band_idx = HcGetBandByWdev(pMacEntry->wdev);
			struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
			INC_COUNTER64(pMacEntry->TxMgmtPacketCount);
			INC_COUNTER64(pMacEntry->pMbss->stat_bss.TxMgmtPacketCount);
			INC_COUNTER64(ctrl->rdev[band_idx].pRadioCtrl->TxMgmtPacketCount);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, " On Channel ::  ap_mlme_dataq_tx ::	Tx Mgmt Subtype : %d\n", pHeader_802_11->FC.SubType);
		}
		if ((scan_in_run_state(pAd, wdev)) && (pAd->ScanCtrl[bandidx].ScanType == SCAN_ACTIVE) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT) && wdev) {
			BSS_STRUCT *mbss = NULL;
			mbss = &pAd->ApCfg.MBSSID[wdev->wdev_idx];
			INC_COUNTER64(mbss->stat_bss.TxMgmtOffChPktCount);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, " Off Channel ::	ap_mlme_dataq_tx ::  Tx Mgmt Subtype : %d\n", pHeader_802_11->FC.SubType);
		}
	}
#endif
#ifdef MGMT_TXPWR_CTRL
		if ((wdev->bPwrCtrlEn == TRUE) && (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP)
			&& (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
			mac_info.WCID = wdev->tr_tb_idx;
			mac_info.IsAutoRate = TRUE;
		}
#endif

	if ((RTMP_GET_PACKET_TYPE(tx_blk->pPacket) != TX_ALTX) && (mac_info.WCID == 0)) {
		enum PACKET_TYPE pkt_type_old = RTMP_GET_PACKET_TYPE(tx_blk->pPacket);
		UCHAR q_idx_old = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
		struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
		struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;

		RTMP_SET_PACKET_TYPE(tx_blk->pPacket, TX_ALTX);
		mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
		tr_cnt->pkt_invalid_wcid++;

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pkt_t(%d),type(%d),sub_type(%d),q_idx(%d,%d),cnt(%d)\n",
			pkt_type_old, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType,
			q_idx_old, mac_info.q_idx, tr_cnt->pkt_invalid_wcid);

		if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) {
			PFRAME_ACTION_HDR Frame = (PFRAME_ACTION_HDR)pHeader_802_11;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cat(%d),act(%d)\n",
				Frame->Category, Frame->Action);
		}
	}

	return asic_mlme_hw_tx(pAd, tmac_info, &mac_info, transmit, tx_blk);
}

INT ap_ampdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr = NULL, *src_ptr;
	USHORT freeCnt = 1;
	BOOLEAN bVLANPkt;
	MAC_TABLE_ENTRY *pMacEntry;
	STA_TR_ENTRY *tr_entry;
#ifndef MT_MAC
	BOOLEAN bHTCPlus = FALSE;
#endif /* MT_MAC */
	UINT hdr_offset, cache_sz = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 TXWISize = cap->TXWISize;
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;

	if (!fill_tx_blk(pAd, wdev, pTxBlk)) {
		ap_tx_drop_update(pAd, wdev, pTxBlk);
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = pTxBlk->tr_entry;

	if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
		if (IS_HIF_TYPE(pAd, HIF_MT))
			hdr_offset = tx_hw_hdr_len;
		else
			hdr_offset = TXINFO_SIZE + TXWISize + TSO_SIZE;

		if ((tr_entry->isCached)) {
#ifndef VENDOR_FEATURE1_SUPPORT
			NdisMoveMemory((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]),
						   (UCHAR *)(&tr_entry->CachedBuf[0]),
						   TXWISize + sizeof(HEADER_802_11));
#else
			pTxBlk->HeaderBuf = (UCHAR *)(tr_entry->HeaderBuf);
#endif /* VENDOR_FEATURE1_SUPPORT */
			pHeaderBufPtr = (UCHAR *)(&pTxBlk->HeaderBuf[hdr_offset]);
			ap_build_cache_802_11_header(pAd, pTxBlk, pHeaderBufPtr);
#ifdef SOFT_ENCRYPT
			RTMPUpdateSwCacheCipherInfo(pAd, pTxBlk, pHeaderBufPtr);
#endif /* SOFT_ENCRYPT */
		} else {
			ap_build_802_11_header(pAd, pTxBlk);
			pHeaderBufPtr = &pTxBlk->HeaderBuf[hdr_offset];
		}

#ifdef SOFT_ENCRYPT

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
			if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
				RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}

#endif /* SOFT_ENCRYPT */
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
#ifdef UAPSD_SUPPORT

			if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
				&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
			   ) {
				/*
				 * we can not use bMoreData bit to get EOSP bit because
				 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
				 */
				if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
					*pHeaderBufPtr |= (1 << 4);
			}

#endif /* UAPSD_SUPPORT */
			pTxBlk->MpduHeaderLen = tr_entry->MpduHeaderLen;
			pTxBlk->wifi_hdr_len = tr_entry->wifi_hdr_len;
			pHeaderBufPtr = ((UCHAR *)wifi_hdr) + pTxBlk->MpduHeaderLen;
			pTxBlk->HdrPadLen = tr_entry->HdrPadLen;
			/* skip 802.3 header */
			pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
			pTxBlk->SrcBufLen -= LENGTH_802_3;
			/* skip vlan tag */
			bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

			if (bVLANPkt) {
				pTxBlk->pSrcBufData += LENGTH_802_1Q;
				pTxBlk->SrcBufLen -= LENGTH_802_1Q;
			}
		} else
#endif /* VENDOR_FEATURE1_SUPPORT */
		{
			/* build QOS Control bytes */
			*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT

			if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
				&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
			   ) {
				/*
				 * we can not use bMoreData bit to get EOSP bit because
				 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
				 */
				if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
					*pHeaderBufPtr |= (1 << 4);
			}

#endif /* UAPSD_SUPPORT */
			*(pHeaderBufPtr + 1) = 0;
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += 2;
			pTxBlk->wifi_hdr_len += 2;
#ifndef MT_MAC

			/* TODO: Shiang-usw, we need a more proper way to handle this instead of ifndef MT_MAC ! */
			/* For MT_MAC, SW not to prepare the HTC field for RDG enable */
			/* build HTC control field after QoS field */
			if ((pAd->CommonCfg.bRdg == TRUE)
				&& (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE))) {
				NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
				((PHT_CONTROL)pHeaderBufPtr)->RDG = 1;
				bHTCPlus = TRUE;
			}

			if (bHTCPlus == TRUE) {
				wifi_hdr->FC.Order = 1;
				pHeaderBufPtr += 4;
				pTxBlk->MpduHeaderLen += 4;
				pTxBlk->wifi_hdr_len += 4;
			}
#endif /* MT_MAC */

			/*pTxBlk->MpduHeaderLen = pHeaderBufPtr - pTxBlk->HeaderBuf - TXWI_SIZE - TXINFO_SIZE; */
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
			   padding at front of LLC header
			   LLC header should locate at 4-octets aligment

			   @@@ MpduHeaderLen excluding padding @@@
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
					Insert LLC-SNAP encapsulation - 8 octets
					if original Ethernet frame contains no LLC/SNAP,
					then an extra LLC/SNAP encap is required
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

		if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
			cache_sz = (pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]));
			src_ptr = (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]);
			NdisMoveMemory((UCHAR *)(&tr_entry->CachedBuf[0]), src_ptr, cache_sz);
		}

#ifdef VENDOR_FEATURE1_SUPPORT
		/* use space to get performance enhancement */
		NdisZeroMemory((UCHAR *)(&tr_entry->HeaderBuf[0]), sizeof(tr_entry->HeaderBuf));
		NdisMoveMemory((UCHAR *)(&tr_entry->HeaderBuf[0]),
					   (UCHAR *)(&pTxBlk->HeaderBuf[0]),
					   (pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[0])));
#endif /* VENDOR_FEATURE1_SUPPORT */
	}
	ap_tx_ok_update(pAd, wdev, pTxBlk);
	asic_write_tx_resource(pAd, pTxBlk, TRUE, &freeCnt);
#ifdef SMART_ANTENNA

	if (pMacEntry)
		pMacEntry->saTxCnt++;

#endif /* SMART_ANTENNA */
	return NDIS_STATUS_SUCCESS;
}

INT ap_amsdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;
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
			ap_tx_drop_update(pAd, wdev, tx_blk);
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS))
			wdev_ops->ieee_802_3_data_tx(pAd, wdev, tx_blk);
		else
			wdev_ops->ieee_802_11_data_tx(pAd, wdev, tx_blk);

		ap_tx_ok_update(pAd, wdev, tx_blk);
		ret = asic_hw_tx(pAd, tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

		tx_blk->frame_idx++;
		index++;
	}

	return NDIS_STATUS_SUCCESS;
}

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
static const UINT8 ac_queue_to_up[4] = {
	1 /* AC_BK */, 0 /* AC_BE */, 5 /* AC_VI */, 7 /* AC_VO */
};

BOOLEAN vow_is_queue_full(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR qidx)
{
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	"STA %d, qidx %d, vow_queue_len %d\n", wcid, qidx, pAd->vow_queue_len[wcid][qidx]);
	if ((pAd->vow_q_len > 0) && (pAd->vow_queue_len[wcid][qidx] >= pAd->vow_q_len))
		return TRUE;
	else
		return FALSE;
}

UINT32 vow_clone_legacy_frame(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UINT32 i, num;
	UINT32 resource_idx = 0, band = 0;
	/*PNDIS_PACKET pkt;*/
	TX_BLK txb, *pTemp_TxBlk;
	struct wifi_dev *wdev = pTxBlk->wdev;
	UINT32 KickRingBitMap = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	static UINT32 last_wcid = 1;
	UINT8 *pData;

	pTemp_TxBlk = &txb;

	/* return BC/MC */
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME) {
		UCHAR wmm_set = 0;

		if (wdev) {
			wmm_set = HcGetWmmIdx(pAd, wdev);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wcid %d, wmm set %d\n", pTxBlk->Wcid, wmm_set);
		} else {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wcid %d, wdev is null!\n", pTxBlk->Wcid);
		}

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[32mbc/mc packet ........ wcid %d\x1b[m\n", pTxBlk->Wcid);


		return KickRingBitMap;
	}

	/*pkt = DuplicatePacket(wdev->if_dev, pTxBlk->pPacket);*/
	/* backup TXBLK */
	os_move_mem(&txb, pTxBlk, sizeof(TX_BLK));


	/*if (pkt == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: DuplicatePacket failed!!\n", __func__));
		return;
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: clone 1 pkt %p, vow_cloned_wtbl_num %d, pkt %p, clone pkt %p\n",
		__func__, pkt, pAd->vow_cloned_wtbl_max, pTxBlk->pPacket, pkt));*/

	if (pAd->CommonCfg.dbdc_mode)
		band = WMODE_CAP_5G(wdev->PhyMode) ? 1 : 0;
	else
		band = 0;
	if (pAd->vow_cloned_wtbl_max[band]) {
		UINT32 end, start;
		struct wifi_dev *wdev = pTxBlk->wdev;
		UCHAR wmm_set;

		if (wdev) {
			wmm_set = HcGetWmmIdx(pAd, wdev);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"wcid %d, wmm set %d\n", pTxBlk->Wcid, wmm_set);
		} else {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wcid %d, wdev is null!\n", pTxBlk->Wcid);
		}

		start = pAd->vow_cloned_wtbl_start[band];
		end = pAd->vow_cloned_wtbl_max[band];

		last_wcid = (last_wcid + 1) % WTBL_MAX_NUM(pAd);
		if ((last_wcid < start) || (last_wcid > end))
			last_wcid = start;

		i = last_wcid;
		num = 0;
		do {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"sta%d, tx_en %d\n", i, pAd->vow_tx_en[i]);

			/* check queue status */
			if (vow_is_queue_full(pAd, i, pAd->vow_sta_ac[i])) {
				/*printk("\x1b[31m%s: full wcid %d, ac %d\x1b[m\n",
					__FUNCTION__, i,  pAd->vow_sta_ac[i]);*/
				goto NEXT;
			}
			if (i == pTxBlk->Wcid)
				goto NEXT;

			resource_idx = hif_get_resource_idx(pAd->hdev_ctrl, wdev, TX_DATA,
								pAd->vow_sta_ac[i]);
			if (pAd->vow_tx_en[i] && (hif_get_tx_resource_free_num(pAd->hdev_ctrl, 0) > 2)) {
				USHORT free_cnt = 1, tx_idx;
				/* clone packet */
				PNDIS_PACKET clone = DuplicatePacket(wdev->if_dev, pTxBlk->pPacket);

				if (clone == NULL) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"DuplicatePacket failed!!\n");
					goto CLONE_DONE;
				}

				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"sta%d, free no %d, clone pkt %p\n",
						i, hif_get_tx_resource_free_num(pAd->hdev_ctrl, 0), clone);
				/* cloned PKT */
				pTemp_TxBlk->pPacket = clone;

				/* pTxBlk->Wcid = 1; */
				pTemp_TxBlk->Wcid = i;
				pTemp_TxBlk->resource_idx = resource_idx;
				RTMP_SET_PACKET_QUEIDX(clone, pAd->vow_sta_ac[i]);
				RTMP_SET_PACKET_UP(clone, ac_queue_to_up[pAd->vow_sta_ac[i]]);
				pTemp_TxBlk->QueIdx = pAd->vow_sta_ac[i];

				/* AC to TOS mapping for WACPU set TxD q_idx */
				pData = GET_OS_PKT_DATAPTR(clone);
				(*(pData+MAT_ETHER_HDR_LEN+1)) &= ~0x0E0;
				(*(pData+MAT_ETHER_HDR_LEN+1)) |= ac_queue_to_up[pAd->vow_sta_ac[i]] << 5;


				/* get MAC TXD buffer */
				pTemp_TxBlk->HeaderBuf = hif_get_tx_buf(pAd->hdev_ctrl,
							pTemp_TxBlk, pTemp_TxBlk->resource_idx,
							pTemp_TxBlk->TxFrameType);

				/* modified DA */
				if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
					HEADER_802_11 *hdr = (HEADER_802_11 *)pTemp_TxBlk->wifi_hdr;

					hdr->Addr1[4] = i;
				} else if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
					INT32 ret = NDIS_STATUS_SUCCESS;
					/* modified DA */
					/* fill TXD */
					if ((pTxBlk->amsdu_state == TX_AMSDU_ID_NO) ||
						(pTxBlk->amsdu_state == TX_AMSDU_ID_LAST))
						asic_write_tmac_info(pAd, &pTemp_TxBlk->HeaderBuf[0], pTemp_TxBlk);

					/* fill TXP in TXD */
					ret = asic_write_txp_info(
						pAd, &pTemp_TxBlk->HeaderBuf[cap->tx_hw_hdr_len], pTemp_TxBlk);
					if (ret != NDIS_STATUS_SUCCESS) {
						RELEASE_NDIS_PACKET(pAd, clone, NDIS_STATUS_FAILURE);
						goto NEXT;
					}
					/* fill DMAD */
					if ((pTxBlk->amsdu_state == TX_AMSDU_ID_NO) ||
						(pTxBlk->amsdu_state == TX_AMSDU_ID_LAST))
						tx_idx = asic_write_tx_resource(pAd, pTemp_TxBlk,
										TRUE, &free_cnt);

					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"sta%d, tx idx %d, resource_idx %d\n",
						 i, tx_idx, pTemp_TxBlk->resource_idx);
				}
			}
			KickRingBitMap |= (1 << pTemp_TxBlk->resource_idx);
NEXT:
			num++;
			i++;
			if (i > end)
				i = start;
		} while (num <= (end-start+1));
	}

CLONE_DONE:
	last_wcid = i;
	/* release original pkt */
	if (pAd->vow_need_drop_cnt[pTxBlk->Wcid] > 0) {
		/*printk("\x1b[31m%s release....wcid %d, drop cnt %d\n\x1b[m\n", __FUNCTION__,
				pTxBlk->Wcid, pAd->vow_need_drop_cnt[pTxBlk->Wcid]);*/
		pAd->vow_need_drop_cnt[pTxBlk->Wcid]--;
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_SUCCESS);
	}

	return KickRingBitMap;
}
#endif /* defined(VOW_SUPPORT) && (defined(VOW_DVT) */

VOID ap_ieee_802_11_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr;
	BOOLEAN bVLANPkt;
#ifdef TXBF_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	ap_build_802_11_header(pAd, pTxBlk);
#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}

#endif /* SOFT_ENCRYPT */
	/* skip 802.3 header */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen -= LENGTH_802_3;
	/* skip vlan tag */
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

	if (bVLANPkt) {
		pTxBlk->pSrcBufData += LENGTH_802_1Q;
		pTxBlk->SrcBufLen -= LENGTH_802_1Q;
	}

	/* record these MCAST_TX frames for group key rekey */
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME) {
		INT idx;
#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters[0].MulticastTransmittedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			if (pAd->ApCfg.MBSSID[idx].WPAREKEY.ReKeyMethod == PKT_REKEY)
				pAd->ApCfg.MBSSID[idx].REKEYCOUNTER += (pTxBlk->SrcBufLen);

		}
	}

#ifdef MT_MAC
	else {
		/* Unicast */
		if (pTxBlk->tr_entry && pTxBlk->tr_entry->PsDeQWaitCnt)
			pTxBlk->Pid = PID_PS_DATA;
	}

#endif /* MT_MAC */
	pHeaderBufPtr = pTxBlk->wifi_hdr;
	wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;
	/* skip common header */
	pHeaderBufPtr += pTxBlk->wifi_hdr_len;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		struct wifi_dev *wdev_wmm = NULL;
		UCHAR ack_policy = pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx];

		wdev_wmm = pTxBlk->wdev;
		if (wdev_wmm) {
			ack_policy = wlan_config_get_ack_policy(wdev_wmm, pTxBlk->QueIdx);
		}
		/* build QOS Control bytes */
		*pHeaderBufPtr = ((pTxBlk->UserPriority & 0x0F) | (ack_policy << 5));
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		*pHeaderBufPtr |= (pAd->vow_sta_ack[pTxBlk->Wcid] << 5);
#endif /* defined(VOW_SUPPORT) && (defined(VOW_DVT) */

#ifdef UAPSD_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
		   ) {
			/*
				we can not use bMoreData bit to get EOSP bit because
				maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}

#endif /* UAPSD_SUPPORT */
		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->wifi_hdr_len += 2;


#ifdef TXBF_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "tx_bf: %d\n",
							 cap->FlgHwTxBfCap);
#endif /* TXBF_SUPPORT */
	}

	/* The remaining content of MPDU header should locate at 4-octets aligment */
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);
	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;
#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		tx_sw_encrypt(pAd, pTxBlk, pHeaderBufPtr, wifi_hdr);
	else
#endif /* SOFT_ENCRYPT */
	{
		/*
			Insert LLC-SNAP encapsulation - 8 octets
			if original Ethernet frame contains no LLC/SNAP,
			then an extra LLC/SNAP encap is required
		*/
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
										   pTxBlk->pExtraLlcSnapEncap);

		if (pTxBlk->pExtraLlcSnapEncap) {
			UCHAR vlan_size;

			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* skip vlan tag */
			vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr,
						   pTxBlk->pSrcBufHeader + 12 + vlan_size,
						   2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}
	}
}

VOID ap_ieee_802_3_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	pTxBlk->MpduHeaderLen = 0;
	pTxBlk->HdrPadLen = 0;
	pTxBlk->wifi_hdr_len = 0;
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
}

INT ap_legacy_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;

	if (!fill_tx_blk(pAd, wdev, tx_blk)) {
		ap_tx_drop_update(pAd, wdev, tx_blk);
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS))
		wdev_ops->ieee_802_3_data_tx(pAd, wdev, tx_blk);
	else
		wdev_ops->ieee_802_11_data_tx(pAd, wdev, tx_blk);

	ap_tx_ok_update(pAd, wdev, tx_blk);

	ret = asic_hw_tx(pAd, tx_blk);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

#ifdef IXIA_C50_MODE
	if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(tx_blk->pPacket)))
		pAd->tx_cnt.tx_pkt_to_hw[TX_LEGACY][smp_processor_id()]++;
#endif

	return NDIS_STATUS_SUCCESS;
}

INT ap_frag_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	HEADER_802_11 *wifi_hdr;
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
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

#ifdef SW_CONNECT_SUPPORT
	if (IS_SW_STA_ENABLED(pAd)) {
		if ((tx_blk->pPacket) && RTMP_GET_PACKET_SW(tx_blk->pPacket)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "drop !!! S/W STA not support frag !!! wdev->wdev_idx=%d\n", wdev->wdev_idx);
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}
	}
#endif /* SW_CONNECT_SUPPORT */

	if (!fill_tx_blk(pAd, wdev, tx_blk)) {
		ap_tx_drop_update(pAd, wdev, tx_blk);
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--(%d): ##########Fail#########\n", __LINE__);
		return NDIS_STATUS_FAILURE;
	}

#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, tx_blk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}
	}

#endif /* SOFT_ENCRYPT */

	if (IS_CIPHER_TKIP(tx_blk->CipherAlg)) {
		tx_blk->pPacket = duplicate_pkt_with_TKIP_MIC(pAd, tx_blk->pPacket);

		if (tx_blk->pPacket == NULL)
			return NDIS_STATUS_FAILURE;

		tx_blk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(tx_blk->pPacket);
		tx_blk->SrcBufLen = RTMP_GET_PKT_LEN(tx_blk->pPacket);
	}

	ops->ieee_802_11_data_tx(pAd, wdev, tx_blk);

	/*  1. If TKIP is used and fragmentation is required. Driver has to
		   append TKIP MIC at tail of the scatter buffer
		2. When TXWI->FRAG is set as 1 in TKIP mode,
		   MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC */
	/*  TKIP appends the computed MIC to the MSDU data prior to fragmentation into MPDUs. */
	if (IS_CIPHER_TKIP(tx_blk->CipherAlg)) {
		RTMPCalculateMICValue(pAd, tx_blk->pPacket, tx_blk->pExtraLlcSnapEncap, tx_blk->pKey->Key, &tx_blk->pKey->Key[LEN_TK], wdev->func_idx);
		/*
			NOTE: DON'T refer the skb->len directly after following copy. Becasue the length is not adjust
				to correct lenght, refer to tx_blk->SrcBufLen for the packet length in following progress.
		*/
		NdisMoveMemory(tx_blk->pSrcBufData + tx_blk->SrcBufLen, &pAd->PrivateInfo.Tx.MIC[0], 8);
		tx_blk->SrcBufLen += 8;
		tx_blk->TotalFrameLen += 8;
	}
	ap_tx_ok_update(pAd, wdev, tx_blk);

	/*
		calcuate the overhead bytes that encryption algorithm may add. This
		affects the calculate of "duration" field
	*/
	if ((tx_blk->CipherAlg == CIPHER_WEP64) || (tx_blk->CipherAlg == CIPHER_WEP128) || (tx_blk->CipherAlg == CIPHER_WEP152))
		EncryptionOverhead = 8; /* WEP: IV[4] + ICV[4]; */
	else if (tx_blk->CipherAlg == CIPHER_TKIP)
		EncryptionOverhead = 12; /* TKIP: IV[4] + EIV[4] + ICV[4], MIC will be added to TotalPacketLength */
	else if (tx_blk->CipherAlg == CIPHER_AES)
		EncryptionOverhead = 16;	/* AES: IV[4] + EIV[4] + MIC[8] */

	else
		EncryptionOverhead = 0;

	pTransmit = tx_blk->pTransmit;

	/* Decide the TX rate */
	if (pTransmit->field.MODE == MODE_CCK)
		tx_blk->TxRate = pTransmit->field.MCS;
	else if (pTransmit->field.MODE == MODE_OFDM)
		tx_blk->TxRate = pTransmit->field.MCS + RATE_FIRST_OFDM_RATE;
	else
		tx_blk->TxRate = RATE_6_5;

	/* decide how much time an ACK/CTS frame will consume in the air */
	if (tx_blk->TxRate <= RATE_LAST_OFDM_RATE)
		AckDuration = RTMPCalcDuration(pAd, pAd->CommonCfg.ExpectedACKRate[tx_blk->TxRate], 14);
	else
		AckDuration = RTMPCalcDuration(pAd, RATE_6_5, 14);

	/*MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, ("!!!Fragment AckDuration(%d), TxRate(%d)!!!\n", AckDuration, tx_blk->TxRate)); */
#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwEncrypt)) {
		/* store the outgoing frame for calculating MIC per fragmented frame */
		os_alloc_mem(pAd, (PUCHAR *)&tmp_ptr, tx_blk->SrcBufLen);

		if (tmp_ptr == NULL) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "no memory for MIC calculation!\n");
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}

		NdisMoveMemory(tmp_ptr, tx_blk->pSrcBufData, tx_blk->SrcBufLen);
	}

#endif /* SOFT_ENCRYPT */
	/* Init the total payload length of this frame. */
	SrcRemainingBytes = tx_blk->SrcBufLen;
	tx_blk->TotalFragNum = 0xff;
	wifi_hdr = (HEADER_802_11 *)tx_blk->wifi_hdr;

	do {
		FreeMpduSize = wlan_operate_get_frag_thld(wdev);
		FreeMpduSize -= LENGTH_CRC;
		FreeMpduSize -= tx_blk->MpduHeaderLen;

#ifdef AUTOMATION
		if (pAd->fpga_ctl.txrx_dbg_type == 3) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[%d]FreeMpduSize=%d\n\r", __LINE__, FreeMpduSize);
		}
#endif /* AUTOMATION */
		if (SrcRemainingBytes <= FreeMpduSize) {
			/* This is the last or only fragment */
			tx_blk->SrcBufLen = SrcRemainingBytes;
			wifi_hdr->FC.MoreFrag = 0;
			wifi_hdr->Duration = pAd->CommonCfg.Dsifs + AckDuration;
			/* Indicate the lower layer that this's the last fragment. */
			tx_blk->TotalFragNum = fragNum;
#ifdef MT_MAC
			tx_blk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_NO : TX_FRAG_ID_LAST);
#endif /* MT_MAC */
		} else {
			/* more fragment is required */
			tx_blk->SrcBufLen = FreeMpduSize;
			NextMpduSize = min(((UINT)SrcRemainingBytes - tx_blk->SrcBufLen),
							   ((UINT)wlan_operate_get_frag_thld(wdev)));
			wifi_hdr->FC.MoreFrag = 1;
			wifi_hdr->Duration = (3 * pAd->CommonCfg.Dsifs) + (2 * AckDuration) +
								 RTMPCalcDuration(pAd, tx_blk->TxRate, NextMpduSize + EncryptionOverhead);
#ifdef MT_MAC
			tx_blk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_FIRST : TX_FRAG_ID_MIDDLE);
#endif /* MT_MAC */
		}

		SrcRemainingBytes -= tx_blk->SrcBufLen;

		if (fragNum == 0)
			tx_blk->FrameGap = IFS_HTTXOP;
		else
			tx_blk->FrameGap = IFS_SIFS;

#ifdef SOFT_ENCRYPT

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwEncrypt)) {
			UCHAR ext_offset = 0;

			NdisMoveMemory(tx_blk->pSrcBufData, tmp_ptr + buf_offset, tx_blk->SrcBufLen);
			buf_offset += tx_blk->SrcBufLen;
			/* Encrypt the MPDU data by software */
			RTMPSoftEncryptionAction(pAd,
									 tx_blk->CipherAlg,
									 (UCHAR *)wifi_hdr,
									 tx_blk->pSrcBufData,
									 tx_blk->SrcBufLen,
									 tx_blk->KeyIdx,
									 tx_blk->pKey,
									 &ext_offset);
			tx_blk->SrcBufLen += ext_offset;
			tx_blk->TotalFrameLen += ext_offset;
		}

#endif /* SOFT_ENCRYPT */
		ret = asic_hw_tx(pAd, tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

#ifdef SMART_ANTENNA

		if (tx_blk->pMacEntry)
			tx_blk->pMacEntry->saTxCnt++;

#endif /* SMART_ANTENNA */
#ifdef SOFT_ENCRYPT

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwEncrypt)) {
				if ((tx_blk->CipherAlg == CIPHER_WEP64) || (tx_blk->CipherAlg == CIPHER_WEP128)) {
					inc_iv_byte(tx_blk->pKey->TxTsc, LEN_WEP_TSC, 1);
					/* Construct and insert 4-bytes WEP IV header to MPDU header */
					RTMPConstructWEPIVHdr(tx_blk->KeyIdx, tx_blk->pKey->TxTsc,
										  pHeaderBufPtr - (LEN_WEP_IV_HDR));
				} else if (tx_blk->CipherAlg == CIPHER_TKIP)
					;
				else if (tx_blk->CipherAlg == CIPHER_AES) {
					inc_iv_byte(tx_blk->pKey->TxTsc, LEN_WPA_TSC, 1);
					/* Construct and insert 8-bytes CCMP header to MPDU header */
					RTMPConstructCCMPHdr(tx_blk->KeyIdx, tx_blk->pKey->TxTsc,
										 pHeaderBufPtr - (LEN_CCMP_HDR));
				}
		} else
#endif /* SOFT_ENCRYPT */
		{
			/* Update the frame number, remaining size of the NDIS packet payload. */
			if (fragNum == 0 && tx_blk->pExtraLlcSnapEncap)
				tx_blk->MpduHeaderLen -= LENGTH_802_1_H;	/* space for 802.11 header. */
		}

		fragNum++;
		/* SrcRemainingBytes -= tx_blk->SrcBufLen; */
		tx_blk->pSrcBufData += tx_blk->SrcBufLen;
		wifi_hdr->Frag++;	/* increase Frag # */
	} while (SrcRemainingBytes > 0);

#ifdef SOFT_ENCRYPT

	if (tmp_ptr != NULL)
		os_free_mem(tmp_ptr);

#endif /* SOFT_ENCRYPT */
	return NDIS_STATUS_SUCCESS;
}

INT ap_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	PQUEUE_ENTRY pQEntry;
	PNDIS_PACKET pPacket = NULL;
	struct wifi_dev_ops *ops = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct DOT11_H *pDot11h = NULL;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#ifdef SW_CONNECT_SUPPORT
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#endif /* SW_CONNECT_SUPPORT */
	UINT16 wcid = RTMP_GET_PACKET_WCID(tx_blk->pPacket);

	if (RTMP_GET_PACKET_WCID(tx_blk->pPacket) != 0)

	if (!wdev) {
		tr_cnt->wdev_null_drop++;
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	ops = wdev->wdev_ops;
	pDot11h = wdev->pDot11_H;

	if (pDot11h == NULL) {
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if ((pDot11h->RDMode != RD_NORMAL_MODE)
#ifdef CARRIER_DETECTION_SUPPORT
		|| (isCarrierDetectExist(pAd) == TRUE)
#endif /* CARRIER_DETECTION_SUPPORT */
	   ) {
#ifdef	CONFIG_RCSA_SUPPORT
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
		UCHAR *pData = GET_OS_PKT_DATAPTR(tx_blk->pPacket);
		HEADER_802_11 *pHeader_802_11 = (HEADER_802_11 *)(pData + tx_hw_hdr_len);

		if (!(tx_blk->TxFrameType == TX_MLME_MGMTQ_FRAME &&
			pHeader_802_11->FC.SubType == SUBTYPE_ACTION
			&& pHeader_802_11->Octet[0] == CATEGORY_SPECTRUM && pHeader_802_11->Octet[1] == SPEC_CHANNEL_SWITCH))
#endif
		{
#ifdef ZERO_LOSS_CSA_SUPPORT
			/*drop frame only if zero pkt loss not enabled*/
			if (!(pAd->Zero_Loss_Enable)) {
#endif /*ZERO_LOSS_CSA_SUPPORT*/
			if (tx_blk->TxFrameType == TX_AMSDU_FRAME) {
				while (tx_blk->TxPacketList.Head) {
					pQEntry = RemoveHeadQueue(&tx_blk->TxPacketList);
					pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

					if (pPacket) {
						tr_cnt->carrier_detect_drop++;
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					}
				}
			} else {
				RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
				tr_cnt->carrier_detect_drop++;
			}

			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "<--(%d)\n", __LINE__);
			return NDIS_STATUS_FAILURE;
#ifdef ZERO_LOSS_CSA_SUPPORT
			}
#endif /*ZERO_LOSS_CSA_SUPPORT*/
		}
	}
#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT
	if (IS_RRM_QUIET(wdev)) {
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */

	switch (tx_blk->TxFrameType) {
	case TX_AMPDU_FRAME:
		ret = ops->ampdu_tx(pAd, wdev, tx_blk);
		break;

	case TX_LEGACY_FRAME:
		ret = ops->legacy_tx(pAd, wdev, tx_blk);
		break;

	case TX_MCAST_FRAME:
		ret = ops->legacy_tx(pAd, wdev, tx_blk);
		break;

	case TX_AMSDU_FRAME:
		ret = ops->amsdu_tx(pAd, wdev, tx_blk);
		break;

	case TX_FRAG_FRAME:
		ret = ops->frag_tx(pAd, wdev, tx_blk);
		break;

	case TX_MLME_MGMTQ_FRAME:
		ret = ops->mlme_mgmtq_tx(pAd, wdev, tx_blk);
		break;

	case TX_MLME_DATAQ_FRAME:
		ret = ops->mlme_dataq_tx(pAd, wdev, tx_blk);
		break;

#ifdef VERIFICATION_MODE
	case TX_VERIFY_FRAME:
		ret = ops->verify_tx(pAd, wdev, tx_blk);
		break;
#endif

	default:
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Send a pacekt was not classified!!\n");
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		tr_cnt->tx_unknow_type_drop++;
		return NDIS_STATUS_FAILURE;
	}

	if (NDIS_STATUS_FAILURE == ret)
		goto end;


	if (IS_WCID_VALID(pAd, wcid)) {
#ifdef SW_CONNECT_SUPPORT
		STA_TR_ENTRY * tr_entry = &tr_ctl->tr_entry[wcid];
		tr_entry->tx_handle_cnt++;
#endif /* SW_CONNECT_SUPPORT */

		pEntry = &pAd->MacTab.Content[wcid];
#ifdef MT7626_REDUCE_TX_OVERHEAD
#else
		INC_COUNTER64(pEntry->TxPackets);
		pEntry->TxBytes += tx_blk->SrcBufLen;
#ifdef TR181_SUPPORT
		if (pEntry->pMbss) {
			pEntry->pMbss->TxCount++;
			pEntry->pMbss->TransmittedByteCount += tx_blk->SrcBufLen;
		}
#endif

#endif
		pEntry->OneSecTxBytes += tx_blk->SrcBufLen;
#ifdef ANTENNA_DIVERSITY_SUPPORT
		pEntry->ant_div_tx_bytes += tx_blk->SrcBufLen;
#endif
		pEntry->one_sec_tx_pkts++;
	}

end:
	return ret;
}

/*
    ==========================================================================
    Description:
	Some STA/AP
    Note:
	This action should never trigger AUTH state transition, therefore we
	separate it from AUTH state machine, and make it as a standalone service
    ==========================================================================
 */
VOID ap_cls2_err_action(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 Hdr;
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT Reason = REASON_CLS2ERR;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR apidx;

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = &(pAd->MacTab.Content[pRxBlk->wcid]);

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		/*ApLogEvent(pAd, pAddr, EVENT_DISASSOCIATED); */
#ifdef WIFI_DIAG
		diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_CLS2ERR);
#endif
		mac_entry_delete(pAd, pEntry);
	} else {
		apidx = get_apidx_by_addr(pAd, pRxBlk->Addr1);

		if (apidx >= pAd->ApCfg.BssidNum) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AUTH - Class 2 error but not my bssid "MACSTR"\n", MAC2STR(pRxBlk->Addr1));
			return;
		}
	}

	/* send out DEAUTH frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "AUTH - Class 2 error, Send DEAUTH frame to "MACSTR"\n",
			  MAC2STR(pRxBlk->Addr2));
	MgtMacHeaderInit(pAd, &Hdr, SUBTYPE_DEAUTH, 0, pRxBlk->Addr2,
					 pRxBlk->Addr1,
					 pRxBlk->Addr1);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &Hdr,
					  2, &Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
}


/*
    ==========================================================================
    Description:
	right part of IEEE 802.11/1999 page 374
    Note:
	This event should never cause ASSOC state machine perform state
	transition, and has no relationship with CNTL machine. So we separate
	this routine as a service outside of ASSOC state transition table.
    ==========================================================================
 */
VOID ap_cls3_err_action(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11         DisassocHdr;
	PUCHAR                pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	NDIS_STATUS           NStatus;
	USHORT                Reason = REASON_CLS3ERR;
	MAC_TABLE_ENTRY       *pEntry = NULL;

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = &(pAd->MacTab.Content[pRxBlk->wcid]);

	if (pEntry) {
#ifdef WIFI_DIAG
		if (IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				Reason);
#endif
		/*ApLogEvent(pAd, pAddr, EVENT_DISASSOCIATED); */
		mac_entry_delete(pAd, pEntry);
	}

	/* 2. send out a DISASSOC request frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ASSOC - Class 3 Error, Send DISASSOC frame to "MACSTR"\n",
			 MAC2STR(pRxBlk->Addr2));
	MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pRxBlk->Addr2,
					 pRxBlk->Addr1,
					 pRxBlk->Addr1);
	MakeOutgoingFrame(pOutBuffer,            &FrameLen,
					  sizeof(HEADER_802_11), &DisassocHdr,
					  2,                     &Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
}

/*
  ========================================================================
  Description:
	This routine checks if a received frame causes class 2 or class 3
	error, and perform error action (DEAUTH or DISASSOC) accordingly
  ========================================================================
*/
BOOLEAN ap_chk_cl2_cl3_err(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry;

	/* software MAC table might be smaller than ASIC on-chip total size. */
	/* If no mathed wcid index in ASIC on chip, do we need more check???  need to check again. 06-06-2006 */
	if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {

		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

#ifdef WTBL_TDD_SUPPORT
		if (IS_WTBL_TDD_ENABLED(pAd)) {
			pEntry = WtblTdd_InactiveList_Lookup(pAd, pRxBlk->Addr2);
		}
#endif /* WTBL_TDD_SUPPORT */

		if (pEntry)
			return FALSE;

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Rx a frame from "MACSTR" with WCID(%d) > %d\n",
				 MAC2STR(pRxBlk->Addr2),
				 pRxBlk->wcid, GET_MAX_UCAST_NUM(pAd));
		ap_cls2_err_action(pAd, pRxBlk);
		return TRUE;
	}

#ifdef WTBL_TDD_SUPPORT
	if (IS_WTBL_TDD_ENABLED(pAd)) {
		pEntry = WtblTdd_InactiveList_Lookup(pAd, pRxBlk->Addr2);
		if (pEntry)
			return FALSE;
	}
#endif /* WTBL_TDD_SUPPORT */

	if (pAd->MacTab.Content[pRxBlk->wcid].Sst == SST_ASSOC)
		/* okay to receive this DATA frame */
		return FALSE;
	else if (pAd->MacTab.Content[pRxBlk->wcid].Sst == SST_AUTH) {
		ap_cls3_err_action(pAd, pRxBlk);
		return TRUE;
	}
	ap_cls2_err_action(pAd, pRxBlk);
	return TRUE;
}

#ifdef RLT_MAC_DBG
static int dump_next_valid;
#endif /* RLT_MAC_DBG */
BOOLEAN ap_check_valid_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	BOOLEAN isVaild = FALSE;

	do {
		if (FC->ToDs == 0)
			break;

#ifdef IDS_SUPPORT

		if ((FC->FrDs == 0) && (pRxBlk->wcid == WCID_NO_MATCHED(pAd))) { /* not in RX WCID MAC table */
			if (++pAd->ApCfg.RcvdMaliciousDataCount > pAd->ApCfg.DataFloodThreshold)
				break;
		}

#endif /* IDS_SUPPORT */

		/* check if Class2 or 3 error */
		if ((FC->FrDs == 0) && (ap_chk_cl2_cl3_err(pAd, pRxBlk)))
			break;

		if (pAd->ApCfg.BANClass3Data == TRUE)
			break;

		isVaild = TRUE;
	} while (0);

	return isVaild;
}

INT ap_rx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT hdr_len = 0;
#if	defined(WDS_SUPPORT)
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#endif
	pEntry = PACInquiry(pAd, pRxBlk->wcid);
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(A4_CONN)

	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)) {
#ifdef CLIENT_WDS

		if (pEntry) {
			/* The CLIENT WDS must be a associated STA */
			if (IS_ENTRY_CLIWDS(pEntry))
				;
			else if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
				SET_ENTRY_CLIWDS(pEntry);
			else
				return FALSE;

			CliWds_ProxyTabUpdate(pAd, pEntry->wcid, pRxBlk->Addr4);
		}

#endif /* CLIENT_WDS */

#ifdef A4_CONN
		if (!pEntry)
			pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

		if (pEntry && (IS_ENTRY_A4(pEntry)
#ifdef MWDS
			|| ((pFmeCtrl->SubType & 0x08) && RTMPEqualMemory(EAPOL, pRxBlk->pData+LENGTH_802_11_WITH_ADDR4+LENGTH_WMMQOS_H+6, 2))
#endif
		) ) {
			MAC_TABLE_ENTRY *pMovedEntry = NULL;
			UINT16 ProtoType = 0;
			UINT32 ARPSenderIP = 0;
			UCHAR *Pos = (pRxBlk->pData + 12);
			BOOLEAN bTAMatchSA = MAC_ADDR_EQUAL(pEntry->Addr, pRxBlk->Addr4);
#ifdef MWDS
			pEntry->MWDSInfo.Addr4PktNum++;
#endif
			/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("APRxPktAllow: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x Recvd MWDS Pkt\n",
				pEntry->wdev->wdev_idx,pEntry->wdev->wdev_type,pEntry->wdev->func_idx);
			*/

			/* if ((((PUCHAR)pRxBlk->pData)[4])& 0x1 == 0x1)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("APRxPktAllow: MWDS Pkt=> wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x\nEth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
				pEntry->wdev->wdev_idx,pEntry->wdev->wdev_type,pEntry->wdev->func_idx,
				((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],
				((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13],((PUCHAR)pRxBlk->pData)[14],((PUCHAR)pRxBlk->pData)[15],
				((PUCHAR)pRxBlk->pData)[16],((PUCHAR)pRxBlk->pData)[17]));
			}*/

			ProtoType = OS_NTOHS(*((UINT16 *)Pos));
			if (ProtoType == 0x0806) /* ETH_P_ARP */
				NdisCopyMemory(&ARPSenderIP, (Pos + 16), 4);

			/*
			   It means this source entry has moved to another one and hidden behind it.
			   So delete this source entry!
			*/
			if (!bTAMatchSA) { /* TA isn't same with SA case*/
				pMovedEntry = MacTableLookup(pAd, pRxBlk->Addr4);
				if (pMovedEntry
#ifdef AIR_MONITOR
					&& !IS_ENTRY_MONITOR(pMovedEntry)
#endif /* AIR_MONITOR */
					&& IS_ENTRY_CLIENT(pMovedEntry)
			) {
				/*
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("APRxPktAllow: AP found a entry(%02X:%02X:%02X:%02X:%02X:%02X) who has moved to another side! Delete it from MAC table.\n",
				PRINT_MAC(pMovedEntry->Addr)));
				*/

#ifdef WH_EVENT_NOTIFIER
				{
					EventHdlr pEventHdlrHook = NULL;

					pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
					if (pEventHdlrHook && pMovedEntry->wdev)
						pEventHdlrHook(pAd, pMovedEntry->wdev, pMovedEntry->Addr, pMovedEntry->wdev->channel);
				}
#endif /* WH_EVENT_NOTIFIER */
				if (ProtoType == ETH_TYPE_ARP || pMovedEntry->NoDataIdleCount > 1) {
#ifdef MAP_R2
					if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
						wapp_handle_sta_disassoc(pAd, pMovedEntry->wcid, REASON_DEAUTH_STA_LEAVING);
#endif
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "AP found a entry(%02X:%02X:%02X:%02X:%02X:%02X) moved to another dev! Delete it from MAC table.\n",
							PRINT_MAC(pMovedEntry->Addr));
					mac_entry_delete(pAd, pMovedEntry);
				}
			}
		}
			a4_proxy_update(pAd, pEntry->func_tb_idx, pEntry->wcid, pRxBlk->Addr4, ARPSenderIP);
		} else {
			if (pEntry != NULL) {
#if defined(CONFIG_BS_SUPPORT) || defined(CONFIG_MAP_SUPPORT)
				if (!IS_MAP_ENABLE(pAd)
#if defined(CONFIG_MAP_SUPPORT)
				|| ((pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)) == 0)
#endif
				)
#endif
#ifdef MWDS
				if (!pEntry->bSupportMWDS)
#endif
				pEntry = NULL;
			}
		}
#endif /* A4_CONN */

#ifdef WDS_SUPPORT

		if (!pEntry) {
			struct wifi_dev *main_bss_wdev = NULL;

			if (pRxBlk->band == DBDC_BAND0)
				main_bss_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
			else
				main_bss_wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;
			/*
				The WDS frame only can go here when in auto learning mode and
				this is the first trigger frame from peer

				So we check if this is un-registered WDS entry by call function
					"FindWdsEntry()"
			*/
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "[band%d]RA:"MACSTR", TA:"MACSTR"\n",
				  pRxBlk->band, MAC2STR(pRxBlk->Addr1), MAC2STR(pRxBlk->Addr2));
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "\tA3:"MACSTR", A4:"MACSTR"\n",
				  MAC2STR(pRxBlk->Addr3), MAC2STR(pRxBlk->Addr4));

			if (MAC_ADDR_EQUAL(pRxBlk->Addr1, main_bss_wdev->if_addr))
				pEntry = FindWdsEntry(pAd, pRxBlk);
			else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "[band%d]WDS for RA "MACSTR" is not enabled!\n", pRxBlk->band,
					  MAC2STR(pRxBlk->Addr1));
				return FALSE;
			}

			/* have no valid wds entry exist, then discard the incoming packet.*/
			if (!(pEntry && WDS_IF_UP_CHECK(pAd, pRxBlk->band, pEntry->func_tb_idx))) {
				if (!pEntry)
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "[band%d]WDS dropped due to entry for "MACSTR" not found!\n",
						 pRxBlk->band, MAC2STR(pRxBlk->Addr2));
				else
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "[band%d]WDS dropped due to Entry[%d] not enabled!\n",
						 pRxBlk->band, pEntry->func_tb_idx);
				return FALSE;
			}

			/*receive corresponding WDS packet, disable TX lock state (fix WDS jam issue) */
			if (pEntry && (pEntry->LockEntryTx == TRUE)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[band%d]Receive WDS packet, disable TX lock state!\n", pRxBlk->band);
				pEntry->ContinueTxFailCnt = 0;
				pEntry->LockEntryTx = FALSE;
				/* TODO: shiang-usw, remove upper setting because we need to mirgate to tr_entry! */
				tr_ctl->tr_entry[pEntry->wcid].ContinueTxFailCnt = 0;
				tr_ctl->tr_entry[pEntry->wcid].LockEntryTx = FALSE;
			}
		} else if (!IS_ENTRY_WDS(pEntry)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[band%d]Receive 4-addr packet, but not from a WDS entry!\n", pRxBlk->band);
			/*return FALSE;*/
		}

#endif /* WDS_SUPPORT */

#ifndef WDS_SUPPORT
		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[band%d]WDS packet dropped due to entry not valid!\n", pRxBlk->band);
			return FALSE;
		}
#endif /* WDS_SUPPORT */

#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT
		if (IS_ENTRY_WDS(pEntry)) {
			RT_802_11_WDS_ENTRY *pWdsEntry = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx];

			pWdsEntry->WdsCounter.ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
			INC_COUNTER64(pWdsEntry->WdsCounter.ReceivedFragmentCount);

			if (IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
				INC_COUNTER64(pWdsEntry->WdsCounter.MulticastReceivedFrameCount);
		}
#endif /* STATS_COUNT_SUPPORT */
#endif /* WDS_SUPPORT */
		RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
		hdr_len = LENGTH_802_11_WITH_ADDR4;
		return hdr_len;

	}

#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) */

	if (!pEntry) {
#ifdef IDS_SUPPORT

		if ((pFmeCtrl->FrDs == 0) && (pRxBlk->wcid == WCID_NO_MATCHED(pAd))) /* not in RX WCID MAC table */
			pAd->ApCfg.RcvdMaliciousDataCount++;

#endif /* IDS_SUPPORT */
		return FALSE;
	}

	if (!((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))) {
#ifdef IDS_SUPPORT

		/*
			Replay attack detection,
			drop it if detect a spoofed data frame from a rogue AP
		*/
		if (pFmeCtrl->FrDs == 1)
			RTMPReplayAttackDetection(pAd, pRxBlk->Addr2, pRxBlk);

#endif /* IDS_SUPPORT */
		return FALSE;
	}

#ifdef A4_CONN
	if (((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))) {
#ifdef MWDS
		if (pEntry && GET_ENTRY_A4(pEntry) == A4_TYPE_MWDS) {
			pEntry->MWDSInfo.Addr3PktNum++;
			if ((pFmeCtrl->SubType == SUBTYPE_DATA_NULL) || (pFmeCtrl->SubType == SUBTYPE_QOS_NULL))
				pEntry->MWDSInfo.NullPktNum++;
			if (pRxInfo->Mcast || pRxInfo->Bcast)
				pEntry->MWDSInfo.bcPktNum++;
		}
#endif
		if ((pFmeCtrl->SubType != SUBTYPE_DATA_NULL) && (pFmeCtrl->SubType != SUBTYPE_QOS_NULL)) {
#ifdef MWDS
			if (pEntry && GET_ENTRY_A4(pEntry) == A4_TYPE_MWDS) {
				return FALSE;
			} else {
				/*if((((PUCHAR)pRxBlk->pData)[4])& 0x1 == 0x1)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("APRxPktAllow: Non MWDS Pkt=> wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x\nEth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
					pEntry->wdev->wdev_idx,pEntry->wdev->wdev_type,pEntry->wdev->func_idx,
					((PUCHAR)pRxBlk->pData)[0],((PUCHAR)pRxBlk->pData)[1],((PUCHAR)pRxBlk->pData)[2],((PUCHAR)pRxBlk->pData)[3],((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],
					((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],
					((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13]));
				}*/
			}
#endif
#ifdef CONFIG_MAP_SUPPORT
			/* do not receive 3-address broadcast/multicast packet, */
			/* because the broadcast/multicast packet woulld be send using 4-address, */
			/* 1905 message is an exception, need to receive 3-address 1905 multicast, */
			/* because some vendor send only one 3-address 1905 multicast packet */
			/* 1905 daemon would filter and drop duplicate packet */
			if (GET_ENTRY_A4(pEntry) == A4_TYPE_MAP &&
				(pRxInfo->Mcast || pRxInfo->Bcast) &&
				(memcmp(pRxBlk->Addr1, multicast_mac_1905, MAC_ADDR_LEN) != 0))
				return FALSE;
#endif
		}
	}
#endif /* A4_CONN */

	/* check if Class2 or 3 error */
	if (ap_chk_cl2_cl3_err(pAd, pRxBlk))
		return FALSE;

	if (pAd->ApCfg.BANClass3Data == TRUE)
		return FALSE;

#ifdef STATS_COUNT_SUPPORT
#if defined (MT7986) || defined (TR181_SUPPORT)
	if (!IS_MT7986(pAd))
#endif
		/* Increase received byte counter per BSS */
		if (pFmeCtrl->FrDs == 0 && pRxInfo->U2M) {
			BSS_STRUCT *pMbss = pEntry->pMbss;

			if (pMbss != NULL) {
				pMbss->ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
				pMbss->RxCount++;
			}
		}

	if (IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
		INC_COUNTER64(pAd->WlanCounters[0].MulticastReceivedFrameCount);

#endif /* STATS_COUNT_SUPPORT */

#ifdef WH_EVENT_NOTIFIER
	if (pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
		&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
		&& ((pFmeCtrl->SubType != SUBTYPE_DATA_NULL) && (pFmeCtrl->SubType != SUBTYPE_QOS_NULL))
	)
		pEntry->rx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */

#ifdef RADIUS_MAC_AUTH_SUPPORT
	if (pEntry->wdev->radius_mac_auth_enable) {
		if (!pEntry->bAllowTraffic)
			return FALSE;
	}
#endif

	hdr_len = LENGTH_802_11;
	RX_BLK_SET_FLAG(pRxBlk, fRX_STA);
	ASSERT(pEntry->wcid == pRxBlk->wcid);
	return hdr_len;
}

INT ap_rx_ps_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR OldPwrMgmt = PWR_ACTIVE; /* 1: PWR_SAVE, 0: PWR_ACTIVE */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* 1: PWR_SAVE, 0: PWR_ACTIVE */
	OldPwrMgmt = RtmpPsIndicate(pAd, pRxBlk->Addr2, pEntry->wcid, FC->PwrMgmt);
#ifdef UAPSD_SUPPORT
	if (cap->APPSMode != APPS_MODE2) {
		if ((FC->PwrMgmt == PWR_SAVE) &&
			(OldPwrMgmt == PWR_SAVE) &&
			(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
			(FC->SubType & 0x08)) {
			/*
			 * In IEEE802.11e, 11.2.1.4 Power management with APSD,
			 * If there is no unscheduled SP in progress, the unscheduled SP begins
			 * when the QAP receives a trigger frame from a non-AP QSTA, which is a
			 * QoS data or QoS Null frame associated with an AC the STA has
			 * configured to be trigger-enabled.
			 *
			 * In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
			 * to/from Power Save Mode is not considered to be a Trigger Frame and
			 * the AP shall not respond with a QoS Null frame.
			 */
			/* Trigger frame must be QoS data or QoS Null frame */
			UCHAR  OldUP;

			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
				OldUP = (*(pRxBlk->pData + 32) & 0x07);
			else
				OldUP = (*(pRxBlk->pData + LENGTH_802_11) & 0x07);

			UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
		}
	}

#endif /* UAPSD_SUPPORT */
	return TRUE;
}

INT ap_rx_pkt_foward(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	MAC_TABLE_ENTRY *pSrcEntry = NULL;
	BOOLEAN to_os, to_air;
	UCHAR *pHeader802_3;
	PNDIS_PACKET pForwardPacket;
	BSS_STRUCT *pMbss;
	struct wifi_dev *dst_wdev = NULL;
	UINT16 wcid;
#ifdef A4_CONN
	INT Ret;
#endif /* A4_CONN */
	PHEADER_802_3 pHDR = NULL;

	if (!VALID_MBSS(pAd, wdev->func_idx)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid func_idx(%d), type(%d)!\n",
				 wdev->func_idx, wdev->wdev_type);
		return FALSE;
	}

	/* only one connected sta, directly to upper layer */
	if (pAd->MacTab.Size <= 1)
		return TRUE;

	/* TODO: shiang-usw, remove pMbss structure here to make it more generic! */
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	pHeader802_3 = GET_OS_PKT_DATAPTR(pPacket);
	pHDR = (PHEADER_802_3)pHeader802_3;
	/* by default, announce this pkt to upper layer (bridge) and not to air */
	to_os = TRUE;
	to_air = FALSE;

	if (pHeader802_3[0] & 0x01) {
		if ((pMbss->StaCount > 1)
#ifdef P2P_SUPPORT
			|| (pAd->P2pCfg.bSigmaEnabled == TRUE)
#endif /* P2P_SUPPORT */
		   ) {
			/* forward the M/Bcast packet back to air if connected STA > 1 */
			to_air = TRUE;
		}
	} else {
		/* if destinated STA is a associated wireless STA */
		pEntry = MacTableLookup(pAd, pHeader802_3);

		if (pEntry && pEntry->Sst == SST_ASSOC && pEntry->wdev) {
			dst_wdev = pEntry->wdev;

			if (wdev == dst_wdev) {
				/*
					STAs in same SSID, default send to air and not to os,
					but not to air if match following case:
						a). pMbss->IsolateInterStaTraffic == TRUE
				*/
				to_air = TRUE;
				to_os = FALSE;

				if (pMbss->IsolateInterStaTraffic == 1)
					to_air = FALSE;
			} else {
				/*
					STAs in different SSID, default send to os and not to air
					but not to os if match any of following cases:
						a). destination VLAN ID != source VLAN ID
						b). pAd->ApCfg.IsolateInterStaTrafficBTNBSSID
				*/
				to_os = TRUE;
				to_air = FALSE;

				if (pAd->ApCfg.IsolateInterStaTrafficBTNBSSID == 1)
					to_os = FALSE;
			}
#ifdef WH_EVENT_NOTIFIER
			if (to_air && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
				&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
			)
				pEntry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */
				/* Check Source STA is in PortSecured then do FWD packet */
				pSrcEntry = MacTableLookup(pAd, pHDR->SAAddr2);
				if (pSrcEntry) {
					struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
					STA_TR_ENTRY *tr_entry = &tr_ctl->tr_entry[pSrcEntry->tr_tb_idx];

					if (tr_entry && (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Not PortSecured Pkt FWD to STAs from wcid(%d)"
							"to wcid(%d)!\n", pSrcEntry->wcid, pEntry->wcid);
						to_os = FALSE;
						to_air = FALSE;
					}
				}
			}
#ifdef A4_CONN
		else if ((((!pEntry)
#ifdef AIR_MONITOR
			|| (pEntry && IS_ENTRY_MONITOR(pEntry))
#endif
		 ) &&a4_proxy_lookup(pAd, wdev->func_idx, pHeader802_3, FALSE, TRUE, (UINT16 *)&wcid))) {
			if (IS_WCID_VALID(pAd, wcid))
				pEntry = &pAd->MacTab.Content[wcid];

			if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
				to_os = FALSE;
				to_air = TRUE;
				dst_wdev = pEntry->wdev;
			}
		}
#endif /* A4_CONN */

#ifdef MBSS_AS_WDS_AP_SUPPORT
#ifdef CLIENT_WDS
		else if ((!pEntry)
#ifdef AIR_MONITOR
			|| (pEntry && IS_ENTRY_MONITOR(pEntry))
#endif
			) {
			PUCHAR pEntryAddr = CliWds_ProxyLookup(pAd, pHeader802_3);
			if ((pEntryAddr != NULL)
				&& (!MAC_ADDR_EQUAL(pEntryAddr, pHeader802_3 + 6))) {
				pEntry = MacTableLookup(pAd, pEntryAddr);
				if (pEntry && (pEntry->Sst == SST_ASSOC) && pEntry->wdev && (!pEntry->wdev->bVLAN_Tag)) {
					to_os = FALSE;
					to_air = TRUE;
					dst_wdev = pEntry->wdev;
					if (wdev == dst_wdev) {
					/*
						STAs in same SSID, default send to air and not to os,
						but not to air if match following case:
						a). pMbss->IsolateInterStaTraffic == TRUE
					*/
						to_air = TRUE;
						to_os = FALSE;
						if (pMbss->IsolateInterStaTraffic == 1)
							to_air = FALSE;
						} else {
					/*
						STAs in different SSID, default send to os and not to air
						but not to os if match any of following cases:
						a). destination VLAN ID != source VLAN ID
						b). pAd->ApCfg.IsolateInterStaTrafficBTNBSSID
					*/
							to_os = TRUE;
							to_air = FALSE;
#ifdef RTMP_UDMA_SUPPORT
#ifdef ALLOW_INTER_STA_TRAFFIC_BTN_BSSID
					if (pAd->CommonCfg.bUdmaFlag) {
					/*
						default send to air and not to os,
						but not to air if match following case:
						a). destination VLAN ID != source VLAN ID
						b). pAd->ApCfg.IsolateInterStaTrafficBTNBSSID
					*/
						to_os = FALSE;
						to_air = TRUE;
					}
#endif
#endif
					if (pAd->ApCfg.IsolateInterStaTrafficBTNBSSID == 1) {
						to_os = FALSE;
#ifdef RTMP_UDMA_SUPPORT
#ifdef ALLOW_INTER_STA_TRAFFIC_BTN_BSSID
						to_air = FALSE;
#endif
#endif
					}
				}
			}
		}
	}
#endif /* CLIENT_WDS */
#endif

	}

	if (to_air) {
#ifdef MAP_TS_TRAFFIC_SUPPORT
		if (pAd->bTSEnable)
			pForwardPacket = CopyPacket(wdev->if_dev, pPacket);
		else
#endif
		pForwardPacket = DuplicatePacket(wdev->if_dev, pPacket);

		if (pForwardPacket == NULL)
			return to_os;

		/* 1.1 apidx != 0, then we need set packet mbssid attribute. */
		if (pEntry) {
			wcid = pEntry->wcid;
			RTMP_SET_PACKET_WDEV(pForwardPacket, dst_wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, wcid);
		} else { /* send bc/mc frame back to the same bss */
			wcid = wdev->tr_tb_idx;
			RTMP_SET_PACKET_WDEV(pForwardPacket, wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, wcid);
		}

		RTMP_SET_PACKET_MOREDATA(pForwardPacket, FALSE);
#ifdef RT_CFG80211_P2P_SUPPORT
		RTMP_SET_PACKET_OPMODE(pForwardPacket, OPMODE_AP);
#endif /* RT_CFG80211_P2P_SUPPORT */


#ifdef REDUCE_TCP_ACK_SUPPORT
		ReduceAckUpdateDataCnx(pAd, pForwardPacket);

		if (ReduceTcpAck(pAd, pForwardPacket) == FALSE)
#endif
		{
#ifndef A4_CONN
			send_data_pkt(pAd, wdev, pForwardPacket);
#else
			RTMP_SET_PACKET_A4_FWDDATA(pForwardPacket, TRUE);
			Ret = send_data_pkt(pAd, wdev, pForwardPacket);	/* rakesh: recheck */

			/* send bc/mc frame back to the same bss */
			if ((pHeader802_3[0] & 0x01) && (Ret == NDIS_STATUS_SUCCESS)) {
				a4_send_clone_pkt(pAd, wdev->func_idx, pPacket, pHeader802_3 + MAC_ADDR_LEN);
			}
#endif /* A4_CONN */
		}
	}

	return to_os;
}

INT ap_ieee_802_3_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	UCHAR wdev_idx = BSS0;
	BOOLEAN bFragment = FALSE;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef TXRX_STAT_SUPPORT
	struct hdev_obj *hdev = (struct hdev_obj *)wdev->pHObj;
#endif
#ifdef TR181_SUPPORT
	UCHAR bandIdx = HcGetBandByWdev(wdev);
#endif

	wdev_idx = wdev->wdev_idx;
	MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, "
			"fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
			pEntry->wcid, wdev->wdev_idx,
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

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	if (IS_SUPPORT_V10_DFS(pAd) && (((FRAME_CONTROL *)pRxBlk->FC)->Type == FC_TYPE_DATA))
		pEntry->LastRxTimeCount = 0;
#endif

	pEntry->NoDataIdleCount = 0;
	tr_ctl->tr_entry[pEntry->wcid].NoDataIdleCount = 0;
	pEntry->RxBytes += pRxBlk->MPDUtotalByteCnt;
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd))
		pEntry->RxBytesMAP += pRxBlk->MPDUtotalByteCnt;
#endif
	pEntry->OneSecRxBytes += pRxBlk->MPDUtotalByteCnt;
#ifdef ANTENNA_DIVERSITY_SUPPORT
	pEntry->ant_div_rx_bytes += pRxBlk->MPDUtotalByteCnt;
#endif
#ifdef TR181_SUPPORT
	pAd->WlanCounters[bandIdx].RxTotByteCount.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
	pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;
	INC_COUNTER64(pEntry->RxPackets);
#ifdef TXRX_STAT_SUPPORT
#ifdef ANDLINK_FEATURE_SUPPORT
	if ((pEntry != NULL) && (pEntry->Sst == SST_ASSOC) &&
			(IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)))
#else
	if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry))
#endif
	{
		INC_COUNTER64(pEntry->RxDataPacketCount);
		INC_COUNTER64(pEntry->RxUnicastPktCount);
		INC_COUNTER64(hdev->rdev->pRadioCtrl->RxDataPacketCount);
		pEntry->RxDataPacketByte.QuadPart += pRxBlk->MPDUtotalByteCnt;
		pEntry->RxUnicastByteCount.QuadPart += pRxBlk->MPDUtotalByteCnt;
		if (pEntry->pMbss) {
			INC_COUNTER64(pEntry->pMbss->stat_bss.RxUnicastDataPacket);
			INC_COUNTER64(pEntry->pMbss->stat_bss.RxDataPacketCount);
			pEntry->pMbss->stat_bss.RxDataPayloadByte.QuadPart += (pRxBlk->MPDUtotalByteCnt - 14);
			pEntry->pMbss->stat_bss.RxDataPacketByte.QuadPart += pRxBlk->MPDUtotalByteCnt;
		}
		hdev->rdev->pRadioCtrl->RxDataPacketByte.QuadPart += pRxBlk->MPDUtotalByteCnt;
		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
			INC_COUNTER64(pEntry->RxDataPacketCountPerAC[WMM_UP2AC_MAP[pRxBlk->UserPriority]]);
			INC_COUNTER64(hdev->rdev->pRadioCtrl->RxDataPacketCountPerAC[WMM_UP2AC_MAP[pRxBlk->UserPriority]]);
			if (pEntry->pMbss) {
				INC_COUNTER64(pEntry->pMbss->stat_bss.RxDataPacketCountPerAC[WMM_UP2AC_MAP[pRxBlk->UserPriority]]);
			}
		}
		{
			int ant_idx;
			for (ant_idx = 0; ant_idx < 4; ant_idx++) {
				pEntry->LastDataPktRssi[ant_idx] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), ant_idx);
				hdev->rdev->pRadioCtrl->LastDataPktRssi[ant_idx] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), ant_idx);
			}
		}
	}
#endif
#ifdef SMART_CARRIER_SENSE_SUPPORT
			{
				UINT idx = 0;

				for (idx = 0; idx < 4; idx++)
					pEntry->ScsDataRssi[idx] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), idx);


			}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#if defined(CUSTOMER_RSG_FEATURE) || defined(CUSTOMER_DCC_FEATURE) || defined(MAP_R2)
	if (pFmeCtrl->FrDs == 0 && pRxInfo->U2M && wdev->wdev_idx < pAd->ApCfg.BssidNum) {
#if defined(CUSTOMER_DCC_FEATURE) || defined(MAP_R2)
		UCHAR *pDA = pRxBlk->Addr3;
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[wdev->wdev_idx];

		if(pMbss) {
			pMbss->ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
			pMbss->RxCount++;
		}
		if (((*pDA) & 0x1) == 0x01) {
			if (IS_BROADCAST_MAC_ADDR(pDA)) {
				if(pMbss) {
					pMbss->bcPktsRx++;
					pMbss->bcBytesRx += pRxBlk->MPDUtotalByteCnt;
				}
#ifdef TR181_SUPPORT
				pAd->WlanCounters[bandIdx].bcPktsRx.QuadPart++;
				pAd->WlanCounters[bandIdx].bcBytesRx.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
			} else {
				if(pMbss) {
					pMbss->mcPktsRx++;
					pMbss->mcBytesRx += pRxBlk->MPDUtotalByteCnt;
				}
#ifdef TR181_SUPPORT
				pAd->WlanCounters[bandIdx].mcPktsRx.QuadPart++;
				pAd->WlanCounters[bandIdx].mcBytesRx.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
			}
		} else {
			if(pMbss) {
				pMbss->ucPktsRx++;
				pMbss->ucBytesRx += pRxBlk->MPDUtotalByteCnt;
			}
#ifdef TR181_SUPPORT
			pAd->WlanCounters[bandIdx].ucPktsRx.QuadPart++;
			pAd->WlanCounters[bandIdx].ucBytesRx.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
		}
#ifdef CUSTOMER_DCC_FEATURE
		pEntry->ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
		pEntry->RxCount++;

		if (pRxBlk->rx_signal.raw_snr[0])
			Update_Snr_Sample(pAd, pEntry, &pRxBlk->rx_signal);
#endif
#endif
	}
#endif

#ifdef RX_COUNT_DETECT
	pEntry->one_sec_rx_pkts++;
#endif /* RX_COUNT_DETECT */

	if (((FRAME_CONTROL *)pRxBlk->FC)->SubType & 0x08) {
		if ((pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID] != 0) && (pRxInfo->U2M))
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
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "duplicate frame drop it!\n");
		return FALSE;
	}

	if (rx_chk_amsdu_invalid_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"invalid amsdu frame drop it!\n");
		return FALSE;
	}
	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if (((FRAME_CONTROL *)pRxBlk->FC)->SubType & 0x04) { /* bit 2 : no DATA */
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Null/QosNull frame!\n");
		return FALSE;
	}

	if ((pRxBlk->FN == 0) && (pFmeCtrl->MoreFrag != 0)) {
		bFragment = TRUE;
		de_fragment_data_pkt(pAd, pRxBlk);
	}

	if (pRxInfo->U2M)
		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);

#ifdef IGMP_SNOOP_SUPPORT
	if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
		&& (wdev->IgmpSnoopEnable)
		&& IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3)) {
		PUCHAR pDA = pRxBlk->Addr3;
		PUCHAR pSA = pRxBlk->Addr2;
		PUCHAR pData = pRxBlk->pData + 12;
		UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));
#ifdef MAP_TS_TRAFFIC_SUPPORT
		/*when TS enable, for igmp need skip vlan header before handle*/
		if (pAd->bTSEnable) {
			if (RTMPEqualMemory(pData, TPID, 2)) {
				pData += LENGTH_802_1Q;
				protoType = OS_NTOHS(*((UINT16 *)(pData)));
			}
		}
#else
		if (protoType == ETH_TYPE_VLAN) {
			pData += LENGTH_802_1Q;
			protoType = OS_NTOHS(*((UINT16 *)(pData)));
		}
#endif
		if (protoType == ETH_P_IP)
			IGMPSnooping(pAd, pDA, pSA, pData, pEntry, pRxBlk->wcid);
		else if (protoType == ETH_P_IPV6)
			MLDSnooping(pAd, pDA, pSA,  pData, pEntry, pRxBlk->wcid);
	}

#ifdef IGMP_TVM_SUPPORT
		/* Convert Unicast Rx packet from AP to Multicast */
		if (IS_ENTRY_APCLI(pEntry)
			&& !IS_BM_MAC_ADDR(pRxBlk->Addr1)
			&& IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			ConvertUnicastMacToMulticast(pAd, wdev, pRxBlk);
		}
#endif /* IGMP_TVM_SUPPORT */

#endif /* IGMP_SNOOP_SUPPORT */

	if (0
#ifdef QOS_R1
		|| IS_QOSR1_ENABLE(pAd)
#endif
#ifdef MAP_R2
		|| IS_MAP_R2_ENABLE(pAd)
#endif
#ifdef MAP_R3
		|| IS_MAP_R3_ENABLE(pAd)
#endif
	) {
		if (pRxBlk->pRxPacket && pRxBlk->UserPriority && (pRxBlk->UserPriority < 8)) {
			RTMP_SET_PACKET_UP(pRxBlk->pRxPacket, pRxBlk->UserPriority);
			RTMP_SET_PACKET_UP_CB33(pRxBlk->pRxPacket, pRxBlk->UserPriority);
		}
	}

	if (pRxBlk->pRxPacket) {
		RTMP_SET_PACKET_WCID(pRxBlk->pRxPacket, pRxBlk->wcid);
		rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, pEntry->wdev);
	}

	ops->detect_wmm_traffic(pAd, wdev, pRxBlk->UserPriority, FLAG_IS_INPUT);

	return TRUE;
}

INT ap_ieee_802_11_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
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
#ifdef SW_CONNECT_SUPPORT
	STA_TR_ENTRY *tr_entry;
#endif /* SW_CONNECT_SUPPORT */
#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT) || defined(WTBL_TDD_SUPPORT) || defined(SW_CONNECT_SUPPORT)
	PCIPHER_KEY pPairwiseKey = NULL;
	UINT8 KeyId = 0;
	NDIS_STATUS status;
#endif /* defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT) || defined(WTBL_TDD_SUPPORT) */
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef TR181_SUPPORT
		UCHAR bandIdx = HcGetBandByWdev(wdev);
#endif

#ifdef WTBL_TDD_SUPPORT
	MAC_TABLE_ENTRY *pInactEntry = NULL;
	if (IS_WTBL_TDD_ENABLED(pAd)) {
		pInactEntry = WtblTdd_InactiveList_Lookup(pAd, pEntry->Addr);
	}
#endif /* WTBL_TDD_SUPPORT */

	wdev_idx = wdev->wdev_idx;
	MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
			 pEntry->wcid, wdev->wdev_idx,
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

#ifdef SW_CONNECT_SUPPORT
	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
#endif /* SW_CONNECT_SUPPORT */

	/*
		update RxBlk->pData, DataSize, 802.11 Header, QOS, HTC, Hw Padding
	*/

#if	defined(A4_CONN) || defined(APCLI_AS_WDS_STA_SUPPORT) || defined(MBSS_AS_WDS_AP_SUPPORT)
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

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

	if (0
#ifdef QOS_R1
		|| IS_QOSR1_ENABLE(pAd)
#endif
#ifdef MAP_R2
		|| IS_MAP_R2_ENABLE(pAd)
#endif
#ifdef MAP_R3
		|| IS_MAP_R3_ENABLE(pAd)
#endif
	) {
		if  (UserPriority && (UserPriority < 8)) {
			RTMP_SET_PACKET_UP(pRxBlk->pRxPacket, UserPriority);
			RTMP_SET_PACKET_UP_CB33(pRxBlk->pRxPacket, UserPriority);
		}
	}

	/*check if duplicate frame, ignore it and then drop*/
	if (rx_chk_duplicate_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, " duplicate frame drop it!\n");
		return FALSE;
	}

	if (rx_chk_amsdu_invalid_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"invalid amsdu frame drop it!\n");
		return FALSE;
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
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Null/QosNull frame!\n");
		drop_err = FALSE;
		return FALSE;
	}

	/* 4. skip HW padding */
	if (pRxInfo->L2PAD) {
		/* just move pData pointer because DataSize excluding HW padding */
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pData += 2;
	}

	if (pRxBlk->AmsduState) {
		struct _RXD_BASE_STRUCT *rx_base = NULL;

		rx_base = (struct _RXD_BASE_STRUCT *)pRxBlk->rmac_info;

		/* skip 2 bytes HW padding */
		pData += 2;
		pRxBlk->DataSize -= 2;

		pData += LENGTH_802_3;
		pRxBlk->DataSize -= LENGTH_802_3;
	}

	pRxBlk->pData = pData;
#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT) || defined(WTBL_TDD_SUPPORT) || defined(SW_CONNECT_SUPPORT)
	/* Use software to decrypt the encrypted frame if necessary.
	   If a received "encrypted" unicast packet(its WEP bit as 1)
	   and it's passed to driver with "Decrypted" marked as 0 in RxInfo.
	*/
	if (
#ifdef WTBL_TDD_SUPPORT
		pInactEntry ||
#endif /* WTBL_TDD_SUPPORT */
#ifdef SW_CONNECT_SUPPORT
		(tr_entry  && (tr_entry->bSw)) ||
#endif /* SW_CONNECT_SUPPORT */
		!IS_HIF_TYPE(pAd, HIF_MT)) {
		if ((pFmeCtrl->Wep == 1) && (pRxInfo->Decrypted == 0)) {
#if defined(WTBL_TDD_SUPPORT) || defined(SW_CONNECT_SUPPORT)
			if (IS_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher)) {
				pPairwiseKey = &pEntry->SecConfig.SwPairwiseKey;
			} else if (IS_CIPHER_TKIP(pEntry->SecConfig.PairwiseCipher)) {
				pPairwiseKey = &pEntry->SecConfig.SwPairwiseKey;
			} else if (IS_CIPHER_WEP40(pEntry->SecConfig.PairwiseCipher)) {
				if (pRxBlk->key_idx < SEC_KEY_NUM)
					KeyId = pRxBlk->key_idx;
				pPairwiseKey = (PCIPHER_KEY)&pEntry->SecConfig.WepKey[KeyId];
				pPairwiseKey->CipherAlg = CIPHER_WEP64;
			} else if (IS_CIPHER_WEP104(pEntry->SecConfig.PairwiseCipher)) {
				if (pRxBlk->key_idx < SEC_KEY_NUM)
					KeyId = pRxBlk->key_idx;
				pPairwiseKey = (PCIPHER_KEY)&pEntry->SecConfig.WepKey[KeyId];
				pPairwiseKey->CipherAlg = CIPHER_WEP128;
			} else {
				ASSERT(0);
				return FALSE;
			}
#endif /* WTBL_TDD_SUPPORT */

#ifdef HDR_TRANS_SUPPORT

			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
				status = RTMPSoftDecryptionAction(pAd,
												  pRxBlk->FC,
												  UserPriority,
												  pPairwiseKey,
#ifdef CONFIG_STA_SUPPORT
												  wdev_idx,
#endif /* CONFIG_STA_SUPPORT */
												  pRxBlk->pTransData + 14,
												  &(pRxBlk->TransDataSize));
			} else
#endif /* HDR_TRANS_SUPPORT */
			{
				status = RTMPSoftDecryptionAction(pAd,
												  pRxBlk->FC,
												  UserPriority,
												  pPairwiseKey,
#ifdef CONFIG_STA_SUPPORT
												  wdev_idx,
#endif /* CONFIG_STA_SUPPORT */
												  pRxBlk->pData,
												  &(pRxBlk->DataSize));
			}

			if (status != NDIS_STATUS_SUCCESS) {
#ifdef SW_CONNECT_SUPPORT
				if (tr_entry->bSw)
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wcid =%d,decryp fail !!\n", pEntry->wcid);
#endif /* SW_CONNECT_SUPPORT */
				/* Fix pkt Double Free */
				return FALSE;
			} else {
#ifdef SW_CONNECT_SUPPORT
				if (tr_entry->bSw) {
					MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "wcid =%d, S/W decryp OK !!\n", pEntry->wcid);
					RX_BLK_CLEAR_FLAG(pRxBlk, fRX_CM);
				}
#endif /* SW_CONNECT_SUPPORT */
			}

			/* Record the Decrypted bit as 1 */
			pRxInfo->Decrypted = 1;
		}
	}

#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT || WTBL_TDD_SUPPORT */
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
	tr_ctl->tr_entry[pEntry->wcid].NoDataIdleCount = 0;

#ifdef SW_CONNECT_SUPPORT
	tr_ctl->tr_entry[pEntry->wcid].rx_handle_cnt++;
#endif /* SW_CONNECT_SUPPORT */

	if (pRxInfo->U2M) {

#ifdef SW_CONNECT_SUPPORT
	tr_ctl->tr_entry[pEntry->wcid].rx_u2m_cnt++;
#endif /* SW_CONNECT_SUPPORT */

		pAd->ApCfg.NumOfAvgRssiSample++;
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
			process complete frame which encrypted by TKIP,
			Minus MIC length and calculate the MIC value
		*/
		if (bFragment && (pFmeCtrl->Wep) && IS_CIPHER_TKIP_Entry(pEntry)) {
			pRxBlk->DataSize -= 8;

			if (rtmp_chk_tkip_mic(pAd, pEntry, pRxBlk) == FALSE)
				return TRUE;
		}

		pEntry->RxBytes += pRxBlk->MPDUtotalByteCnt;
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd))
			pEntry->RxBytesMAP += pRxBlk->MPDUtotalByteCnt;
#endif
#ifdef TR181_SUPPORT
		pAd->WlanCounters[bandIdx].RxTotByteCount.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
		pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;
		INC_COUNTER64(pEntry->RxPackets);

#ifdef RX_COUNT_DETECT
		pEntry->one_sec_rx_pkts++;
#endif /* RX_COUNT_DETECT */

#ifdef MAC_REPEATER_SUPPORT

		if (IS_ENTRY_PEER_AP(pEntry))
			RTMP_SET_PACKET_WCID(pRxBlk->pRxPacket, pRxBlk->wcid);

#endif /* MAC_REPEATER_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
			&& (wdev->IgmpSnoopEnable)
			&& IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3)) {
			PUCHAR pDA = pRxBlk->Addr3;
			PUCHAR pSA = pRxBlk->Addr2;
			PUCHAR pData = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData;
			UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));

			if (protoType == ETH_P_IP)
				IGMPSnooping(pAd, pDA, pSA, pData, pEntry, pRxBlk->wcid);
			else if (protoType == ETH_P_IPV6)
				MLDSnooping(pAd, pDA, pSA,  pData, pEntry, pRxBlk->wcid);
		}

#endif /* IGMP_SNOOP_SUPPORT */

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
		else
			rx_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
	}

	ops->detect_wmm_traffic(pAd, wdev, pRxBlk->UserPriority, FLAG_IS_INPUT);

	return TRUE;
}

BOOLEAN ap_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	INT op_mode = OPMODE_AP;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	UINT32 apidx = 0;
#endif
#ifdef APCLI_SUPPORT
#ifdef WIFI_IAP_BCN_STAT_FEATURE
	UINT i = 0;
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/
#endif/*APCLI_SUPPORT*/

#ifdef IDS_SUPPORT

	/*
		Check if a rogue AP impersonats our mgmt frame to spoof clients
		drop it if it's a spoofed frame
	*/
	if (RTMPSpoofedMgmtDetection(pAd, pRxBlk))
		return FALSE;

	/* update sta statistics for traffic flooding detection later */
	RTMPUpdateStaMgmtCounter(pAd, FC->SubType);
#endif /* IDS_SUPPORT */

	if (!pRxInfo->U2M) {
		if ((FC->SubType != SUBTYPE_BEACON) && (FC->SubType != SUBTYPE_PROBE_REQ)) {
			BOOLEAN bDrop = TRUE;
			/* For PMF TEST Plan 5.4.3.1 & 5.4.3.2 */
#ifdef APCLI_SUPPORT

			if ((pEntry) && IS_ENTRY_PEER_AP(pEntry) &&
				((FC->SubType == SUBTYPE_DISASSOC) || (FC->SubType == SUBTYPE_DEAUTH) || (FC->SubType == SUBTYPE_ACTION)))
				bDrop = FALSE;

#endif /* APCLI_SUPPORT */

#if  defined(FTM_SUPPORT) || defined(WAPP_SUPPORT)

			if (IsPublicActionFrame(pAd, (VOID *)FC))
				bDrop = FALSE;

#endif /* defined(FTM_SUPPORT) || defined(WAPP_SUPPORT) */

#ifdef CONFIG_6G_SUPPORT
			if (FC->SubType == SUBTYPE_PROBE_RSP)
				bDrop = FALSE;
#endif
			if (bDrop == TRUE)
				return FALSE;
		}
#ifdef APCLI_SUPPORT
#ifdef WIFI_IAP_BCN_STAT_FEATURE
		for (i = 0; i < MAX_MULTI_STA; i++) {
			if (!pAd->StaCfg[i].wdev.DevInfo.WdevActive)
				continue;

			if ((FC->SubType == SUBTYPE_BEACON)
				&& (MAC_ADDR_EQUAL(pAd->StaCfg[i].Bssid, pRxBlk->Addr2))) {
					pAd->StaCfg[i].rx_beacon++;
			}
		}
#endif/*APCLI_SUPPORT*/
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/
	}

	/* Software decrypt WEP data during shared WEP negotiation */
	if ((FC->SubType == SUBTYPE_AUTH) &&
		(FC->Wep == 1) && (pRxInfo->Decrypted == 0)) {
		UCHAR *pMgmt = (PUCHAR)FC;
		UINT16 mgmt_len = pRxBlk->MPDUtotalByteCnt;
		UCHAR DefaultKeyId;

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR: SW decrypt WEP data fails - the Entry is empty.\n");
			return FALSE;
		}

		/* Skip 802.11 header */
		pMgmt += LENGTH_802_11;
		mgmt_len -= LENGTH_802_11;
#ifdef CONFIG_AP_SUPPORT
		DefaultKeyId = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PairwiseKeyId;
#endif /*  CONFIG_AP_SUPPORT */

		/* handle WEP decryption */
		if (RTMPSoftDecryptWEP(
				&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.WepKey[DefaultKeyId],
				pMgmt,
				&mgmt_len) == FALSE) {
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_AUTH_FAIL, REASON_DECRYPTION_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_MIC_FAILURE);
#endif

			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR: SW decrypt WEP data fails.\n");
			return FALSE;
		}

#ifdef RT_BIG_ENDIAN
		/* swap 16 bit fields - Auth Alg No. field */
		*(USHORT *)pMgmt = SWAP16(*(USHORT *)pMgmt);
		/* swap 16 bit fields - Auth Seq No. field */
		*(USHORT *)(pMgmt + 2) = SWAP16(*(USHORT *)(pMgmt + 2));
		/* swap 16 bit fields - Status Code field */
		*(USHORT *)(pMgmt + 4) = SWAP16(*(USHORT *)(pMgmt + 4));
#endif /* RT_BIG_ENDIAN */
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Decrypt AUTH seq#3 successfully\n");
		/* Update the total length */
		pRxBlk->DataSize -= (LEN_WEP_IV_HDR + LEN_ICV);
	}

	if (pEntry) {
		if (((op_mode == OPMODE_AP) && IS_ENTRY_CLIENT(pEntry)) ||
			((op_mode == OPMODE_STA) && (IS_ENTRY_TDLS(pEntry))))
			RtmpPsIndicate(pAd, pRxBlk->Addr2, pRxBlk->wcid, FC->PwrMgmt);

		/*
		 * 20190613 - accroding to IEEE802.11-2016
		 * To change power management modes a STA shall inform the AP by completing a successful frame
		 * exchange (as described in Annex G) that is initiated by the STA. This frame exchange shall include a
		 * Management frame, Extension frame or Data frame from the STA, and an Ack or a BlockAck frame from
		 * the AP.
		 */
	}
#ifdef TXRX_STAT_SUPPORT
	if (pEntry && !(scan_in_run_state(pAd, pEntry->wdev))) {
		int ant_idx;
		struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
		HTTRANSMIT_SETTING last_mgmt_rx_rate;
		ULONG MgmtRate;
		if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC || FC->SubType == SUBTYPE_ASSOC_REQ)) {
			UCHAR band_idx = HcGetBandByWdev(pEntry->wdev);
			INC_COUNTER64(pEntry->pMbss->stat_bss.RxMgmtPacketCount);
			INC_COUNTER64(pEntry->RxMgmtPacketCount);
			INC_COUNTER64(ctrl->rdev[band_idx].pRadioCtrl->RxMgmtPacketCount);
			last_mgmt_rx_rate = pRxBlk->rx_rate;
			getRate(last_mgmt_rx_rate, &MgmtRate);
			pEntry->RxLastMgmtPktRate = MgmtRate;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Rx-Pkt Src Address : "MACSTR"\n", MAC2STR(pRxBlk->Addr2));
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Rx Mgmt Subtype : %d\n", FC->SubType);
			for (ant_idx = 0; ant_idx < 4; ant_idx++)
				pEntry->LastMgmtPktRssi[ant_idx] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), ant_idx);
		}
	}
#endif


#ifdef CONVERTER_MODE_SWITCH_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		apidx = get_apidx_by_addr(pAd, pRxBlk->Addr1);
		if (((FC->Type == FC_TYPE_MGMT) && ((FC->SubType == SUBTYPE_ASSOC_REQ) ||
			(FC->SubType == SUBTYPE_REASSOC_REQ) || (FC->SubType == SUBTYPE_PROBE_REQ))) &&
			(pAd->ApCfg.MBSSID[apidx].APStartPseduState != AP_STATE_ALWAYS_START_AP_DEFAULT)) {
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Ignore mgmt packet\n");
			return TRUE;
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */


	/* Signal in MLME_QUEUE isn't used, therefore take this item to save min SNR. */
	{
		struct wifi_dev *recv_wdev = pAd->wdev_list[0];

		if (pEntry && pEntry->wdev && !IS_ENTRY_NONE(pEntry) && (HcGetBandByWdev(pEntry->wdev) == pRxBlk->band))
			recv_wdev = pEntry->wdev;
#ifdef DBDC_MODE
		else if (pAd->CommonCfg.dbdc_mode) {
			if (pRxBlk->band)
				recv_wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;
			else
				recv_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
		}
#endif	/* DBDC_MODE */

#ifdef CONFIG_6G_SUPPORT
		if (!pRxInfo->U2M && (FC->SubType == SUBTYPE_PROBE_RSP) && !WMODE_CAP(recv_wdev->PhyMode, WMODE_AX_6G))
			return FALSE;
#endif
		REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
								  FC,
								  pRxBlk->DataSize,
								  pRxBlk->rx_signal.raw_rssi[0],
								  pRxBlk->rx_signal.raw_rssi[1],
								  pRxBlk->rx_signal.raw_rssi[2],
								  pRxBlk->rx_signal.raw_rssi[3],
								  min(pRxBlk->rx_signal.raw_snr[0], pRxBlk->rx_signal.raw_snr[1]),
								  pRxBlk->channel_freq,
								  op_mode,
								  recv_wdev,
								  pRxBlk->rx_rate.field.MODE);
	}
	return TRUE;
}

struct wifi_dev_ops ap_wdev_ops = {
	.tx_pkt_allowed = ap_tx_pkt_allowed,
	.fp_tx_pkt_allowed = ap_fp_tx_pkt_allowed,
	.send_data_pkt = ap_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = ap_send_mlme_pkt,
	.tx_pkt_handle = ap_tx_pkt_handle,
	.legacy_tx = ap_legacy_tx,
	.ampdu_tx = ap_ampdu_tx,
	.frag_tx = ap_frag_tx,
	.amsdu_tx = ap_amsdu_tx,
	.fill_non_offload_tx_blk = ap_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = ap_fill_offload_tx_blk,
	.mlme_mgmtq_tx = ap_mlme_mgmtq_tx,
	.mlme_dataq_tx = ap_mlme_dataq_tx,
#ifdef CONFIG_ATE
	.ate_tx = mt_ate_tx,
#endif
#ifdef VERIFICATION_MODE
	.verify_tx = verify_pkt_tx,
#endif
	.ieee_802_11_data_tx = ap_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = ap_ieee_802_3_data_tx,
	.rx_pkt_allowed = ap_rx_pkt_allowed,
	.rx_ps_handle = ap_rx_ps_handle,
	.rx_pkt_foward = ap_rx_pkt_foward,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = ap_find_cipher_algorithm,
	.mac_entry_lookup = mac_entry_lookup,
	.media_state_connected = media_state_connected,
	.ioctl = rt28xx_ap_ioctl,
	.open = ap_inf_open,
	.close = ap_inf_close,
	.linkup = ap_link_up,
	.linkdown = ap_link_down,
	.conn_act = ap_conn_act,
	.disconn_act = wifi_sys_disconn_act,
};
