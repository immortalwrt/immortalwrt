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
    ap_repeater.c

    Abstract:
    Support MAC Repeater function.

    Revision History:
	Who		When              What
    --------------  ----------      ----------------------------------------------
    Arvin				11-16-2012      created
*/

#ifdef MAC_REPEATER_SUPPORT

#include "rt_config.h"
#include "hdev/hdev_basic.h"
#include "hdev/hdev.h"

#define OUI_LEN	3
UCHAR VENDOR_DEFINED_OUI_ADDR[][OUI_LEN] =
	{
{0x02, 0x0C, 0x43},
{0x02, 0x0C, 0xE7},
{0x02, 0x0A, 0x00}
	};
static UCHAR  rept_vendor_def_oui_table_size = (sizeof(VENDOR_DEFINED_OUI_ADDR) / sizeof(UCHAR[OUI_LEN]));

/* IOCTL */
INT Show_ReptTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int CliIdx;
	UINT8 band_idx = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct wifi_dev *wdev = NULL;

	RETURN_ZERO_IF_PAD_NULL(pAd);

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		MTWF_PRINT("Band_%d_RpEn(%d),RpEnByAnyBand(%d),RpEn(%d)\n",
			band_idx,
			repeater_get_enable(pAd, band_idx),
			repeater_enable_by_any_band(pAd),
			pAd->ApCfg.bMACRepeaterEn);
	}

	if (!pAd->ApCfg.bMACRepeaterEn)
		return TRUE;

	MTWF_PRINT("---------------------------------\n");
	MTWF_PRINT("--------pRepeaterCliPool --------\n");
	MTWF_PRINT("---------------------------------\n");
	MTWF_PRINT("\n%-3s%-5s%-4s%-5s%-4s%-4s%-8s%-6s%-5s%-5s%-5s%-5s%-19s%-19s%-19s%-19s%-10s\n",
		   "AP", "Band", "CLI", "WCID", "En", "Vld", "CliType", "Block", "Conn", "CTRL", "AUTH", "ASSO", "REAL_MAC", "FAKE_MAC", "MUAR_MAC", "MUAR_ROOT", "Time");

	for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
		PREPEATER_CLIENT_ENTRY		pReptCliEntry;

		pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		wdev = &pReptCliEntry->wdev;

		MTWF_PRINT("%-3d", wdev->func_idx);
		MTWF_PRINT("%-5d", HcGetBandByWdev(wdev));
		MTWF_PRINT("%-4d", pReptCliEntry->MatchLinkIdx);
		MTWF_PRINT("%-5d", pReptCliEntry->pMacEntry ? pReptCliEntry->pMacEntry->wcid : 0);
		MTWF_PRINT("%-4d", pReptCliEntry->CliEnable);
		MTWF_PRINT("%-4d", pReptCliEntry->CliValid);
		MTWF_PRINT("%-8d", pReptCliEntry->Cli_Type);
		MTWF_PRINT("%-6d", pReptCliEntry->bBlockAssoc);
		MTWF_PRINT("%-5d", pReptCliEntry->CliConnectState);
		MTWF_PRINT("%-5lu", wdev->cntl_machine.CurrState);
		MTWF_PRINT("%-5lu", wdev->auth_machine.CurrState);
		MTWF_PRINT("%-5lu", wdev->assoc_machine.CurrState);
		MTWF_PRINT(""MACSTR"  ", MAC2STR(pReptCliEntry->OriginalAddress));
		MTWF_PRINT(""MACSTR"  ", MAC2STR(pReptCliEntry->OriginalAddress));
		/* read muar cr MAR0,MAR1 */
		{
			/* UINT32	mar_val; */
			RMAC_MAR0_STRUC mar0_val;
			RMAC_MAR1_STRUC mar1_val;

			memset(&mar0_val, 0x0, sizeof(mar0_val));
			memset(&mar1_val, 0x0, sizeof(mar1_val));
			mar1_val.field.access_start = 1;
			mar1_val.field.multicast_addr_index = pReptCliEntry->MatchLinkIdx*2;
			/* Issue a read command */
			HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32)mar1_val.word);

			/* wait acess complete*/
			do {
				HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32 *)&mar1_val);
				/* delay */
			} while (mar1_val.field.access_start == 1);

			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR0, (UINT32 *)&mar0_val);
			MTWF_PRINT("%02x:%02x:%02x:%02x:%02x:%02x  ",
				  (UINT8)(mar0_val.addr_31_0 & 0x000000ff),
				  (UINT8)((mar0_val.addr_31_0 & 0x0000ff00) >> 8),
				  (UINT8)((mar0_val.addr_31_0 & 0x00ff0000) >> 16),
				  (UINT8)((mar0_val.addr_31_0 & 0xff000000) >> 24),
				  (UINT8)mar1_val.field.addr_39_32,
				  (UINT8)mar1_val.field.addr_47_40);
			memset(&mar0_val, 0x0, sizeof(mar0_val));
			memset(&mar1_val, 0x0, sizeof(mar1_val));
			mar1_val.field.access_start = 1;
			mar1_val.field.multicast_addr_index = pReptCliEntry->MatchLinkIdx*2+1;
			/* Issue a read command */
			HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32)mar1_val.word);

			/* wait acess complete*/
			do {
				HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32 *)&mar1_val);
				/* delay */
			} while (mar1_val.field.access_start == 1);

			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR0, (UINT32 *)&mar0_val);
			MTWF_PRINT("%02x:%02x:%02x:%02x:%02x:%02x",
				  (UINT8)(mar0_val.addr_31_0 & 0x000000ff),
				  (UINT8)((mar0_val.addr_31_0 & 0x0000ff00) >> 8),
				  (UINT8)((mar0_val.addr_31_0 & 0x00ff0000) >> 16),
				  (UINT8)((mar0_val.addr_31_0 & 0xff000000) >> 24),
				  (UINT8)mar1_val.field.addr_39_32,
				  (UINT8)mar1_val.field.addr_47_40);
		}
		MTWF_PRINT("  (%ld)\n", pReptCliEntry->CliTriggerTime);
	}

	return TRUE;
}

/* End of IOCTL */

VOID ApCliAuthTimeoutExt(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PREPEATER_CLIENT_ENTRY pRepeaterCliEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev = &pRepeaterCliEntry->wdev;

	ASSERT(wdev);
	pAd = pRepeaterCliEntry->pAd;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			 "Repeater Cli AUTH - AuthTimeout\n");

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			 "ifIndex = %d, CliIdx = %d !!!\n",
			  pRepeaterCliEntry->wdev.func_idx,
			  pRepeaterCliEntry->MatchLinkIdx);
	MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_AUTH_TIMEOUT, 0, NULL, 0, wdev);
	RTMP_MLME_HANDLER(pAd);
}

DECLARE_TIMER_FUNCTION(ApCliAuthTimeoutExt);
BUILD_TIMER_FUNCTION(ApCliAuthTimeoutExt);

/*
    ==========================================================================
    Description:
	Association timeout procedure. After association timeout, this function
	will be called and it will put a message into the MLME queue
    Parameters:
	Standard timer parameters
    ==========================================================================
 */
VOID ApCliAssocTimeoutExt(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PREPEATER_CLIENT_ENTRY pRepeaterCliEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd;
	struct wifi_dev *wdev = &pRepeaterCliEntry->wdev;

	ASSERT(wdev);
	pAd = pRepeaterCliEntry->pAd;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "Repeater Cli ASSOC - enqueue APCLI_MT2_ASSOC_TIMEOUT\n");

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			 "ifIndex = %d, CliIdx = %d !!!\n",
			  pRepeaterCliEntry->wdev.func_idx,
			  pRepeaterCliEntry->MatchLinkIdx);
	MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_ASSOC_TIMEOUT, 0, NULL, 0, wdev);
	RTMP_MLME_HANDLER(pAd);
}


DECLARE_TIMER_FUNCTION(ApCliAssocTimeoutExt);
BUILD_TIMER_FUNCTION(ApCliAssocTimeoutExt);

static VOID ReptCompleteInit(REPEATER_CLIENT_ENTRY *pReptEntry)
{
	RTMP_OS_INIT_COMPLETION(&pReptEntry->free_ack);
}

static VOID ReptLinkDownComplete(REPEATER_CLIENT_ENTRY *pReptEntry)
{
	RTMP_OS_COMPLETE(&pReptEntry->free_ack);
}

