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
    tdls.h

    Abstract:

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

#ifdef DOT11Z_TDLS_SUPPORT

#include <linux/timer.h>
#include "rt_config.h"

/*
 *    ==========================================================================
 *    Description:
 *	tdls state machine init, including state transition and timer init
 *    Parameters:
 *	Sm - pointer to the dls state machine
 *    Note:
 *	The state machine looks like this
 *
 *			    TDLS_IDLE
 *
 *	IRQL = PASSIVE_LEVEL
 *
 *    ==========================================================================
 */
VOID TDLS_ChSwStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;
	PRT_802_11_TDLS	pTDLS = NULL;

	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, (ULONG)MAX_TDLS_STATE,
					 (ULONG)MAX_TDLS_CHSW_MSG, (STATE_MACHINE_FUNC)Drop, TDLS_IDLE, TDLS_CHSW_MACHINE_BASE);
	/* the first column */
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_CH_SWITCH_REQ, (STATE_MACHINE_FUNC)TDLS_MlmeChannelSwitchAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_CH_SWITCH_RSP, (STATE_MACHINE_FUNC)TDLS_MlmeChannelSwitchRspAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_CH_SWITCH_REQ, (STATE_MACHINE_FUNC)TDLS_PeerChannelSwitchReqAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_CH_SWITCH_RSP, (STATE_MACHINE_FUNC)TDLS_PeerChannelSwitchRspAction);

	for (i = 0; i < MAX_NUMBER_OF_DLS_ENTRY; i++) {
		pAd->StaCfg[0].TdlsInfo.TDLSEntry[i].pAd = pAd;
		pTDLS = &pAd->StaCfg[0].TdlsInfo.TDLSEntry[i];
		RTMPInitTimer(pAd, &pTDLS->ChannelSwitchTimer,
					  GET_TIMER_FUNCTION(TDLS_ChannelSwitchTimeAction), pTDLS, FALSE);
		RTMPInitTimer(pAd, &pTDLS->ChannelSwitchTimeoutTimer,
					  GET_TIMER_FUNCTION(TDLS_ChannelSwitchTimeOutAction), pTDLS, FALSE);
	}

	RTMPInitTimer(pAd, &pAd->StaCfg[0].TdlsInfo.TdlsDisableChannelSwitchTimer,
				  GET_TIMER_FUNCTION(TDLS_DisablePeriodChannelSwitchAction), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->StaCfg[0].TdlsInfo.TdlsPeriodGoOffChTimer,
				  GET_TIMER_FUNCTION(TDLS_BaseChExpired), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->StaCfg[0].TdlsInfo.TdlsPeriodGoBackBaseChTimer,
				  GET_TIMER_FUNCTION(TDLS_OffChExpired), pAd, FALSE);
}

/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
VOID
TDLS_BuildChannelSwitchRequest(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pPeerAddr,
	IN USHORT ChSwitchTime,
	IN USHORT ChSwitchTimeOut,
	IN UCHAR TargetChannel,
	IN UCHAR TargetChannelBW)
{
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg[0].TdlsInfo;
	PRT_802_11_TDLS	pTDLS = NULL;
	INT LinkId = 0xff;
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST);
	/* Target Channel */
	TDLS_InsertTargetChannel(pAd, (pFrameBuf + *pFrameLen), pFrameLen, TargetChannel);
	/* Regulatory Class */
	TDLS_InsertRegulatoryClass(pAd, (pFrameBuf + *pFrameLen), pFrameLen, TargetChannel, TargetChannelBW);

	/* Secondary Channel Offset */
	if (TargetChannelBW != EXTCHA_NONE) {
		if (TargetChannel > 14) {
			if ((TargetChannel == 36) || (TargetChannel == 44) || (TargetChannel == 52) ||
				(TargetChannel == 60) || (TargetChannel == 100) || (TargetChannel == 108) ||
				(TargetChannel == 116) || (TargetChannel == 124) || (TargetChannel == 132) ||
				(TargetChannel == 149) || (TargetChannel == 157)) {
				TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, EXTCHA_ABOVE);
				pTdlsControl->TdlsDesireChannelBW = EXTCHA_ABOVE;
			} else if ((TargetChannel == 40) || (TargetChannel == 48) || (TargetChannel == 56) |
					   (TargetChannel == 64) || (TargetChannel == 104) || (TargetChannel == 112) ||
					   (TargetChannel == 120) || (TargetChannel == 128) || (TargetChannel == 136) ||
					   (TargetChannel == 153) || (TargetChannel == 161)) {
				TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, EXTCHA_BELOW);
				pTdlsControl->TdlsDesireChannelBW = EXTCHA_BELOW;
			}
		} else {
			UCHAR ExtCh;
			UCHAR Dir = TargetChannelBW;

			ExtCh = TDLS_GetExtCh(TargetChannel, Dir);

			if (TDLS_IsValidChannel(pAd, ExtCh)) {
				TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, Dir);
				pTdlsControl->TdlsDesireChannelBW = Dir;
			} else {
				Dir = (Dir == EXTCHA_ABOVE) ? EXTCHA_BELOW : EXTCHA_ABOVE;
				ExtCh = TDLS_GetExtCh(TargetChannel, Dir);

				if (TDLS_IsValidChannel(pAd, ExtCh)) {
					TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, Dir);
					pTdlsControl->TdlsDesireChannelBW = Dir;
				}
			}
		}
	} else
		pTdlsControl->TdlsDesireChannelBW = EXTCHA_NONE;

	/* fill link identifier */
	LinkId = TDLS_SearchLinkId(pAd, pPeerAddr);

	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - can not find the LinkId!\n", __func__));
		return;
	}

	pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg[0].TdlsInfo.TDLSEntry[LinkId];

	if (pTDLS->bInitiator)
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pPeerAddr, pAd->CurrentAddress);
	else
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pPeerAddr);

	/* Channel Switch Timing */
	TDLS_InsertChannelSwitchTimingIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ChSwitchTime, ChSwitchTimeOut);
}

