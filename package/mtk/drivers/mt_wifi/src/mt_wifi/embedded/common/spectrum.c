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
	action.c

    Abstract:
    Handle association related requests either from WSTA or from local MLME

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
	Fonchi Wu    2008		   created for 802.11h
 */

#include "rt_config.h"
#include "action.h"

UINT8 GetRegulatoryMaxTxPwr(RTMP_ADAPTER *pAd, UINT8 channel, struct wifi_dev *wdev)
{
	ULONG ChIdx;
	PCH_REGION pChRegion = NULL;
	UCHAR FirstChannelIdx, NumCh;
	UCHAR increment = 0, index = 0, ChannelIdx = 0, cfg_bw = 0;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(wdev->PhyMode))
		cfg_bw = wlan_config_get_vht_bw(wdev);
#endif
#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode))
		cfg_bw = wlan_config_get_he_bw(wdev);
#endif
	/* Get Channel Region (CountryCode)*/
	pChRegion = GetChRegion(pAd->CommonCfg.CountryCode);

	if (!pChRegion || !pChRegion->pChDesp) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "pChRegion is NULL\n");
		return 0xff;
	}

	FirstChannelIdx = pChRegion->pChDesp->FirstChannel;
	NumCh = pChRegion->pChDesp->NumOfCh;

	while (FirstChannelIdx != 0) {
		ChannelIdx = FirstChannelIdx;

		for (ChIdx = 0; ChIdx < NumCh; ChIdx++) {
			if (FirstChannelIdx > channel)
				break;

			/* check increment */
			if (ChannelIdx > 14)
				increment = 4;
			else
				increment = 1;

			if (channel == ChannelIdx
#if defined(DOT11_VHT_AC) || defined (DOT11_HE_AX)
				|| (channel == vht_cent_ch_freq(ChannelIdx, cfg_bw, ch_band))
#endif
				)
				return pChRegion->pChDesp[index].MaxTxPwr;

			ChannelIdx += increment;
		}

		index++;
		FirstChannelIdx = pChRegion->pChDesp[index].FirstChannel;
		NumCh = pChRegion->pChDesp[index].NumOfCh;
	}

	return 0xff;
}

typedef struct __TX_PWR_CFG {
	UINT8 Mode;
	UINT8 MCS;
	UINT16 req;
	UINT8 shift;
	UINT32 BitMask;
} TX_PWR_CFG;

/* Note: the size of TxPwrCfg is too large, do not put it to function */
TX_PWR_CFG TxPwrCfg[] = {
	{MODE_CCK, 0, 0, 4, 0x000000f0},
	{MODE_CCK, 1, 0, 0, 0x0000000f},
	{MODE_CCK, 2, 0, 12, 0x0000f000},
	{MODE_CCK, 3, 0, 8, 0x00000f00},

	{MODE_OFDM, 0, 0, 20, 0x00f00000},
	{MODE_OFDM, 1, 0, 16, 0x000f0000},
	{MODE_OFDM, 2, 0, 28, 0xf0000000},
	{MODE_OFDM, 3, 0, 24, 0x0f000000},
	{MODE_OFDM, 4, 1, 4, 0x000000f0},
	{MODE_OFDM, 5, 1, 0, 0x0000000f},
	{MODE_OFDM, 6, 1, 12, 0x0000f000},
	{MODE_OFDM, 7, 1, 8, 0x00000f00}
#ifdef DOT11_N_SUPPORT
	, {MODE_HTMIX, 0, 1, 20, 0x00f00000},
	{MODE_HTMIX, 1, 1, 16, 0x000f0000},
	{MODE_HTMIX, 2, 1, 28, 0xf0000000},
	{MODE_HTMIX, 3, 1, 24, 0x0f000000},
	{MODE_HTMIX, 4, 2, 4, 0x000000f0},
	{MODE_HTMIX, 5, 2, 0, 0x0000000f},
	{MODE_HTMIX, 6, 2, 12, 0x0000f000},
	{MODE_HTMIX, 7, 2, 8, 0x00000f00},
	{MODE_HTMIX, 8, 2, 20, 0x00f00000},
	{MODE_HTMIX, 9, 2, 16, 0x000f0000},
	{MODE_HTMIX, 10, 2, 28, 0xf0000000},
	{MODE_HTMIX, 11, 2, 24, 0x0f000000},
	{MODE_HTMIX, 12, 3, 4, 0x000000f0},
	{MODE_HTMIX, 13, 3, 0, 0x0000000f},
	{MODE_HTMIX, 14, 3, 12, 0x0000f000},
	{MODE_HTMIX, 15, 3, 8, 0x00000f00}
#endif /* DOT11_N_SUPPORT */
};
#define MAX_TXPWR_TAB_SIZE (sizeof(TxPwrCfg) / sizeof(TX_PWR_CFG))


CHAR RTMP_GetTxPwr(RTMP_ADAPTER *pAd, HTTRANSMIT_SETTING HTTxMode, UCHAR Channel, struct wifi_dev *wdev)
{
	UINT32 Value;
	INT Idx;
	UINT8 PhyMode;
	CHAR CurTxPwr;
	UINT8 TxPwrRef = 0;
	CHAR DaltaPwr;
	ULONG TxPwr[5];
	UCHAR cen_ch = wlan_operate_get_cen_ch_1(wdev);
	UCHAR bw = wlan_operate_get_bw(wdev);
	UINT8 TxPath = pAd->Antenna.field.TxPath;
#ifdef SINGLE_SKU
	CurTxPwr = pAd->CommonCfg.DefineMaxTxPwr;
#else
	CurTxPwr = 19;
#endif /* SINGLE_SKU */

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (pAd->bAntennaSetAPEnable[BandIdx])
			TxPath = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	/* check Tx Power setting from UI. */
	if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 90)
		;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 60)  /* reduce Pwr for 1 dB. */
		CurTxPwr -= 1;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 30)  /* reduce Pwr for 3 dB. */
		CurTxPwr -= 3;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 15)  /* reduce Pwr for 6 dB. */
		CurTxPwr -= 6;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 9)   /* reduce Pwr for 9 dB. */
		CurTxPwr -= 9;
	else                                           /* reduce Pwr for 12 dB. */
		CurTxPwr -= 12;

	if (bw == BW_40) {
		if (cen_ch > 14) {
			TxPwr[0] = pAd->Tx40MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgABand[4];
		} else {
			TxPwr[0] = pAd->Tx40MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgGBand[4];
		}
	} else {
		if (Channel > 14) {
			TxPwr[0] = pAd->Tx20MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgABand[4];
		} else {
			TxPwr[0] = pAd->Tx20MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgGBand[4];
		}
	}

	switch (HTTxMode.field.MODE) {
	case MODE_CCK:
	case MODE_OFDM:
		Value = TxPwr[1];
		TxPwrRef = (Value & 0x00000f00) >> 8;
		break;
#ifdef DOT11_N_SUPPORT

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		if (TxPath == 1) {
			Value = TxPwr[2];
			TxPwrRef = (Value & 0x00000f00) >> 8;
		} else if (TxPath == 2) {
			Value = TxPwr[3];
			TxPwrRef = (Value & 0x00000f00) >> 8;
		}

		break;
#endif /* DOT11_N_SUPPORT */
	}

	PhyMode =
#ifdef DOT11_N_SUPPORT
		(HTTxMode.field.MODE == MODE_HTGREENFIELD)
		? MODE_HTMIX :
#endif /* DOT11_N_SUPPORT */
		HTTxMode.field.MODE;

	for (Idx = 0; Idx < MAX_TXPWR_TAB_SIZE; Idx++) {
		if ((TxPwrCfg[Idx].Mode == PhyMode)
			&& (TxPwrCfg[Idx].MCS == HTTxMode.field.MCS)) {
			Value = TxPwr[TxPwrCfg[Idx].req];
			DaltaPwr = TxPwrRef - (CHAR)((Value & TxPwrCfg[Idx].BitMask)
										 >> TxPwrCfg[Idx].shift);
			CurTxPwr -= DaltaPwr;
			break;
		}
	}

	return CurTxPwr;
}


NDIS_STATUS	MeasureReqTabInit(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;

	os_alloc_mem(pAd, (UCHAR **) &(pAd->CommonCfg.pMeasureReqTab), sizeof(MEASURE_REQ_TAB));

	if (pAd->CommonCfg.pMeasureReqTab) {
		NdisZeroMemory(pAd->CommonCfg.pMeasureReqTab, sizeof(MEASURE_REQ_TAB));
		NdisAllocateSpinLock(pAd, &pAd->CommonCfg.MeasureReqTabLock);
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Fail to alloc memory for pAd->CommonCfg.pMeasureReqTab.\n");
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


VOID MeasureReqTabExit(RTMP_ADAPTER *pAd)
{
	if (pAd->CommonCfg.pMeasureReqTab) {
		os_free_mem(pAd->CommonCfg.pMeasureReqTab);
		pAd->CommonCfg.pMeasureReqTab = NULL;
		NdisFreeSpinLock(&pAd->CommonCfg.MeasureReqTabLock);
	}

	return;
}


PMEASURE_REQ_ENTRY MeasureReqLookUp(RTMP_ADAPTER *pAd, UINT8 DialogToken, UINT8 measuretype)
{
	UINT HashIdx;
	PMEASURE_REQ_TAB pTab = pAd->CommonCfg.pMeasureReqTab;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	PMEASURE_REQ_ENTRY pPrevEntry = NULL;

	if (pTab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pMeasureReqTab doesn't exist.\n");
		return NULL;
	}

	RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);
	HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(DialogToken);
	pEntry = pTab->Hash[HashIdx];

	while (pEntry) {
		if ((pEntry->DialogToken == DialogToken) && (pEntry->measuretype == measuretype))
			break;
		else {
			pPrevEntry = pEntry;
			pEntry = pEntry->pNext;
		}
	}

	RTMP_SEM_UNLOCK(&pAd->CommonCfg.MeasureReqTabLock);
	return pEntry;
}


PMEASURE_REQ_ENTRY MeasureReqInsert(RTMP_ADAPTER *pAd, UINT8 DialogToken, UINT8 measuretype)
{
	INT i;
	ULONG HashIdx;
	PMEASURE_REQ_TAB pTab = pAd->CommonCfg.pMeasureReqTab;
	PMEASURE_REQ_ENTRY pEntry = NULL, pCurrEntry;
	ULONG Now;

	if (pTab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pMeasureReqTab doesn't exist.\n");
		return NULL;
	}

	pEntry = MeasureReqLookUp(pAd, DialogToken, measuretype);

	if (pEntry == NULL) {
		RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);

		for (i = 0; i < MAX_MEASURE_REQ_TAB_SIZE; i++) {
			NdisGetSystemUpTime(&Now);
			pEntry = &pTab->Content[i];

			if ((pEntry->Valid == TRUE)
				&& RTMP_TIME_AFTER((unsigned long)Now,
					(unsigned long)(pEntry->lastTime + MQ_REQ_AGE_OUT))
				&& pEntry->skip_time_check == FALSE) {

				PMEASURE_REQ_ENTRY pPrevEntry = NULL;
				ULONG HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
				PMEASURE_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];
				BOOLEAN Cancelled;

				/* update Hash list*/
				do {
					if (pProbeEntry == pEntry) {
						if (pPrevEntry == NULL)
							pTab->Hash[HashIdx] = pEntry->pNext;
						else
							pPrevEntry->pNext = pEntry->pNext;

						break;
					}

					pPrevEntry = pProbeEntry;
					pProbeEntry = pProbeEntry->pNext;
				} while (pProbeEntry);

				RTMPCancelTimer(&pEntry->WaitBCNRepTimer, &Cancelled);
				RTMPReleaseTimer(&pEntry->WaitBCNRepTimer, &Cancelled);
				RTMPCancelTimer(&pEntry->WaitNRRspTimer, &Cancelled);
				RTMPReleaseTimer(&pEntry->WaitNRRspTimer, &Cancelled);
				NdisZeroMemory(pEntry, sizeof(MEASURE_REQ_ENTRY));
				pTab->Size--;
				break;
			}

			if (pEntry->Valid == FALSE)
				break;
		}

		if (i < MAX_MEASURE_REQ_TAB_SIZE) {
			NdisGetSystemUpTime(&Now);
			pEntry->lastTime = Now;
			pEntry->Valid = TRUE;
			pEntry->DialogToken = DialogToken;
			pEntry->measuretype = measuretype;
			pEntry->skip_time_check = FALSE;
			pEntry->RcvBcnRepCnt = 0;
			pTab->Size++;
		} else {
			pEntry = NULL;
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pMeasureReqTab tab full.\n");
		}

		/* add this Neighbor entry into HASH table*/
		if (pEntry) {
			HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(DialogToken);

			if (pTab->Hash[HashIdx] == NULL)
				pTab->Hash[HashIdx] = pEntry;
			else {
				pCurrEntry = pTab->Hash[HashIdx];

				while (pCurrEntry->pNext != NULL)
					pCurrEntry = pCurrEntry->pNext;

				pCurrEntry->pNext = pEntry;
			}
		}

		RTMP_SEM_UNLOCK(&pAd->CommonCfg.MeasureReqTabLock);
	}

	return pEntry;
}


VOID MeasureReqDelete(RTMP_ADAPTER *pAd, UINT8 DialogToken, UINT8 measuretype)
{
	PMEASURE_REQ_TAB pTab = pAd->CommonCfg.pMeasureReqTab;
	PMEASURE_REQ_ENTRY pEntry = NULL;

	if (pTab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pMeasureReqTab doesn't exist.\n");
		return;
	}

	/* if empty, return*/
	if (pTab->Size == 0) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pMeasureReqTab empty.\n");
		return;
	}

	pEntry = MeasureReqLookUp(pAd, DialogToken, measuretype);

	if (pEntry != NULL) {
		PMEASURE_REQ_ENTRY pPrevEntry = NULL;
		ULONG HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
		PMEASURE_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

		RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);

		/* update Hash list*/
		do {
			if (pProbeEntry == pEntry) {
				if (pPrevEntry == NULL)
					pTab->Hash[HashIdx] = pEntry->pNext;
				else
					pPrevEntry->pNext = pEntry->pNext;

				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);

		NdisZeroMemory(pEntry, sizeof(MEASURE_REQ_ENTRY));
		pTab->Size--;
		RTMP_SEM_UNLOCK(&pAd->CommonCfg.MeasureReqTabLock);
	}

	return;
}