VOID ReptWaitLinkDown(REPEATER_CLIENT_ENTRY *pReptEntry)
{
	if (pReptEntry->CliEnable && !RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pReptEntry->free_ack, REPT_WAIT_TIMEOUT)) {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				 "ApCli Rept[%d] can't done.\n", pReptEntry->MatchLinkIdx);
	}
}



VOID CliLinkMapInit(RTMP_ADAPTER *pAd)
{
	UCHAR MbssIdx;
	MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap;
	struct wifi_dev *cli_link_wdev = &pAd->StaCfg[0].wdev;/* default bind to apcli0 */
	struct wifi_dev *mbss_link_wdev;
	int		apcli_idx;

	NdisAcquireSpinLock(&pAd->ApCfg.CliLinkMapLock);

	for (MbssIdx = 0; VALID_MBSS(pAd, MbssIdx); MbssIdx++) {
		mbss_link_wdev = &pAd->ApCfg.MBSSID[MbssIdx].wdev;
		pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[MbssIdx];

		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			for (apcli_idx = 0; apcli_idx < pAd->ApCfg.ApCliNum; apcli_idx++) {
				cli_link_wdev = &pAd->StaCfg[apcli_idx].wdev;

				if (WMODE_CAP_2G(mbss_link_wdev->PhyMode)) { /* 2.4G */
					if (WMODE_CAP_2G(cli_link_wdev->PhyMode)) { /* 2.4G */
						pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
						pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
					}
				} else { /* 5G */
					if (WMODE_CAP_5G(cli_link_wdev->PhyMode)) { /* 5G */
						pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
						pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
					}
				}
			}
		} else {
			pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
			pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
		}
	}

	NdisReleaseSpinLock(&pAd->ApCfg.CliLinkMapLock);
}

VOID repeater_unbind_rp_wdev(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	INT CliIdx = 0;
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL;

	for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
		pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		if (pReptCliEntry->wdev_bound)
			wdev_deinit(pAd, &pReptCliEntry->wdev);
	}
}

VOID RepeaterCtrlInit(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR MaxNumChipRept = GET_MAX_REPEATER_ENTRY_NUM(pChipCap);
	UINT32 Ret = FALSE;
	UCHAR i;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	UINT32 PoolMemSize;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				 "repeater feature already init. by one of bands repeater\n");
		return;
	}

	PoolMemSize = sizeof(REPEATER_CLIENT_ENTRY) * MaxNumChipRept;
	Ret = os_alloc_mem(NULL,
					   (UCHAR **)&pAd->ApCfg.pRepeaterCliPool,
					   PoolMemSize);

	if (Ret != NDIS_STATUS_SUCCESS) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 " Alloc memory for pRepeaterCliPool failed.\n");
		return;
	}

	os_zero_mem(pAd->ApCfg.pRepeaterCliPool, PoolMemSize);
	PoolMemSize = sizeof(REPEATER_CLIENT_ENTRY_MAP) * MaxNumChipRept;
	Ret = os_alloc_mem(NULL,
					   (UCHAR **)&pAd->ApCfg.pRepeaterCliMapPool,
					   PoolMemSize);

	if (Ret != NDIS_STATUS_SUCCESS) {
		if (pAd->ApCfg.pRepeaterCliPool)
			os_free_mem(pAd->ApCfg.pRepeaterCliPool);

		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 " Alloc memory for pRepeaterCliMapPool failed.\n");
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		return;
	}

	os_zero_mem(pAd->ApCfg.pRepeaterCliMapPool, PoolMemSize);

	/*initialize RepeaterEntryPool*/
	for (i = 0; i < MaxNumChipRept; i++) {
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[i];
		pReptEntry->CliConnectState = REPT_ENTRY_DISCONNT;
		pReptEntry->CliDisconnectState = REPT_ENTRY_DISCONNT_STATE_UNKNOWN;
		pReptEntry->CliEnable = FALSE;
		pReptEntry->CliValid = FALSE;
		pReptEntry->wdev_bound = FALSE;
		pReptEntry->pAd = pAd;
		pReptEntry->MatchLinkIdx = i;
		pReptEntry->CliIdx = 0;
		pReptEntry->pMacEntry = NULL;
		pReptEntry->ReptCliIdleCount = 0;
		CLEAN_REPT_CLI_TYPE(pReptEntry); /*Clear client type*/
		ReptCompleteInit(pReptEntry);
		/* RTMPInitTimer(pAd, */
		/* &pReptEntry->ApCliAssocTimer, */
		/* GET_TIMER_FUNCTION(ApCliAssocTimeoutExt), */
		/* pReptEntry, FALSE); */
		/*  */
		/* RTMPInitTimer(pAd, &pReptEntry->ApCliAuthTimer, */
		/* GET_TIMER_FUNCTION(ApCliAuthTimeoutExt), pReptEntry, FALSE); */
	}

	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->ApCfg.RepeaterCliSize[i] = 0;
	os_zero_mem(&pAd->ApCfg.ReptControl, sizeof(REPEATER_CTRL_STRUCT));
	pAd->ApCfg.bMACRepeaterEn = TRUE;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s() is done\n", __func__);
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}

VOID RepeaterCliReset(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	UCHAR MaxNumChipRept = GET_MAX_REPEATER_ENTRY_NUM(pChipCap);
	UCHAR i = 0;

	if (pAd->ApCfg.bMACRepeaterEn != TRUE)
		return;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	/*initialize RepeaterEntryPool*/
	for (i = 0; i < MaxNumChipRept; i++) {
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[i];
		pReptEntry->CliEnable = FALSE;
	}

	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->ApCfg.RepeaterCliSize[i] = 0;
	os_zero_mem(&pAd->ApCfg.ReptControl, sizeof(REPEATER_CTRL_STRUCT));
	os_zero_mem(pAd->ApCfg.ReptCliHash, sizeof(pAd->ApCfg.ReptCliHash));
	os_zero_mem(pAd->ApCfg.ReptMapHash, sizeof(pAd->ApCfg.ReptMapHash));
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s() is done\n", __func__);

	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}


VOID RepeaterCtrlExit(RTMP_ADAPTER *pAd, UCHAR band_idx)
{
	/* TODO: check whole repeater control release. */
	int wait_cnt = 0;
	/*
		Add MacRepeater Entry De-Init Here, and let "iwpriv ra0 set MACRepeaterEn=0"
		can do this instead of "iwpriv apcli0 set ApCliEnable=0"
	*/
	repeater_disconnect_by_band(pAd, band_idx);

	while (pAd->ApCfg.RepeaterCliSize[band_idx] > 0) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				 "wait entry to be deleted\n");
		OS_WAIT(10);
		wait_cnt++;

		if (wait_cnt > 1000)
			break;
	}

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if ((pAd->ApCfg.bMACRepeaterEn == FALSE) || repeater_enable_by_any_band(pAd)) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				 "wrong state(%d,%d)\n",
				 pAd->ApCfg.bMACRepeaterEn,
				 repeater_enable_by_any_band(pAd));
		return;
	}

	repeater_unbind_rp_wdev(pAd);

	pAd->ApCfg.bMACRepeaterEn = FALSE;

	if (pAd->ApCfg.pRepeaterCliMapPool != NULL) {
		os_free_mem(pAd->ApCfg.pRepeaterCliMapPool);
		pAd->ApCfg.pRepeaterCliMapPool = NULL;
	}

	if (pAd->ApCfg.pRepeaterCliPool != NULL) {
		os_free_mem(pAd->ApCfg.pRepeaterCliPool);
		pAd->ApCfg.pRepeaterCliPool = NULL;
	}
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s() is done\n", __func__);
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}

REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry(
	IN PVOID pData,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad)
{
	ULONG HashIdx;
	UCHAR tempMAC[6];
	REPEATER_CLIENT_ENTRY *pEntry = NULL, *pSearchEntry = NULL;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry = NULL;

	COPY_MAC_ADDR(tempMAC, pAddr);
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);

	/* NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock); */
	if (bIsPad == TRUE)
		NdisAcquireSpinLock(&((PRTMP_ADAPTER)pData)->ApCfg.ReptCliEntryLock);
	else
		NdisAcquireSpinLock(((REPEATER_ADAPTER_DATA_TABLE *)pData)->EntryLock);

	if (bRealMAC == TRUE) {
		if (bIsPad == TRUE)
			pMapEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptMapHash[HashIdx];
		else
			pMapEntry = (REPEATER_CLIENT_ENTRY_MAP *)((((REPEATER_ADAPTER_DATA_TABLE *)pData)->MapHash)
						+ HashIdx);

		while (pMapEntry) {
			pSearchEntry = pMapEntry->pReptCliEntry;

			if (pSearchEntry) {
				if (pSearchEntry->CliEnable && MAC_ADDR_EQUAL(pSearchEntry->OriginalAddress, tempMAC)) {
					pEntry = pSearchEntry;
					break;
				}
				pMapEntry = pMapEntry->pNext;
			} else
				pMapEntry = pMapEntry->pNext;
		}
	} else {
		if (bIsPad == TRUE)
			pSearchEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptCliHash[HashIdx];
		else
			pSearchEntry = (REPEATER_CLIENT_ENTRY *)((((REPEATER_ADAPTER_DATA_TABLE *)pData)->CliHash) + HashIdx);

		while (pSearchEntry) {
			if (pSearchEntry->CliEnable && MAC_ADDR_EQUAL(pSearchEntry->CurrentAddress, tempMAC)) {
				pEntry = pSearchEntry;
				break;
			}
			pSearchEntry = pSearchEntry->pNext;
		}
	}

	/* NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock); */
	if (bIsPad == TRUE)
		NdisReleaseSpinLock(&((PRTMP_ADAPTER)pData)->ApCfg.ReptCliEntryLock);
	else
		NdisReleaseSpinLock(((REPEATER_ADAPTER_DATA_TABLE *)pData)->EntryLock);

	return pEntry;
}

BOOLEAN RTMPQueryLookupRepeaterCliEntryMT(
	IN PVOID pData,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad)
{
	REPEATER_CLIENT_ENTRY *pEntry = NULL;

	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG,
			 "%s:: "MACSTR"\n",
			  __func__, MAC2STR(pAddr));
	pEntry = RTMPLookupRepeaterCliEntry(pData, FALSE, pAddr, bIsPad);

	if (pEntry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG,
				 "%s:: not the repeater client\n", __func__);
		return FALSE;
	}
	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG,
			"%s:: is the repeater client\n", __func__);
	return TRUE;
}

REPEATER_CLIENT_ENTRY *lookup_rept_entry(RTMP_ADAPTER *pAd, PUCHAR address)
{
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;

	rept_entry = RTMPLookupRepeaterCliEntry(
					 pAd,
					 FALSE,
					 address,
					 TRUE);

	if (!rept_entry)
		rept_entry = RTMPLookupRepeaterCliEntry(
						 pAd,
						 TRUE,
						 address,
						 TRUE);

	if (rept_entry)
		return rept_entry;

	return NULL;
}

INT ReptGetMuarIdxByCliIdx(RTMP_ADAPTER *pAd, UCHAR CliIdx, UCHAR *muar_idx)
{
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	HD_REPT_ENRTY *pHReptEntry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR ReptOmacIdx;

	if (CliIdx >= GET_MAX_REPEATER_ENTRY_NUM(cap)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
						 "Error CliIdx(%d)\n", CliIdx);

		return 0;
	}

	pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

	pHReptEntry = OcGetRepeaterEntry(pReptEntry->wdev.pHObj, pReptEntry->MatchLinkIdx);
	if (pHReptEntry)
		ReptOmacIdx = pHReptEntry->ReptOmacIdx;
	else
		return FALSE;


	*muar_idx = ((ReptOmacIdx - cap->RepeaterStartIdx) * 2);
	return TRUE;
}

UINT32 ReptTxPktCheckHandler(
	RTMP_ADAPTER *pAd,
	IN struct wifi_dev *cli_link_wdev,
	IN PNDIS_PACKET pPacket,
	OUT UINT16 *pWcid)
{
	PUCHAR pSrcBufVA = NULL;
	STA_TR_ENTRY *tr_entry;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	STA_ADMIN_CONFIG *pApCliEntry = cli_link_wdev->func_dev;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *mbss_wdev = NULL;
	MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap = NULL;
	UINT16 eth_type;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef WSC_AP_SUPPORT
	PWSC_CTRL wsc_control = NULL;

	wsc_control = &cli_link_wdev->WscControl;
#endif /* WSC_AP_SUPPORT */

	pSrcBufVA = RTMP_GET_PKT_SRC_VA(pPacket);

	eth_type = (pSrcBufVA[12] << 8) | pSrcBufVA[13];

#ifdef VLAN_SUPPORT
	if ((pAd->CommonCfg.bEnableVlan) && (eth_type == ETH_TYPE_VLAN))
		eth_type = (pSrcBufVA[16] << 8) | pSrcBufVA[17];
#endif /*VLAN_SUPPORT*/

	pReptEntry = RTMPLookupRepeaterCliEntry(
					 pAd,
					 TRUE,
					 (pSrcBufVA + MAC_ADDR_LEN),
					 TRUE);

	if (pReptEntry  && pReptEntry->CliValid) {
		if ((pReptEntry->main_wdev->func_idx != pApCliEntry->wdev.func_idx)) {
			RepeaterDisconnectRootAP(pAd, pReptEntry, APCLI_DISCONNECT_SUB_REASON_CHANGE_APCLI_IF);
			return INSERT_REPT_ENTRY;
		}

		*pWcid = pReptEntry->pMacEntry->wcid;
		return REPEATER_ENTRY_EXIST;
	}
	/* check SA valid. */
	if (RTMPRepeaterVaildMacEntry(pAd, pSrcBufVA + MAC_ADDR_LEN, HcGetBandByWdev(cli_link_wdev))) {
		tr_entry = &tr_ctl->tr_entry[pApCliEntry->MacTabWCID];

		if ((tr_entry) &&
				(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
			pMacEntry = MacTableLookup(pAd, (pSrcBufVA + MAC_ADDR_LEN));

			if (eth_type == ETH_TYPE_EAPOL)
				return INSERT_REPT_ENTRY_AND_ALLOW;

			if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)) {
				STA_TR_ENTRY *sta_tr_entry;

				sta_tr_entry = &tr_ctl->tr_entry[pMacEntry->wcid];

				if ((sta_tr_entry) &&
						(sta_tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							" wireless client is not ready !!!\n");
					return INSERT_REPT_ENTRY;
				}

				mbss_wdev = pMacEntry->wdev;
				pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[mbss_wdev->func_idx];

				if (pMbssToCliLinkMap->cli_link_wdev->PortSecured != WPA_802_1X_PORT_SECURED
					|| pMbssToCliLinkMap->cli_link_wdev == cli_link_wdev) {
#ifdef WSC_AP_SUPPORT
					if (!((wsc_control->WscConfMode != WSC_DISABLE) &&
						(wsc_control->bWscTrigger == TRUE))) {
#endif /* WSC_AP_SUPPORT  */
						HW_ADD_REPT_ENTRY(pAd, cli_link_wdev, (pSrcBufVA + MAC_ADDR_LEN));
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
							"pMacEntry("MACSTR") connect to mbss idx:%d, use CliLink:%d to RootAP\n",
							MAC2STR((pSrcBufVA + MAC_ADDR_LEN)),
							mbss_wdev->func_idx, cli_link_wdev->func_idx);
						return INSERT_REPT_ENTRY;
#ifdef WSC_AP_SUPPORT
					} else {
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
								"WPS is triggered now, don't add entry\n");
					}
#endif /* WSC_AP_SUPPORT  */
				}
			} else
				/*SA is not in mac table, pkt should from upper layer or eth.*/
			{
				/*
TODO: Carter, if more than one apcli/sta,
the eth pkt or upper layer pkt connecting rule should be refined.
*/
#ifdef WSC_AP_SUPPORT
				if (!((wsc_control->WscConfMode != WSC_DISABLE) &&
					(wsc_control->bWscTrigger == TRUE))) {
#endif /* WSC_AP_SUPPORT  */
					HW_ADD_REPT_ENTRY(pAd, cli_link_wdev, (pSrcBufVA + MAC_ADDR_LEN));
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
						"pAddr("MACSTR") use CliLink:%d to RootAP\n",
						MAC2STR((pSrcBufVA + MAC_ADDR_LEN)),
						cli_link_wdev->func_idx);
					return INSERT_REPT_ENTRY;
#ifdef WSC_AP_SUPPORT
				} else {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							"WPS is triggered now, don't add entry\n");
				}
#endif /* WSC_AP_SUPPORT  */
			}
		}
	}

	return USE_CLI_LINK_INFO;
}

