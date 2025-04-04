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
	oce.c

	Abstract:
	OCE (AP) implementation.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------

*/

#ifdef OCE_SUPPORT

#include "rt_config.h"

static VOID OceResetScanIndication(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	INT OceNonOcePresentOldValue, OceBOnlyPresentOldValue;
	P_OCE_CTRL pOceCtrl = NULL;
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	struct _RTMP_ADAPTER *ad = (RTMP_ADAPTER *)wdev->sys_handle;

	pOceCtrl = &wdev->OceCtrl;
	OceNonOcePresentOldValue = OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
		OCE_NONOCE_PRESENT_MASK, OCE_NONOCE_PRESENT_OFFSET);
	OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
		0, OCE_NONOCE_PRESENT_MASK, OCE_NONOCE_PRESENT_OFFSET);
	OceBOnlyPresentOldValue = OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
		OCE_11B_ONLY_PRESENT_MASK, OCE_11B_ONLY_PRESENT_OFFSET);
	OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
		0, OCE_11B_ONLY_PRESENT_MASK, OCE_11B_ONLY_PRESENT_OFFSET);

	if ((OceNonOcePresentOldValue != OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
		OCE_NONOCE_PRESENT_MASK, OCE_NONOCE_PRESENT_OFFSET)) ||
		(OceBOnlyPresentOldValue != OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
		OCE_11B_ONLY_PRESENT_MASK, OCE_11B_ONLY_PRESENT_OFFSET)))
		OceSendFilsDiscoveryAction(ad, wdev);
	pOceCtrl->Scan11bOceAPTimerRunning = FALSE;
}

DECLARE_TIMER_FUNCTION(OceResetScanIndication);
BUILD_TIMER_FUNCTION(OceResetScanIndication);

static VOID OceSetMaxChannelTimesUp(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	P_OCE_CTRL pOceCtrl = NULL;
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	BOOLEAN Cancelled;

	pOceCtrl = &wdev->OceCtrl;

	pOceCtrl->MaxChannelTimesUp = TRUE;
	RTMPCancelTimer(&pOceCtrl->MaxChannelTimer, &Cancelled);
	pOceCtrl->MaxChannelTimerRunning = FALSE;
}

DECLARE_TIMER_FUNCTION(OceSetMaxChannelTimesUp);
BUILD_TIMER_FUNCTION(OceSetMaxChannelTimesUp);

static VOID OceFdFrameSending(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	BOOLEAN TimerCancelled = FALSE;
	UCHAR FdFrameTimes = 0;

	pAd->ApCfg.FdFrameTimerRunning = FALSE;
	pAd->ApCfg.FdFrameCurNum++;

	FdFrameTimes = (pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)] / OCE_FD_FRAME_PERIOD) - 1;

	if (pAd->ApCfg.FdFrameCurNum >= FdFrameTimes) {
		RTMPCancelTimer(&pAd->ApCfg.FdFrameTimer, &TimerCancelled);
	} else {
		UCHAR intval = OCE_FD_FRAME_PERIOD + 2;

		if (pAd->ApCfg.FdFrameCurNum == (FdFrameTimes-1))
			intval = OCE_FD_FRAME_PERIOD + 3;
		RTMPSetTimer(&pAd->ApCfg.FdFrameTimer, intval);
		pAd->ApCfg.FdFrameTimerRunning = TRUE;
	}

	OceSendFilsDiscoveryAction(pAd, wdev);
}

DECLARE_TIMER_FUNCTION(OceFdFrameSending);
BUILD_TIMER_FUNCTION(OceFdFrameSending);

static VOID OceAPAutoScanNR(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	NDIS_802_11_SSID Ssid;

	if (scan_in_run_state(pAd, wdev) == FALSE) {
		NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
		NdisMoveMemory(Ssid.Ssid, "1", strlen("1"));
		Ssid.SsidLength = strlen("1");
		ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, wdev);
	}
}

DECLARE_TIMER_FUNCTION(OceAPAutoScanNR);
BUILD_TIMER_FUNCTION(OceAPAutoScanNR);

static VOID WextOceSendStaInfoToDaemonEvent(
	PNET_DEV pNetDev,
	struct oce_info *pStaInfo,
	OCE_MSG_TYPE MsgType,
	UINT16 ReportBufLen)
{
	P_OCE_MSG pOceMsg;
	UINT16 buflen = 0;
	char *buf;

	buflen = sizeof(OCE_MSG);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	pOceMsg = (P_OCE_MSG)buf;
	pOceMsg->ifindex = RtmpOsGetNetIfIndex(pNetDev);
	pOceMsg->OceMsgLen = ReportBufLen;
	pOceMsg->OceMsgType = MsgType;

	NdisCopyMemory(&pOceMsg->OceMsgBody.OceEvtStaInfo, pStaInfo, ReportBufLen);

	if (MsgType == OCE_MSG_INFO_UPDATE)
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"indicate Oce STA INFO %d\n",
			pOceMsg->OceMsgBody.OceEvtStaInfo.OceCapIndication);


	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"sizeof %u report_buf_len %d buflen %u msg_type %s\n",
		(UINT32)sizeof(struct oce_info), ReportBufLen, buflen, OceMsgTypeToString(MsgType));
	RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM,
		OID_802_11_OCE_MSG, NULL, (PUCHAR)buf, buflen);

	os_free_mem(buf);
}

