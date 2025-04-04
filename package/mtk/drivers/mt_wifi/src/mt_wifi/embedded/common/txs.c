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
	txs.c
*/

/**
 * @addtogroup tx_rx_path Wi-Fi
 * @{
 * @name TxS Control API
 * @{
 */

#include	"rt_config.h"

/**** TxS Call Back Functions ****/
#ifdef CFG_TDLS_SUPPORT
INT32 TdlsTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
	TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
	TXS_D_4 *TxSD4 = &txs_entry->TxSD4;

	pEntry = &pAd->MacTab.Content[TxSD3->TxS_WlanIdx];
	MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "txs d0 me : %d\n", TxSD0->ME);

	if (TxSD0->ME == 0)
		pEntry->TdlsTxFailCount = 0;
	else
		pEntry->TdlsTxFailCount++;

	if (pEntry->TdlsTxFailCount > 15) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "TdlsTxFailCount > 15!!  teardown link with ("MACSTR")!!\n"
				 , MAC2STR(pEntry->Addr));
		pEntry->TdlsTxFailCount = 0;
		cfg_tdls_auto_teardown(pAd, pEntry);
	}
}
#endif /*CFG_TDLS_SUPPORT*/


INT32 ActionTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}

INT32 BcnTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}

INT32 PsDataTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}


/**** End of TxS Call Back Functions ****/


INT32 InitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index, Index1;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;

	MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, " ");

	/* Per Pkt */
	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++) {
		NdisAllocateSpinLock(pAd, &TxSCtl->TxSTypePerPktLock[Index]);
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		DlListInit(&TxSCtl->TxSTypePerPkt[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
	}

	/* Per Pkt Type */
	for (Index = 0; Index < 3; Index++) {
		for (Index1 = 0; Index1 < TOTAL_PID_HASH_NUMS_PER_PKT_TYPE; Index1++) {
			NdisAllocateSpinLock(pAd, &TxSCtl->TxSTypePerPktTypeLock[Index][Index1]);
			RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			DlListInit(&TxSCtl->TxSTypePerPktType[Index][Index1]);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
		}
	}

	for (Index = 0; Index < TXS_STATUS_NUM; Index++)
		NdisZeroMemory(&TxSCtl->TxSStatus[Index], sizeof(TXS_STATUS));

	return 0;
}

/*7636 psm*/
INT32 NullFrameTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
#if defined(MT7636) && defined(CONFIG_STA_SUPPORT)
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
	TXS_D_4 *TxSD4 = &txs_entry->TxSD4;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s, line(%d)\n", __func__, __LINE__);

	if ((pAd->OpMode == OPMODE_STA) && (pAd->StaCfg[0].BssType == BSS_INFRA) && (pAd->StaCfg[0].WindowsPowerMode != Ndis802_11PowerModeCAM)) {
		if (TxSD4->TxS_Pid == PID_NULL_FRAME_PWR_SAVE) {
			if ((TxSD0->LE == 0) && (TxSD0->RE == 0) && (TxSD0->ME == 0)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Got TXS, RTMPSendNullFrame(PM=1)\n");
				/*7636 psm*/
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(%d)::Enter RTMPOffloadPm(pAd, 0x04, 1);\n", __func__, __LINE__);
				/*In 7636, power saving mechanism is offlaoded to F/W and doesn't need the last argument*/
				RTEnqueueInternalCmd(pAd, HWCMD_ID_FORCE_SLEEP_AUTO_WAKEUP, NULL, 0);
			} else
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Got TXS, ERROR, Peer didn't get NullFrame(PM=1)\n");
		}

		if (TxSD4->TxS_Pid == PID_NULL_FRAME_PWR_ACTIVE)
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Got TXS, RTMPSendNullFrame(PM=0)\n");
	}

#endif /*MT7636*/
	return 0;
}


INT32 InitTxSCommonCallBack(RTMP_ADAPTER *pAd)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}


INT32 ExitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index, Index1;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++) {
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPkt[Index],
						  TXS_TYPE, List) {
			DlListDel(&TxSType->List);
			os_free_mem(TxSType);
		}
		DlListInit(&TxSCtl->TxSTypePerPkt[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		NdisFreeSpinLock(&TxSCtl->TxSTypePerPktLock[Index]);
	}

	for (Index = 0; Index < 3; Index++) {
		for (Index1 = 0; Index1 < TOTAL_PID_HASH_NUMS_PER_PKT_TYPE; Index1++) {
			RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPktType[Index][Index1],
							  TXS_TYPE, List) {
				DlListDel(&TxSType->List);
				os_free_mem(TxSType);
			}
			DlListInit(&TxSCtl->TxSTypePerPktType[Index][Index1]);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			NdisFreeSpinLock(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1]);
		}
	}

	return 0;
}


