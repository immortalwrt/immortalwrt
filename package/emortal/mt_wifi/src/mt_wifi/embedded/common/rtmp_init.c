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
 ***************************************************************************

	Module Name:
	rtmp_init.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* RLM_CAL_CACHE_SUPPORT */


#ifdef OS_ABL_FUNC_SUPPORT
/* Os utility link: printk, scanf */
RTMP_OS_ABL_OPS RaOsOps, *pRaOsOps = &RaOsOps;
#endif /* OS_ABL_FUNC_SUPPORT */

UCHAR NUM_BIT8[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#ifdef DBG
char *CipherName[] = {"none", "wep64", "wep128", "TKIP", "AES", "CKIP64", "CKIP128", "CKIP152", "SMS4", "WEP152"};
#endif


NDIS_STATUS RTMPAllocGlobalUtility(VOID)
{
#ifdef OS_ABL_FUNC_SUPPORT
	/* must put the function before any print message */
	/* init OS utilities provided from UTIL module */
	RtmpOsOpsInit(&RaOsOps);
#endif /* OS_ABL_FUNC_SUPPORT */
	/* init UTIL module */
	RtmpUtilInit();
	return NDIS_STATUS_SUCCESS;
}


/*
	========================================================================

	Routine Description:
		Allocate RTMP_ADAPTER data block and do some initialization

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS RTMPAllocAdapterBlock(VOID *handle, VOID **ppAdapter, INT type)
{
	RTMP_ADAPTER *pAd = NULL;
	NDIS_STATUS	 Status;
	INT index;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> RTMPAllocAdapterBlock\n"));
	RTMPAllocGlobalUtility();
	*ppAdapter = NULL;

	do {
		/* Allocate RTMP_ADAPTER memory block*/
		Status = AdapterBlockAllocateMemory(handle, (PVOID *)&pAd, sizeof(RTMP_ADAPTER));

		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Failed to allocate memory - ADAPTER\n"));
			break;
		} else {
			/* init resource list (must be after pAd allocation) */
			initList(&pAd->RscTimerMemList);
			initList(&pAd->RscTaskMemList);
			initList(&pAd->RscLockMemList);
			initList(&pAd->RscTaskletMemList);
			initList(&pAd->RscSemMemList);
			initList(&pAd->RscAtomicMemList);
			initList(&pAd->RscTimerCreateList);
			pAd->OS_Cookie = handle;
			((POS_COOKIE)(handle))->pAd_va = (LONG)pAd;
		}

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n=== pAd = %p, size = %zu ===\n\n", pAd,
				 sizeof(RTMP_ADAPTER)));

		if (RtmpOsStatsAlloc(&pAd->stats, &pAd->iw_stats) == FALSE) {
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		/*Allocate Timer Lock*/
		NdisAllocateSpinLock(pAd, &pAd->TimerSemLock);
		/* Init spin locks*/
		NdisAllocateSpinLock(pAd, &pAd->WdevListLock);
		NdisAllocateSpinLock(pAd, &pAd->BssInfoIdxBitMapLock);
		NdisAllocateSpinLock(pAd, &pAd->irq_lock);
#ifdef CONFIG_FWOWN_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->DriverOwnLock);
#endif /* CONFIG_FWOWN_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.ReptCliEntryLock);
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.CliLinkMapLock);
#endif
#endif
#endif
#ifdef GREENAP_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.greenap.lock);
#endif /* GREENAP_SUPPORT */
		/*Allocate interface lock*/
		NdisAllocateSpinLock(pAd, &pAd->VirtualIfLock);
#ifdef RLM_CAL_CACHE_SUPPORT
		rlmCalCacheInit(pAd, &pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */
		*ppAdapter = (VOID *)pAd;
	} while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS) {
		if (pAd) {
			if (pAd->stats) {
				os_free_mem(pAd->stats);
				pAd->stats = NULL;
			}

			if (pAd->iw_stats) {
				os_free_mem(pAd->iw_stats);
				pAd->iw_stats = NULL;
			}

			RtmpOsVfree(pAd);
		}

		return Status;
	}

	/* Init ProbeRespIE Table */
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++) {
		if (os_alloc_mem(pAd, &pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN) == NDIS_STATUS_SUCCESS)
			RTMPZeroMemory(pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN);
		else
			pAd->ProbeRespIE[index].pIe = NULL;
	}
	/*allocate hdev_ctrl struct for prepare chip_cap & chip_ops */
	hdev_ctrl_init(pAd, type);
	/*init WifiSys information structure*/
	wifi_sys_init(pAd);
	/*allocate wpf related memory*/
	wpf_config_init(pAd);
#ifdef MULTI_INF_SUPPORT
	Status = multi_inf_adapt_reg((VOID *) pAd);
#endif /* MULTI_INF_SUPPORT */
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- RTMPAllocAdapterBlock, Status=%x\n", Status));
	return Status;
}

BOOLEAN RTMPCheckPhyMode(RTMP_ADAPTER *pAd, UINT8 band_cap, UCHAR *pPhyMode)
{
	BOOLEAN RetVal = TRUE;

	if (band_cap == RFIC_24GHZ) {
		if (!WMODE_2G_ONLY(*pPhyMode)) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): Warning! The board type is 2.4G only!\n",
					  __func__));
			RetVal =  FALSE;
		}
	} else if (band_cap == RFIC_5GHZ) {
		if (!WMODE_5G_ONLY(*pPhyMode)) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): Warning! The board type is 5G only!\n",
					  __func__));
			RetVal =  FALSE;
		}
	} else if (band_cap == (RFIC_24GHZ | RFIC_5GHZ))
		RetVal = TRUE;
	else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): Unknown supported band (%u), assume dual band used.\n",
				  __func__, band_cap));
		RetVal = TRUE;
	}

	if (RetVal == FALSE) {
#ifdef DOT11_N_SUPPORT

		if (band_cap == RFIC_5GHZ) /*5G ony: change to A/N mode */
			*pPhyMode = PHY_11AN_MIXED;
		else /* 2.4G only or Unknown supported band: change to B/G/N mode */
			*pPhyMode = PHY_11BGN_MIXED;

#else

		if (band_cap == RFIC_5GHZ) /*5G ony: change to A mode */
			*pPhyMode = PHY_11A;
		else /* 2.4G only or Unknown supported band: change to B/G mode */
			*pPhyMode = PHY_11BG_MIXED;

#endif /* !DOT11_N_SUPPORT */
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): Changed PhyMode to %u\n",
				  __func__, *pPhyMode));
	}

	return RetVal;
}


/*
	========================================================================

	Routine Description:
		Set default value from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID NICInitAsicFromEEPROM(RTMP_ADAPTER *pAd)
{
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> NICInitAsicFromEEPROM\n"));
	NicConfig2.word = pAd->NicConfig2.word;
	/* finally set primary ant */
	AntCfgInit(pAd);
	RTMP_CHIP_ASIC_INIT_TEMPERATURE_COMPENSATION(pAd);
#ifdef RTMP_RF_RW_SUPPORT
	/*Init RFRegisters after read RFIC type from EEPROM*/
	InitRFRegisters(pAd);
#endif /* RTMP_RF_RW_SUPPORT */
#ifdef CONFIG_ATE
	RTMPCfgTssiGainFromEEPROM(pAd);
#endif /* CONFIG_ATE */
#ifndef MAC_INIT_OFFLOAD
	AsicSetRxStream(pAd, pAd->Antenna.field.RxPath, 0);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UCHAR bandIdx = HcGetBandByWdev(&pAd->StaCfg[MAIN_MSTA_ID].wdev);

		RTMPStaCfgRadioCtrlFromEEPROM(pAd, NicConfig2);
		AsicSetTxStream(pAd, pAd->Antenna.field.TxPath, OPMODE_STA, FALSE, bandIdx);
	}
#endif /* CONFIG_STA_SUPPORT */
	RTMP_EEPROM_ASIC_INIT(pAd);
	AsicBbpInitFromEEPROM(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TxPath = %d, RxPath = %d, RFIC=%d\n",
			 pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, pAd->RfIcType));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- NICInitAsicFromEEPROM\n"));
}



INT32 WfHifSysInit(RTMP_ADAPTER *pAd, HIF_INFO_T *pHifInfo)
{
	NDIS_STATUS status;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()-->\n", __func__));
	hif_dma_disable(pAd->hdev_ctrl);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Disable WPDMA\n", __func__));
#ifdef CONFIG_WIFI_PAGE_ALLOC_SKB
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Use dev_alloc_skb\n"));
#else
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Use alloc_skb\n"));
#endif

#ifdef CUT_THROUGH
	if (IS_ASIC_CAP(pAd, fASIC_CAP_CT)) {
		VOID *pktTokenCb;

		status = token_init(&pktTokenCb, pAd);
		hc_set_ct_cb(pAd->hdev_ctrl, pktTokenCb);
		if (status != NDIS_STATUS_SUCCESS)
			goto err;
	}
#endif /*CUT_THROUGH*/

	/* hif merge should be done here and then init hif tasks */

	status = hif_init_txrx_mem(pAd->hdev_ctrl);
	if (status != NDIS_STATUS_SUCCESS)
		goto err;

	status = hif_register_irq(pAd->hdev_ctrl);
	if (status != NDIS_STATUS_SUCCESS)
		goto err;

	/* adjust sequence due to hif merge is finished in hif_init_txrx_mem */
	status = hif_init_task_group(pAd->hdev_ctrl);
	if (status != NDIS_STATUS_SUCCESS)
		goto err;

#ifdef WLAN_SKB_RECYCLE
	skb_queue_head_init(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */
err:
	return status;
}


INT32 WfHifSysExit(RTMP_ADAPTER *pAd)
{

	WLAN_HOOK_CALL(WLAN_HOOK_HIF_EXIT, pAd, NULL);
	hif_reset_task_group(pAd->hdev_ctrl);
	hif_free_irq(pAd->hdev_ctrl);
	hif_reset_txrx_mem(pAd->hdev_ctrl);
#ifdef CUT_THROUGH
	if (IS_ASIC_CAP(pAd, fASIC_CAP_CT)) {
		PKT_TOKEN_CB *pktTokenCb = hc_get_ct_cb(pAd->hdev_ctrl);

		token_deinit(&pktTokenCb);
	}
#endif /*CUT_THROUGH*/
	hif_dma_reset(pAd->hdev_ctrl);
#ifdef CONFIG_FWOWN_SUPPORT
	FwOwn(pAd);
#endif
	return 0;
}


INT32 WfMcuSysInit(RTMP_ADAPTER *pAd)
{
	MCU_CTRL_INIT(pAd);
	chip_fw_init(pAd);

	return NDIS_STATUS_SUCCESS;
}

INT32 WfMcuSysExit(RTMP_ADAPTER *pAd)
{
	MCUSysExit(pAd);
	return 0;
}


extern RTMP_STRING *mac;

INT32 WfEPROMSysInit(RTMP_ADAPTER *pAd)
{
	UCHAR RfIC;
	/* hook e2p operation */
	RtmpChipOpsEepromHook(pAd, pAd->infType, E2P_NONE);
	/* We should read EEPROM for all cases */
	/* TODO: shiang-7603, revise this! */
	NICReadEEPROMParameters(pAd, (RTMP_STRING *)mac);
	hc_radio_init(pAd, pAd->RfIcType, pAd->CommonCfg.dbdc_mode);
	/* +++Add by shiang for debug */
	RfIC = HcGetRadioRfIC(pAd);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s():PhyCtrl=>RfIcType/rf_band_cap = 0x%x/0x%x\n",
			  __func__, pAd->RfIcType, RfIC));
	RTMP_NET_DEV_NICKNAME_INIT(pAd);
	return NDIS_STATUS_SUCCESS;
}


INT32 WfEPROMSysExit(RTMP_ADAPTER *pAd)
{
	hc_radio_exit(pAd, pAd->CommonCfg.dbdc_mode);
	return NDIS_STATUS_SUCCESS;
}

/*
	========================================================================

	Routine Description:
		Initialize NIC hardware

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS	NICInitializeAdapter(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;

	ret = WfMacInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MAC Init Done!\n"));
	ret = WfPhyInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	ret = NICInitializeAsic(pAd);
err:
	return ret;
}


INT rtmp_hif_cyc_init(RTMP_ADAPTER *pAd, UINT8 val)
{
	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
		return FALSE;
	}

	return TRUE;
}



/*
	========================================================================

	Routine Description:
		Initialize ASIC

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS NICInitializeAsic(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> NICInitializeAsic\n"));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- NICInitializeAsic\n"));
	return NDIS_STATUS_SUCCESS;
}


/*
	========================================================================

	Routine Description:
		Reset NIC from error

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		Reset NIC from error state

	========================================================================
*/
static VOID NICResetFromErrorForRf(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	INT i;
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];
			AsicStaBbpTuning(pAd, pStaCfg);
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	hc_reset_radio(pAd);
}

