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


const CHAR *CNTL_FSM_STATE_STR[MAX_CNTL_STATE] = {
	"CNTL_IDLE",
	"CNTL_WAIT_SYNC",
	"CNTL_WAIT_AUTH",
	"CNTL_WAIT_AUTH2",
	"CNTL_WAIT_DEAUTH",
	"CNTL_WAIT_ASSOC",
	"CNTL_WAIT_DISASSOC",
};

const CHAR *CNTL_FSM_MSG_STR[MAX_CNTL_MSG] = {
	"CNTL_MACHINE_BASE/CNTL_MLME_CONNECT",
	"CNTL_MLME_JOIN_CONF",
	"CNTL_MLME_AUTH_CONF",
	"CNTL_MLME_ASSOC_CONF",
	"CNTL_MLME_REASSOC_CONF",
	"CNTL_MLME_DISCONNECT",
	"CNTL_MLME_DEAUTH_CONF",
	"CNTL_MLME_DISASSOC_CONF",
	"CNTL_MLME_SCAN",
	"CNTL_MLME_SCAN_FOR_CONN",
	"CNTL_MLME_FAIL",
	"CNTL_MLME_RESET_TO_IDLE",
};


inline BOOLEAN cntl_fsm_state_transition(struct wifi_dev *wdev, ULONG next_state, const char *caller)
{
	ULONG old_state = 0;

	old_state = wdev->cntl_machine.CurrState;
	wdev->cntl_machine.CurrState = next_state;

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "CNTL [%s, TYPE:%d %s]: [%s] \t==============================================> [%s] (by %s)\n",
			  wdev->if_dev->name, wdev->wdev_type, (wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
			  CNTL_FSM_STATE_STR[old_state],
			  CNTL_FSM_STATE_STR[next_state],
			  caller);
	return TRUE;
}

static VOID cntl_mlme_connect(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_connect_proc)
		cntl_api->cntl_connect_proc(wdev, Elem->Msg, Elem->MsgLen);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_connect_proc hook api.\n");
	}
}

static VOID cntl_mlme_disconnect(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_disconnect_proc)
		cntl_api->cntl_disconnect_proc(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_disconnect_proc hook api.\n");
	}
}

static VOID cntl_mlme_scan(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;
	BOOLEAN ret = FALSE;
	UCHAR owner = CH_OP_OWNER_IDLE;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_scan_proc)
		ret = cntl_api->cntl_scan_proc(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_scan_proc hook api.\n");
	}

	if (ret == FALSE) {
		/* enq scan req fail, we should release channelOp here.*/
		owner = GetCurrentChannelOpOwner(pAd, wdev);
		if (owner == CH_OP_OWNER_SCAN)
			ReleaseChannelOpCharge(pAd, wdev, owner);
	}

}

static VOID cntl_mlme_join_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_join_conf)
		cntl_api->cntl_join_conf(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_join_conf hook api.\n");
	}
}

static VOID cntl_mlme_auth_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_auth_conf)
		cntl_api->cntl_auth_conf(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_auth_conf hook api.\n");
	}
}

static VOID cntl_mlme_auth2_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_auth2_conf)
		cntl_api->cntl_auth2_conf(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_auth_conf2 hook api.\n");
	}
}

static VOID cntl_mlme_deauth_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_deauth_conf)
		cntl_api->cntl_deauth_conf(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_deauth_conf hook api.\n");
	}
}

static VOID cntl_mlme_assoc_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_assoc_conf)
		cntl_api->cntl_assoc_conf(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_assoc_conf hook api.\n");
	}
}

static VOID cntl_mlme_reassoc_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_reassoc_conf)
		cntl_api->cntl_reassoc_conf(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_reassoc_conf hook api.\n");
	}
}

static VOID cntl_mlme_disassoc_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_disassoc_conf)
		cntl_api->cntl_disassoc_conf(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_disassoc_conf hook api.\n");
	}
}

static VOID cntl_mlme_reset_all_fsm(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_reset_all_fsm_proc)
		cntl_api->cntl_reset_all_fsm_proc(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_disassoc_conf hook api.\n");
	}
}

