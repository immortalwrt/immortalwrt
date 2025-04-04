/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) [2020]  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#if defined(COMPOS_WIN)
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Ra_ctrl_mt.tmh"
#endif
#else
#include "rt_config.h"
#endif

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#ifdef MT_MAC
/*----------------------------------------------------------------------------*/
/*!
* \brief     Initialize Rate Adaptation for this entry
*
* \param[in] pAd
* \param[in] pEntry
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
RAInit(
	PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pEntry
)
{
	P_RA_ENTRY_INFO_T pRaEntry;
	RA_COMMON_INFO_T RaCfg;
	P_RA_INTERNAL_INFO_T pRaInternal;
	P_RA_COMMON_INFO_T pRaCfg;
	struct _RTMP_CHIP_CAP *cap;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	pRaEntry = &pEntry->RaEntry;
	pRaInternal = &pEntry->RaInternal;
	pRaCfg = &RaCfg;
	os_zero_mem(pRaEntry, sizeof(RA_ENTRY_INFO_T));
	os_zero_mem(pRaInternal, sizeof(RA_INTERNAL_INFO_T));
	os_zero_mem(&RaCfg, sizeof(RaCfg));
	raWrapperEntrySet(pAd, pEntry, pRaEntry);
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	eaprawrapperentryset(pAd, pEntry, pRaEntry);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	raWrapperConfigSet(pAd, pEntry->wdev, pRaCfg);
	pRaInternal->ucLastRateIdx = 0xFF;
	pRaInternal->ucLowTrafficCount = 0;
	pRaInternal->fgLastSecAccordingRSSI = FALSE;
	pRaInternal->ucInitialRateMode =  RA_INIT_RATE_BY_RSSI;
	pRaInternal->ucLastSecTxRateChangeAction = RATE_NO_CHANGE;
	pRaInternal->ucCurrTxRateIndex = 0;
	pRaInternal->ucTxRateUpPenalty = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	if (cap->fgRateAdaptFWOffload == TRUE) {
		pRaEntry->fgRaValid = TRUE;
		WifiSysRaInit(pAd, pEntry);
	}

#else

	if (pRaEntry->fgAutoTxRateSwitch == TRUE) {
		UCHAR TableSize = 0;
#ifdef NEW_RATE_ADAPT_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_GRP) {
			raSetMcsGroup(pRaEntry, pRaCfg, pRaInternal);
			raClearTxQuality(pRaInternal);
			raSelectTxRateTable(pRaEntry, pRaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &pRaInternal->ucCurrTxRateIndex);
			NewTxRateMtCore(pAd, pRaEntry, pRaCfg, pRaInternal);
		}

#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef RATE_ADAPT_AGBS_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_AGBS) {
			raSetMcsGroupAGBS(pRaEntry, pRaCfg, pRaInternal);
			raClearTxQualityAGBS(pRaInternal);
			raSelectTxRateTable(pRaEntry, pRaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &pRaInternal->ucCurrTxRateIndex);
			SetTxRateMtCoreAGBS(pAd, pRaEntry, pRaCfg, pRaInternal);
		}

#endif /* RATE_ADAPT_AGBS_SUPPORT */
	} else {
#ifdef MCS_LUT_SUPPORT
#ifdef NEW_RATE_ADAPT_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_GRP)
			MtAsicMcsLutUpdateCore(pAd, pRaEntry, pRaCfg, pRaInternal);

#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef RATE_ADAPT_AGBS_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_AGBS)
			MtAsicMcsLutUpdateCoreAGBS(pAd, pRaEntry, pRaCfg, pRaInternal);

#endif /* RATE_ADAPT_AGBS_SUPPORT */
#endif /* MCS_LUT_SUPPORT */
	}

#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#ifndef COMPOS_WIN
	RA_SAVE_LAST_TX_CFG(pRaEntry);
#endif /* !COMPOS_WIN */
	raWrapperEntryRestore(pAd, pEntry, pRaEntry);
}