VOID NICResetFromError(RTMP_ADAPTER *pAd)
{
	/* Reset BBP (according to alex, reset ASIC will force reset BBP*/
	/* Therefore, skip the reset BBP*/
	/* RTMP_IO_WRITE32(pAd->hdev_ctrl, MAC_CSR1, 0x2);*/
	/* TODO: shaing-7603 */
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): for MT7603\n", __func__));
		NICInitializeAdapter(pAd);
		NICInitAsicFromEEPROM(pAd);
		RTMPEnableRxTx(pAd);
		return;
	}

#ifndef MT_MAC
	RTMP_IO_WRITE32(pAd->hdev_ctrl, MAC_SYS_CTRL, 0x1);
	/* Remove ASIC from reset state*/
	RTMP_IO_WRITE32(pAd->hdev_ctrl, MAC_SYS_CTRL, 0x0);
#endif /*ndef MT_MAC */
	NICInitializeAdapter(pAd);
	NICInitAsicFromEEPROM(pAd);
	NICResetFromErrorForRf(pAd);
}


VOID NICUpdateFifoStaCounters(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_ATE

	/* Nothing to do in ATE mode */
	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		/* MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n", */
		/* __func__, __LINE__)); */
		return;
	}
}


/*
	========================================================================

	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address

	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
ULONG RTMPCompareMemory(VOID *pSrc1, VOID *pSrc2, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++) {
		if (pMem1[Index] > pMem2[Index])
			return 1;
		else if (pMem1[Index] < pMem2[Index])
			return 2;
	}

	/* Equal*/
	return 0;
}


/*
	========================================================================

	Routine Description:
		Zero out memory block

	Arguments:
		pSrc1		Pointer to memory address
		Length		Size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPZeroMemory(VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem;
	ULONG	Index = 0;

	pMem = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
		pMem[Index] = 0x00;
}


/*
	========================================================================

	Routine Description:
		Copy data from memory block 1 to memory block 2

	Arguments:
		pDest		Pointer to destination memory address
		pSrc		Pointer to source memory address
		Length		Copy size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPMoveMemory(VOID *pDest, VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	UINT	Index;

	ASSERT((Length == 0) || (pDest && pSrc));
	pMem1 = (PUCHAR) pDest;
	pMem2 = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++) {
		if (pMem1 && pMem2)
			pMem1[Index] = pMem2[Index];
	}
}


VOID UserCfgExit(RTMP_ADAPTER *pAd)
{
#ifdef RT_CFG80211_SUPPORT
	/* Reset the CFG80211 Internal Flag */
	RTMP_DRIVER_80211_RESET(pAd);
#endif /* RT_CFG80211_SUPPORT */
#ifdef RATE_PRIOR_SUPPORT
	INT idx;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PBLACK_STA pBlackSta = NULL, tmp;
	UINT16 wtbl_max_num;
#endif /*RATE_PRIOR_SUPPORT*/
	entrytb_aid_bitmap_free(&pAd->MacTab.aid_info);
	NdisFreeSpinLock(&pAd->MacTabLock);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering) {
		BndStrg_Release(pAd);
	}
#endif /* BAND_STEERING */
#ifdef RADIUS_MAC_ACL_SUPPORT
		{
			PLIST_HEADER pListHeader = NULL;
			RT_LIST_ENTRY *pListEntry = NULL;
			UCHAR apidx = 0;

			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				pListHeader = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList;

				if (pListHeader->size == 0)
					continue;

				pListEntry = pListHeader->pHead;

				while (pListEntry != NULL) {
					removeHeadList(pListHeader);
					os_free_mem(pListEntry);
					pListEntry = pListHeader->pHead;
				}

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Clean [%d] Radius ACL Cache.\n", apidx));
			}
		}
#endif /* RADIUS_MAC_ACL_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
	pAd->ApCfg.set_ch_async_flag = FALSE;
	pAd->ApCfg.iwpriv_event_flag = FALSE;
	complete_all(&pAd->ApCfg.set_ch_aync_done);
#ifdef CONFIG_STA_SUPPORT

	do {
		INT i;
		PSTA_ADMIN_CONFIG	pStaCfg = NULL;

		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];

			if (pStaCfg->wdev.pEapolPktFromAP) {
				os_free_mem(pStaCfg->wdev.pEapolPktFromAP);
				pStaCfg->wdev.pEapolPktFromAP = NULL;
			}

#ifdef	WSC_STA_SUPPORT
			{
				PWSC_CTRL		pWscControl = NULL;

				pWscControl = &pStaCfg->wdev.WscControl;

				if (pWscControl->pWscRxBuf)
					os_free_mem(pWscControl->pWscRxBuf);

				pWscControl->pWscRxBuf = NULL;

				if (pWscControl->pWscTxBuf)
					os_free_mem(pWscControl->pWscTxBuf);

				pWscControl->pWscTxBuf = NULL;
			}
#endif /*WSC_STA_SUPPORT*/
		}
	} while (0);

#endif /* CONFIG_STA_SUPPORT */
	wdev_config_init(pAd);
#ifdef DOT11_SAE_SUPPORT
	sae_cfg_deinit(pAd, &pAd->SaeCfg);
#endif /* DOT11_SAE_SUPPORT */
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	group_info_bi_deinit();
#endif
#ifdef DOT11_SAE_SUPPORT
	sae_pwd_id_deinit(pAd);
#endif
#ifdef RATE_PRIOR_SUPPORT
	/*clear the list*/
	RTMP_SEM_LOCK(&pAd->LowRateCtrl.BlackListLock);
		DlListForEach(pBlackSta, &pAd->LowRateCtrl.BlackList, BLACK_STA, List) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Remove from blklist, %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pBlackSta->Addr)));
			tmp = pBlackSta;
			pBlackSta = DlListEntry(pBlackSta->List.Prev, BLACK_STA, List);
			DlListDel(&(tmp->List));
			os_free_mem(tmp);
	}
	RTMP_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
	/*clear entry info*/
	wtbl_max_num = WTBL_MAX_NUM(pAd);
	for (idx = 1; idx < wtbl_max_num; idx++) {
		pEntry = &(pAd->MacTab.Content[idx]);
		if (pEntry != NULL) {
			pEntry->McsTotalRxCount = 0;
			pEntry->McsLowRateRxCount = 0;
		}
	}
	NdisFreeSpinLock(&pAd->LowRateCtrl.BlackListLock);
#endif/*RATE_PRIOR_SUPPORT*/
	wpf_exit(pAd);
	scan_release_mem(pAd);
#ifdef WIFI_MD_COEX_SUPPORT
	LteSafeChannelDeinit(pAd);
#endif
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
	{
		PLIST_HEADER pListHeader = NULL;
		RT_LIST_ENTRY *pListEntry = NULL;
		UCHAR apidx = 0;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			pListHeader = &pAd->ApCfg.MBSSID[apidx].BlackList;

			if (pListHeader->size == 0)
				continue;

			pListEntry = pListHeader->pHead;

			while (pListEntry != NULL) {
				removeHeadList(pListHeader);
				os_free_mem(pListEntry);
				pListEntry = pListHeader->pHead;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Clean [%d] BS Blacklist\n", apidx));
		}
	}
#endif
}


/*
	========================================================================

	Routine Description:
		Initialize port configuration structure

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID UserCfgInit(RTMP_ADAPTER *pAd)
{
	UINT i;
	UINT key_index, bss_index, band_idx;
#ifdef SINGLE_SKU_V2
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> UserCfgInit\n"));
	/*init wifi profile*/
	wpf_init(pAd);
	pAd->IndicateMediaState = NdisMediaStateDisconnected;
	/* part I. intialize common configuration */
	pAd->CommonCfg.BasicRateBitmap = 0xF;
	pAd->CommonCfg.BasicRateBitmapOld = 0xF;

#if defined(BB_SOC) && defined(TCSUPPORT_WLAN_SW_RPS)
	pAd->rxThreshold = 400;
	pAd->rxPassThresholdCnt = 2;
#endif

	pAd->ucBFBackOffMode = BF_BACKOFF_4T_MODE;
#ifdef SINGLE_SKU_V2
	if (arch_ops && arch_ops->arch_txpower_sku_cfg_para)
		arch_ops->arch_txpower_sku_cfg_para(pAd);
#endif /* SINGLE_SKU_V2 */

#ifdef TX_POWER_CONTROL_SUPPORT
	os_zero_mem(pAd->CommonCfg.PowerBoostParamV0,
		sizeof(POWER_BOOST_PARA_V0));
	os_zero_mem(pAd->CommonCfg.PowerBoostParamV1,
		sizeof(POWER_BOOST_PARA_V1));
#endif /* TX_POWER_CONTROL_SUPPORT */

	os_zero_mem(&pAd->rxv_dump_ctrl, sizeof(RXV_DUMP_CTRL));
	pAd->rxv_dump_ctrl.rxv_dump_entry_list = NULL;

#ifdef LINK_TEST_SUPPORT
	/* state machine state flag */
	os_fill_mem(pAd->ucLinkBwState, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_BW_STATE);
	os_fill_mem(pAd->ucRxStreamState, sizeof(UINT8) * BAND_NUM, RX_UNDEFINED_RXSTREAM_STATE);
	os_fill_mem(pAd->ucRxStreamStatePrev, sizeof(UINT8) * BAND_NUM, RX_UNDEFINED_RXSTREAM_STATE);
	os_fill_mem(pAd->ucRxFilterstate, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_RXFILTER_STATE);
	os_fill_mem(pAd->ucTxCsdState, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_CSD_STATE);
	os_fill_mem(pAd->ucTxPwrBoostState, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_POWER_STATE);
	os_fill_mem(pAd->ucLinkRcpiState, sizeof(UINT8) * BAND_NUM, RX_UNDEFINED_RCPI_STATE);
	os_fill_mem(pAd->ucLinkSpeState, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_SPEIDX_STATE);
	os_fill_mem(pAd->ucLinkSpeStatePrev, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_SPEIDX_STATE);
	os_fill_mem(pAd->ucLinkBwStatePrev, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_BW_STATE);
	os_fill_mem(pAd->ucTxCsdStatePrev, sizeof(UINT8) * BAND_NUM, TX_UNDEFINED_CSD_STATE);

	/* BW Control Paramter */
	os_fill_mem(pAd->fgBwInfoUpdate, sizeof(BOOLEAN) * BAND_NUM, FALSE);

	/* Rx Control Parameter */
	pAd->ucRxTestTimeoutCount			   =	   0;
	pAd->c8TempRxCount					   =	   0;
	pAd->ucRssiTh						   =	  10;
	pAd->ucRssiSigniTh					   =	  15;
	pAd->c8RxCountTh					   =	   5;
	pAd->ucTimeOutTh					   =	 200;  /* 20s */
	pAd->ucPerTh						   =	  50;
	pAd->cNrRssiTh						   =	 -40;
	pAd->cChgTestPathTh					   =	 -30;
	pAd->ucRxSenCountTh					   =	   3;
	pAd->cWBRssiTh						   =	 -70;
	pAd->cIBRssiTh						   =	 -80;
	os_fill_mem(pAd->u1RxStreamSwitchReason, sizeof(UINT8) * BAND_NUM, 0);
	os_fill_mem(pAd->u1RxSenCount, sizeof(UINT8) * BAND_NUM, 0);
	os_fill_mem(pAd->u1SpeRssiIdxPrev, sizeof(UINT8) * BAND_NUM, 0);
    os_fill_mem(pAd->ucRxSenCount, sizeof(UINT8) * BAND_NUM, 0);

	/* ACR Control Parameter */
	pAd->ucACRConfidenceCntTh			   =	  10;
	pAd->ucMaxInConfidenceCntTh			   =	  10;
	pAd->cMaxInRssiTh					   =	 -40;
	os_fill_mem(pAd->ucRxFilterConfidenceCnt, sizeof(UINT8) * BAND_NUM, 0);

	/* Tx Control Parameter */
	pAd->ucCmwCheckCount				   =	   0;
	pAd->ucCmwCheckCountTh				   =	  20;  /* 2s */
	pAd->fgCmwInstrumBack4T				   =   FALSE;
	pAd->ucRssiBalanceCount				   =	   0;
	pAd->ucRssiIBalanceCountTh			   =	 100;  /* 10s */
	pAd->fgRssiBack4T					   =   FALSE;
	pAd->ucCableRssiTh					   =	  25;
	pAd->fgCmwLinkDone					   =   FALSE;
	pAd->fgApclientLinkUp				   =   FALSE;
	pAd->ucLinkCount					   =	   0;
	pAd->ucLinkCountTh					   =	  30;
	pAd->fgLinkRSSICheck				   =   FALSE;
	os_fill_mem(pAd->ucCmwChannelBand, sizeof(UINT8) * BAND_NUM, CHANNEL_BAND_2G);

	/* channel band Control Paramter */
	os_fill_mem(pAd->fgChannelBandInfoUpdate, sizeof(BOOLEAN) * BAND_NUM, FALSE);

	/* Tx Power Control Paramter */
	os_zero_mem(pAd->ucTxPwrUpTbl, sizeof(UINT8)*CMW_POWER_UP_RATE_NUM*4);

	/* manual command control function enable/disable flag */
	pAd->fgTxSpeEn						   = TRUE;
	pAd->fgRxRcpiEn						   = TRUE;
	pAd->fgTxSpurEn						   = TRUE;
	pAd->fgRxSensitEn					   = TRUE;
	pAd->fgACREn						   = TRUE;
