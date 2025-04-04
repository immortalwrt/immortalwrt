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
	action.c

    Abstract:
    Handle association related requests either from WSTA or from local MLME

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
	Jan Lee		2006		created for rt2860
 */

#include "rt_config.h"
#include "action.h"

#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */

extern UCHAR  ZeroSsid[32];
#ifdef IWSC_SUPPORT
extern UCHAR  IWSC_ACTION_OUI[];
#endif /* IWSC_SUPPORT // */


static VOID ReservedAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);



/*
    ==========================================================================
    Description:
	association state machine init, including state transition and timer init
    Parameters:
	S - pointer to the association state machine
    Note:
	The state machine looks like the following

				    ASSOC_IDLE
	ASSOC_FSM_MLME_DISASSOC_REQ    mlme_disassoc_req_action
	ASSOC_FSM_PEER_DISASSOC_REQ    peer_disassoc_action
	ASSOC_FSM_PEER_ASSOC_REQ       drop
	ASSOC_FSM_PEER_REASSOC_REQ     drop
	MT2_CLS3ERR              cls3err_action
    ==========================================================================
 */
VOID ActionStateMachineInit(
	IN RTMP_ADAPTER * pAd,
	IN STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, MAX_ACT_STATE, MAX_ACT_MSG, (STATE_MACHINE_FUNC)Drop, ACT_IDLE, ACT_MACHINE_BASE);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_SPECTRUM_CATE, (STATE_MACHINE_FUNC)PeerSpectrumAction);
#if defined(TPC_SUPPORT) && defined(TPC_MODE_CTRL)
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_TPC_REQ, (STATE_MACHINE_FUNC)mlmeAPSendTPCReqAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_TPC_REQ_TIMEOUT, (STATE_MACHINE_FUNC)TPCRepTimeout);
#endif
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_QOS_CATE, (STATE_MACHINE_FUNC)PeerQOSAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_DLS_CATE, (STATE_MACHINE_FUNC)ReservedAction);
#ifdef DOT11_N_SUPPORT
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_BA_CATE, (STATE_MACHINE_FUNC)PeerBAAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_HT_CATE, (STATE_MACHINE_FUNC)PeerHTAction);
#ifdef DOT11_VHT_AC
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_VHT_CATE, (STATE_MACHINE_FUNC)PeerVHTAction);
#endif /* DOT11_VHT_AC */
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_ADD_BA_CATE, (STATE_MACHINE_FUNC)MlmeADDBAAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_ORI_DELBA_CATE, (STATE_MACHINE_FUNC)MlmeDELBAAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_REC_DELBA_CATE, (STATE_MACHINE_FUNC)MlmeDELBAAction);
#endif /* DOT11_N_SUPPORT */
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_PUBLIC_CATE, (STATE_MACHINE_FUNC)PeerPublicAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_PD_CATE, (STATE_MACHINE_FUNC)PeerPublicAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_RM_CATE, (STATE_MACHINE_FUNC)PeerRMAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_QOS_CATE, (STATE_MACHINE_FUNC)MlmeQOSAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_DLS_CATE, (STATE_MACHINE_FUNC)MlmeDLSAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_ACT_INVALID, (STATE_MACHINE_FUNC)MlmeInvalidAction);
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	StateMachineSetAction(S, ACT_IDLE, FT_CATEGORY_BSS_TRANSITION, (STATE_MACHINE_FUNC)FT_FtAction);
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_PMF_CATE, (STATE_MACHINE_FUNC)PMF_PeerAction);
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_DOT11V_WNM
	StateMachineSetAction(S, ACT_IDLE, CATEGORY_WNM, (STATE_MACHINE_FUNC)PeerWNMAction);
#endif /* CONFIG_DOT11V_WNM */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_S1G_CATE, (STATE_MACHINE_FUNC)peer_twt_action);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_TWT_TEARDOWN_TWT, (STATE_MACHINE_FUNC)mlme_twt_teradown_action);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_TWT_RESUME_INFO, (STATE_MACHINE_FUNC)mlme_twt_resume_info_action);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_TWT_JOIN_BTWT, (STATE_MACHINE_FUNC)mlme_twt_handle_btwt_join_action);
#ifdef APCLI_SUPPORT
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_S1G_CATE_TWT_SETUP, (STATE_MACHINE_FUNC)twtMlmeSetupAction);
#endif /* APCLI_SUPPORT */
#endif /* WIFI_TWT_SUPPORT */
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_WNM_EVT_REPORT, (STATE_MACHINE_FUNC)sta_send_event_report);
#endif
	StateMachineSetAction(S, ACT_IDLE, CATEGORY_PD, (STATE_MACHINE_FUNC)PeerPublicAction);

#ifdef QOS_R1
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_ROBUST_AV_CATE, (STATE_MACHINE_FUNC)QoS_PeerRobust_AV_Action);
#if defined(MSCS_PROPRIETARY) || defined(QOS_R2)
	StateMachineSetAction(S, ACT_IDLE, MT2_CATEGORY_VSP, (STATE_MACHINE_FUNC)Peer_Vendor_Spec_Action);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_DABS_CFG_TIMEOUT, (STATE_MACHINE_FUNC)Peer_DABS_Cfg_Timeout);
#endif
#endif
}


#ifdef DOT11_N_SUPPORT
VOID MlmeADDBAAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MLME_ADDBA_REQ_STRUCT *pInfo;
	PUCHAR pOutBuffer = NULL;
	FRAME_ADDBA_REQ Frame;
	ULONG FrameLen;
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	struct wifi_dev *wdev;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (MlmeAddBAReqSanity(pAd, Elem->Msg, Elem->MsgLen) != TRUE)
		return;

	pInfo = (MLME_ADDBA_REQ_STRUCT *)Elem->Msg;
	pEntry = &pAd->MacTab.Content[pInfo->Wcid];
	tr_entry = &tr_ctl->tr_entry[pInfo->Wcid];

	if (pEntry->wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pEntry->wdev is NULL!\n");
		return;
	}
	wdev = pEntry->wdev;

	NdisZeroMemory(&Frame, sizeof(FRAME_ADDBA_REQ));

	if (MlmeAllocateMemory(pAd, &pOutBuffer) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BA - MlmeADDBAAction() allocate memory failed\n");
		return;
	}

#ifdef DOT11W_PMF_SUPPORT
	if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) &&
		(tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "PMF connection, Ignore AddBaReq Send DUE TO NOT IN PORTSECURED\n");
		MlmeFreeMemory(pOutBuffer);
		return;
	}
#endif /* DOT11W_PMF_SUPPORT */

		ActHeaderInit(pAd, &Frame.Hdr, pInfo->pAddr, wdev->if_addr, wdev->bssid);

	Frame.Category = CATEGORY_BA;
	Frame.Action = ADDBA_REQ;
	Frame.BaParm.AMSDUSupported = pInfo->amsdu_support;
	Frame.BaParm.BAPolicy = IMMED_BA;
	Frame.BaParm.TID = pInfo->TID;
	Frame.BaParm.BufSize = pInfo->BaBufSize;
	Frame.Token = pInfo->Token;
	Frame.TimeOutValue = pInfo->TimeOutValue;
	Frame.BaStartSeq.field.FragNum = 0;
	Frame.BaStartSeq.field.StartSeq = chip_get_tid_sn(pAd, pInfo->Wcid, pInfo->TID);

#ifdef UNALIGNMENT_SUPPORT
	{
		BA_PARM tmpBaParm;

		NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&Frame.BaParm), sizeof(BA_PARM));
		*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
		NdisMoveMemory((PUCHAR)(&Frame.BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
	}
#else
	*(USHORT *)(&(Frame.BaParm)) = cpu2le16((*(USHORT *)(&(Frame.BaParm))));
#endif /* UNALIGNMENT_SUPPORT */
	Frame.TimeOutValue = cpu2le16(Frame.TimeOutValue);
	Frame.BaStartSeq.word = cpu2le16(Frame.BaStartSeq.word);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(FRAME_ADDBA_REQ), &Frame,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[pInfo->TID]), pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			  "BA - Send ADDBA request. StartSeq = %x,  FrameLen = %ld. BufSize = %d\n",
			  Frame.BaStartSeq.field.StartSeq, FrameLen, pInfo->BaBufSize);
}


