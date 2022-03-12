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
	"CNTL_MLME_SCAN_CONF",
	"CNTL_MLME_SCAN_FOR_CONN",
	"CNTL_MLME_FAIL",
	"CNTL_MLME_RESET_TO_IDLE",
};


inline BOOLEAN cntl_fsm_state_transition(struct wifi_dev *wdev, ULONG next_state, const char *caller)
{
	ULONG old_state = 0;

	old_state = wdev->cntl_machine.CurrState;
	wdev->cntl_machine.CurrState = next_state;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 ("CNTL [%s, TYPE:%d %s]: [%s] \t==============================================> [%s] (by %s)\n",
			  wdev->if_dev->name, wdev->wdev_type, (wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
			  CNTL_FSM_STATE_STR[old_state],
			  CNTL_FSM_STATE_STR[next_state],
			  caller));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_connect_proc hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_disconnect_proc hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_scan(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_scan_proc)
		cntl_api->cntl_scan_proc(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_scan_proc hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_scan_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_scan_conf)
		cntl_api->cntl_scan_conf(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_scan_conf hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_join_conf hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_auth_conf hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_auth_conf2 hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_deauth_conf hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_assoc_conf hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_reassoc_conf hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_disassoc_conf hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_disassoc_conf hook api.\n",
				  __func__));
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

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 ("%s [%s %s]: [%s][%s] ====================> ERR\n",
			  __func__, wdev->if_dev->name, (wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
			  CNTL_FSM_STATE_STR[curr_state],
			  CNTL_FSM_MSG_STR[Elem->MsgType]));

	if (cntl_api->cntl_error_handle)
		cntl_api->cntl_error_handle(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_error_handle hook api.\n",
				  __func__));
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
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s [%s]: [%s][%s], CNTL reset & Msg Drop\n",
				  __func__, wdev->if_dev->name,
				  CNTL_FSM_STATE_STR[wdev->cntl_machine.CurrState],
				  CNTL_FSM_MSG_STR[Elem->MsgType]));
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
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_SCAN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_scan_conf);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_JOIN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_join_conf);	/* for rept */

	/* StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect); */
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_JOIN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_join_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_SCAN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_scan_conf);
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

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	os_alloc_mem(pAd, (UCHAR **)&cntl_conn, sizeof(CNTL_MLME_CONNECT_STRUCT) + data_len);

	if (cntl_conn) {
		cntl_conn->conn_type = conn_type;
		cntl_conn->data_len = data_len;

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
			("%s,type=%d,len=%d,caller:%pS\n", __func__, conn_type, data_len, OS_TRACE));

		if (!cntl_idle(wdev)) {
			SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG,
				("%s,Return since CNTL not IDLE,CNTL(%ld),SYNC(%ld),AUTH(%ld),ASSOC(%ld)\n",
				__func__,
				wdev->cntl_machine.CurrState,
				ScanCtrl->SyncFsm.CurrState,
				wdev->auth_machine.CurrState,
				wdev->assoc_machine.CurrState));
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
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Alloc memory failed.\n",
				  __func__));
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

	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s, caller:%pS,type=%d,reason=%d, dev_name=%s \n", __func__, OS_TRACE, disconn_type, reason, wdev->if_dev->name));

	cntl_disconn.cntl_disconn_type = disconn_type;
	os_move_mem(cntl_disconn.mlme_disconn.addr, addr, MAC_ADDR_LEN);
	cntl_disconn.mlme_disconn.reason = reason;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

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

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_SCAN,
						sizeof(MLME_SCAN_REQ_STRUCT),
						mlme_scan_request,
						0,
						wdev);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_scan_conf(
	struct wifi_dev *wdev,
	USHORT status)
{
	RTMP_ADAPTER *pAd;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_SCAN_CONF,
						sizeof(USHORT),
						&status,
						0,
						wdev);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_join_start_conf(
	struct wifi_dev *wdev,
	USHORT status)
{
	RTMP_ADAPTER *pAd;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
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