#endif /* LINK_TEST_SUPPORT */

#ifdef	ETSI_RX_BLOCKER_SUPPORT
	pAd->c1RWbRssiHTh	= -70;
	pAd->c1RWbRssiLTh	= -70;
	pAd->c1RIbRssiLTh	= -80;
	pAd->c1WBRssiTh4R	= -75;

	pAd->fgFixWbIBRssiEn = FALSE;
	pAd->c1WbRssiWF0 = 0xFF;
	pAd->c1WbRssiWF1 = 0xFF;
	pAd->c1WbRssiWF2 = 0xFF;
	pAd->c1WbRssiWF3 = 0xFF;
	pAd->c1IbRssiWF0 = 0xFF;
	pAd->c1IbRssiWF1 = 0xFF;
	pAd->c1IbRssiWF2 = 0xFF;
	pAd->c1IbRssiWF3 = 0xFF;

	pAd->u1RxBlockerState = ETSI_RXBLOCKER4R;
	pAd->u1To1RCheckCnt  = 10;
	pAd->u2To1RvaildCntTH = 100;
	pAd->u2To4RvaildCntTH = 3;
	pAd->u1ValidCnt		 = 0;
	pAd->u14RValidCnt	= 0;

	pAd->u1CheckTime	 = 1;
	pAd->u1TimeCnt		 = 0;

	pAd->i1MaxWRssiIdxPrev  = 0xFF;
	pAd->fgAdaptRxBlock  = 0;
#endif /* end ETSI_RX_BLOCKER_SUPPORT */


	/* disable QA Effuse Write back status by default */
	pAd->fgQAEffuseWriteBack = FALSE;
	/* Disable EPA flag */
	pAd->fgEPA = FALSE;
	/* Apply Cal-Free Effuse value by default */
	pAd->fgCalFreeApply = TRUE;
	pAd->RFlockTempIdx  =    0;
	pAd->CalFreeTempIdx =    0;

	for (key_index = 0; key_index < SHARE_KEY_NUM; key_index++) {
		for (bss_index = 0; bss_index < MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM; bss_index++) {
			pAd->SharedKey[bss_index][key_index].KeyLen = 0;
			pAd->SharedKey[bss_index][key_index].CipherAlg = CIPHER_NONE;
		}
	}

	pAd->bLocalAdminMAC = FALSE;
	pAd->EepromAccess = FALSE;
	pAd->Antenna.word = 0;
#ifdef RTMP_MAC_PCI
#ifdef LED_CONTROL_SUPPORT
	pAd->LedCntl.LedIndicatorStrength = 0;
	RTMPInitTimer(pAd, &pAd->LedCntl.LEDControlTimer, GET_TIMER_FUNCTION(LEDControlTimer), pAd, FALSE);
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_MAC_PCI */
	pAd->force_one_tx_stream = FALSE;
	pAd->RfIcType = RFIC_2820;
	/* Init timer for reset complete event*/
	pAd->bForcePrintTX = FALSE;
	pAd->bForcePrintRX = FALSE;
	pAd->bStaFifoTest = FALSE;
	pAd->bProtectionTest = FALSE;
	pAd->bHCCATest = FALSE;
	pAd->bGenOneHCCA = FALSE;
	pAd->CommonCfg.Dsifs = 10;      /* in units of usec */
	pAd->CommonCfg.TxPower = 100; /* mW*/
	pAd->CommonCfg.ucTxPowerPercentage[BAND0] = 100; /* AUTO*/
#ifdef DBDC_MODE
	pAd->CommonCfg.ucTxPowerPercentage[BAND1] = 100; /* AUTO*/
#endif /* DBDC_MODE */
	pAd->CommonCfg.ucTxPowerDefault[BAND0] = 100; /* AUTO*/
#ifdef DBDC_MODE
	pAd->CommonCfg.ucTxPowerDefault[BAND1] = 100; /* AUTO*/
#endif /* DBDC_MODE */
	pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto; /* use Long preamble on TX by defaut*/
	pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
	pAd->bDisableRtsProtect = FALSE;
	pAd->CommonCfg.UseBGProtection = 0;    /* 0: AUTO*/
	pAd->CommonCfg.bEnableTxBurst = TRUE; /* 0;	*/
	pAd->CommonCfg.SavedPhyMode = 0xff;
	pAd->CommonCfg.BandState = UNKNOWN_BAND;
	pAd->wmm_cw_min = 4;
#ifdef VLAN_SUPPORT
	pAd->CommonCfg.bEnableVlan = TRUE;	/* default enble vlan function */
