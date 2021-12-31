/****************************************************************************
* Mediatek Inc.
* 5F., No.5, Taiyuan 1st St., Zhubei City,
* Hsinchu County 302, Taiwan, R.O.C.
* (c) Copyright 2014, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Mediatek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
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

static VOID OceResetScanIndication(PVOID SystemSpecific1, PVOID FunctionContext,
				   PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	INT OceNonOcePresentOldValue, OceBOnlyPresentOldValue;
	P_OCE_CTRL pOceCtrl = NULL;
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	BOOLEAN Cancelled;

	pOceCtrl = &wdev->OceCtrl;

	OceNonOcePresentOldValue =
		OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
				      OCE_NONOCE_PRESENT_MASK,
				      OCE_NONOCE_PRESENT_OFFSET);
	OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication, 0,
			      OCE_NONOCE_PRESENT_MASK,
			      OCE_NONOCE_PRESENT_OFFSET);
	OceBOnlyPresentOldValue =
		OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
				      OCE_11B_ONLY_PRESENT_MASK,
				      OCE_11B_ONLY_PRESENT_OFFSET);
	OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication, 0,
			      OCE_11B_ONLY_PRESENT_MASK,
			      OCE_11B_ONLY_PRESENT_OFFSET);

	if ((OceNonOcePresentOldValue !=
	     OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
				   OCE_NONOCE_PRESENT_MASK,
				   OCE_NONOCE_PRESENT_OFFSET)) ||
	    (OceBOnlyPresentOldValue !=
	     OCE_GET_CONTROL_FIELD(pOceCtrl->OceCapIndication,
				   OCE_11B_ONLY_PRESENT_MASK,
				   OCE_11B_ONLY_PRESENT_OFFSET)))
		OceSendFilsDiscoveryAction(ad, wdev);

	RTMPCancelTimer(&pOceCtrl->Scan11bOceAPTimer, &Cancelled);
	pOceCtrl->Scan11bOceAPTimerRunning = FALSE;
}

DECLARE_TIMER_FUNCTION(OceResetScanIndication);
BUILD_TIMER_FUNCTION(OceResetScanIndication);

static VOID OceSetMaxChannelTimesUp(PVOID SystemSpecific1,
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

static VOID OceFdFrameSending(PVOID SystemSpecific1, PVOID FunctionContext,
			      PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	BOOLEAN TimerCancelled = FALSE;
	UCHAR FdFrameTimes = 0;

	pAd->ApCfg.FdFrameTimerRunning = FALSE;
	pAd->ApCfg.FdFrameCurNum++;

	FdFrameTimes = (pAd->CommonCfg.BeaconPeriod / OCE_FD_FRAME_PERIOD) - 1;

	if (pAd->ApCfg.FdFrameCurNum >= FdFrameTimes) {
		RTMPCancelTimer(&pAd->ApCfg.FdFrameTimer, &TimerCancelled);
	} else {
		UCHAR intval = OCE_FD_FRAME_PERIOD + 2;

		if (pAd->ApCfg.FdFrameCurNum == (FdFrameTimes - 1))
			intval = OCE_FD_FRAME_PERIOD + 3;
		RTMPSetTimer(&pAd->ApCfg.FdFrameTimer, intval);
		pAd->ApCfg.FdFrameTimerRunning = TRUE;
	}

	OceSendFilsDiscoveryAction(pAd, wdev);
}

DECLARE_TIMER_FUNCTION(OceFdFrameSending);
BUILD_TIMER_FUNCTION(OceFdFrameSending);

static VOID OceAPAutoScanNR(PVOID SystemSpecific1, PVOID FunctionContext,
			    PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	NDIS_802_11_SSID Ssid;

	if (ScanRunning(pAd) == FALSE) {
		NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
		NdisMoveMemory(Ssid.Ssid, "1", strlen("1"));
		Ssid.SsidLength = strlen("1");
		ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, wdev);
	}
}

DECLARE_TIMER_FUNCTION(OceAPAutoScanNR);
BUILD_TIMER_FUNCTION(OceAPAutoScanNR);

OCE_ERR_CODE OceInit(PRTMP_ADAPTER pAd)
{
	INT loop = 0;
	P_OCE_CTRL pOceCtrl = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;
	struct wifi_dev *pWdev = NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_AP_SUPPORT
	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
		pWdev = &pAd->ApCfg.MBSSID[loop].wdev;
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
		NdisZeroMemory(pOceCtrl, sizeof(OCE_CTRL));
		OCE_SET_CONTROL_FIELD(pOceCtrl->OceCapIndication, OCE_RELEASE,
				      OCE_RELEASE_MASK, OCE_RELEASE_OFFSET)
		/* OCE release = 1 */;
		OCE_SET_CONTROL_FIELD(
			pOceCtrl->OceCapIndication, IS_STA_CFON,
			OCE_IS_STA_CFON_MASK,
			OCE_IS_STA_CFON_OFFSET); /* IS not STA CFON */

		if (pOceCtrl->bOceFilsHlpEnable == TRUE)
			OCE_SET_CONTROL_FIELD(
				pOceCtrl->OceCapIndication, HLP_ENABLED,
				OCE_HLP_ENABLED_MASK,
				OCE_HLP_ENABLED_OFFSET); /* HLP_ENABLED */

		pOceCtrl->bApReducedWanEnable = FALSE;
		pOceCtrl->bApRnrCompleteEnable = FALSE;
		pOceCtrl->bApEspEnable = FALSE;
		pOceCtrl->ShortSSIDEnabled = TRUE;
		RTMPInitTimer(pAd, &pOceCtrl->Scan11bOceAPTimer,
			      GET_TIMER_FUNCTION(OceResetScanIndication), pWdev,
			      TRUE);
		pOceCtrl->Scan11bOceAPTimerRunning = FALSE;
		RTMPInitTimer(pAd, &pOceCtrl->MaxChannelTimer,
			      GET_TIMER_FUNCTION(OceSetMaxChannelTimesUp),
			      pWdev, FALSE);
		pOceCtrl->MaxChannelTimerRunning = FALSE;
	}

	if (IS_FD_FRAME_FW_MODE(cap)) {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s - FdFrame using FW Mode\n", __func__));
	} else {
		/* Need TBTT source in host */
		RTMPInitTimer(pAd, &pAd->ApCfg.FdFrameTimer,
			      GET_TIMER_FUNCTION(OceFdFrameSending), pAd,
			      FALSE);
		pAd->ApCfg.FdFrameTimerRunning = FALSE;
	}

	RTMPInitTimer(pAd, &pAd->ApCfg.APAutoScanNeighborTimer,
		      GET_TIMER_FUNCTION(OceAPAutoScanNR), pAd, TRUE);
