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
	meminfo_list.h
*/
#ifndef __MEMINFO_LIST_H__
#define __MEMINFO_LIST_H__

#ifdef MEM_ALLOC_INFO_SUPPORT

#include "rtmp_comm.h"

#define POOL_ENTRY_NUMBER 2000
#define HASH_SIZE 1024

typedef struct _MEM_INFO_LIST_ENTRY {
	DL_LIST mList;
	UINT32 MemSize;
	VOID *pMemAddr;
	VOID *pCaller;
	VOID *pLastPos;
	UINT uPosIdx;/*18~31 is used for tokenid*/
} MEM_INFO_LIST_ENTRY;

typedef struct _CALLER_INFO_LIST_ENTRY {
	DL_LIST mList;
	INT cnt;
	UINT size;
	VOID *pCaller;
} CALLER_INFO_LIST_ENTRY;

typedef struct _MEM_INFO_LIST_POOL {
	MEM_INFO_LIST_ENTRY Entry[POOL_ENTRY_NUMBER];
	DL_LIST List;
} MEM_INFO_LIST_POOL;

typedef struct _FREE_LIST_POOL {
	MEM_INFO_LIST_ENTRY head;
	MEM_INFO_LIST_POOL Poolhead;
	UINT32 EntryNumber;
	NDIS_SPIN_LOCK Lock;
	UINT32 MLlistCnt;
} FREE_LIST_POOL;

typedef struct _MEM_INFO_LIST {
	MEM_INFO_LIST_ENTRY pHead[HASH_SIZE];
	INT type;
	NDIS_SPIN_LOCK Lock;
	UINT32 EntryNumber;
	UINT32 CurAlcSize;
	UINT32 MaxAlcSize;
	FREE_LIST_POOL *pFreeEntrylist;
	CALLER_INFO_LIST_ENTRY pCaller[HASH_SIZE];
} MEM_INFO_LIST;

enum {
	SHOW_PCALLER_INFO,
	SHOW_ALL_PKT_INFO,
	SHOW_PCALLER_PKT_INFO,
	DUMP_PCALLER_PKT,
	RELEASE_PCALLER_PKT,
	SHOW_PKT_INFO,
	MAX_TYPE,
};

enum {
	MEMINFO,
	PKTMEMINFO,
};

#define MAX_POS_IDX 14

static FREE_LIST_POOL FreeEntrylist;

static inline MEM_INFO_LIST_ENTRY *GetEntryFromFreeList(MEM_INFO_LIST *MIList)
{
	MEM_INFO_LIST_ENTRY *pEntry = NULL;
	MEM_INFO_LIST_ENTRY *pheadEntry = NULL;
	FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;
	ULONG IrqFlags = 0;
	UINT32 i;

	OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags);

	if (DlListEmpty(&pFreeEntrylist->head.mList)) {
		MEM_INFO_LIST_POOL *Pool = NULL;
		MEM_INFO_LIST_POOL *pFreepool = NULL;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: allocated new pool\n", __func__));
		Pool = kmalloc(sizeof(MEM_INFO_LIST_POOL), GFP_ATOMIC);
		pFreepool = &pFreeEntrylist->Poolhead;
		DlListAdd(&pFreepool->List, &Pool->List);
		pheadEntry = &pFreeEntrylist->head;

		for (i = 0; i < POOL_ENTRY_NUMBER; i++) {
			pEntry = &Pool->Entry[i];
			DlListAdd(&pheadEntry->mList, &pEntry->mList);
		}

		pFreeEntrylist->EntryNumber += POOL_ENTRY_NUMBER;
	}

	pheadEntry = &pFreeEntrylist->head;
	pEntry = DlListFirst(&pheadEntry->mList, MEM_INFO_LIST_ENTRY, mList);
	DlListDel(&pEntry->mList);

	if (pEntry != NULL)
		pFreeEntrylist->EntryNumber -= 1;

	OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags);
	return pEntry;
}

