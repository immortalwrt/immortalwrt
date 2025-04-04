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
/****************************************************************************
 ****************************************************************************

    Module Name:
	automation.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "rt_config.h"

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

AUTOMATION_DVT automation_dvt;
static TXS_FREE_LIST_POOL TxsFreeEntrylist;


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
typedef struct GNU_PACKED _FRAME_RTS {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
} FRAME_RTS, *PFRAME_RTS;

/*******************************************************************************
*                              F U N C T I O N S
******************I**************************************************************
*/

static inline VOID TxsPoolInit(VOID)
{
	if (TxsFreeEntrylist.txs_list_cnt == 0) {
		NdisAllocateSpinLock(NULL, &TxsFreeEntrylist.Lock);
		DlListInit(&TxsFreeEntrylist.pool_head.List);
		DlListInit(&TxsFreeEntrylist.head.mList);
	}

	TxsFreeEntrylist.txs_list_cnt++;
}

static inline VOID TxsPoolUnInit(VOID)
{
	TxsFreeEntrylist.txs_list_cnt--;

	if (TxsFreeEntrylist.txs_list_cnt == 0) {
		TXS_LIST_POOL *pEntry = NULL;

		while (!DlListEmpty(&TxsFreeEntrylist.pool_head.List)) {
			pEntry = DlListFirst(&TxsFreeEntrylist.pool_head.List, TXS_LIST_POOL, List);
			DlListDel(&pEntry->List);
			os_free_mem(pEntry);
		}
	}
}

/* Private Function */
static BOOLEAN TxsInit(void)
{
	UINT32 i;
	TXS_LIST *list = &automation_dvt.txs.txs_list;

	if (automation_dvt.txs.init)
		return TRUE;

	automation_dvt.txs.init = FALSE;
	automation_dvt.txs.total_req = 0;
	automation_dvt.txs.total_rsp = 0;
	automation_dvt.txs.stop_send_test = TRUE;
	automation_dvt.txs.test_type = 0;


	NdisAllocateSpinLock(NULL, &list->lock);

	for (i = 0; i < PID_SIZE; i++) {
		DlListInit(&list->pHead[i].mList);
		automation_dvt.txs.check_item[i].time_stamp = 0;
	}

	list->Num = 0;
	TxsPoolInit();

	if (DlListEmpty(&TxsFreeEntrylist.pool_head.List)) {
		TXS_LIST_POOL *Pool = NULL;
		TXS_LIST_POOL *pFreepool = NULL;
		TXS_LIST_ENTRY *pEntry = NULL;
		TXS_LIST_ENTRY *newEntry = NULL;

		os_alloc_mem(NULL, (UCHAR **)&Pool, sizeof(TXS_LIST_POOL));
		pFreepool = &TxsFreeEntrylist.pool_head;
		DlListAdd(&pFreepool->List, &Pool->List);
		pEntry = &TxsFreeEntrylist.head;

		for (i = 0; i < TXS_LIST_ELEM_NUM; i++) {
			newEntry = &Pool->Entry[i];
			DlListAdd(&pEntry->mList, &newEntry->mList);
		}
	}

	list->pFreeEntrylist = &TxsFreeEntrylist;
	automation_dvt.txs.init = TRUE;
	return TRUE;
}

static BOOLEAN TxsExit(void)
{
	UINT32 i = 0;
	ULONG IrqFlags = 0;
	UINT16 wait_cnt = 0;
	TXS_LIST *list = &automation_dvt.txs.txs_list;

	automation_dvt.txs.init = FALSE;
	automation_dvt.txs.total_req = 0;
	automation_dvt.txs.total_rsp = 0;
	automation_dvt.txs.stop_send_test = TRUE;
	automation_dvt.txs.test_type = 0;

	while (automation_dvt.txs.txs_list.Num > 0) {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				 "%s, wait entry to be deleted\n", __func__);
		OS_WAIT(10);
		wait_cnt++;

		if (wait_cnt > 100)
			break;
	}

	OS_INT_LOCK(&list->lock, IrqFlags);

	for (i = 0; i < PID_SIZE; i++) {
		DlListInit(&list->pHead[i].mList);
		automation_dvt.txs.check_item[i].time_stamp = 0;
	}

	OS_INT_UNLOCK(&list->lock, IrqFlags);
	NdisFreeSpinLock(&list->lock);
	list->Num = 0;
	TxsPoolUnInit();
	return TRUE;
}


static BOOLEAN AutomationInit(RTMP_ADAPTER *pAd, AUTOMATION_INIT_TYPE auto_type)
{
	bool ret = TRUE;

	if (!pAd)
		return FALSE;

	if (pAd->auto_dvt == NULL) {
		os_zero_mem(&automation_dvt, sizeof(AUTOMATION_DVT));
		pAd->auto_dvt = &automation_dvt;
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	}

	switch (auto_type) {
	case TXS:
		ret = TxsInit();
		break;
	case RXV:
	case APPS:
		break;
	}

	return ret;
}

