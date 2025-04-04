

#include "rt_config.h"
#include "map.h"
#ifdef MAP_R2
#include <linux/if_vlan.h>
#endif

UCHAR MAP_OUI[3] = {0x50, 0x6F, 0x9A};
UCHAR MAP_OUI_TYPE[1] = {0x1B};
UCHAR MAP_EXT_ATTRI[1] = {0x06};
UCHAR MAP_ATTRI_LEN[1] = {1};

UCHAR multicast_mac_1905[MAC_ADDR_LEN] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x13};
#define MAP_EXT_ATTRIBUTE 0x06
#ifdef MAP_R2
UCHAR MAP_PROFILE_ATTRI = 0x07;
UCHAR MAP_PROFILE_LEN = 1;
UCHAR MAP_TRAFFIC_SEPARATION_ATTRI = 0x08;
UCHAR MAP_TRAFFIC_SEPARATION_LEN = 2;
#define MAP_PROFILE_ATTRIBUTE 0x07
#define MAP_TRAFFIC_SEPARATION_ATTRIBUTE 0x08
#endif

static UCHAR MAP_CheckDevRole(
	PRTMP_ADAPTER pAd,
	UCHAR wdev_type
)
{
	UCHAR res = 0;

	switch (wdev_type) {
	case WDEV_TYPE_AP:
		res = BIT(MAP_ROLE_FRONTHAUL_BSS); /* BH_BSS will be set by map cmd */
	break;

	/* case WDEV_TYPE_APCLI: */
	case WDEV_TYPE_STA:
		res = BIT(MAP_ROLE_BACKHAUL_STA);
	break;

	default:
		res = 0;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s():AP Role not set (Fixed me)\n", __func__);
	}

	return res;
}

INT MAP_InsertMapWscAttr(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf
)
{
	UCHAR MapVendorExt[10] = {0};
	UCHAR va[2] = {0x10, 0x49};
	UCHAR vl[2] = {0x00, 0x06};
	UCHAR vi[3] = {0x00, 0x37, 0x2A};

	/*WPS Vendor Extension */
	NdisMoveMemory(MapVendorExt, va, 2);
	NdisMoveMemory(MapVendorExt + 2, vl, 2);
	NdisMoveMemory(MapVendorExt + 4, vi, 3);
	NdisMoveMemory(MapVendorExt + 7, MAP_EXT_ATTRI, 1);
	NdisMoveMemory(MapVendorExt + 8, MAP_ATTRI_LEN, 1);
	NdisMoveMemory(MapVendorExt + 9, &wdev->MAPCfg.DevOwnRole, 1);

	NdisMoveMemory(pFrameBuf, MapVendorExt, sizeof(MapVendorExt));

	return sizeof(MapVendorExt);
}

VOID MAP_InsertMapCapIE(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen
)
{
	ULONG TmpLen = 0, total_len = 0;
	UCHAR IEType = IE_VENDOR_SPECIFIC;
	UCHAR IELen = 7;
	UCHAR MAP_EXT_ATTRI_LEN = 1;
	UCHAR MAP_EXT_ATTRI_VAL = wdev->MAPCfg.DevOwnRole;
#ifdef MAP_R2
	UCHAR MAP_EXT_PROFILE_VAL;
	UINT16 vid = 0;

	if (IS_MAP_R2_ENABLE(pAd)) {
		IELen += 3;
		if (wdev->wdev_type == WDEV_TYPE_AP && wdev->MAPCfg.vid_num > 0
#ifdef MAP_R4
		&& (wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS))
			&& (wdev->MAPCfg.primary_vid != INVALID_VLAN_ID)
#endif /* MAP_R4 */
		)
			IELen += 4;
	}
#endif
	MakeOutgoingFrame(pFrameBuf, &TmpLen,
						1, &IEType,
						1, &IELen,
						3, MAP_OUI,
						1, MAP_OUI_TYPE,
						1, MAP_EXT_ATTRI,
						1, &MAP_EXT_ATTRI_LEN,
						1, &MAP_EXT_ATTRI_VAL,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TmpLen;
	total_len += TmpLen;
#ifdef MAP_R2
	if (IS_MAP_R2_ENABLE(pAd)
#ifdef MAP_R3
		|| IS_MAP_R3_ENABLE(pAd)
#endif
	) {
		MAP_EXT_PROFILE_VAL = 0x02;
#ifdef MAP_R3
		/* modify the map profile val to 3 for MAP_R3 */
		if (IS_MAP_R3_ENABLE(pAd))
			MAP_EXT_PROFILE_VAL = 0x03;
#endif /* MAP_R3 */
		MakeOutgoingFrame(pFrameBuf + total_len, &TmpLen,
						1, &MAP_PROFILE_ATTRI,
						1, &MAP_PROFILE_LEN,
						1, &MAP_EXT_PROFILE_VAL,
						END_OF_ARGS);

		*pFrameLen = *pFrameLen + TmpLen;
		total_len += TmpLen;

		/*only add default 802.1q setting in assoc response*/
		if (wdev->wdev_type == WDEV_TYPE_AP && wdev->MAPCfg.vid_num > 0
			&& wdev->MAPCfg.primary_vid != INVALID_VLAN_ID) {
			vid = cpu2le16(wdev->MAPCfg.primary_vid);
			MakeOutgoingFrame(pFrameBuf + total_len, &TmpLen,
							1, &MAP_TRAFFIC_SEPARATION_ATTRI,
							1, &MAP_TRAFFIC_SEPARATION_LEN,
							2, &vid,
							END_OF_ARGS);

			*pFrameLen = *pFrameLen + TmpLen;
		}

	}
#endif
}


