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
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/

#ifdef DOT11K_RRM_SUPPORT

#include "rt_config.h"

#define RTMP_INSERT_IE(_FRAMEBUFER, _TOTALFRAMELEN, _FRAMELEN, _FRAME) {\
		ULONG _TempLen; \
		MakeOutgoingFrame((_FRAMEBUFER),	&(_TempLen), \
						  (_FRAMELEN),	(_FRAME), \
						  END_OF_ARGS); \
		*(_TOTALFRAMELEN) += (_TempLen); \
	}


int bcn_rep_cnt;
VOID RRM_InsertBcnReqIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pBcnReq)
{
	RTMP_INSERT_IE(pFrameBuf, pFrameLen, sizeof(RRM_BEACON_REQ_INFO), pBcnReq);
}

VOID RRM_InsertBcnRepIE(
		IN PRTMP_ADAPTER pAd,
		OUT PUCHAR pFrameBuf,
		OUT PULONG pFrameLen,
		IN PUCHAR pBcnRep)
{
	RTMP_INSERT_IE(pFrameBuf, pFrameLen, sizeof(RRM_BEACON_REP_INFO), pBcnRep);
}

static VOID RRM_InsertMeasureRepIE(
		IN RTMP_ADAPTER *pAd,
		OUT PUCHAR pFrameBuf,
		OUT PULONG pFrameLen,
		IN UINT8 Len,
		IN PRRM_MEASURE_REP_INFO pMeasureRepIE)
{
	ULONG TempLen;
	UINT8 ElementID = RRM_NEIGHBOR_REP_MEASUREMENT_REPORT_SUB_ID;

	MakeOutgoingFrame(pFrameBuf,                                    &TempLen,
			1,                                                    &ElementID,
			1,                                                    &Len,
			sizeof(RRM_MEASURE_REP_INFO),     pMeasureRepIE,
			END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}

VOID MakeBeaconRepFrame(
		IN RTMP_ADAPTER *pAd,
		OUT PUCHAR pOutBuffer,
		OUT PULONG pFrameLen,
		IN UINT8 TotalLen,
		IN UINT8 Category,
		IN UINT8 Action,
		IN PRRM_MEASURE_REP_INFO pMeasureReportIE,
		IN UINT8 DialogToken)
{
	InsertActField(pAd, (pOutBuffer + *pFrameLen), pFrameLen, Category, Action);
	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + *pFrameLen), pFrameLen, DialogToken);

	RRM_InsertMeasureRepIE(pAd, (pOutBuffer + *pFrameLen), pFrameLen,
			TotalLen, pMeasureReportIE);
	return;
}

VOID RRM_InsertBcnReqSsidSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pSsid,
	IN UINT8 SsidLen)
{
	ULONG TempLen;
	UINT8 SubId = RRM_BCN_REQ_SUBID_SSID;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
					  1,				&SubId,
					  1,				&SsidLen,
					  SsidLen,		(PUCHAR)pSsid,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

VOID RRM_InsertBcnReqRepCndSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 RepCnd,
	IN UINT8 Threshold)
{
	ULONG TempLen;
	UINT8 Len = 2;
	UINT8 SubId = RRM_BCN_REQ_SUBID_BCN_REP_INFO;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
					  1,				&SubId,
					  1,				&Len,
					  1,				&RepCnd,
					  1,				&Threshold,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

VOID RRM_InsertBcnReqRepDetailSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Detail)
{
	ULONG TempLen;
	UINT8 Len = 1;
	UINT8 SubId = RRM_BCN_REQ_SUBID_RET_DETAIL;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
					  1,				&SubId,
					  1,				&Len,
					  1,				&Detail,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

/*
	==========================================================================
	Description:
		Insert RRM Enable Capabilitys IE into frame for AP mode.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.

	Return	: None.
	==========================================================================
 */
#ifdef CONFIG_AP_SUPPORT
VOID RRM_InsertAPRRMEnCapIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN INT BssIdx)
{
	ULONG TempLen;
	UINT8 Len = 5;
	UINT8 ElementID = IE_RRM_EN_CAP;
	RRM_EN_CAP_IE RrmEnCap;

	BSS_STRUCT *pMBss = NULL;

	if (BssIdx < MAX_MBSSID_NUM(pAd))
		pMBss = &pAd->ApCfg.MBSSID[BssIdx];
	else
		return;

	RrmEnCap.word = 0;
	NdisMoveMemory((PUCHAR)&RrmEnCap, (PUCHAR)&pMBss->wdev.RrmCfg.rrm_capabilities, sizeof(RrmEnCap));
#ifdef RT_BIG_ENDIAN
	(*((UINT64 *)&RrmEnCap)) = cpu2le64(*((UINT64 *)&RrmEnCap));
#endif

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
					  1,							&ElementID,
					  1,							&Len,
					  Len,							(PUCHAR)&RrmEnCap,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}
#endif /* CONFIG_AP_SUPPORT */

/*
	==========================================================================
	Description:
		Insert RRM Enable Capabilitys IE into frame for STA mode.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.

	Return  : None.
	==========================================================================
 */
#ifdef CONFIG_STA_SUPPORT
VOID RRM_InsertSTARRMEnCapIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN INT StaIdx)
{
	ULONG TempLen;
	UINT8 Len = 5;
	UINT8 ElementID = IE_RRM_EN_CAP;
	RRM_EN_CAP_IE RrmEnCap;
	PRRM_CONFIG pRrmCfg;

	if (StaIdx < pAd->MSTANum)
		pRrmCfg = &pAd->StaCfg[StaIdx].wdev.RrmCfg;
	else
		return;

	RrmEnCap.word = 0;
	NdisMoveMemory((PUCHAR)&RrmEnCap, (PUCHAR)&pRrmCfg->rrm_capabilities, sizeof(RrmEnCap));
#ifdef RT_BIG_ENDIAN
	(*((UINT64 *)&RrmEnCap)) = cpu2le64(*((UINT64 *)&RrmEnCap));
#endif

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
					  1,							&ElementID,
					  1,							&Len,
					  Len,							(PUCHAR)&RrmEnCap,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}