static TXS_LIST_ENTRY *GetTxsEntryFromFreeList(VOID)
{
	TXS_LIST_ENTRY *pEntry = NULL;
	TXS_LIST_ENTRY *pheadEntry = NULL;
	TXS_FREE_LIST_POOL *pFreeEntrylist = automation_dvt.txs.txs_list.pFreeEntrylist;
	ULONG IrqFlags = 0;
	UINT32 i;

	if (pFreeEntrylist == NULL)
		return NULL;

	OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags);

	if (DlListEmpty(&pFreeEntrylist->head.mList)) {
		TXS_LIST_POOL *Pool = NULL;
		TXS_LIST_POOL *pFreepool = NULL;

		MTWF_DBG(NULL, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "allocated new pool\n");
		os_alloc_mem(NULL, (UCHAR **)&Pool, sizeof(TXS_LIST_POOL));
		pFreepool = &pFreeEntrylist->pool_head;
		DlListAdd(&pFreepool->List, &Pool->List);
		pheadEntry = &pFreeEntrylist->head;

		for (i = 0; i < TXS_LIST_ELEM_NUM; i++) {
			pEntry = &Pool->Entry[i];
			DlListAdd(&pheadEntry->mList, &pEntry->mList);
		}

		pFreeEntrylist->entry_number += TXS_LIST_ELEM_NUM;
	}

	pheadEntry = &pFreeEntrylist->head;
	pEntry = DlListFirst(&pheadEntry->mList, TXS_LIST_ENTRY, mList);
	DlListDel(&pEntry->mList);

	if (pEntry != NULL)
		pFreeEntrylist->entry_number -= 1;

	OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags);
	return pEntry;
}

static VOID SendRTS(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	FRAME_RTS FrameRTS;
	ULONG FrameLen;
	NDIS_STATUS NStatus;
	UCHAR *pOutBuffer = NULL;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"RTS - MlmeAllocateMemory() allocate memory failed\n");
		return;
	}

	/*	USHORT	Duration;*/
	NdisZeroMemory(&FrameRTS, sizeof(FRAME_RTS));
	FrameRTS.FC.Type = FC_TYPE_CNTL;
	FrameRTS.FC.SubType = SUBTYPE_RTS;
	FrameRTS.Duration = 16 + RTMPCalcDuration(pAd, RATE_1, sizeof(FRAME_RTS));
	COPY_MAC_ADDR(FrameRTS.Addr1, pEntry->Addr);
	COPY_MAC_ADDR(FrameRTS.Addr2, pEntry->wdev->if_addr);

	MakeOutgoingFrame(pOutBuffer,		&FrameLen,
					  sizeof(FRAME_RTS),	&FrameRTS,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RTS - Send RTS\n"));
	MlmeFreeMemory(pOutBuffer);
}

static VOID SendBA(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR TID)
{
	FRAME_BA FrameBA;
	ULONG FrameLen;
	NDIS_STATUS NStatus;
	UCHAR *pOutBuffer = NULL;
	ULONG Idx;
	PBA_REC_ENTRY pBAEntry;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"BA - MlmeAllocateMemory() allocate memory failed\n");
		return;
	}
	Idx = pAd->MacTab.Content[pEntry->wcid].BARecWcidArray[TID];
	pBAEntry = &pAd->BATable.BARecEntry[Idx];
	/*	USHORT	Duration;*/
	NdisZeroMemory(&FrameBA, sizeof(FRAME_BA));
	FrameBA.FC.Type = FC_TYPE_CNTL;
	FrameBA.FC.SubType = SUBTYPE_BLOCK_ACK;
	FrameBA.Duration = 16 + RTMPCalcDuration(pAd, RATE_1, sizeof(FRAME_BA));
	COPY_MAC_ADDR(FrameBA.Addr1, pEntry->Addr);
	COPY_MAC_ADDR(FrameBA.Addr2, pEntry->wdev->if_addr);
	FrameBA.BarControl.ACKPolicy = 1;
	FrameBA.BarControl.Compressed = 1;
	FrameBA.StartingSeq.field.StartSeq = pBAEntry->LastIndSeq;

	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(FRAME_BA), &FrameBA,
					END_OF_ARGS);

	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[TID]),
					pOutBuffer, FrameLen);
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("BA - Send BA, Seq = 0x%x\n", pBAEntry->LastIndSeq));
	MlmeFreeMemory(pOutBuffer);
}


/* Public Function */

bool send_add_txs_queue(UINT8 pid, UINT8 wlan_idx)
{
	TXS_LIST *list = &automation_dvt.txs.txs_list;
	UINT32 idx = 0;
	ULONG IrqFlags = 0;
	TXS_LIST_ENTRY *pEntry;
	TXS_LIST_ENTRY *pheadEntry;

	automation_dvt.txs.total_req++;

	if (!list || !automation_dvt.txs.init) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"txs_list doesnot init\n");
		return FALSE;
	}

	OS_INT_LOCK(&list->lock, IrqFlags);

	pEntry = GetTxsEntryFromFreeList();

	if (!pEntry) {
		OS_INT_UNLOCK(&list->lock, IrqFlags);
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " pEntry is null!!!\n");
		return FALSE;
	}

	idx = automation_dvt.txs.pid % PID_SIZE;
	pheadEntry = &list->pHead[idx];
	pEntry->wlan_idx = wlan_idx;
	DlListAdd(&pheadEntry->mList, &pEntry->mList);
	list->Num++;
	automation_dvt.txs.pid++;

	OS_INT_UNLOCK(&list->lock, IrqFlags);

	return TRUE;
}

bool receive_del_txs_queue(UINT32 sn, UINT8 pid, UINT8 wlan_idx, UINT32 time_stamp)
{
	TXS_LIST *list = &automation_dvt.txs.txs_list;
	ULONG IrqFlags = 0;
	ULONG IrqFlags2 = 0;
	TXS_FREE_LIST_POOL *pFreeEntrylist = NULL;
	TXS_LIST_ENTRY *pheadEntry = NULL, *pEntry = NULL;

	automation_dvt.txs.total_rsp++;

	if (!list || !automation_dvt.txs.init) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"txs_list doesnot init\n");
		return FALSE;
	}

	pFreeEntrylist = list->pFreeEntrylist;
	OS_INT_LOCK(&list->lock, IrqFlags);

	DlListForEach(pEntry, &list->pHead[pid].mList, TXS_LIST_ENTRY, mList) {
		if (pEntry->wlan_idx == wlan_idx) {
			if (automation_dvt.txs.check_item[pid].time_stamp == time_stamp) {
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Duplicate TXS\n");

				automation_dvt.txs.duplicate_txs = TRUE;
			}
			automation_dvt.txs.check_item[pid].time_stamp = time_stamp;
			DlListDel(&pEntry->mList);
			list->Num--;
			OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags2);
			pheadEntry = &pFreeEntrylist->head;
			DlListAddTail(&pheadEntry->mList, &pEntry->mList);
			pFreeEntrylist->entry_number += 1;
			OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags2);
			break;
		}
	}

	OS_INT_UNLOCK(&list->lock, IrqFlags);
	return pEntry;


}