/*----------------------------------------------------------------------------*/
/*!
* \brief     Update RA paramater
*
* \param[in] pAd
* \param[in] pEntry
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
RAParamUpdate(
	PRTMP_ADAPTER pAd,
	PMAC_TABLE_ENTRY pEntry,
	P_CMD_STAREC_AUTO_RATE_UPDATE_T prParam
)
{
	P_RA_ENTRY_INFO_T pRaEntry;
	struct _RTMP_CHIP_CAP *cap;
	/* pRaEntry is used in AsicStaRecUpdate() */
	pRaEntry = &pEntry->RaEntry;

	if (pRaEntry->fgRaValid != TRUE)
		return;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	raWrapperEntrySet(pAd, pEntry, pRaEntry);
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	eaprawrapperentryset(pAd, pEntry, pRaEntry);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	if (cap->fgRateAdaptFWOffload == TRUE)
		WifiSysUpdateRa(pAd, pEntry, prParam);

#else
	{
		RA_COMMON_INFO_T RaCfg;
		P_RA_INTERNAL_INFO_T pRaInternal;
		P_RA_COMMON_INFO_T pRaCfg;

		pRaInternal = &pEntry->RaInternal;
		pRaCfg = &RaCfg;
		os_zero_mem(pRaInternal, sizeof(RA_INTERNAL_INFO_T));
		os_zero_mem(&RaCfg, sizeof(RaCfg));
		raWrapperConfigSet(pAd, pEntry->wdev, pRaCfg);
#ifdef MT_MAC
#ifdef RATE_ADAPT_AGBS_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_AGBS) {
			if (prParam->u4Field == RA_PARAM_VHT_OPERATING_MODE) {
				UCHAR TableSize;

				raSetMcsGroupAGBS(pRaEntry, pRaCfg, pRaInternal);
				raSelectTxRateTable(pRaEntry, pRaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &pRaInternal->ucCurrTxRateIndex);
				SetTxRateMtCoreAGBS(pAd, pRaEntry, pRaCfg, pRaInternal);
			}
		}

#endif /* RATE_ADAPT_AGBS_SUPPORT */
#endif /* MT_MAC */
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
}


