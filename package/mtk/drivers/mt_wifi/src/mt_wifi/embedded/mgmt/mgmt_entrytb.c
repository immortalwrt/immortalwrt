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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#include <rt_config.h>
#ifdef VOW_SUPPORT
#include <ap_vow.h>
#endif /* VOW_SUPPORT */

static BOOLEAN aid_is_assigned(struct _aid_info *aid_info, UINT16 aid)
{
	/*this line shall not happen, allocation fail case is handled at the allocating stage.*/
	if (aid_info->aid_bitmap == NULL)
		return TRUE;

	if (aid_info->aid_bitmap[aid / 32] & (1 << (aid % 32)))
		return TRUE;

	return FALSE;
}

static void aid_set(struct _aid_info *aid_info, UINT16 aid)
{
	/*it shall not happen. check it before the function is called.*/
	if (aid_info->aid_bitmap == NULL)
		return;

	aid_info->aid_bitmap[aid / 32] |= (1 << (aid % 32));
}

#ifndef WTBL_TDD_SUPPORT
static
#endif /* WTBL_TDD_SUPPORT */
void aid_clear(struct _aid_info *aid_info, UINT16 aid)
{
	/*it shall not happen. check it before the function is called.*/
	if (aid_info->aid_bitmap == NULL)
		return;

	aid_info->aid_bitmap[aid / 32] &= ~(1 << (aid % 32));
}

static void aid_dump(struct _aid_info *aid_info)
{
	UINT16 i;

	/* it shall not happen, check it for sanity. */
	if (aid_info->aid_bitmap == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, CATMLME_WTBL, DBG_LVL_ERROR,
			"no aid_bitmap\n");
		return;
	}

	for (i = 0; i <= aid_info->max_aid; i++) {
		if (aid_info->aid_bitmap[i/32] > 0) {
			if (i % 32 == 0) {
				MTWF_DBG(NULL, DBG_CAT_ALL, CATMLME_WTBL, DBG_LVL_DEBUG,
					 "BIT:%d - BIT:%d:\t\t",
						i,
						(i+31) > INVALID_AID ? INVALID_AID-1 : (i+31));
			}
			MTWF_DBG(NULL, DBG_CAT_ALL, CATMLME_WTBL, DBG_LVL_DEBUG,
				 "%d", (aid_info->aid_bitmap[i/32] & (1 << (i%32))) >> (i%32));
			if (i % 32 == 31)
				MTWF_DBG(NULL, DBG_CAT_ALL, CATMLME_WTBL, DBG_LVL_DEBUG, "\n");
		} else
			i += 32;
	}
	MTWF_DBG(NULL, DBG_CAT_ALL, CATMLME_WTBL, DBG_LVL_DEBUG, "\n");
}

VOID TRTableEntryDump(RTMP_ADAPTER *pAd, INT tr_idx, const RTMP_STRING *caller, INT line)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
	INT qidx;
	struct qm_ops *ops = pAd->qm_ops;

	ASSERT(IS_TR_WCID_VALID(pAd, tr_idx));

	if (!IS_TR_WCID_VALID(pAd, tr_idx))
		return;

	MTWF_PRINT("Dump TR_ENTRY called by function %s(%d)\n", caller, line);
	tr_entry = &tr_ctl->tr_entry[tr_idx];
	MTWF_PRINT("TR_ENTRY[%d]\n", tr_idx);
#ifdef SW_CONNECT_SUPPORT
	MTWF_PRINT("\tbSw=%d\n", tr_entry->bSw);
	MTWF_PRINT("\thw_wcid=%u\n", tr_entry->hw_wcid);
#endif /* SW_CONNECT_SUPPORT */
	MTWF_PRINT("\tEntryType=%x\n", tr_entry->EntryType);
	MTWF_PRINT("\twdev=%p\n", tr_entry->wdev);
	MTWF_PRINT("\twcid=%d\n", tr_entry->wcid);
	MTWF_PRINT("\tfunc_tb_idx=%d\n", tr_entry->func_tb_idx);
	MTWF_PRINT("\tAddr="MACSTR"\n", MAC2STR(tr_entry->Addr));
	MTWF_PRINT("\tBSSID="MACSTR"\n", MAC2STR(tr_entry->bssid));
	MTWF_PRINT("\tFlags\n");
	MTWF_PRINT("\t\tbIAmBadAtheros=%d, isCached=%d, PortSecured=%d, PsMode=%d, LockEntryTx=%d\n",
			 tr_entry->bIAmBadAtheros, tr_entry->isCached, tr_entry->PortSecured, tr_entry->PsMode, tr_entry->LockEntryTx);
	MTWF_PRINT("\tTxRx Characters\n");
#ifdef SW_CONNECT_SUPPORT
	/* Tx debug Cnts */
	MTWF_PRINT("\t\ttx_fp_allow_cnt=%u\n", tr_entry->tx_fp_allow_cnt);
	MTWF_PRINT("\t\ttx_send_data_cnt=%u\n", tr_entry->tx_send_data_cnt);
	MTWF_PRINT("\t\ttx_deq_eap_cnt=%u\n", tr_entry->tx_deq_eap_cnt);
	MTWF_PRINT("\t\ttx_deq_arp_cnt=%u\n", tr_entry->tx_deq_arp_cnt);
	MTWF_PRINT("\t\ttx_deq_data_cnt=%u\n", tr_entry->tx_deq_data_cnt);
	MTWF_PRINT("\t\ttx_handle_cnt=%u\n", tr_entry->tx_handle_cnt);
	/* Rx debug Cnts */
	MTWF_PRINT("\t\trx_handle_cnt=%u\n", tr_entry->rx_handle_cnt);
	MTWF_PRINT("\t\trx_u2m_cnt=%u\n", tr_entry->rx_u2m_cnt);
	MTWF_PRINT("\t\trx_eap_cnt=%u\n", tr_entry->rx_eap_cnt);
#endif /* SW_CONNECT_SUPPORT */
	MTWF_PRINT("\t\tNonQosDataSeq=%d\n", tr_entry->NonQosDataSeq);
	MTWF_PRINT("\t\tTxSeq[0]=%d, TxSeq[1]=%d, TxSeq[2]=%d, TxSeq[3]=%d\n",
			 tr_entry->TxSeq[0], tr_entry->TxSeq[1], tr_entry->TxSeq[2], tr_entry->TxSeq[3]);
#ifdef SW_CONNECT_SUPPORT
	MTWF_PRINT("\t\tRxSeq[0]=%d, RxSeq[1]=%d, RxSeq[2]=%d, RxSeq[3]=%d\n",
			 tr_entry->RxSeq[0], tr_entry->RxSeq[1], tr_entry->RxSeq[2], tr_entry->RxSeq[3]);
#endif /* SW_CONNECT_SUPPORT */
	MTWF_PRINT("\tCurrTxRate=%x\n", tr_entry->CurrTxRate);
	MTWF_PRINT("\tQueuing Info\n");
	MTWF_PRINT("\t\tenq_cap=%d, deq_cap=%d\n", tr_entry->enq_cap, tr_entry->deq_cap);
	MTWF_PRINT("\t\tQueuedPkt: TxQ[0]=%d, TxQ[1]=%d, TxQ[2]=%d, TxQ[3]=%d, PSQ=%d\n",
			 tr_entry->tx_queue[0].Number, tr_entry->tx_queue[1].Number,
			 tr_entry->tx_queue[2].Number, tr_entry->tx_queue[3].Number,
			 tr_entry->ps_queue.Number);
	MTWF_PRINT("\t\tdeq_cnt=%d, deq_bytes=%d\n", tr_entry->deq_cnt, tr_entry->deq_bytes);

	for (qidx = 0; qidx < 4; qidx++) {
		if (ops->sta_dump_queue)
			ops->sta_dump_queue(pAd, tr_entry->wcid, TX_DATA, qidx);

		ge_tx_swq_dump(pAd, qidx);
	}
}

VOID TRTableResetEntry(RTMP_ADAPTER *pAd, UINT16 tr_tb_idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx;
	struct qm_ops *ops = pAd->qm_ops;

	if (!IS_TR_WCID_VALID(pAd, tr_tb_idx))
		return;

	tr_entry = &tr_ctl->tr_entry[tr_tb_idx];

	if (IS_ENTRY_NONE(tr_entry))
		return;

	tr_entry->enq_cap = FALSE;
	tr_entry->deq_cap = FALSE;

	if (ops->sta_clean_queue)
		ops->sta_clean_queue(pAd, tr_entry->wcid);

	SET_ENTRY_NONE(tr_entry);

#ifdef SW_CONNECT_SUPPORT
	tr_entry->bSw = FALSE;
	tr_entry->hw_wcid = WCID_INVALID;

	/* Reset Tx Counters */
	tr_entry->tx_fp_allow_cnt = 0;
	tr_entry->tx_send_data_cnt = 0;
	tr_entry->tx_deq_eap_cnt = 0;
	tr_entry->tx_deq_arp_cnt = 0;
	tr_entry->tx_deq_data_cnt = 0;
	tr_entry->tx_handle_cnt = 0;

	/* Reset Rx Counters */
	tr_entry->rx_handle_cnt = 0;
	tr_entry->rx_u2m_cnt = 0;
	tr_entry->rx_eap_cnt = 0;
#endif /* SW_CONNECT_SUPPORT */

	for (qidx = 0; qidx < WMM_QUE_NUM; qidx++)
		NdisFreeSpinLock(&tr_entry->txq_lock[qidx]);

	NdisFreeSpinLock(&tr_entry->ps_queue_lock);
	NdisFreeSpinLock(&tr_entry->ps_sync_lock);
	return;
}


VOID TRTableInsertEntry(RTMP_ADAPTER *pAd, UINT16 tr_tb_idx, MAC_TABLE_ENTRY *pEntry)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx, tid, upId;
#ifdef FQ_SCH_SUPPORT
	struct fq_stainfo_type *pfq_sta = NULL;
#endif
	if (IS_TR_WCID_VALID(pAd, tr_tb_idx)) {
		tr_entry = &tr_ctl->tr_entry[tr_tb_idx];
		pEntry->tr_tb_idx = tr_tb_idx;
		tr_entry->EntryType = pEntry->EntryType;
		tr_entry->wdev = pEntry->wdev;
		tr_entry->func_tb_idx = pEntry->func_tb_idx;
		tr_entry->wcid = pEntry->wcid;
#ifdef SW_CONNECT_SUPPORT
		tr_entry->bSw = pEntry->bSw;
		tr_entry->hw_wcid = pEntry->hw_wcid;
#endif /* SW_CONNECT_SUPPORT */
		NdisMoveMemory(tr_entry->Addr, pEntry->Addr, MAC_ADDR_LEN);
		tr_entry->NonQosDataSeq = 0;

		for (tid = 0; tid < NUM_OF_TID; tid++)
			tr_entry->TxSeq[tid] = 0;

		for (upId = 0; upId < NUM_OF_UP; upId++) {
			tr_entry->cacheSn[upId] = -1;
			tr_entry->previous_sn[upId] = -1;
			tr_entry->previous_amsdu_state[upId] = MSDU_FORMAT;
		}

#ifdef MT_MAC
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			if (tr_entry->wdev)
				tr_entry->OmacIdx = pEntry->wdev->OmacIdx;
			else
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "wdev == NULL\n");
		}
#endif /* MT_MAC */
		tr_entry->PsMode = PWR_ACTIVE;
		tr_entry->isCached = FALSE;
		tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		tr_entry->CurrTxRate = pEntry->CurrTxRate;

		for (qidx = 0; qidx < WMM_QUE_NUM; qidx++) {
			InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
			tr_entry->TotalPageCount[qidx] = 0;
#endif
#endif
			NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
		}

		InitializeQueueHeader(&tr_entry->ps_queue);
		NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);
		NdisAllocateSpinLock(pAd, &tr_entry->ps_sync_lock);
		tr_entry->deq_cnt = 0;
		tr_entry->deq_bytes = 0;
		tr_entry->PsQIdleCount = 0;
		tr_entry->enq_cap = TRUE;
		tr_entry->deq_cap = TRUE;
		tr_entry->NoDataIdleCount = 0;
		tr_entry->ContinueTxFailCnt = 0;
		tr_entry->LockEntryTx = FALSE;
		tr_entry->TimeStamp_toTxRing = 0;
		tr_entry->PsDeQWaitCnt = 0;
		NdisMoveMemory(tr_entry->bssid, pEntry->wdev->bssid, MAC_ADDR_LEN);
#ifdef FQ_SCH_SUPPORT
		if (pAd->fq_ctrl.enable & FQ_READY) {
			pfq_sta = &tr_entry->fq_sta_rec;
			for (qidx = 0; qidx < WMM_QUE_NUM; qidx++)
				pfq_sta->status[qidx] = FQ_UN_CLEAN_STA;
			fq_clean_list(pAd, WMM_NUM_OF_AC);
		}
#endif
	}
}


VOID TRTableInsertMcastEntry(RTMP_ADAPTER *pAd, UINT16 tr_tb_idx, struct wifi_dev *wdev)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct _STA_TR_ENTRY *tr_entry = NULL;
	INT qidx, tid;
	UINT8 band = DBDC_BAND0;

	if (!IS_TR_WCID_VALID(pAd, tr_tb_idx))
		return;

	tr_entry = &tr_ctl->tr_entry[tr_tb_idx];

	if (!tr_entry)
		return;

	tr_entry->EntryType = ENTRY_CAT_MCAST;
	tr_entry->wdev = wdev;
	tr_entry->func_tb_idx = wdev->func_idx;
	tr_entry->PsMode = PWR_ACTIVE;
	tr_entry->isCached = FALSE;
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	tr_entry->CurrTxRate = pAd->CommonCfg.MlmeRate;
	NdisMoveMemory(tr_entry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);
	/* TODO: shiang-usw, for following fields, need better assignment! */
	tr_entry->wcid = tr_tb_idx;
#ifdef SW_CONNECT_SUPPORT
	tr_entry->bSw = FALSE;
	tr_entry->hw_wcid = wdev->hw_bmc_wcid;