/*
    ==========================================================================
    Description:
	send DELBA and delete BaEntry if any
    Parametrs:
	Elem - MLME message MLME_DELBA_REQ_STRUCT

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeDELBAAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MLME_DELBA_REQ_STRUCT *pInfo;
	PUCHAR pOutBuffer = NULL;
	FRAME_DELBA_REQ Frame;
	ULONG FrameLen;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev;
	UCHAR *src_addr = NULL;

	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s\n", __func__);

	if (MlmeDelBAReqSanity(pAd, Elem->Msg, Elem->MsgLen) != TRUE)
		return;

	pInfo = (MLME_DELBA_REQ_STRUCT *)Elem->Msg;
	NdisZeroMemory(&Frame, sizeof(FRAME_DELBA_REQ));

	if (MlmeAllocateMemory(pAd, &pOutBuffer) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "BA - MlmeDELBAAction() allocate memory failed.\n");
		return;
	}

	pEntry = &pAd->MacTab.Content[pInfo->Wcid];

	if (!pEntry->wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "No binding wdev for wcid(%d)\n", pInfo->Wcid);
		MlmeFreeMemory(pOutBuffer);
		return;
	}

	wdev = pEntry->wdev;
	src_addr = wdev->if_addr;

	/* SEND DELBA FRAME*/
	FrameLen = 0;
	ActHeaderInit(pAd, &Frame.Hdr, pEntry->Addr, src_addr, pEntry->wdev->bssid);
	Frame.Category = CATEGORY_BA;
	Frame.Action = DELBA;
	Frame.DelbaParm.Initiator = pInfo->Initiator;
	Frame.DelbaParm.TID = pInfo->TID;
	Frame.ReasonCode = REASON_TIMEOUT;
	*(USHORT *)(&Frame.DelbaParm) = cpu2le16(*(USHORT *)(&Frame.DelbaParm));
	Frame.ReasonCode = cpu2le16(Frame.ReasonCode);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(FRAME_DELBA_REQ), &Frame,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s Initiator = %d DELBA sent\n", __func__, pInfo->Initiator);
}

VOID mlme_send_addba_resp(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PUCHAR pOutBuffer = NULL;
	FRAME_ADDBA_RSP ADDframe;
	MAC_TABLE_ENTRY *pMacEntry;
	MLME_ADDBA_RESP_STRUCT *pInfo;
	UCHAR *pAddr;
	ULONG FrameLen;
#ifdef CONFIG_STA_SUPPORT
	struct wifi_dev *wdev = Elem->wdev;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif

	if (mlme_addba_resp_sanity(pAd, Elem->Msg, Elem->MsgLen) != TRUE)
		return;

	if (MlmeAllocateMemory(pAd, &pOutBuffer) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "ACTION - PeerBAAction() allocate memory failed\n");
		return;
	}

	NdisZeroMemory(&ADDframe, sizeof(FRAME_ADDBA_RSP));

	pInfo = (MLME_ADDBA_RESP_STRUCT *)Elem->Msg;
	pMacEntry = &pAd->MacTab.Content[pInfo->wcid];
	pAddr = pInfo->addr;

#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)

	if (pMacEntry) {
#ifdef CONFIG_STA_SUPPORT

		if (ADHOC_ON(pAd)
#ifdef DOT11Z_TDLS_SUPPORT
			|| (IS_ENTRY_TDLS(pMacEntry))
#endif /* DOT11Z_TDLS_SUPPORT */
		   )
			ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, wdev->if_addr, pAd->CommonCfg.Bssid);
		else
#endif /* CONFIG_STA_SUPPORT */
		{
			ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pMacEntry->wdev->if_addr, pMacEntry->wdev->bssid);
		}
	}

#else
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		UCHAR apidx;
#ifdef CONFIG_STA_SUPPORT

		if (IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry))
			ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, wdev->if_addr, pAddr);
		else
#endif /* CONFIG_STA_SUPPORT */
#ifdef WDS_SUPPORT
			if (IS_ENTRY_WDS(pMacEntry))
				ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, wdev->if_addr, pAddr);
			else
#endif /* WDS_SUPPORT */
			{
				apidx = pMacEntry->func_tb_idx;
				if (!VALID_MBSS(pAd, apidx)) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
							"error AP index \n");
					MlmeFreeMemory(pOutBuffer);
					return;
				}
				ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pAd->ApCfg.MBSSID[apidx].wdev.if_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (ADHOC_ON(pAd)
#ifdef DOT11Z_TDLS_SUPPORT
			|| (IS_ENTRY_TDLS(pMacEntry))
#endif /* DOT11Z_TDLS_SUPPORT */
		   )
			ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pStaCfg->wdev.if_addr, pStaCfg->Bssid);
		else
			ActHeaderInit(pAd, &ADDframe.Hdr, pStaCfg->Bssid, pStaCfg->wdev.if_addr, pAddr);
	}
#endif /* CONFIG_STA_SUPPORT */
#endif /* P2P_SUPPORT */

	ADDframe.Category = CATEGORY_BA;
	ADDframe.Action = ADDBA_RESP;
	ADDframe.Token = pInfo->token;
	ADDframe.StatusCode = pInfo->status;
	ADDframe.BaParm.BAPolicy = IMMED_BA;
	ADDframe.BaParm.AMSDUSupported = pInfo->amsdu_support;
	ADDframe.BaParm.TID = pInfo->tid;
	ADDframe.BaParm.BufSize = pInfo->buf_size;
	ADDframe.TimeOutValue = pInfo->timeout;

#ifdef UNALIGNMENT_SUPPORT
	{
		BA_PARM tmpBaParm;

		NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&ADDframe.BaParm), sizeof(BA_PARM));
		*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
		NdisMoveMemory((PUCHAR)(&ADDframe.BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
	}
#else
	*(USHORT *)(&ADDframe.BaParm) = cpu2le16(*(USHORT *)(&ADDframe.BaParm));
#endif /* UNALIGNMENT_SUPPORT */
	ADDframe.StatusCode = cpu2le16(ADDframe.StatusCode);
	ADDframe.TimeOutValue = cpu2le16(ADDframe.TimeOutValue);
	MakeOutgoingFrame(pOutBuffer,               &FrameLen,
					  sizeof(FRAME_ADDBA_RSP),  &ADDframe,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "%s(%d): TID(%d), BufSize(%d) <==\n", __func__, Elem->Wcid, ADDframe.BaParm.TID,
			 ADDframe.BaParm.BufSize);
}

#endif /* DOT11_N_SUPPORT */


VOID MlmeQOSAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2)
	MLME_QOS_ACTION_STRUCT *pInfo;
	PUCHAR			pOutBuffer = NULL;
	ULONG			FrameLen = 0;
	NDIS_STATUS	NStatus;
	FRAME_ACTION_HDR	Frame;

	pInfo = (MLME_QOS_ACTION_STRUCT *)Elem->Msg;
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ACT - MlmeQOSAction() allocate memory failed\n");
		return;
	}

	if (pInfo->ActionField == ACTION_QOSMAP_CONFIG) {
		UCHAR OosMapIE = IE_QOS_MAP_SET;
		QOSMAP_SET *pQosMapBuf = &pInfo->QOSMap;
		BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[pInfo->apidx];
		UCHAR	ielen = 0;

		ActHeaderInit(pAd, &Frame.Hdr, pInfo->Addr, mbss->wdev.bssid, mbss->wdev.bssid);
		Frame.Category = CATEGORY_QOS;
		Frame.Action = ACTION_QOSMAP_CONFIG;
		ielen = pQosMapBuf->DSCP_Field_Len;
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(FRAME_ACTION_HDR),	  &Frame,
						  1,			&OosMapIE,
						  1,			&ielen,
						  ielen,		pQosMapBuf->DSCP_Field,
						  END_OF_ARGS);
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	}

	MlmeFreeMemory(pOutBuffer);
#endif /* defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2) */
}


VOID MlmeDLSAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
}


VOID MlmeInvalidAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	/*PUCHAR		   pOutBuffer = NULL;*/
	/*Return the receiving frame except the MSB of category filed set to 1.  7.3.1.11*/
}


VOID PeerQOSAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
}




#ifdef DOT11_N_SUPPORT
VOID PeerBAAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 frame = (PFRAME_802_11)&Elem->Msg;
	/* Bypass HTC len 4 bytes */
	UINT8 htc_len = (frame->Hdr.FC.Order) ? 4 : 0;
	UINT8 Action = 0;

	if (htc_len) {
		NdisMoveMemory((void *)(Elem->Msg+LENGTH_802_11),
			(void *)(Elem->Msg+LENGTH_802_11+htc_len),
			(Elem->MsgLen-htc_len));
	}

	Action = Elem->Msg[LENGTH_802_11 + 1];

	switch (Action) {
	case ADDBA_REQ:
		peer_addba_req_action(pAd, Elem);
		break;

	case ADDBA_RESP:
		peer_addba_rsp_action(pAd, Elem);
		break;

	case DELBA:
		peer_delba_action(pAd, Elem);
		break;
	}
}


#ifdef DOT11N_DRAFT3
#ifdef CONFIG_AP_SUPPORT
extern UCHAR get_regulatory_class(IN PRTMP_ADAPTER pAd, UCHAR Channel, USHORT PhyMode, struct wifi_dev *wdev);

VOID ApPublicAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR	Action = Elem->Msg[LENGTH_802_11 + 1];
	BSS_2040_COEXIST_IE	 BssCoexist;

	/* Format as in IEEE 7.4.7.2*/
	if (Action == ACTION_BSS_2040_COEXIST)
		BssCoexist.word = Elem->Msg[LENGTH_802_11 + 2];
}