OCE_ERR_CODE OceInit(
	PRTMP_ADAPTER pAd)
{
	INT loop = 0;
	P_OCE_CTRL pOceCtrl = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;
	struct wifi_dev *pWdev = NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_AP_SUPPORT
	for (loop = 0; loop < pAd->ApCfg.BssidNum; loop++) {
		pWdev = &pAd->ApCfg.MBSSID[loop].wdev;
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
		NdisZeroMemory(pOceCtrl, sizeof(OCE_CTRL));
		OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
			OCE_RELEASE, OCE_RELEASE_MASK, OCE_RELEASE_OFFSET) /* OCE release = 1 */;
		OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
			IS_STA_CFON, OCE_IS_STA_CFON_MASK, OCE_IS_STA_CFON_OFFSET); /* IS not STA CFON */

		if (pOceCtrl->bOceFilsHlpEnable == TRUE)
			OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
				HLP_ENABLED, OCE_HLP_ENABLED_MASK,
				OCE_HLP_ENABLED_OFFSET); /* HLP_ENABLED */

		pOceCtrl->bApReducedWanEnable = FALSE;
		pOceCtrl->bApRnrCompleteEnable = FALSE;
		pOceCtrl->bApEspEnable = FALSE;
		pOceCtrl->ShortSSIDEnabled = TRUE;
		pOceCtrl->AvailableCap = 11;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	for (loop = 0; loop < MAX_MULTI_STA; loop++) {
		pOceCtrl = &pAd->StaCfg[loop].wdev.OceCtrl;
		NdisZeroMemory(pOceCtrl, sizeof(OCE_CTRL));
	}
#endif /* CONFIG_STA_SUPPORT */
	return OCE_SUCCESS;
}

OCE_ERR_CODE OceTimerInit(
	PRTMP_ADAPTER pAd)
{
	INT loop = 0;
	P_OCE_CTRL pOceCtrl = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;
	struct wifi_dev *pWdev = NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_AP_SUPPORT
	for (loop = 0; loop < pAd->ApCfg.BssidNum; loop++) {
		pWdev = &pAd->ApCfg.MBSSID[loop].wdev;
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;

		RTMPInitTimer(pAd, &pOceCtrl->Scan11bOceAPTimer,
			GET_TIMER_FUNCTION(OceResetScanIndication), pWdev, FALSE);
		pOceCtrl->Scan11bOceAPTimerRunning = FALSE;
		RTMPInitTimer(pAd, &pOceCtrl->MaxChannelTimer,
			GET_TIMER_FUNCTION(OceSetMaxChannelTimesUp), pWdev, FALSE);
		pOceCtrl->MaxChannelTimerRunning = FALSE;
	}

	if (IS_FD_FRAME_FW_MODE(cap)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s - FdFrame using FW Mode\n", __func__);
	} else {
		/* Need TBTT source in host */
		RTMPInitTimer(pAd, &pAd->ApCfg.FdFrameTimer, GET_TIMER_FUNCTION(OceFdFrameSending), pAd, FALSE);
		pAd->ApCfg.FdFrameTimerRunning = FALSE;
	}

	RTMPInitTimer(pAd, &pAd->ApCfg.APAutoScanNeighborTimer, GET_TIMER_FUNCTION(OceAPAutoScanNR), pAd, TRUE);
#endif /* CONFIG_AP_SUPPORT */

	return OCE_SUCCESS;
}

OCE_ERR_CODE OceDeInit(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct _RTMP_CHIP_CAP *cap = NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (IS_FD_FRAME_FW_MODE(cap)) {
		HW_SET_FD_FRAME_OFFLOAD(pAd, wdev->wdev_idx, 0,
			FALSE, 0, NULL);
	} else {
		BOOLEAN Cancelled;

		RTMPCancelTimer(&pAd->ApCfg.FdFrameTimer, &Cancelled);
	}
	return OCE_SUCCESS;
}

OCE_ERR_CODE OceRelease(
	PRTMP_ADAPTER pAd)
{
	INT loop;
	BOOLEAN Cancelled;
	P_OCE_CTRL pOceCtrl = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	if (!IS_FD_FRAME_FW_MODE(cap)) {
		RTMPCancelTimer(&pAd->ApCfg.FdFrameTimer, &Cancelled);
		RTMPReleaseTimer(&pAd->ApCfg.FdFrameTimer, &Cancelled);
	}

	RTMPReleaseTimer(&pAd->ApCfg.APAutoScanNeighborTimer, &Cancelled);

	for (loop = 0; loop < pAd->ApCfg.BssidNum; loop++) {
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
		RTMPCancelTimer(&pOceCtrl->Scan11bOceAPTimer, &Cancelled);
		RTMPReleaseTimer(&pOceCtrl->Scan11bOceAPTimer, &Cancelled);
		RTMPCancelTimer(&pOceCtrl->MaxChannelTimer, &Cancelled);
		RTMPReleaseTimer(&pOceCtrl->MaxChannelTimer, &Cancelled);
	}
	return OCE_SUCCESS;
}

