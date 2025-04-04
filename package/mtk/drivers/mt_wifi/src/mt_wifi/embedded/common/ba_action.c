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
	ba_action.c
*/

#include "rt_config.h"

#define BA_ORI_INIT_SEQ		(tr_entry->TxSeq[TID]) /* 1 : inital sequence number of BA session*/
#define ORI_SESSION_MAX_RETRY	8
#if defined(MT7637_FPGA) || defined(MT7615_FPGA) || defined(MT7622_FPGA) || \
	defined(P18_FPGA) || defined(AXE_FPGA) || defined(MT7915_FPGA) || \
	defined(MT7986_FPGA) || defined(MT7916_FPGA) || defined(MT7981_FPGA)
#define ORI_BA_SESSION_TIMEOUT	(10000)	/* ms */
#else
#define ORI_BA_SESSION_TIMEOUT	(2000)	/* ms */
#endif
#define REC_BA_SESSION_IDLE_TIMEOUT	(1000)	/* ms */
#define REORDERING_PACKET_TIMEOUT	((REORDERING_PACKET_TIMEOUT_IN_MS * OS_HZ)/1000)	/* system ticks -- 100 ms*/
#define MAX_REORDERING_PACKET_TIMEOUT	((MAX_REORDERING_PACKET_TIMEOUT_IN_MS * OS_HZ)/1000)	/* system ticks -- 100 ms*/
#define INVALID_RCV_SEQ (0xFFFF)

static inline void ba_enqueue_head(struct reordering_list *list,
							struct reordering_mpdu *mpdu_blk)
{
	list->qlen++;
	mpdu_blk->next = list->next;
	list->next = mpdu_blk;

	if (!list->tail)
		list->tail = mpdu_blk;
}

static inline void ba_enqueue_tail(struct reordering_list *list,
							struct reordering_mpdu *mpdu_blk)
{
	list->qlen++;
	mpdu_blk->next = NULL;

	if (list->tail)
		list->tail->next = mpdu_blk;
	else
		list->next = mpdu_blk;

	list->tail = mpdu_blk;
}

static inline struct reordering_mpdu *ba_dequeue_head(struct reordering_list *list)
{
	struct reordering_mpdu *mpdu_blk = NULL;

	if (list && list->next) {
		list->qlen--;
		mpdu_blk = list->next;
		list->next = mpdu_blk->next;

		if (mpdu_blk == list->tail)
			list->tail = NULL;
	}

	return mpdu_blk;
}

static inline struct reordering_mpdu  *ba_reordering_mpdu_dequeue(struct reordering_list *list)
{
	struct reordering_mpdu *ret = (ba_dequeue_head(list));
	return ret;
}


inline struct reordering_mpdu *ba_reordering_mpdu_probe(struct reordering_list *list)
{
	struct reordering_mpdu *ret = NULL;

	if (list)
		ret = list->next;
	return ret;
}

void dump_ba_list(struct reordering_list *list)
{
	struct reordering_mpdu *mpdu_blk = NULL;

	if (list->next) {
		MTWF_PRINT("\n ba sn list:");
		mpdu_blk = list->next;

		while (mpdu_blk) {
			MTWF_PRINT("%x ", mpdu_blk->Sequence);
			mpdu_blk = mpdu_blk->next;
		}
	}

	MTWF_PRINT("\n\n");
}

static VOID ba_free_ori_entry(RTMP_ADAPTER *pAd, ULONG Idx)
{
	BA_ORI_ENTRY *pBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;

	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_ORI_TABLE))
		return;

	pBAEntry = &ba_ctl->BAOriEntry[Idx];
	if (pBAEntry->ORI_BA_Status != Originator_NONE) {
		pEntry = &pAd->MacTab.Content[pBAEntry->Wcid];
		pEntry->BAOriWcidArray[pBAEntry->TID] = 0;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "%s: Wcid = %d, TID = %d\n", __func__, pBAEntry->Wcid, pBAEntry->TID);

		if (pBAEntry->ORI_BA_Status == Originator_Done) {
			NdisAcquireSpinLock(&ba_ctl->BATabLock);
			ba_ctl->numDoneOriginator -= 1;
			pEntry->TXBAbitmap &= (~(1 << (pBAEntry->TID)));
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "ba_free_ori_entry numAsOriginator= %ld\n", ba_ctl->numDoneOriginator);
			NdisReleaseSpinLock(&ba_ctl->BATabLock);
			/* Erase Bitmap flag.*/
		}

		NdisAcquireSpinLock(&ba_ctl->BATabLock);
		if (ba_ctl->numAsOriginator != 0)
			ba_ctl->numAsOriginator -= 1;
		else
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "%s(): numAsOriginator = 0, ORI_BA_Status = %d\n",
					 __func__, pBAEntry->ORI_BA_Status);
		NdisReleaseSpinLock(&ba_ctl->BATabLock);

		pBAEntry->ORI_BA_Status = Originator_NONE;
		pBAEntry->Token = 0;
	}
}

static UINT announce_non_hw_damsdu_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, UCHAR OpMode)
{
	PUCHAR pData;
	USHORT DataSize;
	UINT nMSDU = 0;

	pData = (PUCHAR)GET_OS_PKT_DATAPTR(pPacket);
	DataSize = (USHORT)GET_OS_PKT_LEN(pPacket);
	nMSDU = deaggregate_amsdu_announce(pAd, pPacket, pData, DataSize, OpMode);
	return nMSDU;
}

static void announce_ba_reorder_pkt(RTMP_ADAPTER *pAd, struct reordering_mpdu *mpdu)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	PNDIS_PACKET pPacket;
	BOOLEAN opmode = pAd->OpMode;

	pPacket = mpdu->pPacket;

	if (mpdu->bAMSDU && tr_ctl->damsdu_type == RX_SW_AMSDU)
		announce_non_hw_damsdu_pkt(pAd, pPacket, mpdu->OpMode);
	else {
		/* pass this 802.3 packet to upper layer or forward this packet to WM directly */
#ifdef P2P_SUPPORT
		opmode = mpdu->OpMode;
#endif /* P2P_SUPPORT */
		announce_or_forward_802_3_pkt(pAd, pPacket,
				wdev_search_by_idx(pAd, RTMP_GET_PACKET_WDEV(pPacket)), opmode);
	}
}

static void ba_mpdu_blk_free(struct ba_control *ba_ctl, struct reordering_mpdu *mpdu_blk)
{
	if (!mpdu_blk)
		return;
	NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[mpdu_blk->band].lock);
	ba_enqueue_head(&ba_ctl->mpdu_blk_pool[mpdu_blk->band].freelist, mpdu_blk);
	NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[mpdu_blk->band].lock);
}


static void ba_refresh_reordering_mpdus(RTMP_ADAPTER *pAd, struct ba_control *ba_ctl, BA_REC_ENTRY *pBAEntry)
{
	struct reordering_mpdu *mpdu_blk, *msdu_blk;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	/* dequeue in-order frame from reodering list */
	while ((mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list))) {
		announce_ba_reorder_pkt(pAd, mpdu_blk);

		while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
			announce_ba_reorder_pkt(pAd, msdu_blk);
			ba_mpdu_blk_free(ba_ctl, msdu_blk);
		}

		pBAEntry->LastIndSeq = mpdu_blk->Sequence;
		ba_mpdu_blk_free(ba_ctl, mpdu_blk);
		/* update last indicated sequence */
	}

	ASSERT(pBAEntry->list.qlen == 0);
	pBAEntry->CurMpdu = NULL;
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}

static VOID ba_ori_session_setup_timeout(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	BA_ORI_ENTRY *pBAEntry = (BA_ORI_ENTRY *)FunctionContext;
	MAC_TABLE_ENTRY *pEntry;
	RTMP_ADAPTER *pAd;

	if (pBAEntry == NULL)
		return;

	if (pBAEntry->ORI_BA_Status == Originator_Done)
		return;

	pAd = pBAEntry->pAdapter;
	pEntry = &pAd->MacTab.Content[pBAEntry->Wcid];
	if ((pEntry == NULL) || (pEntry->wdev == NULL)) {
		/* Do not enque wdev if entry is NULL */
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s(): Entry is NULL\n", __func__);
		return;
	}

	if ((pBAEntry->ORI_BA_Status == Originator_WaitRes) && (pBAEntry->Token < ORI_SESSION_MAX_RETRY)) {
		MLME_ADDBA_REQ_STRUCT AddbaReq;
#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT

		if ((pAd->OpMode == OPMODE_STA) && IS_ENTRY_CLIENT(pEntry) && IS_P2P_ENTRY_NONE(pEntry))
#else
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
		{
			PSTA_ADMIN_CONFIG pStaCfg = NULL;
			pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);

			if (pStaCfg && INFRA_ON(pStaCfg) &&
				RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
				(STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED))) {
				/* In scan progress and have no chance to send out, just re-schedule to another time period */
				RTMPSetTimer(&pBAEntry->ORIBATimer, ORI_BA_SESSION_TIMEOUT);
				return;
			}
		}

#endif /* CONFIG_STA_SUPPORT */
		NdisZeroMemory(&AddbaReq, sizeof(AddbaReq));
		COPY_MAC_ADDR(AddbaReq.pAddr, pEntry->Addr);
		AddbaReq.Wcid = pBAEntry->Wcid;
		AddbaReq.TID = pBAEntry->TID;
		AddbaReq.BaBufSize = pBAEntry->BAWinSize;
		AddbaReq.TimeOutValue = pBAEntry->TimeOutValue;
		AddbaReq.Token = pBAEntry->Token;
		AddbaReq.amsdu_support = pBAEntry->amsdu_cap;

		MlmeEnqueueWithWdev(pAd, ACTION_STATE_MACHINE, MT2_MLME_ADD_BA_CATE, sizeof(MLME_ADDBA_REQ_STRUCT), (PVOID)&AddbaReq, 0, pEntry->wdev);
		RTMP_MLME_HANDLER(pAd);

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "BA Ori Session Timeout(%d) : Send ADD BA again\n", pBAEntry->Token);
		pBAEntry->Token++;
		RTMPSetTimer(&pBAEntry->ORIBATimer, ORI_BA_SESSION_TIMEOUT);
	} else {
		/* either not in the right state or exceed retry count */
		ba_resrc_ori_del(pAd, pBAEntry->Wcid, pBAEntry->TID);
	}
}
BUILD_TIMER_FUNCTION(ba_ori_session_setup_timeout);

static VOID ba_rec_session_idle_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	BA_REC_ENTRY    *pBAEntry = (BA_REC_ENTRY *)FunctionContext;
	PRTMP_ADAPTER   pAd;
	ULONG           Now32;
	struct ba_control *ba_ctl = NULL;

	if (pBAEntry == NULL)
		return;

	if ((pBAEntry->REC_BA_Status == Recipient_Established)) {
		NdisGetSystemUpTime(&Now32);

		if (RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer + REC_BA_SESSION_IDLE_TIMEOUT))) {
			pAd = pBAEntry->pAdapter;
			ba_ctl = &pAd->tr_ctl.ba_ctl;
			ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);
			pBAEntry->REC_BA_Status = Recipient_Initialization;
			MTWF_PRINT("%ld: REC BA session Timeout\n", Now32);
		}
	}
}
BUILD_TIMER_FUNCTION(ba_rec_session_idle_timeout);