INT set_txs_test(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 txs_test_type = 0;
	BOOLEAN fgStatus = TRUE;
	INT32 i4Recv = 0;
	UINT32 txs_test_format = 0;
	UINT32 u4WCID = 0;
	MAC_TABLE_ENTRY *pEntry;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d", &(txs_test_type), &(txs_test_format), &(u4WCID));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"txs_test_type = %d, txs_test_format = %d u4WCID = %d\n",
					txs_test_type, txs_test_format, u4WCID);

			if (i4Recv > 3) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Parameter too long > 3!\n");
				fgStatus = FALSE;
				break;
			}

			if (!AutomationInit(pAd, TXS)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"AutomationInit Fail!\n");
				return FALSE;
			}

			automation_dvt.txs.duplicate_txs = FALSE;

			switch (txs_test_type) {
			case TXS_INIT:
				TxsExit();
				break;

			case TXS_COUNT_TEST:
				automation_dvt.txs.stop_send_test = FALSE;
				automation_dvt.txs.test_type = TXS_COUNT_TEST;
				automation_dvt.txs.format = txs_test_format;

				break;

			case TXS_BAR_TEST:
				automation_dvt.txs.stop_send_test = FALSE;
				automation_dvt.txs.test_type = TXS_BAR_TEST;
				automation_dvt.txs.format = txs_test_format;
				pEntry = &pAd->MacTab.Content[u4WCID];
				if (!pEntry) {
					fgStatus = FALSE;
					break;
				}

				SendRefreshBAR(pAd, pEntry);
				break;

			case TXS_DEAUTH_TEST:
				automation_dvt.txs.stop_send_test = FALSE;
				automation_dvt.txs.test_type = TXS_DEAUTH_TEST;
				automation_dvt.txs.format = txs_test_format;
				pEntry = &pAd->MacTab.Content[u4WCID];
				if (!pEntry) {
					fgStatus = FALSE;
					break;
				}

				APMlmeKickOutSta(pAd, pEntry->Addr, pEntry->wcid, REASON_DISASSOC_INACTIVE);
				break;

			case TXS_RTS_TEST:
				automation_dvt.txs.stop_send_test = FALSE;
				automation_dvt.txs.test_type = TXS_RTS_TEST;
				automation_dvt.txs.format = txs_test_format;
				pEntry = &pAd->MacTab.Content[u4WCID];
				if (!pEntry) {
					fgStatus = FALSE;
					break;
				}

				SendRTS(pAd, pEntry);
				break;

			case TXS_BA_TEST:
				automation_dvt.txs.stop_send_test = FALSE;
				automation_dvt.txs.test_type = TXS_BA_TEST;
				automation_dvt.txs.format = txs_test_format;
				pEntry = &pAd->MacTab.Content[u4WCID];
				SendBA(pAd, pEntry, 0);
				break;

			case TXS_DUMP_DATA:
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"automation_dvt.txs.test_type=%u\n", automation_dvt.txs.test_type);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"automation_dvt.txs.format=%u\n", automation_dvt.txs.format);
				break;
			}
		} while (0);
	}

	if (fgStatus == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set txs_test=[test_type]-[txs_test_format]-[WCID]\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[test_type]1:txs to host, 2:send bar, 3:send deauth\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[test_type]4:send rts, 5:send ba, 6:dump data\n");
	}

	return fgStatus;
}

INT set_txs_test_result(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 txs_test_result = 0, wait_cnt = 0;
	BOOLEAN fgStatus = TRUE;
	INT32 i4Recv = 0;
	TXS_LIST *list = &automation_dvt.txs.txs_list;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d", &(txs_test_result));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"txs_test_type = %d\n",
					txs_test_result);

			if (i4Recv > 1) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Parameter too much > 1!\n");
				fgStatus = FALSE;
				break;
			}

			if (!AutomationInit(pAd, TXS)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"AutomationInit Fail!\n");
				return FALSE;
			}

			automation_dvt.txs.stop_send_test = TRUE;

			if (txs_test_result == 1) {
				while (automation_dvt.txs.total_req != automation_dvt.txs.total_rsp) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							 "wait entry to be deleted\n");
					OS_WAIT(10);
					wait_cnt++;

					if (wait_cnt > 100)
						break;
				}
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("txs.total_req %u\n", automation_dvt.txs.total_req));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("txs.total_rsp %u\n", automation_dvt.txs.total_rsp));

				if (automation_dvt.txs.total_req == automation_dvt.txs.total_rsp
					 && (automation_dvt.txs.total_req != 0))
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("TXS_COUNT_TEST------> PASS\n"));

				else
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("TXS_COUNT_TEST------> ERROR\n"));

			} else if (txs_test_result == 2) {
				while (list->Num > 0) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							 "wait entry to be deleted\n");
					OS_WAIT(10);
					wait_cnt++;

					if (wait_cnt > 500)
						break;
				}

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("txs.total_req %u\n", automation_dvt.txs.total_req));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("txs.total_rsp %u\n", automation_dvt.txs.total_rsp));

				if (list->Num == 0) {
					if ((automation_dvt.txs.duplicate_txs == FALSE) && (automation_dvt.txs.total_req != 0))
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("Correct Frame Test------> PASS\n"));
					else
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("Correct Frame Test------> FAIL duplicate txs"));

				} else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("Correct Frame Test------> FAIL  txs_q->Num = (%d)\n", list->Num));

				}
			}
		} while (0);
	}

	if (fgStatus == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set txs_test_result=[txs_test_result]\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[txs_test_result]1:TXS_COUNT_TEST  2: Correct frame\n");
	}

	return fgStatus;
}

