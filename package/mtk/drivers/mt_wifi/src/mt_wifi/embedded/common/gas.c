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
	gas.c

	Abstract:
	generic advertisement service(GAS)

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"


enum DOT11U_ADVERTISMENT_PROTOCOL_ID dot11GASAdvertisementID[] = {
	ACCESS_NETWORK_QUERY_PROTOCOL,
};

static VOID SendGASIndication(
	IN PRTMP_ADAPTER    pAd,
	GAS_EVENT_DATA *Event);


enum GAS_STATE GASPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PGAS_CTRL pGASCtrl;
	PGAS_PEER_ENTRY GASPeerEntry;
	PGAS_EVENT_DATA Event = (PGAS_EVENT_DATA)Elem->Msg;
#ifdef CONFIG_AP_SUPPORT
	pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {

		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
			RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
			return GASPeerEntry->CurrentState;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
	return GAS_UNKNOWN;
}


VOID GASSetPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	/* IN MLME_QUEUE_ELEM *Elem, */
	PGAS_EVENT_DATA Event,
	IN enum GAS_STATE State)
{
	PGAS_CTRL pGASCtrl;
	PGAS_PEER_ENTRY GASPeerEntry;
	/* PGAS_EVENT_DATA Event = (PGAS_EVENT_DATA)Elem->Msg; */
#ifdef CONFIG_AP_SUPPORT
	pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
#endif /* CONFIG_AP_SUPPORT */
	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
			GASPeerEntry->CurrentState = State;
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
}

VOID GASCtrlRemoveAllIE(PGAS_CTRL pGasCtrl)
{
	RTMP_SEM_LOCK(&pGasCtrl->IeLock);
	if (pGasCtrl->InterWorkingIELen && pGasCtrl->InterWorkingIE) {
		os_free_mem(pGasCtrl->InterWorkingIE);
		pGasCtrl->InterWorkingIE = NULL;
		pGasCtrl->InterWorkingIELen = 0;
	}

	if (pGasCtrl->AdvertisementProtoIELen && pGasCtrl->AdvertisementProtoIE) {
		os_free_mem(pGasCtrl->AdvertisementProtoIE);
		pGasCtrl->AdvertisementProtoIE = NULL;
		pGasCtrl->AdvertisementProtoIELen = 0;
	}
	RTMP_SEM_UNLOCK(&pGasCtrl->IeLock);
}

#ifdef CONFIG_AP_SUPPORT
void wext_send_locationIE_to_daemon_event(PNET_DEV net_dev, const char *location_buf,
		UINT16 location_buf_len, UINT16 info_id)
{
	struct location_IE *query_data;
	u16 buflen = 0;
	char *buf;

	buflen = sizeof(*query_data) + location_buf_len;
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);
	query_data = (struct location_IE *)buf;
	/* query_data->ifindex = RtmpOsGetNetIfIndex(net_dev); */
	query_data->len = location_buf_len;
	query_data->type = info_id;
	memcpy(query_data->location_buf, location_buf, location_buf_len);
	/* printk("!!!!!!!!!!!!!!!!!!!!!!!location_buf_len: %d ,buflen: %d\n",location_buf_len,buflen); */
	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
							OID_802_11_HS_LOCATION_DRV_INFORM_IE, NULL, (PUCHAR)buf, buflen);
	os_free_mem(buf);
}


void SendLocationElementEvent(PNET_DEV net_dev, const char *location_buf,
							  UINT16 location_buf_len, UINT16 info_id)
{
	wext_send_locationIE_to_daemon_event(net_dev,
										 location_buf,
										 location_buf_len,
										 info_id);
}

void wext_send_anqp_req_event(PNET_DEV net_dev, const char *peer_mac_addr,
							  const char *anqp_req, UINT16 anqp_req_len)
{
	struct anqp_req_data *req_data;
	UINT16 buflen = 0;
	char *buf;

	buflen = sizeof(*req_data) + anqp_req_len;
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);
	req_data = (struct anqp_req_data *)buf;
	req_data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(req_data->peer_mac_addr, peer_mac_addr, 6);
	req_data->anqp_req_len	= anqp_req_len;
	memcpy(req_data->anqp_req, anqp_req, anqp_req_len);
	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
							OID_802_11_HS_ANQP_REQ, NULL, (PUCHAR)buf, buflen);
	os_free_mem(buf);
}


void SendAnqpReqEvent(PNET_DEV net_dev, const char *peer_mac_addr,
					  const char *anqp_req, UINT16 anqp_req_len)
{
	wext_send_anqp_req_event(net_dev,
							 peer_mac_addr,
							 anqp_req,
							 anqp_req_len);
}


/* static VOID SendGASRsp( */
/* IN PRTMP_ADAPTER    pAd, */
/* IN MLME_QUEUE_ELEM  *Elem) */
VOID SendGASRsp(
	IN PRTMP_ADAPTER    pAd,
	GAS_EVENT_DATA *Event)
{
	/* GAS_EVENT_DATA *Event = (GAS_EVENT_DATA *)Elem->Msg; */
	UCHAR *Buf, *Pos;
	GAS_FRAME *GASFrame;
	ULONG FrameLen = 0, VarLen = 0, tmpValue = 0;
	PGAS_CTRL pGASCtrl;
	GAS_PEER_ENTRY *GASPeerEntry;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR WildcardBssid[MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	BOOLEAN bPeerFound = FALSE;
#ifdef CONFIG_HOTSPOT
	PHOTSPOT_CTRL pHSCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].HotSpotCtrl;
#endif
	GAS_QUERY_RSP_FRAGMENT *GASQueryRspFrag, *GASQueryRspFragTmp;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO, "\n");

	if (Event->ControlIndex < MAX_MBSSID_NUM(pAd))
		pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "%s:get error wdev idx!\n", __func__);
		return;
	}

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
			bPeerFound = TRUE;
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	if (bPeerFound == FALSE)
		return;

	if (Event->u.GAS_RSP_DATA.AdvertisementProID == ACCESS_NETWORK_QUERY_PROTOCOL) {
		/* Advertisement protocol element + Query response length field */
		VarLen += 6;
	}

	if (Event->EventType == GAS_RSP && Event->u.GAS_RSP_DATA.QueryRspLen > pGASCtrl->MMPDUSize) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_WARN, "Data is too big\n");
		return;
	}

	if (Event->EventType == GAS_RSP) {
		/* Query response field*/
		VarLen += Event->u.GAS_RSP_DATA.QueryRspLen;
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*GASFrame) + VarLen);

	if (!Buf) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Not available memory\n");
		/* GASSetPeerCurrentState(pAd, Elem, WAIT_PEER_GAS_REQ); */
		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		if (Event->EventType == GAS_RSP_MORE) {
			DlListForEachSafe(GASQueryRspFrag, GASQueryRspFragTmp, &GASPeerEntry->GASQueryRspFragList,
					  GAS_QUERY_RSP_FRAGMENT, List) {
				DlListDel(&GASQueryRspFrag->List);
				os_free_mem(GASQueryRspFrag->FragQueryRsp);
				os_free_mem(GASQueryRspFrag);
			}
		}
		DlListDel(&GASPeerEntry->List);
		DlListInit(&GASPeerEntry->GASQueryRspFragList);
		FreeGasPeerEntryTimers(GASPeerEntry);
		os_free_mem(GASPeerEntry);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		GASSetPeerCurrentState(pAd, Event, WAIT_PEER_GAS_REQ);
		return;
	}

	NdisZeroMemory(Buf, sizeof(*GASFrame) + VarLen);
	GASFrame = (GAS_FRAME *)Buf;
	pEntry = MacTableLookup(pAd, GASPeerEntry->PeerMACAddr);

	if ((pEntry)
#ifdef CONFIG_HOTSPOT
		|| (pHSCtrl->bHSOnOff == 1)
#endif
	) {
		ActHeaderInit(pAd, &GASFrame->Hdr, Event->PeerMACAddr, pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					  pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);
	} else { /* peer is not associated, follow 802.11-2012 , fill Addr3 BSSID with wildcard bssid */
		ActHeaderInit(pAd, &GASFrame->Hdr, Event->PeerMACAddr, pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					  WildcardBssid);
	}

	FrameLen += sizeof(HEADER_802_11);