static BOOLEAN ba_reordering_mpdu_insertsorted(struct reordering_list *list,
										struct reordering_mpdu *mpdu)
{
	struct reordering_mpdu **ppScan = &list->next;

	while (*ppScan != NULL) {
		if (SEQ_SMALLER((*ppScan)->Sequence, mpdu->Sequence, MAXSEQ))
			ppScan = &(*ppScan)->next;
		else if ((*ppScan)->Sequence == mpdu->Sequence) {
			/* give up this duplicated frame */
			return FALSE;
		} else {
			/* find position */
			break;
		}
	}

	if (*ppScan == NULL)
		list->tail = mpdu;

	mpdu->next = *ppScan;
	*ppScan = mpdu;
	list->qlen++;
	return TRUE;
}

static VOID ba_resource_dump_sn(struct ba_control *ba_ctl, PMAC_TABLE_ENTRY pEntry)
{
	UINT32 i;
	BA_REC_ENTRY *pRecBAEntry;

	for (i = 0; i < 1; i++) {
		if (pEntry->BARecWcidArray[i] != 0) {
			pRecBAEntry = &ba_ctl->BARecEntry[pEntry->BARecWcidArray[i]];

			if (((pRecBAEntry->REC_BA_Status == Recipient_Established) ||
					(pRecBAEntry->REC_BA_Status == Recipient_Initialization))
					&& pRecBAEntry->ba_rec_dbg) {
				UINT j;
				struct ba_rec_debug *dbg;
				UINT k = pRecBAEntry->ba_rec_dbg_idx;
				for (j = k; j < BA_REC_DBG_SIZE; j++) {
					dbg = &pRecBAEntry->ba_rec_dbg[j];
					if (ba_ctl->dbg_flag & SN_RECORD_MAC) {
						MTWF_PRINT("idx(%d), wcid(%d),\
							ta("MACSTR"),\
							ra("MACSTR"),\
							sn(%d), amsdu(%d), type(%d), last_in_seq%d\n", j,
							dbg->wcid,
							MAC2STR(dbg->ta),
							MAC2STR(dbg->ra),
							dbg->sn, dbg->amsdu, dbg->type, dbg->last_in_seq);
					} else {
						MTWF_PRINT("idx(%d), wcid(%d) sn(%d), amsdu(%d), type(%d), last_in_seq%d\n", j,
							dbg->wcid, dbg->sn, dbg->amsdu, dbg->type, dbg->last_in_seq);
					}
				}
				for (j = 0; j < k; j++) {
					dbg = &pRecBAEntry->ba_rec_dbg[j];
					if (ba_ctl->dbg_flag & SN_RECORD_MAC) {
						MTWF_PRINT("idx(%d), wcid(%d),\
							ta("MACSTR"),\
							ra("MACSTR"),\
							sn(%d), amsdu(%d), type(%d), last_in_seq%d\n", j,
							dbg->wcid,
							MAC2STR(dbg->ta),
							MAC2STR(dbg->ra),
							dbg->sn, dbg->amsdu, dbg->type, dbg->last_in_seq);
					} else {
						MTWF_PRINT("idx(%d), wcid(%d) sn(%d), amsdu(%d), type(%d), last_in_seq%d\n", j,
							dbg->wcid, dbg->sn, dbg->amsdu, dbg->type, dbg->last_in_seq);
					}
				}
			}
		}
	}
}