/* Sync Phy capability, Security, and sys op */
VOID RepeaterSyncWdevWithMainSta(
	struct wifi_dev *rept_wdev,
	struct wifi_dev *main_wdev)
{
	rept_wdev->wdev_ops = main_wdev->wdev_ops;
	rept_wdev->wpf_cfg = main_wdev->wpf_cfg;
	rept_wdev->wpf_op = main_wdev->wpf_op;
	rept_wdev->pHObj = main_wdev->pHObj;
	/*update_att_from_wdev(rept_wdev, main_wdev);*/
	NdisCopyMemory(&rept_wdev->SecConfig, &main_wdev->SecConfig, sizeof(struct _SECURITY_CONFIG));
	COPY_MAC_ADDR(rept_wdev->bssid, main_wdev->bssid);
	rept_wdev->if_up_down_state = main_wdev->if_up_down_state;
	NdisCopyMemory(&rept_wdev->DevInfo, &main_wdev->DevInfo, sizeof(struct _DEV_INFO_CTRL_T));
	rept_wdev->sync_fsm_ops = main_wdev->sync_fsm_ops;
	rept_wdev->assoc_api = main_wdev->assoc_api;
	rept_wdev->auth_api = main_wdev->auth_api;
	rept_wdev->cntl_api = main_wdev->cntl_api;
	rept_wdev->pDot11_H = main_wdev->pDot11_H;
	rept_wdev->PhyMode = main_wdev->PhyMode;
	rept_wdev->channel = main_wdev->channel;
	NdisCopyMemory(&rept_wdev->rate, &main_wdev->rate, sizeof(struct dev_rate_info));
	NdisCopyMemory(&rept_wdev->bss_info_argument, &main_wdev->bss_info_argument, sizeof(struct _BSS_INFO_ARGUMENT_T));

	/* Rate */
	NdisCopyMemory(&rept_wdev->DesiredHtPhyInfo, &main_wdev->DesiredHtPhyInfo, sizeof(RT_PHY_INFO));
	NdisCopyMemory(&rept_wdev->DesiredTransmitSetting, &main_wdev->DesiredTransmitSetting, sizeof(DESIRED_TRANSMIT_SETTING));
	rept_wdev->bAutoTxRateSwitch = main_wdev->bAutoTxRateSwitch;

	/* vlan */
	rept_wdev->bVLAN_Tag = main_wdev->bVLAN_Tag;
	rept_wdev->VLAN_Policy[0] = main_wdev->VLAN_Policy[0];
	rept_wdev->VLAN_Policy[1] = main_wdev->VLAN_Policy[1];
	rept_wdev->VLAN_Priority = main_wdev->VLAN_Priority;
	rept_wdev->VLAN_VID = main_wdev->VLAN_VID;

	/* Check the following code is necessary */
	rept_wdev->BssIdx = main_wdev->BssIdx;
	rept_wdev->hw_bssid_idx = main_wdev->hw_bssid_idx;
	rept_wdev->tr_tb_idx = main_wdev->tr_tb_idx;
	rept_wdev->OpStatusFlags = main_wdev->OpStatusFlags;
	rept_wdev->forbid_data_tx = main_wdev->forbid_data_tx;
	rept_wdev->bWmmCapable = main_wdev->bWmmCapable;
}

BOOLEAN RepeaterInitWdev(
	PRTMP_ADAPTER pAd,
	REPEATER_CLIENT_ENTRY *rept_cli,
	struct wifi_dev *main_wdev,
	struct wifi_dev *rept_wdev)
{
	BOOLEAN ret = TRUE;

	if (!rept_cli->wdev_bound) {
		NdisZeroMemory(rept_wdev, sizeof(struct wifi_dev));
		ret = wdev_init(pAd, rept_wdev, WDEV_TYPE_REPEATER,
						main_wdev->if_dev, rept_cli->CliIdx,
						(VOID *)rept_cli, (VOID *)pAd);

		if (ret == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 " Init repeater wdev failed!\n");
			return FALSE;
		}
		rept_cli->wdev_bound = TRUE;
	} else {
		wdev_init_for_bound_wdev(rept_wdev, WDEV_TYPE_REPEATER,
								main_wdev->if_dev, rept_cli->CliIdx,
								(VOID *)rept_cli, (VOID *)pAd);
	}

	RepeaterSyncWdevWithMainSta(rept_wdev, main_wdev);

	/* Assign rept related wdev param */
	wdev_fsm_init(rept_wdev);
	rept_wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	rept_wdev->SecConfig.Handshake.WpaState = AS_INITPSK;
	COPY_MAC_ADDR(rept_wdev->if_addr, rept_cli->CurrentAddress);
	HcAddRepeaterEntry(rept_wdev);
	rept_wdev->OmacIdx = HcGetRepeaterOmac(rept_wdev);

	return ret;
}

REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry_NoLock(
	IN PVOID pData,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad)
{
	ULONG HashIdx;
	UCHAR tempMAC[6];
	REPEATER_CLIENT_ENTRY *pEntry = NULL;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry = NULL;

	COPY_MAC_ADDR(tempMAC, pAddr);
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);

	/* NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock); */

	if (bRealMAC == TRUE) {
		if (bIsPad == TRUE)
			pMapEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptMapHash[HashIdx];
		else
			pMapEntry = *((((REPEATER_ADAPTER_DATA_TABLE *)pData)->MapHash) + HashIdx);

		while (pMapEntry) {
			pEntry = pMapEntry->pReptCliEntry;

			if (pEntry) {
				if (pEntry->CliEnable && MAC_ADDR_EQUAL(pEntry->OriginalAddress, tempMAC))
					break;
				pEntry = NULL;
				pMapEntry = pMapEntry->pNext;
			} else
				pMapEntry = pMapEntry->pNext;
		}
	} else {
		if (bIsPad == TRUE)
			pEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptCliHash[HashIdx];
		else
			pEntry = *((((REPEATER_ADAPTER_DATA_TABLE *)pData)->CliHash) + HashIdx);

		while (pEntry) {
			if (pEntry->CliEnable && MAC_ADDR_EQUAL(pEntry->CurrentAddress, tempMAC))
				break;
			pEntry = pEntry->pNext;
		}
	}

	return pEntry;
}
BOOLEAN RepeaterAssignMacAddress(
	PRTMP_ADAPTER pAd,
	UINT8 mode,
	CHAR *cli_addr,
	CHAR *rept_addr)
{
	INT idx;
	UCHAR tempMAC[MAC_ADDR_LEN];

	COPY_MAC_ADDR(tempMAC, cli_addr);

	switch (mode) {
	case CASUALLY_DEFINE_MAC_ADDR:
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			 "todo !!!\n");
		break;

	case VENDOR_DEFINED_MAC_ADDR_OUI:
	{
		INT IdxToUse = 0, i;
		UCHAR flag = 0;
		UCHAR checkMAC[MAC_ADDR_LEN];

		COPY_MAC_ADDR(checkMAC, cli_addr);

		for (idx = 0; idx < rept_vendor_def_oui_table_size; idx++) {
			if (RTMPEqualMemory(VENDOR_DEFINED_OUI_ADDR[idx], cli_addr, OUI_LEN)) {
				if (idx < rept_vendor_def_oui_table_size - 1) {
					NdisCopyMemory(checkMAC,
						VENDOR_DEFINED_OUI_ADDR[idx+1], OUI_LEN);
					for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					if (MAC_ADDR_EQUAL(
						pAd->ApCfg.MBSSID[i].wdev.if_addr,
						checkMAC)) {
						flag = 1;
						break;
					}
					}
					if (i >= pAd->ApCfg.BssidNum) {
						IdxToUse = idx+1;
						break;
					}
				}
			} else if (flag == 1) {
				NdisCopyMemory(checkMAC, VENDOR_DEFINED_OUI_ADDR[idx], OUI_LEN);

				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					if (MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[i].wdev.if_addr, checkMAC))
						break;
				}

				if (i >= pAd->ApCfg.BssidNum) {
					IdxToUse = idx;
					break;
				}
			}
		}

		NdisCopyMemory(tempMAC, VENDOR_DEFINED_OUI_ADDR[IdxToUse], OUI_LEN);

		break;
	}

	default:
		/* no change */
		break;
	}

	if (RTMPLookupRepeaterCliEntry_NoLock(pAd, FALSE, tempMAC, TRUE) != NULL) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				"ReptCLI duplicate Insert "MACSTR" !\n",
				MAC2STR(tempMAC));
		return FALSE;
	}
	COPY_MAC_ADDR(rept_addr, tempMAC);
	return TRUE;

}