#endif /* SW_CONNECT_SUPPORT */
	tr_entry->NonQosDataSeq = 0;

	for (tid = 0; tid < NUM_OF_TID; tid++)
		tr_entry->TxSeq[tid] = 0;

	for (qidx = 0; qidx < WMM_QUE_NUM; qidx++) {
		InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
		NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
	}

	InitializeQueueHeader(&tr_entry->ps_queue);
	NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);
	NdisAllocateSpinLock(pAd, &tr_entry->ps_sync_lock);
	tr_entry->deq_cnt = 0;
	tr_entry->deq_bytes = 0;
	tr_entry->PsQIdleCount = 0;
	tr_entry->enq_cap = TRUE;
	tr_entry->deq_cap = TRUE;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (tr_entry->wdev)
			tr_entry->OmacIdx = wdev->OmacIdx;
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "wdev == NULL\n");
	}

#endif /* MT_MAC */
	/*
	    Carter check,
	    if Mcast pkt will reference the bssid field for do something?
	    if so, need to check flow.
	*/
	NdisMoveMemory(tr_entry->bssid, wdev->bssid, MAC_ADDR_LEN);

	band = HcGetBandByWdev(wdev);
	if (!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG)) {
#ifdef VOW_SUPPORT
		if (!pAd->vow_cfg.en_bw_ctrl)
#endif
			pAd->bss_group.group_idx[wdev->func_idx] = wdev->func_idx % pAd->max_bssgroup_num;

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN, "band%u group_idx[%d]=%d\n",
			band, wdev->func_idx, pAd->bss_group.group_idx[wdev->func_idx]);
	}
}

VOID MgmtTableSetMcastEntry(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (wcid >= MAX_LEN_OF_MAC_TABLE)
		return;

	pEntry = &pAd->MacTab.Content[wcid];
	pEntry->EntryType = ENTRY_CAT_MCAST;
	pEntry->Sst = SST_ASSOC;
	pEntry->Aid = wcid;/*FIXME: why BCMC entry needs AID?*/
	pEntry->wcid = wcid;
	pEntry->PsMode = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate;
	pEntry->Addr[0] = 0x01;/* FIXME: is the code segment necessary?
				* the mac address changes to BROADCAST addr in the below code segment right away.
				*/
	pEntry->HTPhyMode.field.MODE = MODE_OFDM;
	pEntry->HTPhyMode.field.MCS = 3;
	NdisMoveMemory(pEntry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pEntry->wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pEntry->wdev = &pAd->StaCfg[MAIN_MSTA_ID].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
}


VOID MacTableSetEntryPhyCfg(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE) {
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->HTPhyMode.field.MODE = MODE_CCK;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
	} else {
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->HTPhyMode.field.MODE = MODE_OFDM;
		pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
	}
}


VOID MacTableSetEntryRaCap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent, struct _vendor_ie_cap *vendor_ie)
{
	ULONG ra_ie = vendor_ie->ra_cap;
	ULONG mtk_ie = vendor_ie->mtk_cap;
#ifdef DOT11_VHT_AC
	ULONG brcm_ie = vendor_ie->brcm_cap;
#endif /*DOT11_VHT_AC*/
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
			 "vendor_ie_cap for ra_cap=%08x, mtk_cap=%08x\n", (UINT32)ra_ie, (UINT32)mtk_ie);
	NdisCopyMemory(&ent->vendor_ie, vendor_ie, sizeof(struct _vendor_ie_cap));
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);

	/* TODO: need MTK CAP ? */

	/* Set cap flags */
	if (vendor_ie->is_rlt == TRUE) {
		CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
#ifdef AGGREGATION_SUPPORT

		if (ra_ie & RALINK_AGG_CAP) {
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);

			if (pAd->CommonCfg.bAggregationCapable) {
				CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
				MTWF_LOG(DBG_CAT_MLME, CATMLME_WTBL,
						 DBG_LVL_DEBUG, ("RaAggregate= 1\n"));
			}
		}

#endif /* AGGREGATION_SUPPORT */
#ifdef PIGGYBACK_SUPPORT

		if (ra_ie & RALINK_PIGGY_CAP) {
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);

			if (pAd->CommonCfg.bPiggyBackCapable) {
				CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
				MTWF_LOG(DBG_CAT_MLME, CATMLME_WTBL,
						 DBG_LVL_DEBUG, ("PiggyBack= 1\n"));
			}
		}

#endif /* PIGGYBACK_SUPPORT */

		if (ra_ie & RALINK_RDG_CAP) {
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);

			if (pAd->CommonCfg.bRdg) {
				CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);
				MTWF_LOG(DBG_CAT_MLME, CATMLME_WTBL,
						 DBG_LVL_DEBUG, ("Rdg = 1\n"));
			}
		}

#ifdef DOT11_VHT_AC

		if ((ra_ie & RALINK_256QAM_CAP)
			&& (pAd->CommonCfg.g_band_256_qam)) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
					 "RALINK_256QAM_CAP for 2.4G\n");
			ent->fgGband256QAMSupport = TRUE;
		}

#endif /*DOT11_VHT_AC*/
	}

#ifdef DOT11_VHT_AC
	else if ((mtk_ie & MEDIATEK_256QAM_CAP)
			 && (pAd->CommonCfg.g_band_256_qam)) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
				 "MEDIATEK_256QAM_CAP for 2.4G\n");
		ent->fgGband256QAMSupport = TRUE;
	} else if ((brcm_ie & BROADCOM_256QAM_CAP)
			   && (pAd->CommonCfg.g_band_256_qam)) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
				 "BROADCOM_256QAM_CAP for 2.4G\n");
		ent->fgGband256QAMSupport = TRUE;
	}

#endif /*DOT11_VHT_AC*/
}

#ifdef DOT11_VHT_AC
VOID MacTableEntryCheck2GVHT(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
#ifdef G_BAND_256QAM
	struct _RTMP_CHIP_CAP *cap = NULL;
	struct wifi_dev *wdev = NULL;

	if (!IS_VALID_ENTRY(pEntry) || pEntry->fgGband256QAMSupport)
		return;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	wdev = pEntry->wdev;

	if (wdev) {
		if ((pAd->CommonCfg.g_band_256_qam) &&
			(cap->mcs_nss.g_band_256_qam) &&
			(WMODE_CAP(wdev->PhyMode, WMODE_GN)) &&
			WMODE_CAP_2G(wdev->PhyMode)) {
			pEntry->fgGband256QAMSupport = TRUE;

			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
				"Peer has 256QAM CAP support for 2.4G!\n");
		}
	}
#endif /* G_BAND_256QAM */
}
#endif /* DOT11_VHT_AC */

/*
	==========================================================================
	Description:
		Look up the MAC address in the MAC table. Return NULL if not found.
	Return:
		pEntry - pointer to the MAC entry; NULL is not found
	==========================================================================
*/
MAC_TABLE_ENTRY *MacTableLookup(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL, *pSearchEntry = NULL;
	UINT16 seek_cnt = 0;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pSearchEntry = pAd->MacTab.Hash[HashIdx];

	while (pSearchEntry && !IS_ENTRY_NONE(pSearchEntry)) {
		if (MAC_ADDR_EQUAL(pSearchEntry->Addr, pAddr)) {
			pEntry = pSearchEntry;
			break;
		} else {
			pSearchEntry = pSearchEntry->pNext;
			seek_cnt++;
		}

		if (seek_cnt >= MAX_LEN_OF_MAC_TABLE) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATMLME_WTBL, DBG_LVL_ERROR,
					  "!!ERROR!! ("MACSTR") search fail, seek_cnt(%d)\n",
					  MAC2STR(pAddr), seek_cnt);
			break;
		}
	}

	return pEntry;
}

MAC_TABLE_ENTRY *MacTableLookup2(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL, *pSearchEntry = NULL;
	UINT16 seek_cnt = 0;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pSearchEntry = pAd->MacTab.Hash[HashIdx];

	if (wdev) {
		while (pSearchEntry && !IS_ENTRY_NONE(pSearchEntry)) {
			if (MAC_ADDR_EQUAL(pSearchEntry->Addr, pAddr) && (pSearchEntry->wdev == wdev)) {
				pEntry = pSearchEntry;
				break;
			}	else {
				pSearchEntry = pSearchEntry->pNext;
				seek_cnt++;
			}

			if (seek_cnt >= MAX_LEN_OF_MAC_TABLE) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATMLME_WTBL, DBG_LVL_ERROR,
						  "!!ERROR!! ("MACSTR") search fail, seek_cnt(%d)\n",
						  MAC2STR(pAddr), seek_cnt);
				break;
			}
		}
	} else {

		while (pSearchEntry && !IS_ENTRY_NONE(pSearchEntry)) {
			if (MAC_ADDR_EQUAL(pSearchEntry->Addr, pAddr)) {
				pEntry = pSearchEntry;
				break;
			} else {
				pSearchEntry = pSearchEntry->pNext;
				seek_cnt++;
			}

			if (seek_cnt >= MAX_LEN_OF_MAC_TABLE) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATMLME_WTBL, DBG_LVL_ERROR,
						  "!!ERROR!! ("MACSTR") search fail, seek_cnt(%d)\n",
						  MAC2STR(pAddr), seek_cnt);
				break;
			}
		}
	}

	return pEntry;
}

#ifdef CONFIG_STA_SUPPORT
static BOOLEAN is_ht_supported(
	struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *peer,
	struct _STA_ADMIN_CONFIG *sta_cfg, struct adhoc_info *adhocInfo,
	struct wifi_dev *wdev, struct common_ies *cmm_ies)
{
	BOOLEAN supported = FALSE;

	if (((!IS_CIPHER_WEP(peer->SecConfig.PairwiseCipher)) &&
		(!IS_CIPHER_TKIP(peer->SecConfig.PairwiseCipher))) ||
		(ad->CommonCfg.HT_DisallowTKIP == FALSE)) {
		if ((sta_cfg->BssType == BSS_INFRA) &&
			HAS_HT_CAPS_EXIST(cmm_ies->ie_exists) && WMODE_CAP_N(wdev->PhyMode))
			supported = TRUE;

		if ((sta_cfg->BssType == BSS_ADHOC) &&
			(adhocInfo->bAdhocN == TRUE) &&
			HAS_HT_CAPS_EXIST(cmm_ies->ie_exists) && WMODE_CAP_N(wdev->PhyMode))
			supported = TRUE;
	}

	return supported;
}

BOOLEAN StaUpdateMacTableEntry(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR MaxSupportedRateIn500Kbps,
	IN IE_LISTS * ie_list,
	IN USHORT cap_info)
{
	UCHAR MaxSupportedRate;
	BOOLEAN bSupportN = FALSE;
#ifdef TXBF_SUPPORT
	BOOLEAN supportsETxBf = FALSE;
#endif
	struct wifi_dev *wdev = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct adhoc_info *adhocInfo = NULL;
	struct _RTMP_CHIP_CAP *cap;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct _HT_CAPABILITY_IE *ht_cap = &cmm_ies->ht_cap;

	if (!pEntry)
		return FALSE;

	wdev = pEntry->wdev;
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	adhocInfo = &pStaCfg->adhocInfo;

	if (ADHOC_ON(pAd))
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);

	if (WMODE_EQUAL(wdev->PhyMode, WMODE_G)
		&& (MaxSupportedRate < RATE_FIRST_OFDM_RATE))
		return FALSE;

#ifdef DOT11_N_SUPPORT
	/* 11n only */
	if (WMODE_HT_ONLY(wdev->PhyMode) && !HAS_HT_CAPS_EXIST(cmm_ies->ie_exists))
		return FALSE;
#endif /* DOT11_N_SUPPORT */

	NdisAcquireSpinLock(&pAd->MacTabLock);
	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
	NdisZeroMemory(pEntry->SecConfig.Handshake.ReplayCounter, sizeof(pEntry->SecConfig.Handshake.ReplayCounter));

	if ((MaxSupportedRate < RATE_FIRST_OFDM_RATE) ||
		WMODE_EQUAL(wdev->PhyMode, WMODE_B)) {
		pEntry->RateLen = 4;

		if (MaxSupportedRate >= RATE_FIRST_OFDM_RATE)
			MaxSupportedRate = RATE_11;
	} else
		pEntry->RateLen = 12;

	pEntry->MaxHTPhyMode.word = 0;
	pEntry->MinHTPhyMode.word = 0;
	pEntry->HTPhyMode.word = 0;
	pEntry->MaxSupportedRate = MaxSupportedRate;
	MacTableSetEntryPhyCfg(pAd, pEntry);
	pEntry->CapabilityInfo = cap_info;
	CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
#ifdef DOT11_N_SUPPORT
	NdisZeroMemory(&pEntry->HTCapability, sizeof(pEntry->HTCapability));

	bSupportN = is_ht_supported(pAd, pEntry, pStaCfg, adhocInfo, wdev, cmm_ies);
	if (bSupportN) {
		if (ADHOC_ON(pAd))
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		ht_mode_adjust(pAd, pEntry, &cmm_ies->ht_cap);
#ifdef P2P_SUPPORT

		/* TODO: shiang-6590, fix this fallback case!! */
		if (pStaCfg->MlmeAux.bBwFallBack == TRUE)
			pEntry->MaxHTPhyMode.field.BW = BW_20;

#endif /* P2P_SUPPORT */
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
		if (cap->FlgHwTxBfCap)
			supportsETxBf = mt_WrapClientSupportsETxBF(pAd, &ht_cap->TxBFCap);

#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
		/* find max fixed rate */
		pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(&wdev->DesiredHtPhyInfo.MCSSet[0], &ht_cap->MCSSet[0]);

		if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO)
			set_ht_fixed_mcs(pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);

		pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
		set_sta_ht_cap(pAd, pEntry, ht_cap);
		NdisMoveMemory(&pEntry->HTCapability, ht_cap, SIZE_HT_CAP_IE);
		assoc_ht_info_debugshow(pAd, pEntry, ht_cap);