UINT32 map_rc_get_band_idx_by_chan(PRTMP_ADAPTER pad, UCHAR channel)
{
#ifdef DBDC_MODE

	/*not enable dbdc mode band should always in band0*/
	if (!pad->CommonCfg.dbdc_mode)
		return 0;

	/*enable dbdc mode, chose bandIdx from channel*/
	if (channel > 14)
		return BAND1;
	else
		return BAND0;
#endif /*DBDC_MODE*/
	return 0;
}

/* return map attribute*/
BOOLEAN map_check_cap_ie(
	IN PEID_STRUCT   eid,
	OUT  unsigned char *cap
#ifdef MAP_R2
	, OUT UCHAR *profile,
	OUT UINT16 *vid
#endif
)
{
	BOOLEAN Ret = FALSE;
	UCHAR *p, *p_old = NULL;
	UINT16 len = 0, ie_len = 0;
#ifdef MAP_R2
	*profile = 0;
	*vid = INVALID_VLAN_ID;
#endif
	if (NdisEqualMemory(eid->Octet, MAP_OUI, sizeof(MAP_OUI)) && (eid->Len >= 7)) {
		if (NdisEqualMemory((UCHAR *)&eid->Octet[3], MAP_OUI_TYPE, sizeof(MAP_OUI_TYPE))) {
			p = &eid->Octet[4];
			p_old = p;
			len = eid->Len - 4;
			Ret = TRUE;
			while (p - p_old < len) {
				ie_len = *(p + 1);
				switch (*p) {
				case MAP_EXT_ATTRIBUTE:
					if (ie_len != MAP_ATTRI_LEN[0]) {
						Ret = FALSE;
						break;
					}
					*cap = *(p + 2);
					break;
#ifdef MAP_R2
				case MAP_PROFILE_ATTRIBUTE:
					if (ie_len != MAP_PROFILE_LEN) {
						Ret = FALSE;
						break;
					}
					*profile = *(p + 2);
					break;
				case MAP_TRAFFIC_SEPARATION_ATTRIBUTE:
					if (ie_len != MAP_TRAFFIC_SEPARATION_LEN) {
						Ret = FALSE;
						break;
					}
					*vid = *((UINT16 *)(p + 2));
					*vid = le2cpu16(*vid);
					break;
#endif
				default:
					break;
				}
				p += ie_len + 2;
			}
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"[MAP] STA Attri = %02x\n", *cap);
#ifdef MAP_R2
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"[MAP] Profile = %02x, vid=%d\n", *profile, *vid);
#endif
			if (Ret == FALSE)
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"[MAP] !!!!!Invalid MAP IE\n");
		}
	}

	return Ret;
}

UCHAR getNonOpChnNum(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR op_class
)
{
	UCHAR i = 0, j = 0;
	UCHAR nonOpChnNum = 0, opChnNum = 0;
	UCHAR *opChList = get_channelset_by_reg_class(pAd, op_class, wdev->PhyMode);
	UCHAR opChListLen = get_channel_set_num(opChList);

	for (i = 0; i < opChListLen; i++) {
		for (j = -0; j < pAd->ChannelListNum; j++) {
			if (opChList[i] == pAd->ChannelList[j].Channel) {
				opChnNum++;
				break;
			}
		}
	}
	nonOpChnNum = opChListLen - opChnNum;

	return nonOpChnNum;
}

UCHAR getAutoChannelSkipListNum(
		IN PRTMP_ADAPTER pAd,
		IN struct wifi_dev *wdev)
{
	if (WMODE_CAP_6G(wdev->PhyMode))
		return pAd->ApCfg.AutoChannelSkipListNum6G;

	return pAd->ApCfg.AutoChannelSkipListNum;
}

