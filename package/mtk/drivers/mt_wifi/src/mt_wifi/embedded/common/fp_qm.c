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

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
#include <net/ip.h>
static const UINT8 ac_queue_to_up[4] = {
	1 /* AC_BK */, 0 /* AC_BE */, 5 /* AC_VI */, 7 /* AC_VO */
};
#endif

static INT32 fp_tx_flow_ctl(RTMP_ADAPTER *pAd, BOOLEAN en)
{
	INT32 ret = 0;
	UINT8 i;
	struct fp_tx_flow_control *flow_ctl = &pAd->fp_tx_flow_ctl;
	struct wifi_dev *wdev = NULL;

	if (en)
		set_bit(0, &flow_ctl->flag);
	else
		clear_bit(0, &flow_ctl->flag);

	for (i = 0; i < 2; i++) {
		OS_SPIN_LOCK_BH(&pAd->fp_que_lock[i]);
		while (1) {
			wdev = DlListFirst(&flow_ctl->TxBlockDevList[i],
								struct wifi_dev, tx_block_list);

			if (!wdev)
				break;

			DlListDel(&wdev->tx_block_list);
			RTMP_OS_NETDEV_WAKE_QUEUE(wdev->if_dev);
		}

		OS_SPIN_UNLOCK_BH(&pAd->fp_que_lock[i]);
	}

	return ret;
}

static INT32 fp_tx_flow_block(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
						UINT8 state, BOOLEAN block, UINT8 q_idx)
{
	INT32 ret = 0;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	struct fp_tx_flow_control *flow_ctl = &pAd->fp_tx_flow_ctl;
	struct wifi_dev *wdev_block = NULL;

	if (block && test_bit(0, &flow_ctl->flag)) {
		DlListForEach(wdev_block, &flow_ctl->TxBlockDevList[q_idx], struct wifi_dev, tx_block_list) {
			if (wdev_block == wdev)
				return ret;
		}
		RTMP_OS_NETDEV_STOP_QUEUE(wdev->if_dev);
		tr_cnt->net_if_stop_cnt++;
		DlListAddTail(&flow_ctl->TxBlockDevList[q_idx], &wdev->tx_block_list);
		set_bit(state, &flow_ctl->TxFlowBlockState[q_idx]);
	} else {
		if (test_and_clear_bit(state, &flow_ctl->TxFlowBlockState[q_idx])) {
			while (1) {
				wdev_block = DlListFirst(&flow_ctl->TxBlockDevList[q_idx],
									struct wifi_dev, tx_block_list);

				if (!wdev_block)
					break;

				DlListDel(&wdev_block->tx_block_list);
				RTMP_OS_NETDEV_WAKE_QUEUE(wdev_block->if_dev);
			}
		}
	}

	return ret;
}

inline BOOLEAN fp_get_queue_state(QUEUE_HEADER *que)
{
	return test_bit(0, &que->state);
}

inline INT fp_set_queue_state(QUEUE_HEADER *que, BOOLEAN state)
{
	if (state == TX_QUE_LOW)
		set_bit(0, &que->state);
	else
		clear_bit(0, &que->state);

	return NDIS_STATUS_SUCCESS;
}