#endif /* CONFIG_STA_SUPPORT */

VOID RRM_InsertRRMEnCapIE(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN INT BssIdx)
{
	switch (wdev->wdev_type) {
#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_STA:
		RRM_InsertSTARRMEnCapIE(pAd, pFrameBuf, pFrameLen, BssIdx);
		break;
#endif /* CONFIG_STA_SUPPORT */
	default:
#ifdef CONFIG_AP_SUPPORT
		RRM_InsertAPRRMEnCapIE(pAd, pFrameBuf, pFrameLen, BssIdx);
#endif /* CONFIG_AP_SUPPORT */
		break;
	}
	return;
}

VOID RRM_InsertNeighborRepIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Len,
	IN RRM_PNEIGHBOR_REP_INFO pNeighborRepInfo)
{
	ULONG TempLen;
	UINT8 IEId = IE_RRM_NEIGHBOR_REP;

	MakeOutgoingFrame(pFrameBuf,						&TempLen,
					  1,								&IEId,
					  1,								&Len,
					  sizeof(RRM_NEIGHBOR_REP_INFO),	pNeighborRepInfo,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}

VOID RRM_InsertNeighborTSFOffsetSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT16 TSFOffset,
	IN UINT16 BcnInterval)
{
	ULONG TempLen;
	UINT8 Len = 4;
	UINT8 SubId = RRM_NEIGHBOR_REP_TSF_INFO_SUB_ID;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
					  1,				&SubId,
					  1,				&Len,
					  2,				&TSFOffset,
					  2,				&BcnInterval,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}


#ifdef QUIET_SUPPORT
VOID RRM_InsertQuietIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 QuietCnt,
	IN UINT8 QuietPeriod,
	IN UINT8 QuietDuration,
	IN UINT8 QuietOffset)
{
	ULONG TempLen;
	UINT8 IEId = IE_QUIET;
	QUIET_INFO QuietInfo;
	UINT8 Len;

	QuietInfo.QuietCnt = QuietCnt;
	QuietInfo.QuietPeriod = QuietPeriod;
	QuietInfo.QuietDuration = cpu2le16(QuietDuration);
	QuietInfo.QuietOffset = cpu2le16(QuietOffset);
	Len = 6;
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
					  1,				&IEId,
					  1,				&Len,
					  Len,			&QuietInfo,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}
#endif

VOID RRM_InsertBssACDelayIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	ULONG TempLen;
	UINT8 IEId = IE_BSS_AC_DELAY;
	RRM_BSS_AC_DELAY_INFO AcDelayInfo;
	UINT8 Len;

	Len = 4;
	AcDelayInfo.BE_ACDelay = 255;
	AcDelayInfo.BK_ACDelay = 255;
	AcDelayInfo.VI_ACDelay = 255;
	AcDelayInfo.VO_ACDelay = 255;
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
					  1,				&IEId,
					  1,				&Len,
					  Len,			&AcDelayInfo,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}

VOID RRM_InsertBssAvailableACIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	INT idx;
	ULONG TempLen;
	UINT8 IEId = IE_BSS_AVAILABLE_AC;
	RRM_BSS_AVAILABLE_AC_INFO AvailableAcInfo;
	PRRM_BSS_AVAILABLE_AC_BITMAP pAcBitMap;
	UINT8 Len;
	pAcBitMap = (PRRM_BSS_AVAILABLE_AC_BITMAP) \
				&AvailableAcInfo.AvailableAcBitMap;
	pAcBitMap->word = 0;

	/* cacule the total length of the IE. */
	Len = 2;

	for (idx = 0; idx < 12; idx++) {
		if (pAcBitMap->word & (0x1 << idx))
			Len += 2;
	}

#ifdef RT_BIG_ENDIAN
	pAcBitMap->word = cpu2le16(pAcBitMap->word);
#endif

	MakeOutgoingFrame(pFrameBuf,							&TempLen,
					  1,									&IEId,
					  1,									&Len,
					  sizeof(RRM_BSS_AVAILABLE_AC_INFO),	&AvailableAcInfo,
					  END_OF_ARGS);
	*pFrameLen += TempLen;
	pFrameBuf += TempLen;
	return;
}

VOID RRM_InsertRequestIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ie_num,
	IN PUINT8 ie_list)
{
	ULONG TempLen;
	UINT8 IEId = IE_802_11D_REQUEST;
	UINT8 Len;

	Len = ie_num;
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
					  1,				&IEId,
					  1,				&Len,
						Len,			ie_list,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}

VOID RRM_InsertRequestIE_11KV_API(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pRequest,
	IN UINT8 RequestLen)
{
	ULONG TempLen = 0;
	UINT8 IEId = IE_802_11D_REQUEST;
	UINT8 Len = 0;

	Len = RequestLen;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&IEId,
						1,				&Len,
						Len,			pRequest,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

VOID RRM_InsertTxStreamReqIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pBuf)
{
	RTMP_INSERT_IE(pFrameBuf, pFrameLen,
				   sizeof(RRM_TRANSMIT_MEASURE_INFO), pBuf);
}