VOID ba_resource_dump_all(RTMP_ADAPTER *pAd, ULONG second_idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = &tr_ctl->ba_ctl;
	INT i, j;
	BA_ORI_ENTRY *pOriBAEntry;
	BA_REC_ENTRY *pRecBAEntry;
	RTMP_STRING tmpBuf[10];
	int ret;
	struct reordering_mpdu *mpdu_blk = NULL, *msdu_blk = NULL;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst != SST_ASSOC))
			continue;

		if (IS_ENTRY_PEER_AP(pEntry)) {
			ret = snprintf(tmpBuf, sizeof(tmpBuf), "%s", "ApCli");
			if (os_snprintf_error(sizeof(tmpBuf), ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (IS_ENTRY_REPEATER(pEntry)) {
			ret = snprintf(tmpBuf, sizeof(tmpBuf), "%s", "Repeater");
			if (os_snprintf_error(sizeof(tmpBuf), ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (IS_ENTRY_WDS(pEntry)) {
			ret = snprintf(tmpBuf, sizeof(tmpBuf), "%s", "WDS");
			if (os_snprintf_error(sizeof(tmpBuf), ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (IS_ENTRY_MESH(pEntry)) {
			ret = snprintf(tmpBuf, sizeof(tmpBuf), "%s", "Mesh");
			if (os_snprintf_error(sizeof(tmpBuf), ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else {
			ret = snprintf(tmpBuf, sizeof(tmpBuf), "%s", "STA");
			if (os_snprintf_error(sizeof(tmpBuf), ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		}

		MTWF_PRINT(""MACSTR" (Aid = %d) (%s) -\n", MAC2STR(pEntry->Addr), pEntry->Aid, tmpBuf);
		MTWF_PRINT("[Originator]\n");

		for (j = 0; j < NUM_OF_TID; j++) {
			if (pEntry->BAOriWcidArray[j] != 0) {
				pOriBAEntry = &ba_ctl->BAOriEntry[pEntry->BAOriWcidArray[j]];

				if (pOriBAEntry->ORI_BA_Status == Originator_Done)
					MTWF_PRINT(" mac="MACSTR", TID=%d, BAWinSize=%d, StartSeq=%d, CurTxSeq=%d\n",
							 MAC2STR(pEntry->Addr), j, pOriBAEntry->BAWinSize, pOriBAEntry->Sequence,
							 tr_ctl->tr_entry[pEntry->wcid].TxSeq[j]);
			}
		}

		MTWF_PRINT("\n");
		MTWF_PRINT("[Recipient]\n");

		for (j = 0; j < NUM_OF_TID; j++) {
			if (pEntry->BARecWcidArray[j] != 0) {
				pRecBAEntry = &ba_ctl->BARecEntry[pEntry->BARecWcidArray[j]];

				if ((pRecBAEntry->REC_BA_Status == Recipient_Established) ||
						(pRecBAEntry->REC_BA_Status == Recipient_Initialization) ||
						(pRecBAEntry->REC_BA_Status == recipient_offload)) {
					MTWF_PRINT("State=%d, TID=%d, BAWinSize=%d, LastIndSeq=%d, ReorderingPkts=%d, FreeMpduBls=%d\n",
						pRecBAEntry->REC_BA_Status, j, pRecBAEntry->BAWinSize,
						pRecBAEntry->LastIndSeq, pRecBAEntry->list.qlen, ba_ctl->mpdu_blk_pool[pRecBAEntry->band].freelist.qlen);
					MTWF_PRINT("drop(duplicated) pkts = %ld, drop(old) pkts = %ld,\
						drop(unknown) state pkts = %ld, sn_large_win_end = %ld\n\n",
						pRecBAEntry->drop_dup_pkts,
						pRecBAEntry->drop_old_pkts,
						pRecBAEntry->drop_unknown_state_pkts,
						pRecBAEntry->ba_sn_large_win_end);
				}
			}
		}

		if (second_idx == SN_HISTORY)
			ba_resource_dump_sn(ba_ctl, pEntry);

		MTWF_PRINT("\n");
		MTWF_PRINT("[RX ReorderBuffer]\n");

		for (j = 0; j < NUM_OF_TID; j++) {
			if (pEntry->BARecWcidArray[j] != 0) {
				pRecBAEntry = &ba_ctl->BARecEntry[pEntry->BARecWcidArray[j]];
				mpdu_blk = ba_reordering_mpdu_probe(&pRecBAEntry->list);

				if (mpdu_blk) {
					MTWF_PRINT("mpdu:SN = %d, AMSDU = %d\n", mpdu_blk->Sequence, mpdu_blk->bAMSDU);
					msdu_blk = ba_reordering_mpdu_probe(&mpdu_blk->AmsduList);

					if (msdu_blk) {
						MTWF_PRINT("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence, msdu_blk->bAMSDU);
						while (msdu_blk->next) {
							msdu_blk = msdu_blk->next;
							MTWF_PRINT("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence, msdu_blk->bAMSDU);
						}
					}

					while (mpdu_blk->next) {
						mpdu_blk = mpdu_blk->next;
						MTWF_PRINT("mpdu:SN = %d, AMSDU = %d\n", mpdu_blk->Sequence, mpdu_blk->bAMSDU);
						msdu_blk = ba_reordering_mpdu_probe(&mpdu_blk->AmsduList);

						if (msdu_blk) {
							MTWF_PRINT("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence, msdu_blk->bAMSDU);

							while (msdu_blk->next) {
								msdu_blk = msdu_blk->next;
								MTWF_PRINT("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence, msdu_blk->bAMSDU);
							}
						}
					}
				}
			}
		}
	}

	return;
}

VOID ba_reordering_resource_dump_all(RTMP_ADAPTER *pAd)
{
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	BA_REC_ENTRY *pBAEntry;
	struct reordering_mpdu *mpdu_blk, *msdu_blk;
	int i;
	UINT32 total_pkt_cnt = 0;

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &ba_ctl->BARecEntry[i];
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		if (pBAEntry->list.next) {
			mpdu_blk = pBAEntry->list.next;
			while (mpdu_blk) {
				if (mpdu_blk->AmsduList.next) {
					msdu_blk = mpdu_blk->AmsduList.next;
					while (msdu_blk) {
						msdu_blk = msdu_blk->next;
						total_pkt_cnt++;
					}
				}
				total_pkt_cnt++;
				mpdu_blk = mpdu_blk->next;
			}
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}

	MTWF_PRINT("total %d msdu packt in ba list\n", total_pkt_cnt);
}

VOID ba_reodering_resource_dump(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	int i, j;
	BA_REC_ENTRY *pBAEntry;
	UINT32 total_pkt_cnt = 0;
	struct reordering_mpdu *mpdu_blk, *msdu_blk;

	if (!(VALID_UCAST_ENTRY_WCID(pAd, wcid)))
		return;

	for (i = 0; i < NUM_OF_TID; i++) {

		j = pAd->MacTab.Content[wcid].BARecWcidArray[i];

		if (j == 0)
			continue;

		pBAEntry = &ba_ctl->BARecEntry[j];

		if (pBAEntry->list.next) {
			mpdu_blk = pBAEntry->list.next;
			while (mpdu_blk) {
				if (mpdu_blk->AmsduList.next) {
					msdu_blk = mpdu_blk->AmsduList.next;
					while (msdu_blk) {
						msdu_blk = msdu_blk->next;
						total_pkt_cnt++;
					}
				}
				total_pkt_cnt++;
				mpdu_blk = mpdu_blk->next;
			}
		}
	}

	MTWF_PRINT("total %d msdu packt in wcid (%d) ba list\n", total_pkt_cnt, wcid);
}

/* free all resource for reordering mechanism */
void ba_reordering_resource_release(RTMP_ADAPTER *pAd)
{
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	BA_REC_ENTRY *pBAEntry;
	struct reordering_mpdu *mpdu_blk, *msdu_blk;
	int i;
#ifdef WHNAT_SUPPORT
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->CommonCfg.whnat_en &&
		(cap->asic_caps & fASIC_CAP_BA_OFFLOAD))
		return;
#endif

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &ba_ctl->BARecEntry[i];
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

		if (pBAEntry->REC_BA_Status != Recipient_NONE) {
			while ((mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list))) {
				while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
					RELEASE_NDIS_PACKET(pAd, msdu_blk->pPacket, NDIS_STATUS_FAILURE);
					ba_mpdu_blk_free(ba_ctl, msdu_blk);
				}

				ASSERT(mpdu_blk->pPacket);
				RELEASE_NDIS_PACKET(pAd, mpdu_blk->pPacket, NDIS_STATUS_FAILURE);
				ba_mpdu_blk_free(ba_ctl, mpdu_blk);
			}
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}

	ASSERT(pBAEntry->list.qlen == 0);
	/* II. free memory of reordering mpdu table */
	for (i = 0; i < 2; i++) {
		NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
		os_free_mem(ba_ctl->mpdu_blk_pool[i].mem);
		NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
	}
}

BOOLEAN ba_reordering_resource_init(RTMP_ADAPTER *pAd, int num)
{
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	int i, j;
	PUCHAR mem;
	struct reordering_mpdu *mpdu_blk;
	struct reordering_list *freelist;
#ifdef WHNAT_SUPPORT
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->CommonCfg.whnat_en &&
		(cap->asic_caps & fASIC_CAP_BA_OFFLOAD))
		return TRUE;
#endif

	for (i = 0; i < 2; i++) {
		/* allocate spinlock */
		NdisAllocateSpinLock(pAd, &ba_ctl->mpdu_blk_pool[i].lock);
		/* initialize freelist */
		NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
		freelist = &ba_ctl->mpdu_blk_pool[i].freelist;
		freelist->next = NULL;
		freelist->qlen = 0;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "Allocate %d memory for BA reordering\n", (UINT32)(num * sizeof(struct reordering_mpdu)));
		/* allocate number of mpdu_blk memory */
		os_alloc_mem(pAd, (PUCHAR *)&mem, (num * sizeof(struct reordering_mpdu)));
		ba_ctl->mpdu_blk_pool[i].mem = mem;

		if (mem == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "Can't Allocate Memory for BA Reordering\n");
			NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
			return FALSE;
		}


		/* build mpdu_blk free list */
		for (j = 0; j < num; j++) {
			/* get mpdu_blk */
			mpdu_blk = (struct reordering_mpdu *)mem;
			/* initial mpdu_blk */
			NdisZeroMemory(mpdu_blk, sizeof(struct reordering_mpdu));
			/* next mpdu_blk */
			mem += sizeof(struct reordering_mpdu);
			/* insert mpdu_blk into freelist */
			ba_enqueue_head(freelist, mpdu_blk);
		}
		NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
	}

	return TRUE;
}

static struct reordering_mpdu *ba_mpdu_blk_alloc(RTMP_ADAPTER *pAd, RX_BLK *rx_blk)
{
	struct reordering_mpdu *mpdu_blk;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;

	NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[rx_blk->band].lock);
	mpdu_blk = ba_dequeue_head(&ba_ctl->mpdu_blk_pool[rx_blk->band].freelist);

	if (mpdu_blk) {
		NdisZeroMemory(mpdu_blk, sizeof(*mpdu_blk));
		mpdu_blk->band = rx_blk->band;
	}

	NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[rx_blk->band].lock);
	return mpdu_blk;
}

static USHORT ba_indicate_reordering_mpdus_in_order(PRTMP_ADAPTER pAd,
		struct ba_control *ba_ctl,
		PBA_REC_ENTRY pBAEntry,
		USHORT StartSeq)
{
	struct reordering_mpdu *mpdu_blk, *msdu_blk;
	USHORT  LastIndSeq = INVALID_RCV_SEQ;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	while ((mpdu_blk = ba_reordering_mpdu_probe(&pBAEntry->list))) {
		/* find in-order frame */
		if (!SEQ_STEPONE(mpdu_blk->Sequence, StartSeq, MAXSEQ))
			break;

		/* dequeue in-order frame from reodering list */
		mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list);
		/* pass this frame up */
		announce_ba_reorder_pkt(pAd, mpdu_blk);
		/* move to next sequence */
		StartSeq = mpdu_blk->Sequence;
		LastIndSeq = StartSeq;

		while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
			announce_ba_reorder_pkt(pAd, msdu_blk);
			ba_mpdu_blk_free(ba_ctl, msdu_blk);
		}

		/* free mpdu_blk */
		ba_mpdu_blk_free(ba_ctl, mpdu_blk);
	}

	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	/* update last indicated sequence */
	return LastIndSeq;
}

static void ba_indicate_reordering_mpdus_le_seq(PRTMP_ADAPTER pAd,
		struct ba_control *ba_ctl,
		PBA_REC_ENTRY pBAEntry,
		USHORT Sequence)
{
	struct reordering_mpdu *mpdu_blk, *msdu_blk = NULL;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	while ((mpdu_blk = ba_reordering_mpdu_probe(&pBAEntry->list))) {
		/* find in-order frame */
		if ((mpdu_blk->Sequence == Sequence) || SEQ_SMALLER(mpdu_blk->Sequence, Sequence, MAXSEQ)) {
			/* dequeue in-order frame from reodering list */
			mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list);
			/* pass this frame up */
			announce_ba_reorder_pkt(pAd, mpdu_blk);

			while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
				announce_ba_reorder_pkt(pAd, msdu_blk);
				ba_mpdu_blk_free(ba_ctl, msdu_blk);
			}

			/* free mpdu_blk */
			ba_mpdu_blk_free(ba_ctl, mpdu_blk);
		} else
			break;
	}

	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}

#ifdef RX_RPS_SUPPORT
void ba_timeout_flush_by_cpu(PRTMP_ADAPTER pAd)
{
	ULONG now;
	UINT32 idx0 = 0;
	UINT32 idx1 = 0;
	PBA_REC_ENTRY pBAEntry = NULL;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	UINT32 cpu = smp_processor_id(), the_cpu = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	NdisGetSystemUpTime(&now);

	for (idx0 = 0; idx0 < BA_TIMEOUT_BITMAP_LEN; idx0++) {
		idx1 = 0;
		while ((ba_ctl->ba_timeout_bitmap_per_cpu[cpu][idx0] != 0) && (idx1 < 32)) {
			if (ba_ctl->ba_timeout_bitmap_per_cpu[cpu][idx0] & 0x1) {
				pBAEntry = &ba_ctl->BARecEntry[(idx0 << 5) + idx1];
				the_cpu = cap->RxSwRpsCpuMap[((pBAEntry->Wcid-1) % cap->RxSwRpsNum)];

				if (cpu == the_cpu)
					ba_flush_reordering_timeout_mpdus(pAd, ba_ctl, pBAEntry, now);
			}

			ba_ctl->ba_timeout_bitmap_per_cpu[cpu][idx0] >>= 1;
			idx1++;
		}
	}

	ba_ctl->ba_timeout_check_per_cpu[cpu] = FALSE;
}

void ba_timeout_monitor_per_cpu(PRTMP_ADAPTER pAd)
{
	UINT32 idx = 0, cpu = 0;
	PBA_REC_ENTRY pBAEntry = NULL;
	ULONG now;
	BOOLEAN need_check[NR_CPUS] = {FALSE};
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);


	NdisGetSystemUpTime(&now);
	for (idx = 0; idx < MAX_LEN_OF_BA_REC_TABLE; idx++) {
		pBAEntry = &ba_ctl->BARecEntry[idx];
		cpu = cap->RxSwRpsCpuMap[((pBAEntry->Wcid-1) % cap->RxSwRpsNum)];
		if (!ba_ctl->ba_timeout_check_per_cpu[cpu]) {
			if ((pBAEntry->REC_BA_Status == Recipient_Established)
					&& (pBAEntry->list.qlen > 0)) {
				if (RTMP_TIME_AFTER((unsigned long)now,
#ifdef IXIA_C50_MODE
				(unsigned long)(pBAEntry->LastIndSeqAtTimer + pAd->ixia_ctl.BA_timeout)))
#else
				(unsigned long)(pBAEntry->LastIndSeqAtTimer + REORDERING_PACKET_TIMEOUT)))
#endif
				{
					need_check[cpu] = TRUE;
					ba_ctl->ba_timeout_bitmap_per_cpu[cpu][(idx >> 5)] |= (1 << (idx % 32));
				}

			}
		}
	}

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		if (need_check[cpu])
			ba_ctl->ba_timeout_check_per_cpu[cpu] = need_check[cpu];

	}
}
#endif

void ba_timeout_flush(PRTMP_ADAPTER pAd)
{
	ULONG now;
	UINT32 idx0 = 0;
	UINT32 idx1 = 0;
	PBA_REC_ENTRY pBAEntry = NULL;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
#ifdef RX_RPS_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#ifdef RX_RPS_SUPPORT
	if (cap->rx_qm_en)
		return ba_timeout_flush_by_cpu(pAd);
#endif
	NdisGetSystemUpTime(&now);

	for (idx0 = 0; idx0 < BA_TIMEOUT_BITMAP_LEN; idx0++) {
		idx1 = 0;
		while ((ba_ctl->ba_timeout_bitmap[idx0] != 0) && (idx1 < 32)) {
			if (ba_ctl->ba_timeout_bitmap[idx0] & 0x1) {
				pBAEntry = &ba_ctl->BARecEntry[(idx0 << 5) + idx1];
				ba_flush_reordering_timeout_mpdus(pAd, ba_ctl, pBAEntry, now);
			}

			ba_ctl->ba_timeout_bitmap[idx0] >>= 1;
			idx1++;
		}
	}

	ba_ctl->ba_timeout_check = FALSE;
}

void ba_timeout_monitor(PRTMP_ADAPTER pAd)
{
	UINT32 idx = 0;
	PBA_REC_ENTRY pBAEntry = NULL;
	ULONG now;
	BOOLEAN need_check = FALSE;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
#ifdef RX_RPS_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#ifdef RX_RPS_SUPPORT
	if (cap->rx_qm_en)
		return ba_timeout_monitor_per_cpu(pAd);
#endif
	if (!ba_ctl->ba_timeout_check) {
		NdisGetSystemUpTime(&now);
		for (idx = 0; idx < MAX_LEN_OF_BA_REC_TABLE; idx++) {
			pBAEntry = &ba_ctl->BARecEntry[idx];
			if ((pBAEntry->REC_BA_Status == Recipient_Established)
					&& (pBAEntry->list.qlen > 0)) {

				if (RTMP_TIME_AFTER((unsigned long)now,
#ifdef IXIA_C50_MODE
				(unsigned long)(pBAEntry->LastIndSeqAtTimer + pAd->ixia_ctl.BA_timeout)))
#else
				(unsigned long)(pBAEntry->LastIndSeqAtTimer + REORDERING_PACKET_TIMEOUT)))
#endif
				{
					need_check = TRUE;
					ba_ctl->ba_timeout_bitmap[(idx >> 5)] |= (1 << (idx % 32));
				}

			}
		}

		if (need_check) {
#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
				struct hdev_ctrl *hdev_ctrl = pAd->hdev_ctrl;
				struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
				struct pci_hif_chip *hif_chip = pci_hif->main_hif_chip;
				struct pci_schedule_task_ops *sched_ops = hif_chip->schedule_task_ops;

				if (IS_ASIC_CAP(pAd, fASIC_CAP_DLY_INT_LUMPED))
					sched_ops->schedule_rx_dly_done(&hif_chip->task_group);
				else
					sched_ops->schedule_rx_data_done(&hif_chip->task_group);
			}