#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(wdev->PhyMode) &&
			HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists) &&
			HAS_VHT_OP_EXIST(cmm_ies->ie_exists)) {
			vht_mode_adjust(pAd, pEntry, &cmm_ies->vht_cap, &cmm_ies->vht_op,
			(cmm_ies->operating_mode_len == 0) ? NULL :  &cmm_ies->operating_mode, NULL);
			dot11_vht_mcs_to_internal_mcs(pAd, wdev,
					&cmm_ies->vht_cap, &pEntry->MaxHTPhyMode);
			set_vht_cap(pAd, pEntry, &cmm_ies->vht_cap);
			NdisMoveMemory(&pEntry->vht_cap_ie, &cmm_ies->vht_cap, sizeof(VHT_CAP_IE));
			assoc_vht_info_debugshow(pAd, pEntry,
					&cmm_ies->vht_cap, &cmm_ies->vht_op);
		}
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
		if (WMODE_CAP_AX(wdev->PhyMode)
				&& HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
			update_peer_he_caps(pEntry, cmm_ies);
			update_peer_he_operation(pEntry, cmm_ies);
			he_mode_adjust(wdev, pEntry, NULL);
		}
#endif /*DOT11_HE_AX*/
	}
#ifdef DOT11_HE_AX
	else if (WMODE_CAP_AX_6G(wdev->PhyMode)
				&& HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
		update_peer_he_caps(pEntry, cmm_ies);
		update_peer_he_operation(pEntry, cmm_ies);
		he_mode_adjust(wdev, pEntry, NULL);
	}
#endif /*DOT11_HE_AX*/
	else
		pAd->MacTab.fAnyStationIsLegacy = TRUE;

#endif /* DOT11_N_SUPPORT */
	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
	pEntry->CurrTxRate = pEntry->MaxSupportedRate;
#ifdef MFB_SUPPORT
	pEntry->lastLegalMfb = 0;
	pEntry->isMfbChanged = FALSE;
	pEntry->fLastChangeAccordingMfb = FALSE;
	pEntry->toTxMrq = TRUE;
	pEntry->msiToTx = 0; /* has to increment whenever a mrq is sent */
	pEntry->mrqCnt = 0;
	pEntry->pendingMfsi = 0;
	pEntry->toTxMfb = FALSE;
	pEntry->mfbToTx = 0;
	pEntry->mfb0 = 0;
	pEntry->mfb1 = 0;
#endif /* MFB_SUPPORT */
	pEntry->freqOffsetValid = FALSE;
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	mt_WrapTxBFInit(pAd, pEntry, ie_list, supportsETxBf);
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (wdev->bAutoTxRateSwitch == TRUE)
			pEntry->bAutoTxRateSwitch = TRUE;
		else {
			pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
			pEntry->HTPhyMode.field.MODE = wdev->HTPhyMode.field.MODE;
			pEntry->bAutoTxRateSwitch = FALSE;

			if (pEntry->HTPhyMode.field.MODE >= MODE_VHT) {
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS +
											  ((wlan_operate_get_tx_stream(wdev) - 1) << 4);
			}

			/* If the legacy mode is set, overwrite the transmit setting of this entry. */
			RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
		}
	}

#endif /* MT_MAC */

	pEntry->Sst = SST_ASSOC;
	pEntry->AuthState = AS_AUTH_OPEN;
	/* pEntry->SecConfig.AKMMap = wdev->SecConfig.AKMMap; */
	/* pEntry->SecConfig.PairwiseCipher = wdev->SecConfig.PairwiseCipher; */
#ifdef HTC_DECRYPT_IOT

	if ((pEntry->HTC_ICVErrCnt)
		|| (pEntry->HTC_AAD_OM_Force)
		|| (pEntry->HTC_AAD_OM_CountDown)
		|| (pEntry->HTC_AAD_OM_Freeze)
	   ) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATMLME_WTBL, DBG_LVL_DEBUG,
			"(wcid=%u), HTC_ICVErrCnt(%u), HTC_AAD_OM_Freeze(%u)\n",
			pEntry->wcid, pEntry->HTC_ICVErrCnt, pEntry->HTC_AAD_OM_Force);
		MTWF_DBG(pAd, DBG_CAT_RX, CATMLME_WTBL, DBG_LVL_DEBUG, ", HTC_AAD_OM_CountDown(%u),  HTC_AAD_OM_Freeze(%u) is in Asso. stage!\n",
				 pEntry->HTC_AAD_OM_CountDown, pEntry->HTC_AAD_OM_Freeze);
		/* Force clean. */
		pEntry->HTC_ICVErrCnt = 0;
		pEntry->HTC_AAD_OM_Force = 0;
		pEntry->HTC_AAD_OM_CountDown = 0;
		pEntry->HTC_AAD_OM_Freeze = 0;
	}

#endif /* HTC_DECRYPT_IOT */

	if (IS_AKM_OPEN(pEntry->SecConfig.AKMMap)
		|| IS_AKM_SHARED(pEntry->SecConfig.AKMMap)
		|| IS_AKM_AUTOSWITCH(pEntry->SecConfig.AKMMap)) {
		pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
		pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	} else {
		pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
	}

	/* In WPA or 802.1x mode, the port is not secured. */
	if (IS_AKM_WPA_CAPABILITY(pEntry->SecConfig.AKMMap)
#ifdef DOT1X_SUPPORT
	|| IS_IEEE8021X_Entry(wdev)
#endif /* DOT1X_SUPPORT */
	)
		tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	else
		tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					  "wdev(name=%s,type=%d,PortSecured=%d/%d), (AID=%d, ssid=%s)\n",
					  wdev->if_dev->name, wdev->wdev_type, tr_entry->PortSecured, wdev->PortSecured,
					  pStaCfg->StaActive.Aid, pStaCfg->Ssid);

	NdisReleaseSpinLock(&pAd->MacTabLock);
	/*update tx burst, must after unlock pAd->MacTabLock*/
	/* rtmp_tx_burst_set(pAd); */
#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT

	if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP) {
		SendAssocIEsToWpaSupplicant(pAd->net_dev, pStaCfg->ReqVarIEs,
									pStaCfg->ReqVarIELen);
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,
								RT_ASSOC_EVENT_FLAG, NULL, NULL, 0);
	}

#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	{
		/*        union iwreq_data    wrqu; */
		wext_notify_event_assoc(pAd->net_dev, pStaCfg->ReqVarIEs,
								pStaCfg->ReqVarIELen);
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1,
								pStaCfg->MlmeAux.Bssid, NULL, 0);
	}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
	return TRUE;
}
#endif /* CONFIG_STA_SUPPORT */


INT MacTableResetEntry(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, BOOLEAN clean)
{
	BOOLEAN Cancelled;
	struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
	UINT8 i;

	RTMPCancelTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
	OS_CLEAR_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
	RTMPCancelTimer(&pEntry->DABSRetryTimer, &Cancelled);
#endif
#endif
#ifdef DOT11W_PMF_SUPPORT
	RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */
	ba_session_tear_down_all(pAd, pEntry->wcid, FALSE);
	NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

	if (clean == TRUE) {
		pEntry->MaxSupportedRate = RATE_11;
		pEntry->CurrTxRate = RATE_11;
		NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

		/* init average rssi */
		for (i = 0; i < RX_STREAM_PATH_SINGLE_MODE; i++)
			pEntry->RssiSample.AvgRssi[i] = MINIMUM_POWER_VALUE;
		pEntry->RssiSample.Rssi_Updated = FALSE;
	}

	return 0;
}


#ifdef OUI_CHECK_SUPPORT
static VOID oui_mgroup_update(MAC_TABLE *mtb, UCHAR *addr, UCHAR act)
{
	UCHAR tmp_cnt = 0;
	UCHAR i = 0;
	MAC_TABLE_ENTRY *entry;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
			 "%s(): update mgroup before num:%d. act=%d\n", __func__, mtb->oui_mgroup_cnt, act);

	for (i = 0; i <= mtb->Size; i++) {
		entry = &mtb->Content[i];

		if (MAC_OUI_EQUAL(entry->Addr, addr) && !MAC_ADDR_EQUAL(entry->Addr, addr)) {
			tmp_cnt++;

			if (tmp_cnt > 2)
				break;
		}
	}

	/*only 1  oui equal means new match group*/
	if (tmp_cnt == 1) {
		mtb->oui_mgroup_cnt = (act == OUI_MGROUP_ACT_JOIN) ?
							  (mtb->oui_mgroup_cnt + 1) : (mtb->oui_mgroup_cnt - 1);
	}

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
			 "%s(): update mgroup after num:%d\n", __func__, mtb->oui_mgroup_cnt);
}
#endif /*OUI_CHECK_SUPPORT*/

#ifdef MT7626_REDUCE_TX_OVERHEAD
extern UINT8 cached_flag_ar[];
#endif /* MT7626_REDUCE_TX_OVERHEAD */
MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN UINT32 ent_type,
	IN UCHAR OpMode,
	IN BOOLEAN CleanAll)
{
	UCHAR HashIdx;

#ifdef WTBL_TDD_SUPPORT
	UCHAR useExt = 0, SegIdx = 0xff;
#endif /* WTBL_TDD_SUPPORT */
#ifdef SW_CONNECT_SUPPORT
	UINT16 hw_wcid = WCID_INVALID;
	BOOLEAN bSw = FALSE;
#endif /* SW_CONNECT_SUPPORT */
#ifdef DATA_TXPWR_CTRL
	UINT8 u1BwIdx = 0, u1McsIdx = 0;
#endif
	BOOLEAN is_apcli = FALSE;
	UINT16 i;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	/* ASIC_SEC_INFO Info = {0}; */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
	struct _RTMP_CHIP_CAP *cap;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

#ifndef WTBL_TDD_SUPPORT
	if (pAd->MacTab.Size >= GET_MAX_UCAST_NUM(pAd)) {
		ASSERT(FALSE);
		return NULL;
	}
#endif /* !WTBL_TDD_SUPPORT */

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	if((ent_type == ENTRY_INFRA) && (OpMode == OPMODE_STA))
		is_apcli = TRUE;
#ifdef WTBL_TDD_SUPPORT
	if (IS_WTBL_TDD_ENABLED(pAd)) {
		i = WtblTdd_AcquireUcastWcid(pAd, wdev, &useExt, &SegIdx);
	} else
#endif /* WTBL_TDD_SUPPORT */
	{
#ifdef SW_CONNECT_SUPPORT
		i = HcAcquireUcastWcid(pAd, wdev, FALSE, is_apcli, &hw_wcid, &bSw);
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "wcid =%u, hw_wcid=%u, bSw=%d\n", i, hw_wcid, bSw);
#else /* SW_CONNECT_SUPPORT */
		i = HcAcquireUcastWcid(pAd, wdev, FALSE, is_apcli);
#endif /* !SW_CONNECT_SUPPORT */
	}

	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);
	if (i == WCID_INVALID
#ifdef SW_CONNECT_SUPPORT
		&& (bSw == FALSE)
#endif /* SW_CONNECT_SUPPORT */
		) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
		"Entry full!\n");
		NdisReleaseSpinLock(&pAd->MacTabLock);
		return NULL;
	}
#ifdef WTBL_TDD_SUPPORT
	if (!IS_WTBL_TDD_ENABLED(pAd))
#endif /* WTBL_TDD_SUPPORT */
	{
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
			 "(caller:%pS): wcid %d EntryType:%d-%d =====\n", OS_TRACE, i, ent_type, pAd->MacTab.Content[i].EntryType);
	}

	/* pick up the first available vacancy*/
	if (IS_ENTRY_NONE(&pAd->MacTab.Content[i])) {
		pEntry = &pAd->MacTab.Content[i];
		MacTableResetEntry(pAd, pEntry, CleanAll);
		/* ENTRY PREEMPTION: initialize the entry */
		pEntry->wdev = wdev;
#ifdef SW_CONNECT_SUPPORT
		pEntry->bSw = bSw;
		/*
			original pEntry->wcid entry usages are most for S/W concept, only minor parts are for H/W concept.
			so add extra pEntry->hw_wcid for hw backup.
		*/
		pEntry->hw_wcid = hw_wcid;

		/* S/W entry Force set valid , w/o cmd resp check */
		if (bSw == TRUE) {
			pEntry->sta_rec_valid = TRUE;
			pEntry->DummyHTPhyMode.word = wdev->pDummy_obj->HTPhyMode.word; /* assign the Dummy Wcid Fixed Rate to per STA */
		} else {
			pEntry->DummyHTPhyMode.word = wdev->rate.MlmeTransmit.word; /* default mlme rate */
		}

#endif /* SW_CONNECT_SUPPORT */
		pEntry->wcid = i;
		pEntry->func_tb_idx = wdev->func_idx;
		pEntry->bIAmBadAtheros = FALSE;
		pEntry->pAd = pAd;
		pEntry->CMTimerRunning = FALSE;
		pEntry->agg_err_flag = FALSE;
		pEntry->winsize_limit = 0xF;
		COPY_MAC_ADDR(pEntry->Addr, pAddr);
#ifdef DATA_TXPWR_CTRL
		pEntry->DataTxPwrEn = FALSE;

		for (u1BwIdx = 0; u1BwIdx < DATA_TXPOWER_MAX_BW_NUM; u1BwIdx++) {
			for (u1McsIdx = 0; u1McsIdx < DATA_TXPOWER_MAX_MCS_NUM; u1McsIdx++) {
				pEntry->PowerOffset[u1BwIdx][u1McsIdx] = 0;
			}
		}

#endif


		if (OpMode != OPMODE_ATE) {
			struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;

			RTMPInitTimer(pAd, &pSecConfig->StartFor4WayTimer, GET_TIMER_FUNCTION(WPAStartFor4WayExec), pEntry, FALSE);
			RTMPInitTimer(pAd, &pSecConfig->StartFor2WayTimer, GET_TIMER_FUNCTION(WPAStartFor2WayExec), pEntry, FALSE);
			RTMPInitTimer(pAd, &pSecConfig->Handshake.MsgRetryTimer, GET_TIMER_FUNCTION(WPAHandshakeMsgRetryExec), pEntry, FALSE);
#ifdef DOT11W_PMF_SUPPORT
			RTMPInitTimer(pAd, &pSecConfig->PmfCfg.SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, FALSE);
			RTMPInitTimer(pAd, &pSecConfig->PmfCfg.SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, FALSE);
#endif /* DOT11W_PMF_SUPPORT */
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
			RTMPInitTimer(pAd, &pEntry->DABSRetryTimer, GET_TIMER_FUNCTION(RTMPDABSretry), pEntry, FALSE);
#endif
#endif

		}
		RTMP_OS_INIT_COMPLETION(&pEntry->WtblSetDone);
		pEntry->Sst = SST_NOT_AUTH;
		pEntry->AuthState = AS_NOT_AUTH;
		pEntry->Aid = entrytb_aid_aquire(&pAd->MacTab.aid_info);
		if (pEntry->Aid == INVALID_AID) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
				"allocate AID fail!\n");
			HcReleaseUcastWcid(pAd, wdev, pEntry->wcid);
			MacTableResetEntry(pAd, pEntry, CleanAll);
			NdisReleaseSpinLock(&pAd->MacTabLock);
			return NULL;
		}
		pEntry->CapabilityInfo = 0;
		pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
		pEntry->PsMode = PWR_ACTIVE;
		pEntry->NoDataIdleCount = 0;
		pEntry->ContinueTxFailCnt = 0;