#endif /* CONFIG_AP_SUPPORT */

	return OCE_SUCCESS;
}

OCE_ERR_CODE OceDeInit(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	struct _RTMP_CHIP_CAP *cap = NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (IS_FD_FRAME_FW_MODE(cap)) {
		HW_SET_FD_FRAME_OFFLOAD(pAd, wdev->wdev_idx, 0, FALSE, 0, NULL);
	} else {
		BOOLEAN Cancelled;

		RTMPCancelTimer(&pAd->ApCfg.FdFrameTimer, &Cancelled);
	}

	return OCE_SUCCESS;
}

OCE_ERR_CODE OceRelease(PRTMP_ADAPTER pAd)
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

	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
		RTMPCancelTimer(&pOceCtrl->Scan11bOceAPTimer, &Cancelled);
		RTMPReleaseTimer(&pOceCtrl->Scan11bOceAPTimer, &Cancelled);
		RTMPCancelTimer(&pOceCtrl->MaxChannelTimer, &Cancelled);
		RTMPReleaseTimer(&pOceCtrl->MaxChannelTimer, &Cancelled);
	}
	return OCE_SUCCESS;
}

OCE_ERR_CODE OceInsertAttrById(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry,
			       PUINT8 pAttrTotalLen, PUINT8 pAttrBuf,
			       UINT8 AttrId)
{
	P_OCE_CTRL pOceCtrl = NULL;
	OCE_ATTR_STRUCT OceAttr;
	PUINT8 pAttrBufOffset = (pAttrBuf + *pAttrTotalLen);
	UINT16 OverflowChk = 0;
	BSS_STRUCT *pMbss = NULL;
	UINT16 i;
	ULONG ReadOffset = 0;

	if (wdev) {
		pOceCtrl = &wdev->OceCtrl;
	} else {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s - Mbss is NULL !!!!!\n", __func__));
		return OCE_INVALID_ARG;
	}

	if (!VALID_OCE_ATTR_ID(AttrId)) {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s %d - Invalid attr Id [%d] !!!!!\n", __func__,
			  __LINE__, AttrId));
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
		if (pMbss && pMbss->ReducedNRListExist &&
		    (pMbss->ReducedNRListInfo.ValueLen)) {
			OceAttr.AttrLen = pMbss->ReducedNRListInfo.ValueLen /
					  OCE_RNR_IE_LEN;

			for (i = 0; i < OceAttr.AttrLen; i++) {
				/* 13 = OCE_RNR_IE_LEN - OCE_SHORT_SSID_LEN*/
				ReadOffset += 13;
				NdisMoveMemory(
					&OceAttr.AttrBody[i *
							  OCE_SHORT_SSID_LEN],
					pMbss->ReducedNRListInfo.Value +
						ReadOffset,
					OCE_SHORT_SSID_LEN);
				ReadOffset += OCE_SHORT_SSID_LEN;
			}
			OceAttr.AttrLen *= OCE_SHORT_SSID_LEN;
		} else
			return OCE_SUCCESS;

		break;