OCE_ERR_CODE OceInsertAttrById(
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry,
	PUINT8 pAttrTotalLen,
	PUINT8 pAttrBuf,
	UINT8 AttrId
	)
{
	P_OCE_CTRL	pOceCtrl = NULL;
	OCE_ATTR_STRUCT OceAttr;
	PUINT8 pAttrBufOffset = (pAttrBuf + *pAttrTotalLen);
	UINT16 OverflowChk = 0;
	BSS_STRUCT *pMbss = NULL;
	UINT16 i;
	ULONG ReadOffset = 0;

	if (wdev) {
		pOceCtrl = &wdev->OceCtrl;
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Mbss is NULL !!!!!\n");
		return OCE_INVALID_ARG;
	}

	if (!VALID_OCE_ATTR_ID(AttrId)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Invalid attr Id [%d] !!!!!\n", AttrId);
		return OCE_INVALID_ARG;
	}

	NdisZeroMemory(&OceAttr, sizeof(OCE_ATTR_STRUCT));

	switch (AttrId) {
	case OCE_ATTR_CAP_INDCATION:
		OceAttr.AttrID = OCE_ATTR_CAP_INDCATION;
		OceAttr.AttrLen = 0x01;
		OceAttr.AttrBody[0] = pOceCtrl->OceCapIndication;
		break;
#ifdef CONFIG_AP_SUPPORT
	case OCE_ATTR_AP_RSSI_REJCTION:
		OceAttr.AttrID = OCE_ATTR_AP_RSSI_REJCTION;
		OceAttr.AttrLen = 0x02;
		OceAttr.AttrBody[0] = pEntry->oceInfo.DeltaAssocRSSI;
		OceAttr.AttrBody[1] = pOceCtrl->AssocRetryDelay;
		break;
	case OCE_ATTR_AP_REDUCED_WAN:
		OceAttr.AttrID = OCE_ATTR_AP_REDUCED_WAN;
		OceAttr.AttrLen = 0x01;
		OceAttr.AttrBody[0] = pOceCtrl->AvailableCap;
		break;
	case OCE_ATTR_AP_RNR_COMPLETE:
		OceAttr.AttrID = OCE_ATTR_AP_RNR_COMPLETE;

		if (wdev->wdev_type == WDEV_TYPE_AP)
			pMbss = wdev->func_dev;

		/*Short SSID List*/
		if (pMbss && pMbss->ReducedNRListExist && (pMbss->ReducedNRListInfo.ValueLen)) {
			OceAttr.AttrLen = pMbss->ReducedNRListInfo.ValueLen/OCE_RNR_IE_LEN;

			for (i = 0; i < OceAttr.AttrLen; i++) {
				/* 13 = OCE_RNR_IE_LEN - SHORT_SSID_LEN*/
				ReadOffset += 13;
				NdisMoveMemory(&OceAttr.AttrBody[i * SHORT_SSID_LEN],
						pMbss->ReducedNRListInfo.Value + ReadOffset, SHORT_SSID_LEN);
				ReadOffset += SHORT_SSID_LEN;
			}
			OceAttr.AttrLen *= SHORT_SSID_LEN;
		} else
			return OCE_SUCCESS;

		break;
#endif

#ifdef CONFIG_STA_SUPPORT
	case OCE_ATTR_STA_PRB_SUP_BSSID:
		OceAttr.AttrID = OCE_ATTR_STA_PRB_SUP_BSSID;
		OceAttr.AttrLen = 0x01;
		/*BSSIDs List*/
		break;
	case OCE_ATTR_STA_PRB_SUP_SSID:
		OceAttr.AttrID = OCE_ATTR_STA_PRB_SUP_SSID;
		OceAttr.AttrLen = 0x01;
		/*Short SSID List*/
		break;
#endif
	default:
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"UNKNOWN Oce AttrId [%d] !!!!!\n", AttrId);
	}
	OverflowChk = *pAttrTotalLen + OceAttr.AttrLen + 2;
	if (OverflowChk >= OCE_ATTR_MAX_LEN) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"AttrTotalLen %d Overflow, should be below %d !!!!!\n",
		OverflowChk, OCE_ATTR_MAX_LEN);
		return OCE_UNEXP;
	}

	/* safe, insert the attribute */
	NdisCopyMemory(pAttrBufOffset, &OceAttr, OceAttr.AttrLen+2);
	*pAttrTotalLen += (OceAttr.AttrLen + 2);
	return OCE_SUCCESS;
}

OCE_ERR_CODE OceCollectAttribute(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *pEntry,
	PUINT8 pAttrLen,
	PUINT8 pAttrBuf,
	UINT8 FrameType
	)
{
	UINT8 ErrCode = OCE_SUCCESS;
	P_OCE_CTRL	pOceCtrl = NULL;

	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"%s - collect oce attr for FrameType %d\n", __func__, FrameType);

	if (!pAd || !wdev || !pAttrLen || !pAttrBuf) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Invalid input argument !!!!!\n");
		return OCE_INVALID_ARG;
	}

	pOceCtrl = &wdev->OceCtrl;

	switch (FrameType) {
#ifdef CONFIG_AP_SUPPORT
	case OCE_FRAME_TYPE_BEACON:
	case OCE_FRAME_TYPE_PROBE_RSP:
		ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen, pAttrBuf, OCE_ATTR_CAP_INDCATION);
		if (pOceCtrl->bApReducedWanEnable)
		ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen, pAttrBuf, OCE_ATTR_AP_REDUCED_WAN);
		if (IS_OCE_RNR_ENABLE(wdev))
		ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen, pAttrBuf, OCE_ATTR_AP_RNR_COMPLETE);
		break;
	case OCE_FRAME_TYPE_ASSOC_RSP:
		ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen, pAttrBuf, OCE_ATTR_CAP_INDCATION);

		if (pEntry->oceInfo.DeltaAssocRSSI)
			ErrCode = OceInsertAttrById(wdev, pEntry, pAttrLen,
				pAttrBuf, OCE_ATTR_AP_RSSI_REJCTION);
		break;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	case OCE_FRAME_TYPE_PROBE_REQ:
		OceInsertAttrById(wdev, NULL, pAttrLen,
			pAttrBuf, OCE_ATTR_CAP_INDCATION);
		OceInsertAttrById(wdev, NULL, pAttrLen,
			pAttrBuf, OCE_ATTR_STA_PRB_SUP_BSSID);
		ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen, pAttrBuf, OCE_ATTR_STA_PRB_SUP_SSID);
		break;
	case OCE_FRAME_TYPE_ASSOC_REQ:
		ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen, pAttrBuf, OCE_ATTR_CAP_INDCATION);
		break;