#ifdef WDS_SUPPORT
		pEntry->LockEntryTx = FALSE;
#endif /* WDS_SUPPORT */
		pEntry->TimeStamp_toTxRing = 0;
		/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
#ifdef SW_CONNECT_SUPPORT
		tr_ctl->tr_entry[i].bSw = bSw;
#endif /* SW_CONNECT_SUPPORT */
		tr_ctl->tr_entry[i].PsMode = PWR_ACTIVE;
		tr_ctl->tr_entry[i].NoDataIdleCount = 0;
		tr_ctl->tr_entry[i].ContinueTxFailCnt = 0;
		tr_ctl->tr_entry[i].LockEntryTx = FALSE;
		tr_ctl->tr_entry[i].TimeStamp_toTxRing = 0;
		tr_ctl->tr_entry[i].PsDeQWaitCnt = 0;
		pEntry->SecConfig.pmkid = NULL;
		pEntry->SecConfig.pmk_cache = NULL;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		pEntry->bTxPktChk = FALSE;
		pEntry->TotalTxSuccessCnt = 0;
		pEntry->TxStatRspCnt = 0;
#endif

		do {
#ifdef CONFIG_STA_SUPPORT

			if (ent_type == ENTRY_INFRA) {
				SET_ENTRY_AP(pEntry);
#ifdef P2P_SUPPORT
				SET_P2P_CLI_ENTRY(pEntry);
				pAd->P2pCfg.MyGOwcid = i;
#endif /* P2P_SUPPORT */
				COPY_MAC_ADDR(pEntry->bssid, pAddr);

				pStaCfg->MacTabWCID = pEntry->wcid;

				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_AP);/*the peer type related to APCLI is AP role.*/
				pStaCfg->pAssociatedAPEntry = (PVOID)pEntry;

				if ((IS_AKM_OPEN(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_AUTOSWITCH(pEntry->SecConfig.AKMMap))) {
					pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				} else {
					pEntry->SecConfig.Handshake.WpaState = AS_INITIALIZE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
				}

#ifdef VOW_SUPPORT
				if ((!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG)) || pAd->vow_cfg.en_bw_ctrl) {
					if (VOW_IS_ENABLED(pAd)) {
						UINT32 orig_group_id = pAd->bss_group.group_idx[pEntry->func_tb_idx];

						if (pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) {
							UINT_8 new_group_id = pAd->max_bssgroup_num - pEntry->wdev->func_idx - 1;
							orig_group_id = pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx];

							pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx] = new_group_id;

							pAd->vow_bss_cfg[new_group_id].group_table_idx = pEntry->wdev->func_idx;
							RTMP_SET_STA_DWRR(pAd, pEntry);
							pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx] = orig_group_id;

						} else {
							if (pAd->vow_cfg.en_bw_ctrl) {
								pAd->bss_group.group_idx[pEntry->func_tb_idx] =
									pAd->max_bssgroup_num - pEntry->func_tb_idx - 1;
							}

							RTMP_SET_STA_DWRR(pAd, pEntry);
							pAd->bss_group.group_idx[pEntry->func_tb_idx] = orig_group_id;
						}
					} else
						RTMP_SET_STA_DWRR(pAd, pEntry);
				}
#else
				if (!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG))
					RTMP_SET_STA_DWRR(pAd, pEntry);
#endif
				break;
			}

#ifdef P2P_SUPPORT

			if (ent_type == ENTRY_GC) {
				SET_ENTRY_CLIENT(pEntry);
				break;
			}

#endif /* P2P_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

			if (ent_type == ENTRY_TDLS) {
				SET_ENTRY_TDLS(pEntry);
				break;
			} else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			if (ent_type == ENTRY_AP || ent_type == ENTRY_ADHOC) {
				if  (ent_type == ENTRY_ADHOC)
					SET_ENTRY_ADHOC(pEntry);

#ifdef IWSC_SUPPORT
				pEntry->bIWscSmpbcAccept = FALSE;
				pEntry->bUpdateInfoFromPeerBeacon = FALSE;
#endif /* IWSC_SUPPORT // */
				COPY_MAC_ADDR(pEntry->bssid, pAddr);
				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
			else if (ent_type == ENTRY_REPEATER) {
				PSTA_ADMIN_CONFIG main_sta = GetStaCfgByWdev(pAd, wdev);
				MAC_TABLE_ENTRY *pRootApEntry = &pAd->MacTab.Content[main_sta->MacTabWCID];

				SET_ENTRY_REPEATER(pEntry);
				COPY_MAC_ADDR(pEntry->bssid, pAddr);
				pEntry->SecConfig.AKMMap = pRootApEntry->SecConfig.AKMMap;
				pEntry->SecConfig.PairwiseCipher = pRootApEntry->SecConfig.PairwiseCipher;
				pEntry->SecConfig.GroupCipher = pRootApEntry->SecConfig.GroupCipher;
				pEntry->pReptCli = NULL;
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_AP);/*the peer type related to Rept is AP role.*/

				if ((IS_AKM_OPEN(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_AUTOSWITCH(pEntry->SecConfig.AKMMap))) {
					pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				} else {
					pEntry->SecConfig.Handshake.WpaState = AS_INITIALIZE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
				}

					/* apcli group idx assignments are from (VOW_MAX_GROUP_NUM-1) reversely
					 * e.g. entry[0] => group idx: VOW_MAX_GROUP_NUM - 1
					 *      entry[1] => group idx: VOW_MAX_GROUP_NUM - 2
					 */
#ifdef VOW_SUPPORT
				if ((!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG)) || pAd->vow_cfg.en_bw_ctrl) {
					if (VOW_IS_ENABLED(pAd)) {
						UINT32 orig_group_id = pAd->bss_group.group_idx[pEntry->func_tb_idx];

						if (pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) {
							UINT_8 new_group_id = pAd->max_bssgroup_num - pEntry->wdev->func_idx - 1;
							orig_group_id = pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx];

							pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx] = new_group_id;

							pAd->vow_bss_cfg[new_group_id].group_table_idx = pEntry->wdev->func_idx;
							RTMP_SET_STA_DWRR(pAd, pEntry);
							pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx] = orig_group_id;

						} else {
							if (pAd->vow_cfg.en_bw_ctrl) {
								pAd->bss_group.group_idx[pEntry->func_tb_idx] =
									pAd->max_bssgroup_num - pEntry->func_tb_idx - 1;
							}

							RTMP_SET_STA_DWRR(pAd, pEntry);
							pAd->bss_group.group_idx[pEntry->func_tb_idx] = orig_group_id;
						}
					} else
						RTMP_SET_STA_DWRR(pAd, pEntry);
				}
#else
				if (!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG))
					RTMP_SET_STA_DWRR(pAd, pEntry);
#endif /* VOW_SUPPORT */

				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
						  "Repeater Security wcid=%d, AKMMap=0x%x, PairwiseCipher=0x%x, GroupCipher=0x%x\n",
						  pEntry->wcid, pEntry->SecConfig.AKMMap,
						  pEntry->SecConfig.PairwiseCipher, pEntry->SecConfig.GroupCipher);
				break;
			}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT

			if (ent_type == ENTRY_WDS) {
				SET_ENTRY_WDS(pEntry);
				SET_CONNECTION_TYPE(pEntry, CONNECTION_WDS);
				COPY_MAC_ADDR(pEntry->bssid, pEntry->wdev->bssid);
				break;
			}

#endif /* WDS_SUPPORT */

			if (ent_type == ENTRY_CLIENT) {
				/* be a regular-entry*/
				if (
#ifdef WTBL_TDD_SUPPORT
				     (!IS_WTBL_TDD_ENABLED(pAd)) &&
#endif /* WTBL_TDD_SUPPORT */
					pAd->ApCfg.EntryClientCount >= GET_MAX_UCAST_NUM(pAd)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
							 " The station number is over MaxUcastEntryNum = %d\n", GET_MAX_UCAST_NUM(pAd));
					HcReleaseUcastWcid(pAd, wdev, pEntry->wcid);
					aid_clear(&pAd->MacTab.aid_info, pEntry->Aid);
					MacTableResetEntry(pAd, pEntry, CleanAll);
					NdisReleaseSpinLock(&pAd->MacTabLock);
					return NULL;
				}

				if ((pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
					(VALID_MBSS(pAd, pEntry->func_tb_idx)) &&
					(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum != 0) &&
					(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount >= pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR," The connection table is full in ra%d.\n", pEntry->func_tb_idx);
					HcReleaseUcastWcid(pAd, wdev, pEntry->wcid);
					aid_clear(&pAd->MacTab.aid_info, pEntry->Aid);
					MacTableResetEntry(pAd, pEntry, CleanAll);
					NdisReleaseSpinLock(&pAd->MacTabLock);
					return NULL;
				}

				if (!VALID_MBSS(pAd, pEntry->func_tb_idx)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
						" The func_tb_idx is over cap = %d\n",  MAX_MBSSID_NUM(pAd));
					HcReleaseUcastWcid(pAd, wdev, pEntry->wcid);
					aid_clear(&pAd->MacTab.aid_info, pEntry->Aid);
					MacTableResetEntry(pAd, pEntry, CleanAll);
					NdisReleaseSpinLock(&pAd->MacTabLock);
					return NULL;
				}

				ASSERT((wdev == &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev));
				SET_ENTRY_CLIENT(pEntry);
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_STA);
				pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
				MBSS_MR_APIDX_SANITY_CHECK(pAd, pEntry->func_tb_idx);
#ifdef P2P_SUPPORT

				if (apidx == MIN_NET_DEVICE_FOR_P2P_GO)
					COPY_MAC_ADDR(pEntry->bssid, pAd->P2PCurrentAddress);

#endif /* P2P_SUPPORT */
				COPY_MAC_ADDR(pEntry->bssid, wdev->bssid);

				if (IS_SECURITY_OPEN_WEP(&wdev->SecConfig)) {
					/* OPEN WEP */
					pEntry->SecConfig.AKMMap = wdev->SecConfig.AKMMap;
					pEntry->SecConfig.PairwiseCipher = wdev->SecConfig.PairwiseCipher;
					pEntry->SecConfig.PairwiseKeyId = wdev->SecConfig.PairwiseKeyId;
					pEntry->SecConfig.GroupCipher = wdev->SecConfig.GroupCipher;
					pEntry->SecConfig.GroupKeyId = wdev->SecConfig.GroupKeyId;
					os_move_mem(pEntry->SecConfig.WepKey, wdev->SecConfig.WepKey, sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
					pEntry->SecConfig.GroupKeyId = wdev->SecConfig.GroupKeyId;
				}

				if ((IS_AKM_OPEN(wdev->SecConfig.AKMMap))
					|| (IS_SECURITY_OPEN_WEP(&wdev->SecConfig))
					|| (IS_AKM_SHARED(wdev->SecConfig.AKMMap)))
					pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
				else
					pEntry->SecConfig.Handshake.WpaState = AS_INITIALIZE;

				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				/* assign default idle timeout value to bss setting */
				pEntry->StaIdleTimeout = pEntry->pMbss->max_idle_period;
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount++;
				pAd->ApCfg.EntryClientCount++;
#ifdef VOW_SUPPORT

				/* vow_set_client(pAd, pEntry->func_tb_idx, pEntry->wcid); */
				if (VOW_IS_ENABLED(pAd)) {
					if (vow_watf_is_enabled(pAd))
						set_vow_watf_sta_dwrr(pAd, &pEntry->Addr[0], pEntry->wcid);
				}

				if ((pAd->vow_cfg.en_bw_ctrl) || vow_watf_is_enabled(pAd) ||
					(!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG))) {
					RTMP_SET_STA_DWRR(pAd, pEntry);
				}
#else
				if (!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG))
					RTMP_SET_STA_DWRR(pAd, pEntry);
#endif /* VOW_SUPPORT */
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
	if (wdev->wdev_type == WDEV_TYPE_AP)
		wdev->wdevStaCnt++;
#endif
#endif
#ifdef WH_EVENT_NOTIFIER
				pEntry->tx_state.CurrentState = WHC_STA_STATE_ACTIVE;
				pEntry->rx_state.CurrentState = WHC_STA_STATE_ACTIVE;
#endif /* WH_EVENT_NOTIFIER */
#ifdef ROAMING_ENHANCE_SUPPORT
#ifdef APCLI_SUPPORT
				pEntry->bRoamingRefreshDone = FALSE;
#endif /* APCLI_SUPPORT */
#endif /* ROAMING_ENHANCE_SUPPORT */
				break;
			}
#ifdef AIR_MONITOR
			else if (ent_type == ENTRY_MONITOR) {
				SET_ENTRY_MONITOR(pEntry);
				if (IS_MT7663(pAd) || IS_MT7626(pAd) || IS_MT7915(pAd) ||
					IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
					SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_STA);
				}
				break;
			}

#endif /* AIR_MONITOR */
#endif /* CONFIG_AP_SUPPORT */
#if defined(CONFIG_ATE)
			else if (ent_type == ENTRY_ATE) {
				SET_ENTRY_CLIENT(pEntry);
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_STA);
#if defined(CONFIG_AP_SUPPORT)
				pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
#endif
				break;
			}
#endif
		} while (FALSE);

		RTMP_SET_TR_ENTRY(pAd, pEntry);

		if (get_starec_by_wcid(pAd, i))
			del_starec(pAd, &tr_ctl->tr_entry[i]);

