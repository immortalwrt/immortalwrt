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
	hotspot.c

	Abstract:
	hotspot2.0 features

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"
#include "mcu/mt_cmd.h"
#include "hotspot.h"

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)

#ifdef CONFIG_HOTSPOT
void wext_hotspot_onoff_event(PNET_DEV net_dev, int onoff)
{
	struct hs_onoff *hotspot_onoff;
	UINT16 buflen = 0;
	char *buf;

	buflen = sizeof(*hotspot_onoff);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);
	hotspot_onoff = (struct hs_onoff *)buf;
	hotspot_onoff->ifindex = RtmpOsGetNetIfIndex(net_dev);
	hotspot_onoff->hs_onoff = onoff;
	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
							OID_802_11_HS_ONOFF, NULL, (PUCHAR)buf, buflen);
	os_free_mem(buf);
}


void HotspotOnOffEvent(PNET_DEV net_dev, int onoff)
{
	wext_hotspot_onoff_event(net_dev, onoff);
}


static void wext_hotspot_ap_reload_event(PNET_DEV net_dev)
{
	struct hs_onoff *hotspot_onoff;
	UINT16 buflen = 0;
	char *buf;

	buflen = sizeof(*hotspot_onoff);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);
	hotspot_onoff = (struct hs_onoff *)buf;
	hotspot_onoff->ifindex = RtmpOsGetNetIfIndex(net_dev);
	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
							OID_802_11_HS_AP_RELOAD, NULL, (PUCHAR)buf, buflen);
	os_free_mem(buf);
}


void HotspotAPReload(PNET_DEV net_dev)
{
	wext_hotspot_ap_reload_event(net_dev);
}

VOID HSCtrlRemoveAllIE(PHOTSPOT_CTRL pHSCtrl)
{
	/* Remove all IE from daemon */
	RTMP_SEM_LOCK(&pHSCtrl->IeLock);
	if (pHSCtrl->P2PIELen && pHSCtrl->P2PIE) {
		os_free_mem(pHSCtrl->P2PIE);
		pHSCtrl->P2PIE = NULL;
		pHSCtrl->P2PIELen = 0;
	}

	if (pHSCtrl->HSIndicationIELen && pHSCtrl->HSIndicationIE) {
		os_free_mem(pHSCtrl->HSIndicationIE);
		pHSCtrl->HSIndicationIE = NULL;
		pHSCtrl->HSIndicationIELen = 0;
	}

	if (pHSCtrl->QosMapSetIELen && pHSCtrl->QosMapSetIE) {
		os_free_mem(pHSCtrl->QosMapSetIE);
		pHSCtrl->QosMapSetIE = NULL;
		pHSCtrl->QosMapSetIELen = 0;
	}

	if (pHSCtrl->RoamingConsortiumIELen && pHSCtrl->RoamingConsortiumIE) {
		os_free_mem(pHSCtrl->RoamingConsortiumIE);
		pHSCtrl->RoamingConsortiumIE = NULL;
		pHSCtrl->RoamingConsortiumIELen = 0;
	}
	RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
}
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_HOTSPOT_R2
VOID hotspot_send_dls_resp(
	IN PRTMP_ADAPTER pAd,
	IN RX_BLK *pRxBlk)
{
	HEADER_802_11 Hdr80211;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UCHAR CategoryType, ActionType;
	USHORT StatusCode;
	UCHAR   DA[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
	UINT8 *Ptr = NULL;
#define MIN_DLS_PACKET_LEN 42 /* 80211_header(24)+Category(1)+Action_field(1)+SA(6)+DA(6)+capability(2)+DLS timeout(2) */

	if (GET_OS_PKT_LEN(pRxBlk->pRxPacket) < MIN_DLS_PACKET_LEN) {
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Invalid pkt len:%d return\n", GET_OS_PKT_LEN(pRxBlk->pRxPacket));
		return;
	}

	os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);
	if (pOutBuffer == NULL)
		return;

	Ptr = pRxBlk->FC;
	Ptr += sizeof(HEADER_802_11);
	/* offset to destination MAC address (Category and Action field) */
	Ptr += 2;
	/* get DA from payload and advance the pointer */
	NdisCopyMemory(DA, Ptr, MAC_ADDR_LEN);
	Ptr += MAC_ADDR_LEN;
	/* get SA from payload and advance the pointer */
	NdisCopyMemory(SA, Ptr, MAC_ADDR_LEN);
	Ptr += MAC_ADDR_LEN;

	ActHeaderInit(pAd, &Hdr80211, pRxBlk->Addr2,pRxBlk->Addr1,pRxBlk->Addr1);

	CategoryType = CATEGORY_DLS;
	ActionType = ACTION_DLS_RESPONSE;
	StatusCode = MLME_DLS_NOT_ALLOW_IN_QBSS;

	MakeOutgoingFrame(pOutBuffer, &FrameLen,
				sizeof(HEADER_802_11), &Hdr80211,
				1, &CategoryType,
				1, &ActionType,
				2, &StatusCode,
				MAC_ADDR_LEN, SA,
				MAC_ADDR_LEN, DA,
				END_OF_ARGS);

//	hex_dump_with_lvl("DLS Resp packet", pOutBuffer, FrameLen, DBG_LVL_INFO);

	MiniportMMRequest(pAd, QID_MGMT, pOutBuffer, FrameLen);
	os_free_mem(pOutBuffer);
}
#endif

