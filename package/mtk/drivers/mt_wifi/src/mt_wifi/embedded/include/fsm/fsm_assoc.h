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

#ifndef __FSM_ASSOC_H__
#define __FSM_ASSOC_H__

struct _RTMP_ADAPTER;

VOID assoc_fsm_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STATE_MACHINE *Sm, STATE_MACHINE_FUNC Trans[]);
VOID assoc_fsm_reset(struct wifi_dev *wdev);
BOOLEAN assoc_fsm_state_transition(struct wifi_dev *wdev, ULONG NextState);

#ifdef CONFIG_AP_SUPPORT
VOID ap_assoc_init(struct wifi_dev *wdev);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
VOID sta_assoc_init(struct wifi_dev *wdev);
#endif /* CONFIG_STA_SUPPORT */

struct _assoc_api_ops {
	/* STA Role */
	VOID (*mlme_assoc_req_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*peer_assoc_rsp_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*mlme_assoc_req_timeout_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);

	VOID (*mlme_reassoc_req_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*peer_reassoc_rsp_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*mlme_reassoc_req_timeout_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);

	/* AP Role */
	VOID (*peer_assoc_req_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*peer_reassoc_req_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);

	/* CMM Role */
	VOID (*mlme_disassoc_req_action)(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*peer_disassoc_action)(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*mlme_disassoc_req_timeout_action)(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);


};

#endif /* __FSM_ASSOC_H__ */

