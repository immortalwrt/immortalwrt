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
/***************************************************************************
 ***************************************************************************

	Module Name:
	Common mgmt cntl

*/
#include "rt_config.h"

struct _cntl_api_ops ap_cntl_api_ops;


static BOOLEAN ap_cntl_scan(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "pAd is NULL!\n");
		return FALSE;
	}

#ifdef CONFIG_ATE
/* Disable scanning when ATE is running. */
	if (ATE_ON(pAd))
		return FALSE;
#endif /* CONFIG_ATE */

	if (MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_SCAN_REQ,
			Elem->MsgLen, Elem->Msg, 0, wdev)) {
		RTMP_MLME_HANDLER(pAd);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
		return TRUE;
	}

	return FALSE;

}

static VOID ap_cntl_error_handle(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	/* USHORT	status = MLME_SUCCESS; */
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	ASSERT(pAd);

	wdev = Elem->wdev;

}

static VOID ap_cntl_disconnect_proc(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev;
	CNTL_MLME_DISCONNECT_STRUCT *cntl_disconn;
	MLME_DISCONNECT_STRUCT mlme_disconn;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (PRTMP_ADAPTER)wdev->sys_handle;
	cntl_disconn = (CNTL_MLME_DISCONNECT_STRUCT *)Elem->Msg;
	mlme_disconn.reason = cntl_disconn->mlme_disconn.reason;
	os_move_mem(mlme_disconn.addr, cntl_disconn->mlme_disconn.addr, MAC_ADDR_LEN);

	if (cntl_disconn->cntl_disconn_type == CNTL_DEAUTH) {
		MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_MLME_DEAUTH_REQ,
			sizeof(MLME_DISCONNECT_STRUCT), &mlme_disconn, 0,
			wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_DEAUTH, __func__);
		RTMP_MLME_HANDLER(pAd);
	} else if (cntl_disconn->cntl_disconn_type == CNTL_DISASSOC) {
		MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_MLME_DISASSOC_REQ,
			sizeof(MLME_DISCONNECT_STRUCT), &mlme_disconn, 0,
			wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_DISASSOC, __func__);
		RTMP_MLME_HANDLER(pAd);
	}

}

VOID ap_cntl_init(
	struct wifi_dev *wdev)
{
	ap_cntl_api_ops.cntl_disconnect_proc = ap_cntl_disconnect_proc;
	ap_cntl_api_ops.cntl_scan_proc = ap_cntl_scan;
	ap_cntl_api_ops.cntl_error_handle = ap_cntl_error_handle;
	wdev->cntl_api = &ap_cntl_api_ops;
}