static VOID cntl_mlme_error_handle(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _cntl_api_ops *cntl_api;
	ULONG curr_state = wdev->cntl_machine.CurrState;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		"[%s %s]: [%s][%s] ====================> ERR\n",
		wdev->if_dev->name, (wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
		CNTL_FSM_STATE_STR[curr_state], CNTL_FSM_MSG_STR[Elem->MsgType]);

	switch (Elem->MsgType) {
	case CNTL_MLME_SCAN:
		cntl_scan_conf(wdev, MLME_INVALID_FORMAT);
		break;
	}

	if (cntl_api->cntl_error_handle)
		cntl_api->cntl_error_handle(Elem);
	else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No cntl_error_handle hook api.\n");
	}
}

static BOOLEAN cntl_fsm_msg_checker(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN isMsgDrop = FALSE;
	struct wifi_dev *wdev = Elem->wdev;

	if (wdev) {
		if (!wdev->DevInfo.WdevActive)
			isMsgDrop = TRUE;

#ifdef APCLI_SUPPORT

		if ((IF_COMBO_HAVE_AP_STA(pAd) && wdev->wdev_type == WDEV_TYPE_STA) &&
			(isValidApCliIf(wdev->func_idx) == FALSE))
			isMsgDrop = TRUE;

#endif /* APCLI_SUPPORT */
	}

	if (isMsgDrop == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "[%s]: [%s][%s], CNTL reset & Msg Drop\n",
				  wdev->if_dev->name,
				  CNTL_FSM_STATE_STR[wdev->cntl_machine.CurrState],
				  CNTL_FSM_MSG_STR[Elem->MsgType]);
		cntl_mlme_error_handle(pAd, Elem);

		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
	}

	return isMsgDrop;
}

void cntl_state_machine_init(
	IN struct wifi_dev *wdev,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;

	StateMachineSetMsgChecker(Sm, (STATE_MACHINE_MSG_CHECKER)cntl_fsm_msg_checker);
	StateMachineInit(Sm, Trans, MAX_CNTL_STATE, MAX_CNTL_MSG,
					 (STATE_MACHINE_FUNC) cntl_mlme_error_handle, CNTL_IDLE,
					 CNTL_MACHINE_BASE);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_CONNECT, (STATE_MACHINE_FUNC) cntl_mlme_connect);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_SCAN, (STATE_MACHINE_FUNC) cntl_mlme_scan);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_JOIN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_join_conf);	/* for rept */

	/* StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect); */
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_JOIN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_join_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_SCAN, (STATE_MACHINE_FUNC) cntl_mlme_scan);

	StateMachineSetAction(Sm, CNTL_WAIT_AUTH, CNTL_MLME_AUTH_CONF, (STATE_MACHINE_FUNC) cntl_mlme_auth_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_AUTH, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect);
	StateMachineSetAction(Sm, CNTL_WAIT_AUTH2, CNTL_MLME_AUTH_CONF, (STATE_MACHINE_FUNC) cntl_mlme_auth2_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_AUTH2, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect);
	StateMachineSetAction(Sm, CNTL_WAIT_DEAUTH, CNTL_MLME_DEAUTH_CONF, (STATE_MACHINE_FUNC) cntl_mlme_deauth_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_ASSOC, CNTL_MLME_ASSOC_CONF, (STATE_MACHINE_FUNC) cntl_mlme_assoc_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_ASSOC, CNTL_MLME_REASSOC_CONF, (STATE_MACHINE_FUNC) cntl_mlme_reassoc_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_ASSOC, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect);
	StateMachineSetAction(Sm, CNTL_WAIT_DISASSOC, CNTL_MLME_DISASSOC_CONF, (STATE_MACHINE_FUNC) cntl_mlme_disassoc_conf);

	/* Cancel Action */
	for (i = 0; i < MAX_CNTL_STATE; i++)
		StateMachineSetAction(Sm, i, CNTL_MLME_RESET_TO_IDLE, (STATE_MACHINE_FUNC) cntl_mlme_reset_all_fsm);

	wdev->cntl_machine.CurrState = CNTL_IDLE;
}

