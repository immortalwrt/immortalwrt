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
    a4_conn.c

    Abstract:
    This is A4 connection function used to process those 4-addr of connected APClient or STA.
    Used by MWDS and MAP feature

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef A4_CONN
#include "rt_config.h"

BOOLEAN a4_interface_init(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap,
	IN UCHAR a4_type
)
{
	BSS_STRUCT *mbss = NULL;
	BOOLEAN add_inf = FALSE;

	if (is_ap) {
		if (!VALID_MBSS(adapter, if_index))
			return FALSE;

		mbss = &adapter->ApCfg.MBSSID[if_index];
		if (mbss->a4_init == 0) {
			NdisAllocateSpinLock(adapter, &mbss->a4_entry_lock);
			DlListInit(&mbss->a4_entry_list);
			RoutingTabInit(adapter, if_index, ROUTING_ENTRY_A4);
			add_inf = TRUE;
		}
		mbss->a4_init |= (1 << a4_type);
	}
#ifdef APCLI_SUPPORT
	else {
		PSTA_ADMIN_CONFIG apcli_entry;

		if (if_index >= MAX_MULTI_STA)
			return FALSE;
		apcli_entry = &adapter->StaCfg[if_index];
		if (apcli_entry->a4_init == 0)
			add_inf = TRUE;
		apcli_entry->a4_init |= (1 << a4_type);

#ifdef CONFIG_MAP_3ADDR_SUPPORT
		if (adapter->MapAccept3Addr && (apcli_entry->eth_list_init == 0)) {
			NdisAllocateSpinLock(adapter, &apcli_entry->eth_entry_lock);
			DlListInit(&apcli_entry->eth_entry_list);
			apcli_entry->eth_list_init = 1;
		}
#endif
	}
#else
	else
		return FALSE;
#endif

	if (add_inf) {
		if (adapter->a4_interface_count == 0)
			MtCmdSetA4Enable(adapter, HOST2CR4, TRUE);
		adapter->a4_interface_count++;
	}

	MTWF_DBG(adapter, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "a4_interface_init a4_interface_count: %d\n",
			adapter->a4_interface_count);
	return TRUE;
}


BOOLEAN a4_interface_deinit(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap,
	IN UCHAR a4_type
)
{
	BSS_STRUCT *mbss = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL, pConnEntryTmp = NULL;
	PDL_LIST a4_entry_list = NULL;
	PMAC_TABLE_ENTRY entry = NULL;
	BOOLEAN delete_inf = FALSE;

	if (is_ap) {
		if (!VALID_MBSS(adapter, if_index))
			return FALSE;

		mbss = &adapter->ApCfg.MBSSID[if_index];
		if (mbss->a4_init) {
			a4_entry_list = &mbss->a4_entry_list;
			DlListForEachSafe(a4_entry, pConnEntryTmp, a4_entry_list, A4_CONNECT_ENTRY, List) {
				if (a4_entry) {
					entry = &adapter->MacTab.Content[a4_entry->wcid];
					a4_ap_peer_disable(adapter, entry, a4_type);
				}
			}

			mbss->a4_init &= ~(1 << a4_type);
			if (!mbss->a4_init) {
				NdisFreeSpinLock(&mbss->a4_entry_lock);
				RoutingTabDestory(adapter, if_index, ROUTING_ENTRY_A4);
				delete_inf = TRUE;
			}
		}
	}
#ifdef APCLI_SUPPORT
	else {
		PSTA_ADMIN_CONFIG apcli_entry;

		if (if_index >= MAX_MULTI_STA)
			return FALSE;
		apcli_entry = &adapter->StaCfg[if_index];
		if (apcli_entry->a4_init) {
			apcli_entry->a4_init &= ~(1 << a4_type);
			if (!apcli_entry->a4_init)
				delete_inf = TRUE;
		}
	}
#else
	else
		return FALSE;
#endif

	if (delete_inf) {
		adapter->a4_interface_count--;
		if (adapter->a4_interface_count == 0)
			MtCmdSetA4Enable(adapter, HOST2CR4, FALSE);

	}

	MTWF_DBG(adapter, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "a4_interface_init a4_interface_count: %d\n",
			adapter->a4_interface_count);
	return TRUE;
}


INT a4_get_entry_count(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index
)
{
	int count = 0;
	BSS_STRUCT *mbss = NULL;

	if (!VALID_MBSS(adapter, if_index))
		return 0;

	mbss = &adapter->ApCfg.MBSSID[if_index];

	if (!mbss->a4_init)
		return 0;

	RTMP_SEM_LOCK(&mbss->a4_entry_lock);
	count = DlListLen(&mbss->a4_entry_list);
	RTMP_SEM_UNLOCK(&mbss->a4_entry_lock);
	return count;
}