#endif

	default:
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s - UNKNOWN Oce AttrId [%d] !!!!!\n", __func__,
			  AttrId));
	}
	OverflowChk = *pAttrTotalLen + OceAttr.AttrLen + 2;
	if (OverflowChk >= OCE_ATTR_MAX_LEN) {
		MTWF_LOG(
			DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s - AttrTotalLen %d Overflow, should be below %d !!!!!\n",
			 __func__, OverflowChk, OCE_ATTR_MAX_LEN));
		return OCE_UNEXP;
	}
	/* safe, insert the attribute */
	NdisCopyMemory(pAttrBufOffset, &OceAttr, OceAttr.AttrLen + 2);
	*pAttrTotalLen += (OceAttr.AttrLen + 2);

	return OCE_SUCCESS;
}

OCE_ERR_CODE OceCollectAttribute(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
				 MAC_TABLE_ENTRY *pEntry, PUINT8 pAttrLen,
				 PUINT8 pAttrBuf, UINT8 FrameType)
{
	UINT8 ErrCode = OCE_SUCCESS;
	P_OCE_CTRL pOceCtrl = NULL;

	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("%s - collect oce attr for FrameType %d\n", __func__,
		  FrameType));

	if (!pAd || !wdev || !pAttrLen || !pAttrBuf) {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s - Invalid input argument !!!!!\n", __func__));
		return OCE_INVALID_ARG;
	}

	pOceCtrl = &wdev->OceCtrl;

	switch (FrameType) {
#ifdef CONFIG_AP_SUPPORT
	case OCE_FRAME_TYPE_BEACON:
	case OCE_FRAME_TYPE_PROBE_RSP:
		ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen, pAttrBuf,
					    OCE_ATTR_CAP_INDCATION);
		if (pOceCtrl->bApReducedWanEnable)
			ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen,
						    pAttrBuf,
						    OCE_ATTR_AP_REDUCED_WAN);
		if (IS_OCE_RNR_ENABLE(wdev))
			ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen,
						    pAttrBuf,
						    OCE_ATTR_AP_RNR_COMPLETE);
		break;
	case OCE_FRAME_TYPE_ASSOC_RSP:
		ErrCode = OceInsertAttrById(wdev, NULL, pAttrLen, pAttrBuf,
					    OCE_ATTR_CAP_INDCATION);

		if (pEntry->oceInfo.DeltaAssocRSSI)
			ErrCode = OceInsertAttrById(wdev, pEntry, pAttrLen,
						    pAttrBuf,
						    OCE_ATTR_AP_RSSI_REJCTION);
		break;
