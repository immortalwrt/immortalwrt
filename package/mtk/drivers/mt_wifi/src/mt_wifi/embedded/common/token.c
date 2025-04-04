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

#include "rtmp_comm.h"

#include "os/rt_linux_cmm.h"
#include "os/rt_drv.h"
#include "rt_os_util.h"
#include "rt_config.h"

static INT token_tx_queue_destroy(
	PKT_TOKEN_CB *pktTokenCb,
	struct token_tx_pkt_queue *que)
{
	INT idx;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)(pktTokenCb->pAd);
	struct token_tx_pkt_entry *entry = NULL;

	if (que->token_inited == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s(): %p,%p\n", __func__, que, &que->token_inited);
		RTMP_SEM_LOCK(&que->enq_lock);
		que->token_inited = FALSE;
		RTMP_SEM_UNLOCK(&que->enq_lock);

		for (idx = que->pkt_tkid_start; idx <= que->pkt_tkid_end; idx++) {
			if (!que->pkt_token)
				break;
			entry = &que->pkt_token[idx];

			if (entry && entry->pkt_buf) {
				PCI_UNMAP_SINGLE(pAd, entry->pkt_phy_addr,
								 entry->pkt_len, RTMP_PCI_DMA_TODEVICE);

				RELEASE_NDIS_PACKET(pktTokenCb->pAd, entry->pkt_buf, NDIS_STATUS_FAILURE);
			}
		}
		if (que->free_id)
			os_free_mem(que->free_id);
		if (que->pkt_token)
			os_free_mem(que->pkt_token);
		NdisFreeSpinLock(&que->enq_lock);
		NdisFreeSpinLock(&que->deq_lock);
	}

	return NDIS_STATUS_SUCCESS;
}

static const UINT16 tx_free_notify_record_boundary[TX_FREE_NOTIFY_DEEP_STAT_SIZE]
					= {10, 20, 30, 40, 50, 60, 128, 200, 256, 312, 400, 512};