VOID setNonOpChnList(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PCHAR nonOpChnList,
	IN UCHAR op_class,
	IN UCHAR nonOpChnNum
)
{
	UCHAR i = 0, j = 0, k = 0;
	BOOLEAN found = false;
	UCHAR *opChList = get_channelset_by_reg_class(pAd, op_class, wdev->PhyMode);
	UCHAR opChListLen = get_channel_set_num(opChList);

	if (nonOpChnNum > 0) {
		for (i = 0; i < opChListLen; i++) {
			for (j = -0; j < pAd->ChannelListNum; j++) {
				if (opChList[i] == pAd->ChannelList[j].Channel)
					found = true;
			}

			if (found == false) {
				nonOpChnList[k] = opChList[i];
				k++;
			} else
				found = false;
		}
	} else
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"No Non Op Channel\n");


}

#define MAX_CH_2G 14

VOID setAutoChannelSkipList(
		IN PRTMP_ADAPTER pAd,
		IN struct wifi_dev *wdev,
		IN wdev_chn_info * chn_list)
{

	UCHAR i = 0, j = 0;
	UCHAR AutoChannelSkipListNum = getAutoChannelSkipListNum(pAd, wdev);

	if (!WMODE_CAP_6G(wdev->PhyMode)) {
		for (i = 0; i < AutoChannelSkipListNum; i++) {
			if (WMODE_CAP_2G(wdev->PhyMode) && (pAd->ApCfg.AutoChannelSkipList[i] <= MAX_CH_2G))
				chn_list->AutoChannelSkipList[j++] = pAd->ApCfg.AutoChannelSkipList[i];
			else if (WMODE_CAP_5G(wdev->PhyMode) && (pAd->ApCfg.AutoChannelSkipList[i] > MAX_CH_2G))
				chn_list->AutoChannelSkipList[j++] = pAd->ApCfg.AutoChannelSkipList[i];
		}
	} else {
		for (i = 0; i < AutoChannelSkipListNum; i++)
			chn_list->AutoChannelSkipList[j++] = pAd->ApCfg.AutoChannelSkipList6G[i];
	}

	chn_list->AutoChannelSkipListNum = j;
}

int map_make_vend_ie(IN PRTMP_ADAPTER pAd, IN UCHAR ApIdx)
{
	struct vendor_map_element *ie = NULL;
	char *buf;
	int ie_len = 0;

	ie_len = sizeof(struct vendor_map_element);

	os_alloc_mem(NULL, (UCHAR **)&buf, sizeof(struct vendor_map_element));
	if (!buf) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"memory is not available\n");
		return -1;
	}
	NdisZeroMemory(buf, ie_len);
	ie = (struct vendor_map_element *)buf;

	ie->eid = VEND_IE_TYPE;
	ie->length = ie_len - 2;
	NdisCopyMemory(ie->oui, MTK_OUI, OUI_LEN);
	ie->mtk_ie_element[0] = 0;
	ie->mtk_ie_element[1] = 1;
	ie->type = 0;
	ie->subtype = 0;
	ie->root_distance = 0;
	ie->controller_connectivity = 0;
	ie->uplink_rate = 0;
	NdisZeroMemory(ie->_2g_bssid, ETH_ALEN);
	NdisZeroMemory(ie->_5g_bssid, ETH_ALEN);
	NdisZeroMemory(ie->uplink_bssid, ETH_ALEN);
	wapp_set_ap_ie(pAd, buf, ie_len, ApIdx);

	os_free_mem(buf);

	return 0;
}


VOID MAP_Init(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UCHAR wdev_type
)
{
	wdev->MAPCfg.DevOwnRole = MAP_CheckDevRole(pAd, wdev_type);
	wdev->MAPCfg.bUnAssocStaLinkMetricRptOpBss = TRUE;/*by default*/
	wdev->MAPCfg.bUnAssocStaLinkMetricRptNonOpBss = FALSE;/*by default*/
#ifdef MAP_R2
	wdev->MAPCfg.primary_vid = INVALID_VLAN_ID;
	wdev->MAPCfg.primary_pcp = 0x08;
	wdev->MAPCfg.vid_num = 0;
	wdev->MAPCfg.fh_vid = INVALID_VLAN_ID;
	NdisZeroMemory(wdev->MAPCfg.vids, sizeof(wdev->MAPCfg.vids));
	NdisZeroMemory(wdev->MAPCfg.bitmap_trans_vlan, sizeof(wdev->MAPCfg.bitmap_trans_vlan));
#endif
	pAd->ApCfg.SteerPolicy.steer_policy = 0;
	pAd->ApCfg.SteerPolicy.cu_thr = 0;
	pAd->ApCfg.SteerPolicy.rcpi_thr = 0;
	NdisZeroMemory(wdev->MAPCfg.vendor_ie_buf, VENDOR_SPECIFIC_LEN);
	NdisZeroMemory(&(wdev->MAPCfg.scan_bh_ssids), sizeof(struct scan_BH_ssids));
#ifdef MAP_R2
	/*// TODO: Raghav: enable per client Tx/Rx airtime calculation*/
#endif

}