#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
/*----------------------------------------------------------------------------*/
/*!
* \brief     set RA Common config to FW ( Driver API)
*
* \param[in] pRaCfg
* \param[out] pCmdStaRecAutoRateCfg
*
* \return    NDIS_STATUS_SUCCESS
*
*/
/*----------------------------------------------------------------------------*/
INT32
BssInfoRACommCfgSet(
	IN P_RA_COMMON_INFO_T pRaCfg,
	OUT P_CMD_BSSINFO_AUTO_RATE_CFG_T pCmdBssInfoAutoRateCfg
)
{
	/* Fill TLV format */
	pCmdBssInfoAutoRateCfg->u2Tag = BSS_INFO_RA;
	pCmdBssInfoAutoRateCfg->u2Length = sizeof(CMD_BSSINFO_AUTO_RATE_CFG_T);
#ifdef RT_BIG_ENDIAN
	pCmdBssInfoAutoRateCfg->u2Tag = cpu2le16(pCmdBssInfoAutoRateCfg->u2Tag);
	pCmdBssInfoAutoRateCfg->u2Length = cpu2le16(pCmdBssInfoAutoRateCfg->u2Length);
#endif
	pCmdBssInfoAutoRateCfg->OpMode = pRaCfg->OpMode;
	pCmdBssInfoAutoRateCfg->fgAdHocOn = pRaCfg->fgAdHocOn;
	pCmdBssInfoAutoRateCfg->fgShortPreamble = pRaCfg->fgShortPreamble;
	pCmdBssInfoAutoRateCfg->TxStream = pRaCfg->TxStream;
	pCmdBssInfoAutoRateCfg->RxStream = pRaCfg->RxStream;
	pCmdBssInfoAutoRateCfg->ucRateAlg = pRaCfg->ucRateAlg;
	pCmdBssInfoAutoRateCfg->TestbedForceShortGI = pRaCfg->TestbedForceShortGI;
	pCmdBssInfoAutoRateCfg->TestbedForceGreenField = pRaCfg->TestbedForceGreenField;
#ifdef DOT11_N_SUPPORT
	pCmdBssInfoAutoRateCfg->HtMode = pRaCfg->HtMode;
	pCmdBssInfoAutoRateCfg->fAnyStation20Only = pRaCfg->fAnyStation20Only;
	pCmdBssInfoAutoRateCfg->bRcvBSSWidthTriggerEvents = pRaCfg->bRcvBSSWidthTriggerEvents;
#ifdef DOT11_VHT_AC
	pCmdBssInfoAutoRateCfg->vht_nss_cap = pRaCfg->vht_nss_cap;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pCmdBssInfoAutoRateCfg->fgSeOff = pRaCfg->fgSeOff;
	pCmdBssInfoAutoRateCfg->ucAntennaIndex = pRaCfg->ucAntennaIndex;
	pCmdBssInfoAutoRateCfg->TrainUpRule = pRaCfg->TrainUpRule;
	pCmdBssInfoAutoRateCfg->TrainUpHighThrd = cpu2le16(pRaCfg->TrainUpHighThrd);
	pCmdBssInfoAutoRateCfg->TrainUpRuleRSSI = cpu2le16(pRaCfg->TrainUpRuleRSSI);
	pCmdBssInfoAutoRateCfg->lowTrafficThrd = cpu2le16(pRaCfg->lowTrafficThrd);
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
	pCmdBssInfoAutoRateCfg->u2MaxPhyRate = cpu2le16(pRaCfg->u2MaxPhyRate);
#endif
	pCmdBssInfoAutoRateCfg->PhyCaps = cpu2le32(pRaCfg->PhyCaps);
	pCmdBssInfoAutoRateCfg->u4RaInterval = cpu2le32(pRaCfg->u4RaInterval);
	pCmdBssInfoAutoRateCfg->u4RaFastInterval = cpu2le32(pRaCfg->u4RaFastInterval);

#ifdef CFG_RATE_ADJUST_PARAM_DEBUG
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: sizeof CMD_BSSINFO_AUTO_RATE_CFG_T = %d\n", __func__, (UINT_32)sizeof(CMD_BSSINFO_AUTO_RATE_CFG_T));
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OpMode=%d\n", pCmdBssInfoAutoRateCfg->OpMode);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgAdHocOn=%d\n", pCmdBssInfoAutoRateCfg->fgAdHocOn);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgShortPreamble=%d\n", pCmdBssInfoAutoRateCfg->fgShortPreamble);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TxStream=%d\n", pCmdBssInfoAutoRateCfg->TxStream);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RxStream=%d\n", pCmdBssInfoAutoRateCfg->RxStream);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucRateAlg=%d\n", pCmdBssInfoAutoRateCfg->ucRateAlg);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TestbedForceShortGI=%d\n", pCmdBssInfoAutoRateCfg->TestbedForceShortGI);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TestbedForceGreenField=%d\n", pCmdBssInfoAutoRateCfg->TestbedForceGreenField);
#ifdef DOT11_N_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HtMode=%d\n", pCmdBssInfoAutoRateCfg->HtMode);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fAnyStation20Only=%d\n", pCmdBssInfoAutoRateCfg->fAnyStation20Only);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "bRcvBSSWidthTriggerEvents=%d\n", pCmdBssInfoAutoRateCfg->bRcvBSSWidthTriggerEvents);
#ifdef DOT11_VHT_AC
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "vht_nss_cap=%d\n", pCmdBssInfoAutoRateCfg->vht_nss_cap);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgSeOff=%d\n", pCmdBssInfoAutoRateCfg->fgSeOff);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucAntennaIndex=%d\n", pCmdBssInfoAutoRateCfg->ucAntennaIndex);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TrainUpRule=%d\n", pCmdBssInfoAutoRateCfg->TrainUpRule);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TrainUpHighThrd=%d\n", pCmdBssInfoAutoRateCfg->TrainUpHighThrd);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TrainUpRuleRSSI=%d\n", pCmdBssInfoAutoRateCfg->TrainUpRuleRSSI);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "lowTrafficThrd=%d\n", pCmdBssInfoAutoRateCfg->lowTrafficThrd);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "u2MaxPhyRate=%d\n", pCmdBssInfoAutoRateCfg->u2MaxPhyRate);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PhyCaps=%d\n", pCmdBssInfoAutoRateCfg->PhyCaps);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "u4RaInterval=%d\n", pCmdBssInfoAutoRateCfg->u4RaInterval);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "u4RaFastInterval=%d\n", pCmdBssInfoAutoRateCfg->u4RaFastInterval);
#endif

	return NDIS_STATUS_SUCCESS;
}
#endif


#if defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622) || \
	defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626) || \
	defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