VOID SendBSS2040CoexistMgmtAction(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN UCHAR apidx,
	IN UCHAR InfoReq)
{
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	FRAME_ACTION_HDR Frame;
	ULONG FrameLen;
	BSS_2040_COEXIST_ELEMENT BssCoexistInfo;
	BSS_2040_INTOLERANT_CH_REPORT BssIntolerantInfo;
	UCHAR *pAddr1;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (wdev) {
		if (WDEV_BSS_STATE(wdev) < BSS_READY) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s, wdev(%d) bss(%d) not ready (state:%d)!!\n",
					 __func__, wdev->wdev_idx, apidx, WDEV_BSS_STATE(wdev));
			return;
		}

		if (!WMODE_CAP_2G(wdev->PhyMode))
			return;
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SendBSS2040CoexistMgmtAction(): Wcid=%d, apidx=%d, InfoReq=%d!\n", Wcid, apidx, InfoReq);
	NdisZeroMemory((PUCHAR)&BssCoexistInfo, sizeof(BSS_2040_COEXIST_ELEMENT));
	NdisZeroMemory((PUCHAR)&BssIntolerantInfo, sizeof(BSS_2040_INTOLERANT_CH_REPORT));
	BssCoexistInfo.ElementID = IE_2040_BSS_COEXIST;
	BssCoexistInfo.Len = 1;
	BssCoexistInfo.BssCoexistIe.word = pAd->CommonCfg.LastBSSCoexist2040.word;
	BssCoexistInfo.BssCoexistIe.field.InfoReq = InfoReq;
	BssIntolerantInfo.ElementID = IE_2040_BSS_INTOLERANT_REPORT;
	BssIntolerantInfo.Len = 1;
	BssIntolerantInfo.RegulatoryClass = get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev);
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ACT - SendBSS2040CoexistMgmtAction() allocate memory failed\n");
		return;
	}

	pAddr1 = pAd->MacTab.Content[Wcid].Addr;
	ActHeaderInit(pAd, &Frame.Hdr, pAddr1, wdev->if_addr, wdev->bssid);
	Frame.Category = CATEGORY_PUBLIC;
	Frame.Action = ACTION_BSS_2040_COEXIST;
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(FRAME_ACTION_HDR), &Frame,
					  sizeof(BSS_2040_COEXIST_ELEMENT), &BssCoexistInfo,
					  sizeof(BSS_2040_INTOLERANT_CH_REPORT), &BssIntolerantInfo,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACT - SendBSS2040CoexistMgmtAction(BSSCoexist2040=0x%x)\n", BssCoexistInfo.BssCoexistIe.word);
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
VOID StaPublicAction(RTMP_ADAPTER *pAd, BSS_2040_COEXIST_IE *pBssCoexIE, struct wifi_dev *wdev)
{
	MLME_SCAN_REQ_STRUCT ScanReq;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACTION - StaPeerPublicAction  Bss2040Coexist = %x\n", *((PUCHAR)pBssCoexIE));

	/* AP asks Station to return a 20/40 BSS Coexistence mgmt frame.  So we first starts a scan, then send back 20/40 BSS Coexistence mgmt frame */
	if ((pBssCoexIE->field.InfoReq == 1) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040))) {
		/* Clear record first.  After scan , will update those bit and send back to transmiter.*/
		pAd->CommonCfg.BSSCoexist2040.field.InfoReq = 1;
		pAd->CommonCfg.BSSCoexist2040.field.Intolerant40 = 0;
		pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 0;
		/* Clear Trigger event table*/
		TriEventInit(pAd);
		/* Fill out stuff for scan request  and kick to scan*/
		ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_2040_BSS_COEXIST);
		cntl_scan_request(wdev, &ScanReq);
	}
}
#endif /* CONFIG_STA_SUPPORT */


/*
Description : Build Intolerant Channel Rerpot from Trigger event table.
return : how many bytes copied.
*/
ULONG BuildIntolerantChannelRep(RTMP_ADAPTER *pAd, UCHAR *pDest)
{
	ULONG			FrameLen = 0;
	ULONG			ReadOffset = 0;
	UCHAR			i, j, k, idx = 0;
	UCHAR			ChannelList[MAX_TRIGGER_EVENT];
	UCHAR			TmpRegClass;
	UCHAR			RegClassArray[7] = {0, 11, 12, 32, 33, 54, 55}; /* Those regulatory class has channel in 2.4GHz. See Annex J.*/

	RTMPZeroMemory(ChannelList, MAX_TRIGGER_EVENT);

	/* Find every regulatory class*/
	for (k = 0; k < 7; k++) {
		TmpRegClass = RegClassArray[k];
		idx = 0;

		/* Find Channel report with the same regulatory class in 2.4GHz.*/
		for (i = 0; i < pAd->CommonCfg.TriggerEventTab.EventANo; i++) {
			if (pAd->CommonCfg.TriggerEventTab.EventA[i].bValid == TRUE) {
				if (pAd->CommonCfg.TriggerEventTab.EventA[i].RegClass == TmpRegClass) {
					for (j = 0; j < idx; j++) {
						if (ChannelList[j] == (UCHAR)pAd->CommonCfg.TriggerEventTab.EventA[i].Channel)
							break;
					}

					if ((j == idx)) {
						ChannelList[idx] = (UCHAR)pAd->CommonCfg.TriggerEventTab.EventA[i].Channel;
						idx++;
					}

					pAd->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;
				}

				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ACT - BuildIntolerantChannelRep , Total Channel number = %d\n", idx);
			}
		}

		/* idx > 0 means this regulatory class has some channel report and need to copy to the pDest.*/
		if (idx > 0) {
			/* For each regaulatory IE report, contains all channels that has the same regulatory class.*/
			*(pDest + ReadOffset) = IE_2040_BSS_INTOLERANT_REPORT;  /* IE*/
			*(pDest + ReadOffset + 1) = 1 + idx;	/* Len = RegClass byte + channel byte.*/
			*(pDest + ReadOffset + 2) = TmpRegClass;	/* Len = RegClass byte + channel byte.*/
			RTMPMoveMemory(pDest + ReadOffset + 3, ChannelList, idx);
			FrameLen += (3 + idx);
			ReadOffset += (3 + idx);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ACT-BuildIntolerantChannelRep(Size=%ld)\n", FrameLen);
	hex_dump("ACT-pDestMsg", pDest, FrameLen);
	return FrameLen;
}


/*
	==========================================================================
	Description:
	After scan, Update 20/40 BSS Coexistence IE and send out.
	According to 802.11n D3.03 11.14.10

	Parameters:
	==========================================================================
 */
VOID Update2040CoexistFrameAndNotify(
	IN PRTMP_ADAPTER	pAd,
	IN UINT16  Wcid,
	IN BOOLEAN	bAddIntolerantCha)
{
	BSS_2040_COEXIST_IE		OldValue;

	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACT -BSSCoexist2040 = %x. EventANo = %d.\n",
			 pAd->CommonCfg.BSSCoexist2040.word,
			 pAd->CommonCfg.TriggerEventTab.EventANo);
	OldValue.word = pAd->CommonCfg.BSSCoexist2040.word;
	/* Reset value.*/
	pAd->CommonCfg.BSSCoexist2040.word = 0;

	if (pAd->CommonCfg.TriggerEventTab.EventBCountDown > 0)
		pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 1;

	/* Need to check !!!!*/
	/* How STA will set Intolerant40 if implementation dependent. Now we don't set this bit first!!!!!*/
	/* So Only check BSS20WidthReq change.*/
	/*if (OldValue.field.BSS20WidthReq != pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq)*/
	{
		Send2040CoexistAction(pAd, Wcid, bAddIntolerantCha);
	}
}


/*
Description : Send 20/40 BSS Coexistence Action frame If one trigger event is triggered.
*/
VOID Send2040CoexistAction(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN BOOLEAN bAddIntolerantCha)
{
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	FRAME_ACTION_HDR Frame;
	ULONG FrameLen;
	UINT32 IntolerantChaRepLen;
	UCHAR HtLen = 1;
#ifdef CONFIG_STA_SUPPORT
	struct wifi_dev *wdev = pAd->MacTab.Content[Wcid].wdev;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT
	UCHAR apidx = 0;
#endif /* APCLI_SUPPORT */
	IntolerantChaRepLen = 0;
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ACT - Send2040CoexistAction() allocate memory failed\n");
		return;
	}

#ifdef CONFIG_STA_SUPPORT
	ActHeaderInit(pAd, &Frame.Hdr, pAd->MacTab.Content[Wcid].Addr, wdev->if_addr, pStaCfg->Bssid);
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT

	if (IS_ENTRY_PEER_AP(&pAd->MacTab.Content[Wcid])) {
		apidx = pAd->MacTab.Content[Wcid].func_tb_idx;
		ActHeaderInit(pAd, &Frame.Hdr, pAd->MacTab.Content[Wcid].Addr, pAd->StaCfg[apidx].wdev.if_addr, pAd->MacTab.Content[Wcid].Addr);
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\x1b[31m\n"MACSTR" \x1b[m\n",
				 MAC2STR(pAd->MacTab.Content[Wcid].Addr));
	}

#endif /* APCLI_SUPPORT */
	Frame.Category = CATEGORY_PUBLIC;
	Frame.Action = ACTION_BSS_2040_COEXIST; /*COEXIST_2040_ACTION;*/
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(FRAME_ACTION_HDR),	  &Frame,
					  1,                                &BssCoexistIe,
					  1,                                &HtLen,
					  1,                                &pAd->CommonCfg.BSSCoexist2040.word,
					  END_OF_ARGS);

	if (bAddIntolerantCha == TRUE)
		IntolerantChaRepLen = BuildIntolerantChannelRep(pAd, pOutBuffer + FrameLen);

	/*2009 PF#3: IOT issue with Motorola AP. It will not check the field of BSSCoexist2040.*/
	/*11.14.12 Switching between 40 MHz and 20 MHz*/
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IntolerantChaRepLen=%d, BSSCoexist2040=0x%x!\n",
			 IntolerantChaRepLen, pAd->CommonCfg.BSSCoexist2040.word);

	if (!((IntolerantChaRepLen == 0) && (pAd->CommonCfg.BSSCoexist2040.word == 0)))
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen + IntolerantChaRepLen);

	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACT - Send2040CoexistAction( BSSCoexist2040 = 0x%x )\n", pAd->CommonCfg.BSSCoexist2040.word);
}