static INT token_tx_two_queues_init(
	RTMP_ADAPTER *pAd,
	UINT8 qidx,
	struct token_tx_pkt_queue *que)
{
	INT idx;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!que) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s(): token que(%d) is NULL!\n", __func__, qidx);
		return NDIS_STATUS_FAILURE;
	}

	if (que->token_inited == FALSE) {

		if (qidx == 0) {
			que->pkt_tkid_end = cap->tkn_info.band0_token_cnt - 1;
			que->pkt_tkid_start = 0;
			que->pkt_tkid_cnt = cap->tkn_info.band0_token_cnt;
			que->pkt_tkid_invalid = cap->tkn_info.token_tx_cnt;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                                 "%s(): ct sw token(%d) number = %d\n",
				 __func__, qidx, que->pkt_tkid_cnt);
		} else if (qidx == 1) {
			que->pkt_tkid_end = (cap->tkn_info.token_tx_cnt - 1);
			que->pkt_tkid_start = cap->tkn_info.band0_token_cnt;
			que->pkt_tkid_cnt = (cap->tkn_info.token_tx_cnt - cap->tkn_info.band0_token_cnt);
			que->pkt_tkid_invalid = (cap->tkn_info.token_tx_cnt + 1);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                                 "%s(): ct sw token(%d) number = %d\n",
				 __func__, qidx, que->pkt_tkid_cnt);
		}

		que->pkt_tkid_aray = (que->pkt_tkid_cnt + 1);

		NdisAllocateSpinLock(pAd, &que->enq_lock);
		NdisAllocateSpinLock(pAd, &que->deq_lock);

		que->id_head = 0;
		que->id_tail = que->pkt_tkid_cnt;
		/* allocate freeid */
		os_alloc_mem(pAd, (UCHAR **)&que->free_id, sizeof(UINT16) * que->pkt_tkid_aray);
		if (!que->free_id) {
			que->token_inited = TRUE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			         "%s(): token que free_id inited fail!\n",  __func__);
			return NDIS_STATUS_RESOURCES;
		}
		os_zero_mem(que->free_id, sizeof(UINT16) * que->pkt_tkid_aray);
		/* allocate pkt_token */
		os_alloc_mem(pAd, (UCHAR **)&que->pkt_token, sizeof(struct token_tx_pkt_entry) * cap->tkn_info.token_tx_cnt);
		if (!que->pkt_token) {
			que->token_inited = TRUE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			         "%s(): token que pkt_token inited fail!\n",  __func__);
			return NDIS_STATUS_RESOURCES;
		}
		os_zero_mem(que->pkt_token, sizeof(struct token_tx_pkt_entry) * cap->tkn_info.token_tx_cnt);

		/*initial freeid*/
		for (idx = 0; idx < que->pkt_tkid_cnt; idx++) {
			que->free_id[idx] = que->pkt_tkid_start + idx;
		}

		que->free_id[que->pkt_tkid_cnt] = que->pkt_tkid_invalid;
		atomic_set(&que->free_token_cnt, que->pkt_tkid_cnt);
		que->total_enq_cnt = 0;
		que->total_deq_cnt = 0;
		que->total_back_cnt = 0;
		que->band_idx = qidx;
		que->low_water_mark = cap->tkn_info.low_water_mark;
		que->high_water_mark_per_band[qidx] = que->pkt_tkid_cnt;
		token_tx_set_hwmark(que, token_tx_get_lwmark(que) + 10);
		clear_bit(TX_TOKEN_LOW, &que->token_state);

		for (idx = 0; idx < TX_FREE_NOTIFY_DEEP_STAT_SIZE; idx++) {
			que->deep_stat[idx].boundary = tx_free_notify_record_boundary[idx];
			que->deep_stat[idx].cnt = 0;
		}

		que->token_inited = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s(): token que(%d) inited done!id_head/tail=%d/%d\n",
			 __func__, qidx, que->id_head, que->id_tail);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s(): %p,%p\n",
			 __func__, que, &que->token_inited);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s(): token que(%d) already inited!shall not happened!\n",
			 __func__, qidx);

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\t%s(): token que(%d) id_head=%d, list.id_tail=%d\n",
			 __func__, qidx, que->id_head, que->id_tail);
	}

	return NDIS_STATUS_SUCCESS;
}