static NDIS_PACKET *fp_get_tx_element(RTMP_ADAPTER *pAd, UINT8 idx)
{
	PQUEUE_ENTRY q_entry;

	OS_SPIN_LOCK_BH(&pAd->mgmt_post_que_lock[idx]);
	q_entry = RemoveHeadQueue(&pAd->mgmt_post_que[idx]);
	OS_SPIN_UNLOCK_BH(&pAd->mgmt_post_que_lock[idx]);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	OS_SPIN_LOCK_BH(&pAd->fp_post_que_lock[idx]);
	q_entry = RemoveHeadQueue(&pAd->fp_post_que[idx]);
	OS_SPIN_UNLOCK_BH(&pAd->fp_post_que_lock[idx]);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static INT32 fp_check_tx_resource(RTMP_ADAPTER *pAd,
						struct wifi_dev *wdev, UINT8 resource_idx)
{
	INT32 ret = 0;
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	PKT_TOKEN_CB *cb = hif->PktTokenCb;
	UINT8 band_idx = HcGetBandByWdev(wdev);
	struct token_tx_pkt_queue *que = token_tx_get_queue_by_band(cb, band_idx);
#endif

	ret = asic_check_hw_resource(pAd, wdev, resource_idx);
#ifdef RTMP_MAC_PCI
	if (ret == ERROR_NO_RING) {
		hif_free_txd(pAd, resource_idx);
		ret = asic_check_hw_resource(pAd, wdev, resource_idx);

		if (ret != ERROR_NO_RING) {
			pci_set_resource_state(pAd, resource_idx, TX_RING_HIGH);
			if (ret == ERROR_NO_TOKEN) {
				pci_rx_event_dma_done_handle(pAd, hif->host_msdu_id_rpt_idx);
				ret = asic_check_hw_resource(pAd, wdev, resource_idx);
				if (ret != ERROR_NO_TOKEN)
					token_tx_set_state(que, TX_TOKEN_HIGH);
			}
		}
	} else if (ret == ERROR_NO_TOKEN) {
		pci_rx_event_dma_done_handle(pAd, hif->host_msdu_id_rpt_idx);
		ret = asic_check_hw_resource(pAd, wdev, resource_idx);
		if (ret != ERROR_NO_TOKEN)
			token_tx_set_state(que, TX_TOKEN_HIGH);
	}
#endif

	if (ret)
		ret = ERROR_NO_TX_RESOURCE;

	return ret;
}

static NDIS_PACKET *fp_first_tx_element(RTMP_ADAPTER *pAd, UINT8 idx, BOOLEAN *from_mgmt)
{
	PQUEUE_ENTRY q_entry;
	NDIS_PACKET *pkt = NULL;
	struct wifi_dev *wdev = NULL;

	OS_SPIN_LOCK_BH(&pAd->mgmt_post_que_lock[idx]);
	q_entry = pAd->mgmt_post_que[idx].Head;
	OS_SPIN_UNLOCK_BH(&pAd->mgmt_post_que_lock[idx]);

	if (q_entry) {
		*from_mgmt = TRUE;
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	}

	OS_SPIN_LOCK_BH(&pAd->fp_post_que_lock[idx]);
	q_entry = pAd->fp_post_que[idx].Head;
	OS_SPIN_UNLOCK_BH(&pAd->fp_post_que_lock[idx]);

	if (q_entry) {
		pkt = QUEUE_ENTRY_TO_PACKET(q_entry);
		wdev = wdev_search_by_pkt(pAd, pkt);

		if (wdev) {
			if (wdev->forbid_data_tx & (0x1 << MSDU_FORBID_CHANNEL_MISMATCH))
				return NULL;
			else
				return pkt;
		} else
			return NULL;
	}
	else
		return NULL;
}

static VOID fp_queue_deep_counting(RTMP_ADAPTER *pAd, struct tr_counter *tr_cnt, UINT8 idx)
{
	UINT32 queue_num = 0;

	queue_num += pAd->mgmt_post_que[idx].Number;
	queue_num += pAd->fp_post_que[idx].Number;

	if (queue_num == 0)
		return;

	if (queue_num > (sizeof(tr_cnt->queue_deep_cnt) / sizeof(UINT32)))
		queue_num = (sizeof(tr_cnt->queue_deep_cnt) / sizeof(UINT32));

	tr_cnt->queue_deep_cnt[queue_num - 1]++;
}


#ifdef DSCP_PRI_SUPPORT
INT8 get_dscp_mapped_priority(
		IN  PRTMP_ADAPTER pAd,
		IN  PNDIS_PACKET  pPkt)
{
	INT8 pri = -1;
	UINT8 dscpVal = 0;
	PUCHAR	pPktHdr = NULL;
	UINT16 protoType;
	struct wifi_dev *wdev;

	pPktHdr = GET_OS_PKT_DATAPTR(pPkt);
	if (!pPktHdr)
		return pri;

	protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + 12)));

	pPktHdr += LENGTH_802_3;

	if (protoType <= 1500) {
		/* 802.3, 802.3 LLC: DestMAC(6) + SrcMAC(6) + Length (2) + DSAP(1) + SSAP(1) + Control(1) + */
		/* if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.*/
		/*	=> + SNAP (5, OriginationID(3) + etherType(2)) */
		/* else */
		/*	=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it */
		if (pPktHdr[0] == 0xAA && pPktHdr[1] == 0xAA && pPktHdr[2] == 0x03) {
			pPktHdr += 6; /* Advance to type LLC 3byte + SNAP OriginationID 3 Byte*/
			protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr)));
		} else {
			return pri;
		}
	}

	/* If it's a VLAN packet, get the real Type/Length field.*/
	if (protoType == ETH_TYPE_VLAN) {
		pPktHdr += 2; /* Skip the VLAN Header.*/
		protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr)));
	}

	switch (protoType) {
	case 0x0800:
		dscpVal = ((pPktHdr[1] & 0xfc) >> 2);
		break;
	case 0x86DD:
			dscpVal = (((pPktHdr[0] & 0x0f) << 2) | ((pPktHdr[1] & 0xc0) >> 6));
		break;
	default:
		return pri;
	}

	if (dscpVal <= 63) {
		UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPkt);

		wdev = get_wdev_by_idx(pAd, wdev_idx);
		if (!wdev)
			return pri;
		pri = pAd->ApCfg.MBSSID[wdev->func_idx].dscp_pri_map[dscpVal];
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,  "ApIdx:%d dscp value:%d local PRI:%d\n",
			wdev->func_idx, dscpVal, pri);
	}

	return pri;
}
#endif /*DSCP_PRI_SUPPORT*/

DECLARE_TIMER_FUNCTION(fp_que_agg_timeout);

VOID fp_que_agg_timeout(PVOID SystemSpecific1, PVOID func_context, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct tx_delay_control *tx_delay_ctl = (struct tx_delay_control *)func_context;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)tx_delay_ctl->priv;
	struct tm_ops *tm_ops = pAd->tm_qm_ops;
	UINT8 idx = tx_delay_ctl->idx;

	if (pAd->tx_dequeue_scheduable[idx]) {
		tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
		tx_delay_ctl->force_deq = TRUE;
		tx_delay_ctl->que_agg_timer_running = FALSE;
	}
}

BUILD_TIMER_FUNCTION(fp_que_agg_timeout);