#endif
			ba_ctl->ba_timeout_check = need_check;
		}
	}
}


void ba_flush_reordering_timeout_mpdus(PRTMP_ADAPTER pAd,
					struct ba_control *ba_ctl,
					PBA_REC_ENTRY pBAEntry,
					ULONG Now32)

{
	USHORT Sequence;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;

	if ((pBAEntry == NULL) || (pBAEntry->list.qlen <= 0))
		return;

	if (RTMP_TIME_AFTER((unsigned long)Now32,
#ifdef IXIA_C50_MODE
	(unsigned long)(pBAEntry->LastIndSeqAtTimer + (pAd->ixia_ctl.max_BA_timeout / 6)))
#else
	(unsigned long)(pBAEntry->LastIndSeqAtTimer + (MAX_REORDERING_PACKET_TIMEOUT / 6)))
#endif
		&& (pBAEntry->list.qlen > 0)
	   ) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_DEBUG, "timeout[%d] (%08lx-%08lx = %d > %d): %x, flush all!\n ", pBAEntry->list.qlen, Now32, (pBAEntry->LastIndSeqAtTimer),
				 (int)((long) Now32 - (long)(pBAEntry->LastIndSeqAtTimer)), MAX_REORDERING_PACKET_TIMEOUT,
				 pBAEntry->LastIndSeq);
		ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);
		tr_cnt->ba_flush_all++;
#ifdef IXIA_C50_MODE
		pAd->rx_cnt.rx_flush_drop[pBAEntry->Wcid]++;
#endif
		pBAEntry->LastIndSeqAtTimer = Now32;
	} else if (RTMP_TIME_AFTER((unsigned long)Now32,
#ifdef IXIA_C50_MODE
		(unsigned long)(pBAEntry->LastIndSeqAtTimer + (pAd->ixia_ctl.BA_timeout)))
#else
		(unsigned long)(pBAEntry->LastIndSeqAtTimer + (REORDERING_PACKET_TIMEOUT)))
#endif
				&& (pBAEntry->list.qlen > 0)
			  ) {
		/* force LastIndSeq to shift to LastIndSeq+1*/
		Sequence = (pBAEntry->LastIndSeq + 1) & MAXSEQ;
		ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, pBAEntry, Sequence);
		pBAEntry->LastIndSeq = Sequence;
		/* indicate in-order mpdus*/
		Sequence = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, Sequence);

		if (Sequence != INVALID_RCV_SEQ) {
			/* update timer value only if in order frame is passed up */
			pBAEntry->LastIndSeqAtTimer = Now32;
			pBAEntry->LastIndSeq = Sequence;
		}

		tr_cnt->ba_flush_one++;
#ifdef IXIA_C50_MODE
		pAd->rx_cnt.rx_flush_drop[pBAEntry->Wcid]++;
#endif
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_DEBUG, "%x, flush one!\n", pBAEntry->LastIndSeq);
	}

}

static BA_ORI_ENTRY *ba_alloc_ori_entry(RTMP_ADAPTER *pAd, USHORT *Idx)
{
	int i;
	BA_ORI_ENTRY *pBAEntry = NULL;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;

	NdisAcquireSpinLock(&ba_ctl->BATabLock);

	if (ba_ctl->numAsOriginator >= (MAX_LEN_OF_BA_ORI_TABLE - 1)) {
		NdisReleaseSpinLock(&ba_ctl->BATabLock);
		goto done;
	}

	NdisReleaseSpinLock(&ba_ctl->BATabLock);

	/* reserve idx 0 to identify BAWcidArray[TID] as empty*/
	for (i = 1; i < MAX_LEN_OF_BA_ORI_TABLE; i++) {
		pBAEntry = &ba_ctl->BAOriEntry[i];
		if ((pBAEntry->ORI_BA_Status == Originator_NONE)) {
			ba_ctl->numAsOriginator++;
			pBAEntry->ORI_BA_Status = Originator_USED;
			pBAEntry->pAdapter = pAd;
			*Idx = i;
			break;
		}
	}

done:
	return pBAEntry;
}

static UINT16 cal_ori_ba_wsize(struct _MAC_TABLE_ENTRY *peer,
		UINT16 cfg_ori_wsize, UINT16 peer_rec_wsize)
{
	struct ppdu_caps *ppdu =
		(struct ppdu_caps *)wlan_config_get_ppdu_caps(peer->wdev);
	UINT16 ori_ba_wsize = 0;

	/* select our own originator win size */
	if (WMODE_CAP_AX(peer->wdev->PhyMode))
		ori_ba_wsize = ppdu->he_tx_ba_wsize;
	else
		ori_ba_wsize = ppdu->non_he_tx_ba_wsize;
	ori_ba_wsize = min(cfg_ori_wsize, ori_ba_wsize);

	/* peer recipient win size sanity check */
	if (!IS_HE_STA(peer->cap.modes)) {
		if (peer_rec_wsize == 0) {
			ori_ba_wsize = min(cfg_ori_wsize, (UINT16)MAX_HT_REORDERBUF);
			return ori_ba_wsize;
		} else
			peer_rec_wsize = min(peer_rec_wsize, (UINT16)MAX_HT_REORDERBUF);
	}

	/* if peer recipient win size is invalid, ignore it */
	if (peer_rec_wsize == 0)
		return ori_ba_wsize;

	/* intersection of ori and rec wsize if rec wsize is valid */
	ori_ba_wsize = min(peer_rec_wsize, ori_ba_wsize);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"addbarsp,(our)txba=%d,(peer)rxba=%d\n",
			ori_ba_wsize, peer_rec_wsize);

	return ori_ba_wsize;
}

static UINT16 cal_rec_ba_wsize(struct _MAC_TABLE_ENTRY *peer,
		UINT16 cfg_rec_wsize, UINT16 peer_ori_wsize)
{
	struct ppdu_caps *ppdu =
		(struct ppdu_caps *)wlan_config_get_ppdu_caps(peer->wdev);
	UINT16 rec_ba_wsize;

	/* select our own recipient win size */
	if (WMODE_CAP_AX(peer->wdev->PhyMode))
		rec_ba_wsize = ppdu->he_rx_ba_wsize;
	else
		rec_ba_wsize = ppdu->non_he_rx_ba_wsize;
	rec_ba_wsize = min(cfg_rec_wsize, rec_ba_wsize);

	/* peer originator win size sanity check */
	if (peer_ori_wsize == 0)
		peer_ori_wsize = MAX_HT_REORDERBUF;
	if (!IS_HE_STA(peer->cap.modes))
		peer_ori_wsize = min(peer_ori_wsize, (UINT16)MAX_HT_REORDERBUF);

	/* intersection of ori and rec wsize */
	rec_ba_wsize = min(peer_ori_wsize, rec_ba_wsize);

	/* SPEC define the buffer size of add ba resp should be at least 1 */
	if (rec_ba_wsize < 1)
		rec_ba_wsize = 1;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"[Peer is Originator]recv. add ba req, (peer:ori)tx_ba_wsize=%d, (our:rec)rx_ba_wsize=%d \n",
			peer_ori_wsize, rec_ba_wsize);

	return rec_ba_wsize;
}

VOID ba_ori_session_setup(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR TID,
	USHORT TimeOut)
{
	BA_ORI_ENTRY *pBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;
	USHORT Idx;
	UCHAR tx_mode;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = &tr_ctl->ba_ctl;
	UINT16 ori_ba_wsize = 0;
	UINT16 cfg_tx_ba_wsize = 0;
	ULONG DelayTime;
	UCHAR amsdu_en = 0;
	struct ppdu_caps *ppdu;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "%s: wcid = %d, tid = %d\n", __func__, wcid, TID);

	/* sanity check */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* Do nothing if monitor mode is on*/
		if (MONITOR_ON(pAd))
			return;
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_ATE

	/* Nothing to do in ATE mode. */
	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	if (TID >= NUM_OF_TID) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "Wrong TID %d!\n", TID);
		return;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	pEntry = &pAd->MacTab.Content[wcid];
	if (!pEntry || !pEntry->wdev)
		return;

	/* if this entry is limited to use legacy tx mode, it doesn't generate BA.  */
	tx_mode = RTMPStaFixedTxMode(pAd, pEntry);
	if (tx_mode == FIXED_TXMODE_CCK || tx_mode == FIXED_TXMODE_OFDM)
		return;

	/* parameter decision */
	cfg_tx_ba_wsize = wlan_config_get_ba_tx_wsize(pEntry->wdev);
	ori_ba_wsize = cal_ori_ba_wsize(pEntry, cfg_tx_ba_wsize, 0);
	ppdu = (struct ppdu_caps *)wlan_config_get_ppdu_caps(pEntry->wdev);
	amsdu_en = wlan_config_get_amsdu_en(pEntry->wdev) && ppdu->tx_amsdu_support;

	/* resource management */
	if (!ba_resrc_ori_prep(pAd, wcid, TID, ori_ba_wsize, amsdu_en, TimeOut))
		return;

	/* set timer to send add ba request */
	if (pEntry->BADeclineBitmap & (1 << TID))
		DelayTime = 3000; /* request has been declined, try again after 3 secs*/
	else
		DelayTime = 10;

	Idx = pEntry->BAOriWcidArray[TID];
	pBAEntry = &ba_ctl->BAOriEntry[Idx];
	if (!pBAEntry)
		return;

	RTMPSetTimer(&pBAEntry->ORIBATimer, DelayTime);
}

BOOLEAN ba_resrc_ori_prep(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 wcid, UCHAR TID, UINT16 ba_wsize, UCHAR amsdu_en, USHORT timeout)
{
	MAC_TABLE_ENTRY *pEntry;
	BA_ORI_ENTRY *pBAEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	USHORT Idx;
	BOOLEAN Cancelled;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = &tr_ctl->ba_ctl;

	pEntry = &pAd->MacTab.Content[wcid];
	Idx = pEntry->BAOriWcidArray[TID];

	if (Idx == 0)
		pBAEntry = ba_alloc_ori_entry(pAd, &Idx);
	else
		pBAEntry = &ba_ctl->BAOriEntry[Idx];

	if (!pBAEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"%s(): alloc BA session failed\n", __func__);
		return FALSE;
	}

	if (pBAEntry->ORI_BA_Status >= Originator_WaitRes) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
				"%s(): ori BA session already exist, status = %d\n", __func__, pBAEntry->ORI_BA_Status);
		if (pEntry->BAAutoTest)
			MTWF_PRINT(
				"<addba_info>: sta_mac="MACSTR", amsdu=%d, bapolice=%d,tid=%d, buffersize=%d\n",
				MAC2STR(pEntry->Addr), amsdu_en, 1, TID, ba_wsize);
		return FALSE;
	}

	pEntry->BAOriWcidArray[TID] = Idx;
	pBAEntry->ORI_BA_Status = Originator_WaitRes;
	pBAEntry->Wcid = wcid;
	pBAEntry->BAWinSize = ba_wsize;
	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
	pBAEntry->Sequence = BA_ORI_INIT_SEQ;
	pBAEntry->Token = 1;	/* (2008-01-21) Jan Lee recommends it - this token can't be 0*/
	pBAEntry->TID = TID;
	pBAEntry->TimeOutValue = timeout;
	pBAEntry->amsdu_cap = amsdu_en;
	pBAEntry->pAdapter = pAd;

	if (!(pEntry->TXBAbitmap & (1 << TID)))
		RTMPInitTimer(pAd, &pBAEntry->ORIBATimer,
				GET_TIMER_FUNCTION(ba_ori_session_setup_timeout),
				pBAEntry, FALSE);
	else
		RTMPCancelTimer(&pBAEntry->ORIBATimer, &Cancelled);

	return TRUE;
}

