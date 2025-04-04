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
    fsm_sync.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/

#include "rt_config.h"


const CHAR *AUTH_FSM_STATE_STR[AUTH_FSM_MAX_STATE] = {
	"AUTH_IDLE",
	"AUTH_WAIT_SEQ2",
	"AUTH_FSM_WAIT_SEQ4",
	"AUTH_FSM_WAIT_SAE"
};

const CHAR *AUTH_FSM_MSG_STR[AUTH_FSM_MAX_MSG] = {
	"MLME_AUTH_REQ",
	"PEER_AUTH_EVEN",
	"PEER_AUTH_ODD",
	"AUTH_TIMEOUT",
	"PEER_DEAUTH",
	"MLME_DEAUTH_REQ",
	"PEER_AUTH_REQ",
	"PEER_AUTH_CONF",
	"SAE_AUTH_REQ",
	"SAE_AUTH_RSP"
};

static VOID auth_fsm_msg_invalid_state(struct _RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	ULONG curr_state = wdev->auth_machine.CurrState;
	USHORT Status = MLME_STATE_MACHINE_REJECT;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "[%s %s]: [%s][%s] ====================> FSM MSG DROP\n",
			   wdev->if_dev->name, (wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
			  AUTH_FSM_STATE_STR[curr_state],
			  AUTH_FSM_MSG_STR[Elem->MsgType]);
	/* inform cntl this error */
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_AUTH_CONF, Status);
}

inline BOOLEAN auth_fsm_state_transition(struct wifi_dev *wdev, ULONG next_state, const char *caller)
{
	ULONG old_state = 0;

		old_state = wdev->auth_machine.CurrState;
		wdev->auth_machine.CurrState = next_state;

	MTWF_DBG(NULL, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "AUTH [%s, TYPE:%d %s]: [%s] \t==============================================> [%s]  (by %s)\n",
			  wdev->if_dev->name, wdev->wdev_type, (wdev->wdev_type == WDEV_TYPE_REPEATER) ? "(REPT)" : "(STA)",
			  AUTH_FSM_STATE_STR[old_state],
			  AUTH_FSM_STATE_STR[next_state],
			  caller);
	return TRUE;
}

VOID auth_fsm_mlme_deauth_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->mlme_deauth_req_action)
			auth_api->mlme_deauth_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->mlme_deauth_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->mlme_deauth_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

VOID auth_fsm_mlme_auth_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->mlme_auth_req_action)
			auth_api->mlme_auth_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->mlme_auth_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->mlme_auth_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "auth_req", DBG_LVL_INFO, &tl);
}

static VOID auth_fsm_auth_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->auth_timeout_action)
			auth_api->auth_timeout_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->auth_timeout_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->auth_timeout_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

static VOID auth_fsm_peer_deauth_action(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_deauth_action)
			auth_api->peer_deauth_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->peer_deauth_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_deauth_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

static VOID auth_fsm_peer_auth_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_auth_req_action)
			auth_api->peer_auth_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->peer_auth_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_auth_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}

	log_time_end(LOG_TIME_CONNECTION, "peer_auth_req", DBG_LVL_DEBUG, &tl);
}


static VOID auth_fsm_peer_auth_rsp_at_seq2_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_auth_rsp_at_seq2_action)
			auth_api->peer_auth_rsp_at_seq2_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->peer_auth_rsp_at_seq2_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_auth_rsp_at_seq2_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
	log_time_end(LOG_TIME_CONNECTION, "peer_auth_rsp_at_seq2", DBG_LVL_INFO, &tl);
}


static VOID auth_fsm_peer_auth_rsp_at_seq4_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_auth_rsp_at_seq4_action)
			auth_api->peer_auth_rsp_at_seq4_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->auth_fsm_peer_auth_rsp_at_seq4_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_auth_rsp_at_seq4_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
	log_time_end(LOG_TIME_CONNECTION, "peer_auth_rsp_at_seq4", DBG_LVL_INFO, &tl);
}