VOID RRM_InsertTxStreamReqTriggerReportSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pBuf)
{
	ULONG TempLen;
	ULONG Len = sizeof(RRM_TRANSMIT_MEASURE_TRIGGER_REPORT);
	UINT8 SubId = RRM_TX_STREAM_SUBID_TRIGGER_REPORT;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
					  1,				&SubId,
					  1,				&Len,
					  Len,			pBuf,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

VOID RRM_EnqueueBcnReq(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid,
	IN UINT8 IfIdx,
	IN PRRM_MLME_BCN_REQ_INFO pMlmeBcnReq)
{
	UINT8 MeasureReqType = RRM_MEASURE_SUBTYPE_BEACON;
	MEASURE_REQ_MODE MeasureReqMode;
	UINT8 MeasureReqToken = RandomByte(pAd);
	RRM_BEACON_REQ_INFO BcnReq;
	UINT8 ReportDetail;
	UINT8 TotalLen;
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	PMEASURE_REQ_ENTRY pMeasureReqEntry = NULL;

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}

	/* build action frame header. */
	if (wcid) {
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->MacTab.Content[wcid].Addr,
					 pAd->ApCfg.MBSSID[IfIdx].wdev.if_addr,
					 pAd->ApCfg.MBSSID[IfIdx].wdev.bssid);
	} else {
		MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pMlmeBcnReq->Addr,
			pAd->ApCfg.MBSSID[IfIdx].wdev.if_addr,
			pAd->ApCfg.MBSSID[IfIdx].wdev.bssid);
	}

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	/*
		Action header has a field to indicate total length of packet
		but the total length is unknow untial whole packet completd.
		So skip the action here and fill it late.
		1. skip Catgore (1 octect), Action(1 octect).
		2. skip dailog token (1 octect).
		3. skip Num Of Repetitions field (2 octects)
		3. skip MeasureReqIE (2 + sizeof(MEASURE_REQ_INFO)).
	*/
	FrameLen += (7 + sizeof(MEASURE_REQ_INFO));
	TotalLen = sizeof(MEASURE_REQ_INFO);
	/*
		Insert Beacon Req IE.
	*/
	BcnReq.RegulatoryClass = pMlmeBcnReq->RegulatoryClass;
	BcnReq.ChNumber = pMlmeBcnReq->MeasureCh;
	BcnReq.RandomInterval = cpu2le16(pMlmeBcnReq->RandInt);

	BcnReq.MeasureDuration = cpu2le16(pMlmeBcnReq->MeasureDuration);
	BcnReq.MeasureMode = pMlmeBcnReq->MeasureMode;
	COPY_MAC_ADDR(BcnReq.Bssid, pMlmeBcnReq->Bssid);
	RRM_InsertBcnReqIE(pAd, (pOutBuffer + FrameLen),
					   &FrameLen, (PUCHAR)&BcnReq);
	TotalLen += sizeof(RRM_BEACON_REQ_INFO);
	/* insert SSID sub field. */
	if (pMlmeBcnReq->SsidLen != 0) {
		RRM_InsertBcnReqSsidSubIE(pAd, (pOutBuffer + FrameLen),
								  &FrameLen, (PUCHAR)pMlmeBcnReq->pSsid, pMlmeBcnReq->SsidLen);
		TotalLen += (pMlmeBcnReq->SsidLen + 2); /* SSID sub field. */
	}

	/* insert report condition sub field. */
	if (pMlmeBcnReq->BcnReqCapFlag.field.ReportCondition == TRUE) {
		RRM_InsertBcnReqRepCndSubIE(pAd, (pOutBuffer + FrameLen), &FrameLen, 0, 0);
		TotalLen += (2 + 2); /* SSID sub field. */
	}

	/* insert channel report sub field. */
	if (pMlmeBcnReq->BcnReqCapFlag.field.ChannelRep == TRUE) {
		ULONG idx;

		idx = 0;

		while (pMlmeBcnReq->ChRepRegulatoryClass[idx] != 0) {
			ULONG FramelenTmp = FrameLen;

			InsertChannelRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
							   (RTMP_STRING *)pAd->CommonCfg.CountryCode,
							   pMlmeBcnReq->ChRepRegulatoryClass[idx],
							   &pMlmeBcnReq->ChRepList[0],
							   pAd->ApCfg.MBSSID[IfIdx].wdev.PhyMode, IfIdx);
			TotalLen += (FrameLen - FramelenTmp);
			idx++;
		}
	}

	/* insert report detail sub field. */
	ReportDetail = pMlmeBcnReq->report_detail;
	if (BcnReq.MeasureMode == RRM_BCN_REQ_MODE_BCNTAB)
		ReportDetail = 0; /*force report detail to 0 when it is table mode*/
	{   /*insert report detail sub ie*/
		ULONG FramelenTmp = FrameLen;
		RRM_InsertBcnReqRepDetailSubIE(pAd, (pOutBuffer + FrameLen), &FrameLen, ReportDetail);
		TotalLen += (FrameLen - FramelenTmp);
	}

	if (ReportDetail == 1) {
		/*if report detail is 1, insert request ie list*/
		ULONG FramelenTmp = FrameLen;
		RRM_InsertRequestIE (pAd,
							(pOutBuffer + FrameLen),
							&FrameLen,
							pMlmeBcnReq->request_ie_num,
							pMlmeBcnReq->request_ie);
		TotalLen += (FrameLen - FramelenTmp);
	}

	if (pMlmeBcnReq->LastBcnRptInd) {
		/* Adjust TotalLen of the Measurement Req while inserting
		 * Bcn Report Indication*/
		ULONG FramelenTmp = FrameLen;
		InsertBcnReportIndicationReqIE(pAd, (pOutBuffer + FrameLen), &FrameLen, 1);
		TotalLen += (FrameLen - FramelenTmp);
	}

	/* Insert Action header here. */
	{
		ULONG tmpLen = sizeof(HEADER_802_11);

		MeasureReqMode.word = 0;
		MeasureReqMode.field.DurationMandatory = 0;
		MeasureReqMode.field.Report = 0;
		MeasureReqMode.field.Request = 0;
		MeasureReqMode.field.Enable = 0;
		MeasureReqMode.field.Parallel = 0;

		MakeMeasurementReqFrame(pAd, pOutBuffer, &tmpLen,
								TotalLen, CATEGORY_RM, RRM_MEASURE_REQ, MeasureReqToken,
								MeasureReqMode.word, MeasureReqType, 0);
	}
	pMeasureReqEntry = MeasureReqInsert(pAd, MeasureReqToken, BCN_MEASURE_REQ);
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);
	if (!pMeasureReqEntry) {
		MlmeFreeMemory(pOutBuffer);
		return;
	}
	pMeasureReqEntry->BcnCurrentState = WAIT_BCN_REP;
	pMeasureReqEntry->skip_time_check = TRUE;
	pMeasureReqEntry->Priv = pAd;
	pMeasureReqEntry->ControlIndex = IfIdx;
	//COPY_MAC_ADDR(pMeasureReqEntry->StaMac, pBcnReq->peer_address);
	RTMPInitTimer(pAd, &pMeasureReqEntry->WaitBCNRepTimer,
		GET_TIMER_FUNCTION(WaitPeerBCNRepTimeout), pMeasureReqEntry, FALSE);
	RTMPSetTimer(&pMeasureReqEntry->WaitBCNRepTimer, 2000);
	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

	return;
}


