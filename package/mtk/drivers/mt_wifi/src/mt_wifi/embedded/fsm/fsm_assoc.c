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
	assoc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John		2004-9-3		porting from RT2500
*/
#include "rt_config.h"

#ifdef DOT11R_FT_SUPPORT
#include "ft.h"
#endif /* DOT11R_FT_SUPPORT */



const CHAR *ASSOC_FSM_STATE_STR[MAX_ASSOC_STATE] = {
	"ASSOC_IDLE",
	"ASSOC_WAIT_RSP",
	"REASSOC_WAIT_RSP",
	"DISASSOC_WAIT_RSP"
};

const CHAR *ASSOC_FSM_MSG_STR[MAX_ASSOC_MSG] = {
	"MLME_ASSOC_REQ",
	"MLME_REASSOC_REQ",
	"MLME_DISASSOC_REQ",
	"PEER_DISASSOC_REQ",
	"PEER_ASSOC_REQ",
	"PEER_ASSOC_RSP",
	"PEER_REASSOC_REQ",
	"PEER_REASSOC_RSP",
	"DISASSOC_TIMEOUT",
	"ASSOC_TIMEOUT",
	"REASSOC_TIMEOUT"
};

/*
	==========================================================================
	Description:
		mlme assoc req handling procedure
	Parameters:
		Adapter - Adapter pointer
		Elem - MLME Queue Element
	Pre:
		the station has been authenticated and the following information is stored in the config
			-# SSID
			-# supported rates and their length
			-# listen interval (Adapter->StaCfg[0].default_listen_count)
			-# Transmit power  (Adapter->StaCfg[0].tx_power)
	Post  :
		-# An association request frame is generated and sent to the air
		-# Association timer starts
		-# Association state -> ASSOC_WAIT_RSP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_assoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_assoc_req_action)
			assoc_api->mlme_assoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_assoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_assoc_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "assoc_req", DBG_LVL_INFO, &tl);
}

/*
	==========================================================================
	Description:
		mlme reassoc req handling procedure
	Parameters:
		Elem -
	Pre:
		-# SSID  (Adapter->StaCfg[0].ssid[])
		-# BSSID (AP address, Adapter->StaCfg[0].bssid)
		-# Supported rates (Adapter->StaCfg[0].supported_rates[])
		-# Supported rates length (Adapter->StaCfg[0].supported_rates_len)
		-# Tx power (Adapter->StaCfg[0].tx_power)

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_reassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_reassoc_req_action)
			assoc_api->mlme_reassoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->mlme_reassoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_reassoc_req_action ? "HOOKED" : "NULL");
	}

	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		Upper layer issues disassoc request
	Parameters:
		Elem -

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_disassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_disassoc_req_action)
			assoc_api->mlme_disassoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_disassoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_disassoc_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		peer sends assoc rsp back
	Parameters:
		Elme - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_peer_assoc_rsp_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_assoc_rsp_action)
			assoc_api->peer_assoc_rsp_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_assoc_rsp_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_assoc_rsp_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "peer_assoc_rsp", DBG_LVL_INFO, &tl);
}


/*
	==========================================================================
	Description:
		peer sends reassoc rsp
	Parametrs:
		Elem - MLME message cntaining the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_peer_reassoc_rsp_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_reassoc_rsp_action)
			assoc_api->peer_reassoc_rsp_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_reassoc_rsp_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_reassoc_rsp_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}


/*
	==========================================================================
	Description:
		left part of IEEE 802.11/1999 p.374
	Parameters:
		Elem - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_peer_disassoc_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_disassoc_action)
			assoc_api->peer_disassoc_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_disassoc_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_disassoc_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		what the state machine will do after assoc timeout
	Parameters:
		Elme -

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_assoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_assoc_req_timeout_action)
			assoc_api->mlme_assoc_req_timeout_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_assoc_req_timeout_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_assoc_req_timeout_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		what the state machine will do after reassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_reassoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_reassoc_req_timeout_action)
			assoc_api->mlme_reassoc_req_timeout_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_reassoc_req_timeout_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_reassoc_req_timeout_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

/*
	==========================================================================
	Description:
		what the state machine will do after disassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID assoc_fsm_mlme_disassoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->mlme_disassoc_req_timeout_action)
			assoc_api->mlme_disassoc_req_timeout_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->mlme_disassoc_req_timeout_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->mlme_disassoc_req_timeout_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

static VOID assoc_fsm_peer_assoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_assoc_req_action)
			assoc_api->peer_assoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_assoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_assoc_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "peer_assoc_req", DBG_LVL_INFO, &tl);
}

static VOID assoc_fsm_peer_reassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _assoc_api_ops *assoc_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		assoc_api = (struct _assoc_api_ops *)wdev->assoc_api;

		if (assoc_api->peer_reassoc_req_action)
			assoc_api->peer_reassoc_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , assoc_api->peer_reassoc_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  assoc_api->peer_reassoc_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "peer_reassoc_req", DBG_LVL_INFO, &tl);
}



static VOID assoc_fsm_msg_invalid_state(struct _RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	ULONG curr_state;
	BOOLEAN isErrHandle = TRUE;
	USHORT Status = MLME_STATE_MACHINE_REJECT;
	PSTA_ADMIN_CONFIG pStaCfg;

	if (wdev) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "wdev(type = %d,func_idx = %d\n", wdev->wdev_type, wdev->func_idx);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "wdev = NULL\n");
		return;
	}
#ifdef CONFIG_STA_SUPPORT
	pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	if (!pStaCfg) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "pStaCfg = NULL\n");
		return;
	}
#endif


	switch (Elem->MsgType) {
	case ASSOC_FSM_MLME_ASSOC_REQ:
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_ASSOC_CONF, Status);
		break;

	case ASSOC_FSM_MLME_REASSOC_REQ:
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_REASSOC_CONF, Status);
		break;

	case ASSOC_FSM_MLME_DISASSOC_REQ:
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_DISASSOC_CONF, Status);
		break;

	default:
		isErrHandle = FALSE;
	}

	curr_state = wdev->assoc_machine.CurrState;

	if (isErrHandle == TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s [%s]: [%s][%s] ====================> state Recovery for CNTL\n",
				  __func__, wdev->if_dev->name,
				  ASSOC_FSM_STATE_STR[curr_state],
				  ASSOC_FSM_MSG_STR[Elem->MsgType]));
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s [%s]: [%s][%s] ====================> FSM MSG DROP\n",
				  __func__, wdev->if_dev->name,
				  ASSOC_FSM_STATE_STR[curr_state],
				  ASSOC_FSM_MSG_STR[Elem->MsgType]));
	}
}

static BOOLEAN assoc_fsm_msg_checker(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN isMsgDrop = FALSE;
	struct wifi_dev *wdev = Elem->wdev;

	if (!wdev)
		return isMsgDrop;

	if (!wdev->DevInfo.WdevActive)
		isMsgDrop = TRUE;

#ifdef APCLI_SUPPORT

	if (IF_COMBO_HAVE_AP_STA(pAd) && wdev->wdev_type == WDEV_TYPE_STA) {
		if (isValidApCliIf(wdev->func_idx) == FALSE)
			isMsgDrop = TRUE;
	}

#endif /* APCLI_SUPPORT */

	return isMsgDrop;
}