/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
VOID
TDLS_BuildChannelSwitchResponse(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	USHORT	ChSwitchTime,
	IN	USHORT	ChSwitchTimeOut,
	IN	UINT16	ReasonCode)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE);
	/* fill reason code */
	TDLS_InsertReasonCode(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ReasonCode);

	/* fill link identifier */
	if (pTDLS->bInitiator)
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->MacAddr, pAd->CurrentAddress);
	else
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pTDLS->MacAddr);

	/* Channel Switch Timing */
	TDLS_InsertChannelSwitchTimingIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ChSwitchTime, ChSwitchTimeOut);
}

/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
NDIS_STATUS
TDLS_ChannelSwitchReqAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS pTDLS,
	IN PUCHAR pPeerAddr,
	IN UCHAR TargetChannel,
	IN UCHAR TargetChannelBW)
{
	UCHAR TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR Header802_3[14];
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	ULONG TempLen;
	UCHAR RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	MAC_TABLE_ENTRY *pEntry = NULL;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %s()\n", __func__);
	MAKE_802_3_HEADER(Header802_3, pPeerAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);
	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus	!= NDIS_STATUS_SUCCESS)
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
					  1,				&RemoteFrameType,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;
	TDLS_BuildChannelSwitchRequest(pAd,
								   pOutBuffer,
								   &FrameLen,
								   pPeerAddr,
								   TDLS_CHANNEL_SWITCH_TIME,
								   TDLS_CHANNEL_SWITCH_TIMEOUT,
								   TargetChannel,
								   TargetChannelBW);
	pEntry = MacTableLookup(pAd, pPeerAddr);

	if (pEntry && IS_ENTRY_TDLS(pEntry)) {
		pTDLS->ChannelSwitchCurrentState = TDLS_CHANNEL_SWITCH_WAIT_RSP;
		TDLS_SendChannelSwitchActionFrame(pAd,
										  pEntry,
										  Header802_3,
										  LENGTH_802_3,
										  pOutBuffer,
										  (UINT)FrameLen,
										  FALSE);
	} else
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Can't find TDLS entry on Mac TABLE !!!!\n"));

	hex_dump("TDLS switch channel request send pack", pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=== %s()\n", __func__);
	return NStatus;
}

/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
NDIS_STATUS
TDLS_ChannelSwitchRspAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	USHORT	ChSwitchTime,
	IN	USHORT	ChSwitchTimeOut,
	IN	UINT16	StatusCode,
	IN	UCHAR	FrameType)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %s()\n", __func__);
	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);
	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus	!= NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ACT - %s() allocate memory failed\n", __func__));
		return NStatus;
	}

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
					  1,				&RemoteFrameType,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;
	TDLS_BuildChannelSwitchResponse(pAd,
									pOutBuffer,
									&FrameLen,
									pTDLS,
									ChSwitchTime,
									ChSwitchTimeOut,
									StatusCode);
	TDLS_SendChannelSwitchActionFrame(pAd,
									  &pAd->MacTab.Content[pTDLS->MacTabMatchWCID],
									  Header802_3,
									  LENGTH_802_3,
									  pOutBuffer,
									  (UINT)FrameLen,
									  FrameType);
	hex_dump("TDLS send channel switch response pack", pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=== %s()\n", __func__);
	return NStatus;
}