/*----------------------------------------------------------------------------*/
/*!
* \brief     Set rate entry info to FW ( Driver API)
*
* \param[in] pRaEntry
* \param[out] pCmdStaRecAutoRate
*
* \return    NDIS_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
INT32
StaRecAutoRateParamSet(
	IN P_RA_ENTRY_INFO_T pRaEntry,
	OUT P_CMD_STAREC_AUTO_RATE_T pCmdStaRecAutoRate
)
{
#ifdef CFG_RATE_ADJUST_PARAM_DEBUG
	UINT_8  ucIdx;
#endif

	/* Fill TLV format */
	pCmdStaRecAutoRate->u2Tag = STA_REC_RA;
	pCmdStaRecAutoRate->u2Length = sizeof(CMD_STAREC_AUTO_RATE_T);
#ifdef RT_BIG_ENDIAN
	pCmdStaRecAutoRate->u2Tag = cpu2le16(pCmdStaRecAutoRate->u2Tag);
	pCmdStaRecAutoRate->u2Length = cpu2le16(pCmdStaRecAutoRate->u2Length);
#endif
	pCmdStaRecAutoRate->fgRaValid = pRaEntry->fgRaValid;
	pCmdStaRecAutoRate->fgAutoTxRateSwitch = pRaEntry->fgAutoTxRateSwitch;

	if (pCmdStaRecAutoRate->fgRaValid == FALSE)
		return NDIS_STATUS_SUCCESS;

	pCmdStaRecAutoRate->ucPhyMode = pRaEntry->ucPhyMode;
	pCmdStaRecAutoRate->ucChannel = pRaEntry->ucChannel;
	pCmdStaRecAutoRate->ucBBPCurrentBW = pRaEntry->ucBBPCurrentBW;
	pCmdStaRecAutoRate->fgDisableCCK = pRaEntry->fgDisableCCK;
	pCmdStaRecAutoRate->fgHtCapMcs32 = pRaEntry->fgHtCapMcs32;
	pCmdStaRecAutoRate->fgHtCapInfoGF = pRaEntry->fgHtCapInfoGF;
	os_move_mem(pCmdStaRecAutoRate->aucHtCapMCSSet, pRaEntry->aucHtCapMCSSet, sizeof(pCmdStaRecAutoRate->aucHtCapMCSSet));
	pCmdStaRecAutoRate->ucMmpsMode = pRaEntry->ucMmpsMode;
	pCmdStaRecAutoRate->ucGband256QAMSupport = pRaEntry->ucGband256QAMSupport;
	pCmdStaRecAutoRate->ucMaxAmpduFactor = pRaEntry->ucMaxAmpduFactor;
	pCmdStaRecAutoRate->fgAuthWapiMode = pRaEntry->fgAuthWapiMode;
	pCmdStaRecAutoRate->RateLen = pRaEntry->RateLen;
	pCmdStaRecAutoRate->ucSupportRateMode = pRaEntry->ucSupportRateMode;
	pCmdStaRecAutoRate->ucSupportCCKMCS = pRaEntry->ucSupportCCKMCS;
	pCmdStaRecAutoRate->ucSupportOFDMMCS = pRaEntry->ucSupportOFDMMCS;
#ifdef DOT11_N_SUPPORT
	pCmdStaRecAutoRate->u4SupportHTMCS = cpu2le32(pRaEntry->u4SupportHTMCS);
#ifdef DOT11_VHT_AC
	pCmdStaRecAutoRate->u2SupportVHTMCS1SS = cpu2le16(pRaEntry->u2SupportVHTMCS1SS);
	pCmdStaRecAutoRate->u2SupportVHTMCS2SS = cpu2le16(pRaEntry->u2SupportVHTMCS2SS);
	pCmdStaRecAutoRate->u2SupportVHTMCS3SS = cpu2le16(pRaEntry->u2SupportVHTMCS3SS);
	pCmdStaRecAutoRate->u2SupportVHTMCS4SS = cpu2le16(pRaEntry->u2SupportVHTMCS4SS);
	pCmdStaRecAutoRate->force_op_mode = pRaEntry->force_op_mode;
	pCmdStaRecAutoRate->vhtOpModeChWidth = pRaEntry->vhtOpModeChWidth;
	pCmdStaRecAutoRate->vhtOpModeRxNss = pRaEntry->vhtOpModeRxNss;
	pCmdStaRecAutoRate->vhtOpModeRxNssType = pRaEntry->vhtOpModeRxNssType;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pCmdStaRecAutoRate->ClientStatusFlags = cpu2le32(pRaEntry->ClientStatusFlags);
	os_move_mem(&pCmdStaRecAutoRate->MaxPhyCfg, &pRaEntry->MaxPhyCfg, sizeof(RA_PHY_CFG_T));

	if (pCmdStaRecAutoRate->fgAutoTxRateSwitch == FALSE) {
		if (pRaEntry->MaxPhyCfg.ShortGI)
			pCmdStaRecAutoRate->MaxPhyCfg.ShortGI = SGI_20 + SGI_40 + SGI_80 + SGI_160;

		if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE))
			pCmdStaRecAutoRate->MaxPhyCfg.ldpc |= VHT_LDPC;
		if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE))
			pCmdStaRecAutoRate->MaxPhyCfg.ldpc |= HT_LDPC;

		if ((pRaEntry->MaxPhyCfg.MODE >= MODE_VHT) &&
				CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE))
			pCmdStaRecAutoRate->MaxPhyCfg.STBC = STBC_USE;
	}