void RRM_measurement_report_to_host(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);

int RRM_EnqueueBcnReqAction(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 IfIdx,
	IN p_bcn_req_data_t p_bcn_req_data)
{
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	UINT unicast_peer = 0, count = 0;
	MAC_TABLE_ENTRY *pEntry;
	UINT8 mode;

	mode = p_bcn_req_data->bcn_req[RRM_BCNREQ_MODE_OFFSET];

	if (mode > 0x2) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, "Incorrect mode\n");
		return NDIS_STATUS_FAILURE;
	}

	while (count < MAC_ADDR_LEN) {
		if (p_bcn_req_data->peer_address[count++] != 0xff) {
			unicast_peer = 1;
			break;
		}
	}

	if (unicast_peer) {
		pEntry = MacTableLookup(pAd, p_bcn_req_data->peer_address);

		if (!pEntry || (pEntry->Sst != SST_ASSOC)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, "Peer Not connected\n");
			return NDIS_STATUS_FAILURE;
		}

		if ((pEntry->func_tb_idx == IfIdx)
			&& IS_RRM_ENABLE(pEntry->wdev)
			&& ((pEntry->RrmEnCap.word >> 0x4) & (1 << mode))) {
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, "Peer does not support this request\n");
			return NDIS_STATUS_FAILURE;
		}
	}

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return NDIS_STATUS_FAILURE;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, p_bcn_req_data->peer_address,
					 pAd->ApCfg.MBSSID[IfIdx].wdev.if_addr,
					 pAd->ApCfg.MBSSID[IfIdx].wdev.bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, MT2_PEER_RM_CATE, RRM_MEASURE_REQ);
	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, p_bcn_req_data->dialog_token);
	RTMP_INSERT_IE(pOutBuffer + FrameLen, &FrameLen, p_bcn_req_data->bcn_req_len, p_bcn_req_data->bcn_req);
	NStatus = MiniportMMRequest(pAd, MGMT_USE_QUEUE_FLAG | QID_AC_BE, pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

	return NStatus;
}

INT rrm_send_beacon_req_param(
	IN PRTMP_ADAPTER pAd,
	IN p_bcn_req_info pBcnReq,
	IN UINT32 BcnReqLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = NULL;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	PBCN_EVENT_DATA Event = NULL;
	UINT32 Len = 0;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	PUCHAR pBuf = NULL;
	UINT8 IfIdx = pObj->ioctl_if;
	UINT8 MeasureReqToken = 0;
	static UINT8 k = 1;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s\n", __func__);

	/* check the AP supports 11k or not */
	pMbss = &pAd->ApCfg.MBSSID[IfIdx];
	if (pMbss->wdev.RrmCfg.bDot11kRRMEnable == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			"rrm off\n");
		return NDIS_STATUS_FAILURE;
	}

	if (BcnReqLen != sizeof(*pBcnReq)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			"bcn req len check failed\n");
		return NDIS_STATUS_FAILURE;
	}

	/*send bcn req msg into mlme*/
	Len = sizeof(*Event) + BcnReqLen;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, Len);
	if (!pBuf) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			"allocate event memory failed \n");
		return NDIS_STATUS_RESOURCES;
	}
	NdisZeroMemory(pBuf, Len);

	/*insert bcn req token into measure table*/
	k++;
	if (k == 0)
		k = 1;
	MeasureReqToken = k;
	pEntry = MeasureReqInsert(pAd, MeasureReqToken, BCN_MEASURE_REQ);
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			"Fail to Insert MesureReq Token(%d)!\n", MeasureReqToken);
		os_free_mem(pBuf);
		return NDIS_STATUS_FAILURE;
	}
	pEntry->skip_time_check = TRUE;
	pEntry->BcnCurrentState = WAIT_BCN_REQ;
	pEntry->Priv = pAd;
	pEntry->ControlIndex = IfIdx;
	COPY_MAC_ADDR(pEntry->StaMac, pBcnReq->peer_address);

	Event = (PBCN_EVENT_DATA)pBuf;
	Event->ControlIndex = IfIdx;
	Event->MeasureReqToken = MeasureReqToken;
	Event->measuretype = BCN_MEASURE_REQ;
	RTMPMoveMemory(Event->stamac, pBcnReq->peer_address, MAC_ADDR_LEN);
	Event->DataLen = BcnReqLen;
	NdisMoveMemory(Event->Data, pBcnReq, BcnReqLen);

	if (MlmeEnqueue(pAd, BCN_STATE_MACHINE, BCN_REQ, Len, pBuf, 0) == FALSE) {
		NStatus = NDIS_STATUS_FAILURE;
		MeasureReqDelete(pAd, MeasureReqToken, BCN_MEASURE_REQ);
	}

	/*free memory*/
	os_free_mem(pBuf);
	return NStatus;
}


#ifdef CONFIG_STA_SUPPORT
VOID RRM_EnqueueNeighborReq(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pDA,
	IN PUINT8 pSsid,
	IN UINT8 SsidLen)
{
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	UINT8 DialogToken = RandomByte(pAd);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA, pAd->StaCfg[0].wdev.if_addr, pAd->StaCfg[0].MlmeAux.Bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	/* fill Dialog Token */
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

	return;
}
#endif /* CONFIG_STA_SUPPORT */

