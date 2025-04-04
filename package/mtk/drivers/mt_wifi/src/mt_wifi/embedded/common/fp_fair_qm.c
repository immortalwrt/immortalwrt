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

*/

#include "rt_config.h"

#ifdef FQ_SCH_SUPPORT

extern INT32 fq_deq_data_pkt(RTMP_ADAPTER *pAd, TX_BLK *tx_blk, INT32 max_cnt, struct dequeue_info *info);
extern INT fq_del_report(RTMP_ADAPTER *pAd, struct dequeue_info *info);

extern INT fq_enq_req(RTMP_ADAPTER *pAd, NDIS_PACKET *pkt, UCHAR qidx,
			STA_TR_ENTRY *tr_entry, QUEUE_HEADER *pPktQueue);

extern UCHAR WMM_UP2AC_MAP[8];

static INT fp_fair_deq_req(RTMP_ADAPTER *pAd, INT cnt, struct dequeue_info *info);

INT32 fp_fair_enq_dataq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 idx;
	struct tm_ops *tm_ops = pAd->tm_qm_ops;
	UCHAR the_q_idx;
	UINT16 wcid;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID)
		goto error;

	the_q_idx = WMM_UP2AC_MAP[q_idx];
	wcid = RTMP_GET_PACKET_WCID(pkt);
	tr_entry = &tr_ctl->tr_entry[wcid];

	if (!fq_enq_req(pAd, pkt, the_q_idx, tr_entry, NULL))
		goto error;

	if (cap->multi_token_ques_per_band)
		idx =  HcGetBandByWdev(wdev);
	else
		idx = 0;

	if (idx > 1)
		goto error;

	tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);

	return NDIS_STATUS_SUCCESS;
error:
	pAd->tr_ctl.tr_cnt.tx_sw_dataq_drop++;
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

static INT fp_fair_deq_req(RTMP_ADAPTER *pAd, INT cnt, struct dequeue_info *info)
{
	CHAR deq_qid = 0, start_q = 0, end_q = 0;
	UINT16 deq_wcid = 0;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;
	unsigned long IrqFlags = 0;
	unsigned int quota = 0;

	if (!info->inited) {
		if (info->target_que < WMM_QUE_NUM) {
			info->start_q = info->target_que;
			info->end_q = info->target_que;
		} else {
			info->start_q = (WMM_QUE_NUM - 1);
			info->end_q = 0;
		}

		info->cur_q = info->start_q;

		/*
		 * a. for specific wcid, quota number "cnt" stored in info->pkt_cnt and shared by 4 ac queue
		 * b. for all wcid, quota stored in info->pkt_cnt and info->q_max_cnt[ac_index] and each ac has quota number "cnt"
		 *    shared by all wcid
		 */
		if (IS_TR_WCID_VALID(pAd, info->target_wcid)) {
			info->pkt_cnt = cnt;
			info->full_qid[0] = FALSE;
			info->full_qid[1] = FALSE;
			info->full_qid[2] = FALSE;
			info->full_qid[3] = FALSE;
		} else {
			info->q_max_cnt[0] = cnt;
			info->q_max_cnt[1] = cnt;
			info->q_max_cnt[2] = cnt;
			info->q_max_cnt[3] = cnt;
		}

		info->inited = 1;
	}

	start_q = info->cur_q;
	end_q = info->end_q;

	/*
	 * decide cur_wcid and cur_que under info->pkt_cnt > 0 condition for specific wcid
	 * cur_wcid = info->target_wcid
	 * cur_que = deq_qid
	 * deq_que has two value, one come from info->target_que for specific ac queue,
	 * another go to check if tr_entry[deq_qid].number > 0 from highest priority
	 * to lowest priority ac queue for all ac queue
	 */
	if (IS_TR_WCID_VALID(pAd, info->target_wcid)) {
		if (info->pkt_cnt <= 0) {
			info->status = NDIS_STATUS_FAILURE;
			goto done;
		}

		deq_wcid = info->target_wcid;

		if (info->target_que >= WMM_QUE_NUM) {
			tr_entry = &tr_ctl->tr_entry[deq_wcid];

			for (deq_qid = start_q; deq_qid >= end_q; deq_qid--) {
				if (info->full_qid[deq_qid] == FALSE && tr_entry->tx_queue[deq_qid].Number)
					break;
			}
		} else if (info->full_qid[info->target_que] == FALSE)
			deq_qid = info->target_que;
		else {
			info->status = NDIS_STATUS_FAILURE;
			goto done;
		}

		if (deq_qid >= 0) {
			info->cur_q = deq_qid;
			info->cur_wcid = deq_wcid;
		} else
			info->status = NDIS_STATUS_FAILURE;

		goto done;
	}

	/*
	 * decide cur_wcid and cur_que for all wcid
	 * cur_wcid = deq_wcid
	 * deq_wcid need to check tx_swq_fifo from highest priority to lowest priority ac queues
	 * and come from tx_swq_fifo.swq[tx_deq_fifo.deqIdx]
	 * cur_que = deq_qid upon found a wcid
	 */
	for (deq_qid = start_q; deq_qid >= end_q; deq_qid--) {
		RTMP_IRQ_LOCK(&pAd->tx_swq_lock[deq_qid], IrqFlags);

		deq_wcid = fq_del_list(pAd, info, deq_qid, &quota);

		RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[deq_qid], IrqFlags);

		if (deq_wcid == 0) {
			MTWF_DBG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "%s():tx_swq[%d] emtpy!\n", __func__, deq_qid);
			info->q_max_cnt[deq_qid] = 0;
			continue;
		}

		if (info->q_max_cnt[deq_qid] > 0) {
			info->cur_q = deq_qid;
			info->cur_wcid = deq_wcid;
			info->pkt_cnt = quota;
			break;
		}
	}

	if (deq_qid < end_q) {
		info->cur_q = deq_qid;
		info->status = NDIS_STATUS_FAILURE;
	}