static INT token_tx_queue_init(
	RTMP_ADAPTER *pAd,
	struct token_tx_pkt_queue *que)
{
	INT idx;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!que) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s(): token que is NULL!\n", __func__);
		return NDIS_STATUS_FAILURE;
	}

	if (que->token_inited == FALSE) {
		que->pkt_tkid_end = (cap->tkn_info.token_tx_cnt - 1);
		que->pkt_tkid_start = 0;

#ifdef WHNAT_SUPPORT
		if (IS_ASIC_CAP(pAd, fASIC_CAP_WHNAT) && pAd->CommonCfg.whnat_en) {
			que->pkt_tkid_end = cap->tkn_info.token_tx_cnt - 1;
			que->pkt_tkid_start = cap->tkn_info.hw_tx_token_cnt;
		}
#endif

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s(): ct SW token number = %d\n",
			__func__, cap->tkn_info.token_tx_cnt - cap->tkn_info.hw_tx_token_cnt);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s(): ct HW token number = %d\n",
			__func__, cap->tkn_info.hw_tx_token_cnt);

		que->pkt_tkid_invalid = (cap->tkn_info.token_tx_cnt + 1);
		que->pkt_tkid_cnt = (que->pkt_tkid_end - que->pkt_tkid_start + 1);
		que->pkt_tkid_aray = (que->pkt_tkid_cnt + 1);

		NdisAllocateSpinLock(pAd, &que->enq_lock);
		NdisAllocateSpinLock(pAd, &que->deq_lock);

		que->id_head = 0;
		que->id_tail = que->pkt_tkid_cnt;
		/* allocate freeid */
		os_alloc_mem(pAd, (UCHAR **)&que->free_id, sizeof(UINT16) * que->pkt_tkid_aray);
		if (!que->free_id) {
			que->token_inited = TRUE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			         "%s(): token que free_id inited fail!\n",  __func__);
			return NDIS_STATUS_RESOURCES;
		}
		os_zero_mem(que->free_id, sizeof(UINT16) * que->pkt_tkid_aray);
		/* allocate pkt_token */
		os_alloc_mem(pAd, (UCHAR **)&que->pkt_token, sizeof(struct token_tx_pkt_entry) * cap->tkn_info.token_tx_cnt);
		if (!que->pkt_token) {
			que->token_inited = TRUE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			         "%s(): token que pkt_token inited fail!\n",  __func__);
			return NDIS_STATUS_RESOURCES;
		}
		os_zero_mem(que->pkt_token, sizeof(struct token_tx_pkt_entry) * cap->tkn_info.token_tx_cnt);

		/*initial freeid*/
		for (idx = 0; idx < que->pkt_tkid_cnt; idx++)
			que->free_id[idx] = idx + que->pkt_tkid_start;

		que->free_id[que->pkt_tkid_cnt] = que->pkt_tkid_invalid;
		atomic_set(&que->free_token_cnt, que->pkt_tkid_cnt);
		que->total_enq_cnt = 0;
		que->total_deq_cnt = 0;
		que->total_back_cnt = 0;
		que->low_water_mark = cap->tkn_info.low_water_mark;

		if (pAd->CommonCfg.dbdc_mode && (cap->tkn_info.high_water_mark_per_band[0] > 0)
			 && (cap->tkn_info.high_water_mark_per_band[1] > 0)) {
			que->high_water_mark_per_band[0] = cap->tkn_info.high_water_mark_per_band[0];
			que->high_water_mark_per_band[1] = cap->tkn_info.high_water_mark_per_band[1];
		} else {
			que->high_water_mark_per_band[0] = que->pkt_tkid_cnt;
			que->high_water_mark_per_band[1] = que->pkt_tkid_cnt;
		}

		token_tx_set_hwmark(que, token_tx_get_lwmark(que) + 10);
		clear_bit(TX_TOKEN_LOW, &que->token_state);

		for (idx = 0; idx < TX_FREE_NOTIFY_DEEP_STAT_SIZE; idx++) {
			que->deep_stat[idx].boundary = tx_free_notify_record_boundary[idx];
			que->deep_stat[idx].cnt = 0;
		}

		que->token_inited = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s(): token que inited done!id_head/tail=%d/%d\n",
			 __func__, que->id_head, que->id_tail);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s(): %p,%p\n",
			 __func__, que, &que->token_inited);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s(): token que already inited!shall not happened!\n",
			 __func__);

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\ttoken que id_head=%d, list.id_tail=%d\n",
			 que->id_head, que->id_tail);
	}

	return NDIS_STATUS_SUCCESS;
}