/*
	reference 2012 spec.
	802.11-2012.pdf
	page#581 (0 is not euqal to no security )
	The Security bit, if 1, indicates that the AP identified by this BSSID supports the same security provisioning
	as used by the STA in its current association. If the bit is 0, it indicates either that the AP does not support
	the same security provisioning or that the security information is not available at this time.
*/
BOOLEAN RRM_CheckBssAndStaSecurityMatch(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BSS_ENTRY *pBssEntry
)
{
	BOOLEAN ret = FALSE;

	if ((pBssEntry->AKMMap & pEntry->SecConfig.AKMMap)
	&& (pBssEntry->PairwiseCipher & pEntry->SecConfig.PairwiseCipher))
		ret = TRUE;
	else
		ret = FALSE;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO,
		"%s: pBssEntry %s/%s & pSTA %s/%s = (%d/%d)  ret %d\n", __func__,
		GetAuthModeStr(pBssEntry->AKMMap), GetEncryModeStr(pBssEntry->PairwiseCipher),
		GetAuthModeStr(pEntry->SecConfig.AKMMap), GetEncryModeStr(pEntry->SecConfig.PairwiseCipher),
		(pBssEntry->AKMMap & pEntry->SecConfig.AKMMap),
		(pBssEntry->PairwiseCipher & pEntry->SecConfig.PairwiseCipher),
		ret
		);

	return ret;
}

VOID RRM_EnqueueNeighborRep(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UINT8 DialogToken,
	IN PCHAR pSsid,
	IN UINT8 SsidLen)
{
#define MIN(_x, _y) ((_x) > (_y) ? (_x) : (_y))
	UINT loop;
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	ULONG PktLen;
	PRRM_CONFIG pRrmCfg;
	RRM_NEIGHBOR_REP_INFO NeighborRepInfo;
	struct wifi_dev *wdev = NULL;
	BSS_TABLE *ScanTab = NULL;

	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, "Invalid STA.\n");
		return;
	}

	wdev = pEntry->wdev;

	if ((wdev == NULL) || (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, "Invalid STA.\n");
		return;
	}

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
					 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
					 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
				   CATEGORY_RM, RRM_NEIGHTBOR_RSP);
	/* fill Dialog Token */
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	pRrmCfg = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.RrmCfg;
#ifdef AP_SCAN_SUPPORT
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	/* insert NeighborRep IE. */
	for (loop = 0; loop < ScanTab->BssNr; loop++) {
		UINT8 BssMatch = FALSE;
		BSS_ENTRY *pBssEntry = &ScanTab->BssEntry[loop];
		/* Discards all remain Bss if the packet length exceed packet buffer size. */
		PktLen = FrameLen + sizeof(RRM_NEIGHBOR_REP_INFO)
				 + (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.RrmCfg.bDot11kRRMNeighborRepTSFEnable == TRUE ? 6 : 0);

		if (PktLen >= MAX_MGMT_PKT_LEN)
			break;

		if (SsidLen != 0)
			BssMatch = RTMPEqualMemory(pBssEntry->Ssid, pSsid,
									   MIN(SsidLen, pBssEntry->SsidLen));
		else
			BssMatch = TRUE;

		if (BssMatch) {
			RRM_BSSID_INFO BssidInfo;

			BssidInfo.word = 0;
			BssidInfo.field.APReachAble = 3;
			BssidInfo.field.Security = RRM_CheckBssAndStaSecurityMatch(pAd, pEntry, pBssEntry);

			BssidInfo.field.KeyScope = 0; /* "report AP has same authenticator as the AP. */
			/*
				reference 2012 spec.
				802.11-2012.pdf
				page#582 (0 means information is not available  )
				The Key Scope bit, when set, indicates the AP indicated by this BSSID has the same authenticator as the AP
				sending the report. If this bit is 0, it indicates a distinct authenticator or the information is not available.
			*/
			BssidInfo.field.SpectrumMng = (pBssEntry->CapabilityInfo & (1 << 8)) ? 1 : 0;
			BssidInfo.field.Qos = (pBssEntry->CapabilityInfo & (1 << 9)) ? 1 : 0;
			BssidInfo.field.APSD = (pBssEntry->CapabilityInfo & (1 << 11)) ? 1 : 0;
			BssidInfo.field.RRM = (pBssEntry->CapabilityInfo & RRM_CAP_BIT) ? 1 : 0;
			BssidInfo.field.DelayBlockAck = (pBssEntry->CapabilityInfo & (1 << 14)) ? 1 : 0;
			BssidInfo.field.ImmediateBA = (pBssEntry->CapabilityInfo & (1 << 15)) ? 1 : 0;
			BssidInfo.field.MobilityDomain = (pBssEntry->bHasMDIE) ? 1 : 0;
			BssidInfo.field.HT = HAS_HT_CAPS_EXIST(pBssEntry->ie_exists) ? 1 : 0;
#ifdef DOT11_VHT_AC
			BssidInfo.field.VHT = HAS_VHT_CAPS_EXIST(pBssEntry->ie_exists) ? 1 : 0;
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
			BssidInfo.field.HE = HAS_HE_CAPS_EXIST(pBssEntry->ie_exists) ? 1 : 0;
#endif /*DOT11_HE_AX*/

			/*
			reference spec:
			dot11FrameRprtPhyType OBJECT-TYPE
			SYNTAX INTEGER {
			fhss(1),
			dsss(2),
			irbaseband(3),
			ofdm(4),
			hrdsss(5),
			erp(6),
			ht(7),
			vht(9)
			}

			*/

			if (pBssEntry->Channel > 14) { /* 5G case */
				if (HAS_HT_CAPS_EXIST(pBssEntry->ie_exists)) { /* HT or Higher case */
#ifdef DOT11_HE_AX
					if (HAS_HE_CAPS_EXIST(pBssEntry->ie_exists))
						pBssEntry->CondensedPhyType = 14;
					else
#endif /*DOT11_HE_AX*/
#ifdef DOT11_VHT_AC
					if (HAS_VHT_CAPS_EXIST(pBssEntry->ie_exists))
						pBssEntry->CondensedPhyType = 9;
					else
#endif /* DOT11_VHT_AC */
						pBssEntry->CondensedPhyType = 7;
				} else /* OFDM case */
					pBssEntry->CondensedPhyType = 4;
			} else { /* 2.4G case */
#ifdef DOT11_HE_AX
				if (HAS_HE_CAPS_EXIST(pBssEntry->ie_exists))
					pBssEntry->CondensedPhyType = 14;
				else
#endif /*DOT11_HE_AX*/
				if (HAS_HT_CAPS_EXIST(pBssEntry->ie_exists)) /* HT case */
					pBssEntry->CondensedPhyType = 7;
				else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp)) /* ERP case */
					pBssEntry->CondensedPhyType = 6;
				else if (pBssEntry->SupRateLen > 4)/* OFDM case (1,2,5.5,11 for CCK 4 Rates) */
					pBssEntry->CondensedPhyType = 4;

				/* no CCK's definition in spec. */
			}

			COPY_MAC_ADDR(NeighborRepInfo.Bssid, pBssEntry->Bssid);
			NeighborRepInfo.BssidInfo = cpu2le32(BssidInfo.word);
			NeighborRepInfo.RegulatoryClass = pBssEntry->RegulatoryClass;
			NeighborRepInfo.ChNum = pBssEntry->Channel;
			NeighborRepInfo.PhyType = pBssEntry->CondensedPhyType;
			RRM_InsertNeighborRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
									sizeof(RRM_NEIGHBOR_REP_INFO), &NeighborRepInfo);
		}

		/*
			shall insert Neighbor Report TSF offset
			when the MIB attribute
			dot11RRMNeighborReportTSFOffsetEnabled is true.
		*/
		if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.RrmCfg.bDot11kRRMNeighborRepTSFEnable) {
			UINT32 Ttfs = (UINT32)(pBssEntry->TTSF[3] << 24)
						  + (UINT32)(pBssEntry->TTSF[2] << 16)
						  + (UINT32)(pBssEntry->TTSF[1] << 8)
						  + (UINT32)(pBssEntry->TTSF[0]);
			UINT32 Ptfs = (UINT32)(pBssEntry->PTSF[3] << 24)
						  + (UINT32)(pBssEntry->PTSF[2] << 16)
						  + (UINT32)(pBssEntry->PTSF[1] << 8)
						  + (UINT32)(pBssEntry->PTSF[0]);
			RRM_InsertNeighborTSFOffsetSubIE(pAd, (pOutBuffer + FrameLen),
				&FrameLen, cpu2le16((UINT16)ABS(Ttfs, Ptfs)),
				cpu2le16(pBssEntry->BeaconPeriod));
		}
	}