BOOLEAN ba_resrc_ori_add(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 wcid, UCHAR TID, UINT16 ba_wsize, UCHAR amsdu_en, USHORT timeout)
{
	MAC_TABLE_ENTRY *pEntry;
	BOOLEAN Cancelled;
	USHORT Idx;
	BA_ORI_ENTRY  *pBAEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = &tr_ctl->ba_ctl;

	pEntry = &pAd->MacTab.Content[wcid];
	Idx = pEntry->BAOriWcidArray[TID];
	pBAEntry = &ba_ctl->BAOriEntry[Idx];

	if (Idx == 0)
		return FALSE;

	/* make sure originator is waiting for response, in case unsolicited add ba response is recieved. */
	if (pBAEntry->ORI_BA_Status == Originator_WaitRes) {
		pBAEntry->BAWinSize = ba_wsize;
		pBAEntry->TimeOutValue = timeout;
		pBAEntry->amsdu_cap = amsdu_en;
		pBAEntry->ORI_BA_Status = Originator_Done;
		ba_ctl->numDoneOriginator++;
		/* reset sequence number */
		tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
		pBAEntry->Sequence = BA_ORI_INIT_SEQ;
		/* Set Bitmap flag.*/
		pEntry->TXBAbitmap |= (1 << TID);
		pEntry->BADeclineBitmap &= ~(1 << TID);

		pEntry->tx_amsdu_bitmap &= ~(1 << TID);
		if (pBAEntry->amsdu_cap)
			pEntry->tx_amsdu_bitmap |= (1 << TID);

		RTMPCancelTimer(&pBAEntry->ORIBATimer, &Cancelled);
		pBAEntry->ORIBATimer.TimerValue = 0;	/*pFrame->TimeOutValue;*/
		RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pEntry->wcid, TID, pBAEntry->Sequence, pBAEntry->BAWinSize, BA_SESSION_ORI, pBAEntry->amsdu_cap);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
				"%s():TXBAbitmap=%x, AMSDUCap=%d, BAWinSize=%d, TimeOut=%ld\n",
				 __func__, pEntry->TXBAbitmap, pBAEntry->amsdu_cap,
				 pBAEntry->BAWinSize, pBAEntry->ORIBATimer.TimerValue);

		if (pBAEntry->ORIBATimer.TimerValue)
			RTMPSetTimer(&pBAEntry->ORIBATimer, pBAEntry->ORIBATimer.TimerValue); /* in mSec */
	}

	return TRUE;
}

BOOLEAN ba_resrc_ori_del(
	RTMP_ADAPTER *pAd,
	UINT16 wcid, UCHAR tid)
{
	UINT Idx = 0;
	BA_ORI_ENTRY *pBAEntry;
	BOOLEAN Cancelled;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;

	if (tid >= NUM_OF_TID)
		return FALSE;
	Idx = pAd->MacTab.Content[wcid].BAOriWcidArray[tid];
	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_ORI_TABLE))
		return FALSE;

	pBAEntry = &ba_ctl->BAOriEntry[Idx];
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "%s():Idx=%d, Wcid=%d, TID=%d, ORI_BA_Status=%d\n",
		__func__, Idx, wcid, tid, pBAEntry->ORI_BA_Status);

	RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, wcid, tid, BA_SESSION_ORI, 0);

	RTMPReleaseTimer(&pBAEntry->ORIBATimer, &Cancelled);
	ba_free_ori_entry(pAd, Idx);

	return TRUE;
}

static BA_REC_ENTRY *ba_alloc_rec_entry(RTMP_ADAPTER *pAd, USHORT *Idx)
{
	int i;
	BA_REC_ENTRY *pBAEntry = NULL;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;

	NdisAcquireSpinLock(&ba_ctl->BATabLock);

	if (ba_ctl->numAsRecipient >= (MAX_LEN_OF_BA_REC_TABLE - 1)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "BA Recipeint Session (%ld) > %d\n",
				 ba_ctl->numAsRecipient, (MAX_LEN_OF_BA_REC_TABLE - 1));
		NdisReleaseSpinLock(&ba_ctl->BATabLock);
		goto done;
	}
	NdisReleaseSpinLock(&ba_ctl->BATabLock);

	/* reserve idx 0 to identify BAWcidArray[TID] as empty*/
	for (i = 1; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &ba_ctl->BARecEntry[i];
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		if ((pBAEntry->REC_BA_Status == Recipient_NONE)) {
			/* get one */
			ba_ctl->numAsRecipient++;
			pBAEntry->REC_BA_Status = Recipient_USED;
			*Idx = i;
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
			break;
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}

done:
	return pBAEntry;
}

BOOLEAN ba_resrc_rec_add(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR tid, USHORT timeout, UINT16 ba_buffer_size)
{
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	MAC_TABLE_ENTRY *pEntry;
	BA_REC_ENTRY *pBAEntry = NULL;
	BOOLEAN Status = TRUE;
	USHORT Idx;
#ifdef WHNAT_SUPPORT
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	BOOLEAN ba_offload = pAd->CommonCfg.whnat_en &&
							(cap->asic_caps & fASIC_CAP_BA_OFFLOAD);
#else
	BOOLEAN ba_offload = FALSE;
#endif

	pEntry = &pAd->MacTab.Content[wcid];
	ASSERT(pEntry);
	/* get software BA rec array index, Idx*/
	Idx = pEntry->BARecWcidArray[tid];

	if (Idx == 0)
		pBAEntry = ba_alloc_rec_entry(pAd, &Idx);
	else {
		pBAEntry = &ba_ctl->BARecEntry[Idx];
		ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "%s(%ld): Idx = %d, BAWinSize = %d\n",
			 __func__, ba_ctl->numAsRecipient, Idx, ba_buffer_size);

	/* Start fill in parameters.*/
	if (pBAEntry != NULL) {
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		if (ba_offload)
			pBAEntry->REC_BA_Status = recipient_offload;
		else
			pBAEntry->REC_BA_Status = Recipient_Initialization;
		pBAEntry->BAWinSize = ba_buffer_size;
		pBAEntry->Wcid = pEntry->wcid;
		pBAEntry->TID = tid;
		pBAEntry->TimeOutValue = timeout;
		pBAEntry->check_amsdu_miss = TRUE;
		pBAEntry->band = HcGetBandByWdev(pEntry->wdev);

		if ((ba_ctl->dbg_flag & SN_HISTORY) && (ba_ctl->numAsRecipient < 5)) {
			if (pBAEntry->ba_rec_dbg != NULL) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "%s Freeing previous Debug BA Rec %p\n",
					__func__, pBAEntry->ba_rec_dbg);
				os_free_mem(pBAEntry->ba_rec_dbg);
				pBAEntry->ba_rec_dbg = NULL;
			}
			os_alloc_mem(NULL, (UCHAR **)&pBAEntry->ba_rec_dbg, sizeof(struct ba_rec_debug) * BA_REC_DBG_SIZE);
			os_zero_mem(pBAEntry->ba_rec_dbg, sizeof(struct ba_rec_debug) * BA_REC_DBG_SIZE);
		}

		/* Set Bitmap flag.*/
		pEntry->RXBAbitmap |= (1 << tid);
		pEntry->BARecWcidArray[tid] = Idx;

		RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pBAEntry->Wcid, pBAEntry->TID, pBAEntry->LastIndSeq,
				pBAEntry->BAWinSize, BA_SESSION_RECP, 0);

		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);

		/* TODO: check BA reorder offload fail case in HW */
		if (ba_offload)
			ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "MACEntry[%d]RXBAbitmap = 0x%x. BARecWcidArray=%d\n",
				 pEntry->wcid, pEntry->RXBAbitmap, pEntry->BARecWcidArray[tid]);
	} else {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "Can't Accept ADDBA for "MACSTR" TID = %d\n",
				 MAC2STR(pEntry->Addr), tid);
	}

	return Status;
}

static VOID ba_free_rec_entry(RTMP_ADAPTER *pAd, ULONG Idx)
{
	BA_REC_ENTRY    *pBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;

	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_REC_TABLE))
		return;

	pBAEntry = &ba_ctl->BARecEntry[Idx];
	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	if (pBAEntry->REC_BA_Status != Recipient_NONE) {
		pEntry = &pAd->MacTab.Content[pBAEntry->Wcid];
		pEntry->BARecWcidArray[pBAEntry->TID] = 0;
		pEntry->RXBAbitmap &= (~(1 << (pBAEntry->TID)));
		pBAEntry->REC_BA_Status = Recipient_NONE;

		NdisAcquireSpinLock(&ba_ctl->BATabLock);
		if (ba_ctl->numAsRecipient > 0)
			ba_ctl->numAsRecipient -= 1;
		else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_WARN, "Idx = %lu, REC_BA_Status = %d, Wcid(pBAEntry) = %d,\
				Wcid(pEntry) = %d, Tid = %d\n", Idx, pBAEntry->REC_BA_Status, pBAEntry->Wcid, pEntry->wcid, pBAEntry->TID);
		}
		NdisReleaseSpinLock(&ba_ctl->BATabLock);

		if (ba_ctl->dbg_flag & SN_HISTORY) {
			if (pBAEntry->ba_rec_dbg) {
				os_free_mem(pBAEntry->ba_rec_dbg);
				pBAEntry->ba_rec_dbg = NULL;
			}
		 }
	}
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}

BOOLEAN ba_resrc_rec_del(
	RTMP_ADAPTER *pAd,
	UINT16 wcid, UCHAR tid)
{
	UINT Idx = 0;
	BA_REC_ENTRY *pBAEntry;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;

	if (tid >= NUM_OF_TID)
		return FALSE;
	Idx = pAd->MacTab.Content[wcid].BARecWcidArray[tid];
	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_ORI_TABLE))
		return FALSE;

	pBAEntry = &ba_ctl->BARecEntry[Idx];

	RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, wcid, tid, BA_SESSION_RECP, 0);

	ba_free_rec_entry(pAd, Idx);
	/*
	 * report all mpdu in reordering buffer after ba_free_rec_entry
	 * to make sure the reordering buffer will be empty.
	 */
	ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);

	return TRUE;
}

VOID ba_ori_session_tear_down(
	INOUT RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN UCHAR TID,
	IN BOOLEAN bPassive)
{
	MLME_DELBA_REQ_STRUCT	DelbaReq;
	MLME_QUEUE_ELEM *Elem;

	/* sanity check */
	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return;

	/* resource management */
	if (!ba_resrc_ori_del(pAd, Wcid, TID))
		return;

#ifdef WF_RESET_SUPPORT
	if (pAd->wf_reset_in_progress == TRUE)
		return;
#endif

	/* send del ba */
	if (bPassive == FALSE) {
		os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

		if (Elem != NULL) {
			NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
			NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
			COPY_MAC_ADDR(DelbaReq.Addr, pAd->MacTab.Content[Wcid].Addr);
			DelbaReq.Wcid = Wcid;
			DelbaReq.TID = TID;
			DelbaReq.Initiator = ORIGINATOR;
			Elem->MsgLen  = sizeof(DelbaReq);
			NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
			MlmeDELBAAction(pAd, Elem);
			os_free_mem(Elem);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "%s():alloc memory failed!\n", __func__);
			return;
		}
	}
}