BOOLEAN a4_lookup_entry_by_wcid(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UINT16 wcid
)
{
	BSS_STRUCT *mbss = NULL;
	PDL_LIST a4_entry_list = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	BOOLEAN found = FALSE;

	if (a4_get_entry_count(adapter, if_index) == 0)
		return FALSE;

	mbss = &adapter->ApCfg.MBSSID[if_index];
	a4_entry_list = &mbss->a4_entry_list;
	DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
		if (a4_entry->valid && (a4_entry->wcid == wcid)) {
			found = TRUE;
			break;
		}
	}
	return found;
}


BOOLEAN a4_lookup_entry_by_addr(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
)
{
	BSS_STRUCT *mbss = NULL;
	PDL_LIST a4_entry_list = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	PMAC_TABLE_ENTRY entry = NULL;
	BOOLEAN found = FALSE;

	if ((a4_get_entry_count(adapter, if_index) == 0) ||
		(mac_addr == NULL))
		return FALSE;

	mbss = &adapter->ApCfg.MBSSID[if_index];
	a4_entry_list = &mbss->a4_entry_list;
	DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
		if (a4_entry->valid && IS_WCID_VALID(adapter, a4_entry->wcid)) {
			entry = &adapter->MacTab.Content[a4_entry->wcid];

			if (MAC_ADDR_EQUAL(mac_addr, entry->Addr)) {
				found = TRUE;
				break;
			}
		}
	}
	return found;
}


VOID a4_add_entry(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UINT16 wcid
)
{
	BSS_STRUCT *mbss = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;

	if (a4_lookup_entry_by_wcid(adapter, if_index, wcid))
		return;

	mbss = &adapter->ApCfg.MBSSID[if_index];
	os_alloc_mem(adapter, (UCHAR **)&a4_entry, sizeof(A4_CONNECT_ENTRY));

	if (a4_entry) {
		NdisZeroMemory(a4_entry, sizeof(A4_CONNECT_ENTRY));
		a4_entry->valid = 1;
		a4_entry->wcid = wcid;
		RTMP_SEM_LOCK(&mbss->a4_entry_lock);
		DlListAddTail(&mbss->a4_entry_list, &a4_entry->List);
		RTMP_SEM_UNLOCK(&mbss->a4_entry_lock);
	} else
		MTWF_DBG(adapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Fail to alloc memory for pNewConnEntry\n");
}


VOID a4_delete_entry(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UINT16 wcid
)
{
	BSS_STRUCT *mbss = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	PDL_LIST a4_entry_list = NULL;

	if (a4_get_entry_count(adapter, if_index) == 0)
		return;

	mbss = &adapter->ApCfg.MBSSID[if_index];
	a4_entry_list = &mbss->a4_entry_list;
	RTMP_SEM_LOCK(&mbss->a4_entry_lock);
	DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
		if (a4_entry->valid && (a4_entry->wcid == wcid)) {
			DlListDel(&a4_entry->List);
			os_free_mem(a4_entry);
			break;
		}
	}
	RTMP_SEM_UNLOCK(&mbss->a4_entry_lock);
}


VOID a4_proxy_delete(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
)
{
	if (a4_get_entry_count(adapter, if_index) == 0)
		return;

	RoutingTabSetOneFree(adapter, if_index, mac_addr, ROUTING_ENTRY_A4);
}

BOOLEAN a4_get_dst_ip(void *pkt, unsigned char *ip)
{
	UCHAR *pSrcBuf;
	UINT16 TypeLen;
	UINT32 type_len_idx = 12;
	UCHAR *ip_header = NULL;

	pSrcBuf = pkt;
	TypeLen = (pSrcBuf[type_len_idx] << 8) | pSrcBuf[type_len_idx + 1];
	while (TypeLen == ETH_TYPE_VLAN) {
		type_len_idx += 2;
		TypeLen = (pSrcBuf[type_len_idx] << 8) | pSrcBuf[type_len_idx + 1];
	}
	if (TypeLen == ETH_TYPE_IPv4) {
		ip_header = &pSrcBuf[type_len_idx + 2];
		NdisCopyMemory(ip, ip_header + 16, 4);
		return TRUE;
	}
	return FALSE;
}
BOOLEAN a4_proxy_lookup(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr,
	IN BOOLEAN update_alive_time,
	IN BOOLEAN is_rx,
	OUT UINT16 *wcid
)
{
	*wcid = 0;

	if (a4_get_entry_count(adapter, if_index) == 0)
		return FALSE;

	if (RoutingTabLookup(adapter, if_index, mac_addr, update_alive_time, wcid) != NULL)
		return TRUE;
	else
		return FALSE;
}


