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
	sec.h

	Abstract:

	Revision History:
	Who                     When                    What
	--------        ----------              ----------------------------------------------
	Name            Date                    Modification logs
*/

#ifndef __FSM_AUTH_H__
#define __FSM_AUTH_H__

struct _RTMP_ADAPTER;

VOID auth_fsm_init(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STATE_MACHINE *Sm, STATE_MACHINE_FUNC Trans[]);
VOID auth_fsm_reset(struct wifi_dev *wdev);
BOOLEAN auth_fsm_state_transition(struct wifi_dev *wdev, ULONG next_state, const char *caller);
VOID auth_fsm_mlme_deauth_req_action(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
VOID auth_fsm_mlme_auth_req_action(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);

void ap_send_broadcast_deauth(void *ad_obj, struct wifi_dev *wdev);
void ap_send_unicast_deauth(void *ad_obj, struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, bool protect);


#ifdef CONFIG_AP_SUPPORT
VOID ap_auth_init(struct wifi_dev *wdev);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
VOID sta_auth_init(struct wifi_dev *wdev);
#endif /* CONFIG_STA_SUPPORT */

struct _auth_api_ops {
	VOID (*mlme_auth_req_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*auth_timeout_action)(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*peer_auth_rsp_at_seq2_action)(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*peer_auth_rsp_at_seq4_action)(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*peer_auth_req_action)(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*peer_auth_confirm_action)(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*mlme_deauth_req_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*peer_deauth_action)(PRTMP_ADAPTER pAd, PMLME_QUEUE_ELEM Elem);
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	VOID (*sae_auth_req_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*sae_auth_rsp_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
#endif /*DOT11_SAE_SUPPORT */
};


#endif /* __FSM_AUTH_H__ */