#ifdef DOT11W_PMF_SUPPORT
	if (pEntry) {
		if (pEntry->SecConfig.PmfCfg.UsePMFConnect && GASPeerEntry->peer_use_protected_dual)
			GASFrame->Category = CATEGORY_PD;
		else
			GASFrame->Category = CATEGORY_PUBLIC;
	}
#endif /* DOT11W_PMF_SUPPORT */
	else
		GASFrame->Category = CATEGORY_PUBLIC;
	FrameLen += 1;
	GASFrame->u.GAS_INIT_RSP.Action = ACTION_GAS_INIT_RSP;
	FrameLen += 1;

	if (Event->EventType == GAS_RSP) {
		GASFrame->u.GAS_INIT_RSP.DialogToken = Event->u.GAS_RSP_DATA.DialogToken;
		FrameLen += 1;
		GASFrame->u.GAS_INIT_RSP.StatusCode = cpu2le16(Event->u.GAS_RSP_DATA.StatusCode);
		FrameLen += 2;
		GASFrame->u.GAS_INIT_RSP.GASComebackDelay = Event->u.GAS_RSP_DATA.GASComebackDelay;
		FrameLen += 2;
		Pos = GASFrame->u.GAS_INIT_RSP.Variable;
		*Pos++ = IE_ADVERTISEMENT_PROTO;
		*Pos++ = 2; /* Length field */
		*Pos++ = 0; /* Query response info field */
		*Pos++ = Event->u.GAS_RSP_DATA.AdvertisementProID; /* Advertisement Protocol ID field */
		tmpValue = cpu2le16(Event->u.GAS_RSP_DATA.QueryRspLen);
		NdisMoveMemory(Pos, &tmpValue, 2);
		Pos += 2;
		FrameLen += 6;

		if (Event->u.GAS_RSP_DATA.QueryRspLen > 0 &&
				Event->u.GAS_RSP_DATA.QueryRspLen < pGASCtrl->MMPDUSize)
			NdisMoveMemory(Pos, Event->u.GAS_RSP_DATA.QueryRsp, Event->u.GAS_RSP_DATA.QueryRspLen);
		FrameLen += Event->u.GAS_RSP_DATA.QueryRspLen;
		/* GASSetPeerCurrentState(pAd, Elem, WAIT_PEER_GAS_REQ); */
		GASSetPeerCurrentState(pAd, Event, WAIT_PEER_GAS_REQ);
		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListDel(&GASPeerEntry->List);
		DlListInit(&GASPeerEntry->GASQueryRspFragList);
		FreeGasPeerEntryTimers(GASPeerEntry);
		os_free_mem(GASPeerEntry);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
	} else if (Event->EventType == GAS_RSP_MORE) {
		GASFrame->u.GAS_INIT_RSP.DialogToken = Event->u.GAS_RSP_MORE_DATA.DialogToken;
		FrameLen += 1;
		GASFrame->u.GAS_INIT_RSP.StatusCode = cpu2le16(Event->u.GAS_RSP_MORE_DATA.StatusCode);
		FrameLen += 2;
		GASFrame->u.GAS_INIT_RSP.GASComebackDelay = Event->u.GAS_RSP_MORE_DATA.GASComebackDelay;
		FrameLen += 2;
		Pos = GASFrame->u.GAS_INIT_RSP.Variable;
		*Pos++ = IE_ADVERTISEMENT_PROTO;
		*Pos++ = 2; /* Length field */
		*Pos++ = 0; /* Query response info field */
		*Pos++ = Event->u.GAS_RSP_MORE_DATA.AdvertisementProID; /* Advertisement Protocol ID field */
		tmpValue = 0;
		NdisMoveMemory(Pos, &tmpValue, 2);
		Pos += 2;
		FrameLen += 6;
		/* GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_CB_REQ); */
		GASSetPeerCurrentState(pAd, Event, WAIT_GAS_CB_REQ);

		/*
		 * Buffer the query response for a minimun of dotGASResponseBufferingTime
		 * after the expiry of the GAS comeback delay
		 */
		if (!GASPeerEntry->GASRspBufferingTimerRunning) {
			RTMPSetTimer(&GASPeerEntry->GASRspBufferingTimer, dotGASResponseBufferingTime
						 + GASFrame->u.GAS_INIT_RSP.GASComebackDelay);
			GASPeerEntry->GASRspBufferingTimerRunning = TRUE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Invalid Event type %d\n",
					Event->EventType);
		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListDel(&GASPeerEntry->List);
		DlListInit(&GASPeerEntry->GASQueryRspFragList);
		FreeGasPeerEntryTimers(GASPeerEntry);
		os_free_mem(GASPeerEntry);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		os_free_mem(Buf);
		return;
	}

	MiniportMMRequest(pAd, 0, Buf, FrameLen);
	os_free_mem(Buf);
}