INT map_send_bh_sta_wps_done_event(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY mac_entry,
	IN BOOLEAN is_ap)
{
	struct wifi_dev *wdev;
	struct wapp_event event;
	BOOLEAN send_event = FALSE;

	if (mac_entry) {

#ifdef APCLI_SUPPORT
		PSTA_ADMIN_CONFIG apcli_entry;
		struct wapp_bhsta_info *bsta_info = &event.data.bhsta_info;
#endif

		if (is_ap) {
			if (IS_MAP_ENABLE(adapter) && (mac_entry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)))
				send_event = TRUE;
		}
#ifdef APCLI_SUPPORT
		else {
			apcli_entry = GetStaCfgByWdev(adapter, mac_entry->wdev);
			if (IS_MAP_ENABLE(adapter) &&
				(mac_entry->DevPeerRole &
					(BIT(MAP_ROLE_FRONTHAUL_BSS) | BIT(MAP_ROLE_BACKHAUL_BSS)))) {
				COPY_MAC_ADDR(bsta_info->connected_bssid, apcli_entry->wdev.bssid);
				COPY_MAC_ADDR(bsta_info->mac_addr, apcli_entry->wdev.if_addr);
				bsta_info->peer_map_enable = 1;
				send_event = TRUE;
			} else {
				bsta_info->peer_map_enable = 0;
				send_event = TRUE;
			}
		}
#endif
		if (send_event) {
			wdev = mac_entry->wdev;
			if (wdev && wdev->if_dev) {
				event.event_id = MAP_BH_STA_WPS_DONE;
				event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
				wext_send_wapp_qry_rsp(adapter->net_dev, &event);
			}
		}
	}

	return 0;
}

void wapp_send_rssi_steer_event(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	char rssi_thrd)
{
	struct wifi_dev *wdev;
	wdev_steer_sta *str_sta;
	struct wapp_event event;

	/* send event to daemon */
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Trigger Rssi steering!\n");
	pEntry->isTriggerSteering = TRUE;

	wdev = pEntry->wdev;
	if (wdev && wdev->if_dev) {
		event.event_id = MAP_TRIGGER_RSSI_STEER;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		str_sta = &event.data.str_sta;
		COPY_MAC_ADDR(str_sta->mac_addr, pEntry->Addr);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
}

VOID map_rssi_status_check(
	IN PRTMP_ADAPTER pAd)
{
	int i = 0;
	char rssi_thrd = 0;

	if (pAd->ApCfg.SteerPolicy.steer_policy == AGENT_INIT_RSSI_STEER_ALLOW ||
		pAd->ApCfg.SteerPolicy.steer_policy == AGENT_INIT_RSSI_STEER_MANDATE) {
		rssi_thrd = (pAd->ApCfg.SteerPolicy.rcpi_thr >> 1) - 110;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

			if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
				if (pEntry->RssiSample.AvgRssi[0] < rssi_thrd) {
					pEntry->cur_rssi_status = BELOW_THRESHOLD;

					/*
					*	If sta's rssi is within RCPI tollenant boundary,
					*	ignore this rssi detection to avoid sending event
					*	to wapp constantly
					*/
					if (pEntry->isTriggerSteering == TRUE &&
						pEntry->pre_rssi_status == ABOVE_THRESHOLD &&
						pEntry->cur_rssi_status == BELOW_THRESHOLD &&
						abs(pEntry->RssiSample.AvgRssi[0] - rssi_thrd) <= RCPI_TOLLENACE)
						return;

					wapp_send_rssi_steer_event(pAd, pEntry, rssi_thrd);
				} else
					pEntry->cur_rssi_status = ABOVE_THRESHOLD;

				if (pEntry->pre_rssi_status == ABOVE_THRESHOLD &&
					pEntry->cur_rssi_status == BELOW_THRESHOLD &&
					abs(pEntry->RssiSample.AvgRssi[0] - rssi_thrd) > RCPI_TOLLENACE)
						pEntry->isTriggerSteering = FALSE;

				pEntry->pre_rssi_status = pEntry->cur_rssi_status;
			}
		}
	}
}