VOID RTMPInsertRepeaterEntry(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *main_sta_wdev,
	PUCHAR pAddr)
{
	INT CliIdx;
	UCHAR HashIdx;
	BOOLEAN Ret;
	/* BOOLEAN Cancelled; */
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL, pCurrEntry = NULL;
	INT pValid_ReptCliIdx;
	PREPEATER_CLIENT_ENTRY_MAP pReptCliMap;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 band_idx = HcGetBandByWdev(main_sta_wdev);

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			 "%s.\n", __func__);
	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if (pAd->ApCfg.RepeaterCliSize[band_idx] >= GET_PER_BAND_MAX_REPEATER_ENTRY_NUM(cap)) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 "band(%d) Repeater Client Full !!!\n", band_idx);
		return;
	}

	pValid_ReptCliIdx = GET_MAX_REPEATER_ENTRY_NUM(cap);

	for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
		pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

		if ((pReptCliEntry->CliEnable) &&
			(MAC_ADDR_EQUAL(pReptCliEntry->OriginalAddress, pAddr) ||
			(pAd->ApCfg.MACRepeaterOuiMode != VENDOR_DEFINED_MAC_ADDR_OUI
			&& MAC_ADDR_EQUAL(pReptCliEntry->CurrentAddress, pAddr)))
		   ) {
			NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG,
					 "\n  receive mac :"MACSTR" !!!\n",
					  MAC2STR(pAddr));
			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG,
					 " duplicate Insert !!!\n");
			return;
		}

		if ((pReptCliEntry->CliEnable == FALSE) && (pValid_ReptCliIdx == GET_MAX_REPEATER_ENTRY_NUM(cap)))
			pValid_ReptCliIdx = CliIdx;
	}

	CliIdx = pValid_ReptCliIdx;

	if (CliIdx >= GET_MAX_REPEATER_ENTRY_NUM(cap)) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 "Repeater Pool Full !!!\n");
		return;
	}

	pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
	pReptCliMap = &pAd->ApCfg.pRepeaterCliMapPool[CliIdx];
	/* ENTRY PREEMPTION: initialize the entry */
	/* timer init */
	RTMPInitTimer(pAd,
				  &pReptCliEntry->ApCliAssocTimer,
				  GET_TIMER_FUNCTION(ApCliAssocTimeoutExt),
				  pReptCliEntry, FALSE);
	/* timer init */
	RTMPInitTimer(pAd, &pReptCliEntry->ApCliAuthTimer,
				  GET_TIMER_FUNCTION(ApCliAuthTimeoutExt), pReptCliEntry, FALSE);
	pReptCliEntry->CliConnectState = REPT_ENTRY_DISCONNT;
	pReptCliEntry->CliDisconnectState = REPT_ENTRY_DISCONNT_STATE_UNKNOWN;
	pReptCliEntry->LinkDownReason = APCLI_LINKDOWN_NONE;
	pReptCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_NONE;
	pReptCliEntry->CliValid = FALSE;
	pReptCliEntry->CliIdx = CliIdx;
	pReptCliEntry->pMacEntry = NULL;
	pReptCliEntry->ReptCliIdleCount = 0;
	CLEAN_REPT_CLI_TYPE(pReptCliEntry); /*Clear client type*/
#ifdef FAST_EAPOL_WAR

	if (pReptCliEntry->pre_entry_alloc == TRUE)
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Unexpected condition,check it (pReptCliEntry->pre_entry_alloc=%d)\n", pReptCliEntry->pre_entry_alloc);

	pReptCliEntry->pre_entry_alloc = FALSE;
#endif /* FAST_EAPOL_WAR */
	pReptCliEntry->AuthReqCnt = 0;
	pReptCliEntry->AssocReqCnt = 0;
	pReptCliEntry->CliTriggerTime = 0;
	pReptCliEntry->pNext = NULL;
	pReptCliEntry->main_wdev = main_sta_wdev;
	pReptCliMap->pReptCliEntry = pReptCliEntry;
#ifdef DOT11_SAE_SUPPORT
	pReptCliEntry->sae_cfg_group = pAd->StaCfg[main_sta_wdev->func_idx].sae_cfg_group;
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	NdisAllocateSpinLock(pAd, &pReptCliEntry->SavedPMK_lock);
#endif

	pReptCliMap->pNext = NULL;
	COPY_MAC_ADDR(pReptCliEntry->OriginalAddress, pAddr);

	Ret = RepeaterAssignMacAddress(pAd,
							pAd->ApCfg.MACRepeaterOuiMode,
							pAddr,
							pReptCliEntry->CurrentAddress);
	if (Ret == FALSE)
		return;

	pReptCliEntry->CliEnable = TRUE;
	pReptCliEntry->CliConnectState = REPT_ENTRY_CONNTING;
	pReptCliEntry->pNext = NULL;
	NdisGetSystemUpTime(&pReptCliEntry->CliTriggerTime);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
		"repeater_entry_time(%ld),CliIdx=%d\n",
		pReptCliEntry->CliTriggerTime, CliIdx);
	pAd->ApCfg.RepeaterCliSize[band_idx]++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	RepeaterInitWdev(pAd, pReptCliEntry, main_sta_wdev, &pReptCliEntry->wdev);

	AsicInsertRepeaterEntry(pAd, CliIdx, pReptCliEntry->CurrentAddress);

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	HashIdx = MAC_ADDR_HASH_INDEX(pReptCliEntry->CurrentAddress);
	if (pAd->ApCfg.ReptCliHash[HashIdx] == NULL)
		pAd->ApCfg.ReptCliHash[HashIdx] = pReptCliEntry;
	else {
		pCurrEntry = pAd->ApCfg.ReptCliHash[HashIdx];

		while (pCurrEntry->pNext != NULL)
			pCurrEntry = pCurrEntry->pNext;

		pCurrEntry->pNext = pReptCliEntry;
	}

	HashIdx = MAC_ADDR_HASH_INDEX(pReptCliEntry->OriginalAddress);
	if (pAd->ApCfg.ReptMapHash[HashIdx] == NULL)
		pAd->ApCfg.ReptMapHash[HashIdx] = pReptCliMap;
	else {
		PREPEATER_CLIENT_ENTRY_MAP pCurrMapEntry;

		pCurrMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];

		while (pCurrMapEntry->pNext != NULL)
			pCurrMapEntry = pCurrMapEntry->pNext;

		pCurrMapEntry->pNext = pReptCliMap;
	}
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);


	/* rept auth start */
	cntl_join_start_conf(&pReptCliEntry->wdev, MLME_SUCCESS);
#ifdef MTFWD
	RtmpOSWrielessEventSend(pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				FWD_CMD_ADD_TX_SRC,
				NULL,
				pReptCliEntry->CurrentAddress,
				MAC_ADDR_LEN);
#endif
}

VOID RTMPRemoveRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx)
{
	USHORT HashIdx;
	REPEATER_CLIENT_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry, *pPrevMapEntry, *pProbeMapEntry;
	BOOLEAN bVaild = TRUE;
	BOOLEAN Cancelled;
	UCHAR band_idx = 0;
#ifdef MTFWD
	PNET_DEV if_dev;
	UCHAR CurrentAddress[MAC_ADDR_LEN];
#endif /* MTFWD */

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
		"CliIdx=%d\n", CliIdx);
	AsicRemoveRepeaterEntry(pAd, CliIdx);
	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	pEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

	/* move NULL check here, to prevent pEntry NULL dereference */
	if (pEntry == NULL) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 "pEntry is NULL !!!\n");
		return;
	}

	band_idx = HcGetBandByWdev(&pEntry->wdev);

	if (pEntry->CliEnable == FALSE) {
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 "CliIdx:%d Enable is FALSE already\n",
				  CliIdx);
		return;
	}
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	NdisFreeSpinLock(&pEntry->SavedPMK_lock);
#endif
	if (pEntry->pMacEntry)
		mac_entry_delete(pAd, pEntry->pMacEntry);


	/*Release OMAC Idx*/
	HcDelRepeaterEntry(&pEntry->wdev);
	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->CurrentAddress);
	pPrevEntry = NULL;
	pProbeEntry = pAd->ApCfg.ReptCliHash[HashIdx];
	ASSERT(pProbeEntry);

	if (pProbeEntry == NULL) {
		bVaild = FALSE;
		goto done;
	}

	if (pProbeEntry != NULL) {
		/* update Hash list*/
		do {
			if (pProbeEntry == pEntry) {
				if (pPrevEntry == NULL)
					pAd->ApCfg.ReptCliHash[HashIdx] = pEntry->pNext;
				else
					pPrevEntry->pNext = pEntry->pNext;

				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);
	}

	/* not found !!!*/
	ASSERT(pProbeEntry != NULL);

	if (pProbeEntry == NULL) {
		bVaild = FALSE;
		goto done;
	}

	pMapEntry = &pAd->ApCfg.pRepeaterCliMapPool[CliIdx];
	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->OriginalAddress);
	pPrevMapEntry = NULL;
	pProbeMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];
	ASSERT(pProbeMapEntry);

	if (pProbeMapEntry != NULL) {
		/* update Hash list*/
		do {
			if (pProbeMapEntry == pMapEntry) {
				if (pPrevMapEntry == NULL)
					pAd->ApCfg.ReptMapHash[HashIdx] = pMapEntry->pNext;
				else
					pPrevMapEntry->pNext = pMapEntry->pNext;

				break;
			}

			pPrevMapEntry = pProbeMapEntry;
			pProbeMapEntry = pProbeMapEntry->pNext;
		} while (pProbeMapEntry);
	}

	/* not found !!!*/
	ASSERT(pProbeMapEntry != NULL);