#endif /* AP_SCAN_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "send Neighbor RSP\n");
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

	return;
}

VOID RRM_EnqueueLinkMeasureReq(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Aid,
	IN UINT8 apidx)
{
	UINT8 DialogToken = RandomByte(pAd);
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	PMAC_TABLE_ENTRY pEntry;
	struct wifi_dev *wdev;

	if ((apidx >= pAd->ApCfg.BssidNum)
		|| (!(VALID_UCAST_ENTRY_WCID(pAd, Aid)))) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, "Invalid STA. apidx=%d Aid=%d\n",
				 apidx, Aid);
		return;
	}

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /* Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}

	pEntry = &pAd->MacTab.Content[Aid];
	wdev = pEntry->wdev;
	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
					 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
					 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
				   CATEGORY_RM, RRM_LNK_MEASURE_REQ);
	/* fill Dialog Token */
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	/* fill Tx Power Used field */
	{
		ULONG TempLen;
		UINT8 TxPwr = RTMP_GetTxPwr(pAd, wdev->rate.MlmeTransmit, wdev->channel, wdev);

		MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
						  1,							&TxPwr,
						  END_OF_ARGS);
		FrameLen += TempLen;
	}
	/* fill Max Tx Power field */
	{
		ULONG TempLen;
		UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
		UINT8 MaxTxPwr = GetCuntryMaxTxPwr(pAd, wdev->PhyMode, wdev, op_ht_bw);

		MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
						  1,							&MaxTxPwr,
						  END_OF_ARGS);
		FrameLen += TempLen;
	}
	/* MeasureReqInsert(pAd, DialogToken); */
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

	return;
}