#ifdef DPP_SUPPORT
VOID DPP_ReceiveGASInitRsp(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	GAS_FRAME *GASFrame = (GAS_FRAME *)Elem->Msg;

	if ((GASFrame->u.GAS_INIT_RSP.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
			NdisEqualMemory(&GASFrame->u.GAS_INIT_RSP.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
			GASFrame->u.GAS_INIT_RSP.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE) {
		if (pAd->bDppEnable) {
			wext_send_dpp_action_frame(pAd, Elem->wdev, &Elem->Msg[10], Elem->Channel,
					Elem->Msg, Elem->MsgLen, 1);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Sent received gas init Rsp frame to WAPP\n");
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPP disabled gas init Rsp not sent to WAPP\n");
		return;
	}
}

VOID DPP_ReceiveGASCBReq(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	GAS_FRAME *GASFrame = (GAS_FRAME *)Elem->Msg;

	if (GASFrame->u.GAS_CB_REQ.Action == ACTION_GAS_CB_REQ) {
		if (pAd->bDppEnable) {
			wext_send_dpp_action_frame(pAd, Elem->wdev, &Elem->Msg[10], Elem->Channel,
					Elem->Msg, Elem->MsgLen, 1);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Sent received gas cb req frame to WAPP\n");
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPP disabled gas cb req not sent to WAPP\n");
		return;
	}
}

VOID DPP_ReceiveGASCBResp(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	GAS_FRAME *GASFrame = (GAS_FRAME *)Elem->Msg;

	if ((GASFrame->u.GAS_CB_RSP.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
			NdisEqualMemory(&GASFrame->u.GAS_CB_RSP.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
			GASFrame->u.GAS_CB_RSP.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE) {
		if (pAd->bDppEnable) {
			wext_send_dpp_action_frame(pAd, Elem->wdev, &Elem->Msg[10], Elem->Channel,
					Elem->Msg, Elem->MsgLen, 1);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Sent received gas cb Rsp frame to WAPP\n");
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPP disabled gas cb Rsp not sent to WAPP\n");
		return;
	}
}
#endif /* DPP_SUPPORT */

VOID ReceiveGASInitReq(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	GAS_EVENT_DATA *Event;
	GAS_FRAME *GASFrame = (GAS_FRAME *)Elem->Msg;
	GAS_PEER_ENTRY *GASPeerEntry = NULL, *GASPeerEntryTmp = NULL;
	PGAS_CTRL pGASCtrl = NULL;
	UCHAR APIndex, *Pos, *Buf;
	UINT16 VarLen;
	UINT32 Len = 0;
	BOOLEAN IsFound = FALSE;
	PNET_DEV NetDev;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO,
		"Addr2 = "MACSTR", Dialog Token = %d\n",
		MAC2STR(GASFrame->Hdr.Addr2), GASFrame->u.GAS_INIT_REQ.DialogToken);

#ifdef DPP_SUPPORT
	if ((GASFrame->u.GAS_INIT_REQ.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
		NdisEqualMemory(&GASFrame->u.GAS_INIT_REQ.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
		GASFrame->u.GAS_INIT_REQ.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE) {
		if (pAd->bDppEnable) {
			wext_send_dpp_action_frame(pAd, Elem->wdev, &Elem->Msg[10], Elem->Channel,
						Elem->Msg, Elem->MsgLen, 1);
			pAd->bDppGasInitReqRecived = TRUE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Sent received gas init Req frame to WAPP\n");
		} else {
			pAd->bDppGasInitReqRecived = FALSE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPP Disabled gas init Req not passed to WAPP\n");
		}
		return;
	} else
		pAd->bDppGasInitReqRecived = FALSE;
#endif /* DPP_SUPPORT */

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
		/*
		according to 802.11-2012, public action frame may have Wildcard BSSID in addr3,
		use addr1(DA) for searching instead.
		*/
		if (MAC_ADDR_EQUAL(GASFrame->Hdr.Addr1, pAd->ApCfg.MBSSID[APIndex].wdev.bssid)) {
			pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;
			break;
		}
	}
	NetDev = pAd->ApCfg.MBSSID[APIndex].wdev.if_dev;

	if (!pGASCtrl) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Can not find Peer Control DA="MACSTR"\n", MAC2STR(GASFrame->Hdr.Addr1));
		return;
	}

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, GASFrame->Hdr.Addr2)) {
			IsFound = TRUE;
			GASPeerEntry->QueryNum++;
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	if (IsFound) {
		/* if (GASPeerEntry->QueryNum++ >= 15) */
		{
			/* Query too many times, remove the peer address from list */
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO,
					 "Q %d, old 0x%x, 0x%x, "MACSTR" remove peer\n",
					  GASPeerEntry->QueryNum, GASPeerEntry->DialogToken, GASFrame->u.GAS_INIT_REQ.DialogToken,
					  MAC2STR(GASPeerEntry->PeerMACAddr));
			RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
			DlListForEachSafe(GASPeerEntry, GASPeerEntryTmp,
				 &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
				if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, GASFrame->Hdr.Addr2))
					FreeGasPeerEntry(GASPeerEntry);
			}
			RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		}
		/* return; */
	}

	os_alloc_mem(NULL, (UCHAR **)&GASPeerEntry, sizeof(*GASPeerEntry));

	if (!GASPeerEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Not available memory\n");
		return;
	}

	NdisZeroMemory(GASPeerEntry, sizeof(*GASPeerEntry));
	GASPeerEntry->InitPostReplyTimer = FALSE;
	GASPeerEntry->InitGASRspBufferingTimer = FALSE;
	GASPeerEntry->CurrentState = WAIT_PEER_GAS_REQ;
	GASPeerEntry->QueryNum++;
	NdisMoveMemory(GASPeerEntry->PeerMACAddr, GASFrame->Hdr.Addr2, MAC_ADDR_LEN);
	GASPeerEntry->DialogToken = GASFrame->u.GAS_INIT_REQ.DialogToken;
	GASPeerEntry->Priv = pAd;
	GASPeerEntry->peer_use_protected_dual = (GASFrame->Category == CATEGORY_PD) ? TRUE : FALSE;
	GASPeerEntry->ControlIndex = APIndex;
	if (!GASPeerEntry->InitPostReplyTimer) {
		RTMPInitTimer(pAd, &GASPeerEntry->PostReplyTimer,
				GET_TIMER_FUNCTION(PostReplyTimeout), GASPeerEntry, FALSE);
		/*
		   Add GASPeerEntry NULL check to remove KASAN warnings. If GasPeerEntry will be freed in timer task
		   and no any protection, there is a risk may make below code take a NULL pointer and used.
		 */
		if (!GASPeerEntry) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "PostReplyTimeout\n");
			return;
		}
		GASPeerEntry->InitPostReplyTimer = TRUE;
	}

	if (!GASPeerEntry->InitGASRspBufferingTimer) {
		RTMPInitTimer(pAd, &GASPeerEntry->GASRspBufferingTimer,
				GET_TIMER_FUNCTION(GASRspBufferingTimeout), GASPeerEntry, FALSE);
		/*
		   Add GASPeerEntry NULL check to remove KASAN warnings. If GasPeerEntry will be freed in timer task
		   and no any protection, there is a risk may make below code take a NULL pointer and used.
		 */
		if (!GASPeerEntry) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "GASRspBufferingTimeout\n");
			return;
		}
		GASPeerEntry->InitGASRspBufferingTimer = TRUE;
	}
	DlListInit(&GASPeerEntry->GASQueryRspFragList);
	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListAddTail(&pGASCtrl->GASPeerList, &GASPeerEntry->List);
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
	NdisMoveMemory(&VarLen, GASFrame->u.GAS_INIT_REQ.Variable + 4, 2);
	VarLen = le2cpu16(VarLen);
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + VarLen);

	if (!Buf) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Not available memory\n");
		goto error;
	}

	NdisZeroMemory(Buf, sizeof(*Event) + VarLen);
	Event = (GAS_EVENT_DATA *)Buf;
	Event->ControlIndex = APIndex;
	Len += 1;
	NdisMoveMemory(Event->PeerMACAddr, GASFrame->Hdr.Addr2, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	Event->EventType = PEER_GAS_REQ;
	Len += 2;
	Event->u.PEER_GAS_REQ_DATA.DialogToken = GASFrame->u.GAS_INIT_REQ.DialogToken;
	Len += 1;
	Pos = GASFrame->u.GAS_INIT_REQ.Variable + 3;
	Event->u.PEER_GAS_REQ_DATA.AdvertisementProID = *Pos;
	GASPeerEntry->AdvertisementProID = *Pos;
	Len += 1;
	Pos++;
	NdisMoveMemory(&Event->u.PEER_GAS_REQ_DATA.QueryReqLen, Pos, 2);
	Event->u.PEER_GAS_REQ_DATA.QueryReqLen = le2cpu16(Event->u.PEER_GAS_REQ_DATA.QueryReqLen);
	Len += 2;
	Pos += 2;
	if (Event->u.PEER_GAS_REQ_DATA.AdvertisementProID == ACCESS_NETWORK_QUERY_PROTOCOL &&
		!pGASCtrl->ExternalANQPServerTest) {
		/* Send anqp request indication to daemon */
		SendAnqpReqEvent(NetDev,
				Event->PeerMACAddr,
				&GASFrame->Category,
				Event->u.PEER_GAS_REQ_DATA.QueryReqLen + 9);
	}
	NdisMoveMemory(Event->u.PEER_GAS_REQ_DATA.QueryReq, Pos, Event->u.PEER_GAS_REQ_DATA.QueryReqLen);
	Len += Event->u.PEER_GAS_REQ_DATA.QueryReqLen;
	SendGASIndication(pAd, (GAS_EVENT_DATA *)Buf);
	/* MlmeEnqueue(pAd, GAS_STATE_MACHINE, PEER_GAS_REQ, Len, Buf,0); */
	/* RTMP_MLME_HANDLER(pAd); */
	os_free_mem(Buf);
	return;
error:
	DlListDel(&GASPeerEntry->List);
	os_free_mem(GASPeerEntry);
}