VOID
TDLS_TriggerChannelSwitchAction(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR DtimCount)
{
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg[0].TdlsInfo;
	PRT_802_11_TDLS	pTdlsEntry = NULL;
	BOOLEAN bCanDoChSwitch = FALSE;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	INT	LinkId = 0xff;

	RTMP_SEM_LOCK(&pTdlsControl->TdlsChSwLock);

	if (INFRA_ON(pAd) &&
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
		(pTdlsControl->bDoingPeriodChannelSwitch) &&
		(pTdlsControl->bChannelSwitchInitiator))
		bCanDoChSwitch = TRUE;

	RTMP_SEM_UNLOCK(&pTdlsControl->TdlsChSwLock);

	if (pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited == TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - AP Prohibite TDLS Channel Switch !!!\n", __func__));
		return;
	}

	if (bCanDoChSwitch) {
		if (DtimCount == 0) {
			BOOLEAN TimerCancelled;
			ULONG Now, Now1, Now2;

			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("101. %ld !!!\n", (jiffies * 1000) / OS_HZ));
			NdisGetSystemUpTime(&Now);

			if (pTdlsControl->TdlsGoBackStartTime == 0)
				Now2 = (pTdlsControl->TdlsActiveSwitchTime / 1000);
			else {
				if (pTdlsControl->TdlsGoBackStartTime > Now)
					Now2 = (pTdlsControl->TdlsActiveSwitchTime / 1000);
				else {
					Now1 = Now - pTdlsControl->TdlsGoBackStartTime;
					Now2 = ((Now1 * 1000) / OS_HZ);
				}
			}

			/* Drop not within my TDLS Table that created before ! */
			LinkId = TDLS_SearchLinkId(pAd, pTdlsControl->TdlsDesireChSwMacAddr);

			if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY) {
				RTMPCancelTimer(&pTdlsControl->TdlsPeriodGoBackBaseChTimer, &TimerCancelled);
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - can not find the LinkId!\n", __func__));
				return;
			}

			/* Point to the current Link ID */
			pTdlsEntry = &pAd->StaCfg[0].TdlsInfo.TDLSEntry[LinkId];

			if (pTdlsEntry->Valid && pTdlsEntry->bDoingPeriodChannelSwitch) {
				pTdlsControl->TdlsForcePowerSaveWithAP = TRUE;

				if (Now2 >= (pTdlsControl->TdlsActiveSwitchTime / 1000))
					RtmpOsMsDelay(9);
				else
					RtmpOsMsDelay(((pTdlsControl->TdlsActiveSwitchTime / 1000) - Now2) + 9);

#if defined(RTMP_MAC) || defined(RLT_MAC)
				/* TODO: shiang-usw, need abstraction for this function! */
				TDLS_KickOutHwNullFrame(pAd, PWR_SAVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
				RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
				RTMPSetTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer,
							 ((pAd->StaCfg[0].DtimPeriod * pAd->CommonCfg.BeaconPeriod) - 5));
				/* Build TDLS channel switch Request Frame */
				NStatus = TDLS_ChannelSwitchReqAction(pAd,
													  pTdlsEntry,
													  pTdlsEntry->MacAddr,
													  pTdlsControl->TdlsDesireChannel,
													  pTdlsControl->TdlsDesireChannelBW);

				if (NStatus != NDIS_STATUS_SUCCESS)
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - Build Channel Switch Request Fail !!!\n", __func__));
				else
					pTdlsControl->TdlsChannelSwitchPairCount++;
			}
		}
	}
}

/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
VOID
TDLS_MlmeChannelSwitchAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PMLME_TDLS_CH_SWITCH_STRUCT pChSwReq = NULL;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg[0].TdlsInfo;
	PRT_802_11_TDLS pTDLS = NULL;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	INT	LinkId = 0xff;
	BOOLEAN TimerCancelled;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %s()\n", __func__);
	pChSwReq = (PMLME_TDLS_CH_SWITCH_STRUCT)Elem->Msg;

	if (pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited == TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - AP Prohibite TDLS Channel Switch !!!\n", __func__));
		return;
	}

	if (INFRA_ON(pAd)) {
		/* Drop not within my TDLS Table that created before ! */
		LinkId = TDLS_SearchLinkId(pAd, pChSwReq->PeerMacAddr);

		if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - can not find the LinkId!\n", __func__));
			return;
		}

		/* Point to the current Link ID */
		pTDLS = &pAd->StaCfg[0].TdlsInfo.TDLSEntry[LinkId];
		pTdlsControl->TdlsForcePowerSaveWithAP = TRUE;
#if defined(RTMP_MAC) || defined(RLT_MAC)
		/* TODO: shiang-usw, need abstraction for this function! */
		TDLS_KickOutHwNullFrame(pAd, PWR_SAVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
		RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
		RTMPSetTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer,
					 ((pAd->StaCfg[0].DtimPeriod * pAd->CommonCfg.BeaconPeriod) - 5));
		/* Build TDLS channel switch Request Frame */
		NStatus = TDLS_ChannelSwitchReqAction(pAd,
											  pTDLS,
											  pChSwReq->PeerMacAddr,
											  pChSwReq->TargetChannel,
											  pChSwReq->TargetChannelBW);

		if (NStatus	!= NDIS_STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - Build Channel Switch Request Fail !!!\n", __func__));
		else
			pTdlsControl->TdlsChannelSwitchPairCount++;
	} else
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - TDLS only support infra mode !!!\n", __func__));

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=== %s()\n", __func__);
}

/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
VOID
TDLS_MlmeChannelSwitchRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PMLME_TDLS_CH_SWITCH_STRUCT pMlmeChSwitchRsp = NULL;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	PRT_802_11_TDLS	pTdls = NULL;
	INT LinkId = 0xff;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %s()\n", __func__);
	pMlmeChSwitchRsp = (PMLME_TDLS_CH_SWITCH_STRUCT)Elem->Msg;

	if (INFRA_ON(pAd)) {
		/* Drop not within my TDLS Table that created before ! */
		LinkId = TDLS_SearchLinkId(pAd, pMlmeChSwitchRsp->PeerMacAddr);

		if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - can not find the LinkId!\n", __func__));
			return;
		}

		/* Point to the current Link ID */
		pTdls = &pAd->StaCfg[0].TdlsInfo.TDLSEntry[LinkId];
		/* Build TDLS channel switch Request Frame */
		NStatus = TDLS_ChannelSwitchRspAction(pAd,
											  pTdls,
											  pTdls->ChSwitchTime,
											  pTdls->ChSwitchTimeout,
											  MLME_SUCCESS,
											  RTMP_TDLS_SPECIFIC_NOACK);

		if (NStatus != NDIS_STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - Build Channel Switch Response Fail !!!\n", __func__));
		else {
			RtmpusecDelay(300);
			NdisGetSystemUpTime(&pAd->StaCfg[0].TdlsInfo.TdlsGoBackStartTime);
			RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
			RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			TDLS_DisablePktChannel(pAd, FIFO_HCCA);
			TDLS_InitChannelRelatedValue(pAd, TRUE, wdev->channel, ext_cha);

			TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
			RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
		}
	} else
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - TDLS only support infra mode !!!\n", __func__));

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=== %s()\n", __func__);
}