VOID a4_proxy_update(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UINT16 wcid,
	IN PUCHAR mac_addr,
	IN UINT32 ip /* ARP Sender IP*/
)
{
	UINT16 proxy_wcid = 0;
	BOOLEAN found = FALSE;
	PROUTING_ENTRY routing_entry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	if (a4_get_entry_count(adapter, if_index) == 0)
		return;

	if (!IS_WCID_VALID(adapter, wcid) || !mac_addr)
		return;

	pMacEntry = MacTableLookup(adapter, mac_addr);
	if (pMacEntry && IS_ENTRY_A4(pMacEntry)) {
		MTWF_DBG(adapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Addr4("MACSTR") is an A4 entry.\n",
				MAC2STR(mac_addr));
		return;
	}
	routing_entry = RoutingTabLookup(adapter, if_index, mac_addr, TRUE, &proxy_wcid);
	found = (routing_entry != NULL) ? TRUE : FALSE;

	if (found) {
		if (ROUTING_ENTRY_TEST_FLAG(routing_entry, ROUTING_ENTRY_A4)) {
			/* Mean the target change to other ProxyAP */
			if (proxy_wcid != wcid) {
#ifdef WHNAT_SUPPORT
				if (adapter->CommonCfg.whnat_en) {
#ifdef CONFIG_FAST_NAT_SUPPORT
					/* In WARP enabled scenario, the old PPE entry need to be
					deleted when the STA is roaming from one agent to another.
					*/
					if (ppe_del_entry_by_mac != NULL) {
						MTWF_DBG(adapter, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"Target "MACSTR" change from %u to %u!\n",
							MAC2STR(mac_addr), proxy_wcid, wcid);
						ppe_del_entry_by_mac(mac_addr);
					}
#endif
				}
#endif
				RoutingTabSetOneFree(adapter, if_index, mac_addr, ROUTING_ENTRY_A4);
				routing_entry = NULL;
				found = FALSE;
			}
		} else {
			/* Assign A4 falg to this one if found. */
			SET_ROUTING_ENTRY(routing_entry, ROUTING_ENTRY_A4);
		}
	}

	if (!found) {
		/* Allocate a new one if not found. */
		routing_entry = RoutingTabGetFree(adapter, if_index);

		if (routing_entry) {
			SET_ROUTING_ENTRY(routing_entry, ROUTING_ENTRY_A4);
			RoutingEntrySet(adapter, if_index, wcid, mac_addr, routing_entry);
		}
	}

	if (routing_entry != NULL && (ip != 0)) /* ARP Sender IP Update*/
		RoutingTabARPLookupUpdate(adapter, if_index, routing_entry, ip);
}


VOID a4_proxy_maintain(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index
)
{
	if (a4_get_entry_count(adapter, if_index) == 0)
		return;

	RoutingTabMaintain(adapter, if_index);
}