/* Export API - Start */
BOOLEAN cntl_connect_request(
	struct wifi_dev *wdev,
	enum _CNTL_CONNECT_TYPE conn_type,
	UCHAR data_len,
	UCHAR *data)
{
	RTMP_ADAPTER *pAd;
	CNTL_MLME_CONNECT_STRUCT *cntl_conn;
	UCHAR owner = CH_OP_OWNER_IDLE;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pAd is NULL!\n");
		return FALSE;
	}
	os_alloc_mem(pAd, (UCHAR **)&cntl_conn, sizeof(CNTL_MLME_CONNECT_STRUCT) + data_len);

	if (cntl_conn) {
		cntl_conn->conn_type = conn_type;
		cntl_conn->data_len = data_len;

		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
			"type=%d,len=%d,caller:%pS\n", conn_type, data_len, OS_TRACE);

		owner = GetCurrentChannelOpOwner(pAd, wdev);
		if ((owner == CH_OP_OWNER_SCAN) || (owner == CH_OP_OWNER_PARTIAL_SCAN)) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
				"scan is ongoing, forbid connect.\n");
			os_free_mem(cntl_conn);
			return FALSE;
		}

		if (!cntl_idle(wdev)) {
			SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				"Return since CNTL not IDLE,CNTL(%ld),SYNC(%ld),AUTH(%ld),ASSOC(%ld)\n",
				wdev->cntl_machine.CurrState,
				ScanCtrl->SyncFsm.CurrState,
				wdev->auth_machine.CurrState,
				wdev->assoc_machine.CurrState);
			os_free_mem(cntl_conn);
			return FALSE;
		}

		if (data && data_len)
			os_move_mem(cntl_conn->data, data, data_len);
		wdev_fsm_init(wdev);
		MlmeEnqueueWithWdev(pAd,
							MLME_CNTL_STATE_MACHINE,
							CNTL_MLME_CONNECT,
							sizeof(CNTL_MLME_CONNECT_STRUCT) + data_len,
							cntl_conn,
							0,
							wdev);
		RTMP_MLME_HANDLER(pAd);
		os_free_mem(cntl_conn);
		return TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Alloc memory failed.\n");
	}

	return FALSE;
}

BOOLEAN cntl_disconnect_request(
	struct wifi_dev *wdev,
	enum _CNTL_DISCONNECT_TYPE disconn_type,
	UCHAR *addr,
	USHORT reason)
{
	RTMP_ADAPTER *pAd;
	CNTL_MLME_DISCONNECT_STRUCT cntl_disconn;

	ASSERT(wdev->sys_handle);

	MTWF_DBG(NULL, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		"caller:%pS,type=%d,reason=%d\n", OS_TRACE, disconn_type, reason);

	cntl_disconn.cntl_disconn_type = disconn_type;
	os_move_mem(cntl_disconn.mlme_disconn.addr, addr, MAC_ADDR_LEN);
	cntl_disconn.mlme_disconn.reason = reason;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pAd is NULL!\n");
		return FALSE;
	}

	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_DISCONNECT,
						sizeof(CNTL_MLME_DISCONNECT_STRUCT),
						&cntl_disconn,
						0,
						wdev);

	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_scan_request(
	struct wifi_dev *wdev,
	MLME_SCAN_REQ_STRUCT *mlme_scan_request)
{
	RTMP_ADAPTER *pAd;
	UCHAR owner = CH_OP_OWNER_IDLE;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pAd is NULL!\n");
		return FALSE;
	}

#ifdef APCLI_SUPPORT
	/* set random MAC addr for every new scan cycle */
	if (wdev->SecConfig.apcli_pe_support) {
		if (apcli_set_random_mac_addr(wdev, FALSE) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"APCLI PE: encountered error, please chk!!!\n");
		}
	}
#endif
	if (MlmeEnqueueWithWdev(pAd, MLME_CNTL_STATE_MACHINE, CNTL_MLME_SCAN,
			sizeof(MLME_SCAN_REQ_STRUCT), mlme_scan_request, 0, wdev))
		RTMP_MLME_HANDLER(pAd);
	else {
		/* enq scan req fail, we should release channelOp here.*/
		owner = GetCurrentChannelOpOwner(pAd, wdev);
		if (owner == CH_OP_OWNER_SCAN)
			ReleaseChannelOpCharge(pAd, wdev, owner);
	}

	return TRUE;
}