#ifdef CONFIG_STA_SUPPORT
VOID UpdateBssScanParm(RTMP_ADAPTER *pAd, OVERLAP_BSS_SCAN_IE APBssScan)
{
	pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = le2cpu16(APBssScan.DelayFactor); /*APBssScan.DelayFactor[1] * 256 + APBssScan.DelayFactor[0];*/

	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor < 5) || (pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor > 100)) {
		/*MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"ACT - UpdateBssScanParm( Dot11BssWidthChanTranDelayFactor out of range !!!!)\n");*/
		pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = 5;
	}

	pAd->CommonCfg.Dot11BssWidthTriggerScanInt = le2cpu16(APBssScan.TriggerScanInt); /*APBssScan.TriggerScanInt[1] * 256 + APBssScan.TriggerScanInt[0];*/

	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11BssWidthTriggerScanInt < 10) || (pAd->CommonCfg.Dot11BssWidthTriggerScanInt > 900)) {
		/*MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"ACT - UpdateBssScanParm( Dot11BssWidthTriggerScanInt out of range !!!!)\n");*/
		pAd->CommonCfg.Dot11BssWidthTriggerScanInt = 900;
	}

	pAd->CommonCfg.Dot11OBssScanPassiveDwell = le2cpu16(APBssScan.ScanPassiveDwell); /*APBssScan.ScanPassiveDwell[1] * 256 + APBssScan.ScanPassiveDwell[0];*/

	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11OBssScanPassiveDwell < 5) || (pAd->CommonCfg.Dot11OBssScanPassiveDwell > 1000)) {
		/*MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"ACT - UpdateBssScanParm( Dot11OBssScanPassiveDwell out of range !!!!)\n");*/
		pAd->CommonCfg.Dot11OBssScanPassiveDwell = 20;
	}

	pAd->CommonCfg.Dot11OBssScanActiveDwell = le2cpu16(APBssScan.ScanActiveDwell); /*APBssScan.ScanActiveDwell[1] * 256 + APBssScan.ScanActiveDwell[0];*/

	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11OBssScanActiveDwell < 10) || (pAd->CommonCfg.Dot11OBssScanActiveDwell > 1000)) {
		/*MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"ACT - UpdateBssScanParm( Dot11OBssScanActiveDwell out of range !!!!)\n");*/
		pAd->CommonCfg.Dot11OBssScanActiveDwell = 10;
	}

	pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = le2cpu16(APBssScan.PassiveTalPerChannel); /*APBssScan.PassiveTalPerChannel[1] * 256 + APBssScan.PassiveTalPerChannel[0];*/

	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel < 200) || (pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel > 10000)) {
		/*MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"ACT - UpdateBssScanParm( Dot11OBssScanPassiveTotalPerChannel out of range !!!!)\n");*/
		pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = 200;
	}

	pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = le2cpu16(APBssScan.ActiveTalPerChannel); /*APBssScan.ActiveTalPerChannel[1] * 256 + APBssScan.ActiveTalPerChannel[0];*/

	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel < 20) || (pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel > 10000)) {
		/*MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"ACT - UpdateBssScanParm( Dot11OBssScanActiveTotalPerChannel out of range !!!!)\n");*/
		pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = 20;
	}

	pAd->CommonCfg.Dot11OBssScanActivityThre = le2cpu16(APBssScan.ScanActThre); /*APBssScan.ScanActThre[1] * 256 + APBssScan.ScanActThre[0];*/

	/* out of range defined in MIB... So fall back to default value.*/
	if (pAd->CommonCfg.Dot11OBssScanActivityThre > 100) {
		/*MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"ACT - UpdateBssScanParm( Dot11OBssScanActivityThre out of range !!!!)\n");*/
		pAd->CommonCfg.Dot11OBssScanActivityThre = 25;
	}

	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (ULONG)(pAd->CommonCfg.Dot11BssWidthTriggerScanInt * pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
	/*MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,("ACT - UpdateBssScanParm( Dot11BssWidthTriggerScanInt = %d )\n", pAd->CommonCfg.Dot11BssWidthTriggerScanInt));*/
}

#endif /* CONFIG_STA_SUPPORT */


BOOLEAN ChannelSwitchSanityCheck(
	IN	PRTMP_ADAPTER	pAd,
	IN    UINT16  Wcid,
	IN    UCHAR  NewChannel,
	IN    UCHAR  Secondary)
{
	UCHAR		i;
	struct wifi_dev *wdev = NULL;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;

	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return FALSE;

	wdev = pAd->MacTab.Content[Wcid].wdev;
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	if ((NewChannel > 7) && (Secondary == 1))
		return FALSE;

	if ((NewChannel < 5) && (Secondary == 3))
		return FALSE;

	/* 0. Check if new channel is in the channellist.*/
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == NewChannel)
			break;
	}

	if (i == pChCtrl->ChListNum)
		return FALSE;

	return TRUE;
}


VOID ChannelSwitchAction(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN UCHAR NewChannel,
	IN UCHAR Secondary)
{
	struct wifi_dev *wdev = NULL;
	struct freq_cfg freq;

	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NewChannel=%d, Secondary=%d\n",
			 NewChannel, Secondary);

	if (ChannelSwitchSanityCheck(pAd, Wcid, NewChannel, Secondary) == FALSE)
		return;

	wdev = pAd->MacTab.Content[Wcid].wdev;
	os_zero_mem(&freq, sizeof(freq));

	if (!wdev)
		return;

	wdev->channel = NewChannel;
	freq.prim_ch = wdev->channel;
	freq.ext_cha = Secondary;
	freq.ht_bw  = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	freq.vht_bw = wlan_operate_get_vht_bw(wdev);
	freq.cen_ch_2 = wlan_operate_get_cen_ch_2(wdev);
#endif /*DOT11_VHT_AC*/

	if (Secondary == EXTCHA_NONE) {
		pAd->MacTab.Content[Wcid].HTPhyMode.field.BW = 0;
		freq.ht_bw = HT_BW_20;
	}
	/* 1.  Switch to BW = 40 And Station supports BW = 40.*/
	else if (((Secondary == EXTCHA_ABOVE) || (Secondary == EXTCHA_BELOW)) &&
			 (freq.ht_bw == HT_BW_40)
			)
		pAd->MacTab.Content[Wcid].HTPhyMode.field.BW = 1;

	if (NewChannel != 0) {
		wlan_operate_set_phy(wdev, &freq);
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%dMHz LINK UP, CtrlChannel=%d,  CentralChannel= %d\n",
				 (wlan_operate_get_bw(wdev) == BW_40 ? 40 : 20),
				 wdev->channel,
				 wlan_operate_get_cen_ch_1(wdev));
	}
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

VOID PeerPublicAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11 + 1];
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
#endif
#ifdef CONFIG_AP_SUPPORT
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#if defined(WAPP_SUPPORT) && defined(CONFIG_AP_SUPPORT)
#ifdef DPP_SUPPORT
	if (!pAd->bDppEnable)
#endif /* DPP_SUPPORT */
		if (!GasEnable(pAd, Elem))
#endif
#ifdef P2P_SUPPORT
		if (!P2P_INF_ON(pAd))