INT a4_hard_transmit(
	IN PRTMP_ADAPTER adapter,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt
)
{
	UINT32 wcid = WCID_INVALID;
	INT ret = NDIS_STATUS_SUCCESS;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	UINT SrcBufLen = 0;

	if (RTMP_GET_PACKET_WCID(pkt) == 0) {
		MTWF_DBG(adapter, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wcid=0 in a4_hard_transmit\n");
		RELEASE_NDIS_PACKET(adapter, pkt, NDIS_STATUS_FAILURE);
		return ret;
	}

	SrcBufLen = RTMP_GET_PKT_LEN(pkt);

	if (SrcBufLen <= 14) {
		MTWF_DBG(adapter, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pkt error(len: %d)\n", SrcBufLen);
		RELEASE_NDIS_PACKET(adapter, pkt, NDIS_STATUS_FAILURE);
		return ret;
	}
	if (IS_ASIC_CAP(adapter, fASIC_CAP_MCU_OFFLOAD)) {
		ret = ops->fp_send_data_pkt(adapter, wdev, pkt);
	} else {
		wcid = RTMP_GET_PACKET_WCID(pkt);
		if (!RTMPCheckEtherType(adapter, pkt, &adapter->MacTab.tr_entry[wcid], wdev)) {
			MTWF_DBG(adapter, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Pkt Len/Ethernet Check Fail\n");
			RELEASE_NDIS_PACKET(adapter, pkt, NDIS_STATUS_FAILURE);
			return ret;
		}
		ret = ops->send_data_pkt(adapter, wdev, pkt);
	}
	return ret;
}


PNDIS_PACKET a4_clone_packet(
	IN PRTMP_ADAPTER adapter,
	IN PNET_DEV if_dev,
	IN PNDIS_PACKET pkt
)
{
	struct sk_buff *skb = NULL;
	PNDIS_PACKET pkt_clone = NULL;

	skb = skb_copy(RTPKT_TO_OSPKT(pkt), GFP_ATOMIC);

	if (skb) {
		skb->dev = if_dev;
		pkt_clone = OSPKT_TO_RTPKT(skb);
	}

	return pkt_clone;
}

void a4_send_clone_pkt(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PNDIS_PACKET pkt,
	IN PUCHAR exclude_mac_addr
)
{
	struct wifi_dev *wdev = NULL;
	PNDIS_PACKET pkt_clone = NULL;
	PDL_LIST a4_entry_list = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	PROUTING_ENTRY routing_entry = NULL;
	PMAC_TABLE_ENTRY entry = NULL;
	BOOLEAN found = FALSE;
	UINT16 wcid = 0;
	struct wifi_dev *ap_wdev = NULL;
#ifdef MAP_R2
	PMAC_TABLE_ENTRY peer_entry = NULL;
#endif

	if (!pkt)
		return;

	if ((a4_get_entry_count(adapter, if_index) > 0)) {
		ap_wdev = &adapter->ApCfg.MBSSID[if_index].wdev;
#if (defined(IGMP_SNOOP_SUPPORT) && defined(IGMP_TX_QUERY_HOLD))
			if (ap_wdev->IgmpSnoopEnable) { /* If snooping enabled*/
				UCHAR *pDestAddr = NULL;
				pDestAddr = GET_OS_PKT_DATAPTR(pPacket);
				if (IS_MULTICAST_MAC_ADDR(pDestAddr)) {
					PUCHAR pData = pDestAddr + 12;
					UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));
					/* Check whether membership query sent by some other device.*/
					/* If detected, set internal query Hold duration to avoid flooding in network*/
					if (protoType == ETH_P_IP) {
						if (isIGMPquery(adapter, pDestAddr, pData)) {
							adapter->ApCfg.MBSSID[if_index].IgmpQueryHoldTick = QUERY_HOLD_PERIOD;
						}
					} else if (protoType == ETH_P_IPV6) {
						if (isMLDquery(adapter, pDestAddr, pData)) {
							adapter->ApCfg.MBSSID[if_index].MldQueryHoldTick = QUERY_HOLD_PERIOD;
						}
					}
				}
			}
#endif

		if (exclude_mac_addr) {
			routing_entry = RoutingTabLookup(adapter, if_index, exclude_mac_addr, FALSE, &wcid);

			if (routing_entry && ROUTING_ENTRY_TEST_FLAG(routing_entry, ROUTING_ENTRY_A4))
				found = TRUE;
			else
				found = FALSE;
		}

		a4_entry_list = &adapter->ApCfg.MBSSID[if_index].a4_entry_list;
		RTMP_SEM_LOCK(&adapter->ApCfg.MBSSID[if_index].a4_entry_lock);
		DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
			if (a4_entry->valid && IS_WCID_VALID(adapter, a4_entry->wcid)) {
				if (found && (wcid == a4_entry->wcid))
					continue;

				entry = &adapter->MacTab.Content[a4_entry->wcid];
				wdev = entry->wdev;

				pkt_clone = a4_clone_packet(adapter, wdev->if_dev, pkt);
				if (pkt_clone == NULL) {
					RTMP_SEM_UNLOCK(&adapter->ApCfg.MBSSID[if_index].a4_entry_lock);
					MTWF_DBG(adapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Fail to alloc memory for pPacketClone\n");
					return;
				}
#ifdef MAP_TS_TRAFFIC_SUPPORT
				if (adapter->bTSEnable) {
					peer_entry = &adapter->MacTab.Content[entry->wcid];

					if (!map_ts_tx_process(adapter, wdev, pkt_clone, peer_entry)) {
						RELEASE_NDIS_PACKET(adapter, pkt_clone, NDIS_STATUS_FAILURE);
						continue;
					}
				}
#endif

				RTMP_SET_PACKET_WCID(pkt_clone, entry->wcid);
				RTMP_SET_PACKET_WDEV(pkt_clone, wdev->wdev_idx);
				RTMP_SET_PACKET_MOREDATA(pkt_clone, FALSE);
				/* packet type is only checked in tx_pkt_classification */
				/* it may happen for unicast, packet type goes as multicast */
				RTMP_SET_PACKET_TXTYPE(pkt_clone, TX_UNKOWN_FRAME);
				/*RTMP_SET_PACKET_QUEIDX(pPacketClone, QID_AC_BE);*/
				a4_hard_transmit(adapter, wdev, pkt_clone);
			}
		}
		RTMP_SEM_UNLOCK(&adapter->ApCfg.MBSSID[if_index].a4_entry_lock);
	}
}


BOOLEAN a4_ap_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
)
{
	BSS_STRUCT *mbss = NULL;
	UCHAR if_index;
	CMD_WTBL_HDR_TRANS_T wtbl_hdr_trans;

	if (!entry || !IS_ENTRY_CLIENT(entry))
		return FALSE;

	if_index = entry->func_tb_idx;
	if (!VALID_MBSS(adapter, if_index))
		return FALSE;

	mbss = &adapter->ApCfg.MBSSID[if_index];
#ifdef WSC_AP_SUPPORT
	if (mbss &&
		(mbss->wdev.WscControl.WscConfMode != WSC_DISABLE) &&
		(mbss->wdev.WscControl.bWscTrigger == TRUE)) {
#ifdef CONFIG_MAP_SUPPORT
		/* 3-address required only during wps onboarding and not for the */
		/* Agent who used the saved profile for BH AP */
		if (IS_MAP_ENABLE(adapter) && entry && (entry->bWscCapable == TRUE))
#endif
			return FALSE;
	}
#endif /* WSC_AP_SUPPORT */

	if (IS_ENTRY_A4(entry) == FALSE) {
		/* To check and remove entry which is created from another side. */
		a4_proxy_delete(adapter, if_index, entry->Addr);

		os_zero_mem(&wtbl_hdr_trans, sizeof(CMD_WTBL_HDR_TRANS_T));
		MTWF_DBG(adapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Enabled A4 for entry:"MACSTR"\n",
					MAC2STR(entry->Addr));
		wtbl_hdr_trans.u2Tag = WTBL_HDR_TRANS;
		wtbl_hdr_trans.u2Length = sizeof(CMD_WTBL_HDR_TRANS_T);
		wtbl_hdr_trans.ucTd = 1;
		wtbl_hdr_trans.ucFd = 1;
		wtbl_hdr_trans.ucDisRhtr = 0;
		if (NDIS_STATUS_SUCCESS != CmdExtWtblUpdate(adapter,
													entry->wcid,
													SET_WTBL,
													&wtbl_hdr_trans,
													sizeof(CMD_WTBL_HDR_TRANS_T)))
			return FALSE;

		a4_add_entry(adapter, if_index, entry->wcid);
	}

	if (type > GET_ENTRY_A4(entry)) {
		SET_ENTRY_A4(entry, type);
		MTWF_PRINT("SET_A4_ENTRY type:%d OK!\n", type);
	}

	return TRUE;
}


