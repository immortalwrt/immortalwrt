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
	client_wds.c

	Abstract:
*/


#ifdef CLIENT_WDS

#include "rt_config.h"

VOID CliWds_ProxyTabInit(
	IN PRTMP_ADAPTER pAd)
{
	INT idx;
	ULONG i;
	NdisAllocateSpinLock(pAd, &pAd->ApCfg.CliWdsTabLock);
	os_alloc_mem(pAd, (UCHAR **)&(pAd->ApCfg.pCliWdsEntryPool), sizeof(CLIWDS_PROXY_ENTRY) * CLIWDS_POOL_SIZE);

	if (pAd->ApCfg.pCliWdsEntryPool) {
		NdisZeroMemory(pAd->ApCfg.pCliWdsEntryPool, sizeof(CLIWDS_PROXY_ENTRY) * CLIWDS_POOL_SIZE);
		initList(&pAd->ApCfg.CliWdsEntryFreeList);

		for (i = 0; i < CLIWDS_POOL_SIZE; i++)
			insertTailList(&pAd->ApCfg.CliWdsEntryFreeList, (RT_LIST_ENTRY *)(pAd->ApCfg.pCliWdsEntryPool + (ULONG)i));
	} else
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Fail to alloc memory for pAd->CommonCfg.pCliWdsEntryPool");

	for (idx = 0; idx < CLIWDS_HASH_TAB_SIZE; idx++)
		initList(&pAd->ApCfg.CliWdsProxyTb[idx]);

	return;
}


VOID CliWds_ProxyTabDestory(
	IN PRTMP_ADAPTER pAd)
{
	INT idx;
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;
	NdisFreeSpinLock(&pAd->ApCfg.CliWdsTabLock);

	for (idx = 0; idx < CLIWDS_HASH_TAB_SIZE; idx++) {
		pCliWdsEntry =
			(PCLIWDS_PROXY_ENTRY)pAd->ApCfg.CliWdsProxyTb[idx].pHead;

		while (pCliWdsEntry) {
			PCLIWDS_PROXY_ENTRY pCliWdsEntryNext = pCliWdsEntry->pNext;
			CliWdsEntyFree(pAd, pCliWdsEntry);
			pCliWdsEntry = pCliWdsEntryNext;
		}
	}

	if (pAd->ApCfg.pCliWdsEntryPool)
		os_free_mem(pAd->ApCfg.pCliWdsEntryPool);

	pAd->ApCfg.pCliWdsEntryPool = NULL;
	return;
}


PCLIWDS_PROXY_ENTRY CliWdsEntyAlloc(
	IN PRTMP_ADAPTER pAd)
{
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;
	RTMP_SEM_LOCK(&pAd->ApCfg.CliWdsTabLock);
	pCliWdsEntry = (PCLIWDS_PROXY_ENTRY)removeHeadList(&pAd->ApCfg.CliWdsEntryFreeList);
	RTMP_SEM_UNLOCK(&pAd->ApCfg.CliWdsTabLock);
	return pCliWdsEntry;
}


VOID CliWdsEntyFree(
	IN PRTMP_ADAPTER pAd,
	IN PCLIWDS_PROXY_ENTRY pCliWdsEntry)
{
	RTMP_SEM_LOCK(&pAd->ApCfg.CliWdsTabLock);
	insertTailList(&pAd->ApCfg.CliWdsEntryFreeList, (RT_LIST_ENTRY *)pCliWdsEntry);
	RTMP_SEM_UNLOCK(&pAd->ApCfg.CliWdsTabLock);
	return;
}

VOID CliWdsEnryFreeAid(
	 IN RTMP_ADAPTER * pAd,
	 IN SHORT Aid)
{
	INT idx;
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;

	for (idx = 0; idx < CLIWDS_HASH_TAB_SIZE; idx++) {
		pCliWdsEntry = (PCLIWDS_PROXY_ENTRY)pAd->ApCfg.CliWdsProxyTb[idx].pHead;
		while (pCliWdsEntry) {
			if (pCliWdsEntry->Aid == Aid) {
				delEntryList(&pAd->ApCfg.CliWdsProxyTb[idx], (RT_LIST_ENTRY *)pCliWdsEntry);
				CliWdsEntyFree(pAd, pCliWdsEntry);
			}
			pCliWdsEntry = pCliWdsEntry->pNext;
		}
	}
	return;
}

