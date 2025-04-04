
#ifndef __SPECTRUM_H__
#define __SPECTRUM_H__

#include "rtmp_type.h"
#include "spectrum_def.h"


typedef enum _ENUM_MEASURE_REQ_TYPE {
	BCN_MEASURE_REQ,
	BCN_MEASURE_REP,
	NB_MEASURE_REQ,
	SET_MEASURE_REQ
} ENUM_MEASURE_REQ_TYPE;

UINT8 GetRegulatoryMaxTxPwr(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 channel,
	IN struct wifi_dev *wdev);

CHAR RTMP_GetTxPwr(
	IN PRTMP_ADAPTER pAd,
	IN HTTRANSMIT_SETTING HTTxMode,
	IN UCHAR Channel,
	IN struct wifi_dev *wdev);

UINT8 GetMaxTxPwr(
	IN PRTMP_ADAPTER pAd);

/*
	==========================================================================
	Description:
		Prepare Measurement request action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
VOID MakeMeasurementReqFrame(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pOutBuffer,
	OUT PULONG pFrameLen,
	IN UINT8 TotalLen,
	IN UINT8 Category,
	IN UINT8 Action,
	IN UINT8 MeasureToken,
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT16 NumOfRepetitions);

/*
	==========================================================================
	Description:
		Prepare Measurement report action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
VOID EnqueueMeasurementRep(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDA,
	IN UINT8 DialogToken,
	IN UINT8 MeasureToken,
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT8 ReportInfoLen,
	IN PUINT8 pReportInfo);

/*
	==========================================================================
	Description:
		Prepare TPC Request action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
#ifdef TPC_SUPPORT
	UCHAR EnqueueTPCReq(
		IN PRTMP_ADAPTER pAd,
		IN PUCHAR pDA,
		IN PUCHAR pSA,
		IN PUCHAR pBssid,
		IN UCHAR DialogToken);

/*
	==========================================================================
	Description:
		Prepare TPC Report action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
	VOID EnqueueTPCRep(
		IN PRTMP_ADAPTER pAd,
		IN PUCHAR pDA,
		IN PUCHAR pSA,
		IN PUCHAR pBssid,
		IN UINT8 DialogToken,
		IN UINT8 TxPwr,
		IN UINT8 LinkMargin);
#endif

#ifdef WDS_SUPPORT
/*
	==========================================================================
	Description:
		Prepare Channel Switch Announcement action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.
		2. Channel switch announcement mode.
		2. a New selected channel.

	Return	: None.
	==========================================================================
 */
VOID EnqueueChSwAnn(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDA,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh);
#endif /* WDS_SUPPORT */

/*
	==========================================================================
	Description:
		Spectrun action frames Handler such as channel switch annoucement,
		measurement report, measurement request actions frames.

	Parametrs:
		Elme - MLME message containing the received frame

	Return	: None.
	==========================================================================
 */
VOID PeerSpectrumAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
INT Set_MeasureReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_PwrConstraint(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef TPC_SUPPORT
INT Set_TpcReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TpcReqByAddr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TpcCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TpcEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TPC_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
INT Set_VoPwrConsTest(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11K_RRM_SUPPORT */

NDIS_STATUS	MeasureReqTabInit(
	IN PRTMP_ADAPTER pAd);

VOID MeasureReqTabExit(
	IN PRTMP_ADAPTER pAd);

PMEASURE_REQ_ENTRY MeasureReqLookUp(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8			DialogToken,
	IN UINT8			measuretype);

PMEASURE_REQ_ENTRY MeasureReqInsert(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8			DialogToken,
	IN UINT8			measuretype);

VOID MeasureReqDelete(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8			DialogToken,
	IN UINT8			measuretype);

VOID InsertChannelRepIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN RTMP_STRING *pCountry,
	IN UINT8 RegulatoryClass,
	IN UINT8 *ChReptList,
	IN USHORT PhyMode,
	IN UINT8 IfIdx
);

VOID InsertBcnReportIndicationReqIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Data
);

VOID InsertTpcReportIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 TxPwr,
	IN UINT8 LinkMargin);

VOID InsertDialogToken(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 DialogToken);
#ifdef TPC_SUPPORT
NDIS_STATUS	TpcReqTabInit(
	IN PRTMP_ADAPTER pAd);

VOID TpcReqTabExit(
	IN PRTMP_ADAPTER pAd);
#endif
VOID NotifyChSwAnnToPeerAPs(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pRA,
	IN PUCHAR pTA,
	IN UINT8 ChSwMode,
	IN UINT8 Channel);

VOID RguClass_BuildBcnChList(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pBuf,
	OUT	PULONG pBufLen,
	IN struct wifi_dev *wdev,
	IN UCHAR RegClass);

void ap_chnl_switch_xmit(IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 target_ch,
	IN UINT8 target_bw);

#ifdef CONFIG_RCSA_SUPPORT
INT notify_channel_switch_to_backhaulAP(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UINT8 Channel,
	IN UINT8 ChSwMode);

INT apcli_peer_csa_sanity(
	IN MLME_QUEUE_ELEM * Elem,
	OUT CSA_IE_INFO *CsaInfo);

VOID channel_switch_action_1(
	IN RTMP_ADAPTER * pAd,
	IN CSA_IE_INFO *CsaInfo);

VOID rcsa_recovery(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev);

#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
VOID NotifyBroadcastChSwAnn(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh);

VOID NotifyBroadcastExtChSwAnn(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh);

VOID EnqueueChSwAnnNew(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh);
#endif /*ZERO_LOSS_CSA_SUPPORT*/
#endif /* __SPECTRUM_H__ */