VOID ba_rec_session_tear_down(
	PRTMP_ADAPTER pAd,
	UINT16 Wcid,
	UCHAR TID,
	BOOLEAN bPassive)
{
	MLME_DELBA_REQ_STRUCT DelbaReq;
	MLME_QUEUE_ELEM *Elem;

	/* sanity check */
	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return;

	/* resource management */
	if (!ba_resrc_rec_del(pAd, Wcid, TID))
		return;

#ifdef WF_RESET_SUPPORT
	if (pAd->wf_reset_in_progress == TRUE)
		return;
#endif

	/* send del ba */
	if (bPassive == FALSE) {
		os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

		if (Elem != NULL) {
			NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
			NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
			COPY_MAC_ADDR(DelbaReq.Addr, pAd->MacTab.Content[Wcid].Addr);
			DelbaReq.Wcid = Wcid;
			DelbaReq.TID = TID;
			DelbaReq.Initiator = RECIPIENT;
			Elem->MsgLen  = sizeof(DelbaReq);
			NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
			MlmeDELBAAction(pAd, Elem);
			os_free_mem(Elem);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "%s():alloc memory failed!\n", __func__);
			return;
		}
	}
}


VOID ba_session_tear_down_all(RTMP_ADAPTER *pAd, UINT16 Wcid, BOOLEAN bPassive)
{
	int i;

	for (i = 0; i < NUM_OF_TID; i++) {
		ba_ori_session_tear_down(pAd, Wcid, i, bPassive);
		ba_rec_session_tear_down(pAd, Wcid, i, bPassive);
	}
}

VOID peer_addba_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Status = MLME_UNSPECIFY_FAIL;
	PFRAME_ADDBA_REQ pAddreqFrame = (PFRAME_ADDBA_REQ)(&Elem->Msg[0]);
	MAC_TABLE_ENTRY *pMacEntry;
	MLME_QUEUE_ELEM *addba_resp_elem;
	UINT8 ba_decline = 0;
#ifdef DOT11W_PMF_SUPPORT
	STA_TR_ENTRY *tr_entry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	PFRAME_802_11 pFrame = NULL;
#endif /* DOT11W_PMF_SUPPORT */
	struct ppdu_caps *ppdu = NULL;
	UINT16 rec_ba_wsize = 0;
	UINT16 cfg_rx_ba_wsize;
	BOOLEAN amsdu_supprot = 0;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "%s ==> (Wcid = %d)\n", __func__, Elem->Wcid);

	/* sanity check */
	RETURN_IF_PAD_NULL(pAd);

	if (!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
		return;

	pMacEntry = &pAd->MacTab.Content[Elem->Wcid];

	if (pMacEntry) {
		if (IS_ENTRY_CLIENT(pMacEntry)) {
			if (pMacEntry->Sst != SST_ASSOC) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "peer entry is not in association state\n");
				return;
			}

#ifdef DOT11W_PMF_SUPPORT
			tr_entry = &tr_ctl->tr_entry[Elem->Wcid];

			if ((pMacEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) &&
				(tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
					"%s: PMF Connection IGNORE THIS PKT DUE TO NOT IN PORTSECURED\n", __func__);
				return;
			}

			pFrame = (PFRAME_802_11)Elem->Msg;

			if ((pMacEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) &&
				(pFrame->Hdr.FC.Wep == 0)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
					"%s: PMF CONNECTION BUT RECV WEP=0 ACTION, DROP FRAME\n", __func__);
				return;
			}

#endif /* DOT11W_PMF_SUPPORT */
		}
	} else
		return;

	if (!PeerAddBAReqActionSanity(pAd, Elem->Msg, Elem->MsgLen)) {
		Status = MLME_UNSPECIFY_FAIL;
		goto send_response;
	}

	ba_decline = wlan_config_get_ba_decline(pMacEntry->wdev);

#ifdef SW_CONNECT_SUPPORT
	/* S/W no AGG support */
	if (pMacEntry->bSw == TRUE)
		ba_decline = 1;
#endif /* SW_CONNECT_SUPPORT */

	if (ba_decline || !IS_HT_STA(pMacEntry)) {
		Status = MLME_REQUEST_DECLINED;
		goto send_response;
	}

	/* parameter decision */
	if (wlan_config_get_ba_rx_wsize(pMacEntry->wdev))
		cfg_rx_ba_wsize = wlan_config_get_ba_rx_wsize(pMacEntry->wdev);
	else
		cfg_rx_ba_wsize = BA_WIN_SZ_64;
	rec_ba_wsize = cal_rec_ba_wsize(pMacEntry, cfg_rx_ba_wsize, pAddreqFrame->BaParm.BufSize);
	ppdu = wlan_config_get_ppdu_caps(pMacEntry->wdev);
	amsdu_supprot = (ppdu->rx_amsdu_in_ampdu_support) ? pAddreqFrame->BaParm.AMSDUSupported : 0;

	/* resource management */
	if (ba_resrc_rec_add(pAd, pMacEntry->wcid, pAddreqFrame->BaParm.TID, pAddreqFrame->TimeOutValue, rec_ba_wsize))
		Status = MLME_SUCCESS;
	else
		Status = MLME_REQUEST_WITH_INVALID_PARAM;

send_response:
	/* send add ba response */
	os_alloc_mem(NULL, (UCHAR **)&addba_resp_elem, sizeof(MLME_QUEUE_ELEM));

	if (addba_resp_elem != NULL) {
		MLME_ADDBA_RESP_STRUCT	mlme_addba_resp;

		NdisZeroMemory(&mlme_addba_resp, sizeof(mlme_addba_resp));
		mlme_addba_resp.wcid = Elem->Wcid;
		COPY_MAC_ADDR(mlme_addba_resp.addr, pAddreqFrame->Hdr.Addr2);
		mlme_addba_resp.status = Status;
		mlme_addba_resp.token = pAddreqFrame->Token;
		mlme_addba_resp.amsdu_support = amsdu_supprot;
		mlme_addba_resp.tid = pAddreqFrame->BaParm.TID;
		mlme_addba_resp.buf_size = rec_ba_wsize;
		mlme_addba_resp.timeout = 0;

		NdisZeroMemory(addba_resp_elem, sizeof(MLME_QUEUE_ELEM));
		addba_resp_elem->Wcid = Elem->Wcid;
		addba_resp_elem->wdev = Elem->wdev;
		addba_resp_elem->MsgLen  = sizeof(mlme_addba_resp);
		NdisMoveMemory(addba_resp_elem->Msg, &mlme_addba_resp, sizeof(mlme_addba_resp));

		mlme_send_addba_resp(pAd, addba_resp_elem);
		os_free_mem(addba_resp_elem);
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "%s():alloc memory failed!\n", __func__);
		return;
	}
}

VOID peer_addba_rsp_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_ADDBA_RSP pFrame = NULL;
	MAC_TABLE_ENTRY *pEntry;
	UINT16 ori_ba_wsize = 0;
	UINT16 cfg_tx_ba_wsize = 0;
	UCHAR amsdu_en = 0;
	struct ppdu_caps *ppdu;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "%s ==> Wcid(%d)\n", __func__, Elem->Wcid);

	/* sanity check */
	if (!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
		return;

	pEntry = &pAd->MacTab.Content[Elem->Wcid];
	if (!pEntry || !pEntry->wdev)
		return;

	if (!PeerAddBARspActionSanity(pAd, Elem->Msg, Elem->MsgLen))
		return;

	pFrame = (PFRAME_ADDBA_RSP)(&Elem->Msg[0]);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "\t\t StatusCode = %d\n", pFrame->StatusCode);

	switch (pFrame->StatusCode) {
	case MLME_SUCCESS:
		/* parameter decision */
		cfg_tx_ba_wsize = wlan_config_get_ba_tx_wsize(pEntry->wdev);
		ori_ba_wsize = cal_ori_ba_wsize(pEntry, cfg_tx_ba_wsize, pFrame->BaParm.BufSize);
		ppdu = (struct ppdu_caps *)wlan_config_get_ppdu_caps(pEntry->wdev);
		amsdu_en = wlan_config_get_amsdu_en(pEntry->wdev) && ppdu->tx_amsdu_support && pFrame->BaParm.AMSDUSupported;
		/* resource management */
		if (!ba_resrc_ori_add(pAd, Elem->Wcid, pFrame->BaParm.TID, ori_ba_wsize, amsdu_en, pFrame->TimeOutValue)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "%s(): add ori resrc fail\n", __func__);
			return;
		}
		/* send BAR after BA session is build up */
#ifdef VLAN_SUPPORT
		if (pAd->tr_ctl.vlan2ethctrl == FALSE)
#endif /* VLAN_SUPPORT */
			SendRefreshBAR(pAd, pEntry);

		if (pEntry->BAAutoTest)
			MTWF_PRINT(
				"<addba_info>: sta_mac="MACSTR", amsdu=%d, bapolice=%d, tid=%d, buffersize=%d\n",
				MAC2STR(pEntry->Addr), amsdu_en, pFrame->BaParm.BAPolicy,
				pFrame->BaParm.TID, ori_ba_wsize);
		break;
	case MLME_REQUEST_DECLINED:
		pAd->MacTab.Content[Elem->Wcid].BADeclineBitmap |= 1 << pFrame->BaParm.TID;
		/* fall through */
		/* don't break, need to delete the session, too */
	default:
		/* delete the ori ba session passively */
		ba_ori_session_tear_down(pAd, Elem->Wcid, pFrame->BaParm.TID, TRUE);
		break;
	}
}

VOID peer_delba_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PFRAME_DELBA_REQ    pDelFrame = NULL;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "%s ==>\n", __func__);

	/* sanity check */
	if (!PeerDelBAActionSanity(pAd, Elem->Wcid, Elem->Msg, Elem->MsgLen))
		return;

	pDelFrame = (PFRAME_DELBA_REQ)(&Elem->Msg[0]);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "Initiator = %d, Reason = %d\n", pDelFrame->DelbaParm.Initiator, pDelFrame->ReasonCode);

	if (pDelFrame->DelbaParm.Initiator == ORIGINATOR) {
		ba_rec_session_tear_down(pAd, Elem->Wcid, pDelFrame->DelbaParm.TID, TRUE);
	} else {
		ba_ori_session_tear_down(pAd, Elem->Wcid, pDelFrame->DelbaParm.TID, TRUE);
	}

}

static BOOLEAN amsdu_sanity(RTMP_ADAPTER *pAd, UINT16 CurSN, UINT8 cur_amsdu_state, PBA_REC_ENTRY pBAEntry, ULONG Now32)
{
	BOOLEAN PreviosAmsduMiss = FALSE;
	USHORT  LastIndSeq;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;

	if (CurSN != pBAEntry->PreviousSN) {
		if ((pBAEntry->PreviousAmsduState == FIRST_AMSDU_FORMAT) ||
				(pBAEntry->PreviousAmsduState == MIDDLE_AMSDU_FORMAT)) {
			PreviosAmsduMiss = TRUE;
		}
	} else {
		if (((pBAEntry->PreviousAmsduState == FIRST_AMSDU_FORMAT) ||
			(pBAEntry->PreviousAmsduState == MIDDLE_AMSDU_FORMAT)) &&
				(cur_amsdu_state == FIRST_AMSDU_FORMAT)) {
			PreviosAmsduMiss = TRUE;
		}


	}

	if (PreviosAmsduMiss) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_DEBUG, "PreviosAmsduMiss or only one MSDU in AMPDU");
		pBAEntry->CurMpdu = NULL;

		switch (pBAEntry->PreviousReorderCase) {
		case STEP_ONE:
			pBAEntry->LastIndSeq = pBAEntry->PreviousSN;
			LastIndSeq = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, pBAEntry->LastIndSeq);

			if (LastIndSeq != INVALID_RCV_SEQ)
				pBAEntry->LastIndSeq = LastIndSeq;

			pBAEntry->LastIndSeqAtTimer = Now32;
			break;

		case REPEAT:
		case OLDPKT:
		case WITHIN:
		case SURPASS:
			break;
		}
	}

	return PreviosAmsduMiss;
}