BOOLEAN cntl_scan_conf(
	struct wifi_dev *wdev,
	USHORT status)
{
	RTMP_ADAPTER *pAd;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	SCAN_INFO *ScanInfo;
	SCAN_CTRL *ScanCtrl = NULL;
	BSS_TABLE *ScanTab = NULL;
	INT BssIdx;
	UCHAR ch, band_idx;
	INT MaxNumBss;
	UCHAR owner = CH_OP_OWNER_IDLE;

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev is NULL!\n");
		return FALSE;
	}

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pAd is NULL!\n");
		return FALSE;
	}

	ScanInfo = &wdev->ScanInfo;
	/* scan completed, init to not FastScan */
	ScanInfo->bImprovedScan = FALSE;

	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	band_idx =  HcGetBandByWdev(wdev);
	MaxNumBss = pAd->ApCfg.BssidNum;

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		pStaCfg = GetStaCfgByWdev(pAd, wdev);
		if (!pStaCfg)
			goto full_reset;
	}

#ifdef APCLI_CFG80211_SUPPORT
	RTEnqueueInternalCmd(pAd, CMDTHREAD_SCAN_END, NULL, 0);
#endif /* APCLI_CFG80211_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	/* */
	/* Set LED status to previous status. */
	/* */
	if (pAd->LedCntl.bLedOnScanning) {
		pAd->LedCntl.bLedOnScanning = FALSE;
		RTMPSetLED(pAd, pAd->LedCntl.LedStatus, HcGetBandByWdev(wdev));
	}
#endif /* LED_CONTROL_SUPPORT */

#ifdef DOT11N_DRAFT3
	/* AP sent a 2040Coexistence mgmt frame, then station perform a scan,
	*  and then send back the respone.
	*/
	if ((pAd->CommonCfg.BSSCoexist2040.field.InfoReq == 1)
		&& pStaCfg && INFRA_ON(pStaCfg)
		&& OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) {
		MAC_TABLE_ENTRY *pEntry;

		pEntry = GetAssociatedAPByWdev(pAd, wdev);
		ASSERT(pEntry);
		if (!pEntry)
			goto full_reset;
		Update2040CoexistFrameAndNotify(pAd, pEntry->wcid, TRUE);
	}
#endif /* DOT11N_DRAFT3 */
#ifdef WPA_SUPPLICANT_SUPPORT
	if (pStaCfg && pStaCfg->bAutoReconnect == TRUE &&
		pAd->IndicateMediaState != NdisMediaStateConnected &&
		(pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_ENABLE_WITH_WEB_UI)) {
		BssTableSsidSort(pAd, &pStaCfg->wdev, &pStaCfg->MlmeAux.SsidBssTab, (PCHAR)pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
		pStaCfg->MlmeAux.BssIdx = 0;
		IterateOnBssTab(pAd, wdev);
	}
#endif /* WPA_SUPPLICANT_SUPPORT */

	if (status == MLME_SUCCESS) {
		/*
			Maintain Scan Table
			MaxBeaconRxTimeDiff: 120 seconds
			MaxSameBeaconRxTimeCount: 1
		*/
		/* MaintainBssTable(pAd, wdev, &pAd->ScanTab, 120, 2); */
		if (wdev->wdev_type == WDEV_TYPE_STA) {
#ifdef CONFIG_MULTI_CHANNEL
			bss_table_maintenance(pAd, &pStaCfg->wdev, ScanTab, 120, 4);
#else
			bss_table_maintenance(pAd, &pStaCfg->wdev, ScanTab, 120, 2);
#endif /* !CONFIG_MULTI_CHANNEL */
		}
		RTMPSendWirelessEvent(pAd, IW_SCAN_COMPLETED_EVENT_FLAG, NULL, BSS0, 0);
#ifdef WPA_SUPPLICANT_SUPPORT
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_SCAN, -1, NULL, NULL, 0);
#endif /* WPA_SUPPLICANT_SUPPORT */
	}

	/* enter psm after end of scan */
	if (pStaCfg && INFRA_ON(pStaCfg)) {
		if (pStaCfg->WindowsPowerMode != Ndis802_11PowerModeCAM &&
			!pStaCfg->PwrMgmt.bDoze)
			RTMP_SLEEP_FORCE_AUTO_WAKEUP(pAd, pStaCfg);
	}