/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
VOID
TDLS_PeerChannelSwitchReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PRT_802_11_TDLS	pTDLS = NULL;
	INT		LinkId = 0xff;
	UCHAR	PeerAddr[MAC_ADDR_LEN];
	BOOLEAN	IsInitator;
	UCHAR	TargetChannel;
	UCHAR	RegulatoryClass;
	UCHAR	NewExtChannelOffset = 0xff;
	USHORT	PeerChSwitchTime;
	USHORT	PeerChSwitchTimeOut;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	USHORT	StatusCode = MLME_SUCCESS;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg[0].TdlsInfo;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %s()\n", __func__);

	/* Not TDLS Capable, ignore it */
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	if (!INFRA_ON(pAd))
		return;

	if (pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited == TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s - AP Prohibite TDLS Channel Switch !!!\n", __func__));
		return;
	}

	hex_dump("TDLS peer channel switch request receive pack", Elem->Msg, Elem->MsgLen);

	if (!PeerTdlsChannelSwitchReqSanity(pAd,
										Elem->Msg,
										Elem->MsgLen,
										PeerAddr,
										&IsInitator,
										&TargetChannel,
										&RegulatoryClass,
										&NewExtChannelOffset,
										&PeerChSwitchTime,
										&PeerChSwitchTimeOut)) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():  from "MACSTR" Sanity Check Fail !!!\n",
				  __func__, MAC2STR(PeerAddr)));
		return;
	}

	if (RtmpPktPmBitCheck(pAd, &pAd->StaCfg[0])) {
		RTMP_SET_PSM_BIT(pAd, &pAd->StaCfg[0], PWR_ACTIVE);
		TDLS_SendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, FALSE);
	}

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s():  from "MACSTR" !!!\n",
			 __func__, MAC2STR(PeerAddr)));
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("300. %ld !!!\n", (jiffies * 1000) / OS_HZ));
	/* Drop not within my TDLS Table that created before ! */
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);

	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s() - can not find from "MACSTR" on TDLS entry !!!\n",
				  __func__, MAC2STR(PeerAddr)));
		return;
	}

	if (pTdlsControl->bChannelSwitchInitiator == FALSE) {
		pTdlsControl->TdlsForcePowerSaveWithAP = TRUE;
#if defined(RTMP_MAC) || defined(RLT_MAC)
		/* TODO: shiang-usw, need abstraction for this function! */
		TDLS_KickOutHwNullFrame(pAd, PWR_SAVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	}

	/* Point to the current Link ID */
	pTDLS = &pAd->StaCfg[0].TdlsInfo.TDLSEntry[LinkId];

	if (PeerChSwitchTime <= TDLS_CHANNEL_SWITCH_TIME)
		PeerChSwitchTime = TDLS_CHANNEL_SWITCH_TIME;

	if (PeerChSwitchTimeOut <= TDLS_CHANNEL_SWITCH_TIMEOUT)
		PeerChSwitchTimeOut = TDLS_CHANNEL_SWITCH_TIMEOUT;

	{
		UINT32 macCfg, TxCount;
		UINT32 MTxCycle;
		UCHAR  TdlsFrameType = 0;

		RTMP_IO_READ32(pAd->hdev_ctrl, TX_REPORT_CNT, &macCfg);

		if  (TargetChannel != wdev->channel)
			TdlsFrameType = RTMP_TDLS_SPECIFIC_WAIT_ACK;
		else
			TdlsFrameType = (RTMP_TDLS_SPECIFIC_WAIT_ACK + RTMP_TDLS_SPECIFIC_PKTQ_HCCA);

		NStatus = TDLS_ChannelSwitchRspAction(pAd,
											  pTDLS,
											  PeerChSwitchTime,
											  PeerChSwitchTimeOut,
											  StatusCode,
											  TdlsFrameType);

		for (MTxCycle = 0; MTxCycle < 50; MTxCycle++) {
			RTMP_IO_READ32(pAd->hdev_ctrl, TX_REPORT_CNT, &macCfg);
			TxCount = macCfg & 0x0000ffff;

			if (TxCount > 0) {
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("MTxCycle = %d, %ld !!!\n", MTxCycle, (jiffies * 1000) / OS_HZ));
				break;
			} else
				RtmpusecDelay(1000);
		}

		if (MTxCycle >= 50) {
#if defined(RTMP_MAC) || defined(RLT_MAC)
			/* TODO: shiang-usw, need abstraction for this function! */
			TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
			NStatus = NDIS_STATUS_FAILURE;
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - TDLS Transmit Channel Switch Response Fail !!!\n", __func__));
		}
	}

	if (NStatus == NDIS_STATUS_SUCCESS) {
		if  (TargetChannel != wdev->channel) {
			BOOLEAN TimerCancelled;
			pTdlsControl->TdlsDesireChannel = TargetChannel;

			if (NewExtChannelOffset != 0)
				pTdlsControl->TdlsDesireChannelBW = NewExtChannelOffset;
			else
				pTdlsControl->TdlsDesireChannelBW = EXTCHA_NONE;

			pTDLS->ChSwitchTime = PeerChSwitchTime;
			pTdlsControl->TdlsActiveSwitchTime = PeerChSwitchTime;
			pTDLS->ChSwitchTimeout = PeerChSwitchTimeOut;
			pTdlsControl->TdlsActiveSwitchTimeOut = PeerChSwitchTimeOut;
			RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
			RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			/* Cancel the timer since the received packet to me. */
			RTMPCancelTimer(&pTDLS->ChannelSwitchTimeoutTimer, &TimerCancelled);
			RTMPSetTimer(&pTDLS->ChannelSwitchTimeoutTimer, (PeerChSwitchTimeOut / 1000));
			RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
			pTDLS->bDoingPeriodChannelSwitch = TRUE;
			pTdlsControl->bChannelSwitchWaitSuccess = TRUE;
			pTdlsControl->bDoingPeriodChannelSwitch = TRUE;

			if (DebugLevel < DBG_LVL_WARN)
				RtmpusecDelay(300);
			else
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("1041. %ld !!!\n", (jiffies * 1000) / OS_HZ));

			TDLS_DisablePktChannel(pAd, FIFO_EDCA);
			TDLS_InitChannelRelatedValue(pAd, FALSE, TargetChannel, NewExtChannelOffset);
		} else {
			pTDLS->bDoingPeriodChannelSwitch = FALSE;
			pTdlsControl->bDoingPeriodChannelSwitch = FALSE;
			pTdlsControl->TdlsForcePowerSaveWithAP = FALSE;
			RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
			RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			RtmpusecDelay(300);
			TDLS_DisablePktChannel(pAd, FIFO_HCCA);
			TDLS_InitChannelRelatedValue(pAd, TRUE, wdev->channell, ext_cha);


			TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
#if defined(RTMP_MAC) || defined(RLT_MAC)
			/* TODO: shiang-usw, need abstraction for this function! */
			TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
			RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=== %s()\n", __func__);
}