/* --> PUBLIC Function Start */
BOOLEAN assoc_fsm_state_transition(struct wifi_dev *wdev, ULONG NextState)
{
	ULONG OldState = 0;

	OldState = wdev->assoc_machine.CurrState;
	wdev->assoc_machine.CurrState = NextState;

	MTWF_DBG(NULL, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ASSOC[%s, TYPE:%d %s]: [%s] \t==============================================> [%s]\n",
			  wdev->if_dev->name, wdev->wdev_type, (wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
			  ASSOC_FSM_STATE_STR[OldState],
			  ASSOC_FSM_STATE_STR[NextState]);
	return TRUE;
}


VOID assoc_fsm_reset(struct wifi_dev *wdev)
{
	assoc_fsm_state_transition(wdev, ASSOC_IDLE);
}


/*
	==========================================================================
	Description:
		association state machine init, including state transition and timer init
	Parameters:
		S - pointer to the association state machine

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
VOID assoc_fsm_init(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(S, Trans, MAX_ASSOC_STATE, MAX_ASSOC_MSG,
					 (STATE_MACHINE_FUNC) assoc_fsm_msg_invalid_state, ASSOC_IDLE,
					 ASSOC_FSM_BASE);
	StateMachineSetMsgChecker(S, (STATE_MACHINE_MSG_CHECKER)assoc_fsm_msg_checker);
	/* first column */
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_MLME_ASSOC_REQ,    (STATE_MACHINE_FUNC) assoc_fsm_mlme_assoc_req_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_MLME_REASSOC_REQ,  (STATE_MACHINE_FUNC) assoc_fsm_mlme_reassoc_req_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_mlme_disassoc_req_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_peer_disassoc_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_PEER_ASSOC_REQ,    (STATE_MACHINE_FUNC) assoc_fsm_peer_assoc_req_action);
	StateMachineSetAction(S, ASSOC_IDLE, ASSOC_FSM_PEER_REASSOC_REQ,  (STATE_MACHINE_FUNC) assoc_fsm_peer_reassoc_req_action);
	/* second column */
	StateMachineSetAction(S, ASSOC_WAIT_RSP, ASSOC_FSM_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_peer_disassoc_action);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, ASSOC_FSM_PEER_ASSOC_RSP,    (STATE_MACHINE_FUNC) assoc_fsm_peer_assoc_rsp_action);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, ASSOC_FSM_PEER_REASSOC_RSP,  (STATE_MACHINE_FUNC) assoc_fsm_peer_assoc_rsp_action);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, ASSOC_FSM_ASSOC_TIMEOUT,     (STATE_MACHINE_FUNC) assoc_fsm_mlme_assoc_req_timeout_action);
	/* third column */
	StateMachineSetAction(S, REASSOC_WAIT_RSP, ASSOC_FSM_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_peer_disassoc_action);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, ASSOC_FSM_PEER_REASSOC_RSP,  (STATE_MACHINE_FUNC) assoc_fsm_peer_reassoc_rsp_action);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, ASSOC_FSM_PEER_ASSOC_RSP,    (STATE_MACHINE_FUNC) assoc_fsm_peer_reassoc_rsp_action);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, ASSOC_FSM_REASSOC_TIMEOUT,   (STATE_MACHINE_FUNC) assoc_fsm_mlme_reassoc_req_timeout_action);
	/* fourth column */
	StateMachineSetAction(S, DISASSOC_WAIT_RSP, ASSOC_FSM_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC) assoc_fsm_peer_disassoc_action);
	StateMachineSetAction(S, DISASSOC_WAIT_RSP, ASSOC_FSM_DISASSOC_TIMEOUT,  (STATE_MACHINE_FUNC) assoc_fsm_mlme_disassoc_req_timeout_action);
	wdev->assoc_machine.CurrState = ASSOC_IDLE;
}