done:
	RTMPReleaseTimer(&pEntry->ApCliAuthTimer, &Cancelled);
	RTMPReleaseTimer(&pEntry->ApCliAssocTimer, &Cancelled);
#ifdef FAST_EAPOL_WAR

	if (pEntry->pre_entry_alloc == TRUE)
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Unexpected condition,check it (pEntry->pre_entry_alloc=%d)\n", pEntry->pre_entry_alloc);

	pEntry->pre_entry_alloc = FALSE;
#endif /* FAST_EAPOL_WAR */

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
		"real="MACSTR",fake="MACSTR"\n",
		MAC2STR(pEntry->OriginalAddress), MAC2STR(pEntry->CurrentAddress));

	pEntry->CliConnectState = REPT_ENTRY_DISCONNT;
	pEntry->CliDisconnectState = REPT_ENTRY_DISCONNT_STATE_UNKNOWN;
	pEntry->CliValid = FALSE;
	pEntry->CliEnable = FALSE;
	pEntry->CliIdx = 0;
	pEntry->pMacEntry = NULL;
	pEntry->ReptCliIdleCount = 0;
	CLEAN_REPT_CLI_TYPE(pEntry);
	pEntry->bBlockAssoc = FALSE;
#ifdef MTFWD
	if_dev = pEntry->wdev.if_dev;
	COPY_MAC_ADDR(CurrentAddress, pEntry->CurrentAddress);
#endif /* MTFWD */
	COPY_MAC_ADDR(pEntry->OriginalAddress, ZERO_MAC_ADDR);
	COPY_MAC_ADDR(pEntry->CurrentAddress, ZERO_MAC_ADDR);

	if (bVaild == TRUE)
		pAd->ApCfg.RepeaterCliSize[band_idx]--;

	ReptLinkDownComplete(pEntry);
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
#ifdef MTFWD
	MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Remove MacRep Sta:"MACSTR"\n", MAC2STR(CurrentAddress));
	RtmpOSWrielessEventSend(if_dev,
				RT_WLAN_EVENT_CUSTOM,
				FWD_CMD_DEL_TX_SRC,
				NULL,
				CurrentAddress,
				MAC_ADDR_LEN);
#endif
}

VOID RTMPRepeaterReconnectionCheck(
	IN PRTMP_ADAPTER pAd)
{
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	INT i;
	PCHAR	pApCliSsid, pApCliCfgSsid;
	UCHAR	CfgSsidLen;
	NDIS_802_11_SSID Ssid;
	ULONG timeDiff[MAX_APCLI_NUM];
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;

	if (pAd->ApCfg.bMACRepeaterEn &&
		pAd->ApCfg.MACRepeaterOuiMode == VENDOR_DEFINED_MAC_ADDR_OUI) {
		for (i = 0; i < MAX_APCLI_NUM; i++) {
			wdev = &pAd->StaCfg[i].wdev;

			if (!wdev->DevInfo.WdevActive)
				continue;

			ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

			if (!APCLI_IF_UP_CHECK(pAd, i) ||
					(pAd->StaCfg[i].ApcliInfStat.Enable == FALSE))
				continue;

			if (ScanCtrl->PartialScan.bScanning == TRUE)
				continue;

			MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG, " %s(): i=%d,%d,%d,%d,%d,%d\n",
					 __func__, (int)i,
					 (int)scan_in_run_state(pAd, wdev),
					 (int)pAd->StaCfg[i].ApCliAutoConnectRunning,
					 (int)pAd->StaCfg[i].ApCliAutoConnectType,
					 (int)pAd->ApCfg.bPartialScanEnable[i],
					 (int)(pAd->Mlme.OneSecPeriodicRound%23));

			if (scan_in_run_state(pAd, wdev))
				continue;
			if (pAd->StaCfg[i].ApCliAutoConnectRunning != FALSE)
				continue;
			if (pAd->StaCfg[i].ApcliInfStat.AutoConnectFlag == FALSE)
				continue;
			pApCliSsid = pAd->StaCfg[i].Ssid;
			pApCliCfgSsid = pAd->StaCfg[i].CfgSsid;
			CfgSsidLen = pAd->StaCfg[i].CfgSsidLen;

			if (((GetAssociatedAPByWdev(pAd, wdev) == NULL) ||
				 !NdisEqualMemory(pApCliSsid, pApCliCfgSsid, CfgSsidLen)) &&
					pAd->StaCfg[i].CfgSsidLen > 0) {
				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->ApCfg.ApCliIssueScanTime[i]))
					timeDiff[i] = (pAd->Mlme.Now32 - pAd->ApCfg.ApCliIssueScanTime[i]);
				else
					timeDiff[i] = (pAd->ApCfg.ApCliIssueScanTime[i] - pAd->Mlme.Now32);
				/* will trigger scan after 23 sec */
				if (timeDiff[i] <= RTMPMsecsToJiffies(23000))
					continue;

				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
						"Scan channels for AP (%s), pApEntry(%p)\n",
						 pApCliCfgSsid, GetAssociatedAPByWdev(pAd, wdev));
				pAd->StaCfg[i].ApCliAutoConnectRunning = TRUE;
				if (pAd->ApCfg.bPartialScanEnable[i]) {
					pAd->ApCfg.bPartialScanning[i] = TRUE;
					ScanCtrl->PartialScan.pwdev = wdev;
					ScanCtrl->PartialScan.bScanning = TRUE;
				}
				Ssid.SsidLength = CfgSsidLen;
				NdisCopyMemory(Ssid.Ssid, pApCliCfgSsid, CfgSsidLen);
				NdisGetSystemUpTime(&pAd->ApCfg.ApCliIssueScanTime[i]);
				ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pAd->StaCfg[i].wdev);
			}
		}
	}

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
}

BOOLEAN RTMPRepeaterVaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr,
	IN UCHAR band_idx)
{
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->ApCfg.RepeaterCliSize[band_idx] >= GET_PER_BAND_MAX_REPEATER_ENTRY_NUM(cap))
		return FALSE;

	if (IS_MULTICAST_MAC_ADDR(pAddr))
		return FALSE;

	if (IS_BROADCAST_MAC_ADDR(pAddr))
		return FALSE;

	pEntry = RepeaterInvaildMacLookup(pAd, pAddr);

	if (pEntry)
		return FALSE;
	else
		return TRUE;
}

INVAILD_TRIGGER_MAC_ENTRY *RepeaterInvaildMacLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	ULONG HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx];

	while (pEntry) {
		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr))
			break;
		pEntry = pEntry->pNext;
	}

	if (pEntry && pEntry->bInsert)
		return pEntry;
	else
		return NULL;
}

VOID InsertIgnoreAsRepeaterEntryTable(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	UCHAR HashIdx, idx = 0;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pCurrEntry = NULL;

	if (pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntrySize >= MAX_IGNORE_AS_REPEATER_ENTRY_NUM)
		return;

	if (MAC_ADDR_EQUAL(pAddr, ZERO_MAC_ADDR))
		return;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	for (idx = 0; idx < MAX_IGNORE_AS_REPEATER_ENTRY_NUM; idx++) {
		pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[idx];

		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr)) {
			if (pEntry->bInsert) {
				NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
				return;
			}
		}

		/* pick up the first available vacancy*/
		if (pEntry->bInsert == FALSE) {
			COPY_MAC_ADDR(pEntry->MacAddr, pAddr);
			pEntry->entry_idx = idx;
			pEntry->bInsert = TRUE;
			break;
		}
	}

	/* add this entry into HASH table */
	if (pEntry) {
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		pEntry->pNext = NULL;

		if (pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx] == NULL)
			pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx] = pEntry;
		else {
			pCurrEntry = pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx];

			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;

			pCurrEntry->pNext = pEntry;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, " Store Invaild MacAddr = "MACSTR". !!!\n",
			 MAC2STR(pEntry->MacAddr));
	pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntrySize++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}

BOOLEAN RepeaterRemoveIngoreEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR idx,
	IN PUCHAR pAddr)
{
	USHORT HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pPrevEntry, *pProbeEntry;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[idx];

	if (pEntry && pEntry->bInsert) {
		pPrevEntry = NULL;
		pProbeEntry = pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx];
		ASSERT(pProbeEntry);

		if (pProbeEntry != NULL) {
			/* update Hash list*/
			do {
				if (pProbeEntry == pEntry) {
					if (pPrevEntry == NULL)
						pAd->ApCfg.ReptControl.IgnoreAsRepeaterHash[HashIdx] = pEntry->pNext;
					else
						pPrevEntry->pNext = pEntry->pNext;

					break;
				}

				pPrevEntry = pProbeEntry;
				pProbeEntry = pProbeEntry->pNext;
			} while (pProbeEntry);
		}

		/* not found !!!*/
		ASSERT(pProbeEntry != NULL);
		pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntrySize--;
	}

	NdisZeroMemory(pEntry->MacAddr, MAC_ADDR_LEN);
	pEntry->bInsert = FALSE;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	return TRUE;
}

VOID RepeaterLinkMonitor(RTMP_ADAPTER *pAd)
{
	REPEATER_CLIENT_ENTRY *ReptPool = pAd->ApCfg.pRepeaterCliPool;
	REPEATER_CLIENT_ENTRY *pReptCliEntry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 Wcid = 0;
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR CliIdx;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	STA_ADMIN_CONFIG *pApCliEntry = NULL;
#endif
	if ((pAd->ApCfg.bMACRepeaterEn) && (ReptPool != NULL)) {
		for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
			pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			if(pReptCliEntry->main_wdev != NULL)
				pApCliEntry = &pAd->StaCfg[pReptCliEntry->main_wdev->func_idx];
			else
				continue;
#endif
			if (pReptCliEntry->CliEnable && pReptCliEntry->pMacEntry) {
				Wcid = pReptCliEntry->pMacEntry->wcid;
				tr_entry = &tr_ctl->tr_entry[Wcid];

				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					"repeater_entry(%d),p_sec(%d),wcid(%d),En(%d),Vld(%d)\n",
					pReptCliEntry->CliIdx, tr_entry->PortSecured, Wcid, pReptCliEntry->CliEnable, pReptCliEntry->CliValid);


				if (tr_entry != NULL) {
					if ((tr_entry->PortSecured != WPA_802_1X_PORT_SECURED) && (
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
					((IS_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap) || IS_AKM_OWE(pApCliEntry->MlmeAux.AKMMap)) &&
					RTMP_TIME_AFTER(pAd->Mlme.Now32, (pReptCliEntry->CliTriggerTime + (30 * OS_HZ)))) ||
#endif
					(RTMP_TIME_AFTER(pAd->Mlme.Now32, (pReptCliEntry->CliTriggerTime + (10 * OS_HZ)))))) {

					if (!IS_REPT_LINK_UP(pReptCliEntry)) {
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							"repeater_entry(%d),time(%ld),allocated->not linkup->overtime->del rp_entry\n",
							pReptCliEntry->CliIdx, pReptCliEntry->CliTriggerTime);
						HW_REMOVE_REPT_ENTRY(pAd, CliIdx);
					} else {
						if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
							continue;

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
						if (IS_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap) || IS_AKM_OWE(pApCliEntry->MlmeAux.AKMMap)) {
							UCHAR pmkid[LEN_PMKID];
							UCHAR pmk[LEN_PMK];
							INT CachedIdx;
							UCHAR ifIndex = pApCliEntry->wdev.func_idx;
							struct wifi_dev *wdev = &pReptCliEntry->wdev;
							UINT32 sec_akm = 0;


							if (IS_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap))
								SET_AKM_SAE_SHA256(sec_akm);
							else if (IS_AKM_OWE(pApCliEntry->MlmeAux.AKMMap)) {

								SET_AKM_OWE(sec_akm);
							}

							/*Update PMK cache and delete sae instance*/
							if (
#ifdef DOT11_SAE_SUPPORT
								(IS_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap) &&
									sae_get_pmk_cache(&pAd->SaeCfg, pReptCliEntry->CurrentAddress, pApCliEntry->MlmeAux.Bssid, pmkid, pmk))
#endif
							) {

								CachedIdx = sta_search_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, ifIndex, wdev
									, sec_akm, &pApCliEntry->MlmeAux.Ssid[0], pApCliEntry->MlmeAux.SsidLen);

								if (CachedIdx != INVALID_PMKID_IDX) {
#ifdef DOT11_SAE_SUPPORT
									SAE_INSTANCE *pSaeIns = search_sae_instance(&pAd->SaeCfg, pReptCliEntry->CurrentAddress, pApCliEntry->MlmeAux.Bssid);

									MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
												"Connection falied with pmkid ,delete cache entry and sae instance \n");
									if (pSaeIns != NULL) {
										delete_sae_instance(pSaeIns);
									}
#endif
									sta_delete_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, ifIndex, wdev,
									sec_akm, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);
								}
							}
						}
#endif
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							"repeater_entry(%d),time(%ld),allocated->linkup->overtime->disconnect\n",
							pReptCliEntry->CliIdx, pReptCliEntry->CliTriggerTime);
						RepeaterDisconnectRootAP(pAd, pReptCliEntry, APCLI_DISCONNECT_SUB_REASON_REPTLM_TRIGGER_TOO_LONG);
						}
					}
				}
			} else if (pReptCliEntry->CliEnable && pReptCliEntry->pMacEntry == NULL) {
				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, (pReptCliEntry->CliTriggerTime + (10 * OS_HZ)))) {
					if (!pReptCliEntry->CliValid) {
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							"repeater_entry(%d),pMacEntry(NULL),En(%d),Vld(%d)\n",
							pReptCliEntry->CliIdx, pReptCliEntry->CliEnable, pReptCliEntry->CliValid);
						HW_REMOVE_REPT_ENTRY(pAd, CliIdx);
					}
				}
			}
		}
	}
}

INT Show_Repeater_Cli_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	ULONG DataRate = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	ADD_HT_INFO_IE *addht;

	if (!wdev)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);

	if (!pAd->ApCfg.bMACRepeaterEn)
		return TRUE;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			"\n");
#ifdef DOT11_N_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			"HT Operating Mode : %d\n", addht->AddHtInfo2.OperaionMode);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			"\n");
#endif /* DOT11_N_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			 "\n%-19s%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
			 "MAC", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1",
			 "RSSI2", "RSSI3", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate");

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (pEntry &&
			(IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst == SST_ASSOC) && IS_REPT_LINK_UP(pEntry->pReptCli)) {
			DataRate = 0;
			getRate(pEntry->HTPhyMode, &DataRate);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 ""MACSTR"  ", MAC2STR(pEntry->pReptCli->CurrentAddress));
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-4d", (int)pEntry->Aid);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-4d-%d", (int)pEntry->apidx, pEntry->func_tb_idx);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-4d", (int)pEntry->PsMode);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-8d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-7d", pEntry->RssiSample.AvgRssi[0]);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-7d", pEntry->RssiSample.AvgRssi[1]);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-7d", pEntry->RssiSample.AvgRssi[2]);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-7d", pEntry->RssiSample.AvgRssi[3]);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-6d", pEntry->HTPhyMode.field.MCS);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-6d", pEntry->HTPhyMode.field.ShortGI);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-6d", pEntry->HTPhyMode.field.STBC);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-7d", (int)DataRate);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
					 (pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 "\n");
		}
	}

	return TRUE;
}

VOID UpdateMbssCliLinkMap(
	RTMP_ADAPTER *pAd,
	UCHAR MbssIdx,
	struct wifi_dev *cli_link_wdev,
	struct wifi_dev *mbss_link_wdev)
{
	MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap = NULL;