#ifdef CONFIG_STA_SUPPORT
#ifdef ADHOC_WPA2PSK_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			RTMPInitTimer(pAd, &pEntry->WPA_Authenticator.MsgRetryTimer, GET_TIMER_FUNCTION(Adhoc_WpaRetryExec), pEntry, FALSE);
		}
#endif /* ADHOC_WPA2PSK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef STREAM_MODE_SUPPORT
		/* Enable Stream mode for first three entries in MAC table */
#endif /* STREAM_MODE_SUPPORT */
#ifdef UAPSD_SUPPORT

		/* Ralink WDS doesn't support any power saving.*/
		if (IS_ENTRY_CLIENT(pEntry)
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			|| IS_ENTRY_TDLS(pEntry)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		   ) {
			/* init U-APSD enhancement related parameters */
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "INIT UAPSD MR ENTRY");
			UAPSD_MR_ENTRY_INIT(pEntry);
		}

#endif /* UAPSD_SUPPORT */

#ifdef WTBL_TDD_SUPPORT
		if (!useExt)
#endif /* WTBL_TDD_SUPPORT */
			pAd->MacTab.Size++;

#ifdef CONFIG_AP_SUPPORT
#ifdef P2P_SUPPORT

		if (OpMode == OPMODE_AP)
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT */
		{
#ifdef WSC_AP_SUPPORT
			pEntry->bWscCapable = FALSE;
			pEntry->Receive_EapolStart_EapRspId = 0;
#endif /* WSC_AP_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "alloc entry #%d, Total= %d\n",
				 i, pAd->MacTab.Size);
	}

	/* add this MAC entry into HASH table */
	if (pEntry) {
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);

		if (pAd->MacTab.Hash[HashIdx] == NULL)
			pAd->MacTab.Hash[HashIdx] = pEntry;
		else {
			pCurrEntry = pAd->MacTab.Hash[HashIdx];

			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;

			pCurrEntry->pNext = pEntry;
		}

#ifdef CONFIG_AP_SUPPORT
#ifdef P2P_SUPPORT

		if (OpMode == OPMODE_AP)
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT */
		{
#ifdef WSC_AP_SUPPORT

			if (IS_ENTRY_CLIENT(pEntry) &&
				(pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
				MAC_ADDR_EQUAL(pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.EntryAddr))
				NdisZeroMemory(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.EntryAddr, MAC_ADDR_LEN);

#endif /* WSC_AP_SUPPORT */
#ifdef SMART_ANTENNA
			pEntry->mcsInUse = -1;
#endif /* SMART_ANTENNA */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

			if (IS_ENTRY_CLIENT(pEntry) &&
				wf_drv_tbl.wf_fwd_add_entry_inform_hook)
				wf_drv_tbl.wf_fwd_add_entry_inform_hook(pEntry->Addr);

#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef MTFWD
			if (IS_ENTRY_CLIENT(pEntry)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN, "New Sta:"MACSTR"\n", MAC2STR(pEntry->Addr));
				RtmpOSWrielessEventSend(pEntry->wdev->if_dev,
							RT_WLAN_EVENT_CUSTOM,
							FWD_CMD_ADD_TX_SRC,
							NULL,
							(PUCHAR)pEntry->Addr,
							MAC_ADDR_LEN);
			}
#endif

		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef IWSC_SUPPORT

		if (pStaCfg->BssType == BSS_ADHOC) {
			WSC_PEER_ENTRY *pWscPeerEntry = NULL;

			pWscPeerEntry = WscFindPeerEntry(&pStaCfg->wdev.WscControl.WscPeerList, pEntry->Addr);

			if (pWscPeerEntry && pWscPeerEntry->bIWscSmpbcAccept)
				IWSC_AddSmpbcEnrollee(pAd, pEntry->Addr);
		}

#endif /* IWSC_SUPPORT */
	}

#ifdef OUI_CHECK_SUPPORT
#ifdef WTBL_TDD_SUPPORT
	if (!useExt)
#endif /* WTBL_TDD_SUPPORT */
	oui_mgroup_update(&pAd->MacTab, pAddr, OUI_MGROUP_ACT_JOIN);
#endif
	NdisReleaseSpinLock(&pAd->MacTabLock);

	/*update tx burst, must after unlock pAd->MacTabLock*/
	/* rtmp_tx_burst_set(pAd); */
#ifdef MT7626_REDUCE_TX_OVERHEAD
	if (pEntry)
		cached_flag_ar[pEntry->wcid] = 0;
#endif /* MT7626_REDUCE_TX_OVERHEAD */

	return pEntry;
}

INT32 MacTableDelEntryFromHash(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	USHORT HashIdx;
	MAC_TABLE_ENTRY  *pPrevEntry, *pProbeEntry;

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->Addr);
	pPrevEntry = NULL;
	pProbeEntry = pAd->MacTab.Hash[HashIdx];

	if (!pProbeEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
			 "pProbeEntry==NULL,pEntry->wcid=%d,MAC="MACSTR"\n",
			 pEntry->wcid, MAC2STR(pEntry->Addr));
	}


	/* update Hash list*/
	while (pProbeEntry) {
		if (pProbeEntry == pEntry) {
			if (pPrevEntry == NULL)
				pAd->MacTab.Hash[HashIdx] = pEntry->pNext;
			else
				pPrevEntry->pNext = pEntry->pNext;

			break;
		}

		pPrevEntry = pProbeEntry;
		pProbeEntry = pProbeEntry->pNext;
	};

	if (!pProbeEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
			"Failed to find pProbeEntry\n");
	}

	return TRUE;
}


/*
	==========================================================================
	Description:
		Delete a specified client from MAC table
	==========================================================================
 */
static VOID mac_entry_disconn_act(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG	pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev *wdev = pEntry->wdev;

	if (!pEntry || IS_ENTRY_NONE(pEntry))
		return;

	if (!wdev)
		return;

	/* Set port secure to NOT_SECURED here to avoid race condition with ba_ori_session_start */
	tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
	tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

	ba_session_tear_down_all(pAd, pEntry->wcid, TRUE);
	/* RTMP_STA_ENTRY_MAC_RESET--> AsicDelWcidTab() should be integrated to below function*/
	/*in the future */
#ifdef CONFIG_STA_SUPPORT
	/* snowpin for ap/sta */
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	if (pStaCfg && pStaCfg->pAssociatedAPEntry)
			if (pStaCfg->pAssociatedAPEntry == pEntry) {
				pStaCfg->pAssociatedAPEntry = NULL;
			}

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)

		if (!((pAd->WOW_Cfg.bEnable) &&
			  (pAd->WOW_Cfg.bWowIfDownSupport) &&
			  INFRA_ON(pStaCfg))) {
#endif /* WOW */
			if (wdev_do_disconn_act(pEntry->wdev, pEntry) != TRUE)
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
					" STA disconnection fail!\n");
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
		}

#endif /* WOW */

#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/* peer link up needs pEntry type information to decide txop disable or not*/
		/* so invalid pEntry type later */
		if (wdev_do_disconn_act(pEntry->wdev, pEntry) != TRUE)
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, " AP disconnection fail!\n");
	}
#endif /* CONFIG_STA_SUPPORT */
}

BOOLEAN MacTableDeleteEntry(RTMP_ADAPTER *pAd, USHORT wcid, UCHAR *pAddr)
{
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN Cancelled;
	BOOLEAN	bDeleteEntry = FALSE;
#ifdef DATA_TXPWR_CTRL
	UINT8 u1BwIdx = 0, u1McsIdx = 0;
#endif
#ifdef SMART_ANTENNA
	unsigned long irqflags;
#endif /* SMART_ANTENNA */
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *mbss = NULL;
#endif /*CONFIG_AP_SUPPORT*/
	struct wifi_dev *wdev;
	ADD_HT_INFO_IE *addht;
	UCHAR i;
#ifdef CONFIG_AP_SUPPORT
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) || defined(WAPP_SUPPORT)
	UCHAR TmpAddrForIndicate[MAC_ADDR_LEN] = {0};
#endif
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT)
	BOOLEAN bIndicateSendEvent = FALSE;
#endif /* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */
	struct _RTMP_CHIP_CAP *cap;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef MTFWD
	PNET_DEV if_dev = NULL;
#endif /* MTFWD */
#ifdef DABS_QOS
	UCHAR idx;
	struct qos_param_rec *prec = NULL;
#endif
	if (!pAd)
		return FALSE;

#ifdef WTBL_TDD_SUPPORT
	if (IS_WTBL_TDD_ENABLED(pAd)) {
		if (WtblTdd_Entry_DeInit(pAd, wcid, pAddr) == SW_TABLE)
			return FALSE;
	}
#endif /* WTBL_TDD_SUPPORT */

	if (!(VALID_UCAST_ENTRY_WCID(pAd, wcid)))
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
			 "(caller:%pS): wcid %d =====\n", OS_TRACE, wcid);

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &tr_ctl->tr_entry[wcid];

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	twt_resource_release_at_link_down(pAd, wcid);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	/*disconnect first*/
	mac_entry_disconn_act(pAd, pEntry);

	NdisAcquireSpinLock(&pAd->MacTabLock);

	if (pEntry &&
	    !IS_ENTRY_NONE(pEntry) &&
	    /** #256STA, we would like to recycle the entry idx 0 to use.
	     *  the line should be removed once the the index 0 has been recycled done.
	     */
	    !IS_ENTRY_MCAST(pEntry)) {
#ifdef DABS_QOS
		delete_qos_param_tbl_by_wlan_idx(pAd, pEntry->wcid, pEntry->wdev);
#endif
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
		pEntry->dabs_cfg = FALSE;
		pEntry->dabs_trans_id = 0;
		OS_CLEAR_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
		RTMPCancelTimer(&pEntry->DABSRetryTimer, &Cancelled);
		RTMPReleaseTimer(&pEntry->DABSRetryTimer, &Cancelled);
#endif
#endif
#ifdef CONFIG_AP_SUPPORT
		mbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
#endif /*CONFIG_AP_SUPPORT*/
#ifdef BAND_STEERING
		if ((pAd->ApCfg.BandSteering) && IS_ENTRY_CLIENT(pEntry) && IS_VALID_MAC(pEntry->Addr))
			BndStrg_UpdateEntry(pAd, pEntry, NULL, FALSE);
#endif
#ifdef MWDS
		MWDSAPPeerDisable(pAd, pEntry);
#endif /* MWDS */
#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
		map_a4_peer_disable(pAd, pEntry, TRUE);
#endif
		/*get wdev*/
		wdev = pEntry->wdev;
#ifdef CONFIG_AP_SUPPORT
		WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->func_tb_idx, pEntry->Aid);
#endif /* CONFIG_AP_SUPPORT */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

		if (wf_drv_tbl.wf_fwd_delete_entry_inform_hook)
			wf_drv_tbl.wf_fwd_delete_entry_inform_hook(pEntry->Addr);

#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef MTFWD
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN, "Del Sta:"MACSTR"\n", MAC2STR(pEntry->Addr));
		if (pEntry->wdev)
			if_dev = pEntry->wdev->if_dev;
#ifdef MAC_REPEATER_SUPPORT
		else if ((pEntry->wdev == NULL) && IS_ENTRY_REPEATER(pEntry))
			if_dev = repeater_get_apcli_ifdev(pAd, pEntry);
#endif /* MAC_REPEATER_SUPPORT */
		RtmpOSWrielessEventSend(if_dev,
					RT_WLAN_EVENT_CUSTOM,
					FWD_CMD_DEL_TX_SRC,
					NULL,
					(PUCHAR)pEntry->Addr,
					MAC_ADDR_LEN);
#endif
		pEntry->agg_err_flag = FALSE;
		pEntry->winsize_limit = 0xF;

		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr)) {


#if defined(CONFIG_AP_SUPPORT) && (defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP))

			if (pAd->ApCfg.MBSSID[pEntry->apidx].WNMCtrl.ProxyARPEnable) {
				RemoveIPv4ProxyARPEntry(pAd, mbss, pEntry->Addr, 0);
				RemoveIPv6ProxyARPEntry(pAd, mbss, pEntry->Addr);
			}

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
			pEntry->IsKeep = 0;
#endif /* CONFIG_HOTSPOT_R2 */
#endif
			bDeleteEntry = TRUE;
#ifdef STREAM_MODE_SUPPORT

			/* Clear Stream Mode register for this client */
			if (pEntry->StreamModeMACReg != 0)
				RTMP_IO_WRITE32(pAd->hdev_ctrl, pEntry->StreamModeMACReg + 4, 0);

#endif /* STREAM_MODE_SUPPORT // */
#ifdef CONFIG_STA_SUPPORT
#ifdef ADHOC_WPA2PSK_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				RTMPReleaseTimer(&pEntry->WPA_Authenticator.MsgRetryTimer, &Cancelled);
			}
#endif /* ADHOC_WPA2PSK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef CLIENT_WDS
			if (IS_ENTRY_CLIWDS(pEntry))
				CliWdsEnryFreeAid(pAd, pEntry->wcid);
#endif

			if (IS_ENTRY_CLIENT(pEntry)
#ifdef P2P_SUPPORT
				&& (IS_P2P_GO_ENTRY(pEntry))
#endif /* P2P_SUPPORT */
			   ) {
#ifdef DOT1X_SUPPORT

				/* Notify 802.1x daemon to clear this sta info*/
				if (IS_AKM_1X_Entry(pEntry)
					|| IS_IEEE8021X_Entry(wdev)
#ifdef RADIUS_ACCOUNTING_SUPPORT
					|| IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
#ifdef OCE_FILS_SUPPORT
					|| IS_AKM_FILS_Entry(pEntry)
#endif /* OCE_FILS_SUPPORT */
				   )
					DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
				IgmpGroupDelMembers(pAd, (PUCHAR)pEntry->Addr, wdev, pEntry->wcid);
#endif /* IGMP_SNOOP_SUPPORT */
				if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount > 0)
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount--;
				if (pAd->ApCfg.EntryClientCount > 0)
					pAd->ApCfg.EntryClientCount--;
#ifdef MAC_REPEATER_SUPPORT
				if (pEntry->ProxySta) {
					RepeaterDisconnectRootAP(pAd,
								(REPEATER_CLIENT_ENTRY *) pEntry->ProxySta,
								APCLI_DISCONNECT_SUB_REASON_MTM_REMOVE_STA);
					pEntry->ProxySta = NULL;
				}