#endif /* CONFIG_AP_SUPPORT */

	default:
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s - UNKNOWN FrameType %d !!!!!\n", __func__,
			  FrameType));
		return OCE_UNEXP;
	}
	return ErrCode;
}

VOID OceParseStaOceIE(PRTMP_ADAPTER pAd, UCHAR *buf, UCHAR len,
		      PEER_PROBE_REQ_PARAM *ProbeReqParam)
{
	UCHAR *pos = NULL;
	UCHAR ParsedLen = 0;
	UCHAR Bssid_offset = 0;
	UCHAR apidx = MAIN_MBSSID;
	PEID_STRUCT eid_ptr;
	UINT32 crc32; /* 32-bit CRC value */
	NDIS_802_11_MAC_ADDRESS OceSuppresBssid;
	UCHAR OceSuppresSsid[OCE_SHORT_SSID_LEN];

	pos = buf;
	/* skip OUI 4 bytes */
	pos += 4;
	ParsedLen += 4;

	eid_ptr = (PEID_STRUCT)pos;

	while (ParsedLen <= len) {
		switch (eid_ptr->Eid) {
		case OCE_ATTR_CAP_INDCATION:
			ProbeReqParam->IsOceCapability = TRUE;
			break; /* is OCE STA*/
		case OCE_ATTR_STA_PRB_SUP_BSSID:
			if (eid_ptr->Len == 0) {
				for (apidx = 0; apidx < pAd->ApCfg.BssidNum;
				     apidx++)
					ProbeReqParam->bProbeSupp[apidx] = TRUE;
				break;
			}

			for (Bssid_offset = 0; Bssid_offset < eid_ptr->Len;
			     Bssid_offset += 6) {
				NdisMoveMemory(OceSuppresBssid,
					       eid_ptr->Octet + Bssid_offset,
					       6);
				if (MAC_ADDR_EQUAL(OceSuppresBssid,
						   BROADCAST_ADDR)) {
					for (apidx = 0;
					     apidx < pAd->ApCfg.BssidNum;
					     apidx++)
						ProbeReqParam
							->bProbeSupp[apidx] =
							TRUE;
					break;
				}

				for (apidx = 0; apidx < pAd->ApCfg.BssidNum;
				     apidx++) {
					if (MAC_ADDR_EQUAL(
						    pAd->wdev_list[apidx]->bssid,
						    OceSuppresBssid))
						ProbeReqParam
							->bProbeSupp[apidx] =
							TRUE;
				}
			}
			break;
		case OCE_ATTR_STA_PRB_SUP_SSID:
			if (eid_ptr->Len == 0) {
				for (apidx = 0; apidx < pAd->ApCfg.BssidNum;
				     apidx++)
					ProbeReqParam->bProbeSupp[apidx] = TRUE;
				break;
			}

			for (Bssid_offset = 0; Bssid_offset < eid_ptr->Len;
			     Bssid_offset += 4) {
				for (apidx = 0; apidx < pAd->ApCfg.BssidNum;
				     apidx++) {
					NdisCopyMemory(OceSuppresSsid,
						       eid_ptr->Octet +
							       Bssid_offset,
						       4);
					crc32 = Crcbitbybitfast(
						pAd->ApCfg.MBSSID[apidx].Ssid,
						pAd->ApCfg.MBSSID[apidx]
							.SsidLen);
					if (NdisEqualMemory(&crc32,
							    eid_ptr->Octet +
								    Bssid_offset,
							    4))
						ProbeReqParam
							->bProbeSupp[apidx] =
							TRUE;
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

void Oce_read_parameters_from_file(IN PRTMP_ADAPTER pAd, RTMP_STRING *tmpbuf,
				   RTMP_STRING *pBuffer)
{
	struct wifi_dev *wdev = NULL;
	OCE_CTRL *oceCtrl = NULL;
	INT Loop = 0;

	if (RTMPGetKeyParameter("OCE_SUPPORT", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0)
				oceCtrl->bOceEnable = TRUE;
			else
				oceCtrl->bOceEnable = FALSE;

			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("I/F(ra%d) OCE_SUPPORT=%d\n", Loop,
				  oceCtrl->bOceEnable));
		}
	}

	if (RTMPGetKeyParameter("OCE_FD_FRAME", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0)
				oceCtrl->bFdFrameEnable = TRUE;
			else
				oceCtrl->bFdFrameEnable = FALSE;

			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("I/F(ra%d) OCE_FD_FRAME=%d\n", Loop,
				  oceCtrl->bFdFrameEnable));
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_HLP", tmpbuf, 32, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0) {
				oceCtrl->bOceFilsHlpEnable = TRUE;
				OCE_SET_CONTROL_FIELD(
					oceCtrl->OceCapIndication, HLP_ENABLED,
					OCE_HLP_ENABLED_MASK,
					OCE_HLP_ENABLED_OFFSET); /* HLP_ENABLED */
			} else
				oceCtrl->bOceFilsHlpEnable = FALSE;

			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("I/F(ra%d) OCE_FILS_HLP=%d\n", Loop,
				  oceCtrl->bOceEnable));
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_DhcpServer", tmpbuf, 32, pBuffer,
				TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), Loop++) {
			UINT32 ip_addr;

			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (rtinet_aton(macptr, &ip_addr)) {
				oceCtrl->FilsDhcpServerIp = ip_addr;
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL,
					 DBG_LVL_TRACE,
					 ("OCE_FILS_DhcpServer=%s(%x)\n",
					  macptr, oceCtrl->FilsDhcpServerIp));
			}
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_DhcpServerPort", tmpbuf, 32, pBuffer,
				TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			oceCtrl->FilsDhcpServerPort = os_str_tol(macptr, 0, 10);

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_TRACE,
				 ("I/F(ra%d) OCE_FILS_DhcpServerPort=%d\n",
				  Loop, oceCtrl->FilsDhcpServerPort));
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_REALMS", tmpbuf, 256, pBuffer,
				TRUE)) {
		RTMP_STRING *macptr;
		UCHAR result[SHA256_BLOCK_SIZE] = { 0 }, i = 0;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			for (i = 0; i < strlen(macptr); i++) {
				macptr[i] = tolower(macptr[i]);
			}

			RT_SHA256(macptr, strlen(macptr), result);
			NdisMoveMemory(&oceCtrl->FilsRealmsHash, result,
				       FILS_REALMS_HASH_LEN);

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_OFF,
				 ("%s: OCE_FILS_Realms%d (%02x:%02x)\n",
				  __func__, (Loop + 1), result[0], result[1]));
		}
	}

	if (RTMPGetKeyParameter("OCE_FILS_CACHE", tmpbuf, 256, pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			if (os_str_tol(macptr, 0, 10) != 0)
				oceCtrl->FilsCacheId =
					RandomByte(pAd) << 8 | RandomByte(pAd);
			else
				oceCtrl->FilsCacheId = 0;

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_OFF,
				 ("%s: OCE_FILS_CACHE%d (%04x)\n", __func__,
				  (Loop + 1), oceCtrl->FilsCacheId));
		}
	}

	if (RTMPGetKeyParameter("OCE_RNR_SUPPORT", tmpbuf, MAX_PARAMETER_LEN,
				pBuffer, TRUE)) {
		RTMP_STRING *macptr;

		for (Loop = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), Loop++) {
			wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
			oceCtrl = &wdev->OceCtrl;

			if (Loop >= MAX_MBSSID_NUM(pAd))
				break;

			oceCtrl->bApRnrCompleteEnable =
				(UINT8)os_str_tol(macptr, 0, 10) ? TRUE : FALSE;

			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s::(bApRnrCompleteEnable[%d]=%d)\n",
				  __func__, Loop,
				  oceCtrl->bApRnrCompleteEnable));
		}
	}

	for (Loop = 0; Loop < MAX_MBSSID_NUM(pAd); Loop++) {
		RTMP_STRING tok_str[16];
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[Loop].wdev;
		OCE_CTRL *oceCtrl = &wdev->OceCtrl;

		/*
			OCE_ASSOC_RssiThres:
		*/
		NdisZeroMemory(tok_str, sizeof(tok_str));
		snprintf(tok_str, sizeof(tok_str), "OCE_ASSOC_RssiThres%d",
			 Loop + 1);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 32, pBuffer, TRUE)) {
			INT8 rssi = os_str_tol(tmpbuf, 0, 10);

			oceCtrl->AssocRSSIThres = rssi;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_TRACE,
				 ("%s: OCE_ASSOC_RssiThres%d (%d)\n", __func__,
				  (Loop + 1), oceCtrl->AssocRSSIThres));
		}

		/*
			OCE_ASSOC_RetryDelay:
		*/
		NdisZeroMemory(tok_str, sizeof(tok_str));
		snprintf(tok_str, sizeof(tok_str), "OCE_ASSOC_RetryDelay%d",
			 Loop + 1);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 32, pBuffer, TRUE)) {
			UINT8 time = os_str_tol(tmpbuf, 0, 10);

			oceCtrl->AssocRetryDelay = time;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_OCE, DBG_LVL_TRACE,
				 ("%s: OCE_ASSOC_RetryDelay%d (%d)\n", __func__,
				  (Loop + 1), oceCtrl->AssocRSSIThres));
		}
	}
}