#ifdef CFG_RATE_ADJUST_PARAM_DEBUG
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: sizeof CMD_STAREC_AUTO_RATE_T = %d\n", __func__, (UINT_32)sizeof(CMD_STAREC_AUTO_RATE_T));
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgRaValid=%d\n", pCmdStaRecAutoRate->fgRaValid);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgAutoTxRateSwitch=%d\n", pCmdStaRecAutoRate->fgAutoTxRateSwitch);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucPhyMode=%d\n", pCmdStaRecAutoRate->ucPhyMode);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucChannel=%d\n", pCmdStaRecAutoRate->ucChannel);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucBBPCurrentBW=%d\n", pCmdStaRecAutoRate->ucBBPCurrentBW);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgDisableCCK=%d\n", pCmdStaRecAutoRate->fgDisableCCK);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgHtCapMcs32=%d\n", pCmdStaRecAutoRate->fgHtCapMcs32);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgHtCapInfoGF=%d\n", pCmdStaRecAutoRate->fgHtCapInfoGF);

	for (ucIdx = 0; ucIdx < 4; ucIdx++)
		MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "aucHtCapMCSSet[%d]=%d\n", ucIdx, pCmdStaRecAutoRate->aucHtCapMCSSet[ucIdx]);

	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucMmpsMode=%d\n", pCmdStaRecAutoRate->ucMmpsMode);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucGband256QAMSupport=%d\n", pCmdStaRecAutoRate->ucGband256QAMSupport);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucMaxAmpduFactor=%d\n", pCmdStaRecAutoRate->ucMaxAmpduFactor);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgAuthWapiMode=%d\n", pCmdStaRecAutoRate->fgAuthWapiMode);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RateLen=%d\n", pCmdStaRecAutoRate->RateLen);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucSupportRateMode=%x\n", pCmdStaRecAutoRate->ucSupportRateMode);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucSupportCCKMCS=%x\n", pCmdStaRecAutoRate->ucSupportCCKMCS);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucSupportOFDMMCS=%x\n", pCmdStaRecAutoRate->ucSupportOFDMMCS);
#ifdef DOT11_N_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "u4SupportHTMCS=%x\n", pCmdStaRecAutoRate->u4SupportHTMCS);
#ifdef DOT11_VHT_AC
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "u2SupportVHTMCS1SS=%x\n", pCmdStaRecAutoRate->u2SupportVHTMCS1SS);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "u2SupportVHTMCS2SS=%x\n", pCmdStaRecAutoRate->u2SupportVHTMCS2SS);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "u2SupportVHTMCS3SS=%x\n", pCmdStaRecAutoRate->u2SupportVHTMCS3SS);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "u2SupportVHTMCS4SS=%x\n", pCmdStaRecAutoRate->u2SupportVHTMCS4SS);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "force_op_mode=%d\n", pCmdStaRecAutoRate->force_op_mode);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "vhtOpModeChWidth=%d\n", pCmdStaRecAutoRate->vhtOpModeChWidth);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "vhtOpModeRxNss=%d\n", pCmdStaRecAutoRate->vhtOpModeRxNss);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "vhtOpModeRxNssType=%x\n", pCmdStaRecAutoRate->vhtOpModeRxNssType);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ClientStatusFlags=%x\n", pCmdStaRecAutoRate->ClientStatusFlags);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MaxPhyCfg.MODE=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.MODE);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MaxPhyCfg.Flags=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.Flags);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MaxPhyCfg.STBC=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.STBC);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MaxPhyCfg.ShortGI=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.ShortGI);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MaxPhyCfg.BW=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.BW);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MaxPhyCfg.ldpc=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.ldpc);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MaxPhyCfg.MCS=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.MCS);
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MaxPhyCfg.VhtNss=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.VhtNss);
#endif
	return NDIS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief     set RA data partial update to FW ( Driver API)