static BOOLEAN fp_ge_tx_deq_delay(RTMP_ADAPTER *pAd, UINT8 idx)
{
	NDIS_PACKET *pkt = NULL;
	BOOLEAN is_udp;
	PQUEUE_HEADER que;
	UINT32 q_nums = 0;
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl[idx];

	if (!(tx_delay_ctl->que_agg_en) ||
		(tx_delay_ctl->force_deq) ||
		pAd->mgmt_post_que[idx].Number > 0) {
		return FALSE;
	}

	OS_SPIN_LOCK_BH(&pAd->fp_post_que_lock[idx]);
	que = &pAd->fp_post_que[idx];
	if (que->Head)
		pkt = QUEUE_ENTRY_TO_PACKET(que->Head);
	OS_SPIN_UNLOCK_BH(&pAd->fp_post_que_lock[idx]);

	if (pkt) {
		q_nums = que->Number;
		is_udp = is_udp_packet(pAd, pkt);
	}

	if ((q_nums > 0) &&
		(q_nums < tx_delay_ctl->tx_process_batch_cnt) &&
		!is_udp) {

			if (!tx_delay_ctl->que_agg_timer_running) {
				RTMPSetTimer(&tx_delay_ctl->que_agg_timer, tx_delay_ctl->que_agg_timeout_value / 1000);
				tx_delay_ctl->que_agg_timer_running = TRUE;
			}

			return TRUE;
	}

	return FALSE;
}

static VOID fp_merge_post_que(QUEUE_HEADER *que, QUEUE_HEADER *post_que)
{
	if (post_que->Number == 0) {
		post_que->Head = que->Head;
		post_que->Tail = que->Tail;
		post_que->Number = que->Number;
	} else {
		if (que->Number) {
			post_que->Tail->Next = que->Head;
			post_que->Tail = que->Tail;
			post_que->Number += que->Number;
		}
	}

	post_que->state = que->state;
	que->Head = NULL;
	que->Tail = NULL;
	que->Number = 0;
}

static VOID fp_init_post_que(RTMP_ADAPTER *pAd, UINT8 idx)
{
	OS_SPIN_LOCK_BH(&pAd->mgmt_que_lock[idx]);
	fp_merge_post_que(&pAd->mgmt_que[idx], &pAd->mgmt_post_que[idx]);
	OS_SPIN_UNLOCK_BH(&pAd->mgmt_que_lock[idx]);

	OS_SPIN_LOCK_BH(&pAd->fp_que_lock[idx]);
	fp_merge_post_que(&pAd->fp_que[idx], &pAd->fp_post_que[idx]);
	OS_SPIN_UNLOCK_BH(&pAd->fp_que_lock[idx]);
}

VOID fp_tx_pkt_deq_func(RTMP_ADAPTER *pAd, UINT8 idx)
{
	TX_BLK blk, *tx_blk;
	NDIS_PACKET *pkt = NULL;
	UINT16 wcid = 0;
	INT ret = 0;
	UINT32 KickRingBitMap = 0;
	struct wifi_dev *wdev = NULL;
	STA_TR_ENTRY *tr_entry;
	struct wifi_dev_ops *wdev_ops;
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;
	BOOLEAN need_schedule = (pAd->tx_dequeue_scheduable[idx] ? TRUE : FALSE);
	UCHAR user_prio;
	UINT8 hif_idx = 0;
	UINT16 tx_process_cnt = 0;
	UINT8 num_of_tx_ring = hif_get_tx_res_num(pAd->hdev_ctrl);
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tm_ops *tm_ops = pAd->tm_qm_ops;

#ifdef CONFIG_TX_DELAY
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct tx_delay_control *tx_delay_ctl = &tr_ctl->tx_delay_ctl[idx];
#endif
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &tr_ctl->tp_dbg;
#endif
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	UINT32 cpu_id;
#ifdef SW_CONNECT_SUPPORT
	/* in this point , still S/W level wcid check */
	UINT16 wtbl_max_num = SW_ENTRY_MAX_NUM(pAd);
	BOOLEAN bSw = FALSE;
#else /* SW_CONNECT_SUPPORT */
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
#endif /* !SW_CONNECT_SUPPORT */
	BOOLEAN deq_from_mgmt = FALSE;
	BOOLEAN reschedule_by_no_resource = FALSE;

	if (RTMP_TEST_FLAG(pAd, TX_FLAG_STOP_DEQUEUE)) {
		return;
	}

	fp_init_post_que(pAd, idx);

#ifdef CONFIG_TX_DELAY
	if (qm_ops->tx_deq_delay(pAd, idx)) {
		return;
	}
#endif

	fp_queue_deep_counting(pAd, tr_cnt, idx);

	while (need_schedule &&
			(tx_process_cnt < qm->max_tx_process_cnt)) {

		NdisZeroMemory((UCHAR *)&blk, sizeof(TX_BLK));
		tx_blk = &blk;

		pkt = fp_first_tx_element(pAd, idx, &deq_from_mgmt);

		if (!pkt) {
			break;
		}

		wdev = wdev_search_by_pkt(pAd, pkt);

		if (wdev) {

			wdev_ops = wdev->wdev_ops;
			tx_blk->resource_idx = hif_get_resource_idx(pAd->hdev_ctrl, wdev, 0, 0);

			ret = fp_check_tx_resource(pAd, wdev, tx_blk->resource_idx);

			if (ret == ERROR_NO_TX_RESOURCE) {
				if (deq_from_mgmt)
					reschedule_by_no_resource = TRUE;
				break;
			}

			pkt = fp_get_tx_element(pAd, idx);
			if (!pkt)
				continue;

			wcid = RTMP_GET_PACKET_WCID(pkt);

			/*if wcid is out of MAC table size, free it*/
			if (wcid >= wtbl_max_num) {
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "%s(): WCID is invalid\n", __func__);
				tr_cnt->tx_wcid_invalid++;
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
				continue;
			}

			tx_blk->wdev = wdev;
		} else {
			/*Dequeue and release the illegal pkt*/
			pkt = fp_get_tx_element(pAd, idx);
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			continue;
		}

		cpu_id = smp_processor_id();

		if (cpu_id < 4)
			tr_cnt->tx_deq_cpu_stat[idx][cpu_id]++;

		tr_entry = &tr_ctl->tr_entry[wcid];
#ifdef SW_CONNECT_SUPPORT
		if (RTMP_GET_PACKET_EAPOL(pkt))
			tr_entry->tx_deq_eap_cnt++;
		else {
			tr_entry->tx_deq_data_cnt++;

			if (RTMP_GET_PACKET_ARP(pkt))
				tr_entry->tx_deq_arp_cnt++;
		}

		if (RTMP_GET_PACKET_SW(pkt))
			bSw = TRUE;
#endif /* SW_CONNECT_SUPPORT */
		tx_blk->TotalFrameNum = 1;
		tx_blk->TotalFragNum = 1;
		tx_blk->tr_entry = tr_entry;
		user_prio = RTMP_GET_PACKET_UP(pkt);
		tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
		tx_blk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		tx_blk->pPacket = pkt;
		tx_blk->TxFrameType = tx_pkt_classification(pAd, tx_blk->pPacket, tx_blk);
		tx_blk->HeaderBuf = hif_get_tx_buf(pAd->hdev_ctrl, tx_blk, tx_blk->resource_idx, tx_blk->TxFrameType);
#ifdef VLAN_SUPPORT
		tx_blk->is_vlan = RTMP_GET_PACKET_VLAN(pkt);
#endif
#ifdef PER_PKT_CTRL_FOR_CTMR
		if(tx_blk->ApplyTid && (tx_blk->TidByHost != user_prio))
			ba_ori_session_start(pAd, tr_entry, tx_blk->TidByHost);
#endif

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		if (pAd->vow_dvt_en) {
			if (tx_blk->TxFrameType == TX_LEGACY_FRAME) {
				UINT8 *pData = GET_OS_PKT_DATAPTR(pkt);
				BOOLEAN is_ipv4 = ((*(pData+MAT_ETHER_HDR_LEN) >> 4 & 0x0F) == 4);
				if ((!RTMP_GET_PACKET_MGMT_PKT(pkt)) &&
					(!RTMP_GET_PACKET_HIGH_PRIO(pkt) && (is_ipv4))) {
					tx_blk->QueIdx = pAd->vow_sta_ac[wcid];

					(*(pData+MAT_ETHER_HDR_LEN+1)) &= ~0x0E0;
					(*(pData+MAT_ETHER_HDR_LEN+1)) |= ac_queue_to_up[pAd->vow_sta_ac[wcid]] << 5;
					ip_send_check((struct iphdr *)(pData + MAT_ETHER_HDR_LEN));
				}
			}
		}
#endif
		MEM_DBG_PKT_RECORD(pkt, 1<<4);

#ifdef DSCP_PRI_SUPPORT
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			/*handle DSCP-UP Mapping in CR4*/
		} else