#endif /*VLAN_SUPPORT*/

	switch (pAd->OpMode) {
	case OPMODE_AP:
		pAd->wmm_cw_max = 6;
		break;

	case OPMODE_STA:
		pAd->wmm_cw_max = 10;
		break;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	os_zero_mem(&pAd->ApCfg.ACSCheckTime, sizeof(UINT32) * DBDC_BAND_NUM);
	os_zero_mem(&pAd->ApCfg.ACSCheckCount, sizeof(UINT32) * DBDC_BAND_NUM);
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	/* If profile parameter is set, ACS parameters of HW bands need to be reset*/
	hc_init_ACSChCtrl(pAd);
#endif/* CONFIG_AP_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	pAd->CommonCfg.CarrierDetect.delta = CARRIER_DETECT_DELTA;
	pAd->CommonCfg.CarrierDetect.div_flag = CARRIER_DETECT_DIV_FLAG;
	pAd->CommonCfg.CarrierDetect.criteria = CARRIER_DETECT_CRITIRIA(pAd);
	pAd->CommonCfg.CarrierDetect.threshold = CARRIER_DETECT_THRESHOLD;
	pAd->CommonCfg.CarrierDetect.recheck1 = CARRIER_DETECT_RECHECK_TIME;
	pAd->CommonCfg.CarrierDetect.CarrierGoneThreshold = CARRIER_GONE_TRESHOLD;
	pAd->CommonCfg.CarrierDetect.VGA_Mask = CARRIER_DETECT_DEFAULT_MASK;
	pAd->CommonCfg.CarrierDetect.Packet_End_Mask = CARRIER_DETECT_DEFAULT_MASK;
	pAd->CommonCfg.CarrierDetect.Rx_PE_Mask = CARRIER_DETECT_DEFAULT_MASK;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	{
		UINT32 IdMbss;

		for (IdMbss = 0; VALID_MBSS(pAd, IdMbss); IdMbss++)
			UAPSD_INFO_INIT(&pAd->ApCfg.MBSSID[IdMbss].wdev.UapsdInfo);
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */
	pAd->CommonCfg.bNeedSendTriggerFrame = FALSE;
	pAd->CommonCfg.TriggerTimerCount = 0;
	pAd->CommonCfg.bAPSDForcePowerSave = FALSE;
	/*pAd->CommonCfg.bCountryFlag = FALSE;*/
	/*pAd->CommonCfg.TxStream = 0;*/
	/*pAd->CommonCfg.RxStream = 0;*/
#ifdef DOT11_N_SUPPORT
	pAd->bBroadComHT = FALSE;
	pAd->CommonCfg.bRdg = FALSE;
#ifdef DOT11N_DRAFT3
	pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
	pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
	pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
	pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
	pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
	pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (ULONG)(pAd->CommonCfg.Dot11BssWidthTriggerScanInt *
			pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
	pAd->CommonCfg.bBssCoexEnable =
		TRUE; /* by default, we enable this feature, you can disable it via the profile or ioctl command*/
	pAd->CommonCfg.BssCoexApCntThr = 0;
	pAd->CommonCfg.Bss2040NeedFallBack = 0;
	for (i = 0; i < DBDC_BAND_NUM; i++)
		NdisZeroMemory(&pAd->CommonCfg.BssCoexScanLastResult[i], sizeof(BSS_COEX_SCAN_LAST_RESULT));
#endif  /* DOT11N_DRAFT3 */
	pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;
	pAd->CommonCfg.bExtChannelSwitchAnnouncement = 1;
	pAd->CommonCfg.bMIMOPSEnable = TRUE;
	pAd->CommonCfg.bDisableReordering = FALSE;

	if (pAd->MACVersion == 0x28720200)
		pAd->CommonCfg.TxBASize = 13; /*by Jerry recommend*/
	else
		pAd->CommonCfg.TxBASize = 7;

	pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;
#endif /* DOT11_N_SUPPORT */
	pAd->CommonCfg.TxRate = RATE_6;
	pAd->CommonCfg.BeaconPeriod = 100;     /* in mSec*/
#ifdef STREAM_MODE_SUPPORT

	if (cap->FlgHwStreamMode) {
		pAd->CommonCfg.StreamMode = 3;
		pAd->CommonCfg.StreamModeMCS = 0x0B0B;
		NdisMoveMemory(&pAd->CommonCfg.StreamModeMac[0][0],
					   BROADCAST_ADDR, MAC_ADDR_LEN);
	}

#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	pAd->CommonCfg.ETxBfNoncompress = 0;
	pAd->CommonCfg.ETxBfIncapable = 0;
#endif /* TXBF_SUPPORT */
#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
	pAd->CommonCfg.lowTrafficThrd = 2;
	pAd->CommonCfg.TrainUpRule = 2; /* 1; */
	pAd->CommonCfg.TrainUpRuleRSSI = -70; /* 0; */
	pAd->CommonCfg.TrainUpLowThrd = 90;
	pAd->CommonCfg.TrainUpHighThrd = 110;
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
	/* WFA policy - disallow TH rate in WEP or TKIP cipher */
	pAd->CommonCfg.HT_DisallowTKIP = TRUE;
	/* Frequency for rate adaptation */
	pAd->ra_interval = DEF_RA_TIME_INTRVAL;
	pAd->ra_fast_interval = DEF_QUICK_RA_TIME_INTERVAL;
#ifdef AGS_SUPPORT

	if (pAd->rateAlg == RATE_ALG_AGS)
		pAd->ra_fast_interval = AGS_QUICK_RA_TIME_INTERVAL;

#endif /* AGS_SUPPORT */
	pAd->CommonCfg.bRalinkBurstMode = FALSE;
	/* global variables mXXXX used in MAC protocol state machines*/
	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
	/* PHY specification*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);  /* CCK use LONG preamble*/
	/* Default for extra information is not valid*/
	pAd->ExtraInfo = EXTRA_INFO_CLEAR;
#ifdef CONFIG_AP_SUPPORT
	/* Default Config change flag*/
	pAd->bConfigChanged = FALSE;
#ifdef GREENAP_SUPPORT
	greenap_init(pAd);
#endif /* GREENAP_SUPPORT */
#endif
	/*
		part III. AP configurations
	*/
#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_APCLI_SUPPORT || RT_CFG80211_P2P_SUPPORT */
	{
		/* Set MBSS Default Configurations*/
		UCHAR j;

#ifdef MT7915
		/*Limit 256 max station number*/
		pAd->ApCfg.MaxStaNum = MAX_NUM_OF_MT7915_STA;
#endif

		pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);

		for (j = BSS0; j < pAd->ApCfg.BssidNum; j++) {
			BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[j];
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[j].wdev;
#ifdef CONFIG_MAP_SUPPORT
		MAP_Init(pAd, wdev, WDEV_TYPE_AP);
#ifdef MAP_BL_SUPPORT
		initList(&mbss->BlackList);
		NdisAllocateSpinLock(pAd, &mbss->BlackListLock);
#endif /*  MAP_BL_SUPPORT */
#endif /* CONFIG_MAP_SUPPORT */
#ifdef DOT1X_SUPPORT
			/* PMK cache setting*/

#ifdef R1KH_HARD_RETRY /* yiwei no give up! */
			/* profile already set to 120to prevent PMK is delete on DUT */
			mbss->PMKCachePeriod = (120 * 60 * OS_HZ); /* unit : tick(default: 120 minute)*/
#else /* R1KH_HARD_RETRY */
			mbss->PMKCachePeriod = (10 * 60 * OS_HZ); /* unit : tick(default: 10 minute)*/
#endif /* !R1KH_HARD_RETRY */
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
			SET_V10_OLD_CHNL_VALID((&mbss->wdev), FALSE);
#endif /* DFS_VENDOR10_CUSTOM_FEATURE*/

			/* dot1x related per BSS */
			mbss->wdev.SecConfig.radius_srv_num = 0;
			mbss->wdev.SecConfig.NasIdLen = 0;
			mbss->wdev.SecConfig.IEEE8021X = FALSE;
			mbss->wdev.SecConfig.PreAuth = FALSE;
#ifdef RADIUS_MAC_ACL_SUPPORT
			NdisZeroMemory(&mbss->wdev.SecConfig.RadiusMacAuthCache, sizeof(RT_802_11_RADIUS_ACL));
			/* Default Timeout Value for 1xDaemon Radius ACL Cache */
			mbss->wdev.SecConfig.RadiusMacAuthCacheTimeout = 30;
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */
			/* VLAN related */
			mbss->wdev.VLAN_VID = 0;
			/* Default MCS as AUTO*/
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			/* Default is zero. It means no limit.*/
			mbss->MaxStaNum = 0;
			mbss->StaCount = 0;

#ifdef WSC_AP_SUPPORT
			wdev->WscSecurityMode = 0xff;
			{
				PWSC_CTRL pWscControl;
				INT idx;
#ifdef WSC_V2_SUPPORT
				PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */
				/*
					WscControl cannot be zero here, because WscControl timers are initial in MLME Initialize
					and MLME Initialize is called before UserCfgInit.
				*/
				pWscControl = &wdev->WscControl;
				NdisZeroMemory(&pWscControl->RegData, sizeof(WSC_REG_DATA));
				NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
				pWscControl->WscMode = 1;
				pWscControl->WscConfStatus = 1;
#ifdef WSC_V2_SUPPORT
				pWscControl->WscConfigMethods = 0x238C;
#else
				pWscControl->WscConfigMethods = 0x0084;
#endif /* WSC_V2_SUPPORT */
#ifdef P2P_SUPPORT
				pWscControl->WscConfigMethods |= 0x0108;
#endif /* P2P_SUPPORT */
				pWscControl->RegData.ReComputePke = 1;
				pWscControl->lastId = 1;
				/* pWscControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_MBSSID | j); */
				pWscControl->WscRejectSamePinFromEnrollee = FALSE;
				pAd->CommonCfg.WscPBCOverlap = FALSE;
#ifdef P2P_SUPPORT
				/*
					Set defaule value of WscConfMode to be (WSC_REGISTRAR | WSC_ENROLLEE) for WiFi P2P.
				*/
				pWscControl->WscConfMode = (WSC_REGISTRAR | WSC_ENROLLEE);
#else /* P2P_SUPPORT */
				pWscControl->WscConfMode = 0;
#endif /* !P2P_SUPPORT */
				pWscControl->WscStatus = 0;
				pWscControl->WscState = 0;
				pWscControl->WscPinCode = 0;
				pWscControl->WscLastPinFromEnrollee = 0;
				pWscControl->WscEnrollee4digitPinCode = FALSE;
				pWscControl->WscEnrolleePinCode = 0;
				pWscControl->WscSelReg = 0;
				pWscControl->WscUseUPnP = 0;
				pWscControl->bWCNTest = FALSE;
				pWscControl->WscKeyASCII = 0; /* default, 0 (64 Hex) */

				/*
					Enrollee 192 random bytes for DH key generation
				*/
				for (idx = 0; idx < 192; idx++)
					pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

				/* Enrollee Nonce, first generate and save to Wsc Control Block*/
				for (idx = 0; idx < 16; idx++)
					pWscControl->RegData.SelfNonce[idx] = RandomByte(pAd);

				NdisZeroMemory(&pWscControl->WscDefaultSsid, sizeof(NDIS_802_11_SSID));
				NdisZeroMemory(&pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR);
				NdisZeroMemory(&pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);
				pWscControl->bCheckMultiByte = FALSE;
				pWscControl->bWscAutoTigeer = FALSE;
				pWscControl->bWscFragment = FALSE;
				pWscControl->WscFragSize = 128;
				initList(&pWscControl->WscPeerList);
				NdisAllocateSpinLock(pAd, &pWscControl->WscPeerListSemLock);
				pWscControl->PinAttackCount = 0;
				pWscControl->bSetupLock = FALSE;
#ifdef WSC_V2_SUPPORT
				pWscV2Info = &pWscControl->WscV2Info;
				pWscV2Info->bWpsEnable = TRUE;
				pWscV2Info->ExtraTlv.TlvLen = 0;
				pWscV2Info->ExtraTlv.TlvTag = 0;
				pWscV2Info->ExtraTlv.pTlvData = NULL;
				pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
				pWscV2Info->bEnableWpsV2 = TRUE;
				pWscControl->SetupLockTime = WSC_WPS_AP_SETUP_LOCK_TIME;
				pWscControl->MaxPinAttack = WSC_WPS_AP_MAX_PIN_ATTACK;
#endif /* WSC_V2_SUPPORT */
			}
#endif /* WSC_AP_SUPPORT */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
			pAd->ApCfg.MBSSID[j].ap_vendor_ie.length = 0;
			pAd->ApCfg.MBSSID[j].ap_vendor_ie.pointer = NULL;
			NdisAllocateSpinLock(pAd, &pAd->ApCfg.MBSSID[j].ap_vendor_ie.vendor_ie_lock);			
			NdisAllocateSpinLock(pAd, &pAd->ApCfg.MBSSID[j].probe_rsp_vendor_ie_lock);
			DlListInit(&mbss->ap_probe_rsp_vendor_ie_list);
			pAd->ApCfg.ap_probe_rsp_vendor_ie_count = 0;
			pAd->ApCfg.ap_probe_rsp_vendor_ie_max_count = AP_PROBE_RSP_VIE_MAX_CNT;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
								mbss->IGMPPeriodicQuerySent = FALSE;
								mbss->MLDPeriodicQuerySent = FALSE;
								mbss->IgmpQueryHoldTick = 0;
								mbss->IgmpQueryHoldTickChanged = FALSE;
								mbss->MldQueryHoldTick = 0;
								mbss->MldQueryHoldTickChanged = FALSE;
								mbss->MldQryChkSum = 0x0;
								NdisZeroMemory(&mbss->ipv6LinkLocalSrcAddr[0], 16);
#endif

			for (i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
				mbss->wdev.bcn_buf.TimBitmaps[i] = 0;
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
				wdev->eap.eap_bcnrate_en = FALSE;
				wdev->eap.eap_mgmrate_en = FALSE;
				wdev->eap.mgmphymode.field.MODE = MODE_OFDM;
				wdev->eap.mgmphymode.field.MCS = MCS_RATE_6;
				wdev->eap.mgmphymode.field.BW = BW_20;
				wdev->eap.bcnphymode.field.MODE = MODE_OFDM;
				wdev->eap.bcnphymode.field.MCS = MCS_RATE_6;
				wdev->eap.bcnphymode.field.BW = BW_20;
				if ((wdev->channel < 14) && (wdev->channel != 0)) {
					wdev->eap.mgmphymode.field.MODE = MODE_CCK;
					wdev->eap.mgmphymode.field.MCS = RATE_1;
					wdev->eap.mgmphymode.field.BW = BW_20;
					wdev->eap.bcnphymode.field.MODE = MODE_CCK;
					wdev->eap.bcnphymode.field.MCS = RATE_1;
					wdev->eap.bcnphymode.field.BW = BW_20;
				}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
				wdev->rate.MCastPhyMode.field.MODE = MODE_OFDM;
				wdev->rate.MCastPhyMode.field.MCS = MCS_RATE_6;
				wdev->rate.MCastPhyMode_5G.field.MODE = MODE_OFDM;
				wdev->rate.MCastPhyMode_5G.field.MCS = MCS_RATE_6;
#else
				wdev->rate.mcastphymode.field.MODE = MODE_OFDM;
				wdev->rate.mcastphymode.field.MCS = MCS_RATE_6;
#endif
#endif /* MCAST_RATE_SPECIFIC */

#ifdef MBSS_DTIM_SUPPORT
			mbss->DtimCount = 0;
			mbss->DtimPeriod = DEFAULT_DTIM_PERIOD;
#endif
			/* Init no forwarding */
			mbss->IsolateInterStaTraffic = 0;

			wdev->bUseShortSlotTime = TRUE;
			wdev->SlotTimeValue = 9;
		}

#ifdef DOT1X_SUPPORT
		/* PMK cache setting*/
		NdisZeroMemory(&pAd->ApCfg.PMKIDCache, sizeof(NDIS_AP_802_11_PMKID));
#endif /* DOT1X_SUPPORT */
		pAd->ApCfg.DtimCount  = 0;
		pAd->ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;
		pAd->ApCfg.ErpIeContent = 0;
		pAd->ApCfg.StaIdleTimeout = MAC_TABLE_AGEOUT_TIME;
		pAd->ApCfg.BANClass3Data = FALSE;
#ifdef IDS_SUPPORT
		/* Default disable IDS threshold and reset all IDS counters*/
		pAd->ApCfg.IdsEnable = FALSE;
		pAd->ApCfg.AuthFloodThreshold = 0;
		pAd->ApCfg.AssocReqFloodThreshold = 0;
		pAd->ApCfg.ReassocReqFloodThreshold = 0;
		pAd->ApCfg.ProbeReqFloodThreshold = 0;
		pAd->ApCfg.DisassocFloodThreshold = 0;
		pAd->ApCfg.DeauthFloodThreshold = 0;
		pAd->ApCfg.EapReqFloodThreshold = 0;
		RTMPClearAllIdsCounter(pAd);
#endif /* IDS_SUPPORT */
#ifdef WDS_SUPPORT
		APWdsInitialize(pAd);
#endif /* WDS_SUPPORT*/
#ifdef WSC_INCLUDED
		pAd->WriteWscCfgToDatFile = 0xFF;
		pAd->WriteWscCfgToAr9DatFile = FALSE;
#ifdef CONFIG_AP_SUPPORT
#if defined(RTMP_PCI_SUPPORT) && defined(RTMP_RBUS_SUPPORT)
		pAd->bWscDriverAutoUpdateCfg = (IS_RBUS_INF(pAd)) ? FALSE : TRUE;
#else
#ifdef RTMP_RBUS_SUPPORT
		pAd->bWscDriverAutoUpdateCfg = FALSE;
#else
		pAd->bWscDriverAutoUpdateCfg = TRUE;
#endif
#endif /* defined(RTMP_PCI_SUPPORT) && defined (RTMP_RBUS_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef APCLI_SUPPORT
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = FALSE;
		pAd->ApCfg.ApCliNum = MAX_APCLI_NUM;
#ifdef BT_APCLI_SUPPORT
		pAd->ApCfg.ApCliAutoBWBTSupport = FALSE;
#endif
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		pAd->ApCfg.ApCliAutoBWRules.majorPolicy.ApCliBWSyncBandSupport = 0;
		pAd->ApCfg.ApCliAutoBWRules.majorPolicy.ApCliBWSyncDeauthSupport = FALSE;

		pAd->ApCfg.ApCliAutoBWRules.minorPolicy.ApCliBWSyncHTSupport = 0;
		pAd->ApCfg.ApCliAutoBWRules.minorPolicy.ApCliBWSyncVHTSupport = 0;
#endif
		for (j = 0; j < MAX_APCLI_NUM; j++) {
			STA_ADMIN_CONFIG *apcli_entry = &pAd->StaCfg[j];
			struct wifi_dev *wdev = &apcli_entry->wdev;
#ifdef CONFIG_MAP_SUPPORT
			MAP_Init(pAd, wdev, WDEV_TYPE_STA);
#endif /* CONFIG_MAP_SUPPORT */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			apcli_entry->ApcliInfStat.AutoConnectFlag = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			apcli_entry->wdev.UapsdInfo.bAPSDCapable = FALSE;
			apcli_entry->bBlockAssoc = FALSE;
#if defined(APCLI_CFG80211_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
#if defined(DOT1X_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
			apcli_entry->wdev.SecConfig.IEEE8021X = FALSE;
#endif
			apcli_entry->wpa_supplicant_info.IEEE8021x_required_keys = FALSE;
			apcli_entry->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant = FALSE;
			apcli_entry->wpa_supplicant_info.bLostAp = FALSE;
			apcli_entry->bConfigChanged = FALSE;
			apcli_entry->wpa_supplicant_info.DesireSharedKeyId = 0;
			apcli_entry->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
			apcli_entry->wpa_supplicant_info.WpaSupplicantScanCount = 0;
			apcli_entry->wpa_supplicant_info.pWpsProbeReqIe = NULL;
			apcli_entry->wpa_supplicant_info.WpsProbeReqIeLen = 0;
			apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
			apcli_entry->SavedPMKNum = 0;
			RTMPZeroMemory(apcli_entry->SavedPMK, (PMKID_NO * sizeof(BSSID_INFO)));
#endif/*WPA_SUPPLICANT_SUPPORT*/
#ifdef APCLI_CONNECTION_TRIAL
			apcli_entry->TrialCh = 0;/* if the channel is 0, AP will connect the rootap is in the same channel with ra0. */
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef WSC_AP_SUPPORT
			apcli_entry->wdev.WscControl.WscApCliScanMode = TRIGGER_FULL_SCAN;
#endif /* WSC_AP_SUPPORT */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
			/* for vendor ie */
			pAd->StaCfg[j].apcli_vendor_ie.length = 0;
			pAd->StaCfg[j].apcli_vendor_ie.pointer = NULL;
			NdisAllocateSpinLock(pAd, &pAd->StaCfg[j].apcli_vendor_ie.vendor_ie_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
		}
#endif /* APCLI_SUPPORT */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
		for (i = 0; i < DBDC_BAND_NUM; i++)
			NdisAllocateSpinLock(pAd, &pAd->ScanCtrl[i].ScanTab.event_bss_entry_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

		pAd->ApCfg.EntryClientCount = 0;
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
			pAd->bIGMPperiodicQuery = TRUE;
			pAd->IgmpQuerySendTick = QUERY_SEND_PERIOD;
			pAd->bMLDperiodicQuery = TRUE;
			pAd->MldQuerySendTick = QUERY_SEND_PERIOD;
#endif
	}

#ifdef VOW_SUPPORT
	pAd->vow_cfg.mcli_sch_cfg.tcp_cnt_th = 200;
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en[i] = TRUE;
		pAd->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_DL_MODE][i] = VOW_MULTI_CLIENT_CWMIN;
		pAd->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_DL_MODE][i] = VOW_MULTI_CLIENT_CWMAX;
		pAd->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_UL_MODE][i] = VOW_MULTI_CLIENT_CWMIN + 1;
		pAd->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_UL_MODE][i] = VOW_MULTI_CLIENT_CWMAX + 1;
	}