#endif /* CONFIG_STA_SUPPORT */

	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"UNKNOWN FrameType %d !!!!!\n", FrameType);
		return OCE_UNEXP;
	}
	return ErrCode;
}

VOID OceParseStaOceIE(
	PRTMP_ADAPTER pAd,
	UCHAR *buf,
	UCHAR len,
	PEER_PROBE_REQ_PARAM * ProbeReqParam
	)
{
	UCHAR *pos = NULL;
	UCHAR ParsedLen = 0;
	UCHAR Bssid_offset = 0;
	UCHAR apidx = MAIN_MBSSID;
	PEID_STRUCT eid_ptr;
	UINT32 crc32; /* 32-bit CRC value */
	NDIS_802_11_MAC_ADDRESS OceSuppresBssid;
	UCHAR OceSuppresSsid[SHORT_SSID_LEN];

	pos = buf;
	/* skip OUI 4 bytes */
	pos += 4;
	ParsedLen += 4;

	eid_ptr = (PEID_STRUCT)pos;

	while (ParsedLen <= len) {
		switch (eid_ptr->Eid) {
		case OCE_ATTR_CAP_INDCATION:
			ProbeReqParam->IsOceCapability = TRUE;
			break;/* is OCE STA*/
		case OCE_ATTR_STA_PRB_SUP_BSSID:
			if (eid_ptr->Len == 0) {
				for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
					ProbeReqParam->bProbeSupp[apidx] = TRUE;
				break;
			}

			for (Bssid_offset = 0; Bssid_offset < eid_ptr->Len; Bssid_offset += 6) {
				NdisMoveMemory(OceSuppresBssid, eid_ptr->Octet + Bssid_offset, 6);
				if (MAC_ADDR_EQUAL(OceSuppresBssid, BROADCAST_ADDR)) {
					for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
						ProbeReqParam->bProbeSupp[apidx] = TRUE;
					break;
				}

				for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
					if (MAC_ADDR_EQUAL(pAd->wdev_list[apidx]->bssid, OceSuppresBssid))
						ProbeReqParam->bProbeSupp[apidx] = TRUE;
				}
			}
			break;
		case OCE_ATTR_STA_PRB_SUP_SSID:
			if (eid_ptr->Len == 0) {
				for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
					ProbeReqParam->bProbeSupp[apidx] = TRUE;
				break;
			}

			for (Bssid_offset = 0; Bssid_offset < eid_ptr->Len; Bssid_offset += 4) {
				for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
					NdisCopyMemory(OceSuppresSsid, eid_ptr->Octet + Bssid_offset, 4);
					crc32 = Crcbitbybitfast(pAd->ApCfg.MBSSID[apidx].Ssid,
						pAd->ApCfg.MBSSID[apidx].SsidLen);
					if (NdisEqualMemory(&crc32, eid_ptr->Octet + Bssid_offset, 4))
						ProbeReqParam->bProbeSupp[apidx] = TRUE;
				}
			}
			break;

		default:
				break;
		}
		ParsedLen += (2 + eid_ptr->Len);
		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}
}

VOID OceParseStaAssoc(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *pEntry,
	UCHAR *buf,
	UCHAR len,
	OCE_FRAME_TYPE OceFrameType)
{
	UCHAR *pos = NULL;
	UCHAR ParsedLen = 0;
	PEID_STRUCT eid_ptr;
	struct oce_info *oceInfo = &pEntry->oceInfo;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pEntry is NULL!!!\n");
		return;
	}

	pos = buf;
	/* skip OUI 4 bytes */
	pos += 4;
	ParsedLen += 4;

	eid_ptr = (PEID_STRUCT)pos;

	while (ParsedLen <= len) {
		switch (eid_ptr->Eid) {
		case OCE_ATTR_CAP_INDCATION:
			COPY_MAC_ADDR(oceInfo->mac_addr, pEntry->Addr);
			oceInfo->OceCapIndication = eid_ptr->Octet[0];
			break;/* is OCE STA*/
		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s - ignored OCE_ATTR [%d]\n", __func__, eid_ptr->Eid);
			break;
		}
		ParsedLen += (2 + eid_ptr->Len);
		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}
}