VOID FreeGasPeerEntryTimers(IN GAS_PEER_ENTRY *GASPeerEntry)
{
	BOOLEAN Cancelled;

	if (!GASPeerEntry)
		return;

	if (GASPeerEntry->PostReplyTimerRunning) {
		RTMPCancelTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
		GASPeerEntry->PostReplyTimerRunning = FALSE;
	}

	if (GASPeerEntry->InitPostReplyTimer) {
		RTMPReleaseTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
		GASPeerEntry->InitPostReplyTimer = FALSE;
	}

	if (GASPeerEntry->GASRspBufferingTimerRunning) {
		RTMPCancelTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
		GASPeerEntry->GASRspBufferingTimerRunning = FALSE;
	}

	if (GASPeerEntry->InitGASRspBufferingTimer) {
		RTMPReleaseTimer(&GASPeerEntry->GASRspBufferingTimer, &Cancelled);
		GASPeerEntry->InitGASRspBufferingTimer = FALSE;
	}
}

VOID FreeGasPeerEntry(IN GAS_PEER_ENTRY *GASPeerEntry)
{
	GAS_QUERY_RSP_FRAGMENT *GASQueryRspFrag, *GASQueryRspFragTmp;

	DlListForEachSafe(GASQueryRspFrag, GASQueryRspFragTmp,
						 &GASPeerEntry->GASQueryRspFragList, GAS_QUERY_RSP_FRAGMENT, List) {
		DlListDel(&GASQueryRspFrag->List);
		os_free_mem(GASQueryRspFrag->FragQueryRsp);
		os_free_mem(GASQueryRspFrag);
	}
	DlListDel(&GASPeerEntry->List);
	DlListInit(&GASPeerEntry->GASQueryRspFragList);

	FreeGasPeerEntryTimers(GASPeerEntry);
	os_free_mem(GASPeerEntry);
}