INT ReadSRMeshUlModeParameterFromFile(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer)
{
	INT  i = 0;
	CHAR *value = 0;

	if (RTMPGetKeyParameter("SRMeshUlMode", tmpbuf, 32, pBuffer, TRUE)) {
		/* parameter parsing */
		for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode) {
				switch (i) {
				case 0:
					pAd->CommonCfg.SRMeshUlMode[BAND0] = simple_strtoul(value, 0, 10);
					break;

				case 1:
					pAd->CommonCfg.SRMeshUlMode[BAND1] = simple_strtoul(value, 0, 10);
					break;

				default:
					break;
				}
			} else {
				switch (i) {
				case 0:
					pAd->CommonCfg.SRMeshUlMode[BAND0] = simple_strtoul(value, 0, 10);
					break;

				default:
					break;
				}
			}

#else

			switch (i) {
			case 0:
				pAd->CommonCfg.SRMeshUlMode[BAND0] = simple_strtoul(value, 0, 10);
				break;

			default:
				break;
			}

#endif /* DBDC_MODE */
		}

#ifdef DBDC_MODE

		if (pAd->CommonCfg.dbdc_mode)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[SRMeshUlMode] BAND0: %d, BAND1: %d\n",
				pAd->CommonCfg.SRMeshUlMode[BAND0],
				pAd->CommonCfg.SRMeshUlMode[BAND1]);
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[SRMeshUlMode] BAND0: %d\n",
				pAd->CommonCfg.SRMeshUlMode[BAND0]);

#else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"[SRMeshUlMode] BAND0: %d\n",
			pAd->CommonCfg.SRMeshUlMode[BAND0]);
#endif /* DBDC_MODE */
	}

	return TRUE;
}


INT ReadMapParameterFromFile(
    PRTMP_ADAPTER pAd,
    RTMP_STRING *tmpbuf,
    RTMP_STRING *pBuffer)
{
#ifdef CONFIG_MAP_SUPPORT
	if (RTMPGetKeyParameter("MapMode", tmpbuf, 25, pBuffer, TRUE)) {
		pAd->MAPMode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MAP_MODE=%d\n", pAd->MAPMode);
	}
#ifdef CONFIG_MAP_3ADDR_SUPPORT
	if (RTMPGetKeyParameter("MapAccept3Addr", tmpbuf, 25, pBuffer, TRUE)) {
		pAd->MapAccept3Addr = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MapAccept3Addr=%d\n", pAd->MapAccept3Addr);
	}
#endif
	if (RTMPGetKeyParameter("MapBalance", tmpbuf, 25, pBuffer, TRUE)) {
		pAd->MapBalance = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"MAP_BALANCE=%u\n", pAd->MapBalance);
	}

	ReadSRMeshUlModeParameterFromFile(pAd, tmpbuf, pBuffer);

#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
		if (IS_MAP_TURNKEY_ENABLE(pAd))
			pAd->ApCfg.bRoamingEnhance = TRUE;
#endif
#endif
		if (IS_MAP_TURNKEY_ENABLE(pAd)) {
			int j;
			for (j = BSS0; j < WDEV_NUM_MAX; j++) {
				struct wifi_dev *wdev = pAd->wdev_list[j];
#ifdef CONFIG_MAP_SUPPORT
				if (wdev && wdev->wdev_type == WDEV_TYPE_AP)
					map_make_vend_ie(pAd, (UCHAR)wdev->func_idx);
#endif /* CONFIG_MAP_SUPPORT */
				if (wdev && wdev->wdev_type == WDEV_TYPE_STA) {
						if (wdev->func_idx >= MAX_APCLI_NUM)
							continue;
						SetApCliEnableByWdev(pAd, wdev, FALSE);
				}
			}
		}

#ifdef CONFIG_RCSA_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd))
		pAd->CommonCfg.DfsParameter.bRCSAEn = FALSE;
#endif
#endif /* CONFIG_MAP_SUPPORT */
	return TRUE;
}
#ifdef MAP_R2
UINT32 is_vid_configed(UINT16 vid, UINT32 vids[])
{
	return vids[vid / 32] & BIT(vid % 32);
}
#endif

#ifdef MAP_TS_TRAFFIC_SUPPORT

#define PKT_TYPE_1905 0x893a
#define ETH_TYPE_SVLAN  0X88A8

BOOLEAN get_vlanid_from_pkt(PNDIS_PACKET pkt, UINT16 *pvlanid)
{
	struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);
	struct vlan_ethhdr *veth = (struct vlan_ethhdr *)skb->data;

	if (veth->h_vlan_proto != OS_HTONS(ETH_P_8021Q) &&
		veth->h_vlan_proto != OS_HTONS(ETH_TYPE_SVLAN))
		return FALSE;

	*pvlanid = (OS_NTOHS(veth->h_vlan_TCI) & 0x0FFF);
	return TRUE;
}