static VOID auth_fsm_peer_auth_confirm_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->peer_auth_confirm_action)
			auth_api->peer_auth_confirm_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->peer_auth_confirm_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->peer_auth_confirm_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
}

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
static VOID auth_fsm_sae_auth_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->sae_auth_req_action)
			auth_api->sae_auth_req_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->auth_fsm_sae_auth_req_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->sae_auth_req_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
	log_time_end(LOG_TIME_CONNECTION, "sae_auth_req", DBG_LVL_INFO, &tl);
}


static VOID auth_fsm_sae_auth_rsp_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _auth_api_ops *auth_api = NULL;
	struct time_log tl;

	log_time_begin(LOG_TIME_UNIT_US, &tl);

	ASSERT(wdev);

	if (wdev) {
		auth_api = (struct _auth_api_ops *)wdev->auth_api;

		if (auth_api->sae_auth_rsp_action)
			auth_api->sae_auth_rsp_action(pAd, Elem);
		else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s , auth_api->auth_fsm_sae_auth_rsp_action %s\n",
				  wdev ? "OK" : "NULL",
				  auth_api->sae_auth_rsp_action ? "HOOKED" : "NULL");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev %s\n",
				  wdev ? "OK" : "NULL");
		return;
	}
	log_time_end(LOG_TIME_CONNECTION, "sae_auth_rsp", DBG_LVL_INFO, &tl);
}
#endif /*DOT11_SAE_SUPPORT */


VOID auth_fsm_init(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STATE_MACHINE *Sm, STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, AUTH_FSM_MAX_STATE, AUTH_FSM_MAX_MSG,
					 (STATE_MACHINE_FUNC)auth_fsm_msg_invalid_state, AUTH_FSM_IDLE, AUTH_FSM_BASE);
	/* cmm */
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_mlme_auth_req_action);
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_PEER_DEAUTH, (STATE_MACHINE_FUNC)auth_fsm_peer_deauth_action);
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_PEER_AUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_peer_auth_req_action);
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_PEER_AUTH_CONF, (STATE_MACHINE_FUNC)auth_fsm_peer_auth_confirm_action);
	/* the first column */
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_mlme_deauth_req_action);
	/* the second column */
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ2, AUTH_FSM_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)auth_fsm_peer_auth_rsp_at_seq2_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ2, AUTH_FSM_PEER_DEAUTH, (STATE_MACHINE_FUNC)auth_fsm_peer_deauth_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ2, AUTH_FSM_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)auth_fsm_auth_timeout_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ2, AUTH_FSM_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_mlme_deauth_req_action);
	/* the third column */
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ4, AUTH_FSM_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)auth_fsm_peer_auth_rsp_at_seq4_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ4, AUTH_FSM_PEER_DEAUTH, (STATE_MACHINE_FUNC)auth_fsm_peer_deauth_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ4, AUTH_FSM_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)auth_fsm_auth_timeout_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SEQ4, AUTH_FSM_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)auth_fsm_mlme_deauth_req_action);

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	/* only for sta mode, todo: unify with ap mode */
	StateMachineSetAction(Sm, AUTH_FSM_IDLE, AUTH_FSM_SAE_AUTH_REQ, (STATE_MACHINE_FUNC) auth_fsm_sae_auth_req_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SAE, AUTH_FSM_SAE_AUTH_RSP, (STATE_MACHINE_FUNC) auth_fsm_sae_auth_rsp_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SAE, AUTH_FSM_PEER_DEAUTH, (STATE_MACHINE_FUNC)auth_fsm_peer_deauth_action);
	StateMachineSetAction(Sm, AUTH_FSM_WAIT_SAE, AUTH_FSM_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)auth_fsm_auth_timeout_action);
#endif /*DOT11_SAE_SUPPORT */

	wdev->auth_machine.CurrState = AUTH_FSM_IDLE;
}

VOID auth_fsm_reset(struct wifi_dev *wdev)
{
	auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
}