static inline VOID ba_inc_dbg_idx(UINT32 *idx, UINT16 ba_dbg_size)
{
	*idx = ((*idx + 1) % ba_dbg_size);
}

BOOLEAN bar_process(RTMP_ADAPTER *pAd, UINT16 Wcid, ULONG MsgLen, PFRAME_BA_REQ pMsg)
{
	PFRAME_BA_REQ pFrame = pMsg;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	PBA_REC_ENTRY pBAEntry;
	ULONG Idx;
	UCHAR TID;
	ULONG Now32;
	TID = (UCHAR)pFrame->BARControl.TID;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "%s(): BAR-Wcid(%d), Tid (%d)\n", __func__, Wcid, TID);

	/*hex_dump("BAR", (PCHAR) pFrame, MsgLen);*/
	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "frame too large, size = %ld\n", MsgLen);
		return FALSE;
	} else if (MsgLen != sizeof(FRAME_BA_REQ)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "BlockAck Request frame length size = %ld incorrect\n", MsgLen);
		return FALSE;
	} else if (MsgLen != sizeof(FRAME_BA_REQ)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "BlockAck Request frame length size = %ld incorrect\n", MsgLen);
		return FALSE;
	}

	if ((VALID_UCAST_ENTRY_WCID(pAd, Wcid)) && (TID < 8)) {
		/* if this receiving packet is from SA that is in our OriEntry. Since WCID <9 has direct mapping. no need search.*/
		Idx = pAd->MacTab.Content[Wcid].BARecWcidArray[TID];
		pBAEntry = &ba_ctl->BARecEntry[Idx];
	} else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "BAR(%d) : Tid (%d) - %04x:%04x\n",
		Wcid, TID, pFrame->BAStartingSeq.field.StartSeq, pBAEntry->LastIndSeq);

	NdisGetSystemUpTime(&Now32);
	amsdu_sanity(pAd, pFrame->BAStartingSeq.field.StartSeq, FIRST_AMSDU_FORMAT, pBAEntry, Now32);
	tr_cnt->bar_cnt++;

	if ((ba_ctl->dbg_flag & SN_HISTORY) && (pBAEntry->ba_rec_dbg)) {
		if (ba_ctl->dbg_flag & SN_RECORD_BASIC) {
			struct ba_rec_debug *dbg = &pBAEntry->ba_rec_dbg[pBAEntry->ba_rec_dbg_idx];
			dbg->wcid = Wcid;
			dbg->sn = pFrame->BAStartingSeq.field.StartSeq;
			dbg->amsdu = 0;
			dbg->type = BA_BAR;
			dbg->last_in_seq = pBAEntry->LastIndSeq;
			if (ba_ctl->dbg_flag & SN_RECORD_MAC) {
				memcpy(dbg->ta, pMsg->Addr2, MAC_ADDR_LEN);
				memcpy(dbg->ra, pMsg->Addr1, MAC_ADDR_LEN);
			}
			ba_inc_dbg_idx(&pBAEntry->ba_rec_dbg_idx, BA_REC_DBG_SIZE);
		}
	}

	if (ba_ctl->dbg_flag & SN_DUMP_BAR)
		ba_resource_dump_all(pAd, SN_HISTORY);

	if (SEQ_SMALLER(pBAEntry->LastIndSeq, pFrame->BAStartingSeq.field.StartSeq, MAXSEQ)) {
		LONG TmpSeq, seq;
		/*MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "BAR Seq = %x, LastIndSeq = %x\n", pFrame->BAStartingSeq.field.StartSeq, pBAEntry->LastIndSeq);*/

		tr_cnt->bar_large_win_start++;
		seq = (pFrame->BAStartingSeq.field.StartSeq == 0) ? MAXSEQ : (pFrame->BAStartingSeq.field.StartSeq - 1);
		ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, pBAEntry, seq);
		pBAEntry->LastIndSeq = seq;
		pBAEntry->LastIndSeqAtTimer = Now32;

		TmpSeq = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, pBAEntry->LastIndSeq);

		if (TmpSeq != INVALID_RCV_SEQ) {
			pBAEntry->LastIndSeq = TmpSeq;
		}
	}

	return TRUE;
}

void convert_reordering_packet_to_preAMSDU_or_802_3_packet(
	RTMP_ADAPTER *pAd,
	RX_BLK *pRxBlk,
	UCHAR wdev_idx)
{
	PNDIS_PACKET pRxPkt;
	UCHAR Header802_3[LENGTH_802_3];
	struct wifi_dev *wdev;

	ASSERT(wdev_idx < WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "%s(): invalid wdev_idx(%d)\n", __func__, wdev_idx);
		return;
	}

	wdev = pAd->wdev_list[wdev_idx];
	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
	ASSERT(pRxBlk->pRxPacket);

	if (pRxBlk->pRxPacket == NULL)
		return;

	pRxPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);
	RTMP_OS_PKT_INIT(pRxBlk->pRxPacket,
					 get_netdev_from_bssid(pAd, wdev_idx),
					 pRxBlk->pData, pRxBlk->DataSize);

	/* copy 802.3 header, if necessary */
	if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU)) {
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;
		USHORT VLAN_VID = 0, VLAN_Priority = 0;
		/* TODO: shiang-usw, fix me!! */
#ifdef CONFIG_AP_SUPPORT

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA) || RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS)) {
			/* Check if need to insert VLAN tag to the received packet */
			WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);

			if (VLAN_VID)
				VLAN_Size = LENGTH_802_1Q;
		}

#endif /* CONFIG_AP_SUPPORT */
		{
			data_p = OS_PKT_HEAD_BUF_EXTEND(pRxPkt, LENGTH_802_3 + VLAN_Size);
			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									 Header802_3, LENGTH_802_3,
									 data_p, TPID);
		}
	}
}

static VOID ba_enqueue_reordering_packet(
	RTMP_ADAPTER *pAd,
	BA_REC_ENTRY *pBAEntry,
	RX_BLK *pRxBlk,
	UCHAR wdev_idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = &tr_ctl->ba_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	struct reordering_mpdu *msdu_blk;
	UINT16 Sequence = pRxBlk->SN;

	msdu_blk = ba_mpdu_blk_alloc(pAd, pRxBlk);

	if ((msdu_blk != NULL) &&
		(!RX_BLK_TEST_FLAG(pRxBlk, fRX_EAP))) {
		/* Write RxD buffer address & allocated buffer length */
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		msdu_blk->Sequence = Sequence;
		msdu_blk->OpMode = pRxBlk->OpMode;
		msdu_blk->bAMSDU = RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU);

		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			convert_reordering_packet_to_preAMSDU_or_802_3_packet(pAd, pRxBlk, wdev_idx);
		else {
			struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);

			pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);

			SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
		}

		/* it is necessary for reordering packet to record
			which BSS it come from
		*/
		RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);
		STATS_INC_RX_PACKETS(pAd, wdev_idx);
		msdu_blk->pPacket = pRxBlk->pRxPacket;

		if (!pBAEntry->CurMpdu) {
			if (ba_reordering_mpdu_insertsorted(&pBAEntry->list, msdu_blk) == FALSE) {
				tr_cnt->ba_err_dup2++;
				pBAEntry->drop_dup_pkts++;
#ifdef IXIA_C50_MODE
				if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(pRxBlk->pRxPacket)))
					pAd->rx_cnt.rx_dup_drop[pRxBlk->wcid]++;
#endif
				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_SUCCESS);
				ba_mpdu_blk_free(ba_ctl, msdu_blk);
				pBAEntry->CurMpdu = NULL;
			} else
				pBAEntry->CurMpdu = msdu_blk;
		} else
			ba_enqueue_tail(&pBAEntry->CurMpdu->AmsduList, msdu_blk);

		if ((pBAEntry->list.qlen < 0) || (pBAEntry->list.qlen > pBAEntry->BAWinSize)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "(qlen:%d, BAWinSize:%d)\n",
				pBAEntry->list.qlen, pBAEntry->BAWinSize);
			dump_ba_list(&pBAEntry->list);
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	} else {
		ULONG Now32;

		if (msdu_blk)
			ba_mpdu_blk_free(ba_ctl, msdu_blk);
		else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "!!! (used:%d/free:%d) Can't allocate reordering mpdu blk\n",
				pBAEntry->list.qlen, ba_ctl->mpdu_blk_pool[pRxBlk->band].freelist.qlen);
		}

		ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, pBAEntry, Sequence);
		pBAEntry->LastIndSeq = Sequence;

		indicate_rx_pkt(pAd, pRxBlk, wdev_idx);

		Sequence = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, pBAEntry->LastIndSeq);

		if (Sequence != INVALID_RCV_SEQ) {
			pBAEntry->LastIndSeq = Sequence;
		}

		NdisGetSystemUpTime(&Now32);
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		pBAEntry->LastIndSeqAtTimer = Now32;
		pBAEntry->CurMpdu = NULL;
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}
}