#ifdef TPC_SUPPORT
NDIS_STATUS TpcReqTabInit(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;

	os_alloc_mem(pAd, (UCHAR **) &(pAd->CommonCfg.pTpcReqTab), sizeof(TPC_REQ_TAB));

	if (pAd->CommonCfg.pTpcReqTab) {
		NdisZeroMemory(pAd->CommonCfg.pTpcReqTab, sizeof(TPC_REQ_TAB));
		NdisAllocateSpinLock(pAd, &pAd->CommonCfg.TpcReqTabLock);
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Fail to alloc memory for pAd->CommonCfg.pTpcReqTab.\n");
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


VOID TpcReqTabExit(RTMP_ADAPTER *pAd)
{
	if (pAd->CommonCfg.pTpcReqTab) {
		os_free_mem(pAd->CommonCfg.pTpcReqTab);
		pAd->CommonCfg.pTpcReqTab = NULL;
		NdisFreeSpinLock(&pAd->CommonCfg.TpcReqTabLock);
	}

	return;
}


static PTPC_REQ_ENTRY TpcReqLookUp(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	UINT HashIdx;
	PTPC_REQ_TAB pTab = pAd->CommonCfg.pTpcReqTab;
	PTPC_REQ_ENTRY pEntry = NULL;
	PTPC_REQ_ENTRY pPrevEntry = NULL;

	if (pTab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTpcReqTab doesn't exist.\n");
		return NULL;
	}

	RTMP_SEM_LOCK(&pAd->CommonCfg.TpcReqTabLock);
	HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(DialogToken);
	pEntry = pTab->Hash[HashIdx];

	while (pEntry) {
		if (pEntry->DialogToken == DialogToken)
			break;
		else {
			pPrevEntry = pEntry;
			pEntry = pEntry->pNext;
		}
	}

	RTMP_SEM_UNLOCK(&pAd->CommonCfg.TpcReqTabLock);
	return pEntry;
}


static PTPC_REQ_ENTRY TpcReqInsert(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	INT i;
	ULONG HashIdx;
	PTPC_REQ_TAB pTab = pAd->CommonCfg.pTpcReqTab;
	PTPC_REQ_ENTRY pEntry = NULL, pCurrEntry;
	ULONG Now;
#ifdef TPC_MODE_CTRL
	BOOLEAN Cancelled;
#endif

	if (pTab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTpcReqTab doesn't exist.\n");
		return NULL;
	}

	pEntry = TpcReqLookUp(pAd, DialogToken);

	if (pEntry == NULL) {
		RTMP_SEM_LOCK(&pAd->CommonCfg.TpcReqTabLock);

		for (i = 0; i < MAX_TPC_REQ_TAB_SIZE; i++) {
			NdisGetSystemUpTime(&Now);
			pEntry = &pTab->Content[i];

			if ((pEntry->Valid == TRUE)
				&& RTMP_TIME_AFTER((unsigned long)Now, (unsigned long)(pEntry->lastTime + TPC_REQ_AGE_OUT))) {
				PTPC_REQ_ENTRY pPrevEntry = NULL;
				ULONG HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
				PTPC_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

				/* update Hash list*/
				do {
					if (pProbeEntry == pEntry) {
						if (pPrevEntry == NULL)
							pTab->Hash[HashIdx] = pEntry->pNext;
						else
							pPrevEntry->pNext = pEntry->pNext;

						break;
					}

					pPrevEntry = pProbeEntry;
					pProbeEntry = pProbeEntry->pNext;
				} while (pProbeEntry);
#ifdef TPC_MODE_CTRL
				RTMPCancelTimer(&pEntry->WaitTPCRspTimer, &Cancelled);
				RTMPReleaseTimer(&pEntry->WaitTPCRspTimer, &Cancelled);
#endif
				NdisZeroMemory(pEntry, sizeof(TPC_REQ_ENTRY));
				pTab->Size--;
				break;
			}

			if (pEntry->Valid == FALSE)
				break;
		}

		if (i < MAX_TPC_REQ_TAB_SIZE) {
			NdisGetSystemUpTime(&Now);
			pEntry->lastTime = Now;
			pEntry->Valid = TRUE;
			pEntry->DialogToken = DialogToken;
			pTab->Size++;
		} else {
			pEntry = NULL;
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTpcReqTab tab full.\n");
		}

		/* add this Neighbor entry into HASH table*/
		if (pEntry) {
			HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(DialogToken);

			if (pTab->Hash[HashIdx] == NULL)
				pTab->Hash[HashIdx] = pEntry;
			else {
				pCurrEntry = pTab->Hash[HashIdx];

				while (pCurrEntry->pNext != NULL)
					pCurrEntry = pCurrEntry->pNext;

				pCurrEntry->pNext = pEntry;
			}
		}

		RTMP_SEM_UNLOCK(&pAd->CommonCfg.TpcReqTabLock);
	}

	return pEntry;
}

static VOID TpcReqDelete(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	PTPC_REQ_TAB pTab = pAd->CommonCfg.pTpcReqTab;
	PTPC_REQ_ENTRY pEntry = NULL;

	if (pTab == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTpcReqTab doesn't exist.\n");
		return;
	}

	/* if empty, return*/
	if (pTab->Size == 0) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTpcReqTab empty.\n");
		return;
	}

	pEntry = TpcReqLookUp(pAd, DialogToken);

	if (pEntry != NULL) {
		PTPC_REQ_ENTRY pPrevEntry = NULL;
		ULONG HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
		PTPC_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

		RTMP_SEM_LOCK(&pAd->CommonCfg.TpcReqTabLock);

		/* update Hash list*/
		do {
			if (pProbeEntry == pEntry) {
				if (pPrevEntry == NULL)
					pTab->Hash[HashIdx] = pEntry->pNext;
				else
					pPrevEntry->pNext = pEntry->pNext;

				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);

		NdisZeroMemory(pEntry, sizeof(TPC_REQ_ENTRY));
		pTab->Size--;
		RTMP_SEM_UNLOCK(&pAd->CommonCfg.TpcReqTabLock);
	}

	return;
}
UINT8 GetMaxTxPwr(RTMP_ADAPTER *pAd)
{
#ifdef TPC_MODE_CTRL
	INT8 val0 = 0;
	UINT8 retPwr = 0x16;/*22dBm*/

	if (pAd->CommonCfg.ctrlTPC.pwr_ofdm6m == -127)
		return retPwr;
	val0 = (pAd->CommonCfg.ctrlTPC.pwr_ofdm6m & 0xff);
	if (val0 <= 0x3f)
		retPwr = ((UINT8)val0 >> 1);
	return retPwr;
#else
	return 0x16;/* 22 dBm */
#endif
}
#endif /* TPC_SUPPORT */


/*
	==========================================================================
	Description:
		Get Current TimeS tamp.

	Parametrs:

	Return	: Current Time Stamp.
	==========================================================================
 */
static UINT64 GetCurrentTimeStamp(RTMP_ADAPTER *pAd)
{
	/* get current time stamp.*/
	return 0;
}


/*
	==========================================================================
	Description:
		Get Maximum Transmit Power.

	Parametrs:

	Return	: Current Transmit Power.
	==========================================================================
 */

/*
	==========================================================================
	Description:
		Get Current Transmit Power.

	Parametrs:

	Return	: Current Time Stamp.
	==========================================================================
 */
VOID InsertChannelRepIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN RTMP_STRING *pCountry,
	IN UINT8 RegulatoryClass,
	IN UINT8 *ChReptList,
	IN USHORT PhyMode,
	IN UINT8 IfIdx
)
{
	ULONG TempLen;
	UINT8 Len;
	UINT8 IEId = IE_AP_CHANNEL_REPORT;
	PUCHAR pChListPtr = NULL;
	UCHAR ChannelList[16] = {0};
	UINT8 NumberOfChannels = 0;
	UINT8 *pChannelList = NULL;
	PUCHAR channel_set = NULL;
	UCHAR channel_set_num;
	UCHAR ch_list_num = 0;
	UINT i, j;
#ifdef OCE_SUPPORT
	UINT8 k, t;
	struct wifi_dev *wdev;
	BOOLEAN Same = FALSE;
	BSS_TABLE *ScanTab = NULL;
	BSS_ENTRY *pBssEntry = NULL;
#endif /* OCE_SUPPORT */

	if (RegulatoryClass == 0)
		return;

	Len = 1;
#ifdef OCE_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
	NumberOfChannels = 0;
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	if (IS_OCE_RNR_ENABLE(wdev)) {

		/* not vap and no nr */
		if (ScanTab->BssNr == 0 && pAd->ApCfg.BssidNum == 0)
			return;

		if (pAd->ApCfg.BssidNum > 1)
		ChannelList[NumberOfChannels++] = wdev->channel;

		for (i = 0; i < ScanTab->BssNr; i++) {
			pBssEntry = &ScanTab->BssEntry[i];

			Same = FALSE;
			for (j = 0; j < NumberOfChannels; j++) {
				if (pBssEntry->Channel == ChannelList[j]) {
					Same = TRUE;
					break;
				}
			}
			if (Same == FALSE)
				ChannelList[NumberOfChannels++] = pBssEntry->Channel;

		}
		if (NumberOfChannels > 0) {
			for (i = 0; i < NumberOfChannels-1; i++) {
				for (j = i+1, k = i; j < NumberOfChannels; j++) {
					if (ChannelList[j] < ChannelList[k])
						k = j;
				}
			t = ChannelList[k];
			ChannelList[k] = ChannelList[i];
			ChannelList[i] = t;
			}
		}

		pChannelList = &ChannelList[0];
		Len += NumberOfChannels;
		pChListPtr = pChannelList;

	} else
#endif /* OCE_SUPPORT */
	{
		channel_set = get_channelset_by_reg_class(pAd, RegulatoryClass, PhyMode);
		channel_set_num = get_channel_set_num(channel_set);

		ch_list_num = get_channel_set_num(ChReptList);

		/* no match channel set. */
		if (channel_set == NULL)
			return;

		/* empty channel set. */
		if (channel_set_num == 0)
			return;

		if (ch_list_num) { /* assign partial channel list */
			for (i = 0; i < channel_set_num; i++) {
				for (j = 0; j < ch_list_num; j++) {
					if (ChReptList[j] == channel_set[i])
						ChannelList[NumberOfChannels++] = channel_set[i];
				}
			}

			pChannelList = &ChannelList[0];
		} else {
			NumberOfChannels = channel_set_num;
			pChannelList = channel_set;
		}

		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "%s: Requlatory class (%d), NumberOfChannels=%d, channel_set_num=%d\n",
				  __func__, RegulatoryClass, NumberOfChannels, channel_set_num);
		Len += NumberOfChannels;
		pChListPtr = pChannelList;
	}

	if (Len > 1) {
		MakeOutgoingFrame(pFrameBuf,	&TempLen,
						  1,				&IEId,
						  1,				&Len,
						  1,				&RegulatoryClass,
						  Len - 1,			pChListPtr,
						  END_OF_ARGS);
		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}


/*
	==========================================================================
	Description:
		Add last beacon report indication request into beacon request

	Parametrs:

	Return	: NAN
	==========================================================================
 */
VOID InsertBcnReportIndicationReqIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Data
)
{
	ULONG TempLen;
	UINT8 Len;
	UINT8 IEId = IE_LAST_BCN_REPORT_INDICATION_REQUEST;

	Len = 1;

	MakeOutgoingFrame(pFrameBuf,	&TempLen,
					  1,				&IEId,
					  1,				&Len,
					  1,				&Data,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;

	return;
}



/*
	==========================================================================
	Description:
		Insert Dialog Token into frame.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. Dialog token.

	Return	: None.
	==========================================================================
 */
VOID InsertDialogToken(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 DialogToken)
{
	ULONG TempLen;

	MakeOutgoingFrame(pFrameBuf,	&TempLen,
					  1,				&DialogToken,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}


/*
	==========================================================================
	Description:
		Insert TPC Request IE into frame.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.

	Return	: None.
	==========================================================================
 */
static VOID InsertTpcReqIE(RTMP_ADAPTER *pAd, UCHAR *frm_buf, ULONG *frm_len)
{
	ULONG TempLen;
	UINT8 Len = 0;
	UINT8 ElementID = IE_TPC_REQUEST;

	MakeOutgoingFrame(frm_buf,					&TempLen,
					  1,							&ElementID,
					  1,							&Len,
					  END_OF_ARGS);
	*frm_len = *frm_len + TempLen;
	return;
}


/*
	==========================================================================
	Description:
		Insert TPC Report IE into frame.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. Transmit Power.
		4. Link Margin.

	Return	: None.
	==========================================================================
 */
VOID InsertTpcReportIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 TxPwr,
	IN UINT8 LinkMargin)
{
	ULONG TempLen;
	UINT8 Len = sizeof(TPC_REPORT_INFO);
	UINT8 ElementID = IE_TPC_REPORT;
	TPC_REPORT_INFO TpcReportIE;

	TpcReportIE.TxPwr = TxPwr;
	TpcReportIE.LinkMargin = LinkMargin;
	MakeOutgoingFrame(pFrameBuf,					&TempLen,
					  1,							&ElementID,
					  1,							&Len,
					  Len,						&TpcReportIE,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}

/*
	==========================================================================
	Description:
		Insert Measure Request IE into frame.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. Measure Token.
		4. Measure Request Mode.
		5. Measure Request Type.
		6. Measure Channel.
		7. Measure Start time.
		8. Measure Duration.


	Return	: None.
	==========================================================================
 */
static VOID InsertMeasureReqIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Len,
	IN PMEASURE_REQ_INFO pMeasureReqIE)
{
	ULONG TempLen;
	UINT8 ElementID = IE_MEASUREMENT_REQUEST;

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
					  1,							&ElementID,
					  1,							&Len,
					  sizeof(MEASURE_REQ_INFO),	pMeasureReqIE,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}


/*
	==========================================================================
	Description:
		Insert Measure Report IE into frame.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. Measure Token.
		4. Measure Request Mode.
		5. Measure Request Type.
		6. Length of Report Infomation
		7. Pointer of Report Infomation Buffer.

	Return	: None.
	==========================================================================
 */
static VOID InsertMeasureReportIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PMEASURE_REPORT_INFO pMeasureReportIE,
	IN UINT8 ReportLnfoLen,
	IN PUINT8 pReportInfo)
{
	ULONG TempLen;
	UINT8 Len;
	UINT8 ElementID = IE_MEASUREMENT_REPORT;

	Len = sizeof(MEASURE_REPORT_INFO) + ReportLnfoLen;
	MakeOutgoingFrame(pFrameBuf,					&TempLen,
					  1,							&ElementID,
					  1,							&Len,
					  Len,						pMeasureReportIE,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;

	if ((ReportLnfoLen > 0) && (pReportInfo != NULL)) {
		MakeOutgoingFrame(pFrameBuf + *pFrameLen,		&TempLen,
						  ReportLnfoLen,				pReportInfo,
						  END_OF_ARGS);
		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}


/*
	==========================================================================
	Description:
		Prepare Measurement request action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
VOID MakeMeasurementReqFrame(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pOutBuffer,
	OUT PULONG pFrameLen,
	IN UINT8 TotalLen,
	IN UINT8 Category,
	IN UINT8 Action,
	IN UINT8 MeasureToken,
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT16 NumOfRepetitions)
{
	ULONG TempLen;
	MEASURE_REQ_INFO MeasureReqIE;
	UINT16 leRepetitions = cpu2le16(NumOfRepetitions);

	InsertActField(pAd, (pOutBuffer + *pFrameLen), pFrameLen, Category, Action);
	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + *pFrameLen), pFrameLen, MeasureToken);

	/* fill Number of repetitions. */
	if (Category == CATEGORY_RM) {
		MakeOutgoingFrame((pOutBuffer + *pFrameLen),	&TempLen,
						  2,							&leRepetitions,
						  END_OF_ARGS);
		*pFrameLen += TempLen;
	}

	/* prepare Measurement IE.*/
	NdisZeroMemory(&MeasureReqIE, sizeof(MEASURE_REQ_INFO));
	MeasureReqIE.Token = MeasureToken;
	MeasureReqIE.ReqMode.word = MeasureReqMode;
	MeasureReqIE.ReqType = MeasureReqType;
	InsertMeasureReqIE(pAd, (pOutBuffer + *pFrameLen), pFrameLen,
					   TotalLen, &MeasureReqIE);
	return;
}


/*
	==========================================================================
	Description:
		Prepare Measurement report action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
VOID EnqueueMeasurementRep(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pDA,
	IN UINT8 DialogToken,
	IN UINT8 MeasureToken,
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT8 ReportInfoLen,
	IN PUINT8 pReportInfo)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	HEADER_802_11 ActHdr;
	MEASURE_REPORT_INFO MeasureRepIE;
	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
					 pAd->CurrentAddress,
					 pAd->CurrentAddress);
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_MRP);
	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	/* prepare Measurement IE.*/
	NdisZeroMemory(&MeasureRepIE, sizeof(MEASURE_REPORT_INFO));
	MeasureRepIE.Token = MeasureToken;
	MeasureRepIE.ReportMode = MeasureReqMode;
	MeasureRepIE.ReportType = MeasureReqType;
	InsertMeasureReportIE(pAd, (pOutBuffer + FrameLen), &FrameLen, &MeasureRepIE, ReportInfoLen, pReportInfo);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	return;
}