BOOLEAN a4_ap_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
)
{
	UCHAR if_index;

	if (!entry || !IS_ENTRY_CLIENT(entry))
		return FALSE;

	if_index = entry->func_tb_idx;
	if (!VALID_MBSS(adapter, if_index))
		return FALSE;

	if (type == GET_ENTRY_A4(entry)) {
		SET_ENTRY_A4(entry, A4_TYPE_NONE);
		a4_delete_entry(adapter, if_index, entry->wcid);
		RoutingTabSetAllFree(adapter, if_index, entry->wcid, ROUTING_ENTRY_A4);
		MTWF_DBG(adapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"a4_ap_peer_disable: Disable A4 for entry : "MACSTR"\n",
				MAC2STR(entry->Addr));
	}

	return TRUE;
}


#ifdef APCLI_SUPPORT
BOOLEAN a4_apcli_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PSTA_ADMIN_CONFIG apcli_entry,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
)
{

	CMD_WTBL_HDR_TRANS_T wtbl_hdr_trans;
	if (!apcli_entry || !entry || !IS_ENTRY_PEER_AP(entry))
		return FALSE;

#ifdef WSC_AP_SUPPORT
	if (((apcli_entry->wdev.WscControl.WscConfMode != WSC_DISABLE) &&
			(apcli_entry->wdev.WscControl.bWscTrigger == TRUE)))
		return FALSE;
#endif /* WSC_AP_SUPPORT */

	if (IS_ENTRY_A4(entry) == FALSE) {

		os_zero_mem(&wtbl_hdr_trans, sizeof(CMD_WTBL_HDR_TRANS_T));
		MTWF_DBG(adapter, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"a4_apcli_peer_enable enabled A4 for entry : "MACSTR"\n",
				MAC2STR(entry->Addr));
		wtbl_hdr_trans.u2Tag = WTBL_HDR_TRANS;
		wtbl_hdr_trans.u2Length = sizeof(CMD_WTBL_HDR_TRANS_T);
		wtbl_hdr_trans.ucTd = 1;
		wtbl_hdr_trans.ucFd = 1;
		wtbl_hdr_trans.ucDisRhtr = 0;
		if (CmdExtWtblUpdate(adapter,
							entry->wcid,
							SET_WTBL,
							&wtbl_hdr_trans,
							sizeof(CMD_WTBL_HDR_TRANS_T)) == NDIS_STATUS_SUCCESS)
			MTWF_PRINT("SET_A4_ENTRY OK!\n");
		else
			return FALSE;
	}

	if (type > GET_ENTRY_A4(entry)) {
		SET_APCLI_A4(apcli_entry, type);
		SET_ENTRY_A4(entry, type);
	}
	return TRUE;
}