#ifdef CONFIG_HOTSPOT
BOOLEAN HSIPv4Check(
	IN PRTMP_ADAPTER pAd,
	PUSHORT pWcid,
	PNDIS_PACKET pPacket,
	PUCHAR pSrcBuf,
	UINT16 srcPort,
	UINT16 dstPort)
{
	struct wifi_dev *wdev;
	BSS_STRUCT *pMbss;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
	struct wifi_dev_ops *ops;

	ASSERT(wdev_idx < WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX)
		return FALSE;

	wdev = pAd->wdev_list[wdev_idx];
	ops = wdev->wdev_ops;

	if (wdev->wdev_type == WDEV_TYPE_WDS) {
	    /* Return if WDEV Type is WDS */
	    return TRUE;
	}

	ASSERT(wdev->func_idx < pAd->ApCfg.BssidNum);
	pMbss =  &pAd->ApCfg.MBSSID[wdev->func_idx];

	if ((pMbss->HotSpotCtrl.HotSpotEnable)
#ifdef CONFIG_HOTSPOT_R2
		|| (pMbss->HotSpotCtrl.bASANEnable)
#endif
	   ) {
		if (srcPort  == 0x43 && dstPort == 0x44) {
			/* UCHAR *pTargetIPAddr = pSrcBuf + 24; */
			/* Client hardware address */
			UCHAR *pTargetMACAddr = pSrcBuf + 36;

			/* Convert group-address DHCP packets to individually-addressed 802.11 frames */
			if (*pWcid == wdev->bss_info_argument.bmc_wlan_idx && pMbss->HotSpotCtrl.DGAFDisable) {
				UCHAR Index;
				PUCHAR pSrcBufOriginal = GET_OS_PKT_DATAPTR(pPacket);

				for (Index = 0; Index < MAC_ADDR_LEN; Index++) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Original source address(%d) = %02x\n", Index, pSrcBufOriginal[Index]);
					pSrcBufOriginal[Index] = pTargetMACAddr[Index];
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Replaced Source address(%d) = %02x\n", Index, pSrcBuf[Index]);
				}

				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "\033[1;32m %s, %u wcid before convert %d \033[0m\n", __func__, __LINE__, *pWcid);
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Convert broadcast dhcp to unicat frame when dgaf disable\n");

				if (wdev_idx <= MAX_BEACON_NUM) {
					if (!ops->tx_pkt_allowed(pAd, &pAd->ApCfg.MBSSID[wdev_idx].wdev, pPacket)) {
						return FALSE;
					}
				}

				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "\033[1;32m %s, %u wcid after convert %d \033[0m\n", __func__, __LINE__, *pWcid);
			}
		}
	}

	return TRUE;
}

static BOOLEAN IsICMPv4EchoPacket(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pData)
{
	UINT16 ProtoType;
	UCHAR *Pos = pData;

	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IP) {
		Pos += 9;

		if (*Pos == 0x01) {
			Pos += 11;

			if (*Pos == 0x08) {
				Pos++;

				if (*Pos == 0x00) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ICMPv4Echp Packet\n");
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOLEAN L2FilterInspection(
	IN PRTMP_ADAPTER pAd,
	IN PHOTSPOT_CTRL pHSCtrl,
	IN PUCHAR pData)
{
	if (IsICMPv4EchoPacket(pAd, pData)) {
		if (pHSCtrl->ICMPv4Deny)
			return TRUE;
		else
			return FALSE;
	}

	return FALSE;
}

BOOLEAN ProbeReqforHSAP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR APIndex,
	IN struct _PEER_PROBE_REQ_PARAM *ProbeReqParam)
{
	PHOTSPOT_CTRL pHSCtrl;

	pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;

	if (pHSCtrl->HotSpotEnable && ProbeReqParam->IsIWIE) {
		if (ProbeReqParam->IsHessid && pHSCtrl->IsHessid) {
			if (NdisEqualMemory(ProbeReqParam->Hessid, pHSCtrl->Hessid, MAC_ADDR_LEN) ||
				NdisEqualMemory(ProbeReqParam->Hessid, BROADCAST_ADDR, MAC_ADDR_LEN))
				;
			else
				return FALSE;
		}

		if ((ProbeReqParam->AccessNetWorkType == pHSCtrl->AccessNetWorkType) ||
			(ProbeReqParam->AccessNetWorkType == 0x0f))
			return TRUE;
		else
			return FALSE;
	} else
		return TRUE;
}

inline INT Set_HotSpot_DGAF(
	IN PRTMP_ADAPTER pAd,
	UCHAR Disable)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[APIndex].wdev;

	pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
	pHSCtrl->DGAFDisable = Disable;
	/* for 7615 offload to CR4 */
	hotspot_update_bssflag(pAd, fgDGAFDisable, Disable, pHSCtrl);
	hotspot_update_bss_info_to_cr4(pAd, APIndex, wdev->bss_info_argument.ucBssIndex);
	return 0;
}

VOID Clear_Hotspot_All_IE(
	IN PRTMP_ADAPTER pAd)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl;
	PGAS_CTRL pGasCtrl;

	pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
	pGasCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;
	HSCtrlRemoveAllIE(pHSCtrl);
	GASCtrlRemoveAllIE(pGasCtrl);
}