BOOLEAN OceCheckOceCap(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR *buf,
	UCHAR len
	)
{
	UCHAR *pos = NULL;
	UCHAR ParsedLen = 0;
	PEID_STRUCT eid_ptr;
	P_OCE_CTRL	pOceCtrl = NULL;
	BOOLEAN hasOceCap = FALSE;

	pOceCtrl = &wdev->OceCtrl;
	pos = buf;

	/* skip OUI 4 bytes */
	pos += 4;
	ParsedLen += 4;

	eid_ptr = (PEID_STRUCT)pos;

	while (ParsedLen <= len) {
		switch (eid_ptr->Eid) {
		case OCE_ATTR_CAP_INDCATION:
			hasOceCap = TRUE;
			break;/* is OCE STA*/
		default:
			break;
		}
		ParsedLen += (2 + eid_ptr->Len);
		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}
	return hasOceCap;
}

INT OceApAutoChSelection2G(
	AUTO_CH_CTRL * pAutoChCtrl,
	AUTOCH_SEL_CH_LIST * pACSChList
	)
{
	INT ChIdx, ChListNum = 0;

	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
		if (pACSChList[ChIdx].Channel != 1 && pACSChList[ChIdx].Channel != 6 &&
			pACSChList[ChIdx].Channel != 11)
			continue;
		else {
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Channel =
				pACSChList[ChIdx].Channel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Bw =
				pACSChList[ChIdx].Bw;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].BwCap = TRUE;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].CentralChannel =
				pACSChList[ChIdx].CentralChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].SkipChannel =
				pACSChList[ChIdx].SkipChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Flags =
				pACSChList[ChIdx].Flags;
			ChListNum++;
		}
	}
	if (ChListNum == 0) {
		for (ChIdx = 1; ChIdx <= 11; ChIdx += 5) {
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Channel =
				pACSChList[ChIdx].Channel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Bw = pACSChList[ChIdx].Bw;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].BwCap = pACSChList[ChIdx].BwCap;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].CentralChannel =
				pACSChList[ChIdx].CentralChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].SkipChannel =
				pACSChList[ChIdx].SkipChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Flags = pACSChList[ChIdx].Flags;
		ChListNum++;
		}
	}
	return ChListNum;
}

VOID OceScanOceAPList(BSS_TABLE *Tab)
{
	UINT i;

	MTWF_PRINT("\nOCE AP List:\n");
	for (i = 0; i < Tab->BssNr  && Tab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		if (Tab->BssEntry[i].is_oce_ap)
			MTWF_PRINT(MACSTR", SSID:%s\n",
				MAC2STR(Tab->BssEntry[i].Bssid), Tab->BssEntry[i].Ssid);
	}
	MTWF_PRINT("\n11b-only AP List:\n");
	for (i = 0; i < Tab->BssNr  && Tab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		if (Tab->BssEntry[i].is_11bonly_ap)
			MTWF_PRINT(MACSTR", SSID:%s\n",
				MAC2STR(Tab->BssEntry[i].Bssid), Tab->BssEntry[i].Ssid);
	}
	MTWF_PRINT("\nNon-OCE AP List:\n");
	for (i = 0; i < Tab->BssNr  && Tab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		if (!Tab->BssEntry[i].is_oce_ap)
			MTWF_PRINT(MACSTR", SSID:%s\n",
				MAC2STR(Tab->BssEntry[i].Bssid), Tab->BssEntry[i].Ssid);
	}
}

VOID OceIndicateStaInfo(PRTMP_ADAPTER pAd, struct wifi_dev *pWdev, UCHAR *mac_addr)
{
	struct oce_info oceInfo;

	NdisZeroMemory(&oceInfo, sizeof(struct oce_info));
	COPY_MAC_ADDR(&oceInfo.mac_addr, mac_addr);
	COPY_MAC_ADDR(&oceInfo.bssid, pWdev->bssid);
	OceIndicateStaInfoToDaemon(pAd, &oceInfo, OCE_MSG_INFO_UPDATE);
}

INT OceIndicateStaInfoToDaemon(
	PRTMP_ADAPTER	pAd,
	struct oce_info *poceInfo,
	OCE_MSG_TYPE MsgType)
{
	/* mac table lookup & update sta's oce info here */
	PMAC_TABLE_ENTRY pEntry = MacTableLookup(pAd, poceInfo->mac_addr);

	if (pEntry != NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("\033[1;33m %s, %u pEntry->wcid %d \033[0m\n"
			, __func__, __LINE__, pEntry->wcid));
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"[ERROR] can't find sta entry!!\n");
		return FALSE;
	}

	/* send wext event */
	WextOceSendStaInfoToDaemonEvent(pAd->net_dev, poceInfo, MsgType, sizeof(struct oce_info));

	return TRUE;
}