BOOLEAN a4_apcli_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PSTA_ADMIN_CONFIG apcli_entry,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
)
{

	if (!apcli_entry || !entry)
		return FALSE;

	if (type == GET_ENTRY_A4(entry)) {
		MTWF_DBG(adapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"a4_apcli_peer_disable, Disable A4 for entry:"MACSTR"\n",
				MAC2STR(entry->Addr));
		SET_APCLI_A4(apcli_entry, A4_TYPE_NONE);
		SET_ENTRY_A4(entry, A4_TYPE_NONE);
	}

	return TRUE;
}
#endif /* APCLI_SUPPORT */


INT Set_APProxy_Status_Show_Proc(
	IN  PRTMP_ADAPTER adapter,
	IN  RTMP_STRING * arg)
{
	POS_COOKIE obj;
	UCHAR if_index;
	INT32 i, count = 0;
	UINT32 ip_addr = 0;
	ULONG now = 0, AliveTime = 0;
	PDL_LIST a4_entry_list = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	PROUTING_ENTRY routing_entry = NULL, *routing_entry_list;
	UCHAR *proxy_mac_addr = NULL, proxy_ip[64];

	BSS_STRUCT *mbss = NULL;
	obj = (POS_COOKIE) adapter->OS_Cookie;
	if_index = obj->ioctl_if;

	os_alloc_mem_suspend(adapter, (UCHAR **)&routing_entry_list, ROUTING_POOL_SIZE*sizeof(routing_entry));

	if (!routing_entry_list) {
		MTWF_DBG(adapter, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Mem alloc fail!\n");
		return FALSE;
	}

	os_zero_mem(routing_entry_list, ROUTING_POOL_SIZE*sizeof(routing_entry));

	if ((obj->ioctl_if_type != INT_MBSSID) && (obj->ioctl_if_type != INT_MAIN)) {
		os_free_mem(routing_entry_list);
		return FALSE;
	}


	if (a4_get_entry_count(adapter, if_index) == 0) {
		os_free_mem(routing_entry_list);
		return TRUE;
	}

	mbss = &adapter->ApCfg.MBSSID[if_index];
	a4_entry_list = &adapter->ApCfg.MBSSID[if_index].a4_entry_list;
	NdisGetSystemUpTime(&now);
	RTMP_SEM_LOCK(&mbss->a4_entry_lock);
	DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
		if (a4_entry->valid && IS_WCID_VALID(adapter, a4_entry->wcid)) {
			count = 0;
			proxy_mac_addr = adapter->MacTab.Content[a4_entry->wcid].Addr;
			MTWF_PRINT("Proxy Mac: "MACSTR"\n", MAC2STR(proxy_mac_addr));

			if (GetRoutingEntryAll(adapter, if_index, a4_entry->wcid, ROUTING_ENTRY_A4,
					ROUTING_POOL_SIZE, (ROUTING_ENTRY **)routing_entry_list, &count)) {
				for (i = 0; i < count; i++) {
					routing_entry = (PROUTING_ENTRY)(*(routing_entry_list + i));

					if (!routing_entry)
						continue;

					if (routing_entry->KeepAliveTime >= now)
						AliveTime = ((routing_entry->KeepAliveTime - now) / OS_HZ);
					else
						AliveTime = 0;

					if (routing_entry->IPAddr != 0) {
						ip_addr = routing_entry->IPAddr;
						sprintf(proxy_ip, "%d.%d.%d.%d",
								(ip_addr & 0xff),
								((ip_addr & (0xff << 8)) >> 8),
								((ip_addr & (0xff << 16)) >> 16),
								((ip_addr & (0xff << 24)) >> 24));
					} else
						sprintf(proxy_ip, "0.0.0.0");

					MTWF_PRINT("MAC:"MACSTR", IP:%s, AgeOut:%lus, Retry:(%d,%d)\n", 
							MAC2STR(routing_entry->Mac), proxy_ip, AliveTime,
							routing_entry->Retry, ROUTING_ENTRY_MAX_RETRY);
				}

				MTWF_PRINT("Total Count = %d\n\n", count);
			}
		}
	}

	RTMP_SEM_UNLOCK(&mbss->a4_entry_lock);
	if (routing_entry_list)
		os_free_mem(routing_entry_list);

	return TRUE;
}

INT Set_APProxy_Refresh_Proc(
	IN	PRTMP_ADAPTER adapter,
	IN	RTMP_STRING *arg)
{
	adapter->a4_need_refresh = TRUE;
	return TRUE;
}