static VOID SendGASCBRsp(
	IN PRTMP_ADAPTER    pAd,
	/* IN MLME_QUEUE_ELEM  *Elem) */
	GAS_EVENT_DATA *Event)
{
	/* GAS_EVENT_DATA *Event = (GAS_EVENT_DATA *)Elem->Msg; */
	UCHAR *Buf, *Pos;
	GAS_FRAME *GASFrame;
	ULONG FrameLen = 0, tmpLen = 0;
	GAS_PEER_ENTRY *GASPeerEntry, *GASPeerEntryTmp;
	GAS_QUERY_RSP_FRAGMENT *GASQueryRspFrag = NULL;
	PGAS_CTRL pGASCtrl;
	BOOLEAN bGASQueryRspFragFound = FALSE, bPeerFound = FALSE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR WildcardBssid[MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	if (Event->ControlIndex < MAX_MBSSID_NUM(pAd))
		pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "%s:get error wdev idx!\n", __func__);
		return;
	}

	/* Pre-allocate max possible length */
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*GASFrame)
			+ (GAS_AD_PROTO_ID_LEN + GAS_QUERY_RESPONSE_LEN) + pGASCtrl->MMPDUSize);

	if (!Buf) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Not available memory\n");
		return;
	}
	NdisZeroMemory(Buf, sizeof(*GASFrame)
			+ (GAS_AD_PROTO_ID_LEN + GAS_QUERY_RESPONSE_LEN) + pGASCtrl->MMPDUSize);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO, "\n");
	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEachSafe(GASPeerEntry, GASPeerEntryTmp,
				 &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
			if (GASPeerEntry->AdvertisementProID == ACCESS_NETWORK_QUERY_PROTOCOL) {
			}

			DlListForEach(GASQueryRspFrag, &GASPeerEntry->GASQueryRspFragList,
						  GAS_QUERY_RSP_FRAGMENT, List) {
				if (GASPeerEntry->CurrentGASFragNum ==
					GASQueryRspFrag->GASRspFragID) {
					GASPeerEntry->CurrentGASFragNum++;
					bGASQueryRspFragFound = TRUE;
					break;
				}
			}
			bPeerFound = TRUE;
			break;
		}
	}
	if ((bPeerFound == FALSE) || (bGASQueryRspFragFound == FALSE && Event->EventType == GAS_CB_REQ_MORE)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO,
					"bPeerFound =%d bGASQueryRspFragFound=%d\n", bPeerFound, bGASQueryRspFragFound);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		os_free_mem(Buf);
		return;
	}

	if ((bGASQueryRspFragFound == TRUE) && (!GASQueryRspFrag->FragQueryRsp)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO,
					"GASQueryRspFrag->FragQueryRsp is Null\n");
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		os_free_mem(Buf);
		return;
	}

	GASFrame = (GAS_FRAME *)Buf;
	pEntry = MacTableLookup(pAd, GASPeerEntry->PeerMACAddr);

	if (pEntry) {
		ActHeaderInit(pAd, &GASFrame->Hdr, Event->PeerMACAddr, pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					  pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);
	} else { /* peer is not associated, follow 802.11-2012 , fill Addr3 BSSID with wildcard bssid */
		ActHeaderInit(pAd, &GASFrame->Hdr, Event->PeerMACAddr, pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					  WildcardBssid);
	}

	FrameLen += sizeof(HEADER_802_11);
#ifdef DOT11W_PMF_SUPPORT
	if (pEntry) {
		if (pEntry->SecConfig.PmfCfg.UsePMFConnect && GASPeerEntry->peer_use_protected_dual)
			GASFrame->Category = CATEGORY_PD;
		else
			GASFrame->Category = CATEGORY_PUBLIC;
	}
#endif /* DOT11W_PMF_SUPPORT */
	else
		GASFrame->Category = CATEGORY_PUBLIC;

	FrameLen += 1;
	GASFrame->u.GAS_CB_RSP.Action = ACTION_GAS_CB_RSP;
	FrameLen += 1;

	if (Event->EventType == GAS_CB_REQ) {
		GASFrame->u.GAS_CB_RSP.DialogToken = Event->u.GAS_CB_REQ_DATA.DialogToken;
		FrameLen += 1;
		GASFrame->u.GAS_CB_RSP.StatusCode = cpu2le16(Event->u.GAS_CB_REQ_DATA.StatusCode);
		FrameLen += 2;

		if (bGASQueryRspFragFound && GASQueryRspFrag)
			GASFrame->u.GAS_CB_RSP.GASRspFragID = (GASQueryRspFrag->GASRspFragID & 0x7F);
		else
			GASFrame->u.GAS_CB_RSP.GASRspFragID = 0;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO, "GASRspFragID = %d\n", GASFrame->u.GAS_CB_RSP.GASRspFragID);
		FrameLen += 1;
		GASFrame->u.GAS_CB_RSP.GASComebackDelay = 0;
		FrameLen += 2;
		Pos = GASFrame->u.GAS_CB_RSP.Variable;
		*Pos++ = IE_ADVERTISEMENT_PROTO;
		*Pos++ = 2; /* Length field */
		*Pos++ = 0; /* Query response info field */
		*Pos++ = Event->u.GAS_CB_REQ_DATA.AdvertisementProID; /* Advertisement Protocol ID field */

		if ((Event->u.GAS_CB_REQ_DATA.StatusCode == 0) && bGASQueryRspFragFound && GASQueryRspFrag) {
			tmpLen = cpu2le16(GASQueryRspFrag->FragQueryRspLen);
			NdisMoveMemory(Pos, &tmpLen, 2);
			Pos += 2;
			FrameLen += 6;
			if (GASQueryRspFrag->FragQueryRspLen > 0)
				NdisMoveMemory(Pos, GASQueryRspFrag->FragQueryRsp,
						   GASQueryRspFrag->FragQueryRspLen);
			FrameLen += GASQueryRspFrag->FragQueryRspLen;
		} else {
			tmpLen = 0;
			NdisMoveMemory(Pos, &tmpLen, 2);
			Pos += 2;
			FrameLen += 6;
		}
		FreeGasPeerEntry(GASPeerEntry);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
	} else if (Event->EventType == GAS_CB_REQ_MORE) {
		GASFrame->u.GAS_CB_RSP.DialogToken = Event->u.GAS_CB_REQ_MORE_DATA.DialogToken;
		FrameLen += 1;
		GASFrame->u.GAS_CB_RSP.StatusCode = cpu2le16(Event->u.GAS_CB_REQ_MORE_DATA.StatusCode);
		FrameLen += 2;
		if (bGASQueryRspFragFound && GASQueryRspFrag)
			GASFrame->u.GAS_CB_RSP.GASRspFragID = (0x80 | (GASQueryRspFrag->GASRspFragID & 0x7F));
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO, "GASRspFragID = %d\n", GASFrame->u.GAS_CB_RSP.GASRspFragID);
		FrameLen += 1;
		GASFrame->u.GAS_CB_RSP.GASComebackDelay = 0;
		FrameLen += 2;
		Pos = GASFrame->u.GAS_CB_RSP.Variable;
		*Pos++ = IE_ADVERTISEMENT_PROTO;
		*Pos++ = 2; /* Length field */
		*Pos++ = 0; /* Query response info field */
		*Pos++ = Event->u.GAS_CB_REQ_MORE_DATA.AdvertisementProID; /* Advertisement Protocol ID field */
		if (bGASQueryRspFragFound && GASQueryRspFrag) {
			tmpLen = cpu2le16(GASQueryRspFrag->FragQueryRspLen);
			NdisMoveMemory(Pos, &tmpLen, 2);
			Pos += 2;
			FrameLen += 6;
			if (GASQueryRspFrag->FragQueryRspLen > 0)
				NdisMoveMemory(Pos, GASQueryRspFrag->FragQueryRsp,
						GASQueryRspFrag->FragQueryRspLen);
			FrameLen += GASQueryRspFrag->FragQueryRspLen;
		}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		/* GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_CB_REQ); */
		GASSetPeerCurrentState(pAd, Event, WAIT_GAS_CB_REQ);
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Invalid Event type %d\n",
					Event->EventType);
		FreeGasPeerEntry(GASPeerEntry);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		os_free_mem(Buf);
		return;
	}

	MiniportMMRequest(pAd, 0, Buf, FrameLen);
	os_free_mem(Buf);
}