#endif
		{
			/*
			* Get the Dscp value of the packet and if there is any mapping defined set the DscpMappedPri value
			*/
			if ((pAd->ApCfg.DscpPriMapSupport) && (!TX_BLK_TEST_FLAG(tx_blk, fTX_bApCliPacket)))
				tx_blk->DscpMappedPri = get_dscp_mapped_priority(pAd, pkt);
		}
#endif /*DSCP_PRI_SUPPORT*/
		ret = wdev_ops->tx_pkt_handle(pAd, wdev, tx_blk);


#ifdef DABS_QOS
		if ((pAd->dabs_qos_op & DABS_DBG_DLY_TIME) && (!ret) && (!RTMP_GET_PACKET_MGMT_PKT(pkt))
#ifdef SW_CONNECT_SUPPORT
			/*
				To fix use after free reference of pkt , that  ap_legacy_tx() ==> asic_hw_tx() return sucess, and let below hif_kickout_data_tx() can move up tx ring
				asic_hw_tx() :  call mt_ct_hw_tx() and tx_blk->FragIdx == TX_FRAG_ID_NO hit to release tx_blk->pPacket !
			*/
			&&(bSw == FALSE)
#endif /* SW_CONNECT_SUPPORT */
			) {
			dabs_host_delay(pAd, pkt);
		}
#endif

		if (!ret) {
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
			if (pAd->vow_dvt_en) {
#ifdef SW_CONNECT_SUPPORT
				if (bSw == FALSE)
#endif /* SW_CONNECT_SUPPORT */
				{
					if (tx_blk->TxFrameType == TX_LEGACY_FRAME) {
						if ((!RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) &&
						    (!RTMP_GET_PACKET_HIGH_PRIO(tx_blk->pPacket)))
							KickRingBitMap |= vow_clone_legacy_frame(pAd, tx_blk);
					}
				}
			}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

			KickRingBitMap |= (1 << tx_blk->resource_idx);
		}
		tx_process_cnt++;
	}

	while (KickRingBitMap != 0 && hif_idx < num_of_tx_ring) {
		if (KickRingBitMap & 0x1) {
			hif_kickout_data_tx(pAd, tx_blk, hif_idx);
#ifdef CONFIG_TP_DBG
			tp_dbg->IoWriteTx++;
#endif
		}

		KickRingBitMap >>= 1;
		hif_idx++;
	}

