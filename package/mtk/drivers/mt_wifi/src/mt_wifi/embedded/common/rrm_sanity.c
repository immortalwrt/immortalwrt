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


/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
BOOLEAN RRM_PeerNeighborReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PCHAR * pSsid,
	OUT PUINT8 pSsidLen)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;
	PMAC_TABLE_ENTRY pEntry;
	PRRM_CONFIG pRrmCfg;
	UINT8 PeerMeasurementType;

	if ((Fr == NULL) || (pDialogToken == NULL))
		return result;

	MsgLen -= sizeof(HEADER_802_11);
	/* skip category and action code. */
	pFramePtr = Fr->Octet;
	pFramePtr += 2;
	MsgLen -= 2;

	pEntry = MacTableLookup(pAd, Fr->Hdr.Addr2);
	if ((pEntry == NULL) || (pEntry->func_tb_idx > pAd->ApCfg.BssidNum))
		return result;

	pRrmCfg = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.RrmCfg;
	*pSsid = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid;
	*pSsidLen = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen;
	result = TRUE;
	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;
	eid_ptr = (PEID_STRUCT)pFramePtr;

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case RRM_NEIGHBOR_REQ_SSID_SUB_ID:
			*pSsid = (PCHAR)eid_ptr->Octet;
			*pSsidLen = eid_ptr->Len;
			break;
		case RRM_NEIGHBOR_REQ_MEASUREMENT_REQUEST_SUB_ID:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s - Got STA Measurement Request\n", __func__);
			pRrmCfg->PeerMeasurementToken = eid_ptr->Octet[0];
			PeerMeasurementType = eid_ptr->Octet[2];
			switch (PeerMeasurementType) {
			case RRM_MEASURE_SUBTYPE_LCI:
					pRrmCfg->bPeerReqLCI = TRUE;
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s - STA Request LCI Measurement Report\n", __func__);
				break;
			case RRM_MEASURE_SUBTYPE_LOCATION_CIVIC:
					pRrmCfg->bPeerReqCIVIC = TRUE;
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s - STA Request CIVIC Measurement Report\n", __func__);
				break;
			default:
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"unknown PeerMeasurementType: %d\n", PeerMeasurementType);
			}
			break;

		case RRM_NEIGHBOR_REQ_VENDOR_SUB_ID:
			break;

		default:
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"unknown Eid: %d\n", eid_ptr->Eid);
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;
}


BOOLEAN RRM_PeerMeasureReportSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PMEASURE_REPORT_INFO pMeasureReportInfo,
	OUT PVOID * pMeasureRep)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;
	/* skip 802.11 header. */
	MsgLen -= sizeof(HEADER_802_11);
	/* skip category and action code. */
	pFramePtr += 2;
	MsgLen -= 2;

	if (pMeasureReportInfo == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;
	eid_ptr = (PEID_STRUCT)pFramePtr;

	if (parse_measurement_ie(eid_ptr->Len) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"Beacon Report length incorrect. Abort parsing\n");
		goto len_error;
	}

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_MEASUREMENT_REPORT:
			NdisMoveMemory(&pMeasureReportInfo->Token, eid_ptr->Octet, 1);
			NdisMoveMemory(&pMeasureReportInfo->ReportMode, eid_ptr->Octet + 1, 1);
			NdisMoveMemory(&pMeasureReportInfo->ReportType, eid_ptr->Octet + 2, 1);
			*pMeasureRep = (PVOID)(eid_ptr->Octet + 3);
			result = TRUE;
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

len_error:
	return result;
}