/*
	return 0 : No Need Test
		   1: Check Data frame
		   2 : Check management and cotrol frame


*/
INT is_frame_test(RTMP_ADAPTER *pAd, UINT8 send_received)
{
	if (!pAd || (pAd->auto_dvt == NULL))
		return 0;

	if (send_received == 0 && automation_dvt.txs.stop_send_test == TRUE)
		return 0;

	switch (automation_dvt.txs.test_type) {
	case TXS_COUNT_TEST:
		return 1;
	case TXS_BAR_TEST:
	case TXS_DEAUTH_TEST:
	case TXS_RTS_TEST:
	case TXS_BA_TEST:
		return 2;

	default:
		return 0;
	}
}

/* RXV Test */
INT set_rxv_test(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN fgStatus = TRUE;
	INT32 i4Recv = 0;
	UINT32 u4Mode = 0, u4Bw = 0, u4Mcs = 0, u4Enable = 0;
	UINT32 u4SGI = 0, u4STBC = 0, u4LDPC = 0, val = 0;

	if (arg) {
		do {

			i4Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d", &(u4Enable),
										&(u4Mode), &(u4Bw), &(u4Mcs),
										&(u4SGI), &(u4STBC), &(u4LDPC));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s():Enable = %d, Mode = %d, BW = %d, MCS = %d\n"
					 "\t\t\t\tSGI = %d, STBC = %d, LDPC = %d\n",
					 __func__, u4Enable, u4Mode, u4Bw, u4Mcs,
					 u4SGI, u4STBC, u4LDPC));

			if (i4Recv > 7) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Parameter too long > 10!\n");
				fgStatus = FALSE;
				break;
			}

			if (!AutomationInit(pAd, RXV)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"AutomationInit Fail!\n");
				return FALSE;
			}

			automation_dvt.rxv.rxv_test_result = TRUE;
			automation_dvt.rxv.enable = u4Enable;
			automation_dvt.rxv.rx_count = 0;


			automation_dvt.rxv.rx_mode = u4Mode;
			automation_dvt.rxv.rx_bw = u4Bw;
			automation_dvt.rxv.rx_rate = u4Mcs;
			automation_dvt.rxv.rx_sgi = u4SGI;
			automation_dvt.rxv.rx_stbc = u4STBC;
			automation_dvt.rxv.rx_ldpc = u4LDPC;


			if (automation_dvt.rxv.enable == TRUE) {
				MAC_IO_READ32(pAd->hdev_ctrl, ARB_RQCR, &val);
				val |= BIT0 | BIT4 | BIT7;
				MAC_IO_WRITE32(pAd->hdev_ctrl, ARB_RQCR, val);

			} else {
				MAC_IO_READ32(pAd->hdev_ctrl, ARB_RQCR, &val);
				val &= ~(BIT4 | BIT7);
				MAC_IO_WRITE32(pAd->hdev_ctrl, ARB_RQCR, val);
			}


		} while (0);
	}

	if (fgStatus == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set rxv_test=[Enable]-[Mode]-[BW]-[MCS]-[SGI]-[STBC]-[LDPC]\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[Mode]CCK=0, OFDM=1, HT=2, GF=3, VHT=4\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[BW]BW20=0, BW40=1, BW80=2,BW160=3\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[MCS]CCK=0~4, OFDM=0~7, HT=0~32, VHT=0~9\n");
	}

	return fgStatus;
}

INT set_rxv_test_result(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	UCHAR result_type = (UCHAR) os_str_tol(arg, 0, 10);

	do {
		if (!AutomationInit(pAd, RXV)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"AutomationInit Fail!\n");
			return FALSE;
		}

		if (result_type == 1) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("RXV Test------> rx_count(%d)\n", automation_dvt.rxv.rx_count));

			if (automation_dvt.rxv.rxv_test_result == TRUE &&
				automation_dvt.rxv.rx_count != 0) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("RXV Test------> PASS\n"));
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("RXV Test------> FAIL\n"));
			}
		}
	} while (0);

	return TRUE;
}