static inline VOID remove_vlan_tag(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UCHAR *pSrcBuf;
	UINT16 VLAN_LEN = 4;
	UCHAR extra_field_offset = 2 * ETH_ALEN;
	struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	memmove(GET_OS_PKT_DATAPTR(pkt) + VLAN_LEN,
		GET_OS_PKT_DATAPTR(pkt), extra_field_offset);
	RtmpOsSkbPullRcsum(RTPKT_TO_OSPKT(pkt), 4);
	RtmpOsSkbResetMacHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetNetworkHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetTransportHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetMacLen(RTPKT_TO_OSPKT(pkt));
	skb->vlan_tci = 0;
}

BOOLEAN add_vlan_tag(void *packet, UINT16 vlan_id, UCHAR vlan_pcp)
{
	struct sk_buff *skb = (struct sk_buff *)packet;
	UINT16 vlan_tci = 0;

	vlan_tci |= 0x0fff & vlan_id;
	vlan_tci |= vlan_pcp << 13;

	skb = vlan_insert_tag(skb, htons(ETH_P_8021Q), vlan_tci);
	if (skb) {
		skb->protocol = htons(ETH_P_8021Q);
		skb->vlan_tci = vlan_tci;
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOLEAN is_ts_configed(struct wifi_dev *wdev)
{
	if (wdev->MAPCfg.primary_vid != INVALID_VLAN_ID ||
		wdev->MAPCfg.vid_num != 0)
		return TRUE;

	return FALSE;
}

static inline UINT16 map_get_dev_vid(struct wifi_dev *wdev)
{
	if (!wdev)
		return 0;
	if (IS_VALID_VID(wdev->MAPCfg.fh_vid))
		return wdev->MAPCfg.fh_vid;
	else if (IS_VALID_VID(wdev->MAPCfg.primary_vid))
		return wdev->MAPCfg.primary_vid;
	else
		return 0;
}

BOOLEAN map_ts_tx_process(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					PNDIS_PACKET pkt, struct _MAC_TABLE_ENTRY *peer_entry)
{
	UINT16 pkt_vid = 0, conf_vid = 0;
	BOOLEAN vlan_tagged = FALSE;
	UCHAR *pSrcBuf = NULL;
	UINT16 pkt_type = 0;

	if (!IS_MAP_R2_ENABLE(pAd))
		goto suc;
	conf_vid = map_get_dev_vid(wdev);
	vlan_tagged = get_vlanid_from_pkt(pkt, &pkt_vid);
	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);

	pkt_type = (pSrcBuf[12] << 8) | pSrcBuf[13];
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"() on %s, DevPeerRole=%02x, profile=%02x, vlan_tagged:%d, DA:"MACSTR" SA:"MACSTR", pkt_type0x%x pkt_vid(%d) conf_vid(%d)\n",
		wdev->if_dev->name, peer_entry->DevPeerRole, peer_entry->profile, vlan_tagged,
		MAC2STR(pSrcBuf), MAC2STR(pSrcBuf+6), pkt_type, pkt_vid, conf_vid);

	/*pass through all vlan tagged packet with transparent vlan id*/
	if (vlan_tagged && is_vid_configed(pkt_vid, wdev->MAPCfg.bitmap_trans_vlan))
		goto suc;

	if (!is_ts_configed(wdev))
		goto suc;

	if (vlan_tagged) {
		if (peer_entry->DevPeerRole == 0) {
			/*normal sta*/
			if (pkt_vid == conf_vid) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"map_ts_tx_process %s remove the matching vid=%d for station\n",
					wdev->if_dev->name, pkt_vid);
				remove_vlan_tag(pAd, pkt);
			} else
				goto fail;
		} else if (peer_entry->profile < 0x02) {
			if (pkt_vid != wdev->MAPCfg.primary_vid) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"map_ts_tx_process %s drop pkts with vid(%d) not equal to primary vlan(%d)\n",
					wdev->if_dev->name, pkt_vid, wdev->MAPCfg.primary_vid);
				goto fail;
			}
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"map_ts_tx_process %s remove tag for r1 vid=%d\n",
					wdev->if_dev->name, pkt_vid);
			/*map r1 device*/
			remove_vlan_tag(pAd, pkt);
		} else if (peer_entry->profile >= 0x02) {
			/*	map r2 device check whether the vid is included
			 *	in the recent received ts policy-TBD
			 */
		}
	} else {
		if (peer_entry->profile >= 0x02) {
			/*	should we assume that if primary vlan id has been
			 *	configured, we should add the primary vid to these
			 *	packets without any vlan tags, including the 1905 and
			 *	EAPOL message???
			 */
			/* tag all packets sending on R2/R3 Backhual */
			if (IS_VALID_VID(wdev->MAPCfg.primary_vid)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"map_ts_tx_process %s add tag for r2 vid=%d to untaged ptk\n",
						wdev->if_dev->name, wdev->MAPCfg.primary_vid);
				/*add primary vlan id for 1905 message*/
				if (!add_vlan_tag(pkt, wdev->MAPCfg.primary_vid, wdev->MAPCfg.primary_pcp))
					goto fail;
			}
		}
	}