VOID RRM_EnqueueTxStreamMeasureReq(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Aid,
	IN UINT8 apidx,
	IN PRRM_MLME_TRANSMIT_REQ_INFO pMlmeTxMeasureReq)
{
	UINT8 MeasureReqType = RRM_MEASURE_SUBTYPE_TX_STREAM;
	MEASURE_REQ_MODE MeasureReqMode;
	UINT8 MeasureReqToken = RandomByte(pAd);
	RRM_TRANSMIT_MEASURE_INFO TxMeasureReq;
	RRM_TRANSMIT_MEASURE_TRIGGER_REPORT TriggerReport;
	UINT8 TotalLen;
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}

	MeasureReqMode.word = 0;
	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->MacTab.Content[Aid].Addr,
					 pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
					 pAd->ApCfg.MBSSID[apidx].wdev.bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	/*
		Action header has a field to indicate total length of packet
		but the total length is unknow untial whole packet completd.
		So skip the action here and fill it late.
		1. skip Catgore (1 octect), Action(1 octect).
		2. skip dailog token (1 octect).
		3. skip Num Of Repetitions field (2 octects)
		3. skip MeasureReqIE (2 + sizeof(MEASURE_REQ_INFO)).
	*/
	FrameLen += (7 + sizeof(MEASURE_REQ_INFO));
	TotalLen = sizeof(MEASURE_REQ_INFO);
	/*
		Insert Tx stream Measure Req IE.
	*/
	/* according to WiFi Voice-enterprise testing req. the RandomInterval shall be zero. */
	TxMeasureReq.RandomInterval = 0;
	TxMeasureReq.MeasureDuration = cpu2le16(pMlmeTxMeasureReq->MeasureDuration);
	COPY_MAC_ADDR(TxMeasureReq.PeerStaMac, pAd->MacTab.Content[Aid].Addr);
	TxMeasureReq.TIDField.Rev = 0;
	TxMeasureReq.TIDField.TID = pMlmeTxMeasureReq->Tid;
	TxMeasureReq.Bin0Range = pMlmeTxMeasureReq->BinRange;
	RRM_InsertTxStreamReqIE(pAd, (pOutBuffer + FrameLen),
							&FrameLen, (PUCHAR)&TxMeasureReq);
	TotalLen += sizeof(RRM_TRANSMIT_MEASURE_INFO);

	/* insert Trigger report sub field. */
	if (pMlmeTxMeasureReq->bTriggerReport == 1) {
		PRRM_TRANSMIT_MEASURE_TRIGGER_CONDITION pTiggerCon
			= (PRRM_TRANSMIT_MEASURE_TRIGGER_CONDITION)&TriggerReport.TriggerCondition;
		pTiggerCon->field.Average = pMlmeTxMeasureReq->ArvCondition;
		pTiggerCon->field.Consecutive = pMlmeTxMeasureReq->ConsecutiveCondition;
		pTiggerCon->field.Delay = pMlmeTxMeasureReq->DelayCondition;

		if (pTiggerCon->field.Average)
			TriggerReport.AvrErrorThreshold = pMlmeTxMeasureReq->AvrErrorThreshold;

		if (pTiggerCon->field.Consecutive)
			TriggerReport.ConsecutiveErrorThreshold = pMlmeTxMeasureReq->ConsecutiveErrorThreshold;

		if (pTiggerCon->field.Delay)
			TriggerReport.DelayThreshold = pMlmeTxMeasureReq->DelayThreshold;

		TriggerReport.TriggerTimeout = pMlmeTxMeasureReq->TriggerTimeout;
		TriggerReport.MeasurementCnt = pMlmeTxMeasureReq->MeasureCnt;
		RRM_InsertTxStreamReqTriggerReportSubIE(pAd, (pOutBuffer + FrameLen),
												&FrameLen, (PUCHAR)&TriggerReport);
		TotalLen +=
			(sizeof(RRM_TRANSMIT_MEASURE_TRIGGER_REPORT) + 2);
		MeasureReqMode.field.Report = 1;
	}

	/* Insert Action header here. */
	{
		ULONG tmpLen = sizeof(HEADER_802_11);

		MeasureReqMode.field.Enable = 1;
		MeasureReqMode.field.DurationMandatory =
			pMlmeTxMeasureReq->bDurationMandatory;
		MakeMeasurementReqFrame(pAd, pOutBuffer, &tmpLen,
								TotalLen, CATEGORY_RM, RRM_MEASURE_REQ, MeasureReqToken,
								MeasureReqMode.word, MeasureReqType, 0xffff);
	}
	/* MeasureReqInsert(pAd, MeasureReqToken); */
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

	return;
}

/**
 * rssi_to_rcpi - Convert RSSI to RCPI
 * @rssi: RSSI to convert
 * Returns: RCPI corresponding to the given RSSI value, or 255 if not available.
 *
 * It's possible to estimate RCPI based on RSSI in dBm. This calculation will
 * not reflect the correct value for high rates, but it's good enough for Action
 * frames which are transmitted with up to 24 Mbps rates.
 */
UINT8 rssi_to_rcpi(CHAR rssi)
{
	if (!rssi)
		return 255; /* not available */
	if (rssi < -110)
		return 0;
	if (rssi > 0)
		return 220;
	return (rssi + 110) * 2;
}

int EnqueueBeaconRepFrame(RTMP_ADAPTER *pAd,
		UCHAR *pOutBuffer,
		UINT FrameLen, UINT8 DialogToken,
		UCHAR *bssid)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	PUCHAR pBuf = NULL;
	PBCN_EVENT_DATA Event;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	UINT32 EventLen = 0;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	UCHAR ifIndex = pObj->ioctl_if;
	static UINT8 Token = 0;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s\n", __func__);

	/*send bcn rep msg into mlme*/
	EventLen = sizeof(*Event) + FrameLen;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, EventLen);
	if (!pBuf) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"allocate event memory failed \n");
		return NDIS_STATUS_RESOURCES;
	}
	NdisZeroMemory(pBuf, EventLen);

	/*insert bcn req token into measure table*/
	if (DialogToken == 0) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"invalid MesureReq Token(0)!\n");
		os_free_mem(pBuf);
		return NDIS_STATUS_FAILURE;
	}
	if (Token != DialogToken) {
		pEntry = MeasureReqInsert(pAd, DialogToken, BCN_MEASURE_REP);
		Token = DialogToken;
		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
					"Fail to Insert MesureReq Token(%d)!\n",
					 DialogToken);
			os_free_mem(pBuf);
			return NDIS_STATUS_FAILURE;
		}
	}

	if (pEntry) {
		pEntry->skip_time_check = TRUE;
		pEntry->BcnCurrentState = WAIT_BCN_REP;
		pEntry->Priv = pAd;
		pEntry->ControlIndex = ifIndex;
	}

	Event = (BCN_EVENT_DATA *)pBuf;
	Event->ControlIndex = ifIndex;
	Event->MeasureReqToken = DialogToken;
	Event->measuretype = BCN_MEASURE_REP;
	RTMPMoveMemory(Event->stamac, bssid, MAC_ADDR_LEN);
	Event->DataLen = FrameLen;

	NdisMoveMemory(Event->Data, pOutBuffer, FrameLen);
	bcn_rep_cnt++;
	if (!MlmeEnqueue(pAd, BCN_STATE_MACHINE, BCN_REP, EventLen, pBuf, 0)) {
		NStatus = NDIS_STATUS_FAILURE;
		MeasureReqDelete(pAd, DialogToken, BCN_MEASURE_REP);
		bcn_rep_cnt--;
	}

	/*free memory*/
	os_free_mem(pBuf);
	return NStatus;
}