INT rxv_correct_test(UCHAR *Data)
{

	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data);

	automation_dvt.rxv.rx_count++;

	if (RXV1_1ST_CYCLE->TxMode != automation_dvt.rxv.rx_mode) {
		automation_dvt.rxv.rxv_test_result = FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Receive TxMode=%d, Check RxMode=%d\n",
		RXV1_1ST_CYCLE->TxMode, automation_dvt.rxv.rx_mode);
	}
	if (RXV1_1ST_CYCLE->TxRate != automation_dvt.rxv.rx_rate) {
		automation_dvt.rxv.rxv_test_result = FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Receive TxRate=%d, Check RxRate=%d\n",
		__func__, RXV1_1ST_CYCLE->TxRate, automation_dvt.rxv.rx_rate);
	}
	if (RXV1_1ST_CYCLE->FrMode != automation_dvt.rxv.rx_bw) {
		automation_dvt.rxv.rxv_test_result = FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Receive BW=%d, Check BW=%d\n",
		RXV1_1ST_CYCLE->FrMode, automation_dvt.rxv.rx_bw);
    }
	if (RXV1_1ST_CYCLE->HtShortGi != automation_dvt.rxv.rx_sgi) {
		automation_dvt.rxv.rxv_test_result = FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Receive Sgi=%d, Check Sgi=%d\n",
		RXV1_1ST_CYCLE->HtShortGi, automation_dvt.rxv.rx_sgi);
	}
	if (RXV1_1ST_CYCLE->HtStbc != automation_dvt.rxv.rx_stbc) {
		automation_dvt.rxv.rxv_test_result = FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Receive Stbc=%d, Check Stbc=%d\n",
		RXV1_1ST_CYCLE->HtStbc, automation_dvt.rxv.rx_stbc);
	}
	if (RXV1_1ST_CYCLE->HtAdCode != automation_dvt.rxv.rx_ldpc) {
		automation_dvt.rxv.rxv_test_result = FALSE;
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Receive Ldpc=%d, Check Ldpc=%d\n",
		RXV1_1ST_CYCLE->HtAdCode, automation_dvt.rxv.rx_ldpc);
	}

	return TRUE;
}

#ifdef HDR_TRANS_RX_SUPPORT
INT set_hdr_translate_blist(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN fgStatus = TRUE;
	INT32 i4Recv = 0;
	UINT32 u4idx = 0, u4Enable = 0;
	UINT32 u4EtherType = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%x", &(u4idx), &(u4Enable), &(u4EtherType));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s():u4idx = %d, u4Enable = %d, u4EtherType = %04x\n",
					 __func__, u4idx, u4Enable, u4EtherType));

			if (i4Recv > 3) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Parameter too long > 3!\n");
				fgStatus = FALSE;
				break;
			}

			AsicRxHeaderTaranBLCtl(pAd, u4idx, u4Enable, u4EtherType);
		} while (0);
	}

	if (fgStatus == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set rxv_test=[Idx]-[Enable]-[EtherType]\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[Idx] = 0~7, vary by chip\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[Enable] = 0/1 \n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[EtherType] = 0x888e , or other wellknown value\n");
	}

	return fgStatus;
}
#endif /* HDR_TRANS_RX_SUPPORT */

/* AP MBSS Test */
BSS_ENTRY_CHECK *ap_bss_check_by_mac_lookup(RTMP_ADAPTER *pAd, BSS_CHECK_CTRL *pBssChkCtrl, UCHAR *pAddr)
{
	BSS_ENTRY_CHECK *pBss = NULL, *pFirstEmpty = NULL;
	UINT32 i;

	if (!pBssChkCtrl || !pAddr)
		return NULL;

	if (pBssChkCtrl->BssCnt == 0)
		return &pBssChkCtrl->BssEntry[0];

	/* Add one for find a empty bss */
	for (i = 0; i < (pBssChkCtrl->BssCnt + 1); i++) {
		pBss = &pBssChkCtrl->BssEntry[i];
		if (!pBss->bValid && (pFirstEmpty == NULL))
			pFirstEmpty = pBss;

		if (pBss->bValid && MAC_ADDR_EQUAL(pBss->BssMAC, pAddr))
			return pBss;
	}

	return pFirstEmpty;
}

VOID rx_peer_beacon_check(RTMP_ADAPTER *pAd, BCN_IE_LIST *ie_list, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFrame = NULL;
	UCHAR SubType;

	if (Elem == NULL || ie_list == NULL)
		return;

	pFrame = (PFRAME_802_11)Elem->Msg;
	if (pFrame == NULL)
		return;

	SubType = (UCHAR)pFrame->Hdr.FC.SubType;

	if (SubType == SUBTYPE_BEACON) {
		UCHAR BandIdx = HcGetBandByChannelRange(pAd, Elem->Channel);

		if (pAd->BssChkCtrl[BandIdx].bEnable) {
			BSS_CHECK_CTRL *pBssChkCtrl = &pAd->BssChkCtrl[BandIdx];
			PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;
			BSS_ENTRY_CHECK *pBss = ap_bss_check_by_mac_lookup(pAd, pBssChkCtrl, pFrame->Hdr.Addr2);

			if (BSS_CHECK_BEACON_SN_ON(pAd->BssChkCtrl[BandIdx]) ||
				BSS_CHECK_BEACON_SSID_ON(pAd->BssChkCtrl[BandIdx])) {
				if (pBss) {
					if (!pBss->bValid) {
						pBss->bValid = TRUE;
						COPY_MAC_ADDR(pBss->BssMAC, pFrame->Hdr.Addr2);
						pBssChkCtrl->BssCnt++;

						if (BSS_CHECK_BEACON_SN_ON(pAd->BssChkCtrl[BandIdx])) {
							pBss->BssCurrentBeaconSN = pFrame->Hdr.Sequence;
							pBss->BssLastBeaconSN = pFrame->Hdr.Sequence;
						}

						if (BSS_CHECK_BEACON_SSID_ON(pAd->BssChkCtrl[BandIdx])) {
							NdisMoveMemory(pBss->Ssid, ie_list->Ssid, ie_list->SsidLen);
							pBss->SsidLen = ie_list->SsidLen;
						}

						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"New BSS MAC:"MACSTR"\n", MAC2STR(pBss->BssMAC));
					} else {
						if (BSS_CHECK_BEACON_SN_ON(pAd->BssChkCtrl[BandIdx])) {
							pBss->BssLastBeaconSN = pBss->BssCurrentBeaconSN;
							pBss->BssCurrentBeaconSN = pFrame->Hdr.Sequence;
						}

						if (BSS_CHECK_BEACON_SSID_ON(pAd->BssChkCtrl[BandIdx])) {
							if (!SSID_EQUAL(pBss->Ssid, pBss->SsidLen, ie_list->Ssid, ie_list->SsidLen)) {
								pBss->bSsidModified = TRUE;
								NdisMoveMemory(pBss->Ssid, ie_list->Ssid, ie_list->SsidLen);
								pBss->SsidLen = ie_list->SsidLen;
								MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
									"Old SSID:%s, New SSID:%s\n", pBss->Ssid, ie_list->Ssid);
							}
						}
					}
				}
			}
		}
	}
}