VOID ReceiveGASCBReq(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	GAS_EVENT_DATA *Event;
	GAS_FRAME *GASFrame = (GAS_FRAME *)Elem->Msg;
	GAS_PEER_ENTRY *GASPeerEntry;
	PGAS_CTRL pGASCtrl = NULL;
	UCHAR APIndex, *Buf;
	UINT32 Len = 0;
	BOOLEAN IsGASCBReqMore = FALSE;
	BOOLEAN IsFound = FALSE;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO,
		"Addr2 = "MACSTR", Dialog Token = %d\n",
		MAC2STR(GASFrame->Hdr.Addr2), GASFrame->u.GAS_CB_REQ.DialogToken);

#ifdef DPP_SUPPORT
	/* For DPP, GAS CB Response is handled in DPP_ReceiveGASCBReq
	 * and further in wappd.
	 * Return from the function to avoid sending GAS CB response
	 * when no GAS Init request recieved */
	if (pAd->bDppEnable && pAd->bDppGasInitReqRecived) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				"%s not used in case of DPP GAS CB Req\n", __func__);
		return;
	}
#endif /* DPP_SUPPORT */

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
		if (MAC_ADDR_EQUAL(GASFrame->Hdr.Addr1, pAd->ApCfg.MBSSID[APIndex].wdev.bssid)) {
			pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;
			break;
		}
	}

	if (!pGASCtrl) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Can not find Peer Control DA="MACSTR"\n", MAC2STR(GASFrame->Hdr.Addr1));
		return;
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event));

	if (!Buf) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Not available memory\n");
		return;
	}

	NdisZeroMemory(Buf, sizeof(*Event));
	Event = (GAS_EVENT_DATA *)Buf;
	Event->ControlIndex = APIndex;
	Len += 1;
	NdisMoveMemory(Event->PeerMACAddr, GASFrame->Hdr.Addr2, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
			if ((GASPeerEntry->CurrentGASFragNum + 1) <
				GASPeerEntry->GASRspFragNum)
				IsGASCBReqMore = TRUE;

			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	if (!IsGASCBReqMore) {
		if (IsFound) {
			Event->EventType = GAS_CB_REQ;
			Len += 2;
			Event->u.GAS_CB_REQ_DATA.DialogToken = GASFrame->u.GAS_CB_REQ.DialogToken;
			Len += 1;
			Event->u.GAS_CB_REQ_DATA.AdvertisementProID = GASPeerEntry->AdvertisementProID;
			Len += 1;
			Event->u.GAS_CB_REQ_DATA.StatusCode = 0;
			Len += 2;
			SendGASCBRsp(pAd, (GAS_EVENT_DATA *)Buf);
			/* MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_CB_REQ, Len, Buf, 0); */
			/* RTMP_MLME_HANDLER(pAd); */
		} else {
			os_alloc_mem(NULL, (UCHAR **)&GASPeerEntry, sizeof(*GASPeerEntry));

			if (!GASPeerEntry) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Not available memory\n");
				goto error;
			}

			NdisZeroMemory(GASPeerEntry, sizeof(*GASPeerEntry));
			GASPeerEntry->CurrentState = WAIT_GAS_CB_REQ;
			GASPeerEntry->ControlIndex = Event->ControlIndex;
			NdisMoveMemory(GASPeerEntry->PeerMACAddr, GASFrame->Hdr.Addr2, MAC_ADDR_LEN);
			GASPeerEntry->DialogToken = GASFrame->u.GAS_CB_REQ.DialogToken;
			GASPeerEntry->AdvertisementProID = ACCESS_NETWORK_QUERY_PROTOCOL; /* FIXME */
			GASPeerEntry->Priv = pAd;
			DlListInit(&GASPeerEntry->GASQueryRspFragList);
			RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
			DlListAddTail(&pGASCtrl->GASPeerList, &GASPeerEntry->List);
			RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
			Event->EventType = GAS_CB_REQ;
			Len += 2;
			Event->u.GAS_CB_REQ_DATA.DialogToken = GASFrame->u.GAS_CB_REQ.DialogToken;
			Len += 1;
			Event->u.GAS_CB_REQ_DATA.AdvertisementProID = ACCESS_NETWORK_QUERY_PROTOCOL;
			Len += 1;
			Event->u.GAS_CB_REQ_DATA.StatusCode = UNSPECIFIED_FAILURE;
			Len += 2;
			SendGASCBRsp(pAd, (GAS_EVENT_DATA *)Buf);
			/* MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_CB_REQ, Len, Buf, 0); */
			/* RTMP_MLME_HANDLER(pAd); */
		}
	} else {
		Event->EventType = GAS_CB_REQ_MORE;
		Len += 2;
		Event->u.GAS_CB_REQ_MORE_DATA.DialogToken = GASFrame->u.GAS_CB_REQ.DialogToken;
		Len += 1;
		Event->u.GAS_CB_REQ_MORE_DATA.AdvertisementProID = GASPeerEntry->AdvertisementProID;
		Len += 1;
		Event->u.GAS_CB_REQ_MORE_DATA.StatusCode = 0;
		Len += 2;
		SendGASCBRsp(pAd, (GAS_EVENT_DATA *)Buf);
		/* MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_CB_REQ_MORE, Len, Buf,0); */
		/* RTMP_MLME_HANDLER(pAd); */
	}

error:
	os_free_mem(Buf);
}