#ifdef CONFIG_TX_DELAY
	tx_delay_ctl->force_deq = FALSE;
#endif

	OS_SPIN_LOCK_BH(&pAd->fp_que_lock[idx]);
	if ((pAd->fp_que[idx].Number + pAd->fp_post_que[idx].Number)
			< (qm->max_data_que_num  - 100)) {
		fp_tx_flow_block(pAd, NULL, 0, FALSE, idx);
		if (reschedule_by_no_resource)
			tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
	} else {
		if (test_bit(0, &pAd->fp_tx_flow_ctl.TxFlowBlockState[idx]))
			tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
	}
	OS_SPIN_UNLOCK_BH(&pAd->fp_que_lock[idx]);
}

#ifdef CONFIG_TX_DELAY
static struct tx_delay_control *fp_get_qm_delay_ctl(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->multi_token_ques_per_band)
		return &tr_ctl->tx_delay_ctl[idx];
	else
		return &tr_ctl->tx_delay_ctl[0];
}

static INT fp_tx_delay_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct tx_delay_control *tx_delay_ctl = NULL;
	UINT8 idx;

	for (idx = 0; idx < 2; idx++) {
		tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl[idx];
		if (cap->tx_delay_support) {
			tx_delay_ctl->force_deq = FALSE;
			tx_delay_ctl->que_agg_en = FALSE;
			tx_delay_ctl->que_agg_timer_running = FALSE;
			tx_delay_ctl->idx = idx;
			tx_delay_ctl->priv = pAd;
			chip_tx_deley_parm_init(pAd->hdev_ctrl, cap->tx_delay_mode, tx_delay_ctl);
			if (IS_TX_DELAY_SW_MODE(cap))
				RTMPInitTimer(pAd, &tx_delay_ctl->que_agg_timer,
						GET_TIMER_FUNCTION(fp_que_agg_timeout), tx_delay_ctl, FALSE);
		} else {
			tx_delay_ctl->que_agg_en = FALSE;
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif

static INT fp_enq_mgmtq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	struct tm_ops *tm_ops = pAd->tm_qm_ops;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;
	UINT8 idx;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID)
		goto error;

	if (cap->multi_token_ques_per_band)
		idx =  HcGetBandByWdev(wdev);
	else
		idx = 0;

	if (idx > 1)
		goto error;

	OS_SPIN_LOCK_BH(&pAd->mgmt_que_lock[idx]);
	if ((pAd->mgmt_que[idx].Number + pAd->mgmt_post_que[idx].Number)
			>= qm->max_mgmt_que_num) {
		OS_SPIN_UNLOCK_BH(&pAd->mgmt_que_lock[idx]);
		tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
		goto error;
	}

	InsertTailQueue(&pAd->mgmt_que[idx], PACKET_TO_QUEUE_ENTRY(pkt));
	OS_SPIN_UNLOCK_BH(&pAd->mgmt_que_lock[idx]);

	tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);

	return NDIS_STATUS_SUCCESS;
error:
	tr_cnt->tx_sw_mgmtq_drop++;
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

INT32 fp_enq_dataq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx)
{
	struct tm_ops *tm_ops = pAd->tm_qm_ops;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 idx;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	UINT32 cpu_id;
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;
	UINT32 data_qm_number;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID)
		goto error;

	if (cap->multi_token_ques_per_band)
		idx = HcGetBandByWdev(wdev);
	else
		idx = 0;

	if (idx > 1)
		goto error;

	OS_SPIN_LOCK_BH(&pAd->fp_que_lock[idx]);
	data_qm_number = pAd->fp_que[idx].Number +
					pAd->fp_post_que[idx].Number;

	if (data_qm_number >= qm->max_data_que_num) {
		fp_tx_flow_block(pAd, wdev, 0, TRUE, idx);

		if (data_qm_number >= qm->max_data_que_num +
				qm->extra_reserved_que_num) {
			OS_SPIN_UNLOCK_BH(&pAd->fp_que_lock[idx]);
			tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);
			goto error;
		}
	}
#ifdef DABS_QOS
	if (pAd->dabs_qos_op & DABS_SET_QOS_PARAM) {
		dabs_active_qos_by_ipaddr(pAd, pkt);
	}
#endif
	MEM_DBG_PKT_RECORD(pkt, 1<<3);

	InsertTailQueue(&pAd->fp_que[idx], PACKET_TO_QUEUE_ENTRY(pkt));
	OS_SPIN_UNLOCK_BH(&pAd->fp_que_lock[idx]);

	cpu_id = smp_processor_id();

	if (cpu_id < 4)
		tr_cnt->tx_enq_cpu_stat[idx][cpu_id]++;

#ifdef IXIA_C50_MODE
	if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(pkt)))
		pAd->tx_cnt.tx_pkt_enq_cnt[cpu_id]++;
#endif

	tm_ops->schedule_task(pAd, TX_DEQ_TASK, idx);

	return NDIS_STATUS_SUCCESS;
error:
	tr_cnt->tx_sw_dataq_drop++;
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