PNDIS_PACKET token_tx_deq(
	RTMP_ADAPTER *pAd,
	struct token_tx_pkt_queue *que,
	UINT16 token,
	UINT8 *type)
{
	PNDIS_PACKET pkt_buf = NULL;
	static BOOLEAN is_dump_wa = FALSE;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR band_idx = 0;

	if (!que) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s(): token que is NULL!\n", __func__);
		return NULL;
	}

	RTMP_SEM_LOCK(&que->deq_lock);

	if (que->token_inited == TRUE) {
		if (que) {
			if (token >= que->pkt_tkid_start && token <= que->pkt_tkid_end) {
				struct token_tx_pkt_entry *entry = NULL;
				STA_TR_ENTRY *tr_entry = NULL;
				if (que->band_idx == 0)
					entry = &que->pkt_token[token];
				else
					entry = &que->pkt_token[token - cap->tkn_info.band0_token_cnt];
				tr_entry = &pAd->tr_ctl.tr_entry[entry->wcid];
				pkt_buf = entry->pkt_buf;
				*type = entry->Type;

				if (pkt_buf == NULL) {
					MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
                                                 "%s(): buggy here? token ID(%d) without pkt!\n",
                                                 __func__, token);
					RTMP_SEM_UNLOCK(&que->deq_lock);
					if (!is_dump_wa) {
#ifdef WIFI_UNIFIED_COMMAND
						if (cap->uni_cmd_support)
							MtUniCmdFwLog2Host(pAd, HOST2CR4, ENUM_CMD_FW_LOG_2_HOST_CTRL_OFF);
						else
#endif /* WIFI_UNIFIED_COMMAND */
							MtCmdFwLog2Host(pAd, 1, 0);
						is_dump_wa = TRUE;
					}
					return pkt_buf;
				}

				entry->pkt_buf = NULL;
				PCI_UNMAP_SINGLE(pAd, entry->pkt_phy_addr, entry->pkt_len, RTMP_PCI_DMA_TODEVICE);
				entry->Type = TOKEN_NONE;
				entry->wcid = WCID_INVALID;
				entry->pkt_len = 0;
				que->free_id[que->id_tail] = token;
				INC_INDEX(que->id_tail, que->pkt_tkid_aray);
				atomic_inc(&que->free_token_cnt);
				que->total_deq_cnt++;
				tr_entry->token_cnt--;

				band_idx = RTMP_GET_BAND_IDX(pkt_buf);

				if (atomic_read(&que->used_token_per_band[band_idx]))
					atomic_dec(&que->used_token_per_band[band_idx]);
				else
					MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR, "%s(): enq cnt error, token ID(%d)\n", __func__, token);
			} else {
				MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
                                         "%s(): Invalid token ID(%d)\n", __func__, token);
			}
		}
	}

	RTMP_SEM_UNLOCK(&que->deq_lock);

	return pkt_buf;
}


UINT16 token_tx_enq(
	RTMP_ADAPTER *pAd,
	struct token_tx_pkt_queue *que,
	PNDIS_PACKET pkt,
	UCHAR type,
	UINT16 wcid,
	NDIS_PHYSICAL_ADDRESS pkt_phy_addr,
	size_t pkt_len)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 idx = 0, token = que->pkt_tkid_invalid;
	struct token_tx_pkt_entry *entry = NULL;
	STA_TR_ENTRY *tr_entry = &pAd->tr_ctl.tr_entry[wcid];
	UINT32 band_idx = 0;

	RTMP_SEM_LOCK(&que->enq_lock);

	if (que->token_inited == TRUE) {
		idx = que->id_head;
		token = que->free_id[idx];

		if (token >= que->pkt_tkid_start && token <= que->pkt_tkid_end) {
			if (que->band_idx == 0)
				entry = &que->pkt_token[token];
			else
				entry = &que->pkt_token[token - cap->tkn_info.band0_token_cnt];

			if (entry->pkt_buf) {
				PCI_UNMAP_SINGLE(pAd, entry->pkt_phy_addr, entry->pkt_len, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, entry->pkt_buf, NDIS_STATUS_FAILURE);
			}

			entry->pkt_buf = pkt;
			entry->wcid = wcid;
			entry->Type = type;
			entry->pkt_phy_addr = pkt_phy_addr;
			entry->pkt_len = pkt_len;
			que->free_id[idx] = que->pkt_tkid_invalid;
			INC_INDEX(que->id_head, que->pkt_tkid_aray);
			atomic_dec(&que->free_token_cnt);
			que->total_enq_cnt++;

			tr_entry->token_cnt++;

			band_idx = RTMP_GET_BAND_IDX(pkt);

			atomic_inc(&que->used_token_per_band[band_idx]);
		} else {
			token = que->pkt_tkid_invalid;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s: token:%d buggy here?\n", __func__, token);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s: token:%d que uninited!\n", __func__, token);
	}

	RTMP_SEM_UNLOCK(&que->enq_lock);

	return token;
}

static INT token_tx_deinit(PKT_TOKEN_CB *cb)
{
	INT ret = NDIS_STATUS_SUCCESS;
	struct token_tx_pkt_queue *que = NULL;
	UINT8 i;

	for (i = 0; i < cb->que_nums; i++) {
		que = &cb->que[i];
		ret = token_tx_queue_destroy(cb, que);
	}

	return ret;
}