#endif /* P2P_SUPPORT */
			if ((!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
#ifdef DOT11Z_TDLS_SUPPORT
				&& (Action != ACTION_TDLS_DISCOVERY_RSP)
#endif /* DOT11Z_TDLS_SUPPORT // */
			   )
				return;

	switch (Action) {
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	case ACTION_BSS_2040_COEXIST: {	/* Format defined in IEEE 7.4.7a.1 in 11n Draf3.03*/
		/*UCHAR	BssCoexist;*/
		BSS_2040_COEXIST_ELEMENT *pCoexistInfo;
		BSS_2040_COEXIST_IE *pBssCoexistIe;
		BSS_2040_INTOLERANT_CH_REPORT *pIntolerantReport = NULL;
#ifdef CONFIG_AP_SUPPORT
		struct wifi_dev *wdev = NULL;

		if (!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Wrong Wcid:%d !!! need to check the root cause!!!!",
					  Elem->Wcid);
			return;
		}

		wdev = pAd->MacTab.Content[Elem->Wcid].wdev;
#endif

		if (Elem->MsgLen <= (LENGTH_802_11 + sizeof(BSS_2040_COEXIST_ELEMENT))) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ACTION - 20/40 BSS Coexistence Management Frame length too short! len = %ld!\n", Elem->MsgLen);
			break;
		}

		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACTION - 20/40 BSS Coexistence Management action---->\n");
		hex_dump("CoexistenceMgmtFrame", Elem->Msg, Elem->MsgLen);
		pCoexistInfo = (BSS_2040_COEXIST_ELEMENT *) &Elem->Msg[LENGTH_802_11 + 2];

		/*hex_dump("CoexistInfo", (PUCHAR)pCoexistInfo, sizeof(BSS_2040_COEXIST_ELEMENT));*/
		if (Elem->MsgLen >= (LENGTH_802_11 + sizeof(BSS_2040_COEXIST_ELEMENT) + sizeof(BSS_2040_INTOLERANT_CH_REPORT)))
			pIntolerantReport = (BSS_2040_INTOLERANT_CH_REPORT *)((PUCHAR)pCoexistInfo + sizeof(BSS_2040_COEXIST_ELEMENT));

		/*hex_dump("IntolerantReport ", (PUCHAR)pIntolerantReport, sizeof(BSS_2040_INTOLERANT_CH_REPORT));*/

		if (
#ifdef BW_VENDOR10_CUSTOM_FEATURE
			IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd, HcGetBandByWdev(wdev)) ||
#endif
			pAd->CommonCfg.bBssCoexEnable == FALSE || (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "20/40 BSS CoexMgmt=%d, bForty_Mhz_Intolerant=%d, ignore this action!!\n",
					 pAd->CommonCfg.bBssCoexEnable,
					 pAd->CommonCfg.bForty_Mhz_Intolerant);
			break;
		}

		pBssCoexistIe = (BSS_2040_COEXIST_IE *)(&pCoexistInfo->BssCoexistIe);
#ifdef CONFIG_AP_SUPPORT
#ifdef P2P_SUPPORT

		if ((Elem->OpMode == OPMODE_AP) || (pAd->OpMode == OPMODE_AP))
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT */
		{
#ifdef APCLI_SUPPORT

			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[Elem->Wcid])) {
#endif /* APCLI_SUPPORT */
				BOOLEAN		bNeedFallBack = FALSE;
				UCHAR BandIdx = 0;
				CHANNEL_CTRL *pChCtrl = NULL;
				if (wdev) {
					BandIdx = HcGetBandByWdev(wdev);
				} else {
					BandIdx = BAND0;
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PeerPublicAction] wdev = NULL\n");
				}
				pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

				/*ApPublicAction(pAd, Elem);*/
				if ((pBssCoexistIe->field.BSS20WidthReq == 1) || (pBssCoexistIe->field.Intolerant40 == 1)) {
					bNeedFallBack = TRUE;
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS_2040_COEXIST: BSS20WidthReq=%d, Intolerant40=%d!\n", pBssCoexistIe->field.BSS20WidthReq, pBssCoexistIe->field.Intolerant40);
				} else if ((pIntolerantReport) && (pIntolerantReport->Len > 1)) {
					int i;
					UCHAR *ptr;
					INT retVal;
					BSS_COEX_CH_RANGE coexChRange;

					ptr = pIntolerantReport->ChList;
					bNeedFallBack = TRUE;
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "The pIntolerantReport len = %d, chlist=", pIntolerantReport->Len);

					for (i = 0; i < (pIntolerantReport->Len - 1); i++, ptr++)
						MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%d,", *ptr);

					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
					if (wdev) {
					retVal = GetBssCoexEffectedChRange(pAd, wdev, &coexChRange, wdev->channel);

					if (retVal == TRUE) {
						ptr = pIntolerantReport->ChList;
						bNeedFallBack = FALSE;
						MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Check IntolerantReport Channel List in our effectedChList(%d~%d)\n",
								pChCtrl->ChList[coexChRange.effectChStart].Channel,
								pChCtrl->ChList[coexChRange.effectChEnd].Channel);

						for (i = 0; i < (pIntolerantReport->Len - 1); i++, ptr++) {
							UCHAR chEntry;

							chEntry = *ptr;

							if (chEntry >= pChCtrl->ChList[coexChRange.effectChStart].Channel &&
								chEntry <= pChCtrl->ChList[coexChRange.effectChEnd].Channel) {
								MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Found Intolerant channel in effect range=%d!\n", *ptr);
								bNeedFallBack = TRUE;
								break;
							}
						}
						MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "After CoexChRange Check, bNeedFallBack=%d!\n", bNeedFallBack);
					}
					}

					if (bNeedFallBack) {
						pBssCoexistIe->field.Intolerant40 = 1;
						pBssCoexistIe->field.BSS20WidthReq = 1;
					}
				}

				if (bNeedFallBack) {
					UCHAR apidx;

					NdisMoveMemory((PUCHAR)&pAd->CommonCfg.LastBSSCoexist2040,
								   (PUCHAR)pBssCoexistIe,
								   sizeof(BSS_2040_COEXIST_IE));
					pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;

					if (!(pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED)) {
						MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								  "Fire the Bss2040CoexistTimer with timeout=%ld!\n",
								  pAd->CommonCfg.Dot11BssWidthChanTranDelay);
						pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_TIMER_FIRED;
						/* More 5 sec for the scan report of STAs.*/
						RTMPSetTimer(&pAd->CommonCfg.Bss2040CoexistTimer,
									 (pAd->CommonCfg.Dot11BssWidthChanTranDelay + 5) * 1000);
					} else {
						MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								 "Already fallback to 20MHz, Extend the timeout of Bss2040CoexistTimer!\n");
						/* More 5 sec for the scan report of STAs.*/
						RTMPModTimer(&pAd->CommonCfg.Bss2040CoexistTimer,
									 (pAd->CommonCfg.Dot11BssWidthChanTranDelay + 5) * 1000);
					}

					for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
						SendBSS2040CoexistMgmtAction(pAd, MCAST_WCID_TO_REMOVE, apidx, 0);

					/*FIXME: Carter, I don't think call update routine here is a good choice,
						the thing need to do for 20/40 update in legacy chip is a historical burden
					*/
					if (wdev) {
						if ((pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_SYNC) &&
							(pAd->CommonCfg.bForty_Mhz_Intolerant == FALSE)) {
							UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
							UCHAR cfg_ext_cha = wlan_config_get_ext_cha(wdev);

							MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
									  "DTIM Period reached, BSS20WidthReq=%d, Intolerant40=%d!\n",
									  pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq,
									  pAd->CommonCfg.LastBSSCoexist2040.field.Intolerant40);
							pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_SYNC);

							if (pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq ||
								pAd->CommonCfg.LastBSSCoexist2040.field.Intolerant40) {
								wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
							} else {
								/*recover to origin bw & extcha*/
								wlan_operate_set_ht_bw(wdev, cfg_ht_bw, cfg_ext_cha);
							}

							MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
									  "\tNow RecomWidth=%d, ExtChanOffset=%d\n",
									  wlan_operate_get_ht_bw(wdev),
									  wlan_operate_get_ext_cha(wdev));
							pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_NOTIFY;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

							/* Update BSS2040COEX for pEntry */
							if (cap->fgRateAdaptFWOffload == TRUE) {
								CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

								NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
								rRaParam.u4Field = RA_PARAM_HT_2040_COEX;
								RAParamUpdate(pAd, &pAd->MacTab.Content[Elem->Wcid], &rRaParam);
							}

#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
						}

						MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								 "ACTION_BSS_2040_COEXIST Update Beacon for idx:%d\n",
								 pAd->MacTab.Content[Elem->Wcid].apidx);
						UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
					}
				}

#ifdef APCLI_SUPPORT
			}

#endif /* APCLI_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT

		if ((Elem->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_STA))
#else
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
		{
			if (INFRA_ON(pStaCfg))
				StaPublicAction(pAd, pBssCoexistIe, Elem->wdev);
		}