#endif /* MAC_REPEATER_SUPPORT */
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
				if (wdev->wdev_type == WDEV_TYPE_AP) {
					wdev->wdevStaCnt--;
					if (MAC_ADDR_EQUAL(wdev->mLkMgnAddr, pEntry->Addr)) {
					/*if Min Link Margin STA leave, reset the Min Link Margin*/
						wdev->MinLinkMargin = 127;
						NdisZeroMemory(wdev->mLkMgnAddr, MAC_ADDR_LEN);
					}
					if (wdev->wdevStaCnt == 0) {
						wdev->pwrCnstrnt = 0;
						if (wdev->LastpwrCnst != wdev->pwrCnstrnt) {
							UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
							wdev->LastpwrCnst = 0;
						}
					}
				}
#endif
#endif
			}

#endif /* CONFIG_AP_SUPPORT */
			MacTableDelEntryFromHash(pAd, pEntry);
#ifdef CONFIG_AP_SUPPORT
			APCleanupPsQueue(pAd, &tr_entry->ps_queue); /* return all NDIS packet in PSQ*/
#endif /* CONFIG_AP_SUPPORT */
			TRTableResetEntry(pAd, wcid);
#ifdef UAPSD_SUPPORT
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			hex_dump("mac=", pEntry->Addr, 6);
			UAPSD_MR_ENTRY_RESET(pAd, pEntry);
#else
#ifdef CONFIG_AP_SUPPORT
			UAPSD_MR_ENTRY_RESET(pAd, pEntry);
#endif /* CONFIG_AP_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
			pEntry->pReptCli = NULL;
#endif

#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#endif /* UAPSD_SUPPORT */
			{
				struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;

				RTMPCancelTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
				RTMPCancelTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
				RTMPCancelTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
				RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
				RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */
			}
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT

			if (IS_ENTRY_CLIENT(pEntry)) {
				PWSC_CTRL	pWscControl = &mbss->wdev.WscControl;

				if (MAC_ADDR_EQUAL(pEntry->Addr, pWscControl->EntryAddr)) {
					/*
					Some WPS Client will send dis-assoc close to WSC_DONE.
					If AP misses WSC_DONE, WPS Client still sends dis-assoc to AP.
					Do not cancel timer if WscState is WSC_STATE_WAIT_DONE.
					*/
					if ((pWscControl->EapolTimerRunning == TRUE) &&
						(pWscControl->WscState != WSC_STATE_WAIT_DONE)) {
						RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
						pWscControl->EapolTimerRunning = FALSE;
						pWscControl->EapMsgRunning = FALSE;
						NdisZeroMemory(&(pWscControl->EntryAddr[0]), MAC_ADDR_LEN);
					}
				}

				pEntry->Receive_EapolStart_EapRspId = 0;
				pEntry->bWscCapable = FALSE;
#ifdef WH_EVENT_NOTIFIER
		pEntry->tx_state.CurrentState = WHC_STA_STATE_IDLE;
		pEntry->rx_state.CurrentState = WHC_STA_STATE_IDLE;
#endif /* WH_EVENT_NOTIFIER */
			}

#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef SMART_ANTENNA
			RTMP_IRQ_LOCK(&pAd->smartAntLock, irqflags);

			if (RTMP_SA_WORK_ON(pAd)) {
				RTMP_SA_TRAINING_PARAM *pTrainEntry = pEntry->pTrainEntry;

				if (pTrainEntry) {
					sa_del_train_entry(pAd, pEntry->Addr, FALSE);
					pEntry->pTrainEntry = NULL;
				}

				pAd->pSAParam->bStaChange = TRUE;
			}

			RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqflags);
#endif /* SMART_ANTENNA */
#ifdef CONFIG_AP_SUPPORT
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) || defined(WAPP_SUPPORT)
			COPY_MAC_ADDR(TmpAddrForIndicate, pEntry->Addr);
#endif
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT)
			bIndicateSendEvent = TRUE;
#endif /* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */

				/* NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY)); */
				NdisZeroMemory(pEntry->Addr, MAC_ADDR_LEN);
#ifdef P2P_SUPPORT

			if (IS_P2P_GO_ENTRY(pEntry)) {
				if (pEntry->WpaState == AS_PTKINITDONE)
					SET_P2P_ENTRY_NONE(pEntry);

				/* Legacy is leaving */
				if (pEntry->bP2pClient == FALSE) {
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
					P2pSendWirelessEvent(pAd, RT_P2P_LEGACY_DISCONNECTED, NULL, pEntry->Addr);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
				}
			}

#endif /* P2P_SUPPORT */
			pAd->MacTab.Size--;
#ifdef P2P_SUPPORT

			if (P2P_GO_ON(pAd)) {
				UINT32 i, p2pEntryCnt = 0;
				MAC_TABLE_ENTRY *pEntry;
				PWSC_CTRL pWscControl = &pAd->ApCfg.MBSSID[0].wdev.WscControl;

				for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					pEntry = &pAd->MacTab.Content[i];

					if (IS_P2P_GO_ENTRY(pEntry))
						p2pEntryCnt++;
				}

				if ((p2pEntryCnt == 0) &&
					(pWscControl->WscState == WSC_STATE_CONFIGURED) &&
					(pAd->flg_p2p_OpStatusFlags == P2P_GO_UP)) {
#ifdef RTMP_MAC_PCI
					P2pLinkDown(pAd, P2P_DISCONNECTED);
#endif /* RTMP_MAC_PCI */
				}

				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "MacTableDeleteEntry1 - Total= %d. p2pEntry = %d.\n", pAd->MacTab.Size, p2pEntryCnt);
			}

#endif /* P2P_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "MacTableDeleteEntry1 - Total= %d\n", pAd->MacTab.Size);
		} else {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN, "\n Impossible Wcid = %d !!!!!\n", wcid);
#ifdef CONFIG_AP_SUPPORT
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT)
			bIndicateSendEvent = FALSE;
#endif /* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */
		}

#ifdef CONFIG_AP_SUPPORT
		ApUpdateCapabilityAndErpIe(pAd, mbss);
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef OUI_CHECK_SUPPORT
	oui_mgroup_update(&pAd->MacTab, pAddr, OUI_MGROUP_ACT_LEAVE);
#endif

	/*update tx burst, must after unlock pAd->MacTabLock*/
	/* rtmp_tx_burst_set(pAd); */

	if (bDeleteEntry) {
		if (pAd->FragFrame.wcid == wcid) {
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "\n Clear Wcid = %d FragBuffer !!!!!\n", wcid);
			RESET_FRAGFRAME(pAd->FragFrame);
		}

#ifdef DATA_TXPWR_CTRL
	pEntry->DataTxPwrEn = FALSE;

	for (u1BwIdx = 0; u1BwIdx < DATA_TXPOWER_MAX_BW_NUM; u1BwIdx++) {
		for (u1McsIdx = 0; u1McsIdx < DATA_TXPOWER_MAX_MCS_NUM; u1McsIdx++) {
			pEntry->PowerOffset[u1BwIdx][u1McsIdx] = 0;
		}
	}
#endif

		/*release ucast wcid*/
		HcReleaseUcastWcid(pAd, pEntry->wdev, wcid);
		aid_clear(&pAd->MacTab.aid_info, pEntry->Aid);
#ifdef CONFIG_AP_SUPPORT
		/*
		* move CFG80211_ApStaDelSendEvent here after the entry & hash are deleted ,
		* to prevent removing the same hash twice
		*/
#ifdef RT_CFG80211_SUPPORT


		if (bIndicateSendEvent && pEntry && !IS_ENTRY_NONE(pEntry) && IS_ENTRY_CLIENT(pEntry)) {
			if (RTMP_CFG80211_HOSTAPD_ON(pAd)
#ifdef RT_CFG80211_P2P_SUPPORT
				|| RTMP_CFG80211_VIF_P2P_GO_ON(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
			   )
				CFG80211_ApStaDelSendEvent(pAd, TmpAddrForIndicate, pEntry->wdev->if_dev);
		}

#endif /* RT_CFG80211_SUPPORT */

#ifdef MBO_SUPPORT
		/* mbo - indicate daemon to remve this sta */
		if (IS_MBO_ENABLE(pEntry->wdev)) {
			MBO_EVENT_STA_DISASSOC evt_sta_disassoc;
			pEntry->is_mbo_bndstr_sta = 0;
			COPY_MAC_ADDR(evt_sta_disassoc.mac_addr, TmpAddrForIndicate);
			MboIndicateStaDisassocToDaemon(pAd, &evt_sta_disassoc, MBO_MSG_REMOVE_STA);
		}
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
		if (IS_ENTRY_CLIENT(pEntry)) {
			if (wdev->if_dev)
				wapp_send_cli_leave_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), TmpAddrForIndicate, pEntry);
		}
#endif /* WAPP_SUPPORT */

		if (IS_ENTRY_CLIENT(pEntry) && wdev->wdev_type != WDEV_TYPE_ATE_AP && wdev->wdev_type != WDEV_TYPE_ATE_STA)
			nonerp_sta_num(pEntry, PEER_LEAVE);
#endif /* CONFIG_AP_SUPPORT */
		/* invalidate the entry */
		SET_ENTRY_NONE(pEntry);

#ifdef GREENAP_SUPPORT
		greenap_check_peer_connection_at_link_up_down(pAd, wdev);
#endif /* GREENAP_SUPPORT */

#ifdef R1KH_HARD_RETRY
		RTMP_OS_EXIT_COMPLETION(&pEntry->ack_r1kh);
#endif /* R1KH_HARD_RETRY */
	}
	NdisReleaseSpinLock(&pAd->MacTabLock);
	/*Reset operating mode when no Sta.*/
	if (pAd->MacTab.Size == 0) {
#ifdef DOT11_N_SUPPORT
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			wdev = pAd->wdev_list[i];

			if (!wdev)
				continue;

			if (wdev->wdev_type != WDEV_TYPE_AP)
				continue;

			addht = wlan_operate_get_addht(wdev);
			addht->AddHtInfo2.OperaionMode = 0;
		}

#endif /* DOT11_N_SUPPORT */
	}

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	/*
	 * TxBF Dynamic Mechanism
	 * Executed when STA disconnected
	 */
	txbf_dyn_mech(pAd);
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */

#ifdef RED_SUPPORT
	if (pAd->MacTab.Size == 0)
		pAd->ixia_mode_ctl.fgProbeRspDetect = FALSE;
#endif
#ifdef DABS_QOS
	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];
		if (prec->wlan_idx == wcid && prec->valid == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				" DELETE table %d!!\n", idx);
			if (HW_UPDATE_QOS_PARAM(pAd, idx, FALSE) != NDIS_STATUS_SUCCESS)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"HW_UPDATE_QOS_PARAM Fail!!\n");
		}
	}
#endif

#ifdef CONFIG_VLAN_GTK_SUPPORT
	pEntry->vlan_id = 0;
#endif

	return TRUE;
}


/*
	==========================================================================
	Description:
		This routine reset the entire MAC table. All packets pending in
		the power-saving queues are freed here.
	==========================================================================
 */
VOID MacTableResetWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	int i;
#ifdef CONFIG_AP_SUPPORT
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	struct _BSS_STRUCT *mbss;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "MacTableResetWdev\n");

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = &pAd->MacTab.Content[i];

		if (pMacEntry->wdev != wdev)
			continue;

		if (IS_ENTRY_CLIENT(pMacEntry)) {
#ifdef CONFIG_STA_SUPPORT
#ifdef ADHOC_WPA2PSK_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				BOOLEAN Cancelled;

				RTMPReleaseTimer(&pMacEntry->WPA_Authenticator.MsgRetryTimer, &Cancelled);
			}
#endif /* ADHOC_WPA2PSK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					/*	send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, " MlmeAllocateMemory fail  ..\n");
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					MTWF_LOG(DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN, ("Send DeAuth (Reason=%d) to "MACSTR"\n",
							 Reason, MAC2STR(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
									 wdev->if_addr,
									 wdev->bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof(HEADER_802_11), &DeAuthHdr,
									  2, &Reason,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					MlmeFreeMemory(pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
#ifdef WSC_AP_SUPPORT
			{
				BOOLEAN Cancelled;

				RTMPCancelTimer(&mbss->wdev.WscControl.EapolTimer, &Cancelled);
				mbss->wdev.WscControl.EapolTimerRunning = FALSE;
				NdisZeroMemory(mbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN);
				mbss->wdev.WscControl.EapMsgRunning = FALSE;
			}
#endif /* WSC_AP_SUPPORT */
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}


VOID MacTableReset(RTMP_ADAPTER *pAd)
{
	int i;
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags = 0;
#endif /* RTMP_MAC_PCI */
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	UCHAR apidx = MAIN_MBSSID;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;
	/*
	MAC_TABLE_ENTRY *Hash[HASH_TABLE_SIZE];
	    MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	NdisZeroMemory(Hash, sizeof(Hash));
	NdisZeroMemory(Content, sizeof(Content));
	*/
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "MacTableReset\n");
	/*NdisAcquireSpinLock(&pAd->MacTabLock);*/

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pMacEntry)) {
#ifdef CONFIG_STA_SUPPORT
#ifdef ADHOC_WPA2PSK_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				BOOLEAN Cancelled;

				RTMPReleaseTimer(&pMacEntry->WPA_Authenticator.MsgRetryTimer, &Cancelled);
			}
#endif /* ADHOC_WPA2PSK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					/*  send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, " MlmeAllocateMemory fail  ..\n");
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
#ifdef WIFI_DIAG
					if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
						diag_conn_error(pAd, pMacEntry->func_tb_idx, pMacEntry->Addr,
							DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
					if (IS_ENTRY_CLIENT(pMacEntry))
						ApSendConnFailMsg(pAd,
							pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].Ssid,
							pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].SsidLen,
							pMacEntry->Addr,
							Reason);