INT32 AddTxSTypePerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format,
					   TXS_HANDLER TxSHandler)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}


INT32 RemoveTxSTypePerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS],
					  TXS_TYPE, List) {
		if ((TxSType->PktPid == PktPid) && (TxSType->Format == Format)) {
			DlListDel(&TxSType->List);
			os_free_mem(TxSType);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS],
										&Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	return -1;
}


INT32 TxSTypeCtlPerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format, BOOLEAN TxS2Mcu,
					   BOOLEAN TxS2Host, BOOLEAN DumpTxSReport, ULONG DumpTxSReportTimes)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List) {
		if ((TxSType->PktPid == PktPid) && (TxSType->Format == Format)) {
			if (TxS2Mcu)
				TxSCtl->TxS2McUStatusPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxS2McUStatusPerPkt &= ~(1 << PktPid);

			if (TxS2Host)
				TxSCtl->TxS2HostStatusPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxS2HostStatusPerPkt &= ~(1 << PktPid);

			if (Format == TXS_FORMAT1)
				TxSCtl->TxSFormatPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxSFormatPerPkt &= ~(1 << PktPid);

			TxSType->DumpTxSReport = DumpTxSReport;
			TxSType->DumpTxSReportTimes = DumpTxSReportTimes;
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS],
										&Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "can not find TxSType(PktPID = %d, Format = %d)\n",
			  PktPid, Format);
	return -1;
}


INT32 AddTxSTypePerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType,
						   UINT8 Format, TXS_HANDLER TxSHandler)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *SearchTxSType = NULL;

	os_alloc_mem(NULL, (PUCHAR *)&TxSType, sizeof(*TxSType));

	if (!TxSType) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "can not allocate TxS Type\n");
		return -1;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEach(SearchTxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) {
		if ((SearchTxSType->PktType == PktType) && (SearchTxSType->PktSubType == PktSubType)
			&& (SearchTxSType->Format == Format)) {
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "already registered TxSType (PktType = %d, PktSubType = %d, Format = %d\n", PktType, PktSubType, Format);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			os_free_mem(TxSType);
			return -1;
		}
	}
	TxSType->Type = TXS_TYPE1;
	TxSType->PktType = PktType;
	TxSType->PktSubType = PktSubType;
	TxSType->Format = Format;
	TxSType->TxSHandler = TxSHandler;
	DlListAddTail(&TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &TxSType->List);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	return 0;
}


INT32 RemoveTxSTypePerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType,
							  UINT8 Format)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) {
		if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
			&& (TxSType->Format == Format)) {
			DlListDel(&TxSType->List);
			os_free_mem(TxSType);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	return -1;
}


INT32 TxSTypeCtlPerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType, UINT16 WlanIdx,
						   UINT8 Format, BOOLEAN TxS2Mcu, BOOLEAN TxS2Host, BOOLEAN DumpTxSReport,
						   ULONG DumpTxSReportTimes)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) {
		if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
			&& (TxSType->Format == Format)) {
			/*register the TYPE/SUB_TYPE need to report to MCU.*/
			if (TxS2Mcu)
				TxSCtl->TxS2McUStatusPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxS2McUStatusPerPktType[PktType] &= ~(1 << PktSubType);

			/*register the TYPE/SUB_TYPE need to report to HOST.*/
			if (TxS2Host)
				TxSCtl->TxS2HostStatusPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxS2HostStatusPerPktType[PktType] &= ~(1 << PktSubType);

			/*register the TXS report type*/
			if (Format == TXS_FORMAT1)
				TxSCtl->TxSFormatPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxSFormatPerPktType[PktType] &= ~(1 << PktSubType);

			/*indicate which widx might be used for send the kinw of type/subtype pkt.*/
			if (WlanIdx < 64)
				TxSCtl->TxSStatusPerWlanIdx[0] |= (1ULL << (UINT64)WlanIdx);
			else if (WlanIdx >= 64 && WlanIdx < 128) {
				WlanIdx -= 64;
				TxSCtl->TxSStatusPerWlanIdx[1] |= (1ULL << (UINT64)WlanIdx);
			} else {
				TxSCtl->TxSStatusPerWlanIdx[0] = 0xffffffffffffffff;
				TxSCtl->TxSStatusPerWlanIdx[1] = 0xffffffffffffffff;
			}

			TxSType->DumpTxSReport = DumpTxSReport;
			TxSType->DumpTxSReportTimes = DumpTxSReportTimes;
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "can not find TxSType(PktType = %d, PktSubType = %d, Format = %d)\n",
			  PktType, PktSubType, Format);
	return -1;
}


