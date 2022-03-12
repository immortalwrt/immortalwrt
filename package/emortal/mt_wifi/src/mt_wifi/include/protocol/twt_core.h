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
    twt_core.h

    Abstract:
    Support twt mlme

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/

#ifndef _TWT_CORE_H_
#define _TWT_CORE_H_

#ifdef MT_MAC

#define TWT_ROLE_STA		0
#define TWT_ROLE_AP			1
#define TWT_ROLE_APCLI		2

#define TWT_TSF_ALIGNMENT_EN		1
#define TWT_PARA_OPTIMIZE_EN		0

#define IS_STA_CAP_TWT(_entry) \
	((_entry->cap.he_mac_cap & HE_TWT_REQUEST) ? TRUE : FALSE)

#define IS_STA_EXT_CAP_TWT(_ie_list) \
	((_ie_list->ExtCapInfo.twt_requester_support != 0) ? TRUE : FALSE)

#define IS_STA_WITH_TWT_IE(_ie_list) \
	(((_ie_list->twt_ie.elem_id == IE_TWT) && (_ie_list->twt_ie.len > 0)) ? TRUE : FALSE)

#ifdef APCLI_SUPPORT
#define TWT_AGRT_PARA_BITMAP_TRIGGER_OFFSET		0
#define TWT_AGRT_PARA_BITMAP_ANNCE_OFFSET		1
#define TWT_AGRT_PARA_BITMAP_PROTECT_OFFSET		2
#endif /* APCLI_SUPPORT */

/* TWT tear down reqest for mlme */
struct mlme_twt_tear_down_req_struct {
	struct wifi_dev *wdev;
	UINT16   wcid;
	UCHAR   peer_addr[MAC_ADDR_LEN];
	UCHAR   twt_flow_id;
};

/* TWT IE parse and build */
VOID parse_twt_ie(
	IN struct _EID_STRUCT *ie_head,
	IN VOID *ie_list);

PUINT8 build_twt_ie(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	OUT PUINT8 f_buf,
	IN PVOID ie_list);

/* TWT action frame state machine management (for peer STA role) */
VOID peer_twt_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

VOID peer_twt_info_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

/* TWT action frame trigger (for AP role) */
VOID mlme_twt_teradown_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

/* Peer STA link down twt management */
VOID twt_resource_release_at_link_down(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid);

/* TWT resource dump */
VOID twt_resource_dump(
	IN struct wifi_dev *wdev);

/* TWT get current tsf */
VOID twt_get_current_tsf(
	IN struct wifi_dev *wdev,
	OUT PUINT32 current_tsf);

#ifdef APCLI_SUPPORT
/* TWT action frame trigger (for AP role) */
VOID twtMlmeSetupAction(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

VOID twtReqFsmSteps(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN enum ENUM_TWT_REQUESTER_STATE_T eNextState,
	IN UINT8 ucTWTFlowId,
	IN void *pParam);

VOID twtReqFsmRunEventRxSetup(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId);

VOID twtReqFsmRunEventRxTeardown(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId);

VOID twtParseTWTElement(
	struct twt_ie *prTWTIE,
	struct twt_params_t *prTWTParams);

UINT8 twtGetRxSetupFlowId(
	struct twt_ie *prTWTIE);

UINT32 twtPlannerAddAgrtTbl(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 ucFlowId);

VOID twtReqFsmSendEvent(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId,
	IN UINT8 eMsgId);

VOID twtTxDoneCheckSetupFrame(
	IN struct _RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pkt);

VOID twtPlannerDbgPrintVal(
	IN struct _RTMP_ADAPTER *ad,
	IN struct twt_params_t *prTWTParams);

UINT32 twtPlannerDrvAgrtInsert(
	IN struct twt_planner_t *prTWTPlanner,
	IN UINT8 ucBssIdx,
	IN UINT8 ucFlowId,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 ucIdx);

UINT32 twtPlannerDrvAgrtAdd(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucFlowId,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 *pucIdx);

UINT32 twtPlannerGetCurrentTSF(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN struct twt_get_tsf_context_t *prGetTsfCtxt,
	IN UINT32 u4SetBufferLen);

UINT32 twtGetTxTeardownFlowId(
	IN struct frame_teardown *pframe_tear_down);
#endif /* APCLI_SUPPORT */
#endif /* MT_MAC */

#endif /* _TWT_CORE_H_ */