BOOLEAN OceCheckOceCap(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR *buf,
		       UCHAR len)
{
	UCHAR *pos = NULL;
	UCHAR ParsedLen = 0;
	PEID_STRUCT eid_ptr;
	P_OCE_CTRL pOceCtrl = NULL;
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
			break; /* is OCE STA*/
		default:
			break;
		}
		ParsedLen += (2 + eid_ptr->Len);
		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}
	return hasOceCap;
}

/* build reduced neighbor report element */
INT build_rnr_element(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf,
		      UINT8 pos, UCHAR subtype)
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
			RemainBeaconLen = 1520 - cap->tx_hw_hdr_len -
					  pos; /*FW limitation*/
		else
			RemainBeaconLen = 512 - cap->tx_hw_hdr_len - pos;
#else
		RemainBeaconLen = 512 - cap->tx_hw_hdr_len - pos;
#endif
		if (pMbss && pMbss->ReducedNRListExist &&
		    (pMbss->ReducedNRListInfo.ValueLen)) {
			if (pMbss->ReducedNRListInfo.ValueLen > RemainBeaconLen)
				pMbss->ReducedNRListInfo.ValueLen =
					RemainBeaconLen;
		}
	}

	if (pMbss && pMbss->ReducedNRListExist &&
	    (pMbss->ReducedNRListInfo.ValueLen))
		MakeOutgoingFrame(buf, &ReducedNRListTmpLen,
				  pMbss->ReducedNRListInfo.ValueLen,
				  pMbss->ReducedNRListInfo.Value, END_OF_ARGS);

	return ReducedNRListTmpLen;
}
/* build estimated service parameters element */
INT build_esp_element(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	UINT i, CurrTxop, ScanTxop = 0;
	BSS_TABLE *ScanTab = &pAd->ScanTab;
	P_OCE_CTRL pOceCtrl = NULL;
	ULONG TmpLen;
	UCHAR operating_ie = IE_WLAN_EXTENSION, ext_ie = IE_EXTENSION_ID_ESP,
	      operating_len = 4;
	ESP_INFO EspInfo;
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
#endif /*AP_QLOAD_SUPPORT*/

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
#ifdef AP_QLOAD_SUPPORT
	if (pQloadCtrl->QloadChanUtilCcaNavTx < 229)
		EspInfo.field.EstimatedAirTimeFraction =
			255 - pQloadCtrl->QloadChanUtilCcaNavTx;
	else if ((pQloadCtrl->QloadChanUtilCcaNavTx > 229) &&
		 (pQloadCtrl->QloadChanUtil < 26))
		EspInfo.field.EstimatedAirTimeFraction =
			pQloadCtrl->QloadChanUtilCcaNavTx / 2;
	else {
		CurrTxop = pAd->CurrEdcaParam[WMM_PARAM_AC_1].u2Txop;
		CurrTxop = (!CurrTxop) ?
				   2000 :
					 CurrTxop * 32; /* 32 us per txop unit */
		for (i = 0; i < ScanTab->BssNr &&
			    ScanTab->BssNr < MAX_LEN_OF_BSS_TABLE;
		     i++)
			ScanTxop +=
				ScanTab->BssEntry[i].EdcaParm.Txop[QID_AC_BK];

		ScanTxop *= 32; /* 32 us per txop unit */
		/* corresponding access category in units of 50 us */
		EspInfo.field.EstimatedAirTimeFraction =
			(255 - ((pQloadCtrl->QloadChanUtilCcaNavTx -
				 pQloadCtrl->QloadChanUtil) *
				((CurrTxop + ScanTxop) / CurrTxop))) *
			((EspInfo.field.DataPPDUDurationTarget * 50) /
			 ((EspInfo.field.DataPPDUDurationTarget * 50) +
			  ScanTxop));
	}
#endif /*AP_QLOAD_SUPPORT*/
	MakeOutgoingFrame(buf, &TmpLen, 1, &operating_ie, 1, &operating_len, 1,
			  &ext_ie, 3, &EspInfo, END_OF_ARGS);

	return TmpLen;
}