/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
VOID
TDLS_PeerChannelSwitchRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PRT_802_11_TDLS pTDLS = NULL;
	INT LinkId = 0xff;
	UCHAR PeerAddr[MAC_ADDR_LEN];
	BOOLEAN TimerCancelled;
	USHORT PeerChSwitchTime;
	USHORT PeerChSwitchTimeOut;
	USHORT StatusCode = MLME_SUCCESS;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg[0].TdlsInfo;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %s()\n", __func__);

	if (!INFRA_ON(pAd))
		return;

	/* Not TDLS Capable, ignore it */
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() - from "MACSTR" !!!\n",
			 __func__, MAC2STR(PeerAddr));
	hex_dump("TDLS peer channel switch response receive pack", Elem->Msg, Elem->MsgLen);

	if (!PeerTdlsChannelSwitchRspSanity(pAd,
										Elem->Msg,
										Elem->MsgLen,
										PeerAddr,
										&StatusCode,
										&PeerChSwitchTime,
										&PeerChSwitchTimeOut)) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - Sanity Check Fail !!!\n", __func__));
		return;
	}

	/* Drop not within my TDLS Table that created before ! */
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);

	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - can not find the MAC on TDLS Table !!!\n", __func__));
		return;
	}

	/* Point to the current Link ID */
	pTDLS = &pAd->StaCfg[0].TdlsInfo.TDLSEntry[LinkId];

	if ((pTDLS->ChannelSwitchCurrentState == TDLS_CHANNEL_SWITCH_NONE) &&
		(StatusCode == MLME_REQUEST_DECLINED)) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s() - received a failed StatusCode = %d on Unsolicited response !!!\n",
				  __func__, StatusCode));
		return;
	}

	if (StatusCode == MLME_REQUEST_DECLINED) {
		if ((pTdlsControl->TdlsChannelSwitchRetryCount > 0) &&
			(pTDLS->bDoingPeriodChannelSwitch) &&
			(pTdlsControl->bDoingPeriodChannelSwitch)) {
			pTdlsControl->TdlsChannelSwitchRetryCount--;
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s() - received a failed StatusCode = %d  re-try again !!!\n",
					  __func__, StatusCode));
		} else {
			pTDLS->bDoingPeriodChannelSwitch = FALSE;
			pTdlsControl->bDoingPeriodChannelSwitch = FALSE;
			pTdlsControl->TdlsForcePowerSaveWithAP = FALSE;
		}