static INT fp_qm_init(RTMP_ADAPTER *pAd)
{
	UINT8 idx;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct fp_qm *qm = NULL;
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct fp_tx_flow_control *flow_ctl = &pAd->fp_tx_flow_ctl;
	ULONG *pTxFlowBlockState = NULL;
	DL_LIST *pTxBlockDevList = NULL;
#ifdef WHNAT_SUPPORT
	BOOLEAN multi_token_ques =
		pAd->CommonCfg.dbdc_mode &&
		!pAd->CommonCfg.whnat_en &&
		cap->multi_token_ques_per_band;
#else
	BOOLEAN multi_token_ques =
		pAd->CommonCfg.dbdc_mode &&
		cap->multi_token_ques_per_band;
#endif

	os_alloc_mem(pAd, (UCHAR **)&qm, sizeof(*qm));

	if (!qm) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s os_alloc_mem fail\n", __func__);
		return NDIS_STATUS_FAILURE;
	}

	NdisZeroMemory(qm, sizeof(*qm));

	if (multi_token_ques) {
		qm->max_data_que_num = 2048;
		qm->max_mgmt_que_num = 256;
		qm->extra_reserved_que_num = 512;
#ifdef MEMORY_SHRINK
	} else if (pAd->CommonCfg.dbdc_mode) {
#ifdef MEMORY_SHRINK_AGGRESS
		qm->max_data_que_num = 512;
		qm->max_mgmt_que_num = 512;
		qm->extra_reserved_que_num = 128;
#else
		qm->max_data_que_num = 3584;
		qm->max_mgmt_que_num = 512;
		qm->extra_reserved_que_num = 512;
#endif	/* MEMORY_SHRINK_AGGRESS */
#endif	/* MEMORY_SHRINK */
	} else {
		qm->max_data_que_num = 4096;
		qm->max_mgmt_que_num = 512;
		qm->extra_reserved_que_num = 1024;
	}

	qm->max_tx_process_cnt = 8192;

	pAd->qm = (VOID *)qm;

	for (idx = 0; idx < 2; idx++) {
		NdisAllocateSpinLock(pAd, &pAd->fp_que_lock[idx]);
		NdisAllocateSpinLock(pAd, &pAd->fp_post_que_lock[idx]);
		InitializeQueueHeader(&pAd->fp_que[idx]);
		InitializeQueueHeader(&pAd->fp_post_que[idx]);
		NdisAllocateSpinLock(pAd, &pAd->mgmt_que_lock[idx]);
		NdisAllocateSpinLock(pAd, &pAd->mgmt_post_que_lock[idx]);
		InitializeQueueHeader(&pAd->mgmt_que[idx]);
		InitializeQueueHeader(&pAd->mgmt_post_que[idx]);
	}
#ifdef RX_RPS_SUPPORT
	/* For RX queue */
	if (cap->rx_qm == GENERIC_QM) {
		UINT32 cpu;
		for (cpu = 0; cpu < NR_CPUS; cpu++) {
			NdisAllocateSpinLock(pAd, &pAd->rx_que_lock[cpu]);
			InitializeQueueHeader(&pAd->rx_que[cpu]);
		}
	}
#else
	if (cap->rx_qm == GENERIC_QM) {
		NdisAllocateSpinLock(pAd, &pAd->rx_que_lock);
		InitializeQueueHeader(&pAd->rx_que);
		InitializeQueueHeader(&pAd->rx_post_que);
	}
#endif
	os_alloc_mem(pAd, (UCHAR **)&pTxFlowBlockState, 2 * sizeof(ULONG));

	if (!pTxFlowBlockState) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s os_alloc_mem fail\n", __func__);
		goto error0;
	}

	NdisZeroMemory(pTxFlowBlockState, 2 * sizeof(ULONG));
	flow_ctl->TxFlowBlockState = pTxFlowBlockState;

	os_alloc_mem(pAd, (UCHAR **)&pTxBlockDevList, 2 * sizeof(DL_LIST));

	if (!pTxBlockDevList) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                         "%s os_alloc_mem fail\n", __func__);
		goto error1;
	}

	NdisZeroMemory(pTxBlockDevList, 2 * sizeof(DL_LIST));
	flow_ctl->TxBlockDevList = pTxBlockDevList;

	for (idx = 0; idx < 2; idx++) {
		OS_SPIN_LOCK_BH(&pAd->fp_que_lock[idx]);
		flow_ctl->TxFlowBlockState[idx] = 0;
		DlListInit(&flow_ctl->TxBlockDevList[idx]);
		OS_SPIN_UNLOCK_BH(&pAd->fp_que_lock[idx]);
	}

	qm_ops->enq_dataq_pkt = fp_enq_dataq_pkt;
	qm_ops->tx_deq_delay = fp_ge_tx_deq_delay;

#ifdef FQ_SCH_SUPPORT
	fq_init(pAd);
#endif

	fp_tx_flow_ctl(pAd, TRUE);

	return NDIS_STATUS_SUCCESS;

error0:
	os_free_mem(qm);
error1:
	os_free_mem(pTxFlowBlockState);
	return NDIS_STATUS_FAILURE;
}