static INT token_tx_init(PKT_TOKEN_CB *cb, RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	UINT8 i;
	INT ret = NDIS_STATUS_SUCCESS;
	struct token_tx_pkt_queue *que = NULL;
#ifdef WHNAT_SUPPORT
	BOOLEAN multi_token_ques =
		ad->CommonCfg.dbdc_mode &&
		!ad->CommonCfg.whnat_en &&
		cap->multi_token_ques_per_band;
#else
	BOOLEAN multi_token_ques =
		ad->CommonCfg.dbdc_mode &&
		cap->multi_token_ques_per_band;
#endif

	if (multi_token_ques) {
		cb->que_nums = 2;
		for (i = 0; i < cb->que_nums; i++) {
			que = &cb->que[i];
			ret = token_tx_two_queues_init(ad, i, que);
			if (ret != NDIS_STATUS_SUCCESS)
				return ret;
		}
	} else {
		cb->que_nums = 1;
		que = &cb->que[0];
		ret = token_tx_queue_init(ad, que);
		if (ret != NDIS_STATUS_SUCCESS)
			return ret;
	}

	if (cap->txd_flow_ctl)
		MtCmdCr4Set(ad, WA_SET_OPTION_TXD_FLOW_CTRL,
					1, 0);

	return ret;
}

INT token_tx_setting(RTMP_ADAPTER *pAd, UINT8 q_idx, INT32 option,
								INT32 sub_option, INT32 value)
{
	INT32 ret = 0;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	struct token_tx_pkt_queue *que = &cb->que[q_idx];

	if (cb->que_nums < 2)
		que = &cb->que[0];

	switch (option) {
	case TOKEN_WATERMARK:
		if (sub_option == TOKEN_LOWMARK)
			token_tx_set_lwmark(que, value);
		else if (sub_option == TOKEN_FREE_HIGHMARK)
			token_tx_set_hwmark(que, value);
		else if (sub_option == TOKEN_ENQ_HIGHMARK)
			que->high_water_mark_per_band[q_idx] = value;
		else
			ret = 1;
		break;
	case TOKEN_BOUNDARY:
		if ((sub_option < TX_FREE_NOTIFY_DEEP_STAT_SIZE) && (sub_option >= 0))
			que->deep_stat[sub_option].boundary = value;
		else
			ret = 1;
		break;
	case TOKEN_DEBUG:
		if (value)
			cb->dbg |= (1 << sub_option);
		else
			cb->dbg &= ~(1 << sub_option);
		break;
	default:
		ret = 1;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s: unknown token option %d\n", __func__, option);
	}

	return ret;
}

inline struct token_tx_pkt_queue *token_tx_get_queue_by_band(PKT_TOKEN_CB *cb, UINT32 band_idx)
{
	if (cb->que_nums != 2)
		return &cb->que[0];
	else
		return &cb->que[band_idx];
}

inline struct token_tx_pkt_queue *token_tx_get_queue_by_token_id(PKT_TOKEN_CB *cb, UINT32 token_id)
{
	struct token_tx_pkt_queue *que = &cb->que[0];

	if (cb->que_nums != 2) {
		return que;
	} else {
		if (token_id >= que->pkt_tkid_start && token_id <= que->pkt_tkid_end)
			return que;
		else
			return &cb->que[1];
	}
}

inline BOOLEAN token_tx_get_state(struct token_tx_pkt_queue *que)
{
	return test_bit(0, &que->token_state);
}

inline INT token_tx_set_state(struct token_tx_pkt_queue *que, BOOLEAN state)
{
	if (state == TX_TOKEN_LOW)
		set_bit(0, &que->token_state);
	else
		clear_bit(0, &que->token_state);

	return NDIS_STATUS_SUCCESS;
}

inline VOID token_tx_inc_full_cnt(struct token_tx_pkt_queue *que)
{
	que->token_full_cnt++;
}