#ifdef WARP_512_SUPPORT
extern INT MacTableResetEntry(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, BOOLEAN clean);
extern INT32 MacTableDelEntryFromHash(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
extern void aid_clear(struct _aid_info *aid_info, UINT16 aid);

MAC_TABLE_ENTRY *ReassignWcidForA4Entry(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN IE_LISTS *ie_list)
{
	UINT16 wcid;
#ifdef SW_CONNECT_SUPPORT
	UINT16 hw_wcid = WCID_INVALID;
	BOOLEAN bSw = FALSE;
#endif /* SW_CONNECT_SUPPORT */
	MAC_TABLE_ENTRY *pA4Entry = NULL, *pCurrEntry;
	STA_TR_ENTRY *pA4TREntry = NULL;
	UCHAR HashIdx;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	/* get a new wcid for A4 entry */
#ifdef SW_CONNECT_SUPPORT
	wcid = HcAcquireUcastWcid(pAd, pEntry->wdev, TRUE, FALSE, &hw_wcid, &bSw);
	if ((wcid == WCID_INVALID) && (bSw == FALSE))
		return NULL;
#else /* SW_CONNECT_SUPPORT */
	wcid = HcAcquireUcastWcid(pAd, pEntry->wdev, TRUE, FALSE);
	if (wcid == WCID_INVALID)
		return NULL;
#endif /* !SW_CONNECT_SUPPORT */

	if (wdev_do_disconn_act(pEntry->wdev, pEntry) != TRUE) {
		HcReleaseUcastWcid(pAd, pEntry->wdev, wcid);
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "AP disconnection fail!\n");
		return NULL;
	}

	NdisAcquireSpinLock(&pAd->MacTabLock);
	pA4Entry = &pAd->MacTab.Content[wcid];
	if (!pA4Entry || IS_ENTRY_NONE(pEntry)) {
		HcReleaseUcastWcid(pAd, pEntry->wdev, wcid);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"pA4Entry NULL or STA-"MACSTR" deleted(0x%x), release new wcid %d!\n",
			MAC2STR(ie_list->Addr2), pEntry->EntryType, wcid);
		NdisReleaseSpinLock(&pAd->MacTabLock);
		return NULL;
	}
	MacTableResetEntry(pAd, pA4Entry, TRUE);
	NdisMoveMemory(pA4Entry, pEntry, sizeof(MAC_TABLE_ENTRY));
	pA4Entry->wcid = wcid;
#ifdef SW_CONNECT_SUPPORT
	pA4Entry->bSw = bSw;
	/*
		original pEntry->wcid entry usages are most for S/W concept, only minor parts are for H/W concept.
		so add extra pEntry->hw_wcid for hw backup.
	*/
	pA4Entry->hw_wcid = hw_wcid;

	/* S/W entry Force set valid , w/o cmd resp check */
	if (bSw == TRUE)
		pA4Entry->sta_rec_valid = TRUE;
#endif /* SW_CONNECT_SUPPORT */

	pA4Entry->pNext = NULL; /*make sure: pNext of the tail is NULL*/

	pSecConfig = &pA4Entry->SecConfig;
	RTMPInitTimer(pAd, &pSecConfig->StartFor4WayTimer, GET_TIMER_FUNCTION(WPAStartFor4WayExec), pA4Entry, FALSE);
	RTMPInitTimer(pAd, &pSecConfig->StartFor2WayTimer, GET_TIMER_FUNCTION(WPAStartFor2WayExec), pA4Entry, FALSE);
	RTMPInitTimer(pAd, &pSecConfig->Handshake.MsgRetryTimer, GET_TIMER_FUNCTION(WPAHandshakeMsgRetryExec), pA4Entry, FALSE);
#ifdef DOT11W_PMF_SUPPORT
	RTMPInitTimer(pAd, &pSecConfig->PmfCfg.SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pA4Entry, FALSE);
	RTMPInitTimer(pAd, &pSecConfig->PmfCfg.SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pA4Entry, FALSE);