VOID hotspot_bssflag_dump(UINT8 ucHotspotBSSFlags)
{
	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			  "pHSCtrl->HotspotEnable = %d\n"
			  "pHSCtrl->ProxyARPEnable = %d\n"
			  "pHSCtrl->ASANEnable = %d\n"
			  "pHSCtrl->DGAFDisable = %d\n"
			  "pHSCtrl->QosMapEnable = %d\n"
			  , IS_HOTSPOT_ENABLE(ucHotspotBSSFlags)
			  , IS_PROXYARP_ENABLE(ucHotspotBSSFlags)
			  , IS_ASAN_ENABLE(ucHotspotBSSFlags)
			  , IS_DGAF_DISABLE(ucHotspotBSSFlags)
			  , IS_QOSMAP_ENABLE(ucHotspotBSSFlags)
			  );
}
#endif

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
VOID hotspot_update_bssflag(RTMP_ADAPTER *pAd, UINT8 flag, UINT8 value, PHOTSPOT_CTRL pHSCtrl)
{
	if (value == 1)
		pHSCtrl->HotspotBSSFlags |= flag;
	else
		pHSCtrl->HotspotBSSFlags &= ~flag;

	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> flag 0x%x value %d pHSCtrl->HotspotBSSFlags 0x%x\n",
			 flag, value, pHSCtrl->HotspotBSSFlags);
	/* hotspot_bssflag_dump(pHSCtrl->HotspotBSSFlags); */
}

VOID hotspot_update_bss_info_to_cr4(RTMP_ADAPTER *pAd, UCHAR APIndex, UINT8 BssIndex)
{
	PHOTSPOT_CTRL pHSCtrl;
	/* 7615 offload to CR4 */
	MT_HOTSPOT_INFO_UPDATE_T HotspotInfoUpdateT;

	pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
	NdisZeroMemory(&HotspotInfoUpdateT, sizeof(HotspotInfoUpdateT));
	HotspotInfoUpdateT.ucUpdateType |= fgUpdateBssCapability;
	HotspotInfoUpdateT.ucHotspotBssFlags = pHSCtrl->HotspotBSSFlags;
#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
	HotspotInfoUpdateT.ucHotspotBssId = BssIndex;
#else
	HotspotInfoUpdateT.ucHotspotBssId = APIndex;
#endif
	MtCmdHotspotInfoUpdate(pAd, &HotspotInfoUpdateT);
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> Update BSS:%d  HotspotFlag:0x%x\n",
			 HotspotInfoUpdateT.ucHotspotBssId, HotspotInfoUpdateT.ucHotspotBssFlags);
	/* hotspot_bssflag_dump(pHSCtrl->HotspotBSSFlags); */
}
#endif

#ifdef CONFIG_HOTSPOT
VOID hotspot_add_qos_map_pool_to_cr4(RTMP_ADAPTER *pAd, UINT8 PoolID)
{
	MT_HOTSPOT_INFO_UPDATE_T HotspotInfoUpdateT;
	P_QOS_MAP_TABLE_T pQosMapPool = NULL;

	if (PoolID >= MAX_QOS_MAP_TABLE_SIZE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "PoolID %d excceed max table size %d..................................!!!!!!!!!!!!!!!!!!!!!!!!\n",
				  PoolID, MAX_QOS_MAP_TABLE_SIZE);
		return;
	}

	NdisZeroMemory(&HotspotInfoUpdateT, sizeof(HotspotInfoUpdateT));
	HotspotInfoUpdateT.ucUpdateType |= fgUpdateDSCPPool;
	pQosMapPool = &pAd->ApCfg.HsQosMapTable[PoolID];
	HotspotInfoUpdateT.ucPoolID = PoolID;
	HotspotInfoUpdateT.ucTableValid = pQosMapPool->ucPoolValid;
	HotspotInfoUpdateT.ucPoolDscpExceptionCount = pQosMapPool->ucDscpExceptionCount;
	HotspotInfoUpdateT.u4Ac = pQosMapPool->u4Ac;
	NdisCopyMemory(HotspotInfoUpdateT.au2PoolDscpRange, pQosMapPool->au2DscpRange,
					sizeof(pQosMapPool->au2DscpRange));
	NdisCopyMemory(HotspotInfoUpdateT.au2PoolDscpException, pQosMapPool->au2DscpException,
					sizeof(pQosMapPool->au2DscpException));
	MtCmdHotspotInfoUpdate(pAd, &HotspotInfoUpdateT);
	RtmpusecDelay(100);
}

VOID hotspot_qosmap_update_sta_mapping_to_cr4(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UINT8 PoolID)
{
	MT_HOTSPOT_INFO_UPDATE_T HotspotInfoUpdateT;

	if (PoolID >= MAX_QOS_MAP_TABLE_SIZE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "PoolID %d excceed max table size...........\n",
				  PoolID);
		return;
	}

	NdisZeroMemory(&HotspotInfoUpdateT, sizeof(HotspotInfoUpdateT));
	HotspotInfoUpdateT.ucUpdateType |= fgUpdateStaDSCP;
	HotspotInfoUpdateT.u2StaWcid = pEntry->wcid;
	HotspotInfoUpdateT.ucStaQosMapFlagAndIdx = PoolID;

	if (pEntry->QosMapSupport)
		HotspotInfoUpdateT.ucStaQosMapFlagAndIdx |= 0x80;
	else
		HotspotInfoUpdateT.ucStaQosMapFlagAndIdx &= 0x7f;

	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "wcid %d  HotspotInfoUpdateT.ucStaQosMapFlagAndIdx 0x%x...........\n",
			  HotspotInfoUpdateT.u2StaWcid, HotspotInfoUpdateT.ucStaQosMapFlagAndIdx);
	MtCmdHotspotInfoUpdate(pAd, &HotspotInfoUpdateT);
}