static BOOLEAN IsAdvertisementProIDValid(
	IN PRTMP_ADAPTER pAd,
	IN enum DOT11U_ADVERTISMENT_PROTOCOL_ID AdvertisementProtoID)
{
	INT32 i;

	for (i = 0; i < sizeof(dot11GASAdvertisementID)
		 / sizeof(enum DOT11U_ADVERTISMENT_PROTOCOL_ID); i++) {
		if (AdvertisementProtoID == dot11GASAdvertisementID[i])
			return TRUE;
	}

	return FALSE;
}

static VOID DelGasPeerEntry(
	IN PRTMP_ADAPTER    pAd,
	/* IN MLME_QUEUE_ELEM  *Elem) */
	GAS_EVENT_DATA *Event)
{
	GAS_PEER_ENTRY *GASPeerEntry = NULL, *GASPeerEntryTmp = NULL;
	PGAS_CTRL pGASCtrl = NULL;

	if (!Event)
		return;

	pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEachSafe(GASPeerEntry, GASPeerEntryTmp,
				 &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
			FreeGasPeerEntry(GASPeerEntry);
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
}


static void PostReplyTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	GAS_PEER_ENTRY *GASPeerEntry = (GAS_PEER_ENTRY *)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
	GAS_EVENT_DATA Event;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO, "\n");

	if (!GASPeerEntry) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "GASPeerEntry is NULL\n");
		return;
	}

	pAd = GASPeerEntry->Priv;
	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "pAd is NULL\n");
		return;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "fRTMP_ADAPTER_HALT_IN_PROGRESS\n");
		return;
	}
	NdisMoveMemory(Event.PeerMACAddr, GASPeerEntry->PeerMACAddr, MAC_ADDR_LEN);
	Event.ControlIndex = GASPeerEntry->ControlIndex;
	MlmeEnqueue(pAd, GAS_STATE_MACHINE, DEL_PEER_ENTRY, sizeof(Event), (VOID *)&Event, 0);
}
BUILD_TIMER_FUNCTION(PostReplyTimeout);

static VOID GASRspBufferingTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	GAS_PEER_ENTRY *GASPeerEntry = (GAS_PEER_ENTRY *)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
	GAS_EVENT_DATA Event;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO, "\n");

	if (!GASPeerEntry) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "GASPeerEntry is NULL\n");
		return;
	}

	pAd = GASPeerEntry->Priv;
	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "pAd is NULL\n");
		return;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS
					   | fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "fRTMP_ADAPTER_HALT_IN_PROGRESS\n");
		return;
	}
	NdisMoveMemory(Event.PeerMACAddr, GASPeerEntry->PeerMACAddr, MAC_ADDR_LEN);
	Event.ControlIndex = GASPeerEntry->ControlIndex;
	MlmeEnqueue(pAd, GAS_STATE_MACHINE, DEL_PEER_ENTRY, sizeof(Event), (VOID *)&Event, 0);
}
BUILD_TIMER_FUNCTION(GASRspBufferingTimeout);

static VOID SendGASIndication(
	IN PRTMP_ADAPTER    pAd,
	/* IN MLME_QUEUE_ELEM  *Elem) */
	GAS_EVENT_DATA *Event)
{
	PGAS_PEER_ENTRY GASPeerEntry;
	/* GAS_EVENT_DATA *Event = (GAS_EVENT_DATA *)Elem->Msg; */
	GAS_EVENT_DATA *GASRspEvent;
	PGAS_CTRL pGASCtrl;
	UCHAR *Buf;
	UINT32 Len = 0;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO, "\n");

	if (Event->ControlIndex < MAX_MBSSID_NUM(pAd))
		pGASCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].GASCtrl;
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "%s:get error wdev idx!\n", __func__);
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO,
		"ExternalANQPServerTest = %d\n", pGASCtrl->ExternalANQPServerTest);

	/*
	 * If advertisement protocol ID equals to the value
	 * contained in any dot11GASAdvertisementID MID objects
	 */
	if (IsAdvertisementProIDValid(pAd, Event->u.PEER_GAS_REQ_DATA.AdvertisementProID) &&
		!pGASCtrl->ExternalANQPServerTest) {
		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		/* Init PostReply timer */
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
				if (!GASPeerEntry->PostReplyTimerRunning) {
					RTMPSetTimer(&GASPeerEntry->PostReplyTimer, dot11GASResponseTimeout);
					GASPeerEntry->PostReplyTimerRunning = TRUE;
				}

				break;
			}
		}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		/* GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_RSP); */
		GASSetPeerCurrentState(pAd, Event, WAIT_GAS_RSP);
	} else if (IsAdvertisementProIDValid(pAd, Event->u.PEER_GAS_REQ_DATA.AdvertisementProID) &&
			(pGASCtrl->ExternalANQPServerTest == 1 ||/* server not reachable for 2F test */
			 pGASCtrl->ExternalANQPServerTest == 2)) { /* server not reachable for 4F test */
		BOOLEAN InList = FALSE;
		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
				InList = TRUE;
				break;
			}
		}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		if (!InList) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				 "Event is not in GASPeerList\n");
			return;
		}

		os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*GASRspEvent));

		if (!Buf) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Not available memory\n");
			return;
		}

		NdisZeroMemory(Buf, sizeof(*GASRspEvent));
		GASRspEvent = (GAS_EVENT_DATA *)Buf;
		GASRspEvent->ControlIndex = Event->ControlIndex;
		Len += 1;
		NdisMoveMemory(GASRspEvent->PeerMACAddr, Event->PeerMACAddr, MAC_ADDR_LEN);
		Len += MAC_ADDR_LEN;
		GASRspEvent->EventType = GAS_RSP;
		Len += 2;
		GASRspEvent->u.GAS_RSP_DATA.DialogToken = GASPeerEntry->DialogToken;
		Len += 1;
		GASRspEvent->u.GAS_RSP_DATA.StatusCode = SERVER_UNREACHABLE;
		Len += 2;
		GASRspEvent->u.GAS_RSP_DATA.GASComebackDelay = 0;
		Len += 2;
		GASRspEvent->u.GAS_RSP_DATA.AdvertisementProID = GASPeerEntry->AdvertisementProID;
		Len += 1;
		GASRspEvent->u.GAS_RSP_DATA.QueryRspLen = 0;
		Len += 2;
		/* GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_RSP); */
		GASSetPeerCurrentState(pAd, Event, WAIT_GAS_RSP);
		SendGASRsp(pAd, (GAS_EVENT_DATA *)Buf);
		/* MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_RSP, Len, Buf, 0); */
		/* RTMP_MLME_HANDLER(pAd); */
		os_free_mem(Buf);
	} else if (!IsAdvertisementProIDValid(pAd, Event->u.PEER_GAS_REQ_DATA.AdvertisementProID)) {
		BOOLEAN InList = FALSE;
		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
				InList = TRUE;
				break;
			}
		}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		if (!InList) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				 "Event is not in GASPeerList\n");
			return;
		}

		/* Do not support this advertisement protocol, such as MIH */
		os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*GASRspEvent));

		if (!Buf) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "Not available memory\n");
			return;
		}

		NdisZeroMemory(Buf, sizeof(*GASRspEvent));
		GASRspEvent = (GAS_EVENT_DATA *)Buf;
		GASRspEvent->ControlIndex = Event->ControlIndex;
		Len += 1;
		NdisMoveMemory(GASRspEvent->PeerMACAddr, Event->PeerMACAddr, MAC_ADDR_LEN);
		Len += MAC_ADDR_LEN;
		GASRspEvent->EventType = GAS_RSP;
		Len += 2;
		GASRspEvent->u.GAS_RSP_DATA.DialogToken = GASPeerEntry->DialogToken;
		Len += 1;
		GASRspEvent->u.GAS_RSP_DATA.StatusCode = ADVERTISEMENT_PROTOCOL_NOT_SUPPORTED;
		Len += 2;
		GASRspEvent->u.GAS_RSP_DATA.GASComebackDelay = 0;
		Len += 2;
		GASRspEvent->u.GAS_RSP_DATA.AdvertisementProID = GASPeerEntry->AdvertisementProID;
		Len += 1;
		GASRspEvent->u.GAS_RSP_DATA.QueryRspLen = 0;
		Len += 2;
		/* GASSetPeerCurrentState(pAd, Elem, WAIT_GAS_RSP); */
		GASSetPeerCurrentState(pAd, Event, WAIT_GAS_RSP);
		MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_RSP, Len, Buf, 0);
		RTMP_MLME_HANDLER(pAd);
		os_free_mem(Buf);
	} else
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "should not in this case\n");
}
#endif /* CONFIG_AP_SUPPORT */