static INT fp_qm_exit(RTMP_ADAPTER *pAd)
{
	UINT8 idx;
	PQUEUE_ENTRY q_entry;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_TX_DELAY
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	BOOLEAN que_agg_timer_cancelled;
	struct tx_delay_control *tx_delay_ctl = NULL;
#endif
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;
	struct wifi_dev *wdev = NULL;
	struct fp_tx_flow_control *flow_ctl = &pAd->fp_tx_flow_ctl;

	for (idx = 0; idx < 2; idx++) {
#ifdef CONFIG_TX_DELAY
		tx_delay_ctl = &tr_ctl->tx_delay_ctl[idx];
		RTMPReleaseTimer(&tx_delay_ctl->que_agg_timer, &que_agg_timer_cancelled);
#endif

		OS_SPIN_LOCK_BH(&pAd->mgmt_que_lock[idx]);
		do {
			q_entry = RemoveHeadQueue(&pAd->mgmt_que[idx]);

			if (!q_entry)
				break;

			RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
		} while (1);

		OS_SPIN_LOCK_BH(&pAd->mgmt_post_que_lock[idx]);
		do {
			q_entry = RemoveHeadQueue(&pAd->mgmt_post_que[idx]);

			if (!q_entry)
				break;

			RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
		} while (1);
		OS_SPIN_UNLOCK_BH(&pAd->mgmt_post_que_lock[idx]);
		OS_SPIN_UNLOCK_BH(&pAd->mgmt_que_lock[idx]);

		NdisFreeSpinLock(&pAd->mgmt_que_lock[idx]);
		NdisFreeSpinLock(&pAd->mgmt_post_que_lock[idx]);

		OS_SPIN_LOCK_BH(&pAd->fp_que_lock[idx]);
		do {
			q_entry = RemoveHeadQueue(&pAd->fp_que[idx]);

			if (!q_entry)
				break;

			RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
		} while (1);

		OS_SPIN_LOCK_BH(&pAd->fp_post_que_lock[idx]);
		do {
			q_entry = RemoveHeadQueue(&pAd->fp_post_que[idx]);

			if (!q_entry)
				break;

			RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
		} while (1);
		OS_SPIN_UNLOCK_BH(&pAd->fp_post_que_lock[idx]);
		OS_SPIN_UNLOCK_BH(&pAd->fp_que_lock[idx]);
		NdisFreeSpinLock(&pAd->fp_post_que_lock[idx]);
	}
#ifdef RX_RPS_SUPPORT
	/* For RX queue */
	if (cap->rx_qm == GENERIC_QM) {
		UINT32 cpu;
		for (cpu = 0 ; cpu < NR_CPUS; cpu++) {
			RTMP_SEM_LOCK(&pAd->rx_que_lock[cpu]);
			do {
				q_entry = RemoveHeadQueue(&pAd->rx_que[cpu]);

				if (!q_entry)
					break;
				RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
			} while (1);
			do {
				q_entry = RemoveHeadQueue(&pAd->rx_post_que[cpu]);

				if (!q_entry)
					break;
				RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
			} while (1);
			RTMP_SEM_UNLOCK(&pAd->rx_que_lock[cpu]);

			NdisFreeSpinLock(&pAd->rx_que_lock[cpu]);
		}
	}
#else
	if (cap->rx_qm == GENERIC_QM) {
		OS_SPIN_LOCK_BH(&pAd->rx_que_lock);

		do {
			q_entry = RemoveHeadQueue(&pAd->rx_que);

			if (!q_entry)
				break;
			RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
		} while (1);

		do {
			q_entry = RemoveHeadQueue(&pAd->rx_post_que);

			if (!q_entry)
				break;
			RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
		} while (1);

		OS_SPIN_UNLOCK_BH(&pAd->rx_que_lock);

		NdisFreeSpinLock(&pAd->rx_que_lock);
	}
#endif
#ifdef FQ_SCH_SUPPORT
	fq_exit(pAd);
#endif

	os_free_mem((VOID *)qm);

	for (idx = 0; idx < 2; idx++) {
		OS_SPIN_LOCK_BH(&pAd->fp_que_lock[idx]);
		while (1) {
			wdev = DlListFirst(&flow_ctl->TxBlockDevList[idx], struct wifi_dev, tx_block_list);

			if (!wdev)
				break;

			DlListDel(&wdev->tx_block_list);
			RTMP_OS_NETDEV_WAKE_QUEUE(wdev->if_dev);
		}
		OS_SPIN_UNLOCK_BH(&pAd->fp_que_lock[idx]);
	}

	os_free_mem(flow_ctl->TxFlowBlockState);
	os_free_mem(flow_ctl->TxBlockDevList);

	for (idx = 0; idx < 2; idx++)
		NdisFreeSpinLock(&pAd->fp_que_lock[idx]);

	return NDIS_STATUS_SUCCESS;
}

static INT fp_schedule_tx_que(RTMP_ADAPTER *pAd, UINT8 idx)
{
	struct tm_ops *tm_ops = pAd->tm_qm_ops;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 i;

	if (cap->multi_token_ques_per_band)
		i = idx;
	else
		i = 0;

	if ((pAd->fp_que[i].Number > 0) || (pAd->fp_post_que[i].Number > 0)
			|| (pAd->mgmt_que[i].Number > 0) || (pAd->mgmt_post_que[i].Number > 0)) {
		tm_ops->schedule_task(pAd, TX_DEQ_TASK, i);
	}

	return NDIS_STATUS_SUCCESS;
}

static INT fp_schedule_tx_que_on(RTMP_ADAPTER *pAd, int cpuid, UINT8 idx)
{
	struct tm_ops *tm_ops = pAd->tm_qm_ops;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 i;

	if (cap->multi_token_ques_per_band)
		i = idx;
	else
		i = 0;

	if ((pAd->fp_que[i].Number > 0) || (pAd->fp_post_que[i].Number > 0)
			|| (pAd->mgmt_que[i].Number > 0) || (pAd->mgmt_post_que[i].Number > 0)) {
		tm_ops->schedule_task_on(pAd, cpuid, TX_DEQ_TASK, i);
	}

	return NDIS_STATUS_SUCCESS;
}