static inline UINT32 HashF(VOID *pMemAddr)
{
	LONG addr = (LONG)pMemAddr;
	UINT32 a = addr & 0xF;
	UINT32 b = (addr & 0xF0) >> 4;
	UINT32 c = (addr & 0xF00) >> 8;
	UINT32 d = (addr & 0xF000) >> 12;
	UINT32 e = (addr & 0xF0000) >> 16;
	UINT32 f = (addr & 0xF00000) >> 20;
	UINT32 g = (addr & 0xF000000) >> 24;
	UINT32 h = (addr & 0xF0000000) >> 28;

	return (a + b * 3 + c * 5 + d * 7 + e * 11 + f * 13 + g * 17 + h * 19) % HASH_SIZE;
}
static inline VOID PoolInit(VOID)
{
	if (FreeEntrylist.MLlistCnt == 0) {
		NdisAllocateSpinLock(NULL, &FreeEntrylist.Lock);
		DlListInit(&FreeEntrylist.Poolhead.List);
		DlListInit(&FreeEntrylist.head.mList);
	}

	FreeEntrylist.MLlistCnt++;
}

static inline VOID PoolUnInit(VOID)
{
	FreeEntrylist.MLlistCnt--;

	if (FreeEntrylist.MLlistCnt == 0) {
		MEM_INFO_LIST_POOL *pEntry = NULL;

		while (!DlListEmpty(&FreeEntrylist.Poolhead.List)) {
			pEntry = DlListFirst(&FreeEntrylist.Poolhead.List, MEM_INFO_LIST_POOL, List);
			DlListDel(&pEntry->List);
			kfree(pEntry);
		}
	}
}


static inline VOID MIListInit(MEM_INFO_LIST *MIList)
{
	UINT32 i;

	NdisZeroMemory(MIList->pCaller, sizeof(MIList->pCaller));

	for (i = 0; i < HASH_SIZE; i++) {
		DlListInit(&MIList->pHead[i].mList);
		DlListInit(&MIList->pCaller[i].mList);
	}

	NdisAllocateSpinLock(NULL, &MIList->Lock);
	MIList->EntryNumber = 0;
	MIList->CurAlcSize = 0;
	MIList->MaxAlcSize = 0;
	PoolInit();

	if (DlListEmpty(&FreeEntrylist.Poolhead.List)) {
		MEM_INFO_LIST_POOL *Pool = NULL;
		MEM_INFO_LIST_POOL *pFreepool = NULL;
		MEM_INFO_LIST_ENTRY *pEntry = NULL;
		MEM_INFO_LIST_ENTRY *newEntry = NULL;

		Pool = kmalloc(sizeof(MEM_INFO_LIST_POOL), GFP_ATOMIC);
		pFreepool = &FreeEntrylist.Poolhead;
		DlListAdd(&pFreepool->List, &Pool->List);
		pEntry = &FreeEntrylist.head;

		for (i = 0; i < POOL_ENTRY_NUMBER; i++) {
			newEntry = &Pool->Entry[i];
			DlListAdd(&pEntry->mList, &newEntry->mList);
		}

		FreeEntrylist.EntryNumber += POOL_ENTRY_NUMBER;
	}

	MIList->pFreeEntrylist = &FreeEntrylist;
}
static inline VOID MIListExit(MEM_INFO_LIST *MIList)
{
	UINT32 i = 0;
	ULONG IrqFlags = 0;
	CALLER_INFO_LIST_ENTRY *pCalEntry = NULL;

	OS_INT_LOCK(&MIList->Lock, IrqFlags);

	for (i = 0; i < HASH_SIZE; i++) {
		DlListInit(&MIList->pHead[i].mList);
		while (!DlListEmpty(&MIList->pCaller[i].mList)) {
			pCalEntry = DlListFirst(&MIList->pCaller[i].mList, CALLER_INFO_LIST_ENTRY, mList);
			kfree(pCalEntry);
		}
		DlListInit(&MIList->pCaller[i].mList);
	}

	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	NdisFreeSpinLock(&MIList->Lock);
	MIList->EntryNumber = 0;
	PoolUnInit();
}