/*
	==========================================================================
	Description:
		Prepare TPC Request action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
#ifdef TPC_SUPPORT
	UCHAR EnqueueTPCReq(
		IN RTMP_ADAPTER *pAd,
		IN PUCHAR pDA,
		IN PUCHAR pSA,
		IN PUCHAR pBssid,
		IN UCHAR DialogToken)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	HEADER_802_11 ActHdr;
	/* build action frame header.*/
	MgtMacHeaderInitExt(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
					 pSA,
					 pBssid);
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return FALSE;
	}

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_TPCRQ);
	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	/* Insert TPC Request IE.*/
	InsertTpcReqIE(pAd, (pOutBuffer + FrameLen), &FrameLen);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	return TRUE;
}


/*
	==========================================================================
	Description:
		Prepare TPC Report action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.

	Return	: None.
	==========================================================================
 */
VOID EnqueueTPCRep(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pDA,
	IN PUCHAR pSA,
	IN PUCHAR pBssid,
	IN UINT8 DialogToken,
	IN UINT8 TxPwr,
	IN UINT8 LinkMargin)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	HEADER_802_11 ActHdr;
	/* build action frame header.*/
	MgtMacHeaderInitExt(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
					 pSA,
					 pBssid);
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_TPCRP);
	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	/* Insert TPC Request IE.*/
	InsertTpcReportIE(pAd, (pOutBuffer + FrameLen), &FrameLen, TxPwr, LinkMargin);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	return;
}
#endif


/*
	==========================================================================
	Description:
		Insert Channel Switch Announcement IE into frame.

	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. channel switch announcement mode.
		4. new selected channel.
		5. channel switch announcement count.

	Return	: None.
	==========================================================================
 */
static VOID InsertChSwAnnIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ChSwMode,
	IN UINT8 NewChannel,
	IN UINT8 ChSwCnt)
{
	ULONG TempLen;
	ULONG Len = sizeof(CH_SW_ANN_INFO);
	UINT8 ElementID = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
	CH_SW_ANN_INFO ChSwAnnIE;

	ChSwAnnIE.ChSwMode = ChSwMode;
	ChSwAnnIE.Channel = NewChannel;
	ChSwAnnIE.ChSwCnt = ChSwCnt;
	MakeOutgoingFrame(pFrameBuf,				&TempLen,
					  1,						&ElementID,
					  1,						&Len,
					  Len,					&ChSwAnnIE,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}


#ifdef DOT11_N_SUPPORT
static VOID InsertSecondaryChOffsetIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Offset)
{
	ULONG TempLen;
	ULONG Len = sizeof(SEC_CHA_OFFSET_IE);
	UINT8 ElementID = IE_SECONDARY_CH_OFFSET;
	SEC_CHA_OFFSET_IE SChOffIE;

	SChOffIE.SecondaryChannelOffset = Offset;

	MakeOutgoingFrame(pFrameBuf,	&TempLen,
				1,	&ElementID,
				1,	&Len,
				Len,	&SChOffIE,
				END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}
#endif

#define BW_LT_20M(_bw) ((((_bw) > BW_20) && ((_bw) <= BW_160)) || ((_bw) == BW_8080))
#define BW_LT_40M(_bw) ((((_bw) > BW_40) && ((_bw) <= BW_160)) || ((_bw) == BW_8080))
#define BW_EQ_160M(_bw) (((_bw) == BW_160) || ((_bw) == BW_8080))

#ifdef DOT11_VHT_AC
static VOID InsertWideBWChnlSwitchIESpecificCh(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pfrmbuf,
	OUT PULONG pfrmlen,
	IN UINT8 target_ch,
	IN UINT8 target_bw)
{
	ULONG temp_len;
	ULONG len = sizeof(WIDE_BW_CH_SWITCH_ELEMENT);
	UINT8 element_ID = IE_WIDE_BW_CH_SWITCH;
	WIDE_BW_CH_SWITCH_ELEMENT wb_info;

	NdisZeroMemory(&wb_info, sizeof(WIDE_BW_CH_SWITCH_ELEMENT));

	if (BW_LT_40M(target_bw)) {
		wb_info.new_ch_width = 1;
		wb_info.center_freq_1 = vht_cent_ch_freq(target_ch, VHT_BW_80, CMD_CH_BAND_5G);
		/* We expect that DUT would switch to continuous 8080,
		 * we have to fix it if we would switch to non-continuous 8080. */
		if (BW_EQ_160M(target_bw))
			wb_info.center_freq_2 = vht_cent_ch_freq(target_ch, VHT_BW_160, CMD_CH_BAND_5G);
	}

	MakeOutgoingFrame(pfrmbuf,	&temp_len,
				1,	&element_ID,
				1,	&len,
				len,	&wb_info,
				END_OF_ARGS);

	*pfrmlen = *pfrmlen + temp_len;
}
#endif

void ap_chnl_switch_xmit(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN UINT8 target_ch, IN UINT8 target_bw)
{
	PUCHAR pch_sw_frm_buf = NULL;
	NDIS_STATUS NStatus = NDIS_STATUS_FAILURE;
	ULONG chnl_switch_frm_len;
	HEADER_802_11 ActHdr;
	struct DOT11_H *pDot11h = NULL;
	UCHAR ChSwCnt = 0;

	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, BROADCAST_ADDR, wdev->if_addr, wdev->bssid);
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pch_sw_frm_buf);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}
	NdisMoveMemory(pch_sw_frm_buf, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	chnl_switch_frm_len = sizeof(HEADER_802_11);
	InsertActField(pAd, (pch_sw_frm_buf + chnl_switch_frm_len), &chnl_switch_frm_len, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);
	pDot11h = wdev->pDot11_H;
	ChSwCnt = pDot11h->CSPeriod - pDot11h->CSCount - 1;

	InsertChSwAnnIE(pAd, (pch_sw_frm_buf + chnl_switch_frm_len), &chnl_switch_frm_len, CS_ANN_MODE_TX_ALLOW, target_ch, ChSwCnt);
	if (BW_LT_20M(target_bw)) {
		/*Insert secondary Channel IE if BW >= 40*/
		UCHAR extcha = EXTCHA_ABOVE;
		UCHAR ht_bw = HT_BW_40;

		ht_ext_cha_adjust(pAd, target_ch, &ht_bw, &extcha, wdev);
		InsertSecondaryChOffsetIE(pAd, (pch_sw_frm_buf + chnl_switch_frm_len), &chnl_switch_frm_len, extcha);
#ifdef DOT11_VHT_AC
		/*Insert Wide BW Channel switch IE if BW > 40*/
		InsertWideBWChnlSwitchIESpecificCh(pAd, (pch_sw_frm_buf + chnl_switch_frm_len), &chnl_switch_frm_len, target_ch, target_bw);
#endif
	}

	MiniportMMRequest(pAd, QID_AC_BK, pch_sw_frm_buf, chnl_switch_frm_len);
	MlmeFreeMemory(pch_sw_frm_buf);
}


#ifdef WDS_SUPPORT
/*
	==========================================================================
	Description:
		Prepare Channel Switch Announcement action frame and enqueue it into
		management queue waiting for transmition.

	Parametrs:
		1. the destination mac address of the frame.
		2. Channel switch announcement mode.
		2. a New selected channel.

	Return	: None.
	==========================================================================
 */
VOID EnqueueChSwAnn(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pDA,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	HEADER_802_11 ActHdr;
	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
					 pAd->CurrentAddress,
					 pAd->CurrentAddress);
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);
	InsertChSwAnnIE(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, NewCh, 0);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	return;
}
#endif /* WDS_SUPPORT */


static BOOLEAN DfsRequirementCheck(RTMP_ADAPTER *pAd, UINT8 Channel)
{
	BOOLEAN Result = FALSE;
	UCHAR ch_idx;
	UCHAR BandIdx = HcGetBandByChannelRange(pAd, Channel);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	do {
		/* check DFS procedure is running.*/
		/* make sure DFS procedure won't start twice.*/
		if (pAd->Dot11_H[BandIdx].RDMode != RD_NORMAL_MODE) {
			Result = FALSE;
			break;
		}

		/* check the new channel carried from Channel Switch Announcemnet is valid.*/
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if ((Channel == pChCtrl->ChList[ch_idx].Channel)
				&& (pChCtrl->ChList[ch_idx].NonOccupancy == 0)) {
				/* found radar signal in the channel. the channel can't use at least for 30 minutes.*/
				pChCtrl->ChList[ch_idx].NonOccupancy = 1800 ;/*30 min = 1800 sec*/
				Result = TRUE;
				break;
			}
		}
	} while (FALSE);

	return Result;
}


VOID NotifyChSwAnnToPeerAPs(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pRA,
	IN PUCHAR pTA,
	IN UINT8 ChSwMode,
	IN UINT8 Channel)
{
#ifdef WDS_SUPPORT

	if (!((pRA[0] & 0xff) == 0xff)) { /* is pRA a broadcase address.*/
		INT i;

		/* info neighbor APs that Radar signal found throgh WDS link.*/
		for (i = 0; i < MAX_WDS_ENTRY; i++) {
			if (wds_entry_is_valid(pAd, i)) {
				PUCHAR pDA = pAd->WdsTab.WdsEntry[i].PeerWdsAddr;

				/* DA equal to SA. have no necessary orignal AP which found Radar signal.*/
				if (MAC_ADDR_EQUAL(pTA, pDA))
					continue;

				/* send Channel Switch Action frame to info Neighbro APs.*/
				EnqueueChSwAnn(pAd, pDA, ChSwMode, Channel);
			}
		}
	}

#endif /* WDS_SUPPORT */
}


static VOID StartDFSProcedure(RTMP_ADAPTER *pAd, UCHAR Channel, UINT8 ChSwMode)
{
	/* start DFS procedure*/
	pAd->Dot11_H[0].RDMode = RD_SWITCHING_MODE;
	pAd->Dot11_H[0].CSCount = 0;
}


/*
	==========================================================================
	Description:
		Channel Switch Announcement action frame sanity check.

	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Channel switch announcement infomation buffer.


	Return	: None.
	==========================================================================
 */

/*
  Channel Switch Announcement IE.
  +----+-----+-----------+------------+-----------+
  | ID | Len |Ch Sw Mode | New Ch Num | Ch Sw Cnt |
  +----+-----+-----------+------------+-----------+
    1    1        1           1            1
*/
static BOOLEAN PeerChSwAnnSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PCH_SW_ANN_INFO pChSwAnnInfo)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;
	/* skip 802.11 header.*/
	MsgLen -= sizeof(HEADER_802_11);
	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pChSwAnnInfo == NULL)
		return result;

	eid_ptr = (PEID_STRUCT)pFramePtr;

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
			if (parse_ch_switch_announcement_ie(eid_ptr)) {
				NdisMoveMemory(&pChSwAnnInfo->ChSwMode, eid_ptr->Octet, 1);
				NdisMoveMemory(&pChSwAnnInfo->Channel, eid_ptr->Octet + 1, 1);
				NdisMoveMemory(&pChSwAnnInfo->ChSwCnt, eid_ptr->Octet + 2, 1);
				result = TRUE;
			} else {
				MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s() - wrong IE_CHANNEL_SWITCH_ANNOUNCEMENT\n", __func__);
				result = FALSE;
				return result;
			}
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;
}


/*
	==========================================================================
	Description:
		Measurement request action frame sanity check.

	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Measurement request infomation buffer.

	Return	: None.
	==========================================================================
 */