*
* \param[in] pRaEntry
* \param[in] pRaInternal
* \param[in] u4Field
* \param[out] pCmdStaRecAutoRateUpdate
*
* \return    NDIS_STATUS_SUCCESS
*
*/
/*----------------------------------------------------------------------------*/
INT32
StaRecAutoRateUpdate(
	IN P_RA_ENTRY_INFO_T pRaEntry,
	IN P_RA_INTERNAL_INFO_T pRaInternal,
	IN P_CMD_STAREC_AUTO_RATE_UPDATE_T pRaParam,
	OUT P_CMD_STAREC_AUTO_RATE_UPDATE_T pCmdStaRecAutoRateUpdate
)
{
	/* Fill TLV format */
	pCmdStaRecAutoRateUpdate->u2Tag = STA_REC_RA_UPDATE;
	pCmdStaRecAutoRateUpdate->u2Length = sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

	if (pRaParam->u4Field == RA_PARAM_VHT_OPERATING_MODE) {
		pCmdStaRecAutoRateUpdate->force_op_mode = pRaEntry->force_op_mode;
		pCmdStaRecAutoRateUpdate->vhtOpModeChWidth = pRaEntry->vhtOpModeChWidth;
		pCmdStaRecAutoRateUpdate->vhtOpModeRxNss = pRaEntry->vhtOpModeRxNss;
		pCmdStaRecAutoRateUpdate->vhtOpModeRxNssType = pRaEntry->vhtOpModeRxNssType;
		pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_VHT_OPERATING_MODE;
	} else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		if (pRaParam->u4Field == RA_PARAM_HT_2040_COEX)
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_HT_2040_COEX;
		else if (pRaParam->u4Field == RA_PARAM_HT_2040_BACK)
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_HT_2040_BACK;
		else if (pRaParam->u4Field == RA_PARAM_MMPS_UPDATE) {
			pCmdStaRecAutoRateUpdate->ucMmpsMode = pRaEntry->ucMmpsMode;
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_MMPS_UPDATE;
		} else if (pRaParam->u4Field == RA_PARAM_FIXED_RATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_FIXED_RATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MODE = pRaParam->FixedRateCfg.MODE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.STBC = pRaParam->FixedRateCfg.STBC;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ShortGI = pRaParam->FixedRateCfg.ShortGI;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.BW = pRaParam->FixedRateCfg.BW;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ldpc = pRaParam->FixedRateCfg.ldpc;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.he_ltf = pRaParam->FixedRateCfg.he_ltf;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MCS = pRaParam->FixedRateCfg.MCS;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.VhtNss = pRaParam->FixedRateCfg.VhtNss;
			pCmdStaRecAutoRateUpdate->ucShortPreamble = pRaParam->ucShortPreamble;
			pCmdStaRecAutoRateUpdate->ucSpeEn = pRaParam->ucSpeEn;
		} else if (pRaParam->u4Field == RA_PARAM_FIXED_RATE_FALLBACK) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_FIXED_RATE_FALLBACK;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MODE = pRaParam->FixedRateCfg.MODE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.STBC = pRaParam->FixedRateCfg.STBC;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ShortGI = pRaParam->FixedRateCfg.ShortGI;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.BW = pRaParam->FixedRateCfg.BW;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ldpc = pRaParam->FixedRateCfg.ldpc;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MCS = pRaParam->FixedRateCfg.MCS;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.VhtNss = pRaParam->FixedRateCfg.VhtNss;
			pCmdStaRecAutoRateUpdate->ucShortPreamble = pRaParam->ucShortPreamble;
			pCmdStaRecAutoRateUpdate->ucSpeEn = pRaParam->ucSpeEn;
			pCmdStaRecAutoRateUpdate->fgIs5G = pRaParam->fgIs5G;
		} else if (pRaParam->u4Field == RA_PARAM_HELTF_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_HELTF_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.he_ltf = pRaParam->FixedRateCfg.he_ltf;
		} else if (pRaParam->u4Field == RA_PARAM_MCS_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_MCS_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MCS = pRaParam->FixedRateCfg.MCS;
		} else if (pRaParam->u4Field == RA_PARAM_VHTNSS_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_VHTNSS_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.VhtNss = pRaParam->FixedRateCfg.VhtNss;
		} else if (pRaParam->u4Field == RA_PARAM_BW_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_BW_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.BW = pRaParam->FixedRateCfg.BW;
		} else if (pRaParam->u4Field == RA_PARAM_GI_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_GI_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ShortGI = pRaParam->FixedRateCfg.ShortGI;
		} else if (pRaParam->u4Field == RA_PARAM_ECC_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_ECC_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ldpc = pRaParam->FixedRateCfg.ldpc;
		} else if (pRaParam->u4Field == RA_PARAM_STBC_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_STBC_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.STBC = pRaParam->FixedRateCfg.STBC;
		} else if (pRaParam->u4Field == RA_PARAM_UL_HELTF_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_UL_HELTF_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.he_ltf = pRaParam->FixedRateCfg.he_ltf;
		} else if (pRaParam->u4Field == RA_PARAM_UL_MCS_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_UL_MCS_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MCS = pRaParam->FixedRateCfg.MCS;
		} else if (pRaParam->u4Field == RA_PARAM_UL_VHTNSS_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_UL_VHTNSS_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.VhtNss = pRaParam->FixedRateCfg.VhtNss;
		} else if (pRaParam->u4Field == RA_PARAM_UL_GI_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_UL_GI_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ShortGI = pRaParam->FixedRateCfg.ShortGI;
		} else if (pRaParam->u4Field == RA_PARAM_UL_ECC_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_UL_ECC_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ldpc = pRaParam->FixedRateCfg.ldpc;
		} else if (pRaParam->u4Field == RA_PARAM_UL_STBC_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_UL_STBC_UPDATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.STBC = pRaParam->FixedRateCfg.STBC;
		} else if (pRaParam->u4Field == RA_PARAM_AUTO_RATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_AUTO_RATE;
		} else if (pRaParam->u4Field == RA_PARAM_UL_AUTO_RATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_UL_AUTO_RATE;
		} else if (pRaParam->u4Field == RA_PARAM_SPE_UPDATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_SPE_UPDATE;
			pCmdStaRecAutoRateUpdate->ucSpeEn = pRaParam->ucSpeEn;
		} else if (pRaParam->u4Field > RA_PARAM_MAX) {
			pCmdStaRecAutoRateUpdate->u4Field = pRaParam->u4Field;
		}