UINT8 hotspot_qosmap_add_pool(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	UINT8 i = 0;
	P_QOS_MAP_TABLE_T pQosMapPool = NULL;
	BOOLEAN found = FALSE;

	for (i = 0; i < MAX_QOS_MAP_TABLE_SIZE; i++) {
		pQosMapPool = &pAd->ApCfg.HsQosMapTable[i];

		if (pQosMapPool->ucPoolValid) {
			if ((pEntry->DscpExceptionCount == pQosMapPool->ucDscpExceptionCount) &&
				NdisEqualMemory(pEntry->DscpRange, pQosMapPool->au2DscpRange, 16) &&
				NdisEqualMemory(pEntry->DscpException, pQosMapPool->au2DscpException, 42)
			   ) {
				found = TRUE;
				break;
			}
		} else
			break;
	}

	if (found) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "=> Found existing QosMap Pool %d for sta wcid [%d]\n"
				  , i, pEntry->wcid);
		return i;
	} else if (i == MAX_QOS_MAP_TABLE_SIZE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "=> QosMap Pool Excceed %d, Using Default 0 for sta wcid [%d]\n"
				  , MAX_QOS_MAP_TABLE_SIZE, pEntry->wcid);
		return 0;
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "=> Insert new QosMap Pool %d for sta wcid [%d]\n"
				  , i, pEntry->wcid);
		pQosMapPool = &pAd->ApCfg.HsQosMapTable[i];
		NdisZeroMemory(pQosMapPool, sizeof(QOS_MAP_TABLE_T));
		pQosMapPool->ucPoolValid = TRUE;
		pQosMapPool->ucDscpExceptionCount = pEntry->DscpExceptionCount;
		NdisCopyMemory(pQosMapPool->au2DscpRange, pEntry->DscpRange, 16);
		NdisCopyMemory(pQosMapPool->au2DscpException, pEntry->DscpException, 42);
		/* send add pool event to CR4 */
		hotspot_add_qos_map_pool_to_cr4(pAd, i);
		set_cp_support_en(pAd, "1");

		return i;
	}
}


#endif
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_HOTSPOT
INT Set_HotSpot_OnOff(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OnOff,
	IN UINT8 EventTrigger,
	IN UINT8 EventType)
{
	UCHAR *Buf;
	HSCTRL_EVENT_DATA *Event;
	UINT32 Len = 0;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;

	/* to prevent HSCtrlON/OFF gets insanely called from UI */
	if (pHSCtrl->bHSOnOff == OnOff) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "wdev[%d] is already in [%s] STATE , skip.\n"
				  , APIndex, (OnOff == TRUE) ? "ON" : "OFF");
		return TRUE;
	} else
		pHSCtrl->bHSOnOff = OnOff;

#endif /* CONFIG_AP_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event));

	if (!Buf) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not available memory\n");
		return FALSE;
	}

	NdisZeroMemory(Buf, sizeof(*Event));
	Event = (HSCTRL_EVENT_DATA *)Buf;
#ifdef CONFIG_STA_SUPPORT
	Event->ControlIndex = 0;
#endif /*CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	Event->ControlIndex = APIndex;
#endif /* CONFIG_STA_SUPPORT */
	Len += 1;
	Event->EventTrigger = EventTrigger;
	Len += 1;
	Event->EventType = EventType;
	Len += 1;

	if (OnOff)
		MlmeEnqueue(pAd, HSCTRL_STATE_MACHINE, HSCTRL_ON, Len, Buf, 0);
	else
		MlmeEnqueue(pAd, HSCTRL_STATE_MACHINE, HSCTRL_OFF, Len, Buf, 0);

	os_free_mem(Buf);
	return TRUE;
}


enum HSCTRL_STATE HSCtrlCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHOTSPOT_CTRL pHSCtrl = NULL;
#ifdef CONFIG_AP_SUPPORT
	PHSCTRL_EVENT_DATA Event = (PHSCTRL_EVENT_DATA)Elem->Msg;
	pHSCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].HotSpotCtrl;
	/* Added NULL check and goto logic, to resolve coverity bug */
	if (pHSCtrl != NULL)
		goto label;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg[0].HotSpotCtrl;
	/* Added NULL check and goto logic, to resolve coverity bug */
	if (pHSCtrl != NULL)
		goto label;
#endif /* CONFIG_STA_SUPPORT */
label:
	return pHSCtrl->HSCtrlState;
}


VOID HSCtrlSetCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem,
	IN enum HSCTRL_STATE State)
{
	PHOTSPOT_CTRL pHSCtrl;
#ifdef CONFIG_AP_SUPPORT
	PHSCTRL_EVENT_DATA Event = (PHSCTRL_EVENT_DATA)Elem->Msg;
	pHSCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].HotSpotCtrl;
	/* Added the assignment for pHSCtrl->HSCtrlState inside macro to resolve
		coverity bug */
	pHSCtrl->HSCtrlState = State;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg[0].HotSpotCtrl;
	/* Added the assignment for pHSCtrl->HSCtrlState inside macro to resolve
		coverity bug */
	pHSCtrl->HSCtrlState = State;
#endif /* CONFIG_STA_SUPPORT */
}