static BOOLEAN PeerMeasureReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PMEASURE_REQ_INFO pMeasureReqInfo,
	OUT PMEASURE_REQ pMeasureReq)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;
	PUCHAR ptr;
	UINT64 MeasureStartTime;
	UINT16 MeasureDuration;
	/* skip 802.11 header.*/
	MsgLen -= sizeof(HEADER_802_11);
	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pMeasureReqInfo == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;
	eid_ptr = (PEID_STRUCT)pFramePtr;

	if (parse_measurement_ie(eid_ptr->Len) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"Beacon Request length incorrect. Abort parsing\n");
		goto len_error;
	}

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_MEASUREMENT_REQUEST:
			NdisMoveMemory(&pMeasureReqInfo->Token, eid_ptr->Octet, 1);
			NdisMoveMemory(&pMeasureReqInfo->ReqMode.word, eid_ptr->Octet + 1, 1);
			NdisMoveMemory(&pMeasureReqInfo->ReqType, eid_ptr->Octet + 2, 1);
			ptr = (PUCHAR)(eid_ptr->Octet + 3);
			NdisMoveMemory(&pMeasureReq->ChNum, ptr, 1);
			NdisMoveMemory(&MeasureStartTime, ptr + 1, 8);
			pMeasureReq->MeasureStartTime = SWAP64(MeasureStartTime);
			NdisMoveMemory(&MeasureDuration, ptr + 9, 2);
			pMeasureReq->MeasureDuration = SWAP16(MeasureDuration);
			result = TRUE;
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

len_error:
	return result;
}


/*
	==========================================================================
	Description:
		Measurement report action frame sanity check.

	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Measurement report infomation buffer.
		4. basic report infomation buffer.

	Return	: None.
	==========================================================================
 */

/*
  Measurement Report IE.
  +----+-----+-------+-------------+--------------+----------------+
  | ID | Len | Token | Report Mode | Measure Type | Measure Report |
  +----+-----+-------+-------------+--------------+----------------+
    1     1      1          1             1            variable

  Basic Report.
  +--------+------------+----------+-----+
  | Ch Num | Start Time | Duration | Map |
  +--------+------------+----------+-----+
      1          8           2        1

  Map Field Bit Format.
  +-----+---------------+---------------------+-------+------------+----------+
  | Bss | OFDM Preamble | Unidentified signal | Radar | Unmeasured | Reserved |
  +-----+---------------+---------------------+-------+------------+----------+
     0          1                  2              3         4          5-7
*/
static BOOLEAN PeerMeasureReportSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PMEASURE_REPORT_INFO pMeasureReportInfo,
	OUT PUINT8 pReportBuf)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;
	PUCHAR ptr;
	/* skip 802.11 header.*/
	MsgLen -= sizeof(HEADER_802_11);
	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pMeasureReportInfo == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;
	eid_ptr = (PEID_STRUCT)pFramePtr;

	if (parse_measurement_ie(eid_ptr->Len) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"Beacon Report length incorrect. Abort parsing\n");
		goto len_error;
	}

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_MEASUREMENT_REPORT:
			NdisMoveMemory(&pMeasureReportInfo->Token, eid_ptr->Octet, 1);
			NdisMoveMemory(&pMeasureReportInfo->ReportMode, eid_ptr->Octet + 1, 1);
			NdisMoveMemory(&pMeasureReportInfo->ReportType, eid_ptr->Octet + 2, 1);

			if (pMeasureReportInfo->ReportType == RM_BASIC) {
				PMEASURE_BASIC_REPORT pReport = (PMEASURE_BASIC_REPORT)pReportBuf;

				ptr = (PUCHAR)(eid_ptr->Octet + 3);
				NdisMoveMemory(&pReport->ChNum, ptr, 1);
				NdisMoveMemory(&pReport->MeasureStartTime, ptr + 1, 8);
				NdisMoveMemory(&pReport->MeasureDuration, ptr + 9, 2);
				NdisMoveMemory(&pReport->Map, ptr + 11, 1);
			} else if (pMeasureReportInfo->ReportType == RM_CCA) {
				PMEASURE_CCA_REPORT pReport = (PMEASURE_CCA_REPORT)pReportBuf;

				ptr = (PUCHAR)(eid_ptr->Octet + 3);
				NdisMoveMemory(&pReport->ChNum, ptr, 1);
				NdisMoveMemory(&pReport->MeasureStartTime, ptr + 1, 8);
				NdisMoveMemory(&pReport->MeasureDuration, ptr + 9, 2);
				NdisMoveMemory(&pReport->CCA_Busy_Fraction, ptr + 11, 1);
			} else if (pMeasureReportInfo->ReportType == RM_RPI_HISTOGRAM) {
				PMEASURE_RPI_REPORT pReport = (PMEASURE_RPI_REPORT)pReportBuf;

				ptr = (PUCHAR)(eid_ptr->Octet + 3);
				NdisMoveMemory(&pReport->ChNum, ptr, 1);
				NdisMoveMemory(&pReport->MeasureStartTime, ptr + 1, 8);
				NdisMoveMemory(&pReport->MeasureDuration, ptr + 9, 2);
				NdisMoveMemory(&pReport->RPI_Density, ptr + 11, 8);
			}

			result = TRUE;
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

len_error:
	return result;
}


#ifdef TPC_SUPPORT
/*
	==========================================================================
	Description:
		TPC Request action frame sanity check.

	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Dialog Token.

	Return	: None.
	==========================================================================
 */
static BOOLEAN PeerTpcReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;

	MsgLen -= sizeof(HEADER_802_11);
	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pDialogToken == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;
	eid_ptr = (PEID_STRUCT)pFramePtr;

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_TPC_REQUEST:
			result = TRUE;
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;
}


/*
	==========================================================================
	Description:
		TPC Report action frame sanity check.

	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Dialog Token.
		4. TPC Report IE.

	Return	: None.
	==========================================================================
 */
static BOOLEAN PeerTpcRepSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PTPC_REPORT_INFO pTpcRepInfo)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;

	MsgLen -= sizeof(HEADER_802_11);
	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pDialogToken == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;
	eid_ptr = (PEID_STRUCT)pFramePtr;

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_TPC_REPORT:
			if (parse_tpc_report_ie(eid_ptr)) {
				NdisMoveMemory(&pTpcRepInfo->TxPwr, eid_ptr->Octet, 1);
				NdisMoveMemory(&pTpcRepInfo->LinkMargin, eid_ptr->Octet + 1, 1);
				result = TRUE;
			} else {
				result = FALSE;
				return result;
			}
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;
}
#endif /* TPC_SUPPORT */


/*
	==========================================================================
	Description:
		Channel Switch Announcement action frame handler.

	Parametrs:
		Elme - MLME message containing the received frame

	Return	: None.
	==========================================================================
 */
static VOID PeerChSwAnnAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	CH_SW_ANN_INFO ChSwAnnInfo;
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
#ifdef CONFIG_STA_SUPPORT
	UCHAR index = 0, NewChannel = 0;
	ULONG Bssidx = 0;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = (struct wifi_dev *)Elem->wdev;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT
	PMAC_TABLE_ENTRY pEntry = NULL;
	BCN_IE_LIST bcn_ie_list;
#endif /* APCLI_SUPPORT */

	NdisZeroMemory(&ChSwAnnInfo, sizeof(CH_SW_ANN_INFO));

	if (!PeerChSwAnnSanity(pAd, Elem->Msg, Elem->MsgLen, &ChSwAnnInfo)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Invalid Channel Switch Action Frame.\n");
		return;
	}
#ifdef APCLI_SUPPORT
	if (!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Invalid WCID for Channel Switch Action Frame.\n"));
		return;
	}

	pEntry = &pAd->MacTab.Content[Elem->Wcid];
	if (pEntry) {
		bcn_ie_list.NewChannel = ChSwAnnInfo.Channel;
		ApCliPeerCsaAction(pAd, pEntry->wdev, &bcn_ie_list);
	}
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	/* ChSwAnn need check.*/
	if ((wdev->wdev_type == WDEV_TYPE_AP) &&
		(DfsRequirementCheck(pAd, ChSwAnnInfo.Channel) == TRUE)) {
		NotifyChSwAnnToPeerAPs(pAd, pFr->Hdr.Addr1, pFr->Hdr.Addr2, ChSwAnnInfo.ChSwMode, ChSwAnnInfo.Channel);
		StartDFSProcedure(pAd, ChSwAnnInfo.Channel, ChSwAnnInfo.ChSwMode);
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, Elem->wdev);
		Bssidx = BssTableSearch(ScanTab, pFr->Hdr.Addr3, pStaCfg->wdev.channel);

		if ((Bssidx == BSS_NOT_FOUND) || (Bssidx >=  MAX_LEN_OF_BSS_TABLE)) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "PeerChSwAnnAction - Bssidx is not found\n");
			return;
		}

		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\nBssidx is %d, Channel = %d\n", index, ScanTab->BssEntry[Bssidx].Channel);
		hex_dump("SSID", ScanTab->BssEntry[Bssidx].Bssid, 6);

		/* ChannelSwitchAction() will modify pStaCfg->wdev.channel first , and let Channel equal to NewChannel*/
		/* Channel = pStaCfg->wdev.channel; */
		NewChannel = ChSwAnnInfo.Channel;

		if ((pAd->CommonCfg.bIEEE80211H == 1) && (NewChannel != 0)
				&& (pStaCfg->wdev.quick_ch_change == QUICK_CH_SWICH_DISABLE)
#ifdef MAP_R2
				&& (IS_MAP_TURNKEY_ENABLE(pAd) && (pStaCfg->wdev.channel != NewChannel))
#endif /* MAP_R2 */
			) {
			/* Switching to channel 1 can prevent from rescanning the current channel immediately (by auto reconnection).*/
			/* In addition, clear the MLME queue and the scan table to discard the RX packets and previous scanning results.*/
			pStaCfg->wdev.channel = 1;
			wlan_operate_set_prim_ch(&pStaCfg->wdev, 1);
			LinkDown(pAd, FALSE, Elem->wdev, Elem);
			MlmeResetByWdev(pAd, wdev);
			RtmpusecDelay(1000000);		/* use delay to prevent STA do reassoc*/

			/* channel sanity check*/
			for (index = 0; index < pChCtrl->ChListNum; index++) {
				if (pChCtrl->ChList[index].Channel == NewChannel) {
					ScanTab->BssEntry[Bssidx].Channel = NewChannel;
					pStaCfg->wdev.channel = NewChannel;
					wlan_operate_set_prim_ch(&pStaCfg->wdev, pStaCfg->wdev.channel);
					MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s():Receive channel switch announcement IE (New Channel =%d)\n",
							 __func__,
							 NewChannel);
					break;
				}
			}

			if (index >= pChCtrl->ChListNum) {
				MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "can not find New Channel=%d in ChannelList[%d]\n",
						 pStaCfg->wdev.channel, pChCtrl->ChListNum);
			}
		}
	}

#endif /* CONFIG_STA_SUPPORT */
	return;
}


/*
	==========================================================================
	Description:
		Measurement Request action frame handler.

	Parametrs:
		Elme - MLME message containing the received frame

	Return	: None.
	==========================================================================
 */
static VOID PeerMeasureReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	UINT8 DialogToken;
	MEASURE_REQ_INFO MeasureReqInfo;
	MEASURE_REQ	MeasureReq;
	MEASURE_REPORT_MODE ReportMode;

	if (PeerMeasureReqSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &MeasureReqInfo, &MeasureReq)) {
		ReportMode.word = 0;
		ReportMode.field.Incapable = 1;
		EnqueueMeasurementRep(pAd, pFr->Hdr.Addr2, DialogToken, MeasureReqInfo.Token, ReportMode.word, MeasureReqInfo.ReqType,
							  0, NULL);
	}

	return;
}


/*
	==========================================================================
	Description:
		Measurement Report action frame handler.

	Parametrs:
		Elme - MLME message containing the received frame

	Return	: None.
	==========================================================================
 */
static VOID PeerMeasureReportAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MEASURE_REPORT_INFO MeasureReportInfo;
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	UINT8 DialogToken;
	PUINT8 pMeasureReportInfo;

	os_alloc_mem(pAd, (UCHAR **)&pMeasureReportInfo, sizeof(MEASURE_RPI_REPORT));

	if (pMeasureReportInfo == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "unable to alloc memory for measure report buffer (size=%zu).\n", sizeof(MEASURE_RPI_REPORT));
		return;
	}

	NdisZeroMemory(&MeasureReportInfo, sizeof(MEASURE_REPORT_INFO));
	NdisZeroMemory(pMeasureReportInfo, sizeof(MEASURE_RPI_REPORT));

	if (PeerMeasureReportSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &MeasureReportInfo, pMeasureReportInfo)) {
		do {
			PMEASURE_REQ_ENTRY pEntry = NULL;

			pEntry = MeasureReqLookUp(pAd, DialogToken, SET_MEASURE_REQ);

			/* Not a autonomous measure report.*/
			/* check the dialog token field. drop it if the dialog token doesn't match.*/
			if ((DialogToken != 0)
				&& (pEntry == NULL))
				break;

			if (pEntry != NULL)
				MeasureReqDelete(pAd, pEntry->DialogToken, SET_MEASURE_REQ);

			if (MeasureReportInfo.ReportType == RM_BASIC) {
				PMEASURE_BASIC_REPORT pBasicReport = (PMEASURE_BASIC_REPORT)pMeasureReportInfo;

				if ((pBasicReport->Map.field.Radar)
					&& (DfsRequirementCheck(pAd, pBasicReport->ChNum) == TRUE)) {
					NotifyChSwAnnToPeerAPs(pAd, pFr->Hdr.Addr1, pFr->Hdr.Addr2, 1, pBasicReport->ChNum);
					StartDFSProcedure(pAd, pBasicReport->ChNum, 1);
				}
			}
		} while (FALSE);
	} else
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Invalid Measurement Report Frame.\n");

	os_free_mem(pMeasureReportInfo);
	return;
}


#ifdef TPC_SUPPORT
/*
	==========================================================================
	Description:
		TPC Request action frame handler.

	Parametrs:
		Elme - MLME message containing the received frame

	Return	: None.
	==========================================================================
 */
static VOID PeerTpcReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	PUCHAR pFramePtr = pFr->Octet;
	UINT8 DialogToken;
	UINT8 TxPwr = 0;
	UINT8 LinkMargin = 0;
	CHAR RealRssi;
	struct wifi_dev *wdev = NULL;

	if (!pAd->CommonCfg.b80211TPC) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(X) b80211TPC=%d\n",
				pAd->CommonCfg.b80211TPC);
		return;
	}
#ifdef TPC_MODE_CTRL
	if (pAd->CommonCfg.ctrlTPC.u1CtrlMode != TPC_MODE_1) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"TPCReq not handle other than MODE1\n");
		return;
	}