static unsigned long Reflect(unsigned long crc, int bitnum)
{
	unsigned long i, j = 1, crcout = 0;

	for (i = (unsigned long)1 << (bitnum - 1); i; i >>= 1) {
		if (crc & i)
			crcout |= j;
		j <<= 1;
	}
	return crcout;
}

unsigned long Crcbitbybitfast(unsigned char *p, unsigned long len)
{
	unsigned long i, j, c, bit, crcmask, crchighbit;
	unsigned long crc = 0xffffffff;
	const int order = 32;

	crcmask = ((((unsigned long)1 << (order - 1)) - 1) << 1) | 1;
	crchighbit = (unsigned long)1 << (order - 1);
	for (i = 0; i < len; i++) {
		c = (unsigned long)*p++;
		c = Reflect(c, 8);

		for (j = 0x80; j; j >>= 1) {
			bit = crc & crchighbit;
			crc <<= 1;
			if (c & j)
				bit ^= crchighbit;
			if (bit)
				crc ^= POLYNOMIAL;
		}
	}

	crc = Reflect(crc, order);
	crc ^= 0xffffffff;
	crc &= crcmask;

	return crc;
}

INT Set_OceRssiThreshold_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	CHAR rssi;
	OCE_CTRL *oceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	oceCtrl = &wdev->OceCtrl;

	rssi = (CHAR)os_str_tol(arg, 0, 10);
	pAd->ApCfg.MBSSID[apidx].AssocReqRssiThreshold = rssi;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s::(RssiThreshold=%d)\n", __func__,
		  pAd->ApCfg.MBSSID[apidx].AssocReqRssiThreshold));

	return TRUE;
}