static VOID HSCtrlOn(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHOTSPOT_CTRL pHSCtrl;
	PGAS_CTRL pGASCtrl;
	PNET_DEV NetDev;
	struct wifi_dev *wdev;
	HSCTRL_EVENT_DATA *Event = (HSCTRL_EVENT_DATA *)Elem->Msg;
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BSSID: %d\n", Event->ControlIndex);
#ifdef CONFIG_STA_SUPPORT
	NetDev = pAd->net_dev;
	pHSCtrl = &pAd->StaCfg[0].HotSpotCtrl;
	pGASCtrl = &pAd->StaCfg[0].GASCtrl;
	wdev = &pAd->StaCfg[0].wdev;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	NetDev = pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.if_dev;
	pHSCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].HotSpotCtrl;
	pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
	wdev = &pAd->ApCfg.MBSSID[Event->ControlIndex].wdev;
#endif /* CONFIG_AP_SUPPORT */
/*	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListInit(&pGASCtrl->GASPeerList);
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);*/
	pHSCtrl->HotSpotEnable = 1;
	/* for 7615 offload to CR4 */
	hotspot_update_bssflag(pAd, fgHotspotEnable, 1, pHSCtrl);
	hotspot_update_bss_info_to_cr4(pAd, Event->ControlIndex, wdev->bss_info_argument.ucBssIndex);
	/* no need to set WNMBTMEnable here, it was set by Set_HotSpot_Param */
	pGASCtrl->b11U_enable = 1;
	pHSCtrl->HSDaemonReady = 1;
#ifdef CONFIG_AP_SUPPORT
	UpdateBeaconHandler(pAd, &pAd->ApCfg.MBSSID[Event->ControlIndex].wdev, BCN_UPDATE_IE_CHG);
#endif /* CONFIG_AP_SUPPORT */
	HSCtrlSetCurrentState(pAd, Elem, HSCTRL_IDLE);

	/* Send indication to daemon */
	if (Event->EventTrigger) {
		switch (Event->EventType) {
		case HS_ON_OFF_BASE:
			HotspotOnOffEvent(NetDev, 1);
			break;

		case HS_AP_RELOAD:
			HotspotAPReload(NetDev);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown event type(%d)\n", Event->EventType);
			break;
		}
	}
}
#endif

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)

static VOID HSCtrlInit(
	IN PRTMP_ADAPTER pAd)
{
	PHOTSPOT_CTRL pHSCtrl;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#ifdef CONFIG_HOTSPOT
	P_QOS_MAP_TABLE_T pQosMapTable;
	UINT8 i = 0;
#endif
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg[0].HotSpotCtrl;
	NdisZeroMemory(pHSCtrl, sizeof(*pHSCtrl));
	pHSCtrl->HotSpotEnable = 0;
#ifdef CONFIG_HOTSPOT
	pHSCtrl->HSCtrlState = HSCTRL_IDLE;
	NdisAllocateSpinLock(pAd, &pHSCtrl->IeLock);
#endif
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
		pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
		NdisZeroMemory(pHSCtrl, sizeof(*pHSCtrl));
		pHSCtrl->HotSpotEnable = 0;
		/* for 7615 offload to CR4 */
		pHSCtrl->HotspotBSSFlags = 0;
#ifdef CONFIG_HOTSPOT
		pHSCtrl->HSCtrlState = HSCTRL_IDLE;
		NdisAllocateSpinLock(pAd, &pHSCtrl->IeLock);
#endif
	}
#ifdef CONFIG_HOTSPOT
	for (i = 0; i < MAX_QOS_MAP_TABLE_SIZE; i++) {
		pQosMapTable = &pAd->ApCfg.HsQosMapTable[i];
		NdisZeroMemory(pQosMapTable, sizeof(*pQosMapTable));
	}
#endif

#endif /* CONFIG_AP_SUPPORT */
}


VOID HSCtrlExit(
	IN PRTMP_ADAPTER pAd)
{
#ifdef CONFIG_HOTSPOT
	PHOTSPOT_CTRL pHSCtrl;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg[0].HotSpotCtrl;
	/* Remove all IE */
	HSCtrlRemoveAllIE(pHSCtrl);
	NdisFreeSpinLock(&pHSCtrl->IeLock);
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
		pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
		/* Remove all IE */
		HSCtrlRemoveAllIE(pHSCtrl);
		NdisFreeSpinLock(&pHSCtrl->IeLock);
	}

#endif /* CONFIG_AP_SUPPORT */
#endif
}
#endif

#ifdef CONFIG_HOTSPOT
VOID HSCtrlHalt(
	IN PRTMP_ADAPTER pAd)
{
	PHOTSPOT_CTRL pHSCtrl;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg[0].HotSpotCtrl;
	pHSCtrl->HotSpotEnable = 0;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
		pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
		pHSCtrl->HotSpotEnable = 0;
	}

#endif /* CONFIG_AP_SUPPORT */
}

static VOID HSCtrlOff(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHOTSPOT_CTRL pHSCtrl;
	PGAS_CTRL pGASCtrl;
	PNET_DEV NetDev;
	HSCTRL_EVENT_DATA *Event = (HSCTRL_EVENT_DATA *)Elem->Msg;
	struct wifi_dev *wdev;
#ifdef CONFIG_DOT11V_WNM
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
#endif
	UCHAR tmp;

	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BSSID %d\n", Event->ControlIndex);
#ifdef CONFIG_STA_SUPPORT
	NetDev = pAd->net_dev;
	pHSCtrl = &pAd->StaCfg[0].HotSpotCtrl;
	pGASCtrl = &pAd->StaCfg[0].GASCtrl;
	wdev = &pAd->StaCfg[0].wdev;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	NetDev = pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.if_dev;
	pHSCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].HotSpotCtrl;
	pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
	wdev = &pAd->ApCfg.MBSSID[Event->ControlIndex].wdev;