#endif
	wdev = Elem->wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Can't Get Wdev\n");
		return;
	}
	/* link margin: Ratio of the received signal power to the minimum desired by the station (STA). The*/
	/*				STA may incorporate rate information and channel conditions, including interference, into its computation*/
	/*				of link margin.*/
	RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
						   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
						   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
	/* skip Category and action code.*/
	pFramePtr += 2;
	/* Dialog token.*/
	NdisMoveMemory(&DialogToken, pFramePtr, 1);
	TxPwr = GetSkuTxPwr(pAd, wdev, SUBTYPE_ACTION);
#ifdef TPC_MODE_CTRL
	LinkMargin = (RealRssi - HIGH_RATE_SENSIT);
	if (pAd->CommonCfg.ctrlTPC.linkmargin != LINK_MARGIN_AUTO_MODE)
		/*For Test Mode, mannually control the link margin.*/
		LinkMargin = pAd->CommonCfg.ctrlTPC.linkmargin;
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"RealRssi=%d,TxPwr[%d],LinkMargin[%d]\n", RealRssi, TxPwr, LinkMargin);
	if (wdev->tpcRssiThld > RealRssi)
		wdev->pwrCnstrnt = 0;
	else if (wdev->isStaNotSprtTpc == FALSE) {
		/*follow far STA link Margin*/
		if (wdev->MinLinkMargin > LinkMargin) {
			wdev->MinLinkMargin = LinkMargin;
			COPY_MAC_ADDR(wdev->mLkMgnAddr, pFr->Hdr.Addr2);
		}
		if (wdev->MinLinkMargin < LINK_MARGIN_6DB)
			wdev->pwrCnstrnt = 0;
		else if (wdev->MinLinkMargin < LINK_MARGIN_9DB)
			wdev->pwrCnstrnt = 3;
		else
			wdev->pwrCnstrnt = 6;
	} else
		wdev->pwrCnstrnt = 0;
	if (wdev->LastpwrCnst != wdev->pwrCnstrnt) {
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
		wdev->LastpwrCnst = wdev->pwrCnstrnt;
	}
#else
	LinkMargin = (RealRssi / MIN_RCV_PWR);
#endif

	if (PeerTpcReqSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken))
		EnqueueTPCRep(pAd, pFr->Hdr.Addr2, wdev->if_addr, wdev->bssid, DialogToken, TxPwr, LinkMargin);
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		":send TPC Report %02x:%02x:%02x:%02x:%02x:%02x -> "
		"%02x:%02x:%02x:%02x:%02x:%02x, bssid:%02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(wdev->if_addr), PRINT_MAC(pFr->Hdr.Addr2), PRINT_MAC(wdev->bssid));
	return;
}


/*
	==========================================================================
	Description:
		TPC Report action frame handler.

	Parametrs:
		Elme - MLME message containing the received frame

	Return	: None.
	==========================================================================
 */
static VOID PeerTpcRepAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	const UINT8 MAX_PWR_LIMIT_DBM = (0x3F >> 1);
	/* Register: signed, only 7 bits for positive integer. Change the unit from 0.5 dBm to 1 dBm */
	UINT8 DialogToken;
	TPC_REPORT_INFO TpcRepInfo;
	PTPC_REQ_ENTRY pEntry = NULL;
	BOOLEAN bUpdated = TRUE;
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	INT MaxTxPower = 0;
#ifdef TPC_MODE_CTRL
	BOOLEAN Cancelled;
	MAC_TABLE_ENTRY *mac_entry = NULL;
	struct wifi_dev *wdev = NULL;
	CHAR rcv_rssi = 0;
	INT8 myLinkMargin = 0;
	INT8 pathloss = 0;
	INT8 peer_rssi_eval = 0;
#endif

	if (!pAd->CommonCfg.b80211TPC) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(X) b80211TPC=%d\n",
				pAd->CommonCfg.b80211TPC);
		return;
	}

	NdisZeroMemory(&TpcRepInfo, sizeof(TPC_REPORT_INFO));

	if (PeerTpcRepSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &TpcRepInfo)) {
		pEntry = TpcReqLookUp(pAd, DialogToken);

		if (pEntry != NULL) {
#ifdef TPC_MODE_CTRL
			RTMPCancelTimer(&pEntry->WaitTPCRspTimer, &Cancelled);
			RTMPReleaseTimer(&pEntry->WaitTPCRspTimer, &Cancelled);
#endif
			TpcReqDelete(pAd, pEntry->DialogToken);
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"DialogToken=%x, TxPwr=%d, LinkMargin=%d\n",
				DialogToken, TpcRepInfo.TxPwr, TpcRepInfo.LinkMargin);
		}
	}
#ifdef TPC_MODE_CTRL
	if (pAd->CommonCfg.ctrlTPC.u1CtrlMode == TPC_MODE_0) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Mode 0 Not Handle TPC Rep\n");
		return;
	}
	rcv_rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
				ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
				ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "rcv_rssi = %d\n", rcv_rssi);
	mac_entry = MacTableLookup(pAd, pFr->Hdr.Addr2);
	if (!mac_entry) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "STA Entry Not Found\n");
		return;
	}
	if (mac_entry->Sst != SST_ASSOC) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "STA disconnected\n");
		return;
	}
	wdev = mac_entry->wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Can't find wdev by entry.\n");
		return;
	}
	if (wdev->tpcRssiThld > rcv_rssi) {
		wdev->pwrCnstrnt = 0;
		mac_entry->tpcPwrAdj = 0;
		goto end;
	}
	if (pAd->CommonCfg.ctrlTPC.u1CtrlMode == TPC_MODE_2) {
		/*Adjust Local Power*/
		if (TpcRepInfo.LinkMargin < LINK_MARGIN_6DB)
			mac_entry->tpcPwrAdj = 0;
		else if (TpcRepInfo.LinkMargin < LINK_MARGIN_9DB)
			mac_entry->tpcPwrAdj = -3;
		else
			mac_entry->tpcPwrAdj = -6;
		/*Protect high rate can be good receive at peer STA*/
		pathloss = TpcRepInfo.TxPwr - rcv_rssi;
		/*evaluate peer's highest rate rssi in the range of sensitivity*/
		peer_rssi_eval = pAd->CommonCfg.ctrlTPC.pwr_mcs11 - pathloss;
		if (peer_rssi_eval < HIGH_RATE_SENSIT)
			mac_entry->tpcPwrAdj = 0;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"Mode 2 peer_rssi_eval[%d]\n", peer_rssi_eval);
	}
	if (pAd->CommonCfg.ctrlTPC.u1CtrlMode == TPC_MODE_1) {
		myLinkMargin = (rcv_rssi - HIGH_RATE_SENSIT);
		if (pAd->CommonCfg.ctrlTPC.linkmargin != LINK_MARGIN_AUTO_MODE)
			/*For Test Mode, mannually control the link margin.*/
			myLinkMargin = pAd->CommonCfg.ctrlTPC.linkmargin;
		if (wdev->isStaNotSprtTpc == FALSE) {
			/*follow smallest STA link Margin*/
			if (wdev->MinLinkMargin > myLinkMargin) {
				wdev->MinLinkMargin = myLinkMargin;
				COPY_MAC_ADDR(wdev->mLkMgnAddr, pFr->Hdr.Addr2);
			}
			if (wdev->MinLinkMargin < LINK_MARGIN_6DB)
				wdev->pwrCnstrnt = 0;
			else if (wdev->MinLinkMargin < LINK_MARGIN_9DB)
				wdev->pwrCnstrnt = 3;
			else
				wdev->pwrCnstrnt = 6;
		} else
			wdev->pwrCnstrnt = 0;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Mode 1\n");
	}
end:
	if (pAd->CommonCfg.ctrlTPC.u1CtrlMode == TPC_MODE_1) {
		if (wdev->LastpwrCnst != wdev->pwrCnstrnt) {
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
			wdev->LastpwrCnst = wdev->pwrCnstrnt;
		}
	}
	if (pAd->CommonCfg.ctrlTPC.u1CtrlMode == TPC_MODE_2) {
		if (mac_entry->tpcOldPwr != mac_entry->tpcPwrAdj) {
			/*update*/
			mac_entry->tpcOldPwr = mac_entry->tpcPwrAdj;
			set_wtbl_pwr_by_entry(pAd, mac_entry);
		}
	}
#else

	MaxTxPower = TpcRepInfo.TxPwr - TpcRepInfo.LinkMargin;

	if (MaxTxPower > MAX_PWR_LIMIT_DBM)
		MaxTxPower = MAX_PWR_LIMIT_DBM;

	MaxTxPower <<= 1;
	/* unit: 0.5 dBm in hardware register */
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MaxTxPower = %d (unit: 0.5 dBm)\n", MaxTxPower);

	if (bUpdated) {
		struct wifi_dev *wdev = NULL;
		MAC_TABLE_ENTRY *mac_entry = MacTableLookup(pAd, pFr->Hdr.Addr2);

		if (mac_entry)
			wdev = mac_entry->wdev;
		else
			wdev = &pAd->ApCfg.MBSSID[0].wdev;

		TxPowerTpcFeatureCtrl(pAd, wdev, (INT8)MaxTxPower);
	}
#endif
	return;
}
#endif /* TPC_SUPPORT */


/*
	==========================================================================
	Description:
		Spectrun action frames Handler such as channel switch annoucement,
		measurement report, measurement request actions frames.

	Parametrs:
		Elme - MLME message containing the received frame

	Return	: None.
	==========================================================================
 */
VOID PeerSpectrumAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR	Action = Elem->Msg[LENGTH_802_11 + 1];

	if (pAd->CommonCfg.bIEEE80211H != TRUE)
		return;

	switch (Action) {
	case SPEC_MRQ:
		/* current rt2860 unable do such measure specified in Measurement Request.*/
		/* reject all measurement request.*/
		PeerMeasureReqAction(pAd, Elem);
		break;

	case SPEC_MRP:
		PeerMeasureReportAction(pAd, Elem);
		break;
#ifdef TPC_SUPPORT

	case SPEC_TPCRQ:
		PeerTpcReqAction(pAd, Elem);
		break;

	case SPEC_TPCRP:
		PeerTpcRepAction(pAd, Elem);
		break;
#endif /* TPC_SUPPORT */

	case SPEC_CHANNEL_SWITCH:
	{
		struct wifi_dev *wdev = pAd->MacTab.Content[Elem->Wcid].wdev;

		if (!wdev) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev NULL! WCID=%d.\n", Elem->Wcid);
			return;
		}

		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AP ignore CSA action frame sent by client WCID=%d.\n", Elem->Wcid);
			return;
		}

#ifdef CONFIG_RCSA_SUPPORT
		if (pAd->CommonCfg.DfsParameter.bRCSAEn) {
			CSA_IE_INFO CsaInfo = {0};
			struct DOT11_H *pDot11h = wdev->pDot11_H;

			if (apcli_peer_csa_sanity(Elem, &CsaInfo) == FALSE)
				return;

			CsaInfo.wcid = Elem->Wcid;
			if (pAd->CommonCfg.DfsParameter.fUseCsaCfg == TRUE) {
				if (CsaInfo.ChSwAnnIE.ChSwCnt)
					pDot11h->CSPeriod = CsaInfo.ChSwAnnIE.ChSwCnt + 1;
				else if (CsaInfo.ExtChSwAnnIE.ChSwCnt)
					pDot11h->CSPeriod = CsaInfo.ExtChSwAnnIE.ChSwCnt + 1;
			}
			pAd->CommonCfg.DfsParameter.fSendRCSA = TRUE;
			channel_switch_action_1(pAd, &CsaInfo);
		} else
#endif
#ifdef DOT11N_DRAFT3
		{
			SEC_CHA_OFFSET_IE	Secondary;
			CHA_SWITCH_ANNOUNCE_IE	ChannelSwitch;
			NdisZeroMemory(&ChannelSwitch, sizeof(CHA_SWITCH_ANNOUNCE_IE));
			NdisZeroMemory(&Secondary, sizeof(SEC_CHA_OFFSET_IE));
			/* 802.11h only has Channel Switch Announcement IE. */
			RTMPMoveMemory(&ChannelSwitch, &Elem->Msg[LENGTH_802_11 + 4], sizeof(CHA_SWITCH_ANNOUNCE_IE));

			/* 802.11n D3.03 adds secondary channel offset element in the end.*/
			if (Elem->MsgLen ==  (LENGTH_802_11 + 2 + sizeof(CHA_SWITCH_ANNOUNCE_IE) + sizeof(SEC_CHA_OFFSET_IE)))
				RTMPMoveMemory(&Secondary, &Elem->Msg[LENGTH_802_11 + 9], sizeof(SEC_CHA_OFFSET_IE));
			else
				Secondary.SecondaryChannelOffset = 0;

			if ((Elem->Msg[LENGTH_802_11 + 2] == IE_CHANNEL_SWITCH_ANNOUNCEMENT) && (Elem->Msg[LENGTH_802_11 + 3] == 3))
				ChannelSwitchAction(pAd, Elem->Wcid, ChannelSwitch.NewChannel, Secondary.SecondaryChannelOffset);
		}

#endif /* DOT11N_DRAFT3 */
		PeerChSwAnnAction(pAd, Elem);
		break;
	}

	}

	return;
}

/*
	==========================================================================
	Description:

	Parametrs:

	Return	: None.
	==========================================================================
 */