#endif /* CONFIG_STA_SUPPORT */
	}
	break;
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#if defined(WAPP_SUPPORT) || defined(FTM_SUPPORT)

	case ACTION_GAS_INIT_REQ:
		if (GasEnable(pAd, Elem)
#ifdef DPP_SUPPORT
		    || pAd->bDppEnable			/* Bypassing Gas Enable Check for DPP Gas Frames */
#endif /* DPP_SUPPORT */
		   )
			ReceiveGASInitReq(pAd, Elem);

		break;

#ifdef DPP_SUPPORT
	case ACTION_GAS_INIT_RSP:
		DPP_ReceiveGASInitRsp(pAd, Elem);
		break;
	case ACTION_GAS_CB_RSP:
		DPP_ReceiveGASCBResp(pAd, Elem);
		break;
#endif /* DPP_SUPPORT */

	case ACTION_GAS_CB_REQ:
#ifdef DPP_SUPPORT
			DPP_ReceiveGASCBReq(pAd, Elem);
#endif /* DPP_SUPPORT */
		if (GasEnable(pAd, Elem)
#ifdef MAP_R3
		&& ((IS_MAP_ENABLE(pAd) && !IS_MAP_CERT_ENABLE(pAd))
		|| !IS_MAP_ENABLE(pAd))
#endif /* MAP_R3 */
		)
			ReceiveGASCBReq(pAd, Elem);

		break;
#endif
#endif /* CONFIG_AP_SUPPORT */

	case ACTION_WIFI_DIRECT:
#ifdef DPP_SUPPORT
		if (NdisEqualMemory(&(Elem->Msg[LENGTH_802_11 + 2]), DPP_OUI, OUI_LEN) &&
			Elem->Msg[LENGTH_802_11 + 5] == WFA_DPP_SUBTYPE) {
			if (pAd->bDppEnable) {
				wext_send_dpp_action_frame(pAd, Elem->wdev, &Elem->Msg[10], Elem->Channel,
							  Elem->Msg, Elem->MsgLen, 0);
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"got one DPP puplic action frame\n");
			} else
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPP Disabled DPP puplic action frame not sent to WAPP\n");
			break;
		}
#endif /* DPP_SUPPORT */
#ifdef IWSC_SUPPORT

		if ((pStaCfg->BssType == BSS_ADHOC) &&
			NdisEqualMemory(&(Elem->Msg[LENGTH_802_11 + 2]), IWSC_ACTION_OUI, 4)) {
			MlmeEnqueue(pAd, IWSC_STATE_MACHINE, IWSC_MT2_PEER_ACTION_FRAME, Elem->MsgLen, Elem->Msg, 0);
			RTMP_MLME_HANDLER(pAd);
			break;
		}

#endif /* IWSC_SUPPORT // */
#ifdef P2P_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<---- Public. Vendor OUI+type = %08x\n", *(PULONG)&Elem->Msg[LENGTH_802_11 + 2]);
#ifdef RT_BIG_ENDIAN

		if (SWAP32(*(PUINT32)&Elem->Msg[LENGTH_802_11 + 2]) == P2P_OUI)
#else
		if (*(PUINT32)&Elem->Msg[LENGTH_802_11 + 2] == P2P_OUI)
#endif /* RT_BIG_ENDIAN */
		{
			UCHAR	Subtype = Elem->Msg[LENGTH_802_11 + 6];

			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!!! Public action frames with Vendor specific OUI = WFAP2P.Subtype = %d.\n", Subtype);
			pAd->P2pCfg.bPeriodicListen = FALSE;

			if (pAd->P2pCfg.P2pCounter.bStartScan) {
				pAd->P2pCfg.P2pCounter.bStartScan = FALSE;
				pAd->P2pCfg.P2pCounter.bListen = FALSE;
				pAd->P2pCfg.P2pCounter.bNextScan = FALSE;
				/* update P2P Ctrl State Machine status. */
				MlmeEnqueue(pAd, P2P_CTRL_STATE_MACHINE, P2P_CTRL_DISC_CANL_EVT, 0, NULL, 0);
				RTMP_MLME_HANDLER(pAd);
			}

			switch (Subtype) {
			case GO_NEGOCIATION_REQ:
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "recv Group Nego. Req\n");
				P2pPeerGoNegoReqAction(pAd, Elem);
				break;

			case GO_NEGOCIATION_RSP:
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "recv Group Nego. Rsp\n");
				P2pPeerGoNegoRspAction(pAd, Elem);
				break;

			case GO_NEGOCIATION_CONFIRM:
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "recv Group Nego. Confirm\n");
				P2pPeerGoNegoConfirmAction(pAd, Elem);
				break;

			case P2P_INVITE_REQ:
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "recv Invite Req\n");
				P2pPeerInvitesReqAction(pAd, Elem);
				break;

			case P2P_INVITE_RSP:
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "recv Invite Rsp\n");
				P2pPeerInvitesRspAction(pAd, Elem);
				break;

			case P2P_DEV_DIS_REQ:
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "recv Device Discovery Req\n");
				P2pPeerDevDiscoverReqAction(pAd, Elem);
				break;

			case P2P_DEV_DIS_RSP:
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "recv Device Discovery Rsp\n");
				P2pPeerDeviceDiscRspAction(pAd, Elem);
				break;

			case P2P_PROVISION_REQ:
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "recv Provision Req\n");
				P2pPeerProvisionReqAction(pAd, Elem);
				break;

			case P2P_PROVISION_RSP:
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "recv Provision Rsp\n");
				P2pPeerProvisionRspAction(pAd, Elem);
				break;

			default:
				pAd->P2pCfg.bPeriodicListen = TRUE;
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown Public action frames with Vendor specific OUI = WFAP2P.Subtype = %d\n", Subtype);
				break;
			}
		}

#endif /* P2P_SUPPORT */
		break;
#ifdef DOT11Z_TDLS_SUPPORT

	case ACTION_TDLS_DISCOVERY_RSP:
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (INFRA_ON(pAd))
				TDLS_DiscoveryRspPublicAction(pAd, Elem, Elem->Msg, Elem->MsgLen);
		}
		break;
#endif /* DOT11Z_TDLS_SUPPORT // */

	default:
		break;
	}
}


static VOID ReservedAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Category;

	if (Elem->MsgLen <= LENGTH_802_11)
		return;

	Category = Elem->Msg[LENGTH_802_11];
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Rcv reserved category(%d) Action Frame\n", Category);
	hex_dump("Reserved Action Frame", &Elem->Msg[0], Elem->MsgLen);
}


VOID PeerRMAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
#ifdef DOT11K_RRM_SUPPORT
	UCHAR Action = Elem->Msg[LENGTH_802_11 + 1];
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
		pEntry = &pAd->MacTab.Content[Elem->Wcid];
	else
		return;

	if (!(pEntry->wdev))
		return;

	if (!IS_RRM_ENABLE(pEntry->wdev))
		return;

	switch (Action) {
#ifdef CONFIG_AP_SUPPORT
	case RRM_MEASURE_REP:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Get RRM Measure report.\n");
		RRM_PeerMeasureRepAction(pAd, Elem);
		break;

	case RRM_NEIGHTBOR_REQ:
		RRM_PeerNeighborReqAction(pAd, Elem);
		break;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	case RRM_MEASURE_REQ:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Get RRM Measure request.\n");
		RRM_PeerMeasureReqAction(pAd, Elem);
		break;
#endif /* CONFIG_STA_SUPPORT */
	}

#endif /* DOT11K_RRM_SUPPORT */
	return;
}


#ifdef DOT11_N_SUPPORT

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11N_DRAFT3
VOID SendNotifyBWActionFrame(RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR apidx)
{
	UCHAR *pOutBuffer = NULL, *pAddr1;
	NDIS_STATUS NStatus;
	FRAME_ACTION_HDR Frame;
	ULONG FrameLen;
	struct wifi_dev *wdev;
	UCHAR op_ht_bw;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ACT - SendNotifyBWAction() allocate memory failed\n");
		return;
	}

	pAddr1 = pAd->MacTab.Content[Wcid].Addr;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	op_ht_bw = wlan_operate_get_ht_bw(wdev);
	ActHeaderInit(pAd, &Frame.Hdr, pAddr1, wdev->if_addr, wdev->bssid);
	Frame.Category = CATEGORY_HT;
	Frame.Action = NOTIFY_BW_ACTION;
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(FRAME_ACTION_HDR), &Frame,
					  END_OF_ARGS);
	*(pOutBuffer + FrameLen) = op_ht_bw;
	FrameLen++;
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACT - SendNotifyBWAction(NotifyBW= %d)!\n", op_ht_bw);
}
#endif /* DOT11N_DRAFT3 */
#endif /* CONFIG_AP_SUPPORT */