	NdisAcquireSpinLock(&pAd->ApCfg.CliLinkMapLock);
	pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[MbssIdx];
	pMbssToCliLinkMap->mbss_wdev = mbss_link_wdev;
	pMbssToCliLinkMap->cli_link_wdev = cli_link_wdev;
	NdisReleaseSpinLock(&pAd->ApCfg.CliLinkMapLock);
}

VOID RepeaterDisconnectRootAP(
	RTMP_ADAPTER * pAd,
	REPEATER_CLIENT_ENTRY *pReptCli,
	UINT reason)
{
	if (!pReptCli)
		return;

	pReptCli->Disconnect_Sub_Reason = reason;
	cntl_disconnect_request(&pReptCli->wdev,
		CNTL_DEAUTH,
		pReptCli->wdev.bssid,
		REASON_DEAUTH_STA_LEAVING);

	return;
}

VOID repeater_disconnect_by_band(
	RTMP_ADAPTER *ad,
	UCHAR band_idx)
{
	UCHAR cli_idx = 0;
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct wifi_dev *wdev = NULL;

	if (ad->ApCfg.bMACRepeaterEn) {
		for (cli_idx = 0; cli_idx < GET_MAX_REPEATER_ENTRY_NUM(cap); cli_idx++) {
			rept_entry = &ad->ApCfg.pRepeaterCliPool[cli_idx];
			wdev = &rept_entry->wdev;

			/* Check if wdev band index match input band index */
			if (!wdev)
				continue;

			if (HcGetBandByWdev(wdev) != band_idx)
				continue;

			/* Disconnect the rept_entry which is bind on the cli_idx */
			if (rept_entry->CliEnable)
				RepeaterDisconnectRootAP(ad,
					rept_entry,
					APCLI_DISCONNECT_SUB_REASON_REPEATER_BAND_DISABLE);
		}
	}
}


#ifdef REPEATER_TX_RX_STATISTIC
BOOLEAN MtRepeaterGetTxRxInfo(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR CliIfIndex, RETRTXRXINFO *pReptStatis)
{
	RETRTXRXINFO RptrTRInfo[MAX_APCLI_NUM];
	UCHAR Index, IfIndex;
	MAC_TABLE_ENTRY *pEntry = NULL;
	PSTA_ADMIN_CONFIG main_sta = NULL;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	if ((wcid != WCID_INVALID) && (!IS_TR_WCID_VALID(pAd, wcid))) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid wcid!\n");
		return FALSE;
	}
	if ((CliIfIndex > MAX_APCLI_NUM) && (CliIfIndex != 0xf)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid Client Index!\n");
		return FALSE;
	}
	if (pReptStatis == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "NULL Pointer!\n");
		return FALSE;
	}
	NdisZeroMemory(RptrTRInfo, sizeof(RptrTRInfo));
	for (Index = 1; IS_TR_WCID_VALID(pAd, Index); Index++) {
		if ((wcid != WCID_INVALID) && (wcid != Index))
			continue;
		pEntry = &pAd->MacTab.Content[Index];
		if (!IS_VALID_ENTRY(pEntry))
			continue;
		if (pEntry->func_tb_idx != CliIfIndex)
			continue;
		if (IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
			RptrTRInfo[CliIfIndex].RxBytes += pEntry->RxBytes;
			RptrTRInfo[CliIfIndex].TxBytes += pEntry->TxBytes;
			RptrTRInfo[CliIfIndex].RxPackets += pEntry->RxPackets.QuadPart;
			RptrTRInfo[CliIfIndex].TxPackets += pEntry->TxPackets.QuadPart;
			if (pEntry->wdev) {
				STA_ADMIN_CONFIG *apcli;
				apcli = GetStaCfgByWdev(pAd, pEntry->wdev);
				RptrTRInfo[CliIfIndex].TxDropPackets = apcli->TxDropCount;
			}
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
			if (cap->fgRateAdaptFWOffload == TRUE) {
				EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
				MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"%s(%d):wcid(%d)failfw(%u),totalfw(%u)-If(%d)\"%s\".\n", __func__, __LINE__,
				pEntry->wcid, rTxStatResult.u4TotalTxFailCount, rTxStatResult.u4TotalTxCount, CliIfIndex,
				(IS_ENTRY_APCLI(pEntry)?"APCLI":(IS_ENTRY_REPEATER(pEntry)?"REPTR":"INVADER")));
				pEntry->TxFailCount += rTxStatResult.u4TotalTxFailCount;
			} else
#endif
			{
				MT_TX_COUNTER TxInfo;
				AsicTxCntUpdate(pAd, pEntry->wcid, &TxInfo);
				pEntry->TxFailCount += TxInfo.TxFailCount;
			}
			RptrTRInfo[CliIfIndex].TxFailPackets += pEntry->TxFailCount;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"%s(%d):IfIdx(%d),wcid(%d)TXInfo:RxBytes:%llu,RxPackets:%llu,TxBytes:%llu,TxDropPackets:%llu,TxFailPackets:%llu,TxPackets:%llu\n",
				__func__, __LINE__, CliIfIndex, pEntry->wcid, RptrTRInfo[CliIfIndex].RxBytes,
				RptrTRInfo[CliIfIndex].RxPackets, RptrTRInfo[CliIfIndex].TxBytes,
				RptrTRInfo[CliIfIndex].TxDropPackets, RptrTRInfo[CliIfIndex].TxFailPackets,
				RptrTRInfo[CliIfIndex].TxPackets);
			} else {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "It's not Repeater Entry!\n");
		}
	}
	if (wcid != WCID_INVALID) {
		main_sta = pAd->MacTab.Content[wcid].wdev;
		CliIfIndex = main_sta->wdev.func_idx;
	}
	for (IfIndex = 0; IfIndex < MAX_APCLI_NUM; IfIndex++) {
		if ((CliIfIndex != 0xf) && (IfIndex != CliIfIndex))
			continue;
		pReptStatis->RxBytes += RptrTRInfo[IfIndex].RxBytes;
		pReptStatis->RxPackets += RptrTRInfo[IfIndex].RxPackets;
		pReptStatis->TxBytes += RptrTRInfo[IfIndex].TxBytes;
		pReptStatis->TxPackets += RptrTRInfo[IfIndex].TxPackets;
		pReptStatis->TxFailPackets += RptrTRInfo[IfIndex].TxFailPackets;
		pReptStatis->TxDropPackets += RptrTRInfo[IfIndex].TxDropPackets;
	}
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	"RxBytes:%llu,RxPackets:%llu,TxBytes:%llu,TxPackets:%llu,TxFailPackets:%llu,TxDropPackets:%llu\n",
		pReptStatis->RxBytes, pReptStatis->RxPackets, pReptStatis->TxBytes,
		pReptStatis->TxPackets, pReptStatis->TxFailPackets, pReptStatis->TxDropPackets);
	return TRUE;
}
#endif /* REPEATER_TX_RX_STATISTIC */

BOOLEAN repeater_enable_by_any_band(RTMP_ADAPTER *ad)
{
	UCHAR i;
	BOOLEAN repeater_en_by_any_band = FALSE;

	for (i = 0; i < DBDC_BAND_NUM; i++)
		repeater_en_by_any_band |= ad->ApCfg.mac_repeater_en[i];

	return repeater_en_by_any_band;
}

VOID repeater_set_enable(RTMP_ADAPTER *ad, BOOLEAN enable, UINT8 idx)
{
	ad->ApCfg.mac_repeater_en[idx] = enable;
}

BOOLEAN repeater_get_enable(RTMP_ADAPTER *ad, UINT8 idx)
{
	return ad->ApCfg.mac_repeater_en[idx];
}

PNET_DEV repeater_get_apcli_ifdev(RTMP_ADAPTER *ad, MAC_TABLE_ENTRY *mac_entry)
{
	PNET_DEV if_dev = NULL;
	PSTA_ADMIN_CONFIG sta_cfg = NULL;
	struct wifi_dev *wdev = NULL;
	UINT8 i = 0;

	for (i = 0; i < MAX_APCLI_NUM; i++) {
		sta_cfg = &ad->StaCfg[i];
		if (MAC_ADDR_EQUAL(sta_cfg->Bssid, mac_entry->Addr)) {
			break;
		}
	}

	if (i < MAX_APCLI_NUM) {
		wdev = &sta_cfg->wdev;
		if (wdev)
			if_dev = wdev->if_dev;
	}

	return if_dev;
}

#endif /* MAC_REPEATER_SUPPORT */