#ifdef RT_BIG_ENDIAN
	pCmdStaRecAutoRateUpdate->u2Tag = cpu2le16(pCmdStaRecAutoRateUpdate->u2Tag);
	pCmdStaRecAutoRateUpdate->u2Length = cpu2le16(pCmdStaRecAutoRateUpdate->u2Length);
	pCmdStaRecAutoRateUpdate->u4Field = cpu2le32(pCmdStaRecAutoRateUpdate->u4Field);
#endif
	return NDIS_STATUS_SUCCESS;
}
#endif



INT32 CmdRaFixRateUpdateWoSta(
	RTMP_ADAPTER *pAd,
	UINT16 u2Wcid,
	RA_PHY_CFG_T *pFixedRateCfg,
	UINT8 u1SpeEn,
	UINT8 u1ShortPreamble)
{
	INT32 ret = 0;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	CMD_FIX_RATE_WO_STA_UPDATE_T param;

	os_move_mem(&param.FixedRateCfg, pFixedRateCfg, sizeof(*pFixedRateCfg));
	param.u1ShortPreamble = u1ShortPreamble;
	param.u1SpeEn = u1SpeEn;
	WCID_SET_H_L(param.u1WcidL, param.u1WcidHnVer, u2Wcid);

	msg = AndesAllocCmdMsg(pAd, sizeof(param));
	if (!msg) {
		ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_FIX_RATE_WO_STA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));

	ret = chip_cmd_tx(pAd, msg);
error:

	MTWF_PRINT("%s:(ret = %d)\n", __func__, ret);

	return ret;
}

#endif /* MT_MAC */