VOID PeerHTAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11 + 1];
	MAC_TABLE_ENTRY *pEntry;
	UCHAR oldMmpsMode;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
		return;

	pEntry = &pAd->MacTab.Content[Elem->Wcid];

	switch (Action) {
	case NOTIFY_BW_ACTION:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ACTION - HT Notify Channel bandwidth action---->\n");
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);

			if (!pStaCfg || pStaCfg->StaActive.SupportedPhyInfo.bHtEnable == FALSE) {
				/* Note, this is to patch DIR-1353 AP. When the AP set to Wep, it will use legacy mode. But AP still keeps */
				/* sending BW_Notify Action frame, and cause us to linkup and linkdown. */
				/* In legacy mode, don't need to parse HT action frame.*/
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACTION -Ignore HT Notify Channel BW when link as legacy mode. BW = %d--->\n",
						 Elem->Msg[LENGTH_802_11 + 2]);
				break;
			}
		}
#endif /* CONFIG_STA_SUPPORT */

		if (Elem->Msg[LENGTH_802_11 + 2] == 0)	/* 7.4.8.2. if value is 1, keep the same as supported channel bandwidth. */
			pEntry->HTPhyMode.field.BW = HT_BW_20;
		else {
			pEntry->HTPhyMode.field.BW = (pEntry->wdev) ? (pEntry->MaxHTPhyMode.field.BW &
										 (wlan_config_get_ht_bw(pEntry->wdev))) : (pEntry->MaxHTPhyMode.field.BW);
		}

		break;

	case SMPS_ACTION:
		/* 7.3.1.25*/
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACTION - SMPS action---->\n");
		oldMmpsMode = pEntry->MmpsMode;

		if (((Elem->Msg[LENGTH_802_11 + 2] & 0x1) == 0))
			pEntry->MmpsMode = MMPS_DISABLE;
		else if (((Elem->Msg[LENGTH_802_11 + 2] & 0x2) == 0))
			pEntry->MmpsMode = MMPS_STATIC;
		else
			pEntry->MmpsMode = MMPS_DYNAMIC;

		if (oldMmpsMode != pEntry->MmpsMode) {
			if (pEntry->MmpsMode == MMPS_DYNAMIC) {
#ifdef MT_MAC
				AsicSetSMPS(pAd, pEntry->wcid, 1);
#endif /* MT_MAC */
			} else {
#ifdef MT_MAC
				AsicSetSMPS(pAd, pEntry->wcid, 0);
#endif /* MT_MAC */
			}

#ifdef CONFIG_AP_SUPPORT
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

			if (cap->fgRateAdaptFWOffload == TRUE) {
				CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

				NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
				rRaParam.u4Field = RA_PARAM_MMPS_UPDATE;
				RAParamUpdate(pAd, pEntry, &rRaParam);
			} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			{
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					APMlmeDynamicTxRateSwitching(pAd);
				}
			}

#endif /* CONFIG_AP_SUPPORT */
		}

		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				  "Wcid(%d) MIMO PS = %d, fgRateAdaptFWOffload=%d\n",
				  Elem->Wcid, pEntry->MmpsMode, cap->fgRateAdaptFWOffload);
		/* rt2860c : add something for smps change.*/
		break;

	case SETPCO_ACTION:
		break;

	case MIMO_CHA_MEASURE_ACTION:
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Unknown HT Action:%d\n", Action);
		break;
	}
}


#ifdef DOT11_VHT_AC
VOID PeerVHTAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11 + 1];
#ifdef ZERO_LOSS_CSA_SUPPORT
	UINT i = 0;
	INT ZeroLossStaIndex = -1;
	struct DOT11_H *pDot11h = NULL;
#endif /*ZERO_LOSS_CSA_SUPPORT*/

	if (!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
		return;

	switch (Action) {
	case ACT_VHT_OPMODE_NOTIFY: {
		OPERATING_MODE *op_mode = (OPERATING_MODE *)&Elem->Msg[LENGTH_802_11 + 2];
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[Elem->Wcid];
		CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACTION - Operating Mode Notification action---->\n");
		hex_dump("OperatingModeNotify", &Elem->Msg[0], Elem->MsgLen);
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\t RxNssType=%d, RxNss=%d, ChBW=%d\n",
				 op_mode->rx_nss_type, op_mode->rx_nss, op_mode->ch_width);
#ifdef ZERO_LOSS_CSA_SUPPORT
		/*Handle: some sta send VHT action frame after switching channel due to CSA*/
		if (pAd->Zero_Loss_Enable == 1) {
			if (pEntry) {
				for (i = 0; i < 3; i++) {
					if (pAd->ZeroLossSta[i].wcid == pEntry->wcid) {
						ZeroLossStaIndex = i;
						break;
					}
				}
			}

			pDot11h = (struct DOT11_H *)pEntry->wdev->pDot11_H;
			if (ZeroLossStaIndex >= 0) {
				if ((pAd->ZeroLossSta[ZeroLossStaIndex].ChnlSwitchSkipTx)
					&& (pDot11h->ChnlSwitchState >= ASIC_CHANNEL_SWITCH_COMMAND_ISSUED)) {
					pAd->ZeroLossSta[ZeroLossStaIndex].ChnlSwitchSkipTx = 0;
					AsicUpdateSkipTx(pAd, pEntry->wcid, 0); //reset skip tx
					pAd->ZeroLossSta[ZeroLossStaIndex].resume_time = jiffies_to_msecs(jiffies);
					pAd->chan_switch_time[15] = jiffies_to_msecs(jiffies);
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN,
										"Tx Enabled for wcid %d\n", pEntry->wcid);
					return;
				}
			}
		}
#endif /*ZERO_LOSS_CSA_SUPPORT*/
		if (op_mode->rx_nss_type == 0) {
			pEntry->force_op_mode = TRUE;
			NdisMoveMemory(&pEntry->operating_mode, op_mode, 1);
		}

		NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
		rRaParam.u4Field = RA_PARAM_VHT_OPERATING_MODE;
		RAParamUpdate(pAd, pEntry, &rRaParam);
	}
	break;
#ifdef VHT_TXBF_SUPPORT

	case ACT_VHT_COMPRESS_BF: {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ACTION - VHT Compressed Beamforming action---->\n");
		hex_dump("VHT Compressed BF", &Elem->Msg[0], Elem->MsgLen);
		break;
	}

#endif /* VHT_TXBF_SUPPORT */

	default:
		break;
	}
}
#endif /* DOT11_VHT_AC */


/*
	==========================================================================
	Description:
		Retry sending ADDBA Reqest.

	IRQL = DISPATCH_LEVEL

	Parametrs:
	p8023Header: if this is already 802.3 format, p8023Header is NULL

	Return	: TRUE if put into rx reordering buffer, shouldn't indicaterxhere.
				FALSE , then continue indicaterx at this moment.
	==========================================================================
 */
VOID ORIBATimerTimeout(RTMP_ADAPTER *pAd)
{
	MAC_TABLE_ENTRY *pEntry;
	INT i, total;
	UCHAR TID;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = &tr_ctl->ba_ctl;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
	total = pAd->MacTab.Size * NUM_OF_TID;

	for (i = 1; ((i < MAX_LEN_OF_BA_ORI_TABLE) && (total > 0)); i++) {
		if  (ba_ctl->BAOriEntry[i].ORI_BA_Status == Originator_Done) {
			pEntry = &pAd->MacTab.Content[ba_ctl->BAOriEntry[i].Wcid];
			TID = ba_ctl->BAOriEntry[i].TID;
			ASSERT(ba_ctl->BAOriEntry[i].Wcid < GET_MAX_UCAST_NUM(pAd));
		}

		total--;
	}
}


VOID SendRefreshBAR(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
#ifdef VLAN_SUPPORT
	if (pAd->tr_ctl.vlan2ethctrl == TRUE) {
		UCHAR i, TID;
		USHORT idx;
		BA_ORI_ENTRY *pBAEntry = NULL;
		struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
		struct ba_control *ba_ctl = &tr_ctl->ba_ctl;

		for (i = 0; i < NUM_OF_TID; i++) {
			idx = pEntry->BAOriWcidArray[i];
			if (idx == 0)
				continue;

			pBAEntry = &ba_ctl->BAOriEntry[idx];
			if  (pBAEntry->ORI_BA_Status == Originator_Done) {
				TID = pBAEntry->TID;
				ASSERT(pBAEntry->Wcid < GET_MAX_UCAST_NUM(pAd));
				ba_ori_session_tear_down(pAd, pBAEntry->Wcid, TID, FALSE);
			}
		}
	} else
#endif /* VLAN_SUPPORT */
	{
		FRAME_BAR FrameBar;
		ULONG FrameLen;
		NDIS_STATUS NStatus;
		UCHAR *pOutBuffer = NULL, i, TID;
		USHORT Sequence, idx;
		BA_ORI_ENTRY *pBAEntry;
		struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
		struct ba_control *ba_ctl = &tr_ctl->ba_ctl;

		for (i = 0; i < NUM_OF_TID; i++) {
			idx = pEntry->BAOriWcidArray[i];

			if (idx == 0)
				continue;

			pBAEntry = &ba_ctl->BAOriEntry[idx];

			if  (pBAEntry->ORI_BA_Status == Originator_Done) {
				TID = pBAEntry->TID;
				ASSERT(pBAEntry->Wcid < GET_MAX_UCAST_NUM(pAd));

				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/

				if (NStatus != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "BA - MlmeADDBAAction() allocate memory failed\n");
					return;
				}

				Sequence = chip_get_tid_sn(pAd, pEntry->wcid, TID);
				BarHeaderInit(pAd, &FrameBar, pEntry->Addr, pEntry->wdev->if_addr);

				FrameBar.StartingSeq.field.FragNum = 0; /* make sure sequence not clear in DEL function.*/
				FrameBar.StartingSeq.field.StartSeq = Sequence; /* make sure sequence not clear in DEL funciton.*/
				FrameBar.BarControl.TID = TID; /* make sure sequence not clear in DEL funciton.*/
				MakeOutgoingFrame(pOutBuffer,		&FrameLen,
								  sizeof(FRAME_BAR),	&FrameBar,
								  END_OF_ARGS);
				MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[TID]), pOutBuffer, FrameLen);
				MlmeFreeMemory(pOutBuffer);
			}
		}
	}
}
#endif /* DOT11_N_SUPPORT */