inline UINT32 token_tx_get_free_cnt(struct token_tx_pkt_queue *que)
{
	return atomic_read(&que->free_token_cnt);
}

inline VOID token_tx_inc_full_cnt_per_band(struct token_tx_pkt_queue *que, UINT32 band_idx)
{
	que->token_full_cnt_per_band[band_idx]++;
}

inline UINT32 token_tx_get_used_cnt_per_band(struct token_tx_pkt_queue *que, UINT32 band_idx)
{
	return atomic_read(&que->used_token_per_band[band_idx]);
}

inline UINT32 token_tx_get_hwmark_per_band(struct token_tx_pkt_queue *que, UINT32 band_idx)
{
	return que->high_water_mark_per_band[band_idx];
}

inline VOID token_tx_set_lwmark(struct token_tx_pkt_queue *que, UINT32 value)
{
	que->low_water_mark = value;
}

inline UINT32 token_tx_get_lwmark(struct token_tx_pkt_queue *que)
{
	return que->low_water_mark;
}

inline VOID token_tx_set_hwmark(struct token_tx_pkt_queue *que, UINT32 value)
{
	que->high_water_mark = value;
}

inline UINT32 token_tx_get_hwmark(struct token_tx_pkt_queue *que)
{
	return que->high_water_mark;
}

inline VOID token_tx_record_free_notify(struct token_tx_pkt_queue *que, UINT32 token_cnt)
{
	UINT8 i = 0;

	do {
		if (token_cnt < que->deep_stat[i].boundary ||
				(i == (TX_FREE_NOTIFY_DEEP_STAT_SIZE - 1))) {
			que->deep_stat[i].cnt++;
			break;
		}
	} while (++i < TX_FREE_NOTIFY_DEEP_STAT_SIZE);
}

static INT token_rx_init(PKT_TOKEN_CB *cb, RTMP_ADAPTER *ad)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct token_rx_pkt_queue *que = &cb->rx_que;

	atomic_set(&que->cur_free_idx, 0);
	que->pkt_tkid_cnt = cap->tkn_info.token_rx_cnt;
	os_alloc_mem(ad, (UCHAR **)&que->pkt_token, sizeof(struct token_rx_pkt_entry) * que->pkt_tkid_cnt);
	if (!que->pkt_token) {
		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s(): token que pkt_token inited fail!\n",  __func__);
		return NDIS_STATUS_RESOURCES;
	}
	os_zero_mem(que->pkt_token, sizeof(struct token_rx_pkt_entry) * que->pkt_tkid_cnt);

	return ret;
}

static INT token_rx_deinit(PKT_TOKEN_CB *cb)
{
	INT ret = NDIS_STATUS_SUCCESS;
	struct token_rx_pkt_queue *que = &cb->rx_que;
	struct token_rx_pkt_entry *entry = NULL;
	int i;
	UINT16 skb_data_size;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)cb->pAd;

	for (i = 0; i < atomic_read(&que->cur_free_idx); i++) {
		if (!que->pkt_token)
			break;
		entry = &que->pkt_token[i];
		if (entry) {
			PCI_UNMAP_SINGLE(ad, entry->dma_buf.AllocPa,
				entry->dma_buf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
			skb_data_size = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + entry->dma_buf.AllocSize) +
				SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);
#if (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
			MEM_DBG_PKT_FREE_INC(entry->pkt_buf);
			page_frag_free(entry->pkt_buf);
#elif (KERNEL_VERSION(4, 3, 0) < LINUX_VERSION_CODE)
			MEM_DBG_PKT_FREE_INC(entry->pkt_buf);
			__free_page_frag(entry->pkt_buf);
#else
			DEV_FREE_FRAG_BUF(entry->pkt_buf);
#endif
		}
	}
	if (que->pkt_token)
		os_free_mem(que->pkt_token);

	return ret;
}