suc:
	return TRUE;
fail:
	return FALSE;
}

BOOLEAN map_ts_rx_process(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					PNDIS_PACKET pkt, struct _MAC_TABLE_ENTRY *peer_entry)
{

	UINT16 pkt_vid = 0;
	BOOLEAN vlan_tagged = FALSE;
	UCHAR *pSrcBuf;
	UINT16 pkt_type = 0;

	if (!IS_MAP_R2_ENABLE(pAd))
		goto suc;

	vlan_tagged = get_vlanid_from_pkt(pkt, &pkt_vid);
	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);

	pkt_type = (pSrcBuf[12] << 8) | pSrcBuf[13];
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"() on %s, DevPeerRole=%02x, profile=%02x, vlan_tagged:%d, DA:"MACSTR", SA:"MACSTR", pkt_type0x%x pkt_vid(%d)\n",
		wdev->if_dev->name, peer_entry->DevPeerRole, peer_entry->profile, vlan_tagged,
		MAC2STR(pSrcBuf), MAC2STR(pSrcBuf+6), pkt_type, pkt_vid);

	if (vlan_tagged && is_vid_configed(pkt_vid, wdev->MAPCfg.bitmap_trans_vlan))
		goto suc;

	if (!is_ts_configed(wdev))
		goto suc;

	if (vlan_tagged) {
		if (peer_entry->DevPeerRole == 0 ||
			peer_entry->profile < 0x02) {
			/*	normal sta drop the packet with vlan which is send from
			 *	normal station or map r1 device
			 */
			if (wdev->MAPCfg.fh_vid != INVALID_VLAN_ID) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s drop ptk vid=%d,entry=%p,DevPeerRole=%02x, profile=%02x\n",
					wdev->if_dev->name, pkt_vid,
					peer_entry, peer_entry->DevPeerRole,
					peer_entry->profile);
				goto fail;
			}
		} else if (peer_entry->profile >= 0x02) {
			/*	map r2 device check whether the vid is included
			 *	in the recent received ts policy
			*/
			if (is_ts_configed(wdev) &&
				!is_vid_configed(pkt_vid, wdev->MAPCfg.vids) &&
				wdev->MAPCfg.primary_vid != pkt_vid) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"map_ts_rx_process %s drop ptk with vid =%d that is not in ts policy\n",
					wdev->if_dev->name, pkt_vid);
				goto fail;
			}
		}
	} else {
		if (peer_entry->DevPeerRole == 0) {
			/*if this bss has been configured, add the corresponding vid*/
			if (IS_VALID_VID(wdev->MAPCfg.fh_vid) ||
				IS_VALID_VID(wdev->MAPCfg.primary_vid)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"map_ts_rx_process %s add for station vid=%d\n",
					wdev->if_dev->name, wdev->MAPCfg.fh_vid);
				if (!add_vlan_tag(pkt, IS_VALID_VID(wdev->MAPCfg.fh_vid) ?
					wdev->MAPCfg.fh_vid : wdev->MAPCfg.primary_vid, wdev->MAPCfg.primary_pcp))
					goto fail;
			}
		} else if (peer_entry->profile < 0x02) {
			if (IS_VALID_VID(wdev->MAPCfg.primary_vid)) {
				/*if received a packet from map r1 device, add a vlan tag*/
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"map_ts_rx_process %s add for R1 with primary vid=%d\n",
						wdev->if_dev->name,
						wdev->MAPCfg.primary_vid);
				if (!add_vlan_tag(pkt, wdev->MAPCfg.primary_vid, wdev->MAPCfg.primary_pcp))
					goto fail;
			}
		} else {
			/*	if received a packet without vlan from map r2 device,
			 *	waht should do??
			 */
		}
	}

suc:
	return TRUE;
fail:
	return FALSE;
}
#endif

#ifdef A4_CONN
BOOLEAN map_a4_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN BOOLEAN is_ap /*if i'm AP or not*/
)
{
#ifdef APCLI_SUPPORT
	PSTA_ADMIN_CONFIG apcli_entry;
#endif

	if (is_ap) {
		if (IS_MAP_ENABLE(adapter) &&
			(entry->wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
			(entry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)))
			return a4_ap_peer_enable(adapter, entry, A4_TYPE_MAP);
	}
#ifdef APCLI_SUPPORT
	else {
		apcli_entry = GetStaCfgByWdev(adapter, entry->wdev);
		if (IS_MAP_ENABLE(adapter) &&
			(entry->DevPeerRole & (BIT(MAP_ROLE_BACKHAUL_BSS)))) {
			return a4_apcli_peer_enable(adapter,
										apcli_entry,
										entry,
										A4_TYPE_MAP);
		}
	}
#endif

	return FALSE;
}