#endif

#endif /* CONFIG_AP_SUPPORT */
#ifdef ETH_CONVERT_SUPPORT

	if (pAd->OpMode == OPMODE_STA) {
		NdisZeroMemory(pAd->EthConvert.EthCloneMac, MAC_ADDR_LEN);
		pAd->EthConvert.ECMode = ETH_CONVERT_MODE_DISABLE;
		pAd->EthConvert.CloneMacVaild = FALSE;
		/*pAd->EthConvert.nodeCount = 0;*/
		NdisZeroMemory(pAd->EthConvert.SSIDStr, MAX_LEN_OF_SSID);
		pAd->EthConvert.SSIDStrLen = 0;
		pAd->EthConvert.macAutoLearn = FALSE;
	}

#endif /* ETH_CONVERT_SUPPORT */
	/*
		part IV. others
	*/
	/* dynamic BBP R66:sensibity tuning to overcome background noise*/
	pAd->BbpTuning.bEnable = TRUE;
	pAd->BbpTuning.FalseCcaLowerThreshold = 100;
	pAd->BbpTuning.FalseCcaUpperThreshold = 512;
	pAd->BbpTuning.R66Delta = 4;
	pAd->Mlme.bEnableAutoAntennaCheck = TRUE;
	/* Also initial R66CurrentValue, RTUSBResumeMsduTransmission might use this value.*/
	/* if not initial this value, the default value will be 0.*/
	pAd->BbpTuning.R66CurrentValue = 0x38;
	/* initialize MAC table and allocate spin lock*/
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
	InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
	NdisAllocateSpinLock(pAd, &pAd->MacTabLock);
	entrytb_aid_bitmap_init(cap, &pAd->MacTab.aid_info);

	/*RTMPInitTimer(pAd, &pAd->RECBATimer, RECBATimerTimeout, pAd, TRUE);*/
	/*RTMPSetTimer(&pAd->RECBATimer, REORDER_EXEC_INTV);*/
	pAd->CommonCfg.bWiFiTest = FALSE;
#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;
#ifdef DOT11R_FT_SUPPORT
	FT_CfgInitial(pAd);
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	pAd->ApCfg.set_ch_async_flag = FALSE;
	pAd->ApCfg.iwpriv_event_flag = FALSE;
	RTMP_OS_INIT_COMPLETION(&pAd->ApCfg.set_ch_aync_done);
#ifdef DOT11_SAE_SUPPORT
	sae_cfg_init(pAd, &pAd->SaeCfg);
#endif /* DOT11_SAE_SUPPORT */
	pAd->RxAnt.Pair1PrimaryRxAnt = 0;
	pAd->RxAnt.Pair1SecondaryRxAnt = 1;
	pAd->RxAnt.EvaluatePeriod = 0;
	pAd->RxAnt.RcvPktNumWhenEvaluate = 0;
	pAd->MaxTxPwr = 27;

#ifdef CONFIG_AP_SUPPORT
	pAd->RxAnt.Pair1AvgRssiGroup1[0] = pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup2[0] = pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
#endif /* CONFIG_AP_SUPPORT */
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	{
		UCHAR BandIdx = 0;

		for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
			BSS_TABLE *ScanTab = &pAd->ScanCtrl[BandIdx].ScanTab;
	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) {
				BSS_ENTRY *pBssEntry = &ScanTab->BssEntry[i];

		if (pAd->ProbeRespIE[i].pIe)
			pBssEntry->pVarIeFromProbRsp = pAd->ProbeRespIE[i].pIe;
		else
			pBssEntry->pVarIeFromProbRsp = NULL;
	}
		}
	}
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->ScanCtrl[i].dfs_ch_utilization = TRUE;
	}
#ifdef WSC_INCLUDED
	NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
	pAd->CommonCfg.WscPBCOverlap = FALSE;
#endif /* WSC_INCLUDED */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if (wf_drv_tbl.wf_fwd_set_cb_num != NULL)
		wf_drv_tbl.wf_fwd_set_cb_num(PACKET_BAND_CB, RECV_FROM_CB);

#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef P2P_SUPPORT
	P2pCfgInit(pAd);
#endif /* P2P_SUPPORT */
#ifdef WFD_SUPPORT
	WfdCfgInit(pAd);
#endif /* WFD_SUPPORT */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	pAd->WOW_Cfg.bEnable = FALSE;
	pAd->WOW_Cfg.bWOWFirmware = FALSE;	/* load normal firmware */
	pAd->WOW_Cfg.bInBand = TRUE;		/* use in-band signal */
	pAd->WOW_Cfg.nSelectedGPIO = 2;
	pAd->WOW_Cfg.nDelay = 3; /* (3+1)*3 = 12 sec */
	pAd->WOW_Cfg.nHoldTime = 1000;	/* unit is us */
	pAd->WOW_Cfg.nWakeupInterface = cap->nWakeupInterface; /* WOW_WAKEUP_BY_USB; */
	pAd->WOW_Cfg.bGPIOHighLow = WOW_GPIO_LOW_TO_HIGH;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WOW Enable %d, WOWFirmware %d\n", pAd->WOW_Cfg.bEnable,
			 pAd->WOW_Cfg.bWOWFirmware));
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */

	/* Clear channel ctrl buffer */
	hc_init_ChCtrl(pAd);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[1;33m [UserCfgInit] - Clear channel ctrl buffer \x1b[m \n"));
	pAd->CommonCfg.ChGrpEn = 0;
	NdisZeroMemory(pAd->CommonCfg.ChGrpChannelList, (MAX_NUM_OF_CHANNELS)*sizeof(UCHAR));
	pAd->CommonCfg.ChGrpChannelNum = 0;

#ifdef WIFI_MD_COEX_SUPPORT
	LteSafeChannelInit(pAd);
#endif

#ifdef MT_DFS_SUPPORT
	DfsParamInit(pAd);/* Jelly20150311 */
#endif
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->Dot11_H[i].RDMode = RD_SILENCE_MODE;
#endif
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->Dot11_H[i].CSCount = 0;
		pAd->Dot11_H[i].CSPeriod = 10;
		pAd->Dot11_H[i].wdev_count = 0;
		pAd->Dot11_H[i].csa_ap_bitmap = 0;
		pAd->Dot11_H[i].cac_time = 65;
		pAd->Dot11_H[i].bDFSIndoor = 1;
	}
	scan_partial_init(pAd);
#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT

	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->StaCfg[i].ApCliAutoConnectType = TRIGGER_SCAN_BY_USER; /* User Trigger SCAN by default */
		pAd->StaCfg[i].ApCliAutoConnectRunning = FALSE;
	}

	pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->ApCfg.bPartialScanEnable[i] = FALSE;
		pAd->ApCfg.bPartialScanning[i] = FALSE;
	}
#endif /* APCLI_SUPPORT */

#ifdef MICROWAVE_OVEN_SUPPORT

	if (pAd->OpMode == OPMODE_AP)
		pAd->CommonCfg.MO_Cfg.bEnable = TRUE;
	else
		pAd->CommonCfg.MO_Cfg.bEnable = FALSE;

	pAd->CommonCfg.MO_Cfg.nFalseCCATh = MO_FALSE_CCA_TH;
#endif /* MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
	pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = TRUE;
	pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = 600;
	pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = 100;
#endif /* DYNAMIC_VGA_SUPPORT */
#ifdef DOT11_VHT_AC
	pAd->CommonCfg.bNonVhtDisallow = FALSE;
#endif /* DOT11_VHT_AC */
#ifdef MT_MAC
	cap->TmrEnable = 0;
#endif
#ifdef CONFIG_MULTI_CHANNEL
	pAd->Mlme.bStartMcc = FALSE;
	pAd->Mlme.bStartScc = FALSE;
	pAd->Mlme.channel_1st_staytime = 40;
	pAd->Mlme.channel_2nd_staytime = 40;
	pAd->Mlme.switch_idle_time = 10;
	pAd->Mlme.null_frame_count = 1;
	pAd->Mlme.channel_1st_bw = 0;
	pAd->Mlme.channel_2nd_bw = 0;
#endif /* CONFIG_MULTI_CHANNEL */
#ifdef SNIFFER_SUPPORT
	pAd->monitor_ctrl.CurrentMonitorMode = 0;
	pAd->monitor_ctrl.FrameType = FC_TYPE_RSVED;
	pAd->monitor_ctrl.FilterSize = RX_DATA_BUFFER_SIZE + sizeof(struct mtk_radiotap_header);
#endif /* SNIFFER_SUPPORT */
	pAd->bPS_Retrieve = 1;
	pAd->CommonCfg.bTXRX_RXV_ON = 0;
	pAd->parse_rxv_stat_enable = 0;

	for (band_idx = BAND0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pAd->rx_stat_rxv[band_idx].rxv_cnt = 0;
		pAd->rxv_raw_data.rxv_pkt = NULL;
		pAd->rxv_raw_data.rxv_byte_cnt = 0;
		pAd->rxv_entry_sta_cnt[band_idx] = 0;
	}

	pAd->AccuOneSecRxBand0FcsErrCnt = 0;
	pAd->AccuOneSecRxBand0MdrdyCnt = 0;
	pAd->AccuOneSecRxBand1FcsErrCnt = 0;
	pAd->AccuOneSecRxBand1MdrdyCnt = 0;
	pAd->CommonCfg.ManualTxop = 0;
	pAd->CommonCfg.ManualTxopThreshold = 10; /* Mbps */
	pAd->CommonCfg.ManualTxopUpBound = 20; /* Ratio */
	pAd->CommonCfg.ManualTxopLowBound = 5; /* Ratio */
#ifdef CONFIG_AP_SUPPORT
	vow_variable_reset(pAd);
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	pAd->ApCfg.bRoamingEnhance = FALSE;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef RED_SUPPORT
	pAd->red_en = TRUE;
	pAd->red_targetdelay = 20000;
	pAd->red_atm_on_targetdelay = 15000;
	pAd->red_atm_off_targetdelay = 20000;
	pAd->red_sta_num = 0;
	pAd->red_in_use_sta = 0;
#endif /* RED_SUPPORT */
	for (band_idx = BAND0; band_idx < DBDC_BAND_NUM; band_idx++)
		pAd->rts_retrylimit[band_idx] = 0;

	for (i = 0; i < 16; i++)
		pAd->retrylimit[i] = 0;

	pAd->cp_support = 3;
	pAd->multi_cli_nums_eap_th = MULTI_CLIENT_NUMS_EAP_TH;
#if defined(MT7615) || defined(MT7622) || defined(MT7663)
	pAd->ucRxvRecordEn = 0;
	pAd->ucRxRateRecordEn = 0;
	pAd->prxvbstartelmt = NULL;
	pAd->prxvbcurelmt = NULL;
#endif /* #if defined(MT7615) || defined(MT7622) || defined(MT7663) */
#ifdef FQ_SCH_SUPPORT
	if ((!IS_MT7615(pAd) && (!(pAd->fq_ctrl.enable & FQ_READY)))) {
		pAd->fq_ctrl.enable = FQ_NEED_ON | FQ_NO_PKT_STA_KEEP_IN_LIST | FQ_ARRAY_SCH;
		pAd->fq_ctrl.factor = 2;
	}
#endif
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	pAd->ICapMode = 0;
	pAd->SpectrumEventCnt = 0;
	pAd->ICapStatus = 0;
	pAd->ICapEventCnt = 0;
	pAd->ICapDataCnt = 0;
	pAd->ICapIdx = 0;
	pAd->ICapCapLen = 0;
	pAd->ICapL32Cnt = 0;
	pAd->ICapM32Cnt = 0;
	pAd->ICapH32Cnt = 0;
	pAd->pL32Bit = NULL;
	pAd->pM32Bit = NULL;
	pAd->pH32Bit = NULL;
	pAd->pSrc_IQ = "/tmp/WifiSpectrum_IQ.txt";
	pAd->pSrc_Gain = "/tmp/WifiSpectrum_LNA_LPF.txt";
	pAd->pSrc_InPhySniffer = "/tmp/InPhySniffer.txt";
	/* Dynamic allocate memory for pIQ_Array buffer */
	{
		UINT32 retval, Len;
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

		Len = pChipCap->ICapMaxIQCnt * sizeof(RBIST_IQ_DATA_T);
		retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pIQ_Array, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : Not enough memory for dynamic allocating !!\n", __func__));
		}
		os_zero_mem(pAd->pIQ_Array, Len);
	}
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
	pAd->CalFileOffset = 0;