INT set_ap_mbss_check_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 TestItem;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 band_idx = HcGetBandByWdev(wdev);
	BSS_CHECK_CTRL *pBssChkCtrl = &pAd->BssChkCtrl[band_idx];

	TestItem = os_str_tol(arg, 0, 10);
	NdisZeroMemory(pBssChkCtrl, sizeof(*pBssChkCtrl));
	pBssChkCtrl->TestItem = TestItem;

	switch (TestItem) {
	case 1:
		pBssChkCtrl->bEnable = TRUE;
		pBssChkCtrl->ChkItem |= ENUM_CHK_BEACON_SN;
		break;

	case 2:
		pBssChkCtrl->bEnable = TRUE;
		pBssChkCtrl->ChkItem |= (ENUM_CHK_BEACON_SN | ENUM_CHK_BEACON_SSID);
		break;

	default:
		break;
	}

	return TRUE;
}

INT set_ap_mbss_get_result_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 i;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 band_idx = HcGetBandByWdev(wdev);
	BSS_CHECK_CTRL *pBssChkCtrl = &pAd->BssChkCtrl[band_idx];
	BSS_ENTRY_CHECK *pBss = NULL;
	BOOLEAN bPass = FALSE;

	switch (pBssChkCtrl->TestItem) {
	case 1: /* Beacon SN */
	case 2: /* Beacon SN and SSID */
		if ((pBssChkCtrl->ChkItem & ENUM_CHK_BEACON_SN)) {
			for (i = 0; i < pBssChkCtrl->BssCnt; i++) {
				pBss = &pBssChkCtrl->BssEntry[i];
				if (pBss->bValid) {
					if (pBss->BssCurrentBeaconSN != pBss->BssLastBeaconSN)
						bPass = TRUE;
					else
						bPass = FALSE;

					if ((pBssChkCtrl->ChkItem & ENUM_CHK_BEACON_SSID))
						bPass &= pBss->bSsidModified;
				}
			}
		}
		break;

	default:
		break;
	}

	pBssChkCtrl->bEnable = FALSE;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Item %d: Test Result is %s!\n", pBssChkCtrl->TestItem, bPass?"Pass":"Fail");

	return TRUE;
}

VOID rxd_wcid_check(RTMP_ADAPTER *pAd, UINT16 RxDWlanIdx)
{
	if (pAd->assignWcid > 0 && (pAd->assignWcid != RxDWlanIdx) && RxDWlanIdx != WCID_NO_MATCHED(pAd))
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Found received wcid not match!(assignWcid=%d)\n", pAd->assignWcid);
}

VOID automation_rx_apps_check(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	BOOLEAN test_result = TRUE;

	if (!automation_dvt.apps.head_chk.enable)
		return;

	if (!AutomationInit(pAd, APPS)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"AutomationInit Fail!\n");
		return;
	}

	/* Check RX Type */
	if (automation_dvt.apps.head_chk.type != 0 &&
		((FRAME_CONTROL *)pRxBlk->FC)->Type != automation_dvt.apps.head_chk.type) {
		test_result = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Receive Type=%d, Check Type=%d\n",
		((FRAME_CONTROL *)pRxBlk->FC)->Type, automation_dvt.apps.head_chk.type);
	}

	/* Check RX SubType */
	if (automation_dvt.apps.head_chk.subtype != 0 &&
		((FRAME_CONTROL *)pRxBlk->FC)->SubType != automation_dvt.apps.head_chk.subtype) {
		test_result = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Receive SubType=%d, Check subtype=%d\n",
		((FRAME_CONTROL *)pRxBlk->FC)->SubType, automation_dvt.apps.head_chk.subtype);
	}

	/* Check RX MoreData */
	if (automation_dvt.apps.head_chk.moredata != 0 &&
		((FRAME_CONTROL *)pRxBlk->FC)->MoreData != automation_dvt.apps.head_chk.moredata) {
		test_result = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Receive MoreData=%d, Check moredata=%d\n",
		((FRAME_CONTROL *)pRxBlk->FC)->MoreData, automation_dvt.apps.head_chk.moredata);
	}

	/* Check RX EOSP */
	if (automation_dvt.apps.head_chk.eosp != 0 && (((FRAME_CONTROL *)pRxBlk->FC)->SubType & 0x08)) {
		UCHAR *pData;

		/* Qos bit 4 */
		pData = pRxBlk->FC + LENGTH_802_11;

		if (!((*pData >> 4) & 0x01)) {
			test_result = FALSE;
			MTWF_DBG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Receive eosp=%d, Check eosp=%d\n",
			((*pData >> 4) & 0x01), automation_dvt.apps.head_chk.eosp);
		}
	}

	if (test_result == TRUE)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s]Header Check Pass\n", __func__));

}

/* APPS Test */
#ifdef APCLI_SUPPORT
#ifdef UAPSD_SUPPORT
INT set_ApCli_UAPSD_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR ifIndex ;
	UINT Enable;
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	Enable = os_str_tol(arg, 0, 10);
	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	pApCliEntry->wdev.UapsdInfo.bAPSDCapable = Enable;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCliUAPSDCapable[%d]=%d\n", ifIndex,
			 pApCliEntry->wdev.UapsdInfo.bAPSDCapable));

	return TRUE;
}