INT Set_MeasureReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT wcid = 1;
	UINT ArgIdx;
	RTMP_STRING *thisChar;
	MEASURE_REQ_MODE MeasureReqMode;
	UINT8 MeasureReqToken = RandomByte(pAd);
	UINT8 MeasureReqType = RM_BASIC;
	UINT8 MeasureCh = 1;
	UINT64 MeasureStartTime = GetCurrentTimeStamp(pAd);
	MEASURE_REQ MeasureReq;
	UINT8 TotalLen;
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		goto END_OF_MEASURE_REQ;
	}

	ArgIdx = 1;

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		switch (ArgIdx) {
		case 1:	/* Aid.*/
			wcid = (UINT16) os_str_tol(thisChar, 0, 16);
			break;

		case 2: /* Measurement Request Type.*/
			MeasureReqType = os_str_tol(thisChar, 0, 16);

			if (MeasureReqType > 3) {
				MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "unknown MeasureReqType(%d)\n", MeasureReqType);
				goto END_OF_MEASURE_REQ;
			}

			break;

		case 3: /* Measurement channel.*/
			MeasureCh = (UINT8) os_str_tol(thisChar, 0, 16);
			break;
		}

		ArgIdx++;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s::wcid = %d, MeasureReqType=%d MeasureCh=%d\n",
			 __func__, wcid, MeasureReqType, MeasureCh);

	if (!IS_WCID_VALID(pAd, wcid)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "unknown sta of wcid(%d)\n", wcid);
		goto END_OF_MEASURE_REQ;
	}

	MeasureReqMode.word = 0;
	MeasureReqMode.field.Enable = 1;
	MeasureReqInsert(pAd, MeasureReqToken, SET_MEASURE_REQ);
	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->MacTab.Content[wcid].Addr,
					 pAd->CurrentAddress,
					 pAd->CurrentAddress);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	TotalLen = sizeof(MEASURE_REQ_INFO) + sizeof(MEASURE_REQ);
	MakeMeasurementReqFrame(pAd, pOutBuffer, &FrameLen,
							sizeof(MEASURE_REQ_INFO), CATEGORY_RM, RM_BASIC,
							MeasureReqToken, MeasureReqMode.word,
							MeasureReqType, 1);
	MeasureReq.ChNum = MeasureCh;
	MeasureReq.MeasureStartTime = cpu2le64(MeasureStartTime);
	MeasureReq.MeasureDuration = cpu2le16(2000);
	{
		ULONG TempLen;

		MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
						  sizeof(MEASURE_REQ),	&MeasureReq,
						  END_OF_ARGS);
		FrameLen += TempLen;
	}
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, (UINT)FrameLen);
END_OF_MEASURE_REQ:
	MlmeFreeMemory(pOutBuffer);
	return TRUE;
}
#ifdef TPC_SUPPORT
INT Set_TpcReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT wcid;
	UINT8 TpcReqToken = RandomByte(pAd);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type;
#ifdef TPC_MODE_CTRL
	PTPC_REQ_ENTRY pTpcEntry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;
	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
	if (pObj->ioctl_if_type == INT_APCLI)
		if (ifIndex < MAX_MULTI_STA)
			wdev = &pAd->StaCfg[ifIndex].wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid MBSS index %d\n", ifIndex);
		return FALSE;
	}
	wcid = (UINT) os_str_tol(arg, 0, 16);
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wcid = %d\n", wcid);

	if (!IS_WCID_VALID(pAd, wcid)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "unknown sta of Aid(%d)\n", wcid);
		return TRUE;
	}
#ifdef TPC_MODE_CTRL
	pTpcEntry = TpcReqInsert(pAd, TpcReqToken);
	if (!pTpcEntry) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "No Tpc Buffer\n");
		return FALSE;
	}
	EnqueueTPCReq(pAd, pAd->MacTab.Content[wcid].Addr, wdev->if_addr, wdev->bssid, TpcReqToken);
	pTpcEntry->Priv = pAd;
	pEntry = &pAd->MacTab.Content[wcid];
	pTpcEntry->wcid = wcid;
	COPY_MAC_ADDR(pTpcEntry->mac, pEntry->Addr);
	RTMPInitTimer(pAd, &pTpcEntry->WaitTPCRspTimer,
	GET_TIMER_FUNCTION(WaitPeerTPCRepTimeout), pTpcEntry, FALSE);
	RTMPSetTimer(&pTpcEntry->WaitTPCRspTimer, 3000);
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"send TPC Req %02x:%02x:%02x:%02x:%02x:%02x -> "
		"%02x:%02x:%02x:%02x:%02x:%02x, bssid:%02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(wdev->if_addr), PRINT_MAC(pAd->MacTab.Content[wcid].Addr),
		PRINT_MAC(wdev->bssid));
#else
	TpcReqInsert(pAd, TpcReqToken);
	EnqueueTPCReq(pAd, pAd->MacTab.Content[wcid].Addr, wdev->if_addr, wdev->bssid, TpcReqToken);
#endif
	return TRUE;
}

INT Set_TpcReqByAddr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char *value;
	UINT8 macAddr[MAC_ADDR_LEN] = {0};
	INT i;
	UINT8 TpcReqToken = RandomByte(pAd);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type;
#ifdef TPC_MODE_CTRL
	PTPC_REQ_ENTRY pTpcEntry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;
	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
	if (pObj->ioctl_if_type == INT_APCLI)
		if (ifIndex < MAX_MULTI_STA)
			wdev = &pAd->StaCfg[ifIndex].wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid MBSS index %d\n", ifIndex);
		return FALSE;
	}
	if (strlen(arg) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "TpcReqToken = %d (0x%02X)\n", TpcReqToken, TpcReqToken);

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid */

		AtoH(value, (UINT8 *)&macAddr[i++], 1);
	}
#ifdef TPC_MODE_CTRL
	pTpcEntry = TpcReqInsert(pAd, TpcReqToken);
	if (!pTpcEntry) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "No Tpc Buffer\n");
		return FALSE;
	}
	EnqueueTPCReq(pAd, macAddr, wdev->if_addr, wdev->bssid, TpcReqToken);
	pEntry = MacTableLookup(pAd, macAddr);/* Found the pEntry from Peer Bcn Content */
	pTpcEntry->Priv = pAd;
	if (pEntry) {
		pTpcEntry->wcid = pEntry->wcid;
		COPY_MAC_ADDR(pTpcEntry->mac, pEntry->Addr);
	} else {
		pTpcEntry->wcid = 0;
		COPY_MAC_ADDR(pTpcEntry->mac, macAddr);
	}
	RTMPInitTimer(pAd, &pTpcEntry->WaitTPCRspTimer,
	GET_TIMER_FUNCTION(WaitPeerTPCRepTimeout), pTpcEntry, FALSE);
	RTMPSetTimer(&pTpcEntry->WaitTPCRspTimer, 3000);
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"send TPC Req %02x:%02x:%02x:%02x:%02x:%02x -> "
		"%02x:%02x:%02x:%02x:%02x:%02x, bssid:%02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(wdev->if_addr), PRINT_MAC(macAddr), PRINT_MAC(wdev->bssid));
#else
	TpcReqInsert(pAd, TpcReqToken);
	EnqueueTPCReq(pAd, macAddr, wdev->if_addr, wdev->bssid, TpcReqToken);
#endif

	return TRUE;
}

INT Set_TpcCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef TPC_MODE_CTRL
	#define CMD_NUM 4
	UINT16 wcid = 1;
#else
	const UINT CMD_NUM = 3;
#endif
	UINT arg_len, i, j, *cmd_pos = NULL;
	UINT BandIdx, CentCh;
	INT Power;

	os_alloc_mem(pAd, (UCHAR **) &cmd_pos, CMD_NUM);
	if (cmd_pos == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Failed to allocate memory for cmd\n");
		return FALSE;
	}

	arg_len = strlen(arg);
	cmd_pos[0] = 0;
	j = 1;

	for (i = 0; i  < arg_len; i++) {
		if (arg[i] == ':') {
			cmd_pos[j++] = i + 1;
			arg[i] = 0;
		}
	}

	if (j != CMD_NUM) {
		MTWF_PRINT("usage format is [band:power:channel], power unit is 0.5 dBm\n\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "e.g.\n\n");
#ifdef TPC_MODE_CTRL
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"iwpriv ra0 set TpcCtrl=0:62:6:1\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"means (band 0), (31 dBm), (channel 6), (wcid 1)\n\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set TpcCtrl=1:10:100:2\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"means (band 1), (5 dBm), (channel 100), (wcid 2)\n\n");
#else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "iwpriv ra0 set TpcCtrl=0:62:6\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "means (band 0), (31 dBm), (channel 6)\n\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "iwpriv ra0 set TpcCtrl=1:10:100\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "means (band 1), (5 dBm), (channel 100)\n\n");
#endif
		os_free_mem(cmd_pos);
		return TRUE;
	}

	BandIdx = os_str_tol(arg + cmd_pos[0], 0, 10);
	Power = os_str_tol(arg + cmd_pos[1], 0, 10);
	CentCh = os_str_tol(arg + cmd_pos[2], 0, 10);
#ifdef TPC_MODE_CTRL
	wcid = os_str_tol(arg + cmd_pos[3], 0, 10);
	if (Power > 0) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "6'b000000: 0db\n"
			"6'b000000: 0db\n"
			"6'b111111: -0.25db\n"
			"6'b111110: -0.5db\n"
			"¡­. Implicit sign bit in bit[6] = 1 (negative)\n"
			"6'b000010: -15.25db\n"
			"6'b000001: -15.5db\n");
		Power = 0;
		return FALSE;
	}
	TxPowerTpcFeatureForceCtrl(pAd, Power, BandIdx, CentCh, wcid);
#else
	if (Power > 63)
		Power = 63;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BandIdx=%d, Power=%d, CentCh=%d\n", BandIdx, Power, CentCh);
	TxPowerTpcFeatureForceCtrl(pAd, Power, BandIdx, CentCh);
#endif
	os_free_mem(cmd_pos);
	return TRUE;
}

INT Set_TpcEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 enable;

	enable = os_str_tol(arg, 0, 10);

	if (enable != FALSE)
		enable = TRUE;

	MTWF_PRINT("%s(): %d -> %d\n", __func__, pAd->CommonCfg.b80211TPC, enable);
	pAd->CommonCfg.b80211TPC = enable;
	return TRUE;
}

#endif /* TPC_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
INT Set_PwrConstraint(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	typedef struct __PWR_CONSTRAIN_CFG {
		CHAR Attenuation;
		UINT8 ucTxPowerPercentage;
	} PWR_CONSTRAIN_CFG;
	PWR_CONSTRAIN_CFG PwrConstrainTab[] = {
		{0, 100},
		{1, 70},
		{4, 50},
		{6, 20},
		{10, 10},
		{13, 5}
	};
#define PWR_CONSTRAION_TAB_SIZE \
	(sizeof(PwrConstrainTab)/sizeof(PWR_CONSTRAIN_CFG))
	INT Idx;
	LONG Value;
	CHAR MaxTxPwr;
	CHAR CurTxPwr;
	CHAR DaltaPwr;
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[0];

	Value = (UINT) os_str_tol(arg, 0, 10);
	MaxTxPwr = GetRegulatoryMaxTxPwr(pAd, pEntry->wdev->channel, pEntry->wdev) - (CHAR)Value;
	CurTxPwr = RTMP_GetTxPwr(pAd, pEntry->HTPhyMode, pEntry->wdev->channel, pEntry->wdev);
	DaltaPwr = CurTxPwr - MaxTxPwr;

	if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 90)
		DaltaPwr += 0;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 60)  /* reduce Pwr for 1 dB. */
		DaltaPwr += 1;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 30)  /* reduce Pwr for 3 dB. */
		DaltaPwr += 3;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 15)  /* reduce Pwr for 6 dB. */
		DaltaPwr += 6;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 9)   /* reduce Pwr for 9 dB. */
		DaltaPwr += 9;
	else                                            /* reduce Pwr for 12 dB. */
		DaltaPwr += 12;

	MTWF_PRINT("MaxTxPwr=%d, CurTxPwr=%d, DaltaPwr=%d\n",
			 MaxTxPwr, CurTxPwr, DaltaPwr);

	for (Idx = 0; Idx < PWR_CONSTRAION_TAB_SIZE; Idx++) {
		if (DaltaPwr < PwrConstrainTab[Idx].Attenuation) {
			pAd->CommonCfg.PwrConstraint = Value;
			pAd->CommonCfg.ucTxPowerPercentage[BAND0] = PwrConstrainTab[Idx].ucTxPowerPercentage;
			break;
		}
	}

	if (Idx == PWR_CONSTRAION_TAB_SIZE) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, \
				 "Power constraint value be in range from 0 to 13dB\n");
	}

	return TRUE;
}

#ifdef DOT11K_RRM_SUPPORT
#endif /* DOT11K_RRM_SUPPORT */

VOID RguClass_BuildBcnChList(RTMP_ADAPTER *pAd, UCHAR *pBuf, ULONG *pBufLen, struct wifi_dev *wdev, UCHAR RegClass)
{
	/* INT loop; */
	ULONG TmpLen;
	PUCHAR channel_set = NULL;
	UCHAR channel_set_num, MaxTxPwr;
	UINT8 i, ChSetMinLimPwr;

	ChSetMinLimPwr = 0xff;

	if (RegClass == 0)
		return;

	channel_set = get_channelset_by_reg_class(pAd, RegClass, wdev->PhyMode);
	channel_set_num = get_channel_set_num(channel_set);

	/* no match channel set. */
	if (channel_set == NULL)
		return;

	/* empty channel set. */
	if (channel_set_num == 0)
		return;

	/*
		There is many channel which have different limit tx power
		we choose the minimum
	*/
	for (i = 0; i < channel_set_num; i++) {
		MaxTxPwr = GetRegulatoryMaxTxPwr(pAd, channel_set[i], wdev);

		if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "CN", 2))
			MaxTxPwr = pAd->MaxTxPwr;/*for CN CountryCode*/

		if (MaxTxPwr < ChSetMinLimPwr)
			ChSetMinLimPwr = MaxTxPwr;
	}

	MakeOutgoingFrame(pBuf + *pBufLen,		&TmpLen,
					  1,		&channel_set[0],
					  1,		&channel_set_num,
					  1,		&ChSetMinLimPwr,
					  END_OF_ARGS);
	*pBufLen += TmpLen;
	return;
}
#endif /* CONFIG_AP_SUPPORT */