VOID fp_qm_leave_queue_pkt(struct wifi_dev *wdev, struct _QUEUE_HEADER *queue,
						struct _QUEUE_HEADER *post_queue,
						NDIS_SPIN_LOCK *lock, NDIS_SPIN_LOCK *post_lock)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct _QUEUE_HEADER local_q;
	struct _QUEUE_ENTRY *q_entry;
	NDIS_PACKET *pkt;

	InitializeQueueHeader(&local_q);
	RTMP_SEM_LOCK(lock);

	/*remove entry owned by wdev*/
	do {
		q_entry = RemoveHeadQueue(queue);

		if (!q_entry)
			break;

		pkt = QUEUE_ENTRY_TO_PACKET(q_entry);

		if (RTMP_GET_PACKET_WDEV(pkt) == wdev->wdev_idx)
			RELEASE_NDIS_PACKET(ad, pkt, NDIS_STATUS_SUCCESS);
		else
			InsertTailQueue(&local_q, q_entry);
	} while (1);

	/*re-enqueue other entries*/
	do {
		q_entry = RemoveHeadQueue(&local_q);

		if (!q_entry)
			break;

		InsertTailQueue(queue, q_entry);
	} while (1);

	RTMP_SEM_LOCK(post_lock);
	/*remove entry owned by wdev*/
	do {
		q_entry = RemoveHeadQueue(post_queue);

		if (!q_entry)
			break;

		pkt = QUEUE_ENTRY_TO_PACKET(q_entry);

		if (RTMP_GET_PACKET_WDEV(pkt) == wdev->wdev_idx)
			RELEASE_NDIS_PACKET(ad, pkt, NDIS_STATUS_SUCCESS);
		else
			InsertTailQueue(&local_q, q_entry);
	} while (1);

	/*re-enqueue other entries*/
	do {
		q_entry = RemoveHeadQueue(&local_q);

		if (!q_entry)
			break;

		InsertTailQueue(post_queue, q_entry);
	} while (1);

	RTMP_SEM_UNLOCK(post_lock);

	RTMP_SEM_UNLOCK(lock);
}

static INT fp_bss_clean_queue(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	UINT8 idx;
	/*TODO: add check de-queue task idle*/
	for (idx = 0; idx < 2; idx++) {
		fp_qm_leave_queue_pkt(wdev, &ad->mgmt_que[idx], &ad->mgmt_post_que[idx],
								&ad->mgmt_que_lock[idx], &ad->mgmt_post_que_lock[idx]);
		fp_qm_leave_queue_pkt(wdev, &ad->fp_que[idx], &ad->fp_post_que[idx],
								&ad->fp_que_lock[idx], &ad->fp_post_que_lock[idx]);
	}
	return NDIS_STATUS_SUCCESS;
}

static INT32 fp_dump_all_sw_queue(RTMP_ADAPTER *pAd)
{
	UINT8 idx;
	struct fp_tx_flow_control *flow_ctl = &pAd->fp_tx_flow_ctl;
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;

	MTWF_PRINT("tx flow control:%d\n", test_bit(0, &flow_ctl->flag));

	MTWF_PRINT("max_mgmt_que_num:%d\n", qm->max_mgmt_que_num);
	MTWF_PRINT("max_data_que_num:%d\n", qm->max_data_que_num);
	MTWF_PRINT("extra_reserved_que_num:%d\n", qm->extra_reserved_que_num);

	for (idx = 0; idx < 2; idx++) {
		OS_SPIN_LOCK_BH(&pAd->mgmt_que_lock[idx]);
		MTWF_PRINT("\nmgmt_que[%d] number:          %d\n", idx, pAd->mgmt_que[idx].Number);
		MTWF_PRINT("mgmt_post_que[%d] number:     %d\n", idx, pAd->mgmt_post_que[idx].Number);
		OS_SPIN_UNLOCK_BH(&pAd->mgmt_que_lock[idx]);

		OS_SPIN_LOCK_BH(&pAd->fp_que_lock[idx]);
		MTWF_PRINT("fp_que[%d] number:            %d\n", idx, pAd->fp_que[idx].Number);
		MTWF_PRINT("fp_post_que[%d] number:       %d\n", idx, pAd->fp_post_que[idx].Number);
		MTWF_PRINT("tx flow block state[%d]:      %d\n", idx,
			test_bit(0, &flow_ctl->TxFlowBlockState[idx]));
		MTWF_PRINT("tx flow block dev number[%d]: %d\n", idx,
			DlListLen(&flow_ctl->TxBlockDevList[idx]));
		OS_SPIN_UNLOCK_BH(&pAd->fp_que_lock[idx]);
	}

	return NDIS_STATUS_SUCCESS;
}


struct qm_ops fp_qm_ops = {
	.init = fp_qm_init,
	.exit = fp_qm_exit,
	.enq_mgmtq_pkt = fp_enq_mgmtq_pkt,
	.deq_tx_pkt = fp_tx_pkt_deq_func,
	.dump_all_sw_queue = fp_dump_all_sw_queue,
	.schedule_tx_que = fp_schedule_tx_que,
	.schedule_tx_que_on = fp_schedule_tx_que_on,
	.bss_clean_queue = fp_bss_clean_queue,
	.enq_rx_dataq_pkt = ge_rx_enq_dataq_pkt,
#ifdef CONFIG_TX_DELAY
	.tx_delay_init = fp_tx_delay_init,
	.get_qm_delay_ctl = fp_get_qm_delay_ctl,
#endif
	.tx_flow_ctl = fp_tx_flow_ctl,
};