INT32 ParseTxSPacket_v2(RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
#ifndef AUTOMATION
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
#endif
#if (defined(WH_EZ_SETUP) || defined(DPP_SUPPORT))
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
	BOOLEAN TxError = (TxSD0->ME || TxSD0->RE || TxSD0->LE || TxSD0->BE || TxSD0->TxOp || TxSD0->PSBit || TxSD0->BAFail);
#endif

	if (Format == TXS_FORMAT0) {

#ifdef DPP_SUPPORT
		if (Pid == PID_MGMT_DPP_FRAME) {
			MAC_TABLE_ENTRY	*pEntry = NULL;
			struct wifi_dev *wdev = NULL;

			pEntry = &pAd->MacTab.Content[TxSD2->TxS_WlanIdx];
			wdev = pEntry->wdev;
			wext_send_dpp_frame_tx_status(pAd, wdev, TxError, TxSD3->type_0.TxS_SN);
		}
#endif /* DPP_SUPPORT */

#ifdef AUTOMATION
		if (is_frame_test(pAd, 1) != 0) {
			TXS_D_2 *TxSD2_ = &txs_entry->TxSD2;
			TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
			TXS_D_4 *TxSD4 = &txs_entry->TxSD4;

			pAd->auto_dvt->txs.received_pid = Pid;
			receive_del_txs_queue(TxSD3->type_0.TxS_SN, Pid, TxSD2_->TxS_WlanIdx, TxSD4->type_0.TimeStamp);
		}
#else
		if (TxSD0->ME || TxSD0->RE || TxSD0->LE || TxSD0->BE || TxSD0->TxOp || TxSD0->PSBit || TxSD0->BAFail) {
			asic_dump_txs(pAd, Format, Data);
			return -1;
		}
#endif /* AUTOMATION */

	} else if (Format == TXS_FORMAT1) {
#ifdef CONFIG_ATE
		if (ATE_ON(pAd)) {
			if ((pAd->ATECtrl.txs_enable) && (pAd->ATECtrl.en_log & fATE_LOG_TXSSHOW))
				asic_dump_txs(pAd, Format, Data);
		}
#endif /* CONFIG_ATE */
	}

	return 0;
}


UINT8 AddTxSStatus(RTMP_ADAPTER *pAd, UINT8 Type, UINT8 PktPid, UINT8 PktType,
				   UINT8 PktSubType, UINT16 TxRate, UINT32 Priv)
{
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	INT idx;

	for (idx = 0; idx < TXS_STATUS_NUM; idx++) {
		if (TxSCtl->TxSStatus[idx].State == TXS_UNUSED) {
			TxSCtl->TxSPid = idx;
			TxSCtl->TxSStatus[idx].TxSPid = TxSCtl->TxSPid;
			TxSCtl->TxSStatus[idx].State = TXS_USED;
			TxSCtl->TxSStatus[idx].Type = Type;
			TxSCtl->TxSStatus[idx].PktPid = PktPid;
			TxSCtl->TxSStatus[idx].PktType = PktType;
			TxSCtl->TxSStatus[idx].PktSubType = PktSubType;
			TxSCtl->TxSStatus[idx].TxRate = TxRate;
			TxSCtl->TxSStatus[idx].Priv = Priv;
			break;
		}
	}

	if (idx >= TXS_STATUS_NUM) {
		TxSCtl->TxSFailCount++;
		idx = TXS_STATUS_NUM - 1;
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Cannot get empty TxSPid, use default(%d)\n",
				  idx);
	}

	return idx;
}


INT32 RemoveTxSStatus(RTMP_ADAPTER *pAd, UINT8 TxSPid, UINT8 *Type, UINT8 *PktPid,
					  UINT8 *PktType, UINT8 *PktSubType, UINT16 *TxRate, UINT32 *TxSPriv)
{
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	*Type = TxSCtl->TxSStatus[TxSPid].Type;
	*PktPid = TxSCtl->TxSStatus[TxSPid].PktPid;
	*PktType = TxSCtl->TxSStatus[TxSPid].PktType;
	*PktSubType = TxSCtl->TxSStatus[TxSPid].PktSubType;
	*TxRate = TxSCtl->TxSStatus[TxSPid].TxRate;
	*TxSPriv = TxSCtl->TxSStatus[TxSPid].Priv;
	TxSCtl->TxSStatus[TxSPid].State = TXS_UNUSED;
	return 0;
}

/** @} */
/** @} */