VOID ba_reorder(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, UCHAR wdev_idx)
{
	ULONG Now32;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = &tr_ctl->ba_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	UINT16 seq = rx_blk->SN;
	PBA_REC_ENTRY ba_entry = NULL;
	BOOLEAN amsdu_miss = FALSE;

	if (VALID_UCAST_ENTRY_WCID(pAd, rx_blk->wcid)) {
		UINT16 idx;

		idx = pAd->MacTab.Content[rx_blk->wcid].BARecWcidArray[rx_blk->TID];

		if (idx == 0) {
			/* recipient BA session had been torn down */
			indicate_rx_pkt(pAd, rx_blk, wdev_idx);
			tr_cnt->ba_err_tear_down++;
			return;
		}

		ba_entry = &ba_ctl->BARecEntry[idx];
	} else {
		tr_cnt->ba_err_wcid_invalid++;
		RELEASE_NDIS_PACKET(pAd, rx_blk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if ((ba_ctl->dbg_flag & SN_HISTORY) && (ba_entry->ba_rec_dbg)) {
		if (ba_ctl->dbg_flag & SN_RECORD_BASIC) {
			struct ba_rec_debug *dbg = &ba_entry->ba_rec_dbg[ba_entry->ba_rec_dbg_idx];
			dbg->sn = seq;
			dbg->amsdu = rx_blk->AmsduState;
			dbg->type = BA_DATA;
			dbg->last_in_seq = ba_entry->LastIndSeq;
			dbg->wcid = rx_blk->wcid;

			if (ba_ctl->dbg_flag & SN_RECORD_MAC) {
				memcpy(dbg->ta, rx_blk->Addr2, MAC_ADDR_LEN);
				memcpy(dbg->ra, rx_blk->Addr1, MAC_ADDR_LEN);
			}

			ba_inc_dbg_idx(&ba_entry->ba_rec_dbg_idx, BA_REC_DBG_SIZE);
		}
	}

	NdisGetSystemUpTime(&Now32);

	switch (ba_entry->REC_BA_Status) {
	case Recipient_NONE:
	case Recipient_USED:
	case Recipient_HandleRes:
		ba_refresh_reordering_mpdus(pAd, ba_ctl, ba_entry);
		return;

	case Recipient_Initialization:
		ba_refresh_reordering_mpdus(pAd, ba_ctl, ba_entry);
		ASSERT((ba_entry->list.qlen == 0) && (ba_entry->list.next == NULL));
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_DEBUG, "%s:Reset Last Indicate Sequence(%d): amsdu state = %d\n", __func__, rx_blk->SN, rx_blk->AmsduState);
		/*
		 * For the first reordering pkt in the BA session, initialize LastIndSeq to (Sequence - 1)
		 * so that the ba_reorder_check will fall in the in-order SEQ_STEPONE case.
		 */
		ba_entry->LastIndSeq = (seq - 1) & MAXSEQ;
		ba_entry->LastIndSeqAtTimer = Now32;
		ba_entry->PreviousAmsduState = rx_blk->AmsduState;
		ba_entry->PreviousSN = seq;
		ba_entry->REC_BA_Status = Recipient_Established;
	case Recipient_Established:
		break;
	case recipient_offload:
		indicate_rx_pkt(pAd, rx_blk, wdev_idx);
		return;
	default:
		tr_cnt->ba_drop_unknown++;
		ba_entry->drop_unknown_state_pkts++;
		RELEASE_NDIS_PACKET(pAd, rx_blk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	NdisGetSystemUpTime(&Now32);

	if (ba_entry->check_amsdu_miss) {
		amsdu_miss = amsdu_sanity(pAd, seq, rx_blk->AmsduState, ba_entry, Now32);

		if (amsdu_miss)
			tr_cnt->ba_amsdu_miss++;
	}

	ba_entry->check_amsdu_miss = TRUE;

	if ((rx_blk->AmsduState == FINAL_AMSDU_FORMAT)
			|| (rx_blk->AmsduState == MSDU_FORMAT) || amsdu_miss)
			ba_flush_reordering_timeout_mpdus(pAd, ba_ctl, ba_entry, Now32);

ba_reorder_check:
	/* I. Check if in order. */
	if (SEQ_STEPONE(seq, ba_entry->LastIndSeq, MAXSEQ)) {
		USHORT  LastIndSeq;

		if (ba_ctl->dbg_flag & SN_DUMP_STEPONE)
			ba_resource_dump_all(pAd, SN_HISTORY);

		indicate_rx_pkt(pAd, rx_blk, wdev_idx);

		if ((rx_blk->AmsduState == FINAL_AMSDU_FORMAT) || (rx_blk->AmsduState == MSDU_FORMAT)) {
			ba_entry->LastIndSeq = seq;
			LastIndSeq = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, ba_entry, ba_entry->LastIndSeq);

			if (LastIndSeq != INVALID_RCV_SEQ)
				ba_entry->LastIndSeq = LastIndSeq;

			ba_entry->LastIndSeqAtTimer = Now32;
		}

		ba_entry->PreviousReorderCase = STEP_ONE;
	}
	/* II. Drop Duplicated Packet*/
	else if (seq == ba_entry->LastIndSeq) {
		if (RX_BLK_TEST_FLAG(rx_blk, fRX_AMSDU) &&
			(tr_ctl->damsdu_type == RX_SW_AMSDU)) {
			ba_entry->LastIndSeqAtTimer = Now32;
			indicate_rx_pkt(pAd, rx_blk, wdev_idx);
		} else {
			if (ba_ctl->dbg_flag & SN_DUMP_DUP)
				ba_resource_dump_all(pAd, SN_HISTORY);
			tr_cnt->ba_err_dup1++;
			ba_entry->drop_dup_pkts++;
#ifdef IXIA_C50_MODE
			if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(rx_blk->pRxPacket))) {
				pAd->rx_cnt.rx_dup_drop[rx_blk->wcid]++;
			}
#endif
			RELEASE_NDIS_PACKET(pAd, rx_blk->pRxPacket, NDIS_STATUS_FAILURE);
		}

		ba_entry->PreviousReorderCase = REPEAT;
	}
	/* III. Drop Old Received Packet*/
	else if (SEQ_SMALLER(seq, ba_entry->LastIndSeq, MAXSEQ)) {
		if (ba_ctl->dbg_flag & SN_DUMP_OLD)
			ba_resource_dump_all(pAd, SN_HISTORY);
		tr_cnt->ba_err_old++;
		ba_entry->drop_old_pkts++;
#ifdef IXIA_C50_MODE
		if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(rx_blk->pRxPacket))) {
			pAd->rx_cnt.rx_old_drop[rx_blk->wcid]++;
		}
#endif
		RELEASE_NDIS_PACKET(pAd, rx_blk->pRxPacket, NDIS_STATUS_FAILURE);
		ba_entry->PreviousReorderCase = OLDPKT;
	}
	/* IV. Receive Sequence within Window Size*/
	else if (SEQ_SMALLER(seq, (((ba_entry->LastIndSeq + ba_entry->BAWinSize + 1)) & MAXSEQ), MAXSEQ)) {
		if (ba_ctl->dbg_flag & SN_DUMP_WITHIN)
			ba_resource_dump_all(pAd, SN_HISTORY);
		ba_enqueue_reordering_packet(pAd, ba_entry, rx_blk, wdev_idx);
		ba_entry->PreviousReorderCase = WITHIN;
	}
	/* V. Receive seq surpasses Win(lastseq + nMSDU). So refresh all reorder buffer*/
	else {
		LONG WinStartSeq, TmpSeq;

		if (ba_ctl->dbg_flag & SN_DUMP_SURPASS)
			ba_resource_dump_all(pAd, SN_HISTORY);

		tr_cnt->ba_sn_large_win_end++;
#ifdef IXIA_C50_MODE
		pAd->rx_cnt.rx_surpass_drop[rx_blk->wcid]++;
#endif
		ba_entry->ba_sn_large_win_end++;

		TmpSeq = seq - (ba_entry->BAWinSize) + 1;

		if (TmpSeq < 0)
			TmpSeq = (MAXSEQ + 1) + TmpSeq;

		WinStartSeq = TmpSeq;
		ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, ba_entry, (WinStartSeq - 1) & MAXSEQ);

		ba_entry->LastIndSeq = (WinStartSeq - 1) & MAXSEQ;
		ba_entry->LastIndSeqAtTimer = Now32;

		TmpSeq = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, ba_entry, ba_entry->LastIndSeq);

		if (TmpSeq != INVALID_RCV_SEQ)
			ba_entry->LastIndSeq = TmpSeq;

		ba_entry->PreviousReorderCase = SURPASS;

		goto ba_reorder_check;
	}

	ba_entry->PreviousAmsduState = rx_blk->AmsduState;
	ba_entry->PreviousSN = seq;

	if ((rx_blk->AmsduState == MSDU_FORMAT) || (rx_blk->AmsduState == FINAL_AMSDU_FORMAT))
		ba_entry->CurMpdu = NULL;
}

VOID ba_reorder_buf_maintain(RTMP_ADAPTER *pAd)
{
	ULONG Now32;
	UINT16 wcid;
	UINT16 Idx;
	UCHAR TID;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	PBA_REC_ENTRY pBAEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	/* update last rx time*/
	NdisGetSystemUpTime(&Now32);

	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = &pAd->MacTab.Content[wcid];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		for (TID = 0; TID < NUM_OF_TID; TID++) {
			Idx = pAd->MacTab.Content[wcid].BARecWcidArray[TID];
			pBAEntry = &ba_ctl->BARecEntry[Idx];
			ba_flush_reordering_timeout_mpdus(pAd, ba_ctl, pBAEntry, Now32);
		}
	}
}

VOID ba_refresh_bar_all(RTMP_ADAPTER *pAd)
{
	UINT16 wcid;
	PMAC_TABLE_ENTRY pEntry = NULL;

	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = &pAd->MacTab.Content[wcid];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		/* send BAR */
		SendRefreshBAR(pAd, pEntry);
	}
}

VOID ba_ori_session_start(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry, UINT8 UPriority)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[tr_entry->wcid];
	UINT8 ba_en = wlan_config_get_ba_enable(pEntry->wdev);

#ifdef SW_CONNECT_SUPPORT
	/* S/W no AGG support */
	if (pEntry->bSw == TRUE)
		return;
#endif /* SW_CONNECT_SUPPORT */

	/* BA has already been setup */
	if ((pEntry->TXBAbitmap & (1 << UPriority)) != 0)
		return;

	if (!ba_en) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "HT_AutoBA = 0, disable BA\n");
		return;
	}
	/* TODO: shiang-usw, fix me for pEntry, we should replace this paramter as tr_entry! */
	if ((tr_entry && tr_entry->EntryType != ENTRY_CAT_MCAST && VALID_UCAST_ENTRY_WCID(pAd, tr_entry->wcid)) &&
		(IS_HT_STA(pEntry) || IS_HE_6G_STA(pEntry->cap.modes))) {
		BOOLEAN isRalink = CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);

		if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			&& (!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
			&& ((isRalink || IS_ENTRY_MESH(pEntry) || IS_ENTRY_WDS(pEntry))
				|| (IS_NO_SECURITY(&pEntry->SecConfig)
					|| IS_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher)
					|| IS_CIPHER_CCMP256(pEntry->SecConfig.PairwiseCipher)
					|| IS_CIPHER_GCMP128(pEntry->SecConfig.PairwiseCipher)
					|| IS_CIPHER_GCMP256(pEntry->SecConfig.PairwiseCipher)
				   ))
		   )
			ba_ori_session_setup(pAd, pEntry->wcid, UPriority, 0);
	}
}

VOID ba_ctl_init(RTMP_ADAPTER *pAd, struct ba_control *ba_ctl)
{
	int i;

	ba_ctl->numAsOriginator = 0;
	ba_ctl->numAsRecipient = 0;
	ba_ctl->numDoneOriginator = 0;
	NdisAllocateSpinLock(pAd, &ba_ctl->BATabLock);
	ba_ctl->ba_timeout_check = FALSE;
	ba_ctl->dbg_flag |= SN_HISTORY;
	os_zero_mem((UCHAR *)&ba_ctl->ba_timeout_bitmap[0], sizeof(UINT32) * BA_TIMEOUT_BITMAP_LEN);
#ifdef RX_RPS_SUPPORT
	os_zero_mem((UCHAR *)&ba_ctl->ba_timeout_bitmap_per_cpu[0][0], sizeof(UINT32) * BA_TIMEOUT_BITMAP_LEN * NR_CPUS);
#endif
	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		ba_ctl->BARecEntry[i].REC_BA_Status = Recipient_NONE;
		NdisAllocateSpinLock(pAd, &(ba_ctl->BARecEntry[i].RxReRingLock));
	}

	for (i = 0; i < MAX_LEN_OF_BA_ORI_TABLE; i++)
		ba_ctl->BAOriEntry[i].ORI_BA_Status = Originator_NONE;
}

VOID ba_ctl_exit(struct ba_control *ba_ctl)
{
	int i;

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++)
		NdisFreeSpinLock(&ba_ctl->BARecEntry[i].RxReRingLock);

	NdisFreeSpinLock(&ba_ctl->BATabLock);
}