#if defined(CONFIG_RCSA_SUPPORT) || defined(ZERO_LOSS_CSA_SUPPORT)
#ifdef DOT11_VHT_AC
static VOID InsertWideBWChSwitchIE(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 NewCh,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	ULONG TempLen;
	ULONG Len = sizeof(WIDE_BW_CH_SWITCH_ELEMENT);
	UINT8 ElementID = IE_WIDE_BW_CH_SWITCH;
	WIDE_BW_CH_SWITCH_ELEMENT wb_info = {0};
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR vht_bw = wlan_config_get_vht_bw(wdev);
	UCHAR ch_band = wlan_config_get_ch_band(wdev);

	if (op_ht_bw == BW_40) {
		switch (vht_bw) {
		case VHT_BW_2040:
			wb_info.new_ch_width = 0;
		break;
		case VHT_BW_80:
			wb_info.new_ch_width = 1;
			wb_info.center_freq_1 = vht_cent_ch_freq(NewCh, vht_bw, ch_band);
			wb_info.center_freq_2 = 0;
		break;
		case VHT_BW_160:
//def DOT11_VHT_R2
			wb_info.new_ch_width = 1;
			wb_info.center_freq_1 = (vht_cent_ch_freq(wdev->channel, vht_bw, ch_band) - 8);
			wb_info.center_freq_2 = vht_cent_ch_freq(wdev->channel, vht_bw, ch_band);
		break;
		case VHT_BW_8080:
//def DOT11_VHT_R2
			wb_info.new_ch_width = 1;
			wb_info.center_freq_1 = vht_cent_ch_freq(wdev->channel, vht_bw, ch_band);
			wb_info.center_freq_2 = wlan_operate_get_cen_ch_2(wdev);
		break;
		}
		MakeOutgoingFrame(pFrameBuf,	&TempLen,
					1,	&ElementID,
					1,      &Len,
					Len,    &wb_info,
					END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}
}
#endif
#endif /*CONFIG_RCSA_SUPPORT || ZERO_LOSS_CSA_SUPPORT*/

#ifdef CONFIG_RCSA_SUPPORT
static VOID InsertExtChSwAnnIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ChSwMode,
	IN UINT8 RegClass,
	IN UINT8 NewChannel,
	IN UINT8 ChSwCnt)
{
	ULONG TempLen;
	ULONG Len = sizeof(EXT_CH_SW_ANN_INFO);
	UINT8 ElementID = IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT;
	EXT_CH_SW_ANN_INFO ExtChSwAnnIE;

	ExtChSwAnnIE.ChSwMode = ChSwMode;
	ExtChSwAnnIE.RegClass = RegClass;
	ExtChSwAnnIE.Channel = NewChannel;
	ExtChSwAnnIE.ChSwCnt = ChSwCnt;
	MakeOutgoingFrame(pFrameBuf,				&TempLen,
					  1,						&ElementID,
					  1,						&Len,
					  Len,						&ExtChSwAnnIE,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

#ifdef APCLI_SUPPORT
VOID EnqueueChSwAnnApCli(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ifIndex,
	IN UINT8 NewCh,
	IN UINT8 ChSwMode)
{
	PUCHAR pOutBuffer = NULL;
	UCHAR ChSwCnt = 0, RegClass;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	HEADER_802_11 ActHdr;
	struct DOT11_H *pDot11h = wdev->pDot11_H;

	/* build action frame header.*/
	ApCliMgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->StaCfg[ifIndex].MlmeAux.Bssid,
					 pAd->StaCfg[ifIndex].MlmeAux.Bssid, ifIndex);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() allocate memory failed\n", __func__);
		return;
	}

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() NULL wdev\n", __func__);
		return;
	}

	ChSwCnt = pDot11h->CSPeriod - pDot11h->CSCount - 1;

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);
	InsertChSwAnnIE(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, NewCh, ChSwCnt);

#ifdef DOT11_N_SUPPORT
	InsertSecondaryChOffsetIE(pAd, (pOutBuffer + FrameLen), &FrameLen, wlan_config_get_ext_cha(wdev));

	RegClass = get_regulatory_class_for_newCh(pAd, NewCh, wdev->PhyMode, wdev);
	InsertExtChSwAnnIE(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, RegClass, NewCh, ChSwCnt);
#endif

#ifdef DOT11_VHT_AC
	InsertWideBWChSwitchIE(pAd, wdev, NewCh, (pOutBuffer + FrameLen), &FrameLen);
#endif

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) %s::MiniportMMRequest\n",
		ifIndex, __func__);
	MiniportMMRequest(pAd, AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:: <--Exit\n", __func__);
}
#endif

INT notify_channel_switch_to_backhaulAP(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UINT8 Channel,
	IN UINT8 ChSwMode)
{
	INT8 inf_idx;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:Channel = %d, ChSwMode = %d\n", __func__, Channel, ChSwMode);

	for (inf_idx = 0; inf_idx < MAX_APCLI_NUM; inf_idx++) {
		if (pAd->StaCfg[inf_idx].wdev.channel == wdev->channel) {
			EnqueueChSwAnnApCli(pAd, wdev, inf_idx, Channel, ChSwMode);
			return TRUE;
		}
	}

	return FALSE;
}

INT apcli_peer_csa_sanity(
	MLME_QUEUE_ELEM * Elem,
	CSA_IE_INFO *CsaInfo)
{
	UCHAR action, IE_ID, Length = 0, status = FALSE;

	action = Elem->Msg[LENGTH_802_11 + 1];

	if (action != SPEC_CHANNEL_SWITCH)
		return FALSE;

	Length = LENGTH_802_11 + 2;

	while (Length < Elem->MsgLen) {
		IE_ID = Elem->Msg[Length];

		switch (IE_ID) {
		case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
			RTMPMoveMemory(&CsaInfo->ChSwAnnIE, &Elem->Msg[Length+2], sizeof(CH_SW_ANN_INFO));
			status = TRUE;
		break;

		case IE_SECONDARY_CH_OFFSET:
			CsaInfo->SChOffIE.SecondaryChannelOffset = Elem->Msg[Length+2];
		break;

		case IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT:
			RTMPMoveMemory(&CsaInfo->ExtChSwAnnIE, &Elem->Msg[Length+2], sizeof(EXT_CH_SW_ANN_INFO));
			status = TRUE;
		break;

		case IE_WIDE_BW_CH_SWITCH:
			RTMPMoveMemory(&CsaInfo->wb_info, &Elem->Msg[Length+2], sizeof(WIDE_BW_CH_SWITCH_ELEMENT));
		break;

		default:
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: Unknown IE=%d\n", IE_ID);
		break;
		}
		Length += Elem->Msg[Length+1] + 2;
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: Dump parsed CSA action frame --->\n", __func__);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CSA: Channel:%d ChSwMode:%d CsaCnt:%d\n",
		CsaInfo->ChSwAnnIE.Channel, CsaInfo->ChSwAnnIE.ChSwMode, CsaInfo->ChSwAnnIE.ChSwCnt);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SecChOffSet:%d\n", CsaInfo->SChOffIE.SecondaryChannelOffset);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ExtCSA: Channel:%d RegClass:%d ChSwMode:%d CsaCnt:%d\n",
		CsaInfo->ExtChSwAnnIE.Channel, CsaInfo->ExtChSwAnnIE.RegClass, CsaInfo->ExtChSwAnnIE.ChSwMode, CsaInfo->ExtChSwAnnIE.ChSwCnt);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WB IE: ChWidth:%d  CentFreq:%d CentFreq:%d\n",
		CsaInfo->wb_info.new_ch_width, CsaInfo->wb_info.center_freq_1, CsaInfo->wb_info.center_freq_2);

	return status;
}

VOID channel_switch_action_1(
	IN	RTMP_ADAPTER * pAd,
	IN	CSA_IE_INFO *CsaInfo)
{
	UINT8 BandIdx;
	struct DOT11_H *pDot11h = NULL;
	struct wifi_dev *wdev = pAd->MacTab.Content[CsaInfo->wcid].wdev;

	if (ChannelSwitchSanityCheck(pAd, CsaInfo->wcid, CsaInfo->ChSwAnnIE.Channel, CsaInfo->SChOffIE.SecondaryChannelOffset) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): Channel Sanity check:%d\n", __func__, __LINE__));
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): NewChannel=%d, Secondary=%d -->\n",
				 __func__, CsaInfo->ChSwAnnIE.Channel, CsaInfo->SChOffIE.SecondaryChannelOffset);

	pDot11h = wdev->pDot11_H;

	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Return\n");
		return;
	}

	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		CsaInfo->ChSwAnnIE.Channel != 0 &&
		wdev->channel != CsaInfo->ChSwAnnIE.Channel &&
		pDot11h->RDMode != RD_SWITCHING_MODE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			"[APCLI] Following root AP to switch channel to ch%u\n", CsaInfo->ChSwAnnIE.Channel);

		if ((pAd->CommonCfg.DfsParameter.fUseCsaCfg == FALSE) ||
			(CsaInfo->ChSwAnnIE.ChSwMode == 1)) {
			BandIdx = HcGetBandByWdev(wdev);
			/* Inform FW(N9) about RDD on mesh network */
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[%s] inform N9 about RDD detect BandIdx:%d\n", __func__, BandIdx);
			mtRddControl(pAd, RDD_DETECT_INFO, BandIdx, 0, 0);
		}

		/* Sync wdev settings as per CSA*/
		if (pAd->CommonCfg.DfsParameter.fUseCsaCfg == TRUE) {
#ifdef DOT11_N_SUPPORT
			wlan_config_set_ext_cha(wdev, CsaInfo->SChOffIE.SecondaryChannelOffset);
#endif

#ifdef DOT11_VHT_AC
			wlan_config_set_vht_bw(wdev, CsaInfo->wb_info.new_ch_width);
			wlan_config_set_cen_ch_2(wdev, CsaInfo->wb_info.center_freq_2);
#endif
		}

		pAd->CommonCfg.DfsParameter.ChSwMode = CsaInfo->ChSwAnnIE.ChSwMode;
#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)
		if (wdev->if_dev)
			wapp_send_csa_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), CsaInfo->ChSwAnnIE.Channel);
#endif
		rtmp_set_channel(pAd, wdev, CsaInfo->ChSwAnnIE.Channel);
	}
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): Exit:%d <---\n", __func__, __LINE__);
}

void rcsa_recovery(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;
	UCHAR BandIdx;

	if ((wdev == NULL) || (pAd->CommonCfg.DfsParameter.bRCSAEn == FALSE))
		return;

	pDot11h = wdev->pDot11_H;
	BandIdx = HcGetBandByWdev(wdev);

	if (pDot11h && (pDot11h->RDMode == RD_SILENCE_MODE)) {
		if (pAd->CommonCfg.DfsParameter.fCheckRcsaTxDone == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s::Got TxDone PAUSE ALTX0\n", __func__);
			mtRddControl(pAd, RDD_ALTX_CTRL, BandIdx, 0, 2);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s() RESUME BF RDD_MODE:%d!!!\n", __func__, pDot11h->RDMode);
			mtRddControl(pAd, RDD_RESUME_BF, BandIdx, 0, 0);
			pAd->CommonCfg.DfsParameter.fCheckRcsaTxDone = FALSE;
		}
	}
}
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
VOID InsertChSwAnnIENew(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ChSwMode,
	IN UINT8 NewChannel,
	IN UINT8 ChSwCnt)
{
	ULONG TempLen;
	ULONG Len = sizeof(CH_SW_ANN_INFO);
	UINT8 ElementID = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
	CH_SW_ANN_INFO ChSwAnnIE;

	ChSwAnnIE.ChSwMode = ChSwMode;
	ChSwAnnIE.Channel = NewChannel;
	ChSwAnnIE.ChSwCnt = ChSwCnt;

	MakeOutgoingFrame(pFrameBuf,	&TempLen,
				1,	&ElementID,
				1,	&Len,
				Len,	&ChSwAnnIE,
				END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

VOID InsertExtChSwAnnIENew(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ChSwMode,
	IN UINT8 NewRegClass,
	IN UINT8 NewChannel,
	IN UINT8 ChSwCnt)
{
	ULONG TempLen;
	ULONG Len = sizeof(EXT_CH_SW_ANN_INFO);
	//UINT8 ElementID = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
	EXT_CH_SW_ANN_INFO ExChSwAnnIE;

	ExChSwAnnIE.ChSwMode = ChSwMode;
	ExChSwAnnIE.RegClass = NewRegClass;
	ExChSwAnnIE.Channel = NewChannel;
	ExChSwAnnIE.ChSwCnt = ChSwCnt;

	MakeOutgoingFrame(pFrameBuf,	&TempLen,
				//1,	&ElementID,
				//1,	&Len,
				Len,	&ExChSwAnnIE,
				END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

VOID NotifyBroadcastChSwAnn(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	HEADER_802_11 ActHdr;

	UINT8 ubandidx = HcGetBandByWdev(wdev);
	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, BROADCAST_ADDR, wdev->bssid, wdev->bssid);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"allocate memory failed \n");
		return;
	}
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);

	InsertChSwAnnIENew(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, NewCh, (pAd->Dot11_H[ubandidx].CSPeriod - 1));

#ifdef DOT11_N_SUPPORT
	InsertSecondaryChOffsetIE(pAd, (pOutBuffer + FrameLen), &FrameLen, wlan_operate_get_ext_cha(wdev));
#endif

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(wdev->PhyMode))
		InsertWideBWChSwitchIE(pAd, wdev, NewCh, (pOutBuffer + FrameLen), &FrameLen);
#endif

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	//printk("%s:action frame sent \n",__func__);
	MlmeFreeMemory(pOutBuffer);
	return;
}

VOID NotifyBroadcastExtChSwAnn(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	HEADER_802_11 ActHdr;

	UINT8 ubandidx = HcGetBandByWdev(wdev);
	UINT8 NewRegClass = get_regulatory_class(pAd, NewCh, wdev->PhyMode, wdev);

	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, BROADCAST_ADDR, wdev->bssid, wdev->bssid);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"allocate memory failed \n");
		return;
	}
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_PUBLIC, SPEC_CHANNEL_SWITCH);

	InsertExtChSwAnnIENew(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, NewRegClass, NewCh, (pAd->Dot11_H[ubandidx].CSPeriod - 1));

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(wdev->PhyMode))
		InsertWideBWChSwitchIE(pAd, wdev, NewCh, (pOutBuffer + FrameLen), &FrameLen);
#endif

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
}

VOID EnqueueChSwAnnNew(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	HEADER_802_11 ActHdr;

	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pEntry->Addr, pAd->CurrentAddress, pEntry->bssid);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"allocate memory failed \n");
		return;
	}
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);
#ifdef DOT11_N_SUPPORT
	InsertSecondaryChOffsetIE(pAd, (pOutBuffer + FrameLen), &FrameLen, wlan_operate_get_ext_cha(pEntry->wdev));
#endif
#ifdef DOT11_VHT_AC
	InsertWideBWChSwitchIE(pAd, pEntry->wdev, NewCh, (pOutBuffer + FrameLen), &FrameLen);
#endif
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);

	MlmeFreeMemory(pOutBuffer);
}
#endif /*ZERO_LOSS_CSA_SUPPORT*/