INT Set_OceAssocRetryDelay_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT time;
	OCE_CTRL *oceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	oceCtrl = &wdev->OceCtrl;

	time = (CHAR)os_str_tol(arg, 0, 10);
	oceCtrl->AssocRetryDelay = time;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s::(AssocRetryDelay=%d)\n", __func__,
		  oceCtrl->AssocRetryDelay));

	return TRUE;
}

INT Set_OceFdFrameCtrl_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	OCE_CTRL *oceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
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
			HW_SET_FD_FRAME_OFFLOAD(pAd, wdev->wdev_idx, 0, FALSE,
						0, NULL);
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s::(FdFrameCtrl=%d)\n", __func__, enable));

	return TRUE;
}

/*	format : iwpriv [interface] set oce_reduced_nr=[append]-[nr_entry_num]
*	sample : iwpriv ra0 set oce_reduced_nr=0-7
*			==> renew list,not append,indicate 7 entries
*/

INT Set_OceReducedNRIndicate_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT8 i = 0, input = 0;
	RTMP_STRING *macptr;
	BOOLEAN AppendMode = FALSE;
	BOOLEAN Cancelled;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *pWdev = get_wdev_by_ioctl_idx_and_iftype(
		pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	P_OCE_CTRL pOceCtrl = &pWdev->OceCtrl;

	for (i = 0, macptr = rstrtok(arg, "-"); macptr;
	     macptr = rstrtok(NULL, "-"), i++) {
		if (i == 0)
			input = (UINT8)simple_strtol(macptr, 0, 10);
		else
			break;
	}
	AppendMode = (input) ? TRUE : FALSE;

	pOceCtrl->bApRnrCompleteEnable = (input) ? TRUE : FALSE;

	if (IS_OCE_RNR_ENABLE(pWdev))
		RTMPSetTimer(&pAd->ApCfg.APAutoScanNeighborTimer,
			     OCE_RNR_SCAN_PERIOD);
	else
		RTMPCancelTimer(&pAd->ApCfg.APAutoScanNeighborTimer,
				&Cancelled);

	return TRUE;
}