UINT32 token_rx_dmad_init(struct token_rx_pkt_queue *que, PNDIS_PACKET pkt,
								ULONG alloc_size, PVOID alloc_va, NDIS_PHYSICAL_ADDRESS alloc_pa)
{
	UINT32 token_id;

	token_id = atomic_read(&que->cur_free_idx);
	que->pkt_token[token_id].pkt_buf = pkt;
	que->pkt_token[token_id].dma_buf.AllocSize = alloc_size;
	que->pkt_token[token_id].dma_buf.AllocVa = alloc_va;
	que->pkt_token[token_id].dma_buf.AllocPa = alloc_pa;
	atomic_inc(&que->cur_free_idx);

	return token_id;
}
EXPORT_SYMBOL(token_rx_dmad_init);

INT token_rx_dmad_lookup(struct token_rx_pkt_queue *que, UINT32 token_id, PNDIS_PACKET *pkt,
							PVOID *alloc_va, NDIS_PHYSICAL_ADDRESS *alloc_pa)
{
	INT32 ret = 0;

	*pkt = que->pkt_token[token_id].pkt_buf;
	*alloc_va = que->pkt_token[token_id].dma_buf.AllocVa;
	*alloc_pa = que->pkt_token[token_id].dma_buf.AllocPa;

	return ret;
}
EXPORT_SYMBOL(token_rx_dmad_lookup);

INT token_rx_dmad_lookup_pa(struct token_rx_pkt_queue *que, UINT32 *token_id, PNDIS_PACKET *pkt,
							PVOID *alloc_va, NDIS_PHYSICAL_ADDRESS alloc_pa)
{
	INT32 ret = -1;
	UINT32 token_loop = 0;
	for (token_loop = 0 ; token_loop < que->pkt_tkid_cnt ; token_loop++) {
		if (alloc_pa == que->pkt_token[token_loop].dma_buf.AllocPa) {
			*token_id = token_loop;
			*alloc_va = que->pkt_token[token_loop].dma_buf.AllocVa;
			*pkt = que->pkt_token[token_loop].pkt_buf;
			MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
                                 "%s(): tkn_rx_id: %d according to %04x\n",
                                 __func__, *token_id, (UINT32)alloc_pa);
			ret = 0;
		}
	}
	return ret;
}

VOID token_rx_dmad_pool_dump(struct token_rx_pkt_queue *que)
{
	return;
}

INT token_rx_dmad_update(struct token_rx_pkt_queue *que, UINT32 token_id, PNDIS_PACKET pkt,
								ULONG alloc_size, PVOID alloc_va, NDIS_PHYSICAL_ADDRESS alloc_pa)
{
	INT32 ret = 0;

	que->pkt_token[token_id].pkt_buf = pkt;
	que->pkt_token[token_id].dma_buf.AllocSize = alloc_size;
	que->pkt_token[token_id].dma_buf.AllocVa = alloc_va;
	que->pkt_token[token_id].dma_buf.AllocPa = alloc_pa;

	return ret;
}

INT token_init(VOID **ppPktTokenCb, VOID *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	PKT_TOKEN_CB *cb;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)pAd;

	os_alloc_mem(pAd, (UCHAR **)&cb, sizeof(PKT_TOKEN_CB));

	if (!cb) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s os_alloc_mem fail\n", __func__);
		return NDIS_STATUS_RESOURCES;
	}

	NdisZeroMemory(cb, sizeof(PKT_TOKEN_CB));
	cb->pAd = pAd;

	*ppPktTokenCb = cb;

	ret = token_tx_init(cb, ad);

	if (ret == NDIS_STATUS_SUCCESS)
		ret = token_rx_init(cb, ad);

	return ret;
}

INT token_deinit(PKT_TOKEN_CB **ppPktTokenCb)
{
	INT ret = NDIS_STATUS_SUCCESS;
	PKT_TOKEN_CB *cb = *ppPktTokenCb;

	if (cb == NULL)
		return NDIS_STATUS_FAILURE;

	ret = token_tx_deinit(cb);

	if (!ret)
		ret = token_rx_deinit(cb);

	os_free_mem((VOID *)cb);
	*ppPktTokenCb = NULL;

	return ret;
}