#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
void tpc_req_entry_reset(IN PRTMP_ADAPTER pAd)
{
	pAd->CommonCfg.ctrlTPC.tpcStaCnt = 0;
	NdisZeroMemory(&pAd->CommonCfg.ctrlTPC.sTpcWcid[0], sizeof(pAd->CommonCfg.ctrlTPC.sTpcWcid));
}
void tpc_req_entry_decision(IN PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry)
{
	UINT16 index = 0;
	UINT16 wcid = 0;
	struct wifi_dev *wdev = NULL;

	wdev = pEntry->wdev;
	if (!wdev)
		return;
	/*STA Not support TPC*/
	if ((pEntry->CapabilityInfo & 0x0100) == 0) {
		wdev->isStaNotSprtTpc = TRUE;
		return;
	}
	wcid = pEntry->wcid;
	index = pAd->CommonCfg.ctrlTPC.tpcStaCnt;
	if (index >= MAX_LEN_OF_MAC_TABLE)
		return;
	pAd->CommonCfg.ctrlTPC.sTpcWcid[index] = wcid;
		/*record which wcid need send TPC Request.*/
	pAd->CommonCfg.ctrlTPC.tpcStaCnt++;
}
void tpc_req_monitor(IN PRTMP_ADAPTER pAd)
{
	UCHAR interval = 0;

	if (pAd->CommonCfg.ctrlTPC.u1CtrlMode != TPC_MODE_2)
		return;
	if (pAd->CommonCfg.ctrlTPC.tpcStaCnt == 0)/*no TPC sta connected.*/
		return;
	interval = pAd->CommonCfg.ctrlTPC.sTpcIntval;
	/*Every 5 seconds default, trigger one TPC request in Mode=2*/
	if ((pAd->Mlme.PeriodicRound % interval) == 0)
		send_tpc_request_trigger(pAd);
}
void send_tpc_request_trigger(RTMP_ADAPTER *pAd)
{
	INT ret = 0;
	ULONG MsgLen = 0;
	UINT16 *eMsg = NULL;
	UINT16 i = 0;

	MsgLen = sizeof(pAd->CommonCfg.ctrlTPC.sTpcWcid);
	eMsg = pAd->CommonCfg.ctrlTPC.sTpcWcid;
	/*for Debug*/
	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		if (pAd->CommonCfg.ctrlTPC.sTpcWcid[i] != 0) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"tpcStaCnt%d,STA:%u\n", pAd->CommonCfg.ctrlTPC.tpcStaCnt,
				pAd->CommonCfg.ctrlTPC.sTpcWcid[i]);
			if ((i % 32) == 0)
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
		}
	}
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"send_tpc_request MsgLen = %lu, %p,%p\n",
		MsgLen, pAd->CommonCfg.ctrlTPC.sTpcWcid, eMsg);
	ret = MlmeEnqueue(pAd, ACTION_STATE_MACHINE, MT2_MLME_TPC_REQ, MsgLen, eMsg, 0);
	if (ret)
		RTMP_MLME_HANDLER(pAd);
	else
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "enqueue MLME failed!\n");
}
BOOLEAN wifi_transmit_tpcreq(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, struct wifi_dev *wdev)
{
	PTPC_REQ_ENTRY pTPCEntry = NULL;
	UINT8 TpcReqToken = 0;
	UCHAR ret = FALSE;

	if (pAd->CommonCfg.ctrlTPC.u1CtrlMode == TPC_MODE_0)
		return ret;
	TpcReqToken = RandomByte(pAd);
	pTPCEntry = TpcReqInsert(pAd, TpcReqToken);
	if (!pTPCEntry) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No Tpc Queue Resource\n");
		return ret;
	}
	pTPCEntry->Priv = pAd;
	pTPCEntry->wcid = pEntry->wcid;
	COPY_MAC_ADDR(pTPCEntry->mac, pEntry->Addr);
	ret = EnqueueTPCReq(pAd, pEntry->Addr, wdev->if_addr, wdev->bssid, TpcReqToken);
	if (ret) {
		RTMPInitTimer(pAd, &pTPCEntry->WaitTPCRspTimer,
		GET_TIMER_FUNCTION(WaitPeerTPCRepTimeout), pTPCEntry, FALSE);
		RTMPSetTimer(&pTPCEntry->WaitTPCRspTimer, 3000);
	} else {
		TpcReqDelete(pAd, pTPCEntry->DialogToken);
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wcid[%d] send TPC Req FAIL\n", pEntry->wcid);
	}
	return ret;
}
VOID mlmeAPSendTPCReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UINT16 rwcid[MAX_LEN_OF_MAC_TABLE] = {0};
	UINT16 gwcid = 0, i = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev = NULL;

	if (pAd->CommonCfg.bIEEE80211H != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Not Support 80211H\n");
		return;
	}
	if (Elem->Msg != NULL)
		NdisMoveMemory((UCHAR *)rwcid, Elem->Msg, Elem->MsgLen);
	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		if (i >= pAd->CommonCfg.ctrlTPC.tpcStaCnt)
			break;
		gwcid = rwcid[i];
		if ((gwcid == 0)
			|| !IS_WCID_VALID(pAd, gwcid))
			continue;
		pEntry = &pAd->MacTab.Content[gwcid];
		if (!pEntry)
			continue;
		if (IS_ENTRY_NONE(pEntry) || IS_ENTRY_MCAST(pEntry))
			continue;
		if (pEntry->Sst != SST_ASSOC)
			continue;
		wdev = pEntry->wdev;
		if (!wdev)
			continue;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"wcid[%d] prepare TPC Req %02x:%02x:%02x:%02x:%02x:%02x ->"
			"%02x:%02x:%02x:%02x:%02x:%02x\n", gwcid, PRINT_MAC(wdev->if_addr),
			PRINT_MAC(pEntry->Addr));
		wifi_transmit_tpcreq(pAd, pEntry, wdev);
	}
}
VOID TPCRepTimeout(IN PRTMP_ADAPTER pAd, IN MLME_QUEUE_ELEM *Elem)
{
	PTPC_REQ_ENTRY pEntry = NULL;
	UINT8 *eMsg = (UINT8 *)Elem->Msg;
	UINT8 TPCReqToken = 0;
	BOOLEAN Cancelled;

	NdisMoveMemory(&TPCReqToken, eMsg, Elem->MsgLen);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO,
		"MeasureReqToken=%d\n", TPCReqToken);
	pEntry = TpcReqLookUp(pAd, TPCReqToken);
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			"TPC entry not founded\n");
		return;
	}
	RTMPCancelTimer(&pEntry->WaitTPCRspTimer, &Cancelled);
	RTMPReleaseTimer(&pEntry->WaitTPCRspTimer, &Cancelled);
	TpcReqDelete(pAd, TPCReqToken);
}

VOID WaitPeerTPCRepTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTPC_REQ_ENTRY pTpcEntry = (PTPC_REQ_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
	UINT32 Len = 0;
	UINT8 Event;
	UINT16 wcid = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR mac_addr[MAC_ADDR_LEN] = {0};

	pAd = pTpcEntry->Priv;
	Len = sizeof(Event);
	Event = 0;
	Event = pTpcEntry->DialogToken;
	wcid = pTpcEntry->wcid;
	COPY_MAC_ADDR(mac_addr, pTpcEntry->mac);
	/*if some STA not response TPC request, change it not support TPC*/
	if (VALID_UCAST_ENTRY_WCID(pAd, wcid)) {
		pEntry = &pAd->MacTab.Content[wcid];
		/*conform the same STA*/
		if (!IS_ENTRY_NONE(pEntry) && pEntry->Sst == SST_ASSOC) {
			if (NdisEqualMemory(mac_addr, pEntry->Addr, MAC_ADDR_LEN))
				pEntry->CapabilityInfo &= ~0x100;
		}
	}
	MlmeEnqueue(pAd, ACTION_STATE_MACHINE, MT2_MLME_TPC_REQ_TIMEOUT, Len, (PUCHAR)&Event, 0);
}
BUILD_TIMER_FUNCTION(WaitPeerTPCRepTimeout);

void set_wtbl_pwr_by_entry(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *entry)
{
	CMD_WTBL_PWR_T WtblPwrOffset = {0};
	UINT16 ucWlanIdx = 0;

	/* update power offset to wtbl*/
	WtblPwrOffset.u2Tag = WTBL_PWR_OFFSET;
	WtblPwrOffset.u2Length = sizeof(CMD_WTBL_PWR_T);
	if ((entry->tpcPwrAdj == -6)
		|| (entry->tpcPwrAdj == -3))
		WtblPwrOffset.ucPwrOffset = entry->tpcPwrAdj*2 + 64;
	else
		WtblPwrOffset.ucPwrOffset = 0;
	ucWlanIdx = entry->wcid;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_INFO, "wlanidx[%d], PwrOffset[%d]\n",
			ucWlanIdx, WtblPwrOffset.ucPwrOffset);
	CmdExtWtblUpdate(pAd, ucWlanIdx, SET_WTBL, &WtblPwrOffset, sizeof(CMD_WTBL_PWR_T));
	return;
}
INT Set_TpcCtrlMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 CtrlMode;
	struct wifi_dev *wdev = NULL;
	int wcid = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;

	CtrlMode = os_str_tol(arg, 0, 10);
	if ((CtrlMode > TPC_MODE_2)
		|| (CtrlMode < TPC_MODE_0)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "[0 < CtrlMode <= 2]error[%d]\n", CtrlMode);
		return FALSE;
	}
	pAd->CommonCfg.ctrlTPC.u1CtrlMode = CtrlMode;
	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = &pAd->MacTab.Content[wcid];
		if (!pEntry)
			continue;
		if (IS_ENTRY_NONE(pEntry) || IS_ENTRY_MCAST(pEntry))
			continue;
		if (pEntry->Sst != SST_ASSOC)
			continue;
		wdev = pEntry->wdev;
		if (!wdev)
			continue;
		if (pAd->CommonCfg.ctrlTPC.u1CtrlMode == TPC_MODE_2) {
			wdev->pwrCnstrnt = 0;
			pAd->CommonCfg.b80211TPC = 1;
		} else if (pAd->CommonCfg.ctrlTPC.u1CtrlMode == TPC_MODE_1) {
			pAd->CommonCfg.b80211TPC = 1;
			if (pEntry->tpcPwrAdj) {
				pEntry->tpcPwrAdj = 0;
				pEntry->tpcOldPwr = 0;
				set_wtbl_pwr_by_entry(pAd, pEntry);
			}
		} else {/*TPC mode 0*/
			wdev->pwrCnstrnt = 0;
			if (pEntry->tpcPwrAdj) {
				pEntry->tpcPwrAdj = 0;
				pEntry->tpcOldPwr = 0;
				set_wtbl_pwr_by_entry(pAd, pEntry);
			}
			pAd->CommonCfg.b80211TPC = 0;
		}
		if (wdev->LastpwrCnst != wdev->pwrCnstrnt) {
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
			wdev->LastpwrCnst = 0;
		}
	}
	MTWF_PRINT("u1CtrlMode=%d\n", pAd->CommonCfg.ctrlTPC.u1CtrlMode);
	return TRUE;
}
INT Set_TpcInterval_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 interval;

	interval = os_str_tol(arg, 0, 10);
	if (interval < 10) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Suggest interval bigger than 1s\n");
		return FALSE;
	}
	pAd->CommonCfg.ctrlTPC.sTpcIntval = interval;
	MTWF_PRINT("sTpcIntval=%u\n", pAd->CommonCfg.ctrlTPC.sTpcIntval);
	return TRUE;
}

INT Set_tpc_RssiThld_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type;
	CHAR rssithld = -127;

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;
	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid MBSS index %d\n", ifIndex);
			return FALSE;
		}
	}
	rssithld = (CHAR)os_str_tol(arg, 0, 10);
	if ((rssithld < -127)
		|| (rssithld > 0)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid rssithld=%d\n", rssithld);
		return FALSE;
	}
	if (wdev) {
		wdev->tpcRssiThld = rssithld;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev\n");
		return FALSE;
	}
	return TRUE;
}
INT Set_TpcLinkMargin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 link_margin;

	link_margin = os_str_tol(arg, 0, 10);
	if ((link_margin < -128) || (link_margin > 127)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid link_margin\n");
		return;
	}
	pAd->CommonCfg.ctrlTPC.linkmargin = link_margin;
	MTWF_PRINT("link_margin=%d\n", pAd->CommonCfg.ctrlTPC.linkmargin);
	return TRUE;
}

INT Show_TPC_Info_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type;
	int wcid = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;
	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid MBSS index %d\n", ifIndex);
	}
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev null\n");
		return FALSE;
	}
	MTWF_PRINT("---u1CtrlMode=%d----------------\n", pAd->CommonCfg.ctrlTPC.u1CtrlMode);
	MTWF_PRINT("---sTpcIntval=%-5d------------\n", pAd->CommonCfg.ctrlTPC.sTpcIntval);
	MTWF_PRINT("---linkmargin %s Mode--------\n",
		(pAd->CommonCfg.ctrlTPC.linkmargin == LINK_MARGIN_AUTO_MODE)?"AUTO":"MANU");
	MTWF_PRINT("---Pwrofdm6M =%-5d------------\n", pAd->CommonCfg.ctrlTPC.pwr_ofdm6m);
	MTWF_PRINT("---Pwrmcs11  =%-5d------------\n", pAd->CommonCfg.ctrlTPC.pwr_mcs11);
	if (wdev->if_dev)
		MTWF_PRINT("------------%-6s-------------\n", RtmpOsGetNetDevName(wdev->if_dev));
	MTWF_PRINT("---ifAddr[%02x:%02x:%02x:%02x:%02x:%02x]---\n", PRINT_MAC(wdev->if_addr));
	MTWF_PRINT("---tpcRssiThld=%-4d------------\n", wdev->tpcRssiThld);
	MTWF_PRINT("---PwrCnStrnt=%-4d-------------\n", wdev->pwrCnstrnt);
	MTWF_PRINT("---BSSAllStaSptTPC:%-3s---------\n", wdev->isStaNotSprtTpc ? "NOP":"YES");
	MTWF_PRINT("---MinLinkMargin=%-4d----------\n", wdev->MinLinkMargin);
	MTWF_PRINT("---mLKMGAddr[%02x:%02x:%02x:%02x:%02x:%02x]\n", PRINT_MAC(wdev->mLkMgnAddr));
	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = &pAd->MacTab.Content[wcid];
		if (!pEntry)
			continue;
		if (IS_ENTRY_NONE(pEntry) || IS_ENTRY_MCAST(pEntry))
			continue;
		if (pEntry->Sst != SST_ASSOC)
			continue;
		MTWF_PRINT("-------------------------------\n");
		MTWF_PRINT("---STA[%02x:%02x:%02x:%02x:%02x:%02x]------\n", PRINT_MAC(pEntry->Addr));
		MTWF_PRINT("---CurPwrDrop=%-2d,LastPwrDrop=%-2d\n", pEntry->tpcPwrAdj, pEntry->tpcOldPwr);
		MTWF_PRINT("---SupportTPC:%s\n", (pEntry->CapabilityInfo & 0x0100)?"YES":"NOP");
	}
	return TRUE;
}
#endif
#endif