#ifdef CONFIG_HOTSPOT_R2
	pHSCtrl->bASANEnable = 0;
	pHSCtrl->QLoadTestEnable = 0;
#endif
#endif /* CONFIG_AP_SUPPORT */
	pHSCtrl->HotSpotEnable = 0;
	pHSCtrl->HSDaemonReady = 0;
	pHSCtrl->DGAFDisable = 0;
	pHSCtrl->L2Filter = 0;
	pHSCtrl->ICMPv4Deny = 0;
#ifdef CONFIG_DOT11V_WNM
	pWNMCtrl->ProxyARPEnable = 0;
#ifdef CONFIG_HOTSPOT_R2
	pWNMCtrl->WNMNotifyEnable = 0;
	pHSCtrl->QosMapEnable = 0;

	for (tmp = 0; tmp < 21; tmp++) {
		pHSCtrl->DscpException[tmp] = 0xff;
		pHSCtrl->DscpException[tmp] |= (0xff << 8);
	}

	for (tmp = 0; tmp < 8; tmp++) {
		pHSCtrl->DscpRange[tmp] = 0xff;
		pHSCtrl->DscpRange[tmp] |= (0xff << 8);
	}

#endif
#endif
#ifdef CONFIG_AP_SUPPORT
	UpdateBeaconHandler(pAd, &pAd->ApCfg.MBSSID[Event->ControlIndex].wdev, BCN_UPDATE_IE_CHG);
#endif /* CONFIG_AP_SUPPORT */
	/* for 7615 offload to CR4 */
	pHSCtrl->HotspotBSSFlags = 0;
	hotspot_update_bss_info_to_cr4(pAd, Event->ControlIndex, wdev->bss_info_argument.ucBssIndex);
	HSCtrlSetCurrentState(pAd, Elem, HSCTRL_IDLE);

	if (Event->EventTrigger) {
		switch (Event->EventType) {
		case HS_ON_OFF_BASE:
			HotspotOnOffEvent(NetDev, 0);
			break;

		case HS_AP_RELOAD:
			HotspotAPReload(NetDev);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown event type(%d)\n", Event->EventType);
			break;
		}
	}
}

BOOLEAN HotSpotEnable(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem,
	IN INT Type)
{
	PHOTSPOT_CTRL pHSCtrl = NULL;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
	PGAS_EVENT_DATA Event;
	GAS_FRAME *GASFrame;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (Type == GAS_STATE_MESSAGES) {
		Event =  (PGAS_EVENT_DATA)Elem->Msg;
		pHSCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].HotSpotCtrl;
	} else if (Type == ACTION_STATE_MESSAGES) {
		GASFrame = (GAS_FRAME *)Elem->Msg;

		for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
			/*
			according to 802.11-2012, public action frame may have Wildcard BSSID in addr3,
			use addr1(DA) for searching instead.
			*/
			if (MAC_ADDR_EQUAL(GASFrame->Hdr.Addr1, pAd->ApCfg.MBSSID[APIndex].wdev.bssid)) {
				pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;

				if (pHSCtrl->HotSpotEnable != TRUE) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "%s, %u [Disable] %02X:%02X:%02X:%02X:%02X:%02X\n",
							  __LINE__, PRINT_MAC(GASFrame->Hdr.Addr1));
				}

				break;
			}
		}

		if (!pHSCtrl) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Can not find Peer Control DA=%02X:%02X:%02X:%02X:%02X:%02X\n",
					  PRINT_MAC(GASFrame->Hdr.Addr1));
			return FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "can't recognize Type %d\n", Type);
		return FALSE;
	}
	/* Added NULL check and goto logic, to resolve coverity bug */
	if (pHSCtrl != NULL)
		goto label;

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg[0].HotSpotCtrl;
	/* Added NULL check and goto logic, to resolve coverity bug */
	if (pHSCtrl != NULL)
		goto label;
#endif /* CONFIG_STA_SUPPORT */
label:
	return pHSCtrl->HotSpotEnable;
}


VOID HSCtrlStateMachineInit(
	IN	PRTMP_ADAPTER		pAd,
	IN	STATE_MACHINE * S,
	OUT	STATE_MACHINE_FUNC	Trans[])
{
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	HSCtrlInit(pAd);
	StateMachineInit(S,	(STATE_MACHINE_FUNC *)Trans, MAX_HSCTRL_STATE, MAX_HSCTRL_MSG, (STATE_MACHINE_FUNC)Drop, HSCTRL_IDLE, HSCTRL_MACHINE_BASE);
	StateMachineSetAction(S, HSCTRL_IDLE, HSCTRL_ON, (STATE_MACHINE_FUNC)HSCtrlOn);
	StateMachineSetAction(S, HSCTRL_IDLE, HSCTRL_OFF, (STATE_MACHINE_FUNC)HSCtrlOff);
}
#endif

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)