full_reset:

	MTWF_DBG(wdev->sys_handle, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_NOTICE,
		"reset scan fsm, current status is 0x%x\n", status);

	cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);

	if (ScanInfo->LastScanChannel != 0) {
		ch = wlan_operate_get_prim_ch(wdev);
		ASSERT((ch != 0));
		/*restore to original channel*/
		wlan_operate_set_prim_ch(wdev, ch);
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			" - End of SCAN (TYPE/BandIdx: %d/%d), restore ch %d, Total BSS[%02d]\n",
			ScanCtrl->ScanType, ScanCtrl->BandIdx, ch, ScanTab->BssNr);
	}

	AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[band_idx], HW_BSSID_0, OPMODE_AP);
	/* Enable beacon tx for all BSS */
	for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
		struct wifi_dev *apWdev = NULL;

		apWdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
		if (!apWdev)
			continue;
		if (apWdev->channel != wdev->channel)
			continue;

		if (apWdev->bAllowBeaconing)
			UpdateBeaconHandler(pAd, apWdev, BCN_UPDATE_ENABLE_TX);
	}

	ScanCtrl->Channel = 0;
	ScanInfo->LastScanChannel = 0;
#ifdef OFFCHANNEL_SCAN_FEATURE
	ScanCtrl->OffChScan_Ongoing = FALSE;
	ScanCtrl->state = OFFCHANNEL_SCAN_INVALID;
	ScanCtrl->Num_Of_Channels = 0;
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd))
		memset(ScanCtrl->ScanGivenChannel, 0, MAX_AWAY_CHANNEL);
	else
#endif
		ScanCtrl->ScanGivenChannel[ScanCtrl->CurrentGivenChan_Index] = 0;
	ScanCtrl->CurrentGivenChan_Index = 0;
#endif

	MTWF_PRINT("SCAN DONE, Reset FSM/CNTL IDLE.\n");

	owner = GetCurrentChannelOpOwner(pAd, wdev);
	if (owner == CH_OP_OWNER_SCAN)
		ReleaseChannelOpCharge(pAd, wdev, owner);
	else if (owner == CH_OP_OWNER_PARTIAL_SCAN) {
		ScanCtrl->PartialScan.bScanning = FALSE;
		ScanCtrl->PartialScan.BreakTime = 0;
		ScanCtrl->PartialScan.pwdev = NULL;
		ScanCtrl->ScanType = SCAN_ACTIVE;
		ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_PARTIAL_SCAN);
	}
#ifdef SCAN_RADAR_COEX_SUPPORT
	if (wdev != NULL && wdev->RadarDetected)
		RTMP_OS_COMPLETE(&wdev->scan_complete);
#endif
	if (wdev != NULL && wdev->ch_set_in_progress && wdev->ch_wait_in_progress)
		RTMP_OS_COMPLETE(&wdev->ch_wait_for_scan);
	return TRUE;
}

BOOLEAN cntl_join_start_conf(
	struct wifi_dev *wdev,
	USHORT status)
{
	RTMP_ADAPTER *pAd;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pAd is NULL!\n");
		return FALSE;
	}

	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_JOIN_CONF,
						sizeof(USHORT),
						&status,
						0,
						wdev);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_auth_assoc_conf(
	struct wifi_dev *wdev,
	enum _CNTL_MLME_EVENT event_type,
	USHORT reason)
{
	RTMP_ADAPTER *pAd;
	ULONG cntl_curr_state = wdev->cntl_machine.CurrState;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pAd is NULL!\n");
		return FALSE;
	}

	if ((event_type == CNTL_MLME_DISASSOC_CONF)
		&& (cntl_curr_state != CNTL_WAIT_DISASSOC)) {
		/*
		 *	Ignore this message directly for this case.
		 */
		return FALSE;
	}

	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						event_type,
						sizeof(USHORT),
						&reason,
						0,
						wdev);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_do_disassoc_now(
	struct wifi_dev *wdev)
{
	if (wdev->cntl_machine.CurrState == CNTL_WAIT_DISASSOC)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN cntl_idle(
	struct wifi_dev *wdev)
{
	return (wdev->cntl_machine.CurrState == CNTL_IDLE ? TRUE : FALSE);
}

VOID cntl_fsm_reset(struct wifi_dev *wdev)
{
	cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
}

BOOLEAN cntl_reset_all_fsm_in_ifdown(
	struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd;
	USHORT status = MLME_SUCCESS;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pAd is NULL!\n");
		return FALSE;
	}

	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_RESET_TO_IDLE,
						sizeof(USHORT),
						&status,
						0,
						wdev);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}


/* Export API - End */