#endif /* DOT11W_PMF_SUPPORT */
	RTMP_OS_INIT_COMPLETION(&pA4Entry->WtblSetDone);

	/* copy old tr_entry to the new one*/
	pA4TREntry = &tr_ctl->tr_entry[wcid];

	TRTableResetEntry(pAd, pEntry->wcid);
	TRTableInsertEntry(pAd, wcid, pA4Entry);

	/* delete pEntry from Hash table */
	MacTableDelEntryFromHash(pAd, pEntry);

	HashIdx = MAC_ADDR_HASH_INDEX(ie_list->Addr2);

	if (pAd->MacTab.Hash[HashIdx] == NULL)
		pAd->MacTab.Hash[HashIdx] = pA4Entry;
	else {
		pCurrEntry = pAd->MacTab.Hash[HashIdx];

		while (pCurrEntry->pNext != NULL)
			pCurrEntry = pCurrEntry->pNext;

		pCurrEntry->pNext = pA4Entry;
	}

	HcReleaseUcastWcid(pAd, pEntry->wdev, pEntry->wcid);
	MacTableResetEntry(pAd, pEntry, FALSE);
	NdisReleaseSpinLock(&pAd->MacTabLock);

	return pA4Entry;
}
#endif
#ifdef CONFIG_MAP_3ADDR_SUPPORT
BOOLEAN eth_lookup_entry_by_addr(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr,
	IN UCHAR update_time
)
{
	PSTA_ADMIN_CONFIG apcli_entry = NULL;
	PDL_LIST eth_entry_list = NULL;
	PEth_CONNECT_ENTRY eth_entry = NULL;
	BOOLEAN found = FALSE;

	if (mac_addr == NULL)
		return FALSE;

	if (if_index >= MAX_MULTI_STA)
		return FALSE;

	apcli_entry = &adapter->StaCfg[if_index];

	eth_entry_list = &apcli_entry->eth_entry_list;
	DlListForEach(eth_entry, eth_entry_list, Eth_CONNECT_ENTRY, List) {
		if (MAC_ADDR_EQUAL(mac_addr, eth_entry->mac)) {
			if (update_time == TRUE)
				eth_entry->entry_flush_count = Eth_Entry_Flush_Time;
			found = TRUE;
			break;
		}
	}
	return found;
}

VOID eth_add_entry(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
)
{
	PEth_CONNECT_ENTRY eth_entry = NULL;
	PSTA_ADMIN_CONFIG apcli_entry = NULL;

	if (if_index >= MAX_MULTI_STA)
		return FALSE;

	apcli_entry = &adapter->StaCfg[if_index];

	if (eth_lookup_entry_by_addr(adapter, if_index, mac_addr, TRUE))
		return;
	MTWF_DBG(adapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				"%s added "MACSTR" if_index %d in list \n", __func__, MAC2STR(mac_addr), if_index);
	os_alloc_mem(adapter, (UCHAR **)&eth_entry, sizeof(Eth_CONNECT_ENTRY));

	if (eth_entry) {
		NdisZeroMemory(eth_entry, sizeof(A4_CONNECT_ENTRY));
		eth_entry->entry_flush_count = Eth_Entry_Flush_Time;
		memcpy(eth_entry->mac, mac_addr, MAC_ADDR_LEN);
		RTMP_SEM_LOCK(&apcli_entry->eth_entry_lock);
		DlListAddTail(&apcli_entry->eth_entry_list, &eth_entry->List);
		RTMP_SEM_UNLOCK(&apcli_entry->eth_entry_lock);
	} else
		MTWF_DBG(adapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Fail to alloc memory for New ethernet client Entry\n");
}

VOID eth_delete_entry(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
)
{
	PEth_CONNECT_ENTRY eth_entry = NULL;
	PSTA_ADMIN_CONFIG apcli_entry = NULL;
	PDL_LIST eth_entry_list = NULL;

	if (mac_addr == NULL)
		return FALSE;

	if (if_index >= MAX_MULTI_STA)
			return FALSE;

	apcli_entry = &adapter->StaCfg[if_index];

	eth_entry_list = &apcli_entry->eth_entry_list;
	RTMP_SEM_LOCK(&apcli_entry->eth_entry_lock);
	DlListForEach(eth_entry, eth_entry_list, Eth_CONNECT_ENTRY, List) {
		if (MAC_ADDR_EQUAL(mac_addr, eth_entry->mac)) {
			MTWF_DBG(adapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				"%s deleted "MACSTR" if_index %d in list \n", __func__, MAC2STR(mac_addr), if_index);
			DlListDel(&eth_entry->List);
			os_free_mem(eth_entry);
			break;
		}
	}
	RTMP_SEM_UNLOCK(&apcli_entry->eth_entry_lock);
}

VOID eth_update_list(
	IN PRTMP_ADAPTER adapter
)
{
	PEth_CONNECT_ENTRY eth_entry = NULL;
	PSTA_ADMIN_CONFIG apcli_entry = NULL;
	PDL_LIST eth_entry_list = NULL;
	UCHAR if_index;

	for (if_index = 0; if_index < MAX_MULTI_STA; if_index++) {

		apcli_entry = &adapter->StaCfg[if_index];

		if (apcli_entry->eth_list_init) {
			eth_entry_list = &apcli_entry->eth_entry_list;
			DlListForEach(eth_entry, eth_entry_list, Eth_CONNECT_ENTRY, List) {
				eth_entry->entry_flush_count--;
				if (eth_entry->entry_flush_count <= 0) {
					eth_delete_entry(adapter, if_index, eth_entry->mac);
					break;
				}
			}
		}
	}
}
#endif
#endif /* A4_CONN */