BOOLEAN hotspot_rx_snoop(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk)
{
	BOOLEAN drop = FALSE;
	BOOLEAN FoundProxyARPEntry;
	BSS_STRUCT *pMbss = pEntry->pMbss;
	/* PUCHAR pData = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData; */
	PUCHAR pData = pRxBlk->pData + 12;
	UCHAR Offset = 0;

	/* Check if Proxy ARP Candidate for IPv4 */
	if (IsIPv4ProxyARPCandidate(pAd, pData)) {
		FoundProxyARPEntry = IPv4ProxyARP(pAd, pMbss, pData, FALSE, 1);

		if (FoundProxyARPEntry) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Find proxy entry for IPv4\n");
			drop = TRUE;
			goto done;
		}
	}

	/* Check if Neighbor solicitation during duplicate address detection procedure */
	if (IsIpv6DuplicateAddrDetect(pAd, pData, &Offset)) {
		/* Proxy MAC address/IPv6 mapping */
		/* AddIPv6ProxyARPEntry(pAd, pMbss, pEntry->Addr, (pData + 50)); */
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AddIPv6ProxyARPEntry:offset=%d\n", Offset);
		AddIPv6ProxyARPEntry(pAd, pMbss, pEntry->Addr, (pData + Offset), 1);
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Drop IPv6 DAD\n");
		drop = TRUE;
		goto done;
	}

	/* Check if Router solicitation */
	if (IsIPv6RouterSolicitation(pAd, pData)) {
		/* Proxy MAC address/IPv6 mapping for link local address */
		AddIPv6ProxyARPEntry(pAd, pMbss, pEntry->Addr,  (pData + 10), 1);
	}

	/* JERRY: add to parse DHCPv6 solicit to check proxy arp entry */
	if (IsIPv6DHCPv6Solicitation(pAd, pData))
		AddIPv6ProxyARPEntry(pAd, pMbss, pEntry->Addr,  (pData + 10), 1);

	/* Check if Proxy ARP Candidate for IPv6 */
	if (IsIPv6ProxyARPCandidate(pAd, pData)) {
		FoundProxyARPEntry = IPv6ProxyARP(pAd, pMbss, pData, FALSE, 1);

		if (FoundProxyARPEntry) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Find proxy entry for IPv6\n");
			drop = TRUE;
			goto done;
		} else {
			/* Proxy MAC address/IPv6 mapping */
			AddIPv6ProxyARPEntry(pAd, pMbss, pEntry->Addr, (pData + 10), 1);
		}
	}
#ifdef CONFIG_HOTSPOT
	if (!pEntry->pMbss->HotSpotCtrl.DGAFDisable) {
		if (IsGratuitousARP(pAd, pData, pRxBlk->Addr3, pMbss, 1)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Drop Gratutious ARP\n");
			drop = TRUE;
			goto done;
		}

		if (IsUnsolicitedNeighborAdver(pAd, pData)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Drop unsoclicited neighbor advertisement packet\n");
			drop = TRUE;
			goto done;
		}
	}
#endif

done:
	return drop;
}
#endif

#ifdef CONFIG_HOTSPOT
BOOLEAN hotspot_rx_l2_filter(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pRxEntry = NULL;
	BSS_STRUCT *pMbss = pEntry->pMbss;
	BOOLEAN drop = FALSE;
	PUCHAR pData_tdls = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData;
	PUCHAR pData = pRxBlk->pData + 12;
	pRxEntry = MacTableLookup(pAd, pRxBlk->Addr3);

	if (pEntry->pMbss->HotSpotCtrl.L2Filter == L2FilterBuiltIn) {
		BOOLEAN NeedDrop = FALSE;

		if (pRxEntry)
			NeedDrop = L2FilterInspection(pAd, &pMbss->HotSpotCtrl, pData);

		if (NeedDrop) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Drop Filter BuiltIn packet\n");
			drop = TRUE;
			goto done;
		}
	} else if (pMbss->HotSpotCtrl.L2Filter == L2FilterExternal) {
		UINT16 Index;
		BOOLEAN NeedSendToExternal;
		BSS_STRUCT *pMbss = pEntry->pMbss;
		PUCHAR pData = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData;

		NeedSendToExternal = L2FilterInspection(pAd, &pMbss->HotSpotCtrl, pData);

		if (NeedSendToExternal) {
			/* Change to broadcast DS */
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Change to broadcast under L2FilterExternal\n");

			for (Index = 0; Index < MAC_ADDR_LEN; Index++)
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DA[%d] = %x\n", Index, pRxBlk->Addr3[Index]);

			pRxBlk->Addr3[0] = 0xf0;
			pRxBlk->Addr3[1] = 0xde;
			pRxBlk->Addr3[2] = 0xf1;
			pRxBlk->Addr3[3] = 0x70;
			pRxBlk->Addr3[4] = 0x86;
			pRxBlk->Addr3[5] = 0x52;
		}
	}

	/* Check if TDLS/DLS frame */
	if (IsTDLSPacket(pAd, pData_tdls)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Drop TDLS Packet\n");
		drop = TRUE;
	}