BOOLEAN map_a4_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN BOOLEAN is_ap /*if i'm AP or not*/
)
{
	if (is_ap)
		return a4_ap_peer_disable(adapter, entry, A4_TYPE_MAP);
#ifdef APCLI_SUPPORT
	else
		return a4_apcli_peer_disable(adapter, GetStaCfgByWdev(adapter, entry->wdev), entry, A4_TYPE_MAP);
#else
	return FALSE;
#endif
}


BOOLEAN map_a4_init(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
)
{
	return a4_interface_init(adapter, if_index, is_ap, A4_TYPE_MAP);
}


BOOLEAN map_a4_deinit(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
)
{
	return a4_interface_deinit(adapter, if_index, is_ap, A4_TYPE_MAP);
}
BOOLEAN MapNotRequestedChannel(struct wifi_dev *wdev, unsigned char channel)
{
	int i = 0;

	if (wdev->MAPCfg.scan_bh_ssids.scan_channel_count == 0)
		return FALSE;
	for (i = 0; i < wdev->MAPCfg.scan_bh_ssids.scan_channel_count; i++) {
		if (channel == wdev->MAPCfg.scan_bh_ssids.scan_channel_list[i])
			return FALSE;
	}
	return TRUE;
}
#endif

/* Blacklist for BS2.0 */
#ifdef MAP_BL_SUPPORT
BOOLEAN map_is_entry_bl(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR apidx)
{
	PLIST_HEADER pBlackList = &pAd->ApCfg.MBSSID[apidx].BlackList;
	RT_LIST_ENTRY *pListEntry = pBlackList->pHead;
	PBS_BLACKLIST_ENTRY	pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;

	while (pBlEntry != NULL) {
		if (NdisEqualMemory(pBlEntry->addr, pAddr, MAC_ADDR_LEN))
			return TRUE;

		pListEntry = pListEntry->pNext;
		pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;
	}

	return FALSE;
}

PBS_BLACKLIST_ENTRY	map_find_bl_entry(
	IN  PLIST_HEADER pBlackList,
	IN  PUCHAR pMacAddr)
{
	PBS_BLACKLIST_ENTRY	pBlEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pBlackList->pHead;
	pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;

	while (pBlEntry != NULL) {
		if (NdisEqualMemory(pBlEntry->addr, pMacAddr, MAC_ADDR_LEN))
			return pBlEntry;

		pListEntry = pListEntry->pNext;
		pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;
	}

	return NULL;
}

VOID map_blacklist_add(
	IN  PLIST_HEADER pBlackList,
	IN  PUCHAR pMacAddr)
{
	PBS_BLACKLIST_ENTRY pBlEntry = NULL;

	pBlEntry = map_find_bl_entry(pBlackList, pMacAddr);

	if (pBlEntry) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "entry already presnet\n");
	} else {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "New entry add\n");
		os_alloc_mem(NULL, (UCHAR **)&pBlEntry, sizeof(BS_BLACKLIST_ENTRY));
		if (pBlEntry) {
			NdisZeroMemory(pBlEntry, sizeof(BS_BLACKLIST_ENTRY));
			NdisMoveMemory(pBlEntry->addr, pMacAddr, MAC_ADDR_LEN);
			insertTailList(pBlackList, (RT_LIST_ENTRY *)pBlEntry);
		}
		ASSERT(pBlEntry != NULL);
	}
}

VOID map_blacklist_del(
	IN  PLIST_HEADER pBlackList,
	IN  PUCHAR pMacAddr)
{
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = (RT_LIST_ENTRY *)map_find_bl_entry(pBlackList, pMacAddr);

	if (pListEntry) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"[%s] : pMacAddr = "MACSTR"\n", __func__, MAC2STR(pMacAddr));
		delEntryList(pBlackList, pListEntry);
		os_free_mem(pListEntry);
	} else {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"[%s] : Entry not present in list ["MACSTR"]\n", __func__, MAC2STR(pMacAddr));
	}
}

VOID map_blacklist_show(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR apidx)
{
	BSS_STRUCT *pBss = &pAd->ApCfg.MBSSID[apidx];
	PLIST_HEADER pBlackList = &pBss->BlackList;
	PBS_BLACKLIST_ENTRY	pBlEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	if (pBlackList->size != 0) {
		RTMP_SEM_LOCK(&pBss->BlackListLock);
		pListEntry = pBlackList->pHead;
		pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;
		while (pBlEntry != NULL) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"STA :: "MACSTR"\n", MAC2STR(pBlEntry->addr));
			pListEntry = pListEntry->pNext;
			pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;
		}
		RTMP_SEM_UNLOCK(&pBss->BlackListLock);
	}
}
#endif /*  MAP_BL_SUPPORT */