INT set_ApCli_APSDAC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *value = NULL;
	UCHAR i = 0;
	BOOLEAN apsd_ac[4] = {0};
	POS_COOKIE pObj;
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); (i < WMM_NUM_OF_AC && value != NULL); value = rstrtok(NULL, ":"), i++) {
		apsd_ac[i] = (BOOLEAN)os_str_tol(value, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APSDAC[%d]=%d\n", i,  apsd_ac[i]);
	}

	if (i  !=  WMM_NUM_OF_AC) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Invalid parameters\n");
		return FALSE;
	}

	pAd->CommonCfg.bAPSDAC_BE = apsd_ac[0];
	pAd->CommonCfg.bAPSDAC_BK = apsd_ac[1];
	pAd->CommonCfg.bAPSDAC_VI = apsd_ac[2];
	pAd->CommonCfg.bAPSDAC_VO = apsd_ac[3];
	pAd->CommonCfg.bACMAPSDTr[0] = apsd_ac[0];
	pAd->CommonCfg.bACMAPSDTr[1] = apsd_ac[1];
	pAd->CommonCfg.bACMAPSDTr[2] = apsd_ac[2];
	pAd->CommonCfg.bACMAPSDTr[3] = apsd_ac[3];
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("APSDAC::(BE,BK,VI,VO)=(%d,%d,%d,%d)\n",
			 pAd->CommonCfg.bAPSDAC_BE,
			 pAd->CommonCfg.bAPSDAC_BK,
			 pAd->CommonCfg.bAPSDAC_VI,
			 pAd->CommonCfg.bAPSDAC_VO));
	return TRUE;
}

INT set_ApCli_MaxSPLength_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	pAd->CommonCfg.MaxSPLength = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxSPLength=%d\n", pAd->CommonCfg.MaxSPLength));
	return TRUE;
}
#endif /* UAPSD_SUPPORT */
INT set_ApCli_Block_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR ifIndex;
	UINT block_case = 0;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	block_case = os_str_tol(arg, 0, 10);

	if (!AutomationInit(pAd, APPS)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"AutomationInit Fail!\n");
		return FALSE;
	}

	automation_dvt.apps.block_packet = block_case;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("-->%s():block_case:%d\n",
			__func__, automation_dvt.apps.block_packet));

	return TRUE;
}

INT set_ApCli_Rx_Packet_Check_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	INT32 i4Recv = 0;
	UINT32 u4Enable = 0;
	UINT32 u4Type, u4SubType, u4MoreData, u4Eosp;
	BOOLEAN fgStatus = TRUE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d-%d-%d", &(u4Enable),
										&(u4Type), &(u4SubType), &(u4MoreData), &(u4Eosp));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s():Enable = %d, Type = %d, SubType = %d, MoreData = %d\n",
					 __func__, u4Enable, u4Type, u4SubType, u4MoreData));

			if (i4Recv > 5) {
				MTWF_DBG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Parameter too long > 5!\n");
				fgStatus = FALSE;
				break;
			}

			if (!AutomationInit(pAd, APPS)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"AutomationInit Fail!\n");
				return FALSE;
			}

			automation_dvt.apps.head_chk.test_result = TRUE;
			automation_dvt.apps.head_chk.enable = u4Enable;
			automation_dvt.apps.head_chk.type = u4Type;
			automation_dvt.apps.head_chk.subtype = u4SubType;
			automation_dvt.apps.head_chk.moredata = u4MoreData;
			automation_dvt.apps.head_chk.eosp = u4Eosp;
		} while (0);

		if (fgStatus == FALSE) {
			MTWF_DBG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set ApCliCheckPkt=[Enable]-[Type]-[SubType]-[MoreData]-[Eosp]\n");
		}
	}

	return TRUE;
}

VOID sta_rx_packet_check(RTMP_ADAPTER *pAd, VOID *pRx_Blk)
{
	RX_BLK *pRxBlk = (RX_BLK *)pRx_Blk;
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;

	automation_rx_apps_check(pAd, pRxBlk);

	if (!pRxInfo)
		return;

	MTWF_DBG(NULL, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "-->%s():U2M:%d Mcast:%d Bcast:%d\n",
			__func__, pRxInfo->U2M, pRxInfo->Mcast, pRxInfo->Bcast);
}
#endif /* APCLI_SUPPORT */

INT set_txrx_dbg_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN		fgStatus = TRUE;
	UINT32		u4Dbg_type = 0;

	if (IS_MT7663(pAd) || IS_MT7626(pAd)) {
		if (arg) {
			do {
#ifdef HW_TX_AMSDU_SUPPORT
				RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
				u4Dbg_type = os_str_tol(arg, 0, 10);

				if (u4Dbg_type > 8) {
					fgStatus = FALSE;
					break;
				}
				pAd->fpga_ctl.txrx_dbg_type = u4Dbg_type;

				if (pAd->fpga_ctl.txrx_dbg_type == 4) {
#ifdef HW_TX_AMSDU_SUPPORT
					pChipCap->asic_caps &= ~fASIC_CAP_HW_TX_AMSDU;
#endif /* HW_TX_AMSDU_SUPPORT */
				} else {
#ifdef HW_TX_AMSDU_SUPPORT
					pChipCap->asic_caps |= fASIC_CAP_HW_TX_AMSDU;
#endif /* HW_TX_AMSDU_SUPPORT */
				}

			} while (0);
		} else
			fgStatus = FALSE;

		if (fgStatus == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"iwpriv ra0 set txrx_dbg_type=[u4Dbg_type]\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"[u4Dbg_type]0~8\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"     0: disable dbg dump\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"     1: tx: dump vlan tag remove\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"     2: rx: dump rx data type which length > 500\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"     3: tx/rx: dump tx/rx fragment data\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"     4: tx: disable hw amsdu\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"     5: tx: keep vlan source tag\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"     6: rx: Rx CSO dump/Rx VLAN EtherType dump\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"     7: rx: RxV dump!\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"     8: rx: Rx BCN SN dump!\n");
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"[u4Dbg_type]: %d\n", u4Dbg_type);
		}
	} else {
		fgStatus = FALSE;
	}
	return fgStatus;
}