static inline MEM_INFO_LIST_ENTRY *MIListReCord(MEM_INFO_LIST *MIList, VOID *pMemAddr, UINT Idx, VOID *pCaller)
{
	UINT32 Index = HashF(pMemAddr);
	ULONG IrqFlags = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;

	OS_INT_LOCK(&MIList->Lock, IrqFlags);
	DlListForEach(pEntry, &MIList->pHead[Index].mList, MEM_INFO_LIST_ENTRY, mList) {
		if (pEntry->pMemAddr == pMemAddr) {
			pEntry->pLastPos = pCaller;
			pEntry->uPosIdx |= Idx;
			break;
		}
	}
	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	return pEntry;
}

static inline MEM_INFO_LIST_ENTRY *MIListRemove(MEM_INFO_LIST *MIList, VOID *pMemAddr, VOID *pCaller)
{
	UINT32 Index = HashF(pMemAddr);
	ULONG IrqFlags = 0;
	ULONG IrqFlags2 = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;
	MEM_INFO_LIST_ENTRY *pheadEntry = NULL;
	FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;
	int find = 0;

	OS_INT_LOCK(&MIList->Lock, IrqFlags);
	DlListForEach(pEntry, &MIList->pHead[Index].mList, MEM_INFO_LIST_ENTRY, mList) {
		if (pEntry->pMemAddr == pMemAddr) {
			UINT32 CallerIdx = HashF(pEntry->pCaller);
			CALLER_INFO_LIST_ENTRY *pCalEntry = NULL;

			find = 1;
			DlListForEach(pCalEntry, &MIList->pCaller[CallerIdx].mList, CALLER_INFO_LIST_ENTRY, mList) {
				if (pCalEntry->pCaller == pEntry->pCaller) {
					pCalEntry->cnt--;
					pCalEntry->size -= pEntry->MemSize;
					if (pCalEntry->cnt == 0 && pCalEntry->size > 0)
						MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("error addr = %p, size = %u, caller is %pS pCalEntry->size:%x\n",
							 pEntry->pMemAddr, pEntry->MemSize, pEntry->pCaller, pCalEntry->size));
					break;
				}
			}
			DlListDel(&pEntry->mList);
			MIList->EntryNumber--;
			MIList->CurAlcSize -= pEntry->MemSize;
			OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags2);
			pheadEntry = &pFreeEntrylist->head;
			DlListAddTail(&pheadEntry->mList, &pEntry->mList);
			pFreeEntrylist->EntryNumber += 1;
			OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags2);

			break;
		}
	}
	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	if (find == 0)
		return NULL;
	return pEntry;
}


static inline VOID MIListAddHead(MEM_INFO_LIST *MIList, UINT32 Size, VOID *pMemAddr, VOID *pCaller)
{
	UINT32 Index = HashF(pMemAddr);
	ULONG IrqFlags = 0;
	MEM_INFO_LIST_ENTRY *pEntry;
	MEM_INFO_LIST_ENTRY *pheadEntry;
	UINT32 CallerIdx = HashF(pCaller);
	CALLER_INFO_LIST_ENTRY *pCalEntry = NULL;
	CALLER_INFO_LIST_ENTRY *pheadCalEntry;
	int find = 0;

	if (pMemAddr == 0) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("new addr = %p, size = %u, caller is %pS\n", pEntry->pMemAddr, pEntry->MemSize, pEntry->pCaller));
		dump_stack();
	}

	OS_INT_LOCK(&MIList->Lock, IrqFlags);
	pEntry = GetEntryFromFreeList(MIList);

	if (!pEntry) {
		OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pEntry is null!!!\n", __func__));
		return;
	}

	pEntry->MemSize = Size;
	pEntry->pMemAddr = pMemAddr;
	pEntry->pCaller = pCaller;
	pEntry->pLastPos = 0;
	pEntry->uPosIdx = 0;
	pheadEntry = &MIList->pHead[Index];
	pheadCalEntry = &MIList->pCaller[CallerIdx];

	DlListForEach(pCalEntry, &MIList->pCaller[CallerIdx].mList, CALLER_INFO_LIST_ENTRY, mList) {
		if (pCalEntry->pCaller == pCaller) {
			pCalEntry->cnt++;
			pCalEntry->size += Size;
			find = 1;
			break;
		}
	}

	if (!find) {
		pCalEntry = kmalloc(sizeof(CALLER_INFO_LIST_ENTRY), GFP_ATOMIC);
		DlListInit(&pCalEntry->mList);
		pCalEntry->pCaller = pCaller;
		pCalEntry->size = Size;
		pCalEntry->cnt = 1;
		DlListAdd(&pheadCalEntry->mList, &pCalEntry->mList);
	}
	DlListAdd(&pheadEntry->mList, &pEntry->mList);
	MIList->EntryNumber++;
	MIList->CurAlcSize += pEntry->MemSize;

	if (MIList->type == PKTMEMINFO) {
		if (MIList->EntryNumber > MIList->MaxAlcSize)
			MIList->MaxAlcSize = MIList->EntryNumber;
	} else {
		if (MIList->CurAlcSize > MIList->MaxAlcSize)
			MIList->MaxAlcSize = MIList->CurAlcSize;
	}

	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
}