#if defined(RTMP_MAC) || defined(RLT_MAC)
		/* TODO: shiang-usw, need abstraction for this function! */
		TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() - received a failed StatusCode = %d !!!\n",
				 __func__, StatusCode));
		return;
	}

	if (StatusCode == MLME_SUCCESS) {
		if (pTDLS->ChannelSwitchCurrentState == TDLS_CHANNEL_SWITCH_NONE) {
			if (pTdlsControl->bChannelSwitchInitiator) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() - i am Initiator !!!\n", __func__);
			} else
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() - i am responder!!!\n",  __func__);

			if (pAd->StaCfg[0].TdlsInfo.TdlsAsicOperateChannel != wdev->channel) {
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("106. %ld !!!\n", (jiffies * 1000) / OS_HZ));
				NdisGetSystemUpTime(&pTdlsControl->TdlsGoBackStartTime);
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
				RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
				TDLS_DisablePktChannel(pAd, FIFO_HCCA);
				TDLS_InitChannelRelatedValue(pAd, TRUE, wdev->channel, ext_cha);
				TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
#if defined(RTMP_MAC) || defined(RLT_MAC)
				/* TODO: shiang-usw, need abstraction for this function! */
				TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
				RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
				RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
			}
		} else {
			if (pTdlsControl->TdlsDesireChannel != wdev->channel) {
				if (pTdlsControl->bChannelSwitchInitiator) {
					pTdlsControl->TdlsChannelSwitchPairCount--;
					pTdlsControl->TdlsChannelSwitchRetryCount = 10;

					if (PeerChSwitchTime <= TDLS_CHANNEL_SWITCH_TIME)
						PeerChSwitchTime = TDLS_CHANNEL_SWITCH_TIME;

					if (PeerChSwitchTimeOut <= TDLS_CHANNEL_SWITCH_TIMEOUT)
						PeerChSwitchTimeOut = TDLS_CHANNEL_SWITCH_TIMEOUT;

					pTDLS->ChSwitchTime = PeerChSwitchTime;
					pTdlsControl->TdlsActiveSwitchTime = PeerChSwitchTime;
					pTDLS->ChSwitchTimeout = PeerChSwitchTimeOut;
					pTdlsControl->TdlsActiveSwitchTimeOut = PeerChSwitchTimeOut;
					pTDLS->ChannelSwitchCurrentState = TDLS_CHANNEL_SWITCH_NONE;
					RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
					RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
					RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
					/* Cancel the timer since the received packet to me. */
					RTMPCancelTimer(&pTDLS->ChannelSwitchTimer, &TimerCancelled);
					RTMPSetTimer(&pTDLS->ChannelSwitchTimer, (PeerChSwitchTime / 1000));
					NdisGetSystemUpTime(&pTdlsControl->TdlsChSwSilenceTime);

					if (DebugLevel < DBG_LVL_WARN)
						RtmpusecDelay(300);
					else
						MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("104. %ld !!!\n", (jiffies * 1000) / OS_HZ));

					TDLS_DisablePktChannel(pAd, FIFO_EDCA);
					TDLS_InitChannelRelatedValue(pAd, FALSE, pTdlsControl->TdlsDesireChannel, pTdlsControl->TdlsDesireChannelBW);
				}
			} else {
				pTDLS->bDoingPeriodChannelSwitch = FALSE;
				pTdlsControl->bDoingPeriodChannelSwitch = FALSE;
				pTdlsControl->TdlsForcePowerSaveWithAP = FALSE;

				if (pTdlsControl->bChannelSwitchInitiator) {
					pTdlsControl->bChannelSwitchInitiator = FALSE;
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s() - i am channel switch Initiator !!!\n", __func__));
				} else
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s() - i am channel switch responder!!!\n",  __func__));

				RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);

				if (pAd->StaCfg[0].TdlsInfo.TdlsAsicOperateChannel != wdev->channel) {
					RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
					RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
					TDLS_DisablePktChannel(pAd, FIFO_HCCA);
					TDLS_InitChannelRelatedValue(pAd, TRUE, wdev->channel, ext_cha);
					TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
					RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
					RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
				}

#if defined(RTMP_MAC) || defined(RLT_MAC)
				/* TODO: shiang-usw, need abstraction for this function! */
				TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=== %s()\n", __func__);
}

VOID
TDLS_ForceSendChannelSwitchResponse(
	IN PRTMP_ADAPTER pAd,
	IN PRT_802_11_TDLS pTDLS)
{
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %s()\n", __func__);

	if (INFRA_ON(pAd)) {
		/* Build TDLS channel switch Response Frame */
		UCHAR TDLS_ETHERTYPE[] = {0x89, 0x0d};
		UCHAR Header802_3[14];
		PUCHAR pOutBuffer = NULL;
		ULONG FrameLen = 0;
		ULONG TempLen;
		UCHAR RemoteFrameType = PROTO_NAME_TDLS;
		NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

		MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);
		/* Allocate buffer for transmitting message */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ACT - %s() allocate memory failed\n", __func__));
			return;
		}

		MakeOutgoingFrame(pOutBuffer,		&TempLen,
						  1,				&RemoteFrameType,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
		TDLS_BuildChannelSwitchResponse(pAd,
										pOutBuffer,
										&FrameLen,
										pTDLS,
										pTDLS->ChSwitchTime,
										pTDLS->ChSwitchTimeout,
										MLME_SUCCESS);
		TDLS_SendChannelSwitchActionFrame(pAd,
										  &pAd->MacTab.Content[pTDLS->MacTabMatchWCID],
										  Header802_3,
										  LENGTH_802_3,
										  pOutBuffer,
										  (UINT)FrameLen,
										  RTMP_TDLS_SPECIFIC_NOACK);

		if (NStatus != NDIS_STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - Build Channel Switch Response Fail !!!\n", __func__));
		else
			hex_dump("TDLS send channel switch response pack", pOutBuffer, FrameLen);

		MlmeFreeMemory(pOutBuffer);
	} else
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - TDLS only support infra mode !!!\n", __func__));

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=== %s()\n", __func__);
}

