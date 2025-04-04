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
#define MEM_HASH_SIZE 256

typedef struct _CALLER_INFO_LIST_ENTRY {
	DL_LIST cList;
	DL_LIST mList;
	INT cnt;
	UINT size;
	VOID *pCaller;
	UINT Line;
} CALLER_INFO_LIST_ENTRY;

typedef struct _MEM_INFO_LIST_ENTRY {
	DL_LIST cList;
	DL_LIST mList;
	UINT32 MemSize;
	VOID *pMemAddr;
	VOID *pCaller;
	VOID *pLastPos;
	CHAR Func[50];
	UINT Line;
	UINT uPosIdx;/*18~31 is used for tokenid*/
	CALLER_INFO_LIST_ENTRY *pCalEntry;
} MEM_INFO_LIST_ENTRY;

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
	MEM_INFO_LIST_ENTRY pMemAddr[MEM_HASH_SIZE];
	INT type;
	NDIS_SPIN_LOCK Lock;
	UINT32 EntryNumber;
	UINT32 CurAlcSize;
	UINT32 MaxAlcSize;
	FREE_LIST_POOL *pFreeEntrylist;
	CALLER_INFO_LIST_ENTRY pCaller[MEM_HASH_SIZE];
} MEM_INFO_LIST;

enum {
	SHOW_PCALLER_INFO,
	SHOW_ACTIVE_PCALLER_INFO,
	SHOW_PCALLER_ALL_PKT_INFO,
	SHOW_PCALLER_PART_PKT_INFO,
	DUMP_PCALLER_PART_PKT,
	RELEASE_PCALLER_ALL_PKT,
	SHOW_PKT_INFO,
	SHOW_ALL_PKT_INFO,
	MAX_TYPE
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

		MTWF_PRINT("%s: allocated new pool\n", __func__);
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

	return (a + b * 3 + c * 5 + d * 7 + e * 11 + f * 13 + g * 17 + h * 19) % MEM_HASH_SIZE;
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

		MTWF_PRINT("%s: begin relese pool num %d\n", __func__,FreeEntrylist.EntryNumber);

		while (!DlListEmpty(&FreeEntrylist.Poolhead.List)) {
			pEntry = DlListFirst(&FreeEntrylist.Poolhead.List, MEM_INFO_LIST_POOL, List);
			DlListDel(&pEntry->List);
			FreeEntrylist.EntryNumber--;
			kfree(pEntry);
		}
		MTWF_PRINT("%s: relese pool num %d\n", __func__,FreeEntrylist.EntryNumber);
	}
}

static UINT32 MIListClear(MEM_INFO_LIST *MIList)
{
	UINT32 i, cnt = 0;
	ULONG IrqFlags = 0;
	ULONG IrqFlags2 = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;
	MEM_INFO_LIST_ENTRY *pheadFreeEntry = NULL;
	FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;

	OS_INT_LOCK(&MIList->Lock, IrqFlags);
	
	MTWF_PRINT("%s: begin relese num %d\n", __func__,FreeEntrylist.EntryNumber);

	for (i = 0; i < MEM_HASH_SIZE; i++) {
		while (!DlListEmpty(&MIList->pMemAddr[i].mList)) {
			pEntry = DlListFirst(&MIList->pMemAddr[i].mList, MEM_INFO_LIST_ENTRY, mList);
			MTWF_PRINT("rm addr = %llx, size = %u, func %s:%d caller:%pS, pLastPos: %pS posidx:0x%x\n",
					(UINT64)pEntry->pMemAddr, pEntry->MemSize, pEntry->Func ,pEntry->Line ,pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx);

			pEntry->pCalEntry->cnt--;
			pEntry->pCalEntry->size -= pEntry->MemSize;
			if (pEntry->pCalEntry->cnt == 0 && pEntry->pCalEntry->size > 0)
				MTWF_PRINT("error addr = %p, size = %u, caller is %pS pCalEntry->size:%x\n",
					 pEntry->pMemAddr, pEntry->MemSize, pEntry->pCaller, pEntry->pCalEntry->size);

			DlListDel(&pEntry->cList);
			DlListDel(&pEntry->mList);
			MIList->EntryNumber--;
			MIList->CurAlcSize -= pEntry->MemSize;
			OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags2);
			pheadFreeEntry = &pFreeEntrylist->head;
			DlListAddTail(&pheadFreeEntry->mList, &pEntry->mList);
			pFreeEntrylist->EntryNumber += 1;
			OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags2);
			cnt++;
		}
	}
	
		MTWF_PRINT("%s: end relese num %d\n", __func__,FreeEntrylist.EntryNumber);
	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	return cnt;
}


