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

*/

#include "rt_config.h"

static VOID wtbl_tdd_handle_pending_pkt(struct _RTMP_ADAPTER *pAd, UCHAR action, UCHAR wcid)
{
    QUEUE_HEADER *pQue = NULL;
    QUEUE_ENTRY *pQEntry = NULL;
    UINT count, idx = 0;
    MAC_TABLE_ENTRY *pCheckEntry = NULL;
    PACKET_INFO pkt_info;
    UCHAR *pkt_va;
    UINT pkt_len;

    if (action == WTBL_TDD_ACTION_TX) {
		RTMP_SEM_LOCK(&pAd->wtblTddInfo.txPendingListLock);
		pQue = &pAd->wtblTddInfo.txPendingList;
		count = pAd->wtblTddInfo.txPendingList.Number;
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): <--- count %d\n", __func__, count);

		/* swap-in mac entry sending packet firstly */
		while (pQue->Head) {
			pQEntry = RemoveHeadQueue(pQue);

			RTMP_QueryPacketInfo(QUEUE_ENTRY_TO_PACKET(pQEntry), &pkt_info, &pkt_va, &pkt_len);
			pCheckEntry = &pAd->MacTab.Content[wcid];

			if (pCheckEntry && NdisEqualMemory(pCheckEntry->Addr, pkt_va, MAC_ADDR_LEN)) {
				send_data_pkt(pAd, pCheckEntry->wdev, QUEUE_ENTRY_TO_PACKET(pQEntry));
				pAd->wtblTddInfo.nodeInfo[pCheckEntry->wtblTddCtrl.ConnectTime].pktCnt++;
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s(): 1. sending with Data packt wcid:%d\n", __func__, pCheckEntry->wcid);
			} else {
				InsertTailQueue(pQue, pQEntry);
			}

			/* check the list for each once and exit */
			idx++;
			if (idx == count)
				break;
		}

		/* then sending all about remain in packet pending queue */
		idx = 0;
		while (pQue->Head) {
			pQEntry = RemoveHeadQueue(pQue);
			RTMP_QueryPacketInfo(QUEUE_ENTRY_TO_PACKET(pQEntry), &pkt_info, &pkt_va, &pkt_len);
			pCheckEntry = MacTableLookup(pAd, pkt_va);
			if (pCheckEntry && NdisEqualMemory(pCheckEntry->Addr, pkt_va, MAC_ADDR_LEN)) {
				send_data_pkt(pAd, pCheckEntry->wdev, QUEUE_ENTRY_TO_PACKET(pQEntry));
				pAd->wtblTddInfo.nodeInfo[pCheckEntry->wtblTddCtrl.ConnectTime].pktCnt++;
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s(): 2. sending with Data packt wcid:%d\n", __func__, pCheckEntry->wcid);
			} else {
				InsertTailQueue(pQue, pQEntry);
			}

			/* check the list for each once and exit */
			idx++;
			if (idx == count)
				break;
		}
		RTMP_SEM_UNLOCK(&pAd->wtblTddInfo.txPendingListLock);
    }
}

static VOID wtbl_tdd_swap_req_action(struct _RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MAC_TABLE_ENTRY *pInActEntry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	PWTBL_TDD_MSG msg;
	QUEUE_HEADER *pEventQue = NULL;
	QUEUE_ENTRY *pQEntry = NULL;
	UINT16 inActWcid = WCID_INVALID;
	UINT16 ActWcid = WCID_INVALID;
	UCHAR SegIdx = 0;
	UCHAR count = 0;
	UCHAR times = 0;

	count = pAd->wtblTddInfo.swapReqEventQue.Number;
	pEventQue = &pAd->wtblTddInfo.swapReqEventQue;

	MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG, ("%s(): ---> pEventQue=%p, count=%d\n", __func__, pEventQue, count));

	while (pEventQue->Head) {
		RTMP_SEM_LOCK(&pAd->wtblTddInfo.swapReqEventQueLock);
		pQEntry = RemoveHeadQueue(pEventQue);
		RTMP_SEM_UNLOCK(&pAd->wtblTddInfo.swapReqEventQueLock);

		times++;
		msg  = (PWTBL_TDD_MSG)pQEntry;
		pInActEntry = WtblTdd_InactiveList_Lookup(pAd, msg->addr);

		if (!pInActEntry) {
			MTWF_DBG(NULL, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): <--- !pInActEntry\n", __func__);
			goto ret;
		}

		MT_WTBL_TDD_LOG(pAd, WTBL_TDD_DBG_STATE_FLAG, ("%s(): ---> pInActEntry=%p, pInActEntry->wtblTddCtrl.state=%d\n", __func__, pInActEntry, pInActEntry->wtblTddCtrl.state));

		if ((pInActEntry->wtblTddCtrl.state != WTBL_TDD_STA_SWAP_IN_ING) &&
			(pInActEntry->wtblTddCtrl.state != WTBL_TDD_STA_SWAP_IN_DONE)) {
			pInActEntry->wtblTddCtrl.state = WTBL_TDD_STA_SWAP_IN_ING;
			inActWcid = pInActEntry->wtblTddCtrl.LastExtIdx;
			SegIdx = pInActEntry->wtblTddCtrl.SegIdx;
			ActWcid = WtblTdd_ActiveList_SwapIn(pAd, pInActEntry, &pAd->MacTab.tr_entryExt[SegIdx][inActWcid]);
			pEntry = &pAd->MacTab.Content[ActWcid];
			pEntry->wtblTddCtrl.state = WTBL_TDD_STA_SWAP_IN_DONE;
			wtbl_tdd_handle_pending_pkt(pAd, msg->action, ActWcid);
		}
ret:
		if (msg)
			os_free_mem(msg);
	}
}

static VOID wtbl_tdd_recycle_req_action(struct _RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
}

static BOOLEAN wtbl_tdd_fsm_msg_checker(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem)
{
	return FALSE;
}

static VOID wtbl_tdd_fsm_msg_invalid_state(struct _RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
}


VOID wtbl_tdd_fsm_init(struct _RTMP_ADAPTER *pAd, UCHAR wcid, STATE_MACHINE *Sm, STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, WTBL_TDD_FSM_MAX_STATE, WTBL_TDD_FSM_MAX_MSG,
					 (STATE_MACHINE_FUNC)wtbl_tdd_fsm_msg_invalid_state, WTBL_TDD_FSM_IDLE, WTBL_TDD_FSM_BASE);
	StateMachineSetMsgChecker(Sm, (STATE_MACHINE_MSG_CHECKER)wtbl_tdd_fsm_msg_checker);
	StateMachineSetAction(Sm, WTBL_TDD_FSM_IDLE, WTBL_TDD_FSM_SWAP_REQ, (STATE_MACHINE_FUNC)wtbl_tdd_swap_req_action);
	StateMachineSetAction(Sm, WTBL_TDD_FSM_SWAP_DONE, WTBL_TDD_FSM_RECYCLE_REQ, (STATE_MACHINE_FUNC)wtbl_tdd_recycle_req_action);
}