static UCHAR *CliWds_ProxyLookupWithAid(RTMP_ADAPTER *pAd, UCHAR *pMac, SHORT Aid)
{
	UINT8 HashId = (*(pMac + 5) & (CLIWDS_HASH_TAB_SIZE - 1));
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;

	pCliWdsEntry = (PCLIWDS_PROXY_ENTRY)pAd->ApCfg.CliWdsProxyTb[HashId].pHead;

	while (pCliWdsEntry) {
		if (MAC_ADDR_EQUAL(pMac, pCliWdsEntry->Addr)) {
			ULONG Now;

			NdisGetSystemUpTime(&Now);
			pCliWdsEntry->LastRefTime = Now;

			if ((pCliWdsEntry->Aid == Aid) && (VALID_UCAST_ENTRY_WCID(pAd, pCliWdsEntry->Aid)))
				return pAd->MacTab.Content[pCliWdsEntry->Aid].Addr;
			else {
				CliWdsEnryFreeAid(pAd, pCliWdsEntry->Aid);
				return NULL;
			}
		}

		pCliWdsEntry = pCliWdsEntry->pNext;
	}

	return NULL;
}


UCHAR *CliWds_ProxyLookup(RTMP_ADAPTER *pAd, UCHAR *pMac)
{
	UINT8 HashId = (*(pMac + 5) & (CLIWDS_HASH_TAB_SIZE - 1));
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;
	pCliWdsEntry = (PCLIWDS_PROXY_ENTRY)pAd->ApCfg.CliWdsProxyTb[HashId].pHead;

	while (pCliWdsEntry) {
		if (MAC_ADDR_EQUAL(pMac, pCliWdsEntry->Addr)) {
			ULONG Now;
			NdisGetSystemUpTime(&Now);
			pCliWdsEntry->LastRefTime = Now;

			if (VALID_UCAST_ENTRY_WCID(pAd, pCliWdsEntry->Aid))
				return pAd->MacTab.Content[pCliWdsEntry->Aid].Addr;
			else
				return NULL;
		}

		pCliWdsEntry = pCliWdsEntry->pNext;
	}

	return NULL;
}


VOID CliWds_ProxyTabUpdate(
	IN PRTMP_ADAPTER pAd,
	IN SHORT Aid,
	IN PUCHAR pMac)
{
	UINT8 HashId = (*(pMac + 5) & (CLIWDS_HASH_TAB_SIZE - 1));
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;

	if (CliWds_ProxyLookupWithAid(pAd, pMac, Aid) != NULL)
		return;

	pCliWdsEntry = CliWdsEntyAlloc(pAd);

	if (pCliWdsEntry) {
		ULONG Now;
		NdisGetSystemUpTime(&Now);
		pCliWdsEntry->Aid = Aid;
		COPY_MAC_ADDR(&pCliWdsEntry->Addr, pMac);
		pCliWdsEntry->LastRefTime = Now;
		pCliWdsEntry->pNext = NULL;
		insertTailList(&pAd->ApCfg.CliWdsProxyTb[HashId], (RT_LIST_ENTRY *)pCliWdsEntry);
	}

	return;
}


VOID CliWds_ProxyTabMaintain(
	IN PRTMP_ADAPTER pAd)
{
	ULONG idx;
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;
	ULONG Now;
	NdisGetSystemUpTime(&Now);

	for (idx = 0; idx < CLIWDS_HASH_TAB_SIZE; idx++) {
		pCliWdsEntry = (PCLIWDS_PROXY_ENTRY)(pAd->ApCfg.CliWdsProxyTb[idx].pHead);

		while (pCliWdsEntry) {
			PCLIWDS_PROXY_ENTRY pCliWdsEntryNext = pCliWdsEntry->pNext;

			if (RTMP_TIME_AFTER(Now, pCliWdsEntry->LastRefTime + (CLI_WDS_ENTRY_AGEOUT * OS_HZ / 1000))) {
				delEntryList(&pAd->ApCfg.CliWdsProxyTb[idx], (RT_LIST_ENTRY *)pCliWdsEntry);
				CliWdsEntyFree(pAd, pCliWdsEntry);
			}

			pCliWdsEntry = pCliWdsEntryNext;
		}
	}

	return;
}

#endif /* CLIENT_WDS */