VOID
TDLS_SendChannelSwitchActionFrame(
	IN  PRTMP_ADAPTER pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  PUCHAR pHeader802_3,
	IN  UINT HdrLen,
	IN  PUCHAR pData,
	IN  UINT DataLen,
	IN  UCHAR FrameType)
{
	PNDIS_PACKET    pPacket;
	NDIS_STATUS     Status;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %s()\n", __func__);

	if ((!pEntry) || (!IS_ENTRY_TDLS(pEntry)))
		return;

	do {
		/* build a NDIS packet*/
		Status = RTMPAllocateNdisPacket(pAd, &pPacket, pHeader802_3, HdrLen, pData, DataLen);

		if (Status != NDIS_STATUS_SUCCESS)
			break;

		RTMP_SET_PACKET_WCID(pPacket, pEntry->wcid);
		RTMP_SET_PACKET_WDEV(pPacket, pEntry->wdev->wdev_idx);
		RTMP_SET_PACKET_MOREDATA(pPacket, FALSE);

		if (FrameType > 0)
			RTMP_SET_TDLS_SPECIFIC_PACKET(pPacket, FrameType);
		else
			RTMP_SET_TDLS_SPECIFIC_PACKET(pPacket, 0);

		Status = send_data_pkt(pAd, pEntry->wdev, pPacket);

		if (Status == NDIS_STATUS_SUCCESS) {
			UCHAR   Index;

			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
				for (Index = 0; Index < 5; Index++)
					if (pAd->TxSwQueue[Index].Number > 0)
						RTMPDeQueuePacket(pAd, FALSE, Index, WCID_ALL, pAd->tr_ctl.max_tx_process);
			}
		}
	} while (FALSE);

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=== %s()\n", __func__);
}

VOID TDLS_OffChExpired(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg[0].TdlsInfo;
	UCHAR idx = 0;
	ULONG Now, GapJiffies, BPtoJiffies, CurTime;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	if ((pAd != NULL) && INFRA_ON(pAd)) {
		if (pAd->StaCfg[0].DtimPeriod > 1) {
			NdisGetSystemUpTime(&Now);
			BPtoJiffies = (((pAd->CommonCfg.BeaconPeriod * 1024 / 1000) * OS_HZ) / 1000);

			if (RTMP_TIME_AFTER(pAd->StaCfg[0].LastBeaconRxTime + (BPtoJiffies * pAd->StaCfg[0].DtimPeriod), Now)) {
				GapJiffies = (pAd->StaCfg[0].LastBeaconRxTime + (pAd->StaCfg[0].DtimPeriod * BPtoJiffies) - Now);
				CurTime = (GapJiffies * 1000) / OS_HZ;

				if (CurTime >= (pAd->CommonCfg.BeaconPeriod + 20)) {
					RTMPModTimer(&pTdlsControl->TdlsPeriodGoBackBaseChTimer, pAd->CommonCfg.BeaconPeriod);
					goto done;
				} else {
					if (CurTime > 20) {
						RTMPModTimer(&pTdlsControl->TdlsPeriodGoBackBaseChTimer, CurTime - 20);
						goto done;
					}
				}
			}
		}

		if (pAd->StaCfg[0].TdlsInfo.TdlsAsicOperateChannel != wdev->channel) {
			BOOLEAN TimerCancelled;

			RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);

			if (pTdlsControl->bChannelSwitchInitiator)
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("200. Initiator %ld !!!\n", (jiffies * 1000) / OS_HZ));
			else
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("201. Responder %ld !!!\n", (jiffies * 1000) / OS_HZ));

			RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
			RTMPSetTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, ((pAd->StaCfg[0].DtimPeriod * pAd->CommonCfg.BeaconPeriod) - 11));

			for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++) {
				PRT_802_11_TDLS pTdlsEntry = &pAd->StaCfg[0].TdlsInfo.TDLSEntry[idx];

				if (pTdlsEntry->Valid)
					TDLS_ForceSendChannelSwitchResponse(pAd, pTdlsEntry);
			}

			RtmpusecDelay(4000);
			NdisGetSystemUpTime(&pTdlsControl->TdlsGoBackStartTime);
			RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			TDLS_DisablePktChannel(pAd, FIFO_HCCA);
			TDLS_InitChannelRelatedValue(pAd, TRUE, wdev->channel, ext_cha);

			TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
#if defined(RTMP_MAC) || defined(RLT_MAC)
			/* TODO: shiang-usw, need abstraction for this function! */
			TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
			RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
		} else {
			TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
			MTWF_DBG(NULL, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "I am lready on Base Channel :(%d) !!!\n", wdev->channel);
		}
	}

done:
	return;
}

VOID TDLS_BaseChExpired(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg[0].TdlsInfo;
	PRT_802_11_TDLS	pTdlsEntry = NULL;

	if ((pAd != NULL) && INFRA_ON(pAd)) {
		UCHAR	idx = 0;
		MLME_TDLS_CH_SWITCH_STRUCT	TdlsChSwitch;

		if (pTdlsControl->bChannelSwitchInitiator == FALSE)
			return;

		for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++) {
			pTdlsEntry = &pAd->StaCfg[0].TdlsInfo.TDLSEntry[idx];

			if (pTdlsEntry->Valid && pTdlsEntry->bDoingPeriodChannelSwitch) {
				NdisZeroMemory(&TdlsChSwitch, sizeof(MLME_TDLS_CH_SWITCH_STRUCT));
				COPY_MAC_ADDR(TdlsChSwitch.PeerMacAddr, pTdlsEntry->MacAddr);
				TdlsChSwitch.TargetChannel = pTdlsControl->TdlsDesireChannel;
				TdlsChSwitch.TargetChannelBW = pTdlsControl->TdlsDesireChannelBW;
				MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("102. %d,%d,%ld !!!\n",
						 pTdlsControl->TdlsDesireChannel, pTdlsControl->TdlsDesireChannelBW, (jiffies * 1000) / OS_HZ));
				MlmeEnqueue(pAd,
							TDLS_CHSW_STATE_MACHINE,
							MT2_MLME_TDLS_CH_SWITCH_REQ,
							sizeof(MLME_TDLS_CH_SWITCH_STRUCT),
							&TdlsChSwitch,
							0);
				RTMP_MLME_HANDLER(pAd);
			}
		}
	}
}