static VOID GASCtrlInit(IN PRTMP_ADAPTER pAd)
{
	PGAS_CTRL pGASCtrl;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif
#ifdef CONFIG_AP_SUPPORT

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
		pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;
		NdisZeroMemory(pGASCtrl, sizeof(*pGASCtrl));
		NdisAllocateSpinLock(pAd, &pGASCtrl->GASPeerListLock);
		DlListInit(&pGASCtrl->GASPeerList);
		NdisAllocateSpinLock(pAd, &pGASCtrl->IeLock);
	}

#endif
}

VOID GASCtrlExit(IN PRTMP_ADAPTER pAd)
{
	PGAS_CTRL pGASCtrl;
	GAS_PEER_ENTRY *GASPeerEntry, *GASPeerEntryTmp;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif
#ifdef CONFIG_AP_SUPPORT

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
		pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;
		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		/* Remove all GAS peer entry */
		DlListForEachSafe(GASPeerEntry, GASPeerEntryTmp,
						  &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
			FreeGasPeerEntry(GASPeerEntry);
		}
		DlListInit(&pGASCtrl->GASPeerList);
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
		NdisFreeSpinLock(&pGASCtrl->GASPeerListLock);
		GASCtrlRemoveAllIE(pGASCtrl);
		NdisFreeSpinLock(&pGASCtrl->IeLock);
	}

#endif /* CONFIG_AP_SUPPORT */
}


VOID GASStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[])
{
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO, "\n");
	GASCtrlInit(pAd);
	StateMachineInit(S,	(STATE_MACHINE_FUNC *)Trans, MAX_GAS_STATE, MAX_GAS_MSG, (STATE_MACHINE_FUNC)Drop, GAS_UNKNOWN, GAS_MACHINE_BASE);
#ifdef CONFIG_AP_SUPPORT
	StateMachineSetAction(S, WAIT_PEER_GAS_REQ, PEER_GAS_REQ, (STATE_MACHINE_FUNC)SendGASIndication);
	StateMachineSetAction(S, WAIT_GAS_RSP, GAS_RSP, (STATE_MACHINE_FUNC)SendGASRsp);
	StateMachineSetAction(S, WAIT_GAS_RSP, GAS_RSP_MORE, (STATE_MACHINE_FUNC)SendGASRsp);
	StateMachineSetAction(S, WAIT_GAS_RSP, GAS_CB_REQ, (STATE_MACHINE_FUNC)SendGASCBRsp);
	StateMachineSetAction(S, WAIT_GAS_CB_REQ, GAS_CB_REQ, (STATE_MACHINE_FUNC)SendGASCBRsp);
	StateMachineSetAction(S, WAIT_GAS_CB_REQ, GAS_CB_REQ_MORE, (STATE_MACHINE_FUNC)SendGASCBRsp);
	StateMachineSetAction(S, WAIT_PEER_GAS_REQ, DEL_PEER_ENTRY, (STATE_MACHINE_FUNC)DelGasPeerEntry);
	StateMachineSetAction(S, WAIT_GAS_RSP, DEL_PEER_ENTRY, (STATE_MACHINE_FUNC)DelGasPeerEntry);
	StateMachineSetAction(S, WAIT_GAS_CB_REQ, DEL_PEER_ENTRY, (STATE_MACHINE_FUNC)DelGasPeerEntry);
#endif /* CONFIG_AP_SUPPORT */
}
BOOLEAN GasEnable(IN PRTMP_ADAPTER pAd, IN MLME_QUEUE_ELEM * Elem)
{
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
	GAS_FRAME *GASFrame;
	PGAS_CTRL pGASCtrl = NULL;

	GASFrame = (GAS_FRAME *)Elem->Msg;

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
		/*
		according to 802.11-2012, public action frame may have Wildcard BSSID in addr3,
		use addr1(DA) for searching instead.
		*/
		if (MAC_ADDR_EQUAL(GASFrame->Hdr.Addr1, pAd->ApCfg.MBSSID[APIndex].wdev.bssid)) {
			pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;
			break;
		}
	}

	if (!pGASCtrl) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "%s Can not find Peer Control DA="MACSTR"\n", __func__, MAC2STR(GASFrame->Hdr.Addr1));
		return FALSE;
	}

	return pGASCtrl->b11U_enable;
#endif /* CONFIG_AP_SUPPORT */
}