#endif /* CAL_BIN_FILE_SUPPORT */

	/* ===================================================== */
#ifdef CONFIG_STA_SUPPORT
	/* Following code is needed for both STA mode and ApCli Mode */
	if ((IF_COMBO_HAVE_AP_STA(pAd)) || (IF_COMBO_HAVE_STA(pAd))) {
		pAd->MSTANum = 1;
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode)
			pAd->MSTANum = MAX_MULTI_STA;
#endif

		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_DIRECT);
			RX_FILTER_CLEAR_FLAG(pAd, fRX_FILTER_ACCEPT_MULTICAST);
			RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_BROADCAST);
			RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_ALL_MULTICAST);
			pAd->CommonCfg.NdisRadioStateOff = FALSE;
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);
#ifdef PROFILE_STORE
			pAd->bWriteDat = FALSE;
#endif /* PROFILE_STORE */
			pAd->RxAnt.Pair1AvgRssi[0] = pAd->RxAnt.Pair1AvgRssi[1] = 0;

			for (i = 0; i < DBDC_BAND_NUM; i++)
				pAd->Dot11_H[i].RDMode = RD_NORMAL_MODE;
		}

		for (i = 0; i < MAX_MULTI_STA; i++) {
			PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[i];
			struct adhoc_info *adhocInfo = &pStaCfg->adhocInfo;
			struct wifi_dev *wdev = &pStaCfg->wdev;
			SCAN_INFO *ScanInfo = &wdev->ScanInfo;

			pStaCfg->PwrMgmt.bDoze = FALSE;
			pStaCfg->CountDowntoPsm = 0;
#ifdef ETH_CONVERT_SUPPORT
#ifdef IP_ASSEMBLY
			pStaCfg->bFragFlag = TRUE;
#endif /* #ifdef IP_ASSEMBLY */
#endif
#ifdef UAPSD_SUPPORT
			wdev->UapsdInfo.bAPSDCapable = FALSE;
#endif
			pStaCfg->PwrMgmt.Psm = PWR_ACTIVE;
			CLEAR_CIPHER(pStaCfg->PairwiseCipher);
			CLEAR_CIPHER(pStaCfg->GroupCipher);
			/* 802.1x port control*/
			pStaCfg->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
			wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			pStaCfg->LastMicErrorTime = 0;
			pStaCfg->MicErrCnt        = 0;
			pStaCfg->bBlockAssoc      = FALSE;
			pStaCfg->WpaState         = SS_NOTUSE;
			pStaCfg->RssiTrigger = 0;
			NdisZeroMemory(&pStaCfg->RssiSample, sizeof(RSSI_SAMPLE));
			pStaCfg->RssiTriggerMode = RSSI_TRIGGERED_UPON_BELOW_THRESHOLD;
			adhocInfo->AtimWin = 0;
			pStaCfg->DefaultListenCount = 3;/*default listen count;*/
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			pStaCfg->DefaultListenCount = 1;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			pStaCfg->BssType = BSS_INFRA;  /* BSS_INFRA or BSS_ADHOC or BSS_MONITOR*/
			pStaCfg->bSkipAutoScanConn = FALSE;
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			pStaCfg->bAutoConnectIfNoSSID = FALSE;
			wdev->bLinkUpDone = FALSE;

			if (!pStaCfg->wdev.pEapolPktFromAP)
				os_alloc_mem(NULL,
							 (UCHAR **)&pStaCfg->wdev.pEapolPktFromAP,
							 sizeof(*pStaCfg->wdev.pEapolPktFromAP));
			else
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 ("non-NULL pEapolPktFromAP 0x%p\n",
						  pStaCfg->wdev.pEapolPktFromAP));

#ifdef EXT_BUILD_CHANNEL_LIST
			pStaCfg->IEEE80211dClientMode = Rt802_11_D_None;
#endif /* EXT_BUILD_CHANNEL_LIST */
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON);
			/* user desired power mode*/
			pStaCfg->WindowsPowerMode = Ndis802_11PowerModeCAM;
			pStaCfg->WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
			pStaCfg->bWindowsACCAMEnable = FALSE;
			pStaCfg->bHwRadio = TRUE; /* Default Hardware Radio status is On*/
			pStaCfg->bSwRadio = TRUE; /* Default Software Radio status is On*/
			pStaCfg->bRadio = TRUE; /* bHwRadio && bSwRadio*/
			pStaCfg->bHardwareRadio = FALSE;        /* Default is OFF*/
			pStaCfg->bShowHiddenSSID = FALSE;       /* Default no show*/
			/* Nitro mode control*/
#if defined(NATIVE_WPA_SUPPLICANT_SUPPORT) || defined(RT_CFG80211_SUPPORT)
			pStaCfg->bAutoReconnect = FALSE;
#else
			pStaCfg->bAutoReconnect = TRUE;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT || RT_CFG80211_SUPPORT*/
			/* Save the init time as last scan time, the system should do scan after 2 seconds.*/
			/* This patch is for driver wake up from standby mode, system will do scan right away.*/
			NdisGetSystemUpTime(&pStaCfg->LastScanTime);

			if (pStaCfg->LastScanTime > 10 * OS_HZ)
				pStaCfg->LastScanTime -= (10 * OS_HZ);

			NdisZeroMemory(pAd->nickname, IW_ESSID_MAX_SIZE + 1);
#ifdef WPA_SUPPLICANT_SUPPORT
			wdev->SecConfig.IEEE8021X = FALSE;
			pStaCfg->wpa_supplicant_info.IEEE8021x_required_keys = FALSE;
			pStaCfg->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
			pStaCfg->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant = FALSE;
#if defined(NATIVE_WPA_SUPPLICANT_SUPPORT) || defined(RT_CFG80211_SUPPORT)
			pStaCfg->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
#ifdef PROFILE_STORE
			pAd->bWriteDat = TRUE;
#endif /* PROFILE_STORE */
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT || RT_CFG80211_SUPPORT */
			pStaCfg->wpa_supplicant_info.bLostAp = FALSE;
			pStaCfg->wpa_supplicant_info.pWpsProbeReqIe = NULL;
			pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen = 0;
			pStaCfg->wpa_supplicant_info.pWpaAssocIe = NULL;
			pStaCfg->wpa_supplicant_info.WpaAssocIeLen = 0;
			pStaCfg->wpa_supplicant_info.WpaSupplicantScanCount = 0;
#ifdef CFG_TDLS_SUPPORT
			NdisZeroMemory(&(pStaCfg->wpa_supplicant_info.CFG_Tdls_info), sizeof(CFG_TDLS_STRUCT));
			pStaCfg->wpa_supplicant_info.CFG_Tdls_info.bCfgTDLSCapable = 1;
			pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TdlsChSwitchSupp = 1;
			pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelStayTime = 15;
			cfg_tdls_TimerInit(pAd);
			RTMPInitTimer(pAd, &(pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelSwitchTimer),
						  GET_TIMER_FUNCTION(cfg_tdls_BaseChannelTimeoutAction), pAd, FALSE);
#endif /* CFG_TDLS_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_STA_SUPPORT
			{
				INT                 idx;
				PWSC_CTRL           pWscControl;
#ifdef WSC_V2_SUPPORT
				PWSC_V2_INFO    pWscV2Info;
#endif /* WSC_V2_SUPPORT */
				/*
				 WscControl cannot be zero here, because WscControl timers are initial in MLME Initialize
				 and MLME Initialize is called before UserCfgInit.
				*/
				pWscControl = &pStaCfg->wdev.WscControl;
				pWscControl->WscConfMode = WSC_DISABLE;
				pWscControl->WscMode = WSC_PIN_MODE;
				pWscControl->WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
#ifdef WSC_V2_SUPPORT
				pWscControl->WscConfigMethods = 0x238C;
#else
				pWscControl->WscConfigMethods = 0x008C;
#endif /* WSC_V2_SUPPORT */
#ifdef P2P_SUPPORT
				pWscControl->WscConfigMethods |= 0x0100;
#endif /* P2P_SUPPORT */
				pWscControl->WscState = WSC_STATE_OFF;
				pWscControl->WscStatus = STATUS_WSC_NOTUSED;
				pWscControl->WscPinCode = 0;
				pWscControl->WscLastPinFromEnrollee = 0;
				pWscControl->WscEnrollee4digitPinCode = FALSE;
				pWscControl->WscEnrolleePinCode = 0;
				pWscControl->WscSelReg = 0;
				NdisZeroMemory(&pWscControl->RegData, sizeof(WSC_REG_DATA));
				NdisZeroMemory(&pWscControl->WscProfile, sizeof(WSC_PROFILE));
				pWscControl->WscUseUPnP = 0;
				pWscControl->WscEnAssociateIE = TRUE;
				pWscControl->WscEnProbeReqIE = TRUE;
				pWscControl->RegData.ReComputePke = 1;
				pWscControl->lastId = 1;
				pWscControl->EntryIfIdx = BSS0;
				pWscControl->WscDriverAutoConnect = 0x02;
				pAd->WriteWscCfgToDatFile = 0xFF;
				pWscControl->WscRejectSamePinFromEnrollee = FALSE;
				pWscControl->WpsApBand = PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION;
				pWscControl->bCheckMultiByte = FALSE;
				pWscControl->bWscAutoTigeer = FALSE;

				/* Enrollee Nonce, first generate and save to Wsc Control Block*/
				for (idx = 0; idx < 16; idx++)
					pWscControl->RegData.SelfNonce[idx] = RandomByte(pAd);

				pWscControl->WscRxBufLen = 0;
				pWscControl->pWscRxBuf = NULL;
				os_alloc_mem(pAd, &pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

				if (pWscControl->pWscRxBuf)
					NdisZeroMemory(pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

				pWscControl->WscTxBufLen = 0;
				pWscControl->pWscTxBuf = NULL;
				os_alloc_mem(pAd, &pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);

				if (pWscControl->pWscTxBuf)
					NdisZeroMemory(pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);

				pWscControl->bWscFragment = FALSE;
				pWscControl->WscFragSize = 128;
				initList(&pWscControl->WscPeerList);
				NdisAllocateSpinLock(pAd, &pWscControl->WscPeerListSemLock);
#ifdef WSC_V2_SUPPORT
				pWscV2Info = &pWscControl->WscV2Info;
				pWscV2Info->bWpsEnable = TRUE;
				pWscV2Info->ExtraTlv.TlvLen = 0;
				pWscV2Info->ExtraTlv.TlvTag = 0;
				pWscV2Info->ExtraTlv.pTlvData = NULL;
				pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
				pWscV2Info->bEnableWpsV2 = TRUE;
				pWscV2Info->bForceSetAP = FALSE;
#endif /* WSC_V2_SUPPORT */
			}
#ifdef IWSC_SUPPORT
			IWSC_Init(pAd);
#endif /* IWSC_SUPPORT */
#endif /* WSC_STA_SUPPORT */
			NdisZeroMemory(pStaCfg->ReplayCounter, 8);
#ifdef DOT11R_FT_SUPPORT
			NdisZeroMemory(&pStaCfg->Dot11RCommInfo, sizeof(DOT11R_CMN_STRUC));
#endif /* DOT11R_FT_SUPPORT */
			pStaCfg->bAutoConnectByBssid = FALSE;
			pStaCfg->BeaconLostTime = BEACON_LOST_TIME;
			NdisZeroMemory(pStaCfg->WpaPassPhrase, 64);
			pStaCfg->WpaPassPhraseLen = 0;
			pStaCfg->bAutoRoaming = FALSE;
			pStaCfg->bForceTxBurst = FALSE;
			pStaCfg->bNotFirstScan = FALSE;
			ScanInfo->bImprovedScan = FALSE;
#ifdef DOT11_N_SUPPORT
			adhocInfo->bAdhocN = TRUE;
#endif /* DOT11_N_SUPPORT */
			pStaCfg->bFastConnect = FALSE;
			adhocInfo->bAdhocCreator = FALSE;
			pStaCfg->MlmeAux.OldChannel = 0;
#ifdef WIDI_SUPPORT
			pStaCfg->bWIDI = TRUE;
#endif /* WIDI_SUPPORT */
#ifdef IP_ASSEMBLY
			pStaCfg->bFragFlag = TRUE;
#endif /* IP_ASSEMBLY */
#ifdef DOT11Z_TDLS_SUPPORT
			pStaCfg->TdlsInfo.bTDLSCapable = FALSE;
			pStaCfg->TdlsInfo.TdlsChSwitchSupp = TRUE;
			pStaCfg->TdlsInfo.TdlsPsmSupp = FALSE;
			pStaCfg->TdlsInfo.TdlsKeyLifeTime = TDLS_LEY_LIFETIME;
#ifdef TDLS_AUTOLINK_SUPPORT
			initList(&pStaCfg->TdlsInfo.TdlsDiscovPeerList);
			NdisAllocateSpinLock(&pStaCfg->TdlsInfo.TdlsDiscovPeerListSemLock);
			initList(&pStaCfg->TdlsInfo.TdlsBlackList);
			NdisAllocateSpinLock(&pStaCfg->TdlsInfo.TdlsBlackListSemLock);
			pStaCfg->TdlsInfo.TdlsAutoSetupRssiThreshold = TDLS_AUTO_SETUP_RSSI_THRESHOLD;
			pStaCfg->TdlsInfo.TdlsAutoTeardownRssiThreshold = TDLS_AUTO_TEARDOWN_RSSI_THRESHOLD;
			pStaCfg->TdlsInfo.TdlsRssiMeasurementPeriod = TDLS_RSSI_MEASUREMENT_PERIOD;
			pStaCfg->TdlsInfo.TdlsDisabledPeriodByTeardown = TDLS_DISABLE_PERIOD_BY_TEARDOWN;
			pStaCfg->TdlsInfo.TdlsAutoDiscoveryPeriod = TDLS_AUTO_DISCOVERY_PERIOD;
#endif /* TDLS_AUTOLINK_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */
		}
	}