done:
	return drop;
}
#endif
#ifdef CONFIG_HOTSPOT_R3
BOOLEAN hotspot_osu_data_handler(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk, struct wifi_dev *wdev)
{
	BOOLEAN drop = FALSE;
	BSS_STRUCT *pMbss,*pA3Mbss;
	MAC_TABLE_ENTRY *pA3Entry;

	pA3Entry = MacTableLookup(pAd, pRxBlk->Addr3);

	if (pA3Entry && wdev->SecConfig.bIsWPA2EntOSEN &&
			(pEntry->SecConfig.AKMMap != pA3Entry->SecConfig.AKMMap)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OSEN Ent enabled and different AKMs check for drop\n");
	} else {
#ifdef HOSTAPD_HS_R3_SUPPORT
		if (pA3Entry && pA3Entry->pMbss && pA3Entry->pMbss->osu_enable && (!NdisEqualMemory(pRxBlk->Addr2, pRxBlk->Addr3, 6))) {
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return TRUE;
		} else
#endif
			return drop;
	}

	if ((pEntry->pMbss) && (pA3Entry->pMbss)) {
		pMbss = pEntry->pMbss;
		pA3Mbss = pA3Entry->pMbss;
		if ((NdisEqualMemory(pRxBlk->Addr1, pA3Mbss->wdev.bssid, MAC_ADDR_LEN) == TRUE) ||
			(NdisEqualMemory(pRxBlk->Addr1, pMbss->wdev.bssid, MAC_ADDR_LEN))) {
			drop = TRUE;
		}
	}
	if (drop == TRUE)
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);

	return drop;
}
#endif

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
BOOLEAN hotspot_rx_handler(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk)
{
	BOOLEAN drop = FALSE;

	if (pEntry->pMbss->WNMCtrl.ProxyARPEnable)
		drop = hotspot_rx_snoop(pAd, pEntry, pRxBlk);

#ifdef CONFIG_HOTSPOT
	if ((drop == FALSE) && (pEntry->pMbss->HotSpotCtrl.L2Filter != L2FilterDisable))
		drop = hotspot_rx_l2_filter(pAd, pEntry, pRxBlk);
#endif
	if (drop == TRUE)
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);

	return drop;
}
#endif

#ifdef CONFIG_HOTSPOT
VOID hotspot_update_ap_qload_to_bcn(RTMP_ADAPTER *pAd)
{
	if ((pAd->Mlme.OneSecPeriodicRound % 2) == 0) {
		ULONG UpTime;
		UINT BssIdx;
		struct wifi_dev *wdev = NULL;
		BSS_STRUCT *pMbss = NULL;
		NdisGetSystemUpTime(&UpTime);
		QBSS_LoadUpdate(pAd, UpTime);

		for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
			wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;

			pMbss = wdev->func_dev;

			if (pMbss && pMbss->HotSpotCtrl.HotSpotEnable) {
				if (wdev->bAllowBeaconing)
					UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
			}
		}
	}

	return;
}
#endif

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
BOOLEAN hotspot_check_dhcp_arp(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket
)
{
	UINT16 TypeLen;
	UCHAR *pSrcBuf = NULL;
	USHORT Wcid = RTMP_GET_PACKET_WCID(pPacket);
	struct wifi_dev *wdev = NULL;
	UCHAR ApIndex;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Invalid wdev_idx(%d) return FALSE\n", wdev_idx);
		return FALSE;
	}
	wdev = pAd->wdev_list[wdev_idx];

	if (wdev && wdev->func_idx < pAd->ApCfg.BssidNum)
		ApIndex = wdev->func_idx;
	else
		return FALSE;

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	ASSERT(pSrcBuf);
	/* get Ethernet protocol field and skip the Ethernet Header */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	pSrcBuf += LENGTH_802_3;

	switch (TypeLen) {
	case ETH_TYPE_IPv4: {
		if (*(pSrcBuf + 9) == IP_PROTO_UDP) {
			UINT16 srcPort, dstPort;

			pSrcBuf += IP_HDR_LEN;
			srcPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf)));
			dstPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf + 2)));
			WNMIPv4ProxyARPCheck(pAd, pPacket, srcPort, dstPort, pSrcBuf, 1);

#ifdef CONFIG_HOTSPOT
			if (!HSIPv4Check(pAd, &Wcid, pPacket, pSrcBuf, srcPort, dstPort))
				return FALSE;
#endif

			return TRUE;
		}
	}
	break;

	case ETH_TYPE_ARP: {
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[ApIndex];

		if (pMbss->WNMCtrl.ProxyARPEnable) {
			/* Check if IPv4 Proxy ARP Candidate from DS */
			if (IsIPv4ProxyARPCandidate(pAd, pSrcBuf - 2)) {
				BOOLEAN FoundProxyARPEntry;

				FoundProxyARPEntry = IPv4ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE, 1);

				if (!FoundProxyARPEntry)
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Can not find proxy entry\n");

				return FALSE;
			}
		}
#ifdef CONFIG_HOTSPOT
		if (pMbss->HotSpotCtrl.HotSpotEnable) {
			if (!pMbss->HotSpotCtrl.DGAFDisable) {
				if (IsGratuitousARP(pAd, pSrcBuf - 2, pSrcBuf - 14, pMbss, 1)) {
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DROP %d\n", __LINE__);
					return FALSE;
				}
			}
		}
#endif
	}
	break;

	case ETH_P_IPV6: {
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[ApIndex];

		WNMIPv6ProxyARPCheck(pAd, pPacket, pSrcBuf, 1);

		if (pMbss->WNMCtrl.ProxyARPEnable) {
			/* Check if IPv6 Proxy ARP Candidate from DS */
			if (IsIPv6ProxyARPCandidate(pAd, pSrcBuf - 2)) {
				BOOLEAN FoundProxyARPEntry;

				FoundProxyARPEntry = IPv6ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE, 1);

				if (!FoundProxyARPEntry)
					MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Can not find IPv6 proxy entry\n");

				return FALSE;
			}
		}
#ifdef CONFIG_HOTSPOT
		if (pMbss->HotSpotCtrl.HotSpotEnable) {
			if (!pMbss->HotSpotCtrl.DGAFDisable) {
				if (IsUnsolicitedNeighborAdver(pAd, pSrcBuf - 2))
					return FALSE;
			}
		}
#endif
	}
	break;

	default:
		break;
	}

	return TRUE;
}
#endif
#endif