#endif
#ifdef MAP_R2
					if (IS_ENTRY_CLIENT(pMacEntry) && IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
						wapp_handle_sta_disassoc(pAd, pMacEntry->wcid, Reason);
#endif
					MTWF_LOG(DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN, ("Send DeAuth (Reason=%d) to "MACSTR"\n",
							 Reason, MAC2STR(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
									 pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.if_addr,
									 pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof(HEADER_802_11), &DeAuthHdr,
									  2, &Reason,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					MlmeFreeMemory(pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MAC_TABLE_ENTRY **Hash = NULL;
		MAC_TABLE_ENTRY *Content = NULL;
		STA_TR_ENTRY *tr_entry = NULL;

		os_alloc_mem(NULL, (UCHAR **)&Hash, sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		if (!Hash) {
			ASSERT(0);
			return;/*ALPS05330059*/
		}
		os_alloc_mem(NULL, (UCHAR **)&Content, sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		if (!Content) {
			ASSERT(0);
			os_free_mem(Hash);
			return;/*ALPS05330298*/
		}
		os_alloc_mem(NULL, (UCHAR **)&tr_entry, sizeof(STA_TR_ENTRY)*MAX_LEN_OF_TR_TABLE);
		if (!tr_entry) {
			ASSERT(0);
			os_free_mem(Hash);
			os_free_mem(Content);
			return;/*ALPS05330350*/
		}
		if (!IS_WCID_VALID(pAd, GET_MAX_UCAST_NUM(pAd))) {
			ASSERT(0);
			os_free_mem(Hash);
			os_free_mem(Content);
			os_free_mem(tr_entry);
			return;
		}

		NdisZeroMemory(&Hash[0], sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		NdisZeroMemory(&Content[0], sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		NdisZeroMemory(&tr_entry[0], sizeof(STA_TR_ENTRY) * MAX_LEN_OF_TR_TABLE);

		/* MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE]; */

		for (apidx = MAIN_MBSSID; apidx < pAd->ApCfg.BssidNum; apidx++) {
#ifdef WSC_AP_SUPPORT
			BOOLEAN Cancelled;

			RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EapolTimer, &Cancelled);
			pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EapolTimerRunning = FALSE;
			NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EntryAddr, MAC_ADDR_LEN);
			pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EapMsgRunning = FALSE;
#endif /* WSC_AP_SUPPORT */
			pAd->ApCfg.MBSSID[apidx].StaCount = 0;
		}

		os_zero_mem(&(pAd->bss_group), sizeof(struct bss_group_rec));
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number);

		if (pAd->MacTab.McastPsQueue.Number > 0)
			APCleanupPsQueue(pAd, &pAd->MacTab.McastPsQueue);

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "2McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number);
		/* ENTRY PREEMPTION: Zero Mac Table but entry's content */
		/* NdisZeroMemory(&pAd->MacTab.Size, sizeof(MAC_TABLE)-offsetof(MAC_TABLE, Size)); */
		NdisCopyMemory(&Hash[0], pAd->MacTab.Hash, sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		NdisCopyMemory(&Content[0], pAd->MacTab.Content, sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		NdisCopyMemory(&tr_entry[0], tr_ctl->tr_entry, sizeof(STA_TR_ENTRY) * MAX_LEN_OF_TR_TABLE);
		NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
		NdisCopyMemory(pAd->MacTab.Hash, &Hash[0], sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		NdisCopyMemory(pAd->MacTab.Content, &Content[0], sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		NdisCopyMemory(tr_ctl->tr_entry, &tr_entry[0], sizeof(STA_TR_ENTRY) * MAX_LEN_OF_TR_TABLE);
		os_free_mem(Hash);
		os_free_mem(Content);
		os_free_mem(tr_entry);
		InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
		/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}

static VOID SetHtVhtForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef DOT11_N_SUPPORT
	SetCommonHT(pAd, wdev);
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode))
		SetCommonVHT(pAd, wdev);

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
}

INT	SetCommonHtVht(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	if (wdev)
		SetHtVhtForWdev(pAd, wdev);
	else
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "Can't update HT/VHT due to wdev is null!\n");

	return TRUE;
}

inline SCAN_CTRL *get_scan_ctrl_by_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR BandIdx = DBDC_BAND0;

	if (wdev) {
		if (wdev->pHObj) {
			BandIdx = HcGetBandByWdev(wdev);
			return &pAd->ScanCtrl[BandIdx];
		}
		ASSERT(wdev->pHObj);
	} else
		ASSERT(wdev);/*ALPS05330224*/

	return &pAd->ScanCtrl[BandIdx];
}

inline PBSS_TABLE get_scan_tab_by_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	return &ScanCtrl->ScanTab;
}

#ifdef CONFIG_STA_SUPPORT

inline PSTA_ADMIN_CONFIG GetStaCfgByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	if (pwdev && pwdev->func_dev) {
		if (pwdev->wdev_type == WDEV_TYPE_STA) {
			return (struct _STA_ADMIN_CONFIG *)pwdev->func_dev;
		}
#ifdef MAC_REPEATER_SUPPORT
		else if (pwdev->wdev_type == WDEV_TYPE_REPEATER) {
			REPEATER_CLIENT_ENTRY *rept_cli = (struct _REPEATER_CLIENT_ENTRY *)pwdev->func_dev;
			return (struct _STA_ADMIN_CONFIG *)rept_cli->main_wdev->func_dev;
		}
#endif /* MAC_REPEATER_SUPPORT */
	}
	return NULL;
}



MAC_TABLE_ENTRY *GetAssociatedAPByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	PSTA_ADMIN_CONFIG pStafCfg;

	if (pwdev == NULL)
		return NULL;

	pStafCfg = GetStaCfgByWdev(pAd, pwdev);

	if (pStafCfg == NULL) {
		MTWF_LOG(DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN,
				("%s(): Avoid calling this function when wdev type is not client!\n", __func__));
		return NULL;
	}

	return (MAC_TABLE_ENTRY *)pStafCfg->pAssociatedAPEntry;
}

VOID sta_mac_entry_lookup(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, MAC_TABLE_ENTRY **entry)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	if (wdev) {
		while (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr) && (pEntry->wdev == wdev))
				break;
			else
				pEntry = pEntry->pNext;
		}
	} else {

		while (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
				break;
			else
				pEntry = pEntry->pNext;
		}
	}

	*entry = pEntry;
}
#endif

VOID mac_entry_lookup(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, MAC_TABLE_ENTRY **entry)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry && !IS_ENTRY_NONE(pEntry)) {
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
			break;
		else
			pEntry = pEntry->pNext;
	}

	*entry = pEntry;
}

/*
* Delete MacTableEntry and equeue to cmd thread
*/
VOID mac_entry_delete(struct _RTMP_ADAPTER	*ad, struct _MAC_TABLE_ENTRY *entry)
{
	UCHAR buf[32] = {0};

	*((UINT16 *)buf) = entry->wcid;
	memcpy(&buf[2], entry->Addr, 6);
	RTEnqueueInternalCmd(ad, CMDTHRED_MAC_TABLE_DEL, (VOID *) buf, sizeof(buf));
}

void entrytb_aid_bitmap_init(struct _RTMP_CHIP_CAP *cap, struct _aid_info *aid_info)
{
	UINT32 *aid_bitmap = NULL;

	UINT32 map_size = 0;

	/*allocate up to can contain MAX_VALID_AID*/
	map_size = ((INVALID_AID + 31) / 32) * sizeof(UINT32);
	os_alloc_mem(NULL, (UCHAR **)&aid_bitmap, map_size);
	if (aid_bitmap == NULL) {
		dump_stack();
		MTWF_DBG(NULL, DBG_CAT_ALL, CATMLME_WTBL, DBG_LVL_ERROR,
				 "Allocate memory size:%d for aid_bitmap failed!\n",
				  map_size);
		return;
	}

	aid_info->aid_bitmap = aid_bitmap;
	if (aid_info->aid_bitmap == NULL) {
		dump_stack();
		MTWF_DBG(NULL, DBG_CAT_ALL, CATMLME_WTBL, DBG_LVL_ERROR,
				 "Allocate memory size:%d for aid_info->aid_bitmap failed!\n",
				  map_size);
		return;
	}

	os_zero_mem(aid_bitmap, map_size);
	aid_info->max_aid = INVALID_AID - 1;
}

void entrytb_aid_bitmap_free(struct _aid_info *aid_info)
{
	UINT32 *aid_bitmap = aid_info->aid_bitmap;

	if (aid_bitmap)
		os_free_mem(aid_bitmap);

	aid_info->aid_bitmap = NULL;
}

void entrytb_aid_bitmap_reserve(
	struct _aid_info *aid_info,
	UINT16 aid_order_reserved)
{
	/*aid bitmap needs to consider the amounts of the non-transmitted bss of 11V mbss*/
	aid_info->aid_allocate_from_idx = (1 << aid_order_reserved);
}

UINT16 entrytb_aid_aquire(struct _aid_info *aid_info)
{
	UINT16 aid;

	for (aid = aid_info->aid_allocate_from_idx; aid <= aid_info->max_aid; aid++) {
		if (!aid_is_assigned(aid_info, aid)) {
			aid_set(aid_info, aid);
			MTWF_LOG(DBG_CAT_ALL, CATMLME_WTBL, DBG_LVL_WARN,
					 ("%s(): found non-occupied aid:%d, allocated from:%d\n",
					  __func__, aid, aid_info->aid_allocate_from_idx));
			break;
		}
	}

	if (aid > aid_info->max_aid)
		aid = INVALID_AID;

	return aid;
}

INT show_aid_info(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	struct _aid_info *aid_info = &ad->MacTab.aid_info;
	UINT32 aid;

	if (arg && strlen(arg)) {
		aid = simple_strtol(arg, NULL, 10);
		if ((aid == 0) || (aid >= INVALID_AID)) {
			MTWF_DBG(ad, DBG_CAT_CFG, CATMLME_WTBL, DBG_LVL_ERROR,
				"wrong AID input\n");
			return FALSE;
		}
	}

	aid_dump(aid_info);

	return TRUE;
}

UINT32 traversal_func_find_entry_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie)
{
	UINT32 result = FALSE;
	entrytb_aid_search_t *aid_map = (entrytb_aid_search_t *)cookie;

	if (aid_map->aid_search == entry->Aid) {
		aid_map->entry = entry;
		result = TRUE;
	}

	return result;
}

UINT32 traversal_func_dump_entry_associated_to_bss(struct _MAC_TABLE_ENTRY *entry, void *cookie)
{
	UINT32 result = FALSE;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)entry->pAd;
	entrytb_bss_idx_search_t *check_bss = (entrytb_bss_idx_search_t *)cookie;
	ULONG DataRate = 0;
	ULONG DataRate_r = 0;
	UCHAR	tmp_str[30];
	INT	temp_str_len = sizeof(tmp_str);
	int ret;
	UINT tmp_str_left = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
#endif

	if (check_bss->need_print_field_name) {
		MTWF_PRINT("Dump the entries associated to BssIDX:%d\n", check_bss->bss_idx);
#ifdef CONFIG_HOTSPOT_R2
		MTWF_PRINT("\n%-19s%-7s%-4s%-4s%-20s%-12s%-9s%-12s%-9s%-10s%-10s\n",
			"MAC", "WCID", "BSS", "PSM",
			"RSSI0/1/2/3", "PhMd", "BW", "MCS", "SGI",
			"STBC", "Rate");
#else
		MTWF_PRINT("\n%-19s%-7s%-4s%-4s%-20s%-12s%-9s%-12s%-9s%-10s%-10s\n",
			"MAC", "WCID", "BSS", "PSM",
			"RSSI0/1/2/3", "PhMd(T/R)", "BW(T/R)", "MCS(T/R)", "SGI(T/R)",
			"STBC(T/R)", "Rate(T/R)");
#endif /* CONFIG_HOTSPOT_R2 */
		check_bss->need_print_field_name = 0;
	}

	if (check_bss->bss_idx == entry->func_tb_idx) {
		DataRate = 0;
		getRate(entry->HTPhyMode, &DataRate);
		MTWF_PRINT(MACSTR, MAC2STR(entry->Addr));
		MTWF_PRINT("%-7d", (int)entry->wcid);
		MTWF_PRINT("%-4d", (int)entry->func_tb_idx);
		MTWF_PRINT("%-4d", (int)entry->PsMode);
		ret = snprintf(tmp_str, temp_str_len, "%d/%d/%d/%d", entry->RssiSample.AvgRssi[0],
				entry->RssiSample.AvgRssi[1],
				entry->RssiSample.AvgRssi[2],
				entry->RssiSample.AvgRssi[3]);
		if (os_snprintf_error(temp_str_len, ret)) {
			MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
			return FALSE;
		}
		MTWF_PRINT("%-20s", tmp_str);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (cap->fgRateAdaptFWOffload == TRUE && (entry->bAutoTxRateSwitch == TRUE)) {
			UCHAR phy_mode, rate, bw, sgi, stbc;
			UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
#ifdef DOT11_VHT_AC
			UCHAR vht_nss;
			UCHAR vht_nss_r;
#endif
			UINT32 RawData;
			UINT32 RawData_r;
			UINT32 lastTxRate;
			UINT32 lastRxRate = entry->LastRxRate;

			if (entry->bAutoTxRateSwitch == TRUE) {
				EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
				HTTRANSMIT_SETTING LastTxRate;
				HTTRANSMIT_SETTING LastRxRate;

				os_zero_mem(&rTxStatResult, sizeof(rTxStatResult));
				MtCmdGetTxStatistic(ad,
						    GET_TX_STAT_ENTRY_TX_RATE,
						    0/*Don't Care*/,
						    entry->wcid,
						    &rTxStatResult);
				LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
				LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
				LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
				LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
				LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

				if (LastTxRate.field.MODE >= MODE_VHT)
					LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) +
								rTxStatResult.rEntryTxRate.MCS;
				else if (LastTxRate.field.MODE == MODE_OFDM)
					LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) &
								0x0000003F;
				else
					LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

				lastTxRate = (UINT32)(LastTxRate.word);
				LastRxRate.word = (USHORT)lastRxRate;
				RawData = lastTxRate;
				phy_mode = (RawData >> 13) & 0x7;
				rate = RawData & 0x3F;
				bw = (RawData >> 7) & 0x3;
				sgi = (RawData >> 9) & 0x1;
				stbc = ((RawData >> 10) & 0x1);
				/* ---- */
				RawData_r = lastRxRate;
				phy_mode_r = (RawData_r >> 13) & 0x7;
				rate_r = RawData_r & 0x3F;
				bw_r = (RawData_r >> 7) & 0x3;
				sgi_r = (RawData_r >> 9) & 0x1;
				stbc_r = ((RawData_r >> 10) & 0x1);
				ret = snprintf(tmp_str,
					 temp_str_len,
					 "%s/%s",
					 get_phymode_str(phy_mode),
					 get_phymode_str(phy_mode_r));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-12s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%s/%s",
					get_bw_str(bw), get_bw_str(bw_r));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-9s", tmp_str);