VOID ActHeaderInit(
	IN RTMP_ADAPTER *pAd,
	IN OUT PHEADER_802_11 pHdr80211,
	IN UCHAR *da,
	IN UCHAR *sa,
	IN UCHAR *bssid)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	pHdr80211->FC.Type = FC_TYPE_MGMT;
	pHdr80211->FC.SubType = SUBTYPE_ACTION;
	COPY_MAC_ADDR(pHdr80211->Addr1, da);
	COPY_MAC_ADDR(pHdr80211->Addr2, sa);
	COPY_MAC_ADDR(pHdr80211->Addr3, bssid);
}


VOID BarHeaderInit(
	IN RTMP_ADAPTER *pAd,
	IN OUT PFRAME_BAR pCntlBar,
	IN PUCHAR pDA,
	IN PUCHAR pSA)
{
	/*	USHORT	Duration;*/
	NdisZeroMemory(pCntlBar, sizeof(FRAME_BAR));
	pCntlBar->FC.Type = FC_TYPE_CNTL;
	pCntlBar->FC.SubType = SUBTYPE_BLOCK_ACK_REQ;
	pCntlBar->BarControl.MTID = 0;
	pCntlBar->BarControl.Compressed = 1;
	pCntlBar->BarControl.ACKPolicy = 0;
	pCntlBar->Duration = 16 + RTMPCalcDuration(pAd, RATE_1, sizeof(FRAME_BAR));
	COPY_MAC_ADDR(pCntlBar->Addr1, pDA);
	COPY_MAC_ADDR(pCntlBar->Addr2, pSA);
}


/*
	==========================================================================
	Description:
		Insert Category and action code into the action frame.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. category code of the frame.
		4. action code of the frame.

	Return	: None.
	==========================================================================
 */
VOID InsertActField(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Category,
	IN UINT8 ActCode)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
					  1,				&Category,
					  1,				&ActCode,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}

/* Description : Send SMPS Action frame If SMPS mode switches. */
VOID SendSMPSAction(RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR smps)
{
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	FRAME_SMPS_ACTION Frame;
	ULONG FrameLen;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	 /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "BA - MlmeADDBAAction() allocate memory failed\n");
		return;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid)) {
		MlmeFreeMemory(pOutBuffer);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "BA - Invalid WCID(%d)\n",  Wcid);
		return;
	}

	pEntry = &pAd->MacTab.Content[Wcid];
	wdev = pEntry->wdev;

	if (!wdev) {
		MlmeFreeMemory(pOutBuffer);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "BA - wdev is null\n");
		return;
	}

		ActHeaderInit(pAd, &Frame.Hdr, pEntry->Addr, wdev->if_addr, wdev->bssid);

	Frame.Category = CATEGORY_HT;
	Frame.Action = SMPS_ACTION;

	switch (smps) {
	case MMPS_DISABLE:
		Frame.smps = 0;
		break;

	case MMPS_DYNAMIC:
		Frame.smps = 3;
		break;

	case MMPS_STATIC:
		Frame.smps = 1;
		break;
	}

	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(FRAME_SMPS_ACTION), &Frame,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "HT - ( %d )\n", Frame.smps);
}


#define RADIO_MEASUREMENT_REQUEST_ACTION	0

typedef struct GNU_PACKED _BEACON_REQUEST {
	UCHAR	RegulatoryClass;
	UCHAR	ChannelNumber;
	USHORT	RandomInterval;
	USHORT	MeasurementDuration;
	UCHAR	MeasurementMode;
	UCHAR   BSSID[MAC_ADDR_LEN];
	UCHAR	ReportingCondition;
	UCHAR	Threshold;
	UCHAR   SSIDIE[2];			/* 2 byte*/
} BEACON_REQUEST;


#ifdef CONFIG_AP_SUPPORT
VOID SendBeaconRequest(RTMP_ADAPTER *pAd, UINT16 Wcid)
{
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	FRAME_RM_REQ_ACTION Frame;
	ULONG FrameLen;
	BEACON_REQUEST BeaconReq;
	MEASUREMENT_REQ MeasureReg;
	UCHAR apidx;

	if (IS_ENTRY_PEER_AP(&pAd->MacTab.Content[Wcid]) || IS_ENTRY_REPEATER(&pAd->MacTab.Content[Wcid]))
		return;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	 /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "Radio - SendBeaconRequest() allocate memory failed\n");
		return;
	}

	apidx = pAd->MacTab.Content[Wcid].apidx;
	ActHeaderInit(pAd, &Frame.Hdr, pAd->MacTab.Content[Wcid].Addr, pAd->ApCfg.MBSSID[apidx].wdev.if_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
	Frame.Category = CATEGORY_RM;
	Frame.Action = RADIO_MEASUREMENT_REQUEST_ACTION;
	Frame.Token = 1;
	Frame.Repetition = 0;	/* executed once*/
	BeaconReq.RegulatoryClass = 32;		/* ?????*/
	BeaconReq.ChannelNumber = 255;		/* all channels*/
	BeaconReq.RandomInterval = 0;
	BeaconReq.MeasurementDuration = 10;	/* 10 TU*/
	BeaconReq.MeasurementMode = 1;		/* Active mode */
	COPY_MAC_ADDR(BeaconReq.BSSID,	BROADCAST_ADDR);
	BeaconReq.ReportingCondition = 254;	/* report not necesssary*/
	BeaconReq.Threshold = 0;			/* minimum RCPI*/
	BeaconReq.SSIDIE[0] = 0;
	BeaconReq.SSIDIE[1] = 0;			/* wildcard SSID zero length */
	MeasureReg.ID = IE_MEASUREMENT_REQUEST;
	MeasureReg.Token = 0;
	MeasureReg.RequestMode = 0;
	MeasureReg.Type = 5;				/* Beacon Request*/
	MeasureReg.Length = sizeof(MEASUREMENT_REQ) + sizeof(BEACON_REQUEST) - 2;
	MakeOutgoingFrame(pOutBuffer,               &FrameLen,
					  sizeof(FRAME_RM_REQ_ACTION),      &Frame,
					  sizeof(MEASUREMENT_REQ),			&MeasureReg,
					  sizeof(BEACON_REQUEST),			&BeaconReq,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "Radio - SendBeaconRequest\n");
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef DOT11_HE_AX
void sta_send_event_report(struct _RTMP_ADAPTER *pAd, struct _MLME_QUEUE_ELEM *Elem)
{
#ifdef CONFIG_DOT11V_WNM
	struct _MLME_WNM_EVT_REPORT_STRUCT *report;
	struct _FRAME_WNM_EVT_REPORT frame;
	struct wifi_dev *wdev;
	UCHAR *buffer = NULL;
	NDIS_STATUS status;
	ULONG frame_len;

	report = (struct _MLME_WNM_EVT_REPORT_STRUCT *)Elem->Msg;
	wdev = report->wdev;

	status = MlmeAllocateMemory(pAd, &buffer);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR, "WNM event report allocate memory failed\n");
		return;
	}

	os_zero_mem(&frame, sizeof(frame));
	ActHeaderInit(pAd, &frame.Hdr, wdev->bssid, wdev->if_addr, wdev->bssid);
	frame.Category = CATEGORY_WNM;
	frame.Action = EVENT_REPORT;
	frame.Token = report->diag_token;
	frame.Element_ID = report->element_id;
	frame.Length = sizeof(struct _FRAME_WNM_EVT_REPORT) - sizeof(HEADER_802_11) - 5;
	frame.EvtToken = report->token;
	frame.EvtType = report->type;
	frame.EvtStatus = report->status;
	os_move_mem(&frame.EvtReport, report->report, 8);

	MakeOutgoingFrame(buffer, &frame_len,
			sizeof(struct _FRAME_WNM_EVT_REPORT), &frame,
			END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, buffer, frame_len);
	MlmeFreeMemory(buffer);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_INFO, "WNM event report\n");
#endif
}
#endif