static inline VOID ShowMIList(MEM_INFO_LIST *MIList, UINT show, UINT pCaller)
{
	UINT32 i, total = 0, find = 0;
	ULONG IrqFlags = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;
	CALLER_INFO_LIST_ENTRY *pCalEntry = NULL;
	UINT32 CallerIdx;

	CallerIdx = HashF((VOID *)pCaller);

	OS_INT_LOCK(&MIList->Lock, IrqFlags);
begin:
	if (show == SHOW_PCALLER_INFO) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("show caller size:%d %d\n", sizeof(MEM_INFO_LIST_POOL), sizeof(CALLER_INFO_LIST_ENTRY)));
		for (i = 0; i < HASH_SIZE; i++) {
			DlListForEach(pCalEntry, &MIList->pCaller[i].mList, CALLER_INFO_LIST_ENTRY, mList) {
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("caller:%p  cnt:%4d size:%10d caller:%pS\n",
					pCalEntry->pCaller, pCalEntry->cnt, pCalEntry->size, pCalEntry->pCaller));
			}
		}

	}  else if (show == SHOW_ALL_PKT_INFO) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("show all memaddr\n"));
		for (i = 0; i < HASH_SIZE; i++) {
			DlListForEach(pEntry, &MIList->pHead[i].mList, MEM_INFO_LIST_ENTRY, mList) {
				if (pEntry->MemSize == 0)
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%d-%u: addr = %p, caller is %pS, pLastPos is %pS posidx:0x%x\n",
						i, ++total, pEntry->pMemAddr, pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx));
				else
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%d-%u: addr = %p, size = %u, caller is %pS, pLastPos is %pS posidx:0x%x\n",
						i, ++total, pEntry->pMemAddr, pEntry->MemSize, pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx));
			}
		}
	} else if (show == SHOW_PKT_INFO) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("show mem :%x\n", pCaller));
		for (i = 0; i < HASH_SIZE; i++) {
			DlListForEach(pEntry, &MIList->pHead[i].mList, MEM_INFO_LIST_ENTRY, mList) {
				if ((int)pEntry->pMemAddr == pCaller) {
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("MIList %d-%u: addr = %p, caller is %pS, pLastPos is %pS posidx:0x%x\n",
						i, ++total, pEntry->pMemAddr, pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx));
				}
			}
		}
	} else if (show < MAX_TYPE && show > 0) {
		DlListForEach(pCalEntry, &MIList->pCaller[CallerIdx].mList, CALLER_INFO_LIST_ENTRY, mList) {
			if (pCaller == (int)pCalEntry->pCaller) {
				find = 1;
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("find caller:%x  cnt:%d  caller:%pS\n", pCaller, pCalEntry->cnt, (VOID *)pCaller));
				break;
			}
		}
		if (!find) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("can't find caller:%x. please choose from below:\n", pCaller));
			show = SHOW_PCALLER_INFO;
			goto begin;
		}
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\tiwpriv $(inf_name) show pktmeminfo=[type]-[calleridx]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\t[type]:\n\t%s\t%s\t%s\t%s\t%s", "0: Show alloc num of caller\n",
			"1: Show all the alloc info\n", "2: Show all the alloc info of caller\n",
			"3: Dump alloc data of caller\n", "4: Release all the memory of caller\n"));
	}

	if (show == SHOW_PCALLER_PKT_INFO) {
		for (i = 0; i < HASH_SIZE; i++) {
			DlListForEach(pEntry, &MIList->pHead[i].mList, MEM_INFO_LIST_ENTRY, mList) {
				if (pCaller == (int)pEntry->pCaller) {
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%u: addr = %p, size = %u, caller is %pS, pLastPos is %pS posidx:0x%x\n",
						++total, pEntry->pMemAddr, pEntry->MemSize, pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx));
				}
			}
		}
	} else if (show == DUMP_PCALLER_PKT) {
		int cnt = 10;
		for (i = 0; i < HASH_SIZE; i++) {
			DlListForEach(pEntry, &MIList->pHead[i].mList, MEM_INFO_LIST_ENTRY, mList) {
				if (pCaller == (int)pEntry->pCaller && cnt > 0) {
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%u: addr = %p, size = %u, caller is %pS, pLastPos is %pS posidx:0x%x\n",
						++total, pEntry->pMemAddr, pEntry->MemSize, pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx));
					if (MIList->type == PKTMEMINFO) {
						UCHAR *pPacket = GET_OS_PKT_DATAPTR(pEntry->pMemAddr);
						hex_dump_with_lvl("dump pkt", (pEntry->pMemAddr), 50, 0);
						if ((INT)pPacket > 0X80000000 && (INT)pPacket < 0x87FFFFFF)
							hex_dump_with_lvl("dump data", pPacket, 50, 0);
					} else
						hex_dump_with_lvl("dump mem", (pEntry->pMemAddr), pEntry->MemSize, 0);
					cnt--;
				}
				if (cnt <= 0) {
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cnt:%d\n", cnt));
					goto done;
				}
			}
		}
	} else if (show == RELEASE_PCALLER_PKT) {
		FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;
		ULONG IrqFlags2 = 0;
		MEM_INFO_LIST_ENTRY *pheadEntry = NULL;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("here release pktmeminfo use dev_kfree_skb_any.\n"));
		for (i = 0; i < HASH_SIZE; i++) {
			DlListForEach(pEntry, &MIList->pHead[i].mList, MEM_INFO_LIST_ENTRY, mList) {
				MEM_INFO_LIST_ENTRY *tempEntry = DlListEntry(pEntry->mList.Prev, MEM_INFO_LIST_ENTRY, mList);
				if (pCaller == (int)pEntry->pCaller) {
					if (MIList->type == PKTMEMINFO) {
						/*default the pkt is builded skb*/
						dev_kfree_skb_any(RTPKT_TO_OSPKT(pEntry->pMemAddr));
					} else
						kfree(pEntry->pMemAddr);
					DlListDel(&pEntry->mList);
					MIList->EntryNumber--;
					MIList->CurAlcSize -= pEntry->MemSize;
					OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags2);
					pheadEntry = &pFreeEntrylist->head;
					DlListAddTail(&pheadEntry->mList, &pEntry->mList);
					pFreeEntrylist->EntryNumber += 1;
					OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags2);
					pCalEntry->cnt--;
					pEntry = tempEntry;
				}
			}
		}
	}
done:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("type:%d max allocmem:%u cur allocmem:%u\n", MIList->type, MIList->MaxAlcSize, MIList->CurAlcSize));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("the number of allocated memory = %u free pool entry:%u\n", MIList->EntryNumber, MIList->pFreeEntrylist->EntryNumber));
	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);

}

#endif /* MEM_ALLOC_INFO_SUPPORT */
#endif