#ifdef DOT11_VHT_AC

				if (phy_mode >= MODE_VHT) {
					vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
					rate = rate & 0xF;
					ret = snprintf(tmp_str, temp_str_len, "%dS-M%d/", vht_nss, rate);
					if (os_snprintf_error(temp_str_len, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif /* DOT11_VHT_AC */
				{
					ret = snprintf(tmp_str, temp_str_len, "%d/", rate);
					if (os_snprintf_error(temp_str_len, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}
#ifdef DOT11_VHT_AC

				if (phy_mode_r >= MODE_VHT) {
					vht_nss_r = ((rate_r & (0x3 << 4)) >> 4) + 1;
					rate_r = rate_r & 0xF;
					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						tmp_str_left,
						"%dS-M%d",
						vht_nss_r,
						rate_r);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
				if (phy_mode_r >= MODE_HTMIX) {
					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 rate_r);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}
				else
#endif
				if (phy_mode_r == MODE_OFDM) {
					if (rate_r == TMI_TX_RATE_OFDM_6M)
						LastRxRate.field.MCS = 0;
					else if (rate_r == TMI_TX_RATE_OFDM_9M)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_OFDM_12M)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_OFDM_18M)
						LastRxRate.field.MCS = 3;
					else if (rate_r == TMI_TX_RATE_OFDM_24M)
						LastRxRate.field.MCS = 4;
					else if (rate_r == TMI_TX_RATE_OFDM_36M)
						LastRxRate.field.MCS = 5;
					else if (rate_r == TMI_TX_RATE_OFDM_48M)
						LastRxRate.field.MCS = 6;
					else if (rate_r == TMI_TX_RATE_OFDM_54M)
						LastRxRate.field.MCS = 7;
					else
						LastRxRate.field.MCS = 0;

					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 LastRxRate.field.MCS);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else if (phy_mode_r == MODE_CCK) {
					if (rate_r == TMI_TX_RATE_CCK_1M_LP)
						LastRxRate.field.MCS = 0;
					else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
						LastRxRate.field.MCS = 3;
					else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
						LastRxRate.field.MCS = 3;
					else
						LastRxRate.field.MCS = 0;

					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 LastRxRate.field.MCS);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}

				MTWF_PRINT("%-12s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%d/%d", sgi, sgi_r);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-9s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%d/%d",  stbc, stbc_r);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-10s", tmp_str);
				getRate(LastTxRate, &DataRate);
				getRate(LastRxRate, &DataRate_r);
			}
		} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		{
			MTWF_PRINT("%-12s", get_phymode_str(entry->HTPhyMode.field.MODE));
			MTWF_PRINT("%-9s", get_bw_str(entry->HTPhyMode.field.BW));
#ifdef DOT11_VHT_AC

			if (entry->HTPhyMode.field.MODE >= MODE_VHT) {
				ret = snprintf(tmp_str, temp_str_len, "%dS-M%d", ((entry->HTPhyMode.field.MCS >> 4) + 1),
						(entry->HTPhyMode.field.MCS & 0xf));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
			} else
#endif /* DOT11_VHT_AC */
			{
				ret = snprintf(tmp_str, temp_str_len, "%d", entry->HTPhyMode.field.MCS);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
			}

			MTWF_PRINT("%-12s", tmp_str);
			MTWF_PRINT("%-9d", entry->HTPhyMode.field.ShortGI);
			MTWF_PRINT("%-10d", entry->HTPhyMode.field.STBC);
		}

		ret = snprintf(tmp_str, temp_str_len, "%d/%d", (int)DataRate, (int)DataRate_r);
		if (os_snprintf_error(temp_str_len, ret)) {
			MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
			return FALSE;
		}
		MTWF_PRINT("%-10s", tmp_str);
		MTWF_PRINT("\n");
	}

	return result;
}

UINT32 traversal_func_dump_entry_psm_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie)
{
	UINT32 result = FALSE;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)entry->pAd;
	UINT32 *aid = (UINT32 *)cookie;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);
	ULONG now;

	NdisGetSystemUpTime(&now);

	if (*aid == entry->Aid) {
		MTWF_PRINT("dump PSM info for AID:%d\n", entry->Aid);
		MTWF_PRINT("\n%-19s\t%s\t%s\t%s\t%-9s\t%s\n",
			"MAC", "WCID", "BSS", "PSM", "NoRxData", "SLEEP TIME(msec)");

		MTWF_PRINT(MACSTR, MAC2STR(entry->Addr));
		MTWF_PRINT("\t%d", (int)entry->wcid);
		MTWF_PRINT("\t%d", (int)entry->func_tb_idx);
		MTWF_PRINT("\t%d", (int)entry->PsMode);
		MTWF_PRINT("\t%-9d", (int)entry->NoDataIdleCount);
		MTWF_PRINT("\t%d\n", entry->PsMode ? jiffies_to_msecs(now - entry->sleep_from) : 0);

		if (chip_dbg->show_ple_info_by_idx)
			chip_dbg->show_ple_info_by_idx(ad->hdev_ctrl, entry->wcid);

		result = TRUE;
	}

	return result;

}

UINT32 traversal_func_dump_entry_rate_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie)
{
	UINT32 result = FALSE;
	UINT32 *aid = (UINT32 *)cookie;
	ULONG DataRate = 0;
	ULONG DataRate_r = 0;
	UCHAR	tmp_str[30];
	INT		temp_str_len = sizeof(tmp_str);
	int ret;
	UINT tmp_str_left = 0;

	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)entry->pAd;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
#endif

	if (*aid == entry->Aid) {
		MTWF_PRINT("dump rate info for AID:%d\n", entry->Aid);
#ifdef CONFIG_HOTSPOT_R2
		MTWF_PRINT("\n%-19s%-7s%-4s%-4s%-20s%-12s%-9s%-12s%-9s%-10s%-10s\n",
			"MAC", "WCID", "BSS", "PSM",
			"RSSI0/1/2/3", "PhMd", "BW", "MCS", "SGI",
			"STBC", "Rate");
#else
		MTWF_PRINT("\n%-19s%-7s%-4s%-4s%-20s%-12s%-9s%-12s%-9s%-10s%-10s\n",
			"MAC", "WCID", "BSS", "PSM",
			"RSSI0/1/2/3", "PhMd(T/R)", "BW(T/R)", "MCS(T/R)", "SGI(T/R)",
			"STBC(T/R)", "Rate(T/R)");
#endif /* CONFIG_HOTSPOT_R2 */

		DataRate = 0;
		getRate(entry->HTPhyMode, &DataRate);
		MTWF_PRINT(MACSTR, MAC2STR(entry->Addr));
		MTWF_PRINT("%-7d", (int)entry->wcid);
		MTWF_PRINT("%-4d", (int)entry->func_tb_idx);
		MTWF_PRINT("%-4d", (int)entry->PsMode);
		ret = snprintf(tmp_str, temp_str_len, "%d/%d/%d/%d", entry->RssiSample.AvgRssi[0],
				entry->RssiSample.AvgRssi[1],
				entry->RssiSample.AvgRssi[2],
				entry->RssiSample.AvgRssi[3]);
		if (os_snprintf_error(temp_str_len, ret)) {
			MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
			return FALSE;
		}
		MTWF_PRINT("%-20s", tmp_str);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (cap->fgRateAdaptFWOffload == TRUE && (entry->bAutoTxRateSwitch == TRUE)) {
			UCHAR phy_mode, rate, bw, sgi, stbc;
			UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
#ifdef DOT11_VHT_AC
			UCHAR vht_nss;
			UCHAR vht_nss_r;
#endif
			UINT32 RawData;
			UINT32 RawData_r;
			UINT32 lastTxRate;
			UINT32 lastRxRate = entry->LastRxRate;

			if (entry->bAutoTxRateSwitch == TRUE) {
				EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
				HTTRANSMIT_SETTING LastTxRate;
				HTTRANSMIT_SETTING LastRxRate;

				os_zero_mem(&rTxStatResult, sizeof(rTxStatResult));
				MtCmdGetTxStatistic(ad,
						    GET_TX_STAT_ENTRY_TX_RATE,
						    0/*Don't Care*/,
						    entry->wcid,
						    &rTxStatResult);
				LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
				LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
				LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
				LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
				LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

				if (LastTxRate.field.MODE >= MODE_VHT)
					LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) +
								rTxStatResult.rEntryTxRate.MCS;
				else if (LastTxRate.field.MODE == MODE_OFDM)
					LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) &
								0x0000003F;
				else
					LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

				lastTxRate = (UINT32)(LastTxRate.word);
				LastRxRate.word = (USHORT)lastRxRate;
				RawData = lastTxRate;
				phy_mode = (RawData >> 13) & 0x7;
				rate = RawData & 0x3F;
				bw = (RawData >> 7) & 0x3;
				sgi = (RawData >> 9) & 0x1;
				stbc = ((RawData >> 10) & 0x1);
				/* ---- */
				RawData_r = lastRxRate;
				phy_mode_r = (RawData_r >> 13) & 0x7;
				rate_r = RawData_r & 0x3F;
				bw_r = (RawData_r >> 7) & 0x3;
				sgi_r = (RawData_r >> 9) & 0x1;
				stbc_r = ((RawData_r >> 10) & 0x1);
				ret = snprintf(tmp_str,
					 temp_str_len,
					 "%s/%s",
					 get_phymode_str(phy_mode),
					 get_phymode_str(phy_mode_r));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-12s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%s/%s", get_bw_str(bw), get_bw_str(bw_r));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-9s", tmp_str);
#ifdef DOT11_VHT_AC

				if (phy_mode >= MODE_VHT) {
					vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
					rate = rate & 0xF;
					ret = snprintf(tmp_str, temp_str_len, "%dS-M%d/", vht_nss, rate);
					if (os_snprintf_error(temp_str_len, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif /* DOT11_VHT_AC */
				{
					ret = snprintf(tmp_str, temp_str_len, "%d/", rate);
					if (os_snprintf_error(temp_str_len, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}
#ifdef DOT11_VHT_AC

				if (phy_mode_r >= MODE_VHT) {
					vht_nss_r = ((rate_r & (0x3 << 4)) >> 4) + 1;
					rate_r = rate_r & 0xF;
					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						tmp_str_left,
						"%dS-M%d",
						vht_nss_r,
						rate_r);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
				if (phy_mode_r >= MODE_HTMIX) {
					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 rate_r);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif
				if (phy_mode_r == MODE_OFDM) {
					if (rate_r == TMI_TX_RATE_OFDM_6M)
						LastRxRate.field.MCS = 0;
					else if (rate_r == TMI_TX_RATE_OFDM_9M)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_OFDM_12M)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_OFDM_18M)
						LastRxRate.field.MCS = 3;
					else if (rate_r == TMI_TX_RATE_OFDM_24M)
						LastRxRate.field.MCS = 4;
					else if (rate_r == TMI_TX_RATE_OFDM_36M)
						LastRxRate.field.MCS = 5;
					else if (rate_r == TMI_TX_RATE_OFDM_48M)
						LastRxRate.field.MCS = 6;
					else if (rate_r == TMI_TX_RATE_OFDM_54M)
						LastRxRate.field.MCS = 7;
					else
						LastRxRate.field.MCS = 0;

					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 LastRxRate.field.MCS);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else if (phy_mode_r == MODE_CCK) {
					if (rate_r == TMI_TX_RATE_CCK_1M_LP)
						LastRxRate.field.MCS = 0;
					else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
						LastRxRate.field.MCS = 3;
					else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
						LastRxRate.field.MCS = 3;
					else
						LastRxRate.field.MCS = 0;

					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 LastRxRate.field.MCS);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}

				MTWF_PRINT("%-12s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%d/%d", sgi, sgi_r);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-9s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%d/%d",  stbc, stbc_r);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-10s", tmp_str);
				getRate(LastTxRate, &DataRate);
				getRate(LastRxRate, &DataRate_r);
			}
		} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		{
			MTWF_PRINT("%-12s", get_phymode_str(entry->HTPhyMode.field.MODE));
			MTWF_PRINT("%-9s", get_bw_str(entry->HTPhyMode.field.BW));
#ifdef DOT11_VHT_AC

			if (entry->HTPhyMode.field.MODE >= MODE_VHT) {
				ret = snprintf(tmp_str, temp_str_len, "%dS-M%d", ((entry->HTPhyMode.field.MCS >> 4) + 1),
						(entry->HTPhyMode.field.MCS & 0xf));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
			} else
#endif /* DOT11_VHT_AC */
			{
				ret = snprintf(tmp_str, temp_str_len, "%d", entry->HTPhyMode.field.MCS);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
			}

			MTWF_PRINT("%-12s", tmp_str);
			MTWF_PRINT("%-9d", entry->HTPhyMode.field.ShortGI);
			MTWF_PRINT("%-10d", entry->HTPhyMode.field.STBC);
		}

		ret = snprintf(tmp_str, temp_str_len, "%d/%d", (int)DataRate, (int)DataRate_r);
		if (os_snprintf_error(temp_str_len, ret)) {
			MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
			return FALSE;
		}
		MTWF_PRINT("%-10s", tmp_str);
		MTWF_PRINT("\n");

		result = TRUE;
	}

	return result;
}

UINT32 entrytb_traversal(struct _RTMP_ADAPTER *ad, entrytb_traversal_func func, void *cookie)
{
	UINT32 i;
	UINT32 result = FALSE;

	MAC_TABLE_ENTRY *entry = NULL;

	for (i = 0; VALID_UCAST_ENTRY_WCID(ad, i); i++) {
		entry = &ad->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(entry))
			continue;

		result = func(entry, cookie);

		/**
		 * for some specific purpose traversal func,
		 * found the matched parameter and done the corresponding process.
		 * skip the remain entries here.
		 */
		if (result == TRUE)
			break;
	}

	return result;
}