/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
VOID
TDLS_ChannelSwitchTimeAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRT_802_11_TDLS	pTDLS = (PRT_802_11_TDLS)FunctionContext;
	PRTMP_ADAPTER pAd = pTDLS->pAd;
	UINT32 macCfg, TxCount;
	UINT32 MTxCycle;
	UINT16 MaxWaitingTime;
	ULONG Now, RealSwitchTime;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Channel switch procedure for ("MACSTR")\n",
			 MAC2STR(pTDLS->MacAddr));
	NdisGetSystemUpTime(&Now);

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Not finish for HIF_MT yet!\n", __func__));
		return;
	}

	RealSwitchTime = ((Now - pAd->StaCfg[0].TdlsInfo.TdlsChSwSilenceTime) * 1000) / OS_HZ;

	if (RealSwitchTime < (pTDLS->ChSwitchTime / 1000))
		RtmpOsMsDelay((pTDLS->ChSwitchTime / 1000) - RealSwitchTime);

	TDLS_EnableMacTx(pAd);
	RTMP_IO_READ32(pAd->hdev_ctrl, TX_REPORT_CNT, &macCfg);
	RtmpusecDelay(2000);
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("105. %ld !!!\n", (jiffies * 1000) / OS_HZ));
	TDLS_SendOutNullFrame(pAd, &pAd->MacTab.Content[pTDLS->MacTabMatchWCID], TRUE, TRUE);
	MaxWaitingTime = (pTDLS->ChSwitchTimeout - pTDLS->ChSwitchTime);

	for (MTxCycle = 0; MTxCycle < MaxWaitingTime; MTxCycle += 1000) {
		RTMP_IO_READ32(pAd->hdev_ctrl, TX_REPORT_CNT, &macCfg);
		TxCount = macCfg & 0x0000ffff;

		if (TxCount > 0)
			break;
		else
			RtmpusecDelay(1000);
	}

	if (MTxCycle == MaxWaitingTime) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("24. %ld @!!!\n", (jiffies * 1000) / OS_HZ));
		NdisGetSystemUpTime(&pAd->StaCfg[0].TdlsInfo.TdlsGoBackStartTime);
		TDLS_InitChannelRelatedValue(pAd, TRUE, wdev->channel, ext_cha);

#if defined(RTMP_MAC) || defined(RLT_MAC)
		/* TODO: shiang-usw, need abstraction for this function! */
		TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
		TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
	} else
		TDLS_EnablePktChannel(pAd, FIFO_HCCA);

	RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
	RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
}

/*
 *==========================================================================
 *	Description:
 *
 *	IRQL = PASSIVE_LEVEL
 *==========================================================================
 */
VOID
TDLS_ChannelSwitchTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRT_802_11_TDLS	pTDLS = (PRT_802_11_TDLS)FunctionContext;
	PRTMP_ADAPTER pAd = pTDLS->pAd;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg[0].TdlsInfo;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 ("Channel switch timeout , terminate the channel switch procedure ("MACSTR")\n",
			  MAC2STR(pTDLS->MacAddr)));
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Channel switch timeout\n"));
	pTdlsControl->bChannelSwitchWaitSuccess = FALSE;
	pTdlsControl->bDoingPeriodChannelSwitch = FALSE;

	TDLS_InitChannelRelatedValue(pAd, TRUE, wdev->channel, ext_cha);

#if defined(RTMP_MAC) || defined(RLT_MAC)
	/* TODO: shiang-usw, need abstraction for this function! */
	TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
	RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
	RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
}

VOID
TDLS_DisablePeriodChannelSwitchAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg[0].TdlsInfo;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	if (!(pAd->StaCfg[0].TdlsInfo.bChannelSwitchInitiator)) {
		pTdlsControl->bChannelSwitchInitiator = FALSE;
		pTdlsControl->bDoingPeriodChannelSwitch = FALSE;
	}

	pTdlsControl->TdlsForcePowerSaveWithAP = FALSE;

	if (pAd->StaCfg[0].TdlsInfo.TdlsAsicOperateChannel != wdev->channel) {
		RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
		RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
		TDLS_DisablePktChannel(pAd, FIFO_HCCA);
		TDLS_InitChannelRelatedValue(pAd, TRUE, wdev->channel, ext_cha);
	}

#ifdef RTMP_MAC_PCI
	RTMP_SEM_LOCK(&pAd->StaCfg[0].TdlsInfo.TdlsInitChannelLock);
#endif /* RTMP_MAC_PCI */
	pAd->StaCfg[0].TdlsInfo.TdlsCurrentTargetChannel = wdev->channel;
#ifdef RTMP_MAC_PCI
	RTMP_SEM_UNLOCK(&pAd->StaCfg[0].TdlsInfo.TdlsInitChannelLock);
#endif /* RTMP_MAC_PCI */
	TDLS_EnableMacTxRx(pAd);
#if defined(RTMP_MAC) || defined(RLT_MAC)
	/* TODO: shiang-usw, need abstraction for this function! */
	TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
	RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
	RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
}
#endif /* DOT11Z_TDLS_SUPPORT */