#endif

#ifdef SCS_FW_OFFLOAD
if (IS_MT7626(pAd))
	pAd->SCSCtrl.SCSGeneration = SCS_Gen5;
#endif
#ifdef SMART_CARRIER_SENSE_SUPPORT
	if (IS_MT7615(pAd))
		pAd->SCSCtrl.SCSGeneration = SCS_Gen2;
	else if (IS_MT7622(pAd))
		pAd->SCSCtrl.SCSGeneration = SCS_Gen3;
	else if (IS_MT7663(pAd) || IS_MT7626(pAd))
		pAd->SCSCtrl.SCSGeneration = SCS_Gen4;
	else
		pAd->SCSCtrl.SCSGeneration = SCS_Gen1;
#ifdef SCS_FW_OFFLOAD
	if (IS_MT7915(pAd))
		pAd->SCSCtrl.SCSGeneration = SCS_Gen6;
#endif
	/* SCS Variable initialization */
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->SCSCtrl.SCSEnable[i] = SCS_DISABLE;
		pAd->SCSCtrl.SCSTrafficThreshold[i] = TriggerTrafficeTh; /* 2M */
		pAd->SCSCtrl.SCSStatus[i] = PD_BLOCKING_OFF;
		pAd->SCSCtrl.OneSecTxByteCount[i] = 0;
		pAd->SCSCtrl.OneSecRxByteCount[i] = 0;
		pAd->SCSCtrl.CckPdBlkTh[i] = PdBlkCckThDefault;
		pAd->SCSCtrl.OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
		pAd->SCSCtrl.SCSThTolerance[i] = ThTolerance;
		pAd->SCSCtrl.SCSMinRssiTolerance[i] = MinRssiTolerance;
		pAd->SCSCtrl.OfdmPdSupport[i] = TRUE;
		pAd->SCSCtrl.CckFalseCcaUpBond[i] = FalseCcaUpBondDefault;
		pAd->SCSCtrl.CckFalseCcaLowBond[i] = FalseCcaLowBondDefault;
		pAd->SCSCtrl.OfdmFalseCcaUpBond[i] = FalseCcaUpBondDefault;
		pAd->SCSCtrl.OfdmFalseCcaLowBond[i] = FalseCcaLowBondDefault;
		pAd->SCSCtrl.CckFixedRssiBond[i] = CckFixedRssiBondDefault;
		pAd->SCSCtrl.OfdmFixedRssiBond[i] = OfdmFixedRssiBondDefault;
		/*SCSGen4_for_MT7663*/
		{
			pAd->SCSCtrl.PHY_MIN_PRI_PWR_OFFSET = 0;
			pAd->SCSCtrl.PHY_RXTD_CCKPD_7_OFFSET = 0;
			pAd->SCSCtrl.PHY_RXTD_CCKPD_8_OFFSET = 0;
		}
		/*SCSGen6_for_MT7915*/
		pAd->SCSCtrl.LastETput[i] = 0;
	}
	pAd->SCSCtrl.SCSEnable[DBDC_BAND0] = SCS_ENABLE;
#endif /* SMART_CARRIER_SENSE_SUPPORT */

	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->CCI_ACI_TxOP_Value[i] = 0;

	pAd->g_mode_txop_wdev = NULL;
	pAd->G_MODE_INFRA_TXOP_RUNNING = FALSE;
	pAd->MUMIMO_TxOP_Value = 0;
	pAd->partial_mib_show_en = 0;
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->txop_ctl[i].multi_client_nums = 0;
		pAd->txop_ctl[i].multi_tcp_nums = 0;
		pAd->txop_ctl[i].cur_wdev = NULL;
		pAd->txop_ctl[i].multi_cli_txop_running = FALSE;
		pAd->txop_ctl[i].near_far_txop_running = FALSE;
	}

#ifdef PKT_BUDGET_CTRL_SUPPORT
	pAd->pbc_bound[PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE;
	pAd->pbc_bound[PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK;
	pAd->pbc_bound[PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO;
	pAd->pbc_bound[PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI;
	pAd->pbc_bound[PBC_AC_MGMT] = PBC_WMM_UP_DEFAULT_MGMT;
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#ifdef TX_AGG_ADJUST_WKR
	pAd->TxAggAdjsut = TRUE;
#endif /* TX_AGG_ADJUST_WKR */
#ifdef HTC_DECRYPT_IOT
	pAd->HTC_ICV_Err_TH = 5;
#endif /* HTC_DECRYPT_IOT */
#ifdef DHCP_UC_SUPPORT
	pAd->DhcpUcEnable = FALSE;
#endif /* DHCP_UC_SUPPORT */

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pAd->CommonCfg.ucEDCCACtrl[i] = TRUE; /* EDCCA default is ON. */
	}
#ifdef DSCP_PRI_SUPPORT
	{
		UINT8 bss_idx;
		/*fill default dscp value, overwrite by profile param or iwpriv command*/
		for (bss_idx = 0; bss_idx < MAX_BEACON_NUM; bss_idx++) {
			for (i = 0; i < 64; i++)
				pAd->ApCfg.MBSSID[bss_idx].dscp_pri_map[i] = i >> 3;
		}
	}
#endif /*DSCP_PRI_SUPPORT*/
#ifdef AIR_MONITOR
	pAd->MntRuleBitMap = DEFAULT_MNTR_RULE;
#endif /* AIR_MONITOR */
#ifdef MBO_SUPPORT
	pAd->reg_domain = REG_GLOBAL;
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->bss_load_info.high_thrd[i] = MAX_BSSLOAD_THRD;
#endif /* WAPP_SUPPORT */
#ifdef FW_LOG_DUMP
	for (i = 0; i < BIN_DBG_LOG_NUM; i++)
		pAd->fw_log_ctrl.debug_level_ctrl[i] = 0;
	for (i = 0; i < MAC_ADDR_LEN; i++)
		pAd->fw_log_ctrl.fw_log_server_mac[i] = 0xFF;
	pAd->fw_log_ctrl.fw_log_server_ip = 0xFFFFFFFF;
	sprintf(pAd->fw_log_ctrl.fw_log_dest_dir, DEFAULT_FW_LOG_DESTINATION);
#endif /* FW_LOG_DUMP */
#ifdef RATE_PRIOR_SUPPORT
	DlListInit(&pAd->LowRateCtrl.BlackList);
	NdisAllocateSpinLock(pAd, &pAd->LowRateCtrl.BlackListLock);
#endif/*RATE_PRIOR_SUPPORT*/

#ifdef CFG_SUPPORT_FALCON_MURU
	pAd->CommonCfg.bShowMuEdcaParam = FALSE;
#endif
	/* fw core dump is dumped */
	pAd->bIsBeenDumped = FALSE;

	/* fwcmd timeout dump */
	pAd->FwCmdTimeoutCnt = 0;
	pAd->FwCmdTimeoutPrintCnt = FW_CMD_TO_PRINT_CNT;
	NdisZeroMemory(pAd->FwCmdTimeoutRecord, sizeof(pAd->FwCmdTimeoutRecord));

#ifdef ANTENNA_DIVERSITY_SUPPORT
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		ant_diversity_ctrl_init(pAd, band_idx);
	}
	ant_diversity_ctrl_reset(pAd);
#endif

#ifdef CONFIG_COLGIN_MT6890
    if (pAd->bIsLowPower == TRUE)
        pAd->WaitBeaconTime = 110;
#endif
#ifdef DABS_QOS
	NdisAllocateSpinLock(pAd, &qos_param_table_lock);
	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	memset(&qos_param_table[0], 0, sizeof(struct qos_param_rec)*MAX_QOS_PARAM_TBL);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
#endif

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- UserCfgInit\n"));
}


/* IRQL = PASSIVE_LEVEL*/
UCHAR BtoH(RTMP_STRING ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';        /* Handle numerals*/

	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 0xA;  /* Handle capitol hex digits*/

	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 0xA;  /* Handle small hex digits*/

	return 255;
}


/*
	FUNCTION: AtoH(char *, UCHAR *, int)

	PURPOSE:  Converts ascii string to network order hex

	PARAMETERS:
		src    - pointer to input ascii string
		dest   - pointer to output hex
		destlen - size of dest

	COMMENTS:

		2 ascii bytes make a hex byte so must put 1st ascii byte of pair
		into upper nibble and 2nd ascii byte of pair into lower nibble.

	IRQL = PASSIVE_LEVEL
*/
void AtoH(RTMP_STRING *src, PUCHAR dest, int destlen)
{
	RTMP_STRING *srcptr;
	PUCHAR destTemp;

	srcptr = src;
	destTemp = (PUCHAR) dest;

	while (destlen--) {
		*destTemp = BtoH(*srcptr++) << 4;    /* Put 1st ascii byte in upper nibble.*/
		*destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
		destTemp++;
	}
}


/*
========================================================================
Routine Description:
	Add a timer to the timer list.

Arguments:
	pAd				- WLAN control block pointer
	pRsc			- the OS resource

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_TimerListAdd(RTMP_ADAPTER *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);

	while (1) {
		if (pObj == NULL)
			break;

		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc)
			return;

		pObj = pObj->pNext;
	}

	/* allocate a timer record entry */
	os_alloc_mem(NULL, (UCHAR **) &(pObj), sizeof(LIST_RESOURCE_OBJ_ENTRY));

	if (pObj == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: alloc timer obj fail!\n", __func__));
		return;
	} else {
		pObj->pRscObj = pRsc;
		insertTailList(pRscList, (RT_LIST_ENTRY *)pObj);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: add timer obj %lx!\n", __func__, (ULONG)pRsc));
	}
}


VOID RTMP_TimerListRelease(RTMP_ADAPTER *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	RT_LIST_ENTRY *pListEntry;

	if (pRscList == NULL || pRscList->pHead == NULL)
		return;

	pListEntry = pRscList->pHead;
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;

	while (pObj) {
		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc) {
			pListEntry = (RT_LIST_ENTRY *)pObj;
			break;
		}

		pListEntry = pListEntry->pNext;
		pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;
	}

	if (pListEntry) {
		delEntryList(pRscList, pListEntry);
		/* free a timer record entry */
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: release timer obj %lx!\n", __func__, (ULONG)pRsc));
		if (pObj == (LIST_RESOURCE_OBJ_ENTRY *)pListEntry)
			os_free_mem(pObj);
		else
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Pointers mismatch in %s %d\n",
					 __func__, __LINE__));

	}
}



/*
* Show Timer Information
*/
VOID RTMPShowTimerList(RTMP_ADAPTER *pAd)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	RALINK_TIMER_STRUCT *pTimer;
	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Timer List Size:%d\n", pRscList->size));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=====================================\n"));

	while (1) {
		if (pObj == NULL)
			break;

		pTimer = (RALINK_TIMER_STRUCT *)pObj->pRscObj;
		pObj = pObj->pNext;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Valid:%d\n", pTimer->Valid));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pObj:%lx\n", (ULONG)pTimer));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PeriodicType:%d\n", pTimer->PeriodicType));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Repeat:%d\n", pTimer->Repeat));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("State:%d\n", pTimer->State));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TimerValue:%ld\n", pTimer->TimerValue));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("timer_lock:%lx\n", (ULONG)pTimer->timer_lock));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pCaller:%pS\n", pTimer->pCaller));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=====================================\n"));
	}
}