VOID automation_rx_payload_check(RTMP_ADAPTER *pAd, PNDIS_PACKET pRxPkt)
{
	if ((pAd->fpga_ctl.txrx_dbg_type == 5)
		|| (pAd->fpga_ctl.txrx_dbg_type == 6)) {

		UINT16 protocol = ntohs(RTPKT_TO_OSPKT(pRxPkt)->protocol);
		if  (protocol == ETH_TYPE_VLAN) {
			/*
				due to nping tool can't be bigger size vlan pkts, payload only 46 bytes
			*/
			if (GET_OS_PKT_LEN(pRxPkt) <= 50)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s[%d] protocol=0x%04x, RAW :%02X %02x, GET_OS_PKT_LEN(pRxPkt)=%d\n",
					__func__, __LINE__, protocol, *(GET_OS_PKT_DATAPTR(pRxPkt)),  *(GET_OS_PKT_DATAPTR(pRxPkt)+1), GET_OS_PKT_LEN(pRxPkt)));
		} else if (GET_OS_PKT_LEN(pRxPkt) > 500) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s[%d] protocol=0x%04x, GET_OS_PKT_LEN(pRxPkt)=%d\n\r",
				__func__, __LINE__, protocol, GET_OS_PKT_LEN(pRxPkt)));
			switch (protocol) {
			case ETH_TYPE_IPv4:
				if ((*(GET_OS_PKT_DATAPTR(pRxPkt) + 9)) == IP_PROTO_UDP)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Rx IPv4,UDP CS: %02x %02x\n", __func__, *(GET_OS_PKT_DATAPTR(pRxPkt)+26), *(GET_OS_PKT_DATAPTR(pRxPkt)+27)));
				else if ((*(GET_OS_PKT_DATAPTR(pRxPkt) + 9)) == IP_PROTOCOL_TCP)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Rx IPv4,TCP CS: %02x %02x\n", __func__, *(GET_OS_PKT_DATAPTR(pRxPkt)+36), *(GET_OS_PKT_DATAPTR(pRxPkt)+37)));
					break;
			case ETH_TYPE_IPv6:
				if ((*(GET_OS_PKT_DATAPTR(pRxPkt) + 6)) == IP_PROTO_UDP)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Rx IPv6,UDP CS: %02x %02x\n", __func__, *(GET_OS_PKT_DATAPTR(pRxPkt)+46), *(GET_OS_PKT_DATAPTR(pRxPkt)+47)));
				else if ((*(GET_OS_PKT_DATAPTR(pRxPkt) + 6)) == IP_PROTOCOL_TCP)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Rx IPv6,TCP CS: %02x %02x\n", __func__, *(GET_OS_PKT_DATAPTR(pRxPkt)+56), *(GET_OS_PKT_DATAPTR(pRxPkt)+57)));
					break;
			}
		}
	}
}


VOID automation_dump_rxd_rxblk(RTMP_ADAPTER *pAd, CHAR *func, INT line, struct _RX_BLK *pRxBlk, struct _RXD_BASE_STRUCT *rx_base)
{
	if ((pAd->fpga_ctl.txrx_dbg_type == 2) || (pAd->fpga_ctl.txrx_dbg_type == 6)) {
		if (pRxBlk->MPDUtotalByteCnt > 500) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s[%d]size=%d\n", func, line,
					pRxBlk->MPDUtotalByteCnt));
			if (((FRAME_CONTROL *)pRxBlk->FC)->Type == FC_TYPE_DATA) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s[%d]FC_TYPE_DATA\n", func, line));
				if (((FRAME_CONTROL *)pRxBlk->FC)->SubType == SUBTYPE_QDATA) {
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("%s[%d]SUBTYPE_QDATA, RxD2.NonAmpduFrm=%d, RxD2.NonAmpduSfrm=%d, rx_base->RxD1.PayloadFmt=0x%x, rx_base->RxD1.HdrTranslation=%d\n",
							func, line, rx_base->RxD2.NonAmpduFrm, rx_base->RxD2.NonAmpduSfrm, rx_base->RxD1.PayloadFmt, rx_base->RxD1.HdrTranslation));
				} else if (((FRAME_CONTROL *)pRxBlk->FC)->SubType == SUBTYPE_DATA) {
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("%s[%d]SUBTYPE_DATA,  RxD2.NonAmpduFrm=%d, RxD2.NonAmpduSfrm=%d, rx_base->RxD1.PayloadFmt=0x%x, rx_base->RxD1.HdrTranslation=%d\n",
							func, line, rx_base->RxD2.NonAmpduFrm, rx_base->RxD2.NonAmpduSfrm, rx_base->RxD1.PayloadFmt, rx_base->RxD1.HdrTranslation));
				}
			}
		}
	}

	if (pAd->fpga_ctl.txrx_dbg_type == 3) {
		if (pRxBlk->pRxInfo->FRAG)
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s[%d]FRAG, size=%d\n", func, line,
					pRxBlk->MPDUtotalByteCnt));
		else
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s[%d]Non FRAG\n", func, line);
	}
}