void Oce_read_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer)
{
	struct wifi_dev *wdev = NULL;
	OCE_CTRL *oceCtrl = NULL;
	INT Loop = 0;
	INT n = 0;

	if (RTMPGetKeyParameter("OCE_SUPPORT", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, Loop)].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0)
				oceCtrl->bOceEnable = TRUE;
			else
				oceCtrl->bOceEnable = FALSE;

			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"I/F(ra%d) OCE_SUPPORT=%d\n", Loop, oceCtrl->bOceEnable);
		}
	}

	if (RTMPGetKeyParameter("OCE_FD_FRAME", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, Loop)].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop > MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0)
				oceCtrl->bFdFrameEnable = TRUE;
			else
				oceCtrl->bFdFrameEnable = FALSE;

			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"I/F(ra%d) OCE_FD_FRAME=%d\n", Loop, oceCtrl->bFdFrameEnable);
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_HLP", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, Loop)].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop > MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0) {
				oceCtrl->bOceFilsHlpEnable = TRUE;
				OCE_SET_CONTROL_FIELD(oceCtrl->OceCapIndication,
					HLP_ENABLED, OCE_HLP_ENABLED_MASK,
					OCE_HLP_ENABLED_OFFSET); /* HLP_ENABLED */
			} else
				oceCtrl->bOceFilsHlpEnable = FALSE;

			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) OCE_FILS_HLP=%d\n",
					 Loop, oceCtrl->bOceEnable);
		}
	}


	if (RTMPGetKeyParameter("OCE_FILS_DhcpServer", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), Loop++) {
			UINT32 ip_addr;

			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, Loop)].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop > MAX_MBSSID_NUM(pAd))
				break;

			if (rtinet_aton(macptr, &ip_addr)) {
				oceCtrl->FilsDhcpServerIp = ip_addr;
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"OCE_FILS_DhcpServer=%s(%x)\n", macptr, oceCtrl->FilsDhcpServerIp);
			}
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_DhcpServerPort", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, Loop)].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop > MAX_MBSSID_NUM(pAd))
				break;

			oceCtrl->FilsDhcpServerPort = os_str_tol(macptr, 0, 10);

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "I/F(ra%d) OCE_FILS_DhcpServerPort=%d\n",
					 Loop, oceCtrl->FilsDhcpServerPort);
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_REALMS", tmpbuf, 256, pBuffer, TRUE)) {
		RTMP_STRING *macptr;
		UCHAR result[SHA256_BLOCK_SIZE] = {0}, i = 0;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, Loop)].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop > MAX_MBSSID_NUM(pAd))
				break;

			for (i = 0 ; i < strlen(macptr); i++) {
				macptr[i] = tolower(macptr[i]);
			}

			RT_SHA256(macptr, strlen(macptr), result);
			NdisMoveMemory(&oceCtrl->FilsRealmsHash, result, FILS_REALMS_HASH_LEN);

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				" OCE_FILS_Realms%d (%02x:%02x)\n", (Loop + 1),
				result[0], result[1]);

		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_CACHE", tmpbuf, 256, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, Loop)].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop > MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0)
				oceCtrl->FilsCacheId = RandomByte(pAd) << 8 | RandomByte(pAd);
			else
				oceCtrl->FilsCacheId = 0;


			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				" OCE_FILS_CACHE%d (%04x)\n", (Loop + 1),
				oceCtrl->FilsCacheId);

		}
	}

	if (RTMPGetKeyParameter("OCE_RNR_SUPPORT", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop > MAX_MBSSID_NUM(pAd))
				break;

			oceCtrl->bApRnrCompleteEnable = (UINT8)os_str_tol(macptr, 0, 10) ? TRUE : FALSE;

			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(bApRnrCompleteEnable[%d]=%d)\n", Loop,
				oceCtrl->bApRnrCompleteEnable);
		}
	}

	for (Loop = 0; Loop < MAX_MBSSID_NUM(pAd); Loop++) {
		RTMP_STRING tok_str[30];
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, Loop)].wdev;
		OCE_CTRL *oceCtrl = &wdev->OceCtrl;

		/*
			OCE_ASSOC_RssiThres:
		*/
		NdisZeroMemory(tok_str, sizeof(tok_str));
		n = snprintf(tok_str, sizeof(tok_str), "OCE_ASSOC_RssiThres%d", Loop + 1);
		if (n < 0 || n >= sizeof(tok_str)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_ERROR,
				 "%s:%d snprintf Error\n", __func__, __LINE__);
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 32, pBuffer, TRUE)) {
			INT8 rssi = os_str_tol(tmpbuf, 0, 10);

			oceCtrl->AssocRSSIThres = rssi;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				"%s: OCE_ASSOC_RssiThres%d (%d)\n", __func__, (Loop + 1),
				oceCtrl->AssocRSSIThres);
		}

		/*
			OCE_ASSOC_RetryDelay:
		*/
		NdisZeroMemory(tok_str, sizeof(tok_str));
		n = snprintf(tok_str, sizeof(tok_str), "OCE_ASSOC_RetryDelay%d", Loop + 1);
		if (n < 0 || n >= sizeof(tok_str)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_ERROR,
				 "%s:%d snprintf Error\n", __func__, __LINE__);
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 32, pBuffer, TRUE)) {
			UINT8 time = os_str_tol(tmpbuf, 0, 10);

			oceCtrl->AssocRetryDelay = time;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_INFO,
				"%s: OCE_ASSOC_RetryDelay%d (%d)\n", __func__, (Loop + 1),
				oceCtrl->AssocRSSIThres);
		}
	}
}