BOOLEAN RRM_PeerBeaconReqSanity(
		IN PRTMP_ADAPTER pAd,
		IN VOID *pMsg,
		IN ULONG MsgLen,
		OUT PUINT8 pDialogToken,
		OUT PCHAR *pSsid,
		OUT PUINT8 pSsidLen,
		OUT PMEASURE_REQ_INFO pMeasureReqInfo,
		OUT PRRM_BEACON_REQ_INFO pBeaconReq)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = NULL;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PUCHAR ptr = NULL;
	UINT16 RandomInterval = 0;
	UINT16 MeasureDuration = 0;
	UINT8 report_info = 0;
	enum beacon_report_detail report_detail = BEACON_REPORT_DETAIL_ALL_FIELDS_AND_ELEMENTS;
	PRRM_SUBFRAME_INFO pBcnReqSubElem = NULL;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s - Got STA Beacon Measurement Request\n", __func__);

	if ((Fr == NULL) || (pDialogToken == NULL))
		return result;

	if (pMeasureReqInfo == NULL)
		return result;

	MsgLen -= sizeof(HEADER_802_11);
	/* skip category and action code. */
	pFramePtr = Fr->Octet;
	pFramePtr += 2;
	MsgLen -= 2;

	pEntry = MacTableLookup(pAd, Fr->Hdr.Addr2);
	if (pEntry == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;
	/* Skip Repetition */
	pFramePtr += 2;
	MsgLen -= 2;
	eid_ptr = (PEID_STRUCT)pFramePtr;

	switch (eid_ptr->Eid) {
	case RRM_NEIGHBOR_REQ_MEASUREMENT_REQUEST_SUB_ID:
		NdisMoveMemory(&pMeasureReqInfo->Token, eid_ptr->Octet, 1);
		NdisMoveMemory(&pMeasureReqInfo->ReqMode.word, eid_ptr->Octet + 1, 1);
		NdisMoveMemory(&pMeasureReqInfo->ReqType, eid_ptr->Octet + 2, 1);
		switch (pMeasureReqInfo->ReqType) {
		case RRM_MEASURE_SUBTYPE_BEACON:
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s - STA PeerMeasurementType %d\n",
						 __func__, pMeasureReqInfo->ReqType);
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"unhandled PeerMeasurementType: %d\n", pMeasureReqInfo->ReqType);
			break;
		}
		ptr = (PUCHAR)(eid_ptr->Octet + 3);

		NdisMoveMemory(&pBeaconReq->RegulatoryClass, ptr, 1);
		NdisMoveMemory(&pBeaconReq->ChNumber, ptr+1, 1);
		NdisMoveMemory(&RandomInterval, ptr+2, 2);
		pBeaconReq->RandomInterval = SWAP16(RandomInterval);
		NdisMoveMemory(&MeasureDuration, ptr + 4, 2);
		pBeaconReq->MeasureDuration = SWAP16(MeasureDuration);
		NdisMoveMemory(&pBeaconReq->MeasureMode, ptr + 6, 1);
		if (pBeaconReq->MeasureMode != RRM_BCN_REQ_MODE_PASSIVE &&
				pBeaconReq->MeasureMode != RRM_BCN_REQ_MODE_ACTIVE &&
				pBeaconReq->MeasureMode != RRM_BCN_REQ_MODE_BCNTAB)
			return FALSE;

		switch (pBeaconReq->MeasureMode) {
		case RRM_BCN_REQ_MODE_PASSIVE:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"STA RRM_BCN_REQ_MODE_PASSIVE\n");
			break;
		case RRM_BCN_REQ_MODE_ACTIVE:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"STA RRM_BCN_REQ_MODE_ACTIVE\n");
			break;
		case RRM_BCN_REQ_MODE_BCNTAB:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"STA RRM_BCN_REQ_MODE_BCNTAB\n");
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"unknown PeerMeasurementMode: %d\n", pBeaconReq->MeasureMode);
			break;
		}
		COPY_MAC_ADDR(&pBeaconReq->Bssid, ptr + 7);

		result = TRUE;
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s BSSID: ("MACSTR")\n", __func__,
				 MAC2STR(pBeaconReq->Bssid));

		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s - IE_MEASUREMENT_REQUEST., RegClass=%d ChNum=%d RandTime=%d Duration=%d Mode %d\n",
				 __func__, pBeaconReq->RegulatoryClass, pBeaconReq->ChNumber,
				 pBeaconReq->RandomInterval, pBeaconReq->MeasureDuration,
				 pBeaconReq->MeasureMode);
		pBcnReqSubElem = (PRRM_SUBFRAME_INFO)(ptr + 13);

		while (((UCHAR *)pBcnReqSubElem + pBcnReqSubElem->Length + 2) <= ((PUCHAR)pFramePtr + MsgLen)) {
			switch (pBcnReqSubElem->SubId) {
			case RRM_BCN_REQ_SUBID_SSID:
				if (!pBcnReqSubElem->Length) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
							"SSID subelement length wild card SSID %d\n", pBcnReqSubElem->Length);
					break;
				}

				if (pBcnReqSubElem->Length > MAX_LEN_OF_SSID) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Invalid SSID subelement length: %d\n", pBcnReqSubElem->Length);
					return FALSE;
				}

				if (*pSsidLen == 0) {
					*pSsid = (PCHAR)pBcnReqSubElem->Oct;
					*pSsidLen = pBcnReqSubElem->Length;
					*(*pSsid + pBcnReqSubElem->Length) = '\0';
				}
				result = TRUE;
				break;

			case RRM_BCN_REQ_SUBID_BCN_REP_INFO:
				if (pBcnReqSubElem->Length != 2) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Invalid reporting information subelement length: %d\n",
							 pBcnReqSubElem->Length);
					return FALSE;
				}
				report_info = pBcnReqSubElem->Oct[0];
				if (report_info != 0) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"reporting information=%d is not supported\n",
							 report_info);
					return FALSE;
				}
				break;
			case RRM_BCN_REQ_SUBID_RET_DETAIL:
				if (pBcnReqSubElem->Length != 1) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Invalid reporting Detail subelement length: %d\n",
							 pBcnReqSubElem->Length);
					return FALSE;
				}
				report_detail = pBcnReqSubElem->Oct[0];
				if (report_detail > BEACON_REPORT_DETAIL_ALL_FIELDS_AND_ELEMENTS) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Invalid reporting detail: %d \n",
							 report_detail);
					return FALSE;
				}
				break;

			default:
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"unknown Eid: %d\n", pBcnReqSubElem->SubId);
				break;
			}
			pBcnReqSubElem = (PRRM_SUBFRAME_INFO)((UCHAR *)pBcnReqSubElem + 2 + pBcnReqSubElem->Length);
		}
	}

	return result;
}


#endif /* DOT11K_RRM_SUPPORT */