INT Set_OceDownlinkAvailCap_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT loop;
	P_OCE_CTRL pOceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
		if (pOceCtrl && pOceCtrl->bOceEnable)
			OCE_SET_DOWNLINK_AVAILABLE_CAP(
				pOceCtrl->AvailableCap,
				simple_strtol(arg, 0, 10),
				OCE_AVAILABLE_CAP_MASK,
				OCE_AVAILABLE_CAP_OFFSET);
	}
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

	return TRUE;
}

INT Set_OceUplinkAvailCap_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT loop;
	P_OCE_CTRL pOceCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
		pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
		if (pOceCtrl && pOceCtrl->bOceEnable)
			OCE_SET_UPLINK_AVAILABLE_CAP(pOceCtrl->AvailableCap,
						     simple_strtol(arg, 0, 10),
						     OCE_AVAILABLE_CAP_MASK,
						     OCE_AVAILABLE_CAP_OFFSET);
	}
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

	return TRUE;
}

INT Set_OceReducedWanEnable_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT loop = 0;
	P_OCE_CTRL pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
	BOOLEAN enable;

	enable = (BOOLEAN)simple_strtol(arg, 0, 10);
	pOceCtrl->bApReducedWanEnable = enable;

	return enable;
}

INT Set_OceEspEnable_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT loop = 0;
	P_OCE_CTRL pOceCtrl = &pAd->ApCfg.MBSSID[loop].wdev.OceCtrl;
	BOOLEAN enable;

	enable = (BOOLEAN)simple_strtol(arg, 0, 10);

	pOceCtrl->bApEspEnable = enable;

	return enable;
}

#endif /* OCE_SUPPORT */