/* build reduced neighbor report element */
INT build_rnr_element(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf, UINT8 pos, UCHAR subtype)
{
	BSS_STRUCT *pMbss = NULL;
	ULONG ReducedNRListTmpLen = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	ULONG RemainBeaconLen = 0;
	UCHAR apidx = 0;

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = wdev->func_dev;
		apidx = wdev->func_idx;
	}

	if (subtype == SUBTYPE_BEACON) {
#ifdef BCN_V2_SUPPORT
		if (apidx < cap->max_v2_bcn_num)
			RemainBeaconLen = 1520 - cap->tx_hw_hdr_len - pos;/*FW limitation*/
		else
			RemainBeaconLen = 512 - cap->tx_hw_hdr_len - pos;
#else
		RemainBeaconLen = 512 - cap->tx_hw_hdr_len - pos;
#endif
		if (pMbss && pMbss->ReducedNRListExist && (pMbss->ReducedNRListInfo.ValueLen)) {
			if (pMbss->ReducedNRListInfo.ValueLen > RemainBeaconLen)
				pMbss->ReducedNRListInfo.ValueLen = RemainBeaconLen;
		}
	}

	if (pMbss && pMbss->ReducedNRListExist && (pMbss->ReducedNRListInfo.ValueLen))
		MakeOutgoingFrame(buf, &ReducedNRListTmpLen,
						pMbss->ReducedNRListInfo.ValueLen, pMbss->ReducedNRListInfo.Value,
						END_OF_ARGS);

	return ReducedNRListTmpLen;
}
/* build estimated service parameters element */
INT build_esp_element(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	ULONG TmpLen;
	UCHAR operating_ie = IE_WLAN_EXTENSION, ext_ie = IE_EXTENSION_ID_ESP, operating_len = 4;
	UINT i, CurrTxop, ScanTxop = 0;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	P_OCE_CTRL pOceCtrl = NULL;
	ESP_INFO EspInfo;
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	pOceCtrl = &wdev->OceCtrl;

	if (!pOceCtrl->bApEspEnable)
		return 0;

	EspInfo.word = 0;
	EspInfo.field.ACI = 1;
	EspInfo.field.DataFormat = 0;
	EspInfo.field.BAWinSize = 0;
	EspInfo.field.DataPPDUDurationTarget = 10;

	/* if channel is not that busy, can share all the remaining resource to new joining STA */
	/* 10% deviation of Channel Utilization is acceptable, 255 * 0.1 = 26, 229 = 255 - 26 */
	if (pQloadCtrl->QloadChanUtilCcaNavTx < 229)
		EspInfo.field.EstimatedAirTimeFraction = 255 - pQloadCtrl->QloadChanUtilCcaNavTx;
	else if ((pQloadCtrl->QloadChanUtilCcaNavTx > 229) && (pQloadCtrl->QloadChanUtil < 26))
		EspInfo.field.EstimatedAirTimeFraction = pQloadCtrl->QloadChanUtilCcaNavTx / 2;
	else {
		CurrTxop = pAd->CurrEdcaParam[WMM_PARAM_AC_1].u2Txop;
		CurrTxop = (!CurrTxop) ? 2000 : CurrTxop * 32; /* 32 us per txop unit */
		for (i = 0; i < ScanTab->BssNr && ScanTab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++)
			ScanTxop += ScanTab->BssEntry[i].EdcaParm.Txop[QID_AC_BK];

		ScanTxop *= 32; /* 32 us per txop unit */
		/* corresponding access category in units of 50 us */
		EspInfo.field.EstimatedAirTimeFraction = (255 - ((pQloadCtrl->QloadChanUtilCcaNavTx - pQloadCtrl->QloadChanUtil) * ((CurrTxop + ScanTxop) / CurrTxop))) *
							((EspInfo.field.DataPPDUDurationTarget * 50) / ((EspInfo.field.DataPPDUDurationTarget * 50) + ScanTxop));
	}
	MakeOutgoingFrame(buf, &TmpLen,
			1,    &operating_ie,
			1,    &operating_len,
			1,    &ext_ie,
			3,    &EspInfo,
			END_OF_ARGS);

		return TmpLen;

}
INT	Set_OceRssiThreshold_Proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	CHAR rssi;
	OCE_CTRL *oceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	oceCtrl = &wdev->OceCtrl;

	rssi = (CHAR)os_str_tol(arg, 0, 10);
	pAd->ApCfg.MBSSID[apidx].AssocReqRssiThreshold = rssi;

	MTWF_PRINT("%s::(RssiThreshold=%d)\n",
			 __func__, pAd->ApCfg.MBSSID[apidx].AssocReqRssiThreshold);

	return TRUE;
}

INT	Set_OceAssocRetryDelay_Proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	UINT time;
	OCE_CTRL *oceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	oceCtrl = &wdev->OceCtrl;

	time = (CHAR)os_str_tol(arg, 0, 10);
	oceCtrl->AssocRetryDelay = time;

	MTWF_PRINT("%s::(AssocRetryDelay=%d)\n",
			 __func__, oceCtrl->AssocRetryDelay);

	return TRUE;
}

INT	Set_OceFdFrameCtrl_Proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	OCE_CTRL *oceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct _RTMP_CHIP_CAP *cap = NULL;
	UINT enable = 0;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	oceCtrl = &wdev->OceCtrl;
	enable = (CHAR)os_str_tol(arg, 0, 10);

	if (IS_FD_FRAME_FW_MODE(cap)) {
		if (enable) {
			oceCtrl->bFdFrameEnable = TRUE;
			OceSendFilsDiscoveryAction(pAd, wdev);
		} else {
			oceCtrl->bFdFrameEnable = FALSE;
			HW_SET_FD_FRAME_OFFLOAD(pAd, wdev->wdev_idx, 0,
				FALSE, 0, NULL);
		}
	}

	MTWF_PRINT("%s::(FdFrameCtrl=%d)\n",
			 __func__, enable);

	return TRUE;
}