done:
	return TRUE;
}
static NDIS_PACKET *fp_fair_get_data_tx_element(RTMP_ADAPTER *pAd, struct dequeue_info
						*deq_info, STA_TR_ENTRY *tr_entry)
{
	PQUEUE_ENTRY q_entry;
	UCHAR q_idx = deq_info->cur_q;

	q_entry = RemoveHeadQueue(&tr_entry->tx_queue[q_idx]);
	TR_ENQ_COUNT_DEC(tr_entry);
	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}

static NDIS_PACKET *fp_fair_first_data_tx_element(RTMP_ADAPTER *pAd, struct dequeue_info
						*deq_info, STA_TR_ENTRY *tr_entry)
{
	PQUEUE_ENTRY q_entry;
	UCHAR q_idx = deq_info->cur_q;

	q_entry = tr_entry->tx_queue[q_idx].Head;
	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}
static NDIS_PACKET *fp_fair_get_mgmt_tx_element(RTMP_ADAPTER *pAd, UINT8 idx)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock[idx]);

	q_entry = RemoveHeadQueue(&pAd->mgmt_que[idx]);
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock[idx]);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}

static NDIS_PACKET *fp_fair_first_mgmt_tx_element(RTMP_ADAPTER *pAd, UINT8 idx)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock[idx]);
	q_entry = pAd->mgmt_que[idx].Head;
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock[idx]);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}
VOID fp_fair_tx_pkt_deq_func(RTMP_ADAPTER *pAd, UINT8 idx)
{
	TX_BLK tx_blk, *pTxBlk;
	NDIS_PACKET *pkt = NULL;
	UINT16 Wcid = 0;
	UCHAR q_idx = 0;
	INT ret = 0;
	UINT32 KickRingBitMap = 0;
	UINT32 hif_idx = 0;
	struct wifi_dev *wdev = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev_ops *wdev_ops;
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	BOOLEAN need_schedule = (pAd->tx_dequeue_scheduable ? TRUE : FALSE);
	UCHAR user_prio = 0;
	BOOLEAN data_turn = FALSE;
	struct dequeue_info deq_info = {0};
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	/*TODO and FIXME, tx_ring_size shall reference from ring_size*/
	INT32 max_cnt = GET_DATA_TX_RING_SIZE(cap);
	UINT8 num_of_tx_ring = hif_get_tx_res_num(pAd->hdev_ctrl);
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	deq_info.target_wcid = WCID_ALL;
	deq_info.target_que = WMM_NUM_OF_AC;

	while (need_schedule) {
		if (data_turn == FALSE)	{
			pkt = fp_fair_first_mgmt_tx_element(pAd, idx);
			if (!pkt) {
				os_zero_mem(&deq_info, sizeof(struct dequeue_info));
				deq_info.status = NDIS_STATUS_SUCCESS;
				deq_info.target_wcid = WCID_ALL;
				deq_info.target_que = WMM_NUM_OF_AC;
				deq_info.inited = 0;
				fp_fair_deq_req(pAd, max_cnt, &deq_info);
				if (deq_info.status == NDIS_STATUS_FAILURE)
					break;

				data_turn = TRUE;
				q_idx = deq_info.cur_q;
				Wcid = deq_info.cur_wcid;

				deq_info.deq_pkt_cnt = 0;
				tr_entry = &tr_ctl->tr_entry[Wcid];
				RTMP_SEM_LOCK(&tr_entry->txq_lock[q_idx]);
				pkt = fp_fair_first_data_tx_element(pAd, &deq_info, tr_entry);
				if (!pkt)
					goto DATA_EXIT;
			}
		} else {
			pkt = fp_fair_first_data_tx_element(pAd, &deq_info, tr_entry);
			if (!pkt)
				goto DATA_EXIT;
		}

		NdisZeroMemory((UCHAR *)&tx_blk, sizeof(TX_BLK));
		pTxBlk = &tx_blk;

		wdev = wdev_search_by_pkt(pAd, pkt);
		if (!wdev)
			continue;

		wdev_ops = wdev->wdev_ops;
		pTxBlk->resource_idx = hif_get_resource_idx(pAd->hdev_ctrl, wdev, 0, 0);
		if (arch_ops->check_hw_resource(pAd, wdev, pTxBlk->resource_idx)) {
			if (data_turn == FALSE)
				break;
			if (IS_TR_WCID_VALID(pAd, deq_info.target_wcid))
				deq_info.full_qid[q_idx] = TRUE;
			else
				deq_info.q_max_cnt[q_idx] = 0;
			goto DATA_EXIT;
		}

		if (data_turn == FALSE)	{
			pkt = fp_fair_get_mgmt_tx_element(pAd, idx);
			if (!pkt)
				continue;
			Wcid = RTMP_GET_PACKET_WCID(pkt);
		} else {
			pkt = fp_fair_get_data_tx_element(pAd, &deq_info, tr_entry);
			deq_info.deq_pkt_cnt++;
		}

		if (data_turn == FALSE) {
			/*if wcid is out of MAC table size, free it*/
			if (Wcid >= wtbl_max_num) {
				MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "%s(): WCID is invalid\n", __func__);
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
				continue;
			}
			tr_entry = &tr_ctl->tr_entry[Wcid];
		}
		if (wdev)
			pTxBlk->wdev = wdev;
		else {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			continue;
		}
		pTxBlk->TotalFrameNum = 1;
		pTxBlk->TotalFragNum = 1;
		pTxBlk->tr_entry = tr_entry;
		user_prio = RTMP_GET_PACKET_UP(pkt);
		pTxBlk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
		pTxBlk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		pTxBlk->pPacket = pkt;
		pTxBlk->TxFrameType = tx_pkt_classification(pAd, pTxBlk->pPacket, pTxBlk);
		pTxBlk->HeaderBuf = hif_get_tx_buf(pAd->hdev_ctrl, pTxBlk, pTxBlk->resource_idx, pTxBlk->TxFrameType);
		InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));

		ret = wdev_ops->tx_pkt_handle(pAd, wdev, pTxBlk);

		if (!ret)
			KickRingBitMap |= (1 << pTxBlk->resource_idx);

		if (data_turn == TRUE)	{
			if (deq_info.q_max_cnt[q_idx] > 0)
				deq_info.q_max_cnt[q_idx]--;
			if (IS_TR_WCID_VALID(pAd, deq_info.target_wcid))
				deq_info.pkt_cnt--;
			max_cnt -= deq_info.deq_pkt_cnt;
			if (max_cnt <= 0)
				goto DATA_EXIT;
			if ((deq_info.q_max_cnt[q_idx] <= 0) ||
				(deq_info.pkt_cnt <= deq_info.deq_pkt_cnt)) {
				data_turn = FALSE;
				RTMP_SEM_UNLOCK(&tr_entry->txq_lock[q_idx]);
				fq_del_report(pAd, &deq_info);
			}
		}
	}
DATA_EXIT:
	if (data_turn == TRUE) {
		RTMP_SEM_UNLOCK(&tr_entry->txq_lock[q_idx]);
		fq_del_report(pAd, &deq_info);
	}

	while (KickRingBitMap != 0 && hif_idx < num_of_tx_ring) {
		if (KickRingBitMap & 0x1) {
			hif_kickout_data_tx(pAd, pTxBlk, idx);
#ifdef CONFIG_TP_DBG
			tp_dbg->IoWriteTx++;
#endif
		}

		KickRingBitMap >>= 1;
		hif_idx++;
	}
}


struct qm_ops fp_fair_qm_ops = {0};
#else
struct qm_ops fp_fair_qm_ops = {
	.init = NULL,
	.exit = NULL,
};
VOID fp_fair_tx_pkt_deq_func(RTMP_ADAPTER *pAd, UINT8 idx)
{
	return;
}
#endif /* FQ_SCH_SUPPORT */