static inline VOID MIListInit(MEM_INFO_LIST *MIList)
{
	UINT32 i;
	
	MTWF_PRINT("MIListInit\n");

	NdisZeroMemory(MIList->pCaller, sizeof(MIList->pCaller));

	for (i = 0; i < MEM_HASH_SIZE; i++) {
		DlListInit(&MIList->pMemAddr[i].mList);
		DlListInit(&MIList->pCaller[i].mList);
		DlListInit(&MIList->pMemAddr[i].cList);
		DlListInit(&MIList->pCaller[i].cList);
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
	MEM_INFO_LIST_ENTRY *pEntry = NULL;

	MIListClear(MIList);

	OS_INT_LOCK(&MIList->Lock, IrqFlags);

	for (i = 0; i < MEM_HASH_SIZE; i++) {
		DlListInit(&MIList->pMemAddr[i].mList);
		while (!DlListEmpty(&MIList->pCaller[i].mList)) {
			pCalEntry = DlListFirst(&MIList->pCaller[i].mList, CALLER_INFO_LIST_ENTRY, mList);
			if (pCalEntry->cnt)
				MTWF_PRINT("caller:%llx cnt:%4d size:%10d caller:%pS\n",
						(UINT64)pCalEntry->pCaller, pCalEntry->cnt, pCalEntry->size, pCalEntry->pCaller);

			DlListForEach(pEntry, &pCalEntry->cList, MEM_INFO_LIST_ENTRY, cList) {
				MTWF_PRINT("\taddr = %llx, size = %u, func %s:%d caller:%pS, pLastPos: %pS posidx:0x%x\n",
						(UINT64)pEntry->pMemAddr, pEntry->MemSize, pEntry->Func ,pEntry->Line ,pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx);
			}
			DlListDel(&pCalEntry->mList);
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
	DlListForEach(pEntry, &MIList->pMemAddr[Index].mList, MEM_INFO_LIST_ENTRY, mList) {
		if (pEntry->pMemAddr == pMemAddr) {
			pEntry->pLastPos = pCaller;
			pEntry->uPosIdx |= Idx;
			break;
		}
	}
	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	return pEntry;
}

static inline VOID MIListAddHead(MEM_INFO_LIST *MIList, UINT32 Size, VOID *pMemAddr, const CHAR *Func, UINT Line, VOID *pCaller)
{
	UINT32 MemIndex = HashF(pMemAddr);
	ULONG IrqFlags = 0;
	MEM_INFO_LIST_ENTRY *pEntry;
	MEM_INFO_LIST_ENTRY *pheadMemEntry;
	UINT32 CallerIdx = HashF(pCaller);
	CALLER_INFO_LIST_ENTRY *pCalEntry = NULL;
	CALLER_INFO_LIST_ENTRY *pheadCalEntry;
	INT bFind = 0;

	OS_INT_LOCK(&MIList->Lock, IrqFlags);
	pEntry = GetEntryFromFreeList(MIList);

	if (!pEntry) {
		OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
		MTWF_PRINT("%s: pEntry is null!!!\n", __func__);
		return;
	}

	pEntry->MemSize = Size;
	pEntry->pMemAddr = pMemAddr;
	pEntry->pCaller = pCaller;
	pEntry->Line = Line;
	pEntry->pLastPos = 0;
	pEntry->uPosIdx = 0;
	strlcpy(pEntry->Func, Func, sizeof(pEntry->Func));
	pheadMemEntry = &MIList->pMemAddr[MemIndex];
	pheadCalEntry = &MIList->pCaller[CallerIdx];
	
	DlListInit(&pEntry->cList);
	DlListForEach(pCalEntry, &MIList->pCaller[CallerIdx].mList, CALLER_INFO_LIST_ENTRY, mList) {
		if (pCalEntry->pCaller == pCaller && pCalEntry->Line == pEntry->Line) {
			pCalEntry->cnt++;
			pCalEntry->size += Size;
			pEntry->pCalEntry = pCalEntry;
			bFind = 1;
			DlListAdd(&pCalEntry->cList, &pEntry->cList);

			break;
		}
	}

	if (!bFind) {
		pCalEntry = kmalloc(sizeof(CALLER_INFO_LIST_ENTRY), GFP_ATOMIC);
		pCalEntry->pCaller = pCaller;
		pCalEntry->Line = Line;
		pCalEntry->size = Size;
		pCalEntry->cnt = 1;
		pEntry->pCalEntry = pCalEntry;
		DlListInit(&pCalEntry->mList);
		DlListInit(&pCalEntry->cList);
		DlListAdd(&pheadCalEntry->mList, &pCalEntry->mList);
		DlListAdd(&pCalEntry->cList, &pEntry->cList);
	}
	DlListAdd(&pheadMemEntry->mList, &pEntry->mList);
	MIList->EntryNumber++;
	MIList->CurAlcSize += pEntry->MemSize;

	if (MIList->type == PKTMEMINFO) {
		if (MIList->EntryNumber > MIList->MaxAlcSize)
			MIList->MaxAlcSize = MIList->EntryNumber;
	} else {
		if (MIList->CurAlcSize > MIList->MaxAlcSize)
			MIList->MaxAlcSize = MIList->CurAlcSize;
	}

	//if (pEntry->MemSize == 1700 && !MIList->type)
		//MTWF_PRINT("error addr = %p, size = %u,%s:%d caller is %pS-%pS pCalEntry->size:%d-%d\n",
			 //pEntry->pMemAddr, pEntry->MemSize, pEntry->Func, pEntry->Line, pEntry->pCaller,pCaller, pEntry->pCalEntry->size,pEntry->pCalEntry->cnt);

	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
}

static inline MEM_INFO_LIST_ENTRY *MIListRemove(MEM_INFO_LIST *MIList, VOID *pMemAddr, VOID *pCaller)
{
	UINT32 Index = HashF(pMemAddr);
	ULONG IrqFlags = 0;
	ULONG IrqFlags2 = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;
	MEM_INFO_LIST_ENTRY *pheadFreeEntry = NULL;
	FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;

	OS_INT_LOCK(&MIList->Lock, IrqFlags);
	DlListForEach(pEntry, &MIList->pMemAddr[Index].mList, MEM_INFO_LIST_ENTRY, mList) {
		if (pEntry->pMemAddr == pMemAddr) {
			pEntry->pCalEntry->cnt--;
			pEntry->pCalEntry->size -= pEntry->MemSize;
			if (pEntry->pCalEntry->cnt == 0 && pEntry->pCalEntry->size > 0)
				MTWF_PRINT("error addr = %p, size = %u, caller is %pS-%pS pCalEntry->size:%d-%d\n",
					 pEntry->pMemAddr, pEntry->MemSize, pEntry->pCaller,pCaller, pEntry->pCalEntry->size,pEntry->pCalEntry->cnt);

			DlListDel(&pEntry->cList);
			DlListDel(&pEntry->mList);
			MIList->EntryNumber--;
			MIList->CurAlcSize -= pEntry->MemSize;
			OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags2);
			pheadFreeEntry = &pFreeEntrylist->head;
			DlListAddTail(&pheadFreeEntry->mList, &pEntry->mList);
			pFreeEntrylist->EntryNumber += 1;
			OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags2);

			break;
		}
	}
	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	return pEntry;
}

static inline VOID ShowMIList(MEM_INFO_LIST *MIList, UINT show, UINT64 Caller)
{
	UINT32 i;
	INT bFind = 1;
	ULONG IrqFlags = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;
	CALLER_INFO_LIST_ENTRY *pCalEntry = NULL;
	VOID *pCaller = (VOID *)Caller;
	UINT32 CallerIdx = HashF(pCaller);
	VOID *pMemAddr = pCaller;
	UINT32 MemIndex = CallerIdx;
	
	OS_INT_LOCK(&MIList->Lock, IrqFlags);
	MTWF_PRINT("%s ", MIList->type ? "pktinfo" : "meminfo");
begin:
	switch (show) {
	case SHOW_PCALLER_INFO:
		MTWF_PRINT("show all caller info:\n");
		for (i = 0; i < MEM_HASH_SIZE; i++) {
			DlListForEach(pCalEntry, &MIList->pCaller[i].mList, CALLER_INFO_LIST_ENTRY, mList) {
				MTWF_PRINT("\tcaller:%llx cnt:%4d size:%10d caller:%pS\n",
					(UINT64)pCalEntry->pCaller, pCalEntry->cnt, pCalEntry->size, pCalEntry->pCaller);
			}
		}
		goto showinfo;

	case SHOW_ACTIVE_PCALLER_INFO:
		MTWF_PRINT("show active caller info:\n");
		for (i = 0; i < MEM_HASH_SIZE; i++) {
			DlListForEach(pCalEntry, &MIList->pCaller[i].mList, CALLER_INFO_LIST_ENTRY, mList) {
				if (pCalEntry->cnt)
					MTWF_PRINT("\tcaller:%llx cnt:%4d size:%10d caller:%pS\n",
						(UINT64)pCalEntry->pCaller, pCalEntry->cnt, pCalEntry->size, pCalEntry->pCaller);
			}
		}
		goto showinfo;

	case  SHOW_ALL_PKT_INFO:
		MTWF_PRINT("show all memaddr:\n");
		for (i = 0; i < MEM_HASH_SIZE; i++) {
			DlListForEach(pEntry, &MIList->pMemAddr[i].mList, MEM_INFO_LIST_ENTRY, mList) {
				MTWF_PRINT("\taddr = %llx, size = %4u, func %s:%d caller:%pS, pLastPos: %pS posidx:0x%x\n",
						(UINT64)pEntry->pMemAddr, pEntry->MemSize, pEntry->Func ,pEntry->Line ,pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx);
			}
		}
		break;

	case  SHOW_PKT_INFO:
		MTWF_PRINT("show mem :%llx\n", Caller);
		DlListForEach(pEntry, &MIList->pMemAddr[MemIndex].mList, MEM_INFO_LIST_ENTRY, mList) {
				if (pEntry->pMemAddr == pMemAddr) {
					MTWF_PRINT("\taddr = %llx, size = %4u, func %s:%d caller:%pS-%llx, pLastPos: %pS posidx:0x%x\n",
							(UINT64)pEntry->pMemAddr, pEntry->MemSize, pEntry->Func ,pEntry->Line, pEntry->pCaller, (UINT64)pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx);
				}
		}
		break;

	case SHOW_PCALLER_ALL_PKT_INFO:
	case SHOW_PCALLER_PART_PKT_INFO:
	case DUMP_PCALLER_PART_PKT:
	case RELEASE_PCALLER_ALL_PKT:
		bFind = 0;
		DlListForEach(pCalEntry, &MIList->pCaller[CallerIdx].mList, CALLER_INFO_LIST_ENTRY, mList) {
			if (pCaller == pCalEntry->pCaller) {
				int cnt = 0;
				bFind = 1;

				MTWF_PRINT("find caller:%llx cnt:%4d size:%10d\n",
					(UINT64)pCalEntry->pCaller, pCalEntry->cnt, pCalEntry->size);

				DlListForEach(pEntry, &pCalEntry->cList, MEM_INFO_LIST_ENTRY, cList) {
					MTWF_PRINT("\taddr = %llx, size = %4u, func %s:%d caller:%pS, pLastPos: %pS posidx:0x%x\n",
							(UINT64)pEntry->pMemAddr, pEntry->MemSize, pEntry->Func ,pEntry->Line ,pEntry->pCaller, pEntry->pLastPos, pEntry->uPosIdx);

					switch (show) {
					case DUMP_PCALLER_PART_PKT:
					{
						cnt++;
						if (MIList->type == PKTMEMINFO) {
							UCHAR *pPacket = GET_OS_PKT_DATAPTR(pEntry->pMemAddr);
							hex_dump_with_lvl("\tdump pkt", (pEntry->pMemAddr), 50, 0);
							if ((INT64)pPacket > 0X80000000 && (INT64)pPacket < 0x87FFFFFF)
								hex_dump_with_lvl("\tdump data", pPacket, 50, 0);
						} else {
							hex_dump_with_lvl("\tdump mem", (pEntry->pMemAddr), pEntry->MemSize, 0);
						}
						break;
					}
					case RELEASE_PCALLER_ALL_PKT:
					{
						ULONG IrqFlags2 = 0;
						FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;
						MEM_INFO_LIST_ENTRY *pheadFreeEntry = NULL;
						MEM_INFO_LIST_ENTRY *tempEntry = DlListEntry(pEntry->mList.Prev, MEM_INFO_LIST_ENTRY, mList);
						if (MIList->type == PKTMEMINFO) {
							/*default the pkt is builded skb*/
							dev_kfree_skb_any(RTPKT_TO_OSPKT(pEntry->pMemAddr));
						} else
							kfree(pEntry->pMemAddr);
						DlListDel(&pEntry->mList);
						MIList->EntryNumber--;
						MIList->CurAlcSize -= pEntry->MemSize;
						OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags2);
						pheadFreeEntry = &pFreeEntrylist->head;
						DlListAddTail(&pheadFreeEntry->mList, &pEntry->mList);
						pFreeEntrylist->EntryNumber += 1;
						OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags2);
						pCalEntry->cnt--;
						pEntry = tempEntry;
						break;
					}
					case SHOW_PCALLER_PART_PKT_INFO:
						cnt++;
						break;
					}

					if (cnt >= 15) {
						goto done;
					}
				}
			}
		}
		break;

	default :
		{
			int i;
			UCHAR *sInfo[]={
				"SHOW_PCALLER_INFO",
				"SHOW_ACTIVE_PCALLER_INFO",
				"SHOW_PCALLER_ALL_PKT_INFO",
				"SHOW_PCALLER_PART_PKT_INFO",
				"DUMP_PCALLER_PART_PKT",
				"RELEASE_PCALLER_ALL_PKT",
				"SHOW_PKT_INFO",
				"SHOW_ALL_PKT_INFO",
				"MAX_TYPE"};
			MTWF_PRINT("\tiwpriv $(inf_name) show pktmeminfo=[type]-[calleridx]\n\t[type]:\n");
			for(i = 0; i < MAX_TYPE; i++)
				MTWF_PRINT("\t%d:%s\n", i, sInfo[i]);
			goto showinfo;
		}
	}

	if (!bFind) {
		MTWF_PRINT("can't find caller:%llx. please choose from below:\n", Caller);
		show = SHOW_PCALLER_INFO;
		bFind = 1;
		goto begin;
	}

done:
	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	return;

showinfo:
	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	MTWF_PRINT("%s:cur allocmem:%u max allocmem:%u\n", MIList->type?"pktinfo":"meminfo:", MIList->CurAlcSize, MIList->MaxAlcSize);
	MTWF_PRINT("allocated entry= %u free entry:%u\n", MIList->EntryNumber, MIList->pFreeEntrylist->EntryNumber);
}

#endif /* MEM_ALLOC_INFO_SUPPORT */
#endif