VOID RRM_EnqueuePeerBeaconRep(
		IN RTMP_ADAPTER *pAd,
		IN PUCHAR pDA,
		IN PUCHAR pSA,
		IN UINT8 DialogToken,
		MEASURE_REQ_INFO MeasureReqInfo,
		IN RRM_BEACON_REQ_INFO BeaconReq,
		BSS_ENTRY *pBssEntry)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 ActHdr;
	RRM_MEASURE_REP_INFO MeasureRepIE;
	RRM_BEACON_REP_INFO BcnRep;
	UINT8 TotalLen = 0;
	UINT32 ptsf = 0;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s::\n", __func__);
	/* build action frame header.*/
	MgtMacHeaderInitExt(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
			pSA, pDA);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "allocate memory failed\n");
		return;
	}

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	/*
	   Action header has a field to indicate total length of packet
	   but the total length is unknow until whole packet completd.
	   So skip the action here and fill it late.
	   1. skip Catgore (1 octect), Action(1 octect).
	   2. skip dailog token (1 octect).
	   3. skip MeasureRepIE (2 + sizeof(RRM_MEASURE_REP_INFO)).
	 */
	FrameLen += (5 + sizeof(RRM_MEASURE_REP_INFO));
	TotalLen = sizeof(RRM_MEASURE_REP_INFO);

	if (pBssEntry != NULL) {
		if (pBssEntry->Channel > 14) { /* 5G case */
			if (HAS_HT_CAPS_EXIST(pBssEntry->ie_exists)) { /* HT or Higher case */
#ifdef DOT11_VHT_AC
				if (HAS_VHT_CAPS_EXIST(pBssEntry->ie_exists))
					pBssEntry->CondensedPhyType = 9;
				else
#endif /* DOT11_VHT_AC */
					pBssEntry->CondensedPhyType = 7;
			} else /* OFDM case */
				pBssEntry->CondensedPhyType = 4;
		} else { /* 2.4G case */
			if (HAS_HT_CAPS_EXIST(pBssEntry->ie_exists)) /* HT case */
				pBssEntry->CondensedPhyType = 7;
			else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp)) /* ERP case */
				pBssEntry->CondensedPhyType = 6;
			else if (pBssEntry->SupRateLen > 4)/* OFDM case (1,2,5.5,11 for CCK 4 Rates) */
				pBssEntry->CondensedPhyType = 4;

			/* no CCK's definition in spec. */
		}

		NdisZeroMemory(&BcnRep, sizeof(RRM_BEACON_REP_INFO));
		if (!pBssEntry->RegulatoryClass)
			BcnRep.RegulatoryClass = BeaconReq.RegulatoryClass;
		else
			BcnRep.RegulatoryClass = pBssEntry->RegulatoryClass;
		BcnRep.ChNumber =  pBssEntry->Channel;
		BcnRep.ActualMeasureStartTime = cpu2le64(0);
		BcnRep.MeasureDuration = cpu2le16(BeaconReq.MeasureDuration);
		BcnRep.RepFrameInfo = pBssEntry->CondensedPhyType;
		BcnRep.RCPI = rssi_to_rcpi(pBssEntry->Rssi);
		BcnRep.RSNI = pBssEntry->RSNI; /* 255 indicates that RSNI is not available */
		COPY_MAC_ADDR(BcnRep.Bssid, pBssEntry->Bssid);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "%s::RegulatoryClass=%d,ChNumber=%d,RCPI=%d,Rssi=%d\n",
			__func__, BcnRep.RegulatoryClass, BcnRep.ChNumber, BcnRep.RCPI, pBssEntry->Rssi);

		BcnRep.AnntaId = 0; /* unknown */
		ptsf = (UINT32)(pBssEntry->PTSF[3] << 24)
			+ (UINT32)(pBssEntry->PTSF[2] << 16)
			+ (UINT32)(pBssEntry->PTSF[1] << 8)
			+ (UINT32)(pBssEntry->PTSF[0]);
		BcnRep.ParentTSF = cpu2le32(ptsf);
		RRM_InsertBcnRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen, (PUCHAR)&BcnRep);
		TotalLen += sizeof(RRM_BEACON_REP_INFO);
	}

	/* Insert Action header here. */
	{
		ULONG tmpLen = sizeof(HEADER_802_11);

		/* prepare Measurement IE.*/
		NdisZeroMemory(&MeasureRepIE, sizeof(RRM_MEASURE_REP_INFO));
		MeasureRepIE.Token = MeasureReqInfo.Token;
		MeasureRepIE.ReportMode.word = MeasureReqInfo.ReqMode.word;
		MeasureRepIE.ReportMode.field.Late = 0;
		MeasureRepIE.ReportMode.field.Incapable = 0;
		MeasureRepIE.ReportMode.field.Refused = 0;
		MeasureRepIE.ReportMode.field.Rev = 0;
		MeasureRepIE.ReportType = MeasureReqInfo.ReqType;

		MakeBeaconRepFrame(pAd, pOutBuffer, &tmpLen, TotalLen, CATEGORY_RM, RRM_MEASURE_REP, &MeasureRepIE, DialogToken);

	}
	NStatus = EnqueueBeaconRepFrame(pAd, pOutBuffer, FrameLen, DialogToken, pDA);
	if (NStatus != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
		"%s:: Beacon Request frame Enqueue failed\n", __func__);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);
	return;

}

#endif /* DOT11K_RRM_SUPPORT */