/*	format : iwpriv [interface] set oce_reduced_nr=[append]-[nr_entry_num]
*	sample : iwpriv ra0 set oce_reduced_nr=0-7
*			==> renew list,not append,indicate 7 entries
*/

INT Set_OceDownlinkAvailCap_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg)
{
	INT loop;
	P_OCE_CTRL pOceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
		if (pOceCtrl && pOceCtrl->bOceEnable)
			OCE_SET_DOWNLINK_AVAILABLE_CAP(pOceCtrl->AvailableCap, simple_strtol(arg, 0, 10),
							OCE_AVAILABLE_CAP_MASK, OCE_AVAILABLE_CAP_OFFSET);
	}
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

	return TRUE;
}

INT Set_OceUplinkAvailCap_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg)
{
	INT loop;
	P_OCE_CTRL pOceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
		if (pOceCtrl && pOceCtrl->bOceEnable)
			OCE_SET_UPLINK_AVAILABLE_CAP(pOceCtrl->AvailableCap, simple_strtol(arg, 0, 10),
							OCE_AVAILABLE_CAP_MASK, OCE_AVAILABLE_CAP_OFFSET);
	}
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

	return TRUE;
}


INT Set_OceEspEnable_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	P_OCE_CTRL pOceCtrl = NULL;
	BOOLEAN enable;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pOceCtrl = &wdev->OceCtrl;

	enable = (BOOLEAN) simple_strtol(arg, 0, 10);

	pOceCtrl->bApEspEnable = enable;

	return enable;
}

INT32 Show_OceStat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 loop = 0;
	P_OCE_CTRL pOceCtrl = NULL;
	BSS_TABLE *ScanTab = NULL;
#ifdef CONFIG_AP_SUPPORT
	MTWF_PRINT("Total BssidNum \t\t %d\n",
		pAd->ApCfg.BssidNum);

	for (loop = 0; loop < pAd->ApCfg.BssidNum; loop++) {
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;

		MTWF_PRINT("========= apidx %d ===========\n", loop);

		MTWF_PRINT("bOceEnable \t\t %d\n",
			pOceCtrl->bOceEnable);
		MTWF_PRINT("bFdFrameEnable \t\t %d\n",
			pOceCtrl->bFdFrameEnable);
		MTWF_PRINT("bApEspEnable \t\t %d\n",
			pOceCtrl->bApEspEnable);
		MTWF_PRINT("bOceFilsHlpEnable \t\t %d\n",
			pOceCtrl->bOceFilsHlpEnable);
		MTWF_PRINT("OceCapIndication \t\t 0x%x\n",
			pOceCtrl->OceCapIndication);
		MTWF_PRINT("ShortSSIDEnabled \t\t %d\n",
			pOceCtrl->ShortSSIDEnabled);
		if (pAd->ApCfg.MBSSID[loop].wdev.pHObj)
			ScanTab = get_scan_tab_by_wdev(pAd, &pAd->ApCfg.MBSSID[loop].wdev);
		if (ScanTab)
			OceScanOceAPList(ScanTab);
	}

#endif /* CONFIG_AP_SUPPORT */
	return true;
}

RTMP_STRING *OceMsgTypeToString(OCE_MSG_TYPE MsgType)
{
	if (MsgType == OCE_MSG_INFO_UPDATE)
		return "OCE_MSG_INFO_UPDATE";
	else
		return "UNKNOWN MSG TYPE";
}

/* OCE_SUPPORT */

/*	format : iwpriv [interface] set oce_reduced_nr=[append]-[nr_entry_num]
*	sample : iwpriv ra0 set oce_reduced_nr=0-7
*			==> renew list,not append,indicate 7 entries
*/

INT Set_OceReducedNRIndicate_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg)
{
	UINT8 i = 0, input = 0;
	RTMP_STRING *macptr;
	BOOLEAN AppendMode = FALSE;
	BOOLEAN Cancelled;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *pWdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	P_OCE_CTRL pOceCtrl = &pWdev->OceCtrl;

	for (i = 0, macptr = rstrtok(arg, "-"); macptr; macptr = rstrtok(NULL, "-"), i++) {
		if (i == 0)
			input = (UINT8)simple_strtol(macptr, 0, 10);
		else
			break;
	}
	AppendMode = (input)?TRUE:FALSE;

	pOceCtrl->bApRnrCompleteEnable = (input)?TRUE:FALSE;

	if (IS_OCE_RNR_ENABLE(pWdev))
		RTMPSetTimer(&pAd->ApCfg.APAutoScanNeighborTimer, OCE_RNR_SCAN_PERIOD);
	else
		RTMPCancelTimer(&pAd->ApCfg.APAutoScanNeighborTimer, &Cancelled);

	return TRUE;
}

INT Set_OceReducedWanEnable_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg)
{
	INT loop = 0;
	P_OCE_CTRL pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
	BOOLEAN enable;

	enable = (BOOLEAN) simple_strtol(arg, 0, 10);
	pOceCtrl->bApReducedWanEnable = enable;

	return enable;
}

#endif /* OCE_SUPPORT */