/*
========================================================================
Routine Description:
	Cancel all timers in the timer list.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_AllTimerListRelease(RTMP_ADAPTER *pAd)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj, *pObjOld;
	BOOLEAN Cancel;
	RALINK_TIMER_STRUCT *pTimer;
	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Size=%d\n", __func__, pRscList->size));

	while (1) {
		if (pObj == NULL)
			break;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Cancel timer obj %lx!\n", __func__,
				 (ULONG)(pObj->pRscObj)));
		pObjOld = pObj;
		pObj = pObj->pNext;
		pTimer = (RALINK_TIMER_STRUCT *)pObjOld->pRscObj;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Timer is allocated by %pS,Valid:%d,Lock:%lx,State:%d\n",
				 __func__,
				 pTimer->pCaller, pTimer->Valid, (ULONG)pTimer->timer_lock, pTimer->State));
		RTMPReleaseTimer(pObjOld->pRscObj, &Cancel);
	}

	/* reset TimerList */
	initList(&pAd->RscTimerCreateList);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pAd			Pointer to our adapter
		pTimer				Timer structure
		pTimerFunc			Function to execute when timer expired
		Repeat				Ture for period timer

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPInitTimer(
	IN	RTMP_ADAPTER *pAd,
	IN	RALINK_TIMER_STRUCT *pTimer,
	IN	VOID *pTimerFunc,
	IN	VOID *pData,
	IN	BOOLEAN	 Repeat)
{
	pTimer->timer_lock = &pAd->TimerSemLock;
	RTMP_SEM_LOCK(pTimer->timer_lock);
	RTMP_TimerListAdd(pAd, pTimer);
	/* Set Valid to TRUE for later used.*/
	/* It will crash if we cancel a timer or set a timer */
	/* that we haven't initialize before.*/
	/* */
	pTimer->Valid      = TRUE;
	pTimer->PeriodicType = Repeat;
	pTimer->State      = FALSE;
	pTimer->cookie = (ULONG) pData;
	pTimer->pAd = pAd;
	pTimer->pCaller = (VOID *)OS_TRACE;
	RTMP_OS_Init_Timer(pAd, &pTimer->TimerObj,	pTimerFunc, (PVOID) pTimer, &pAd->RscTimerMemList);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	RTMP_SEM_UNLOCK(pTimer->timer_lock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	if (!pTimer->timer_lock)
		return;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		RTMP_ADAPTER *pAd;

		pAd = (RTMP_ADAPTER *)pTimer->pAd;

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPSetTimer failed, Halt in Progress!\n"));
			RTMP_SEM_UNLOCK(pTimer->timer_lock);
			return;
		}

		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;

		if (pTimer->PeriodicType == TRUE) {
			pTimer->Repeat = TRUE;
			RTMP_SetPeriodicTimer(&pTimer->TimerObj, Value);
		} else {
			pTimer->Repeat = FALSE;
			RTMP_OS_Add_Timer(&pTimer->TimerObj, Value);
		}

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	} else
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPSetTimer failed, Timer hasn't been initialize! <caller: %pS>\n", __builtin_return_address(0)));

	RTMP_SEM_UNLOCK(pTimer->timer_lock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	BOOLEAN	Cancel;

	if (!pTimer->timer_lock)
		return;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;

		if (pTimer->PeriodicType == TRUE) {
			RTMP_SEM_UNLOCK(pTimer->timer_lock);
			RTMPCancelTimer(pTimer, &Cancel);
			RTMPSetTimer(pTimer, Value);
		} else {
			RTMP_OS_Mod_Timer(&pTimer->TimerObj, Value);
			RTMP_SEM_UNLOCK(pTimer->timer_lock);
		}

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	} else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPModTimer failed, Timer hasn't been initialize!\n"));
		RTMP_SEM_UNLOCK(pTimer->timer_lock);
	}
}


/*
	========================================================================

	Routine Description:
		Cancel timer objects

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:
		1.) To use this routine, must call RTMPInitTimer before.
		2.) Reset NIC to initial state AS IS system boot up time.

	========================================================================
*/
VOID RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{
	if (!pTimer->timer_lock)
		goto end;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		RTMP_SEM_UNLOCK(pTimer->timer_lock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		RTMP_SEM_LOCK(pTimer->timer_lock);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		if (IS_ASIC_CAP(((struct _RTMP_ADAPTER *)pTimer->pAd), fASIC_CAP_MGMT_TIMER_TASK)) {
			/* We need to go-through the TimerQ to findout this timer handler and remove it if */
			/*		it's still waiting for execution.*/
			RtmpTimerQRemove(pTimer->pAd, pTimer);
		}
#endif /* RTMP_TIMER_TASK_SUPPORT */
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	}

	RTMP_SEM_UNLOCK(pTimer->timer_lock);
	return;
end:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RTMPCancelTimer failed, Timer hasn't been initialize!\n"));
}


VOID RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{
	if (!pTimer->timer_lock)
		goto end;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid) {
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		RTMP_SEM_UNLOCK(pTimer->timer_lock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		RTMP_SEM_LOCK(pTimer->timer_lock);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		if (IS_ASIC_CAP(((struct _RTMP_ADAPTER *)pTimer->pAd), fASIC_CAP_MGMT_TIMER_TASK)) {
			/* We need to go-through the TimerQ to findout this timer handler and remove it if */
			/*		it's still waiting for execution.*/
			RtmpTimerQRemove(pTimer->pAd, pTimer);
		}
#endif /* RTMP_TIMER_TASK_SUPPORT */
		/* release timer */
		RTMP_OS_Release_Timer(&pTimer->TimerObj);
		pTimer->Valid = FALSE;
		/* TODO: shiang-usw, merge this from NXTC, make sure if that's necessary here!! */
		RTMP_TimerListRelease(pTimer->pAd, pTimer);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n", __func__, (ULONG)pTimer));
	}

	RTMP_SEM_UNLOCK(pTimer->timer_lock);
end:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RTMPReleasefailed, Timer hasn't been initialize!\n"));
}


/*
	========================================================================

	Routine Description:
		Enable RX

	Arguments:
		pAd						Pointer to our adapter

	Return Value:
		None

	IRQL <= DISPATCH_LEVEL

	Note:
		Before Enable RX, make sure you have enabled Interrupt.
	========================================================================
*/
VOID RTMPEnableRxTx(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> RTMPEnableRxTx\n"));
	hif_dma_enable(pAd->hdev_ctrl);
	AsicSetRxFilter(pAd);

	if (pAd->CommonCfg.bTXRX_RXV_ON)
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE);
	else
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== RTMPEnableRxTx\n"));
}


void CfgInitHook(RTMP_ADAPTER *pAd)
{
	/*pAd->bBroadComHT = TRUE;*/
}

static INT RtmpChipOpsRegister(RTMP_ADAPTER *pAd, INT infType)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	int ret = 0;

	NdisZeroMemory(cap, sizeof(RTMP_CHIP_CAP));
	ret = RtmpChipOpsHook(pAd);

	if (ret) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("chip ops hook error\n"));
		return ret;
	}

	/* MCU related */
	ChipOpsMCUHook(pAd, cap->MCUType);
	get_dev_config_idx(pAd);
	return ret;
}




PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR wdev_idx)
{
	PNET_DEV dev_p = NULL;

	if ((wdev_idx < WDEV_NUM_MAX) && (pAd->wdev_list[wdev_idx] != NULL))
		dev_p = pAd->wdev_list[wdev_idx]->if_dev;

	if (dev_p == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s,dev_p=NULL,caller:%pS\n", __func__, OS_TRACE));
	}

	return dev_p;
}


INT RtmpRaDevCtrlInit(VOID *pAdSrc, RTMP_INF_TYPE infType)
{
	RTMP_ADAPTER *pAd = (PRTMP_ADAPTER)pAdSrc;
#ifdef FW_DUMP_SUPPORT
	pAd->fw_dump_max_size = MAX_FW_DUMP_SIZE;
	RTMP_OS_FWDUMP_PROCINIT(pAd);
#endif
	/* Assign the interface type. We need use it when do register/EEPROM access.*/
	pAd->infType = infType;
#ifdef CONFIG_STA_SUPPORT
	pAd->OpMode = OPMODE_STA;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("STA Driver version-%s\n", STA_DRIVER_VERSION));
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	pAd->OpMode = OPMODE_AP;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AP Driver version-%s\n", AP_DRIVER_VERSION));
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->infType=%d\n", pAd->infType));
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	pAd->OpMode = OPMODE_STA;
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */

	pAd->iface_combinations = 0;
#ifdef CONFIG_STA_SUPPORT
	pAd->iface_combinations |= HAVE_STA_INF;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	pAd->iface_combinations |= HAVE_AP_INF;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_EVENT_INIT(&(pAd->AutoRateLock), &pAd->RscSemMemList);

	/* add to handle wifi_sys operation race condition */
	RTMP_SEM_EVENT_INIT(&(pAd->wf_link_lock), &pAd->RscSemMemList);
	pAd->wf_link_lock_flag = FALSE;
	pAd->wf_link_timestamp = 0;
	pAd->MonitorSemaphore = TRUE;
	pAd->wf_lock_op = WDEV_LOCK_OP_ALL;

	pAd->IoctlHandleFlag = FALSE;
	
#ifdef WIFI_MD_COEX_SUPPORT
	NdisAllocateSpinLock(pAd, &pAd->LteSafeChCtrl.SafeChDbLock);
#endif

	if (RtmpChipOpsRegister(pAd, infType))
		return FALSE;

	/*prepeare hw resource depend on chipcap*/
	hdev_resource_init(pAd->hdev_ctrl);

	/*initial wlan hook module*/
	WLAN_HOOK_INIT();

#ifdef CONFIG_CSO_SUPPORT

	if (IS_ASIC_CAP(pAd, fASIC_CAP_CSO))
		RTMP_SET_MORE_FLAG(pAd, fASIC_CAP_CSO);

#endif /* CONFIG_CSO_SUPPORT */
#ifdef MCS_LUT_SUPPORT

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCS_LUT)) {
		if (WTBL_MAX_NUM(pAd) < 128)
			RTMP_SET_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT);
		else {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): MCS_LUT not used becasue MacTb size(%d) > 128!\n",
					 __func__, WTBL_MAX_NUM(pAd)));
		}
	}

#endif /* MCS_LUT_SUPPORT */

	return 0;
}


BOOLEAN RtmpRaDevCtrlExit(IN VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	INT index;
#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
	NdisFreeSpinLock(&pAd->StaCtIf.Lock);
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */
	RTMP_SEM_EVENT_DESTORY(&(pAd->AutoRateLock));

#ifdef WIFI_MD_COEX_SUPPORT
	NdisFreeSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
#endif

	/* add to handle wifi_sys operation race condition */
	RTMP_SEM_EVENT_DESTORY(&(pAd->wf_link_lock));
	pAd->wf_link_lock_flag = FALSE;
	pAd->wf_link_timestamp = 0;
	pAd->MonitorSemaphore = FALSE;

	pAd->IoctlHandleFlag = FALSE;

	/*
		Free ProbeRespIE Table
	*/
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++) {
		if (pAd->ProbeRespIE[index].pIe)
			os_free_mem(pAd->ProbeRespIE[index].pIe);
	}

#ifdef FW_DUMP_SUPPORT
	RTMP_OS_FWDUMP_PROCREMOVE(pAd);

	if (pAd->fw_dump_buffer) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Free FW dump buffer\n", __func__));
		os_free_mem(pAd->fw_dump_buffer);
		pAd->fw_dump_buffer = 0;
		pAd->fw_dump_size = 0;
		pAd->fw_dump_read = 0;
	}

#endif
	wpf_config_exit(pAd);
	RTMPFreeAdapter(pAd);
	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(RTMP_ADAPTER *pAd)
{
	RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);
}

VOID RTMP_11N_D3_TimerRelease(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancel;

	RTMPReleaseTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancel);
}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

VOID AntCfgInit(RTMP_ADAPTER *pAd)
{

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
		return;
	}

	{
		if (pAd->NicConfig2.field.AntOpt == 1) { /* ant selected by efuse */
			if (pAd->NicConfig2.field.AntDiversity == 0) { /* main */
				pAd->RxAnt.Pair1PrimaryRxAnt = 0;
				pAd->RxAnt.Pair1SecondaryRxAnt = 1;
			} else { /* aux */
				pAd->RxAnt.Pair1PrimaryRxAnt = 1;
				pAd->RxAnt.Pair1SecondaryRxAnt = 0;
			}
		} else if (pAd->NicConfig2.field.AntDiversity == 0) { /* Ant div off: default ant is main */
			pAd->RxAnt.Pair1PrimaryRxAnt = 0;
			pAd->RxAnt.Pair1SecondaryRxAnt = 1;
		} else if (pAd->NicConfig2.field.AntDiversity == 1) /* Ant div on */
		{/* eeprom on, but sw ant div support is not enabled: default ant is main */
			pAd->RxAnt.Pair1PrimaryRxAnt = 0;
			pAd->RxAnt.Pair1SecondaryRxAnt = 1;
		}
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: primary/secondary ant %d/%d\n",
			 __func__,
			 pAd->RxAnt.Pair1PrimaryRxAnt,
			 pAd->RxAnt.Pair1SecondaryRxAnt));
}
#ifdef CFG_SUPPORT_CSI
VOID csi_support_init(RTMP_ADAPTER *pAd)
{
	/* init CSI wait queue */
	init_waitqueue_head(&pAd->rCSIInfo.waitq);
	/* inti lock */
	NdisAllocateSpinLock(pAd, &pAd->rCSIInfo.CSIBufferLock);
	/* init proc fs*/
	csi_proc_init(pAd);
}

VOID csi_support_deinit(RTMP_ADAPTER *pAd)
{
	/* deinti lock */
	NdisFreeSpinLock(&pAd->rCSIInfo.CSIBufferLock);
	/* init proc fs*/
	csi_proc_deinit(pAd);
}
#endif

