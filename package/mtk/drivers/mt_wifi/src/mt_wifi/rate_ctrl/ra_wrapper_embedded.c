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
#include "rt_config.h"
#include "hdev/hdev.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
typedef enum _HERA_PROC_ACTION {
	HERA_PROC_ACTION_STBC_PRIORITY = 0,
	HERA_PROC_ACTION_MAX
} HERA_PROC_ACTION;

typedef enum _HERA_STBC_PRIORITY_OPERATION {
	HERA_STBC_PRIORITY_OP_GET = 0,
	HERA_STBC_PRIORITY_OP_SET
} HERA_STBC_PRIORITY_OPERATION;

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

VOID
HeraConfigStbcPriority(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 u1BandIdx,
	IN UINT8 u1Operation,
	IN UINT8 u1StbcPriority
)
{
	CMD_HERA_STBC_PRIORITY_T rHeraStbcPriority;

	rHeraStbcPriority.u1BandIdx = u1BandIdx;
	rHeraStbcPriority.u1Operation = u1Operation;
	rHeraStbcPriority.u1StbcPriority = u1StbcPriority;

	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"u1BandIdx = %u, u1Operation=%u, u1StbcPriority = %u\n",
					rHeraStbcPriority.u1BandIdx,
					rHeraStbcPriority.u1Operation,
					rHeraStbcPriority.u1StbcPriority);

	AsicHeraStbcPriorityCtrl(pAd, (PUINT8)&rHeraStbcPriority);
}


VOID
HeraInitStbcPriority(
	IN PRTMP_ADAPTER pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	RADIO_CTRL *pRadioCtrl = NULL;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	UINT8 u1BandIdx, u1StbcPriority, i;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		pPhyCtrl =  &pHwResource->PhyCtrl[i];
		pRadioCtrl =  &pPhyCtrl->RadioCtrl;

		u1BandIdx = pRadioCtrl->BandIdx;
		u1StbcPriority = pAd->CommonCfg.HeraStbcPriority[u1BandIdx];

		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"u1BandIdx = %u, u1StbcPriority = %u\n",
						u1BandIdx,
						u1StbcPriority);

		HeraConfigStbcPriority(pAd, u1BandIdx, HERA_STBC_PRIORITY_OP_SET, u1StbcPriority);
	}
}

INT Set_Hera_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8 *s1Value = NULL;
	UINT32 u4Input[4] = {0}, u4Action;
	UINT8 i, u1ArgCnt = 0;

	for (i = 0, s1Value = rstrtok(arg, "-"); s1Value && (i < ARRAY_SIZE(u4Input)); s1Value = rstrtok(NULL, "-"), i++) {
		u4Input[i] = os_str_toul(s1Value, 0, 10);
		u1ArgCnt++;
	}

	if (u1ArgCnt == 0)
		goto error;

	if (u4Input[0] >= HERA_PROC_ACTION_MAX)
		goto error;

	u4Action = u4Input[0];
	MTWF_PRINT("%s: u4Action = %u\n", __func__, u4Action);

	switch (u4Action) {
	case HERA_PROC_ACTION_STBC_PRIORITY:
		HeraConfigStbcPriority(pAd, (UINT8)u4Input[1], (UINT8)u4Input[2], (UINT8)u4Input[3]);
		break;

	default:
		break;
	}

	return TRUE;

error:
	MTWF_PRINT("Wrong Cmd Format. Plz input:\n");
	MTWF_PRINT("iwpriv ra0 set hera=[0]-[1]-[2]-[3]\n");
	MTWF_PRINT("[0]=0: STBC Priority Configuration:\n");
	MTWF_PRINT("	hera=0-[1]-[2]-[3]\n");
	MTWF_PRINT("	[1]: Band Index (0, 1)\n");
	MTWF_PRINT("	[2]: Operation (0: Get, 1: Set)\n");
	MTWF_PRINT("	[3]: STBC Priority\n");
	MTWF_PRINT("		(0: < IBF, 1: > IBF, 2: > EBF. Used when Operation=Set)\n");
	MTWF_PRINT("[0]=Others: TBD\n");
	return FALSE;
}
#ifdef CONFIG_6G_SUPPORT
static void ra_6ghe_entry_set(struct _MAC_TABLE_ENTRY *entry, struct _RA_ENTRY_INFO_T *ra_entry)
{
	struct _RA_PHY_CFG_T *tx_phy_cfg = &ra_entry->TxPhyCfg;
	struct _RA_PHY_CFG_T *max_phy_cfg = &ra_entry->MaxPhyCfg;
	struct caps_info *cap = &entry->cap;
	/*phy conifugration*/
	tx_phy_cfg->MODE = MODE_HE;
	tx_phy_cfg->STBC = 1;
	tx_phy_cfg->ShortGI = 1.;
	tx_phy_cfg->BW = ra_entry->ucBBPCurrentBW < cap->ch_bw.he6g_ch_width ?
					ra_entry->ucBBPCurrentBW : cap->ch_bw.he6g_ch_width;
	tx_phy_cfg->ldpc = cap->he_phy_cap & HE_LDPC ? 1 : 0;
	tx_phy_cfg->MCS = 9;

	max_phy_cfg->MODE = MODE_HE;
	max_phy_cfg->STBC = 1;
	max_phy_cfg->ShortGI = 1;
	max_phy_cfg->BW = ra_entry->ucBBPCurrentBW < cap->ch_bw.he6g_ch_width ?
					ra_entry->ucBBPCurrentBW : cap->ch_bw.he6g_ch_width;
	max_phy_cfg->ldpc = cap->he_phy_cap & HE_LDPC ? 1 : 0;
	max_phy_cfg->MCS = 9;
	/*update smps*/
	ra_entry->ucMmpsMode = entry->MmpsMode;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief     Set RaEntry by pEntry
 *
 * \param[in] pAd
 * \param[in] pEntry
 * \param[out] pRaEntry
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
raWrapperEntrySet(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	OUT P_RA_ENTRY_INFO_T pRaEntry
)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"wcid: %d -------------------------------------------------\n",
		pEntry->wcid);
	MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"sup_rate_mode: 0x%x, sup_cck_mcs: 0x%x, sup_ofdm_mcs: 0x%x\n",
		pEntry->SupportRateMode, pEntry->SupportCCKMCS, pEntry->SupportOFDMMCS);
#ifdef DOT11_N_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"sup_ht_mcs: 0x%x\n",
		pEntry->SupportHTMCS);
#ifdef DOT11_VHT_AC
	MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"sup_vht_mcs(1ss-2ss-3ss-4ss): 0x%x-0x%x-0x%x-0x%x\n",
		pEntry->SupportVHTMCS1SS, pEntry->SupportVHTMCS2SS,
		pEntry->SupportVHTMCS3SS, pEntry->SupportVHTMCS4SS);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"----------------------------------------------------------\n");

	pRaEntry->u2Wcid = pEntry->wcid;
	pRaEntry->fgAutoTxRateSwitch = pEntry->bAutoTxRateSwitch;

	pRaEntry->ucPhyMode = pEntry->wdev->PhyMode;
	pRaEntry->ucChannel = pEntry->wdev->channel;
	/* use the maximum bw capability */
	pRaEntry->ucBBPCurrentBW = HcGetBw(pAd, pEntry->wdev);
#ifdef P2P_SUPPORT

	if (P2P_GO_ON(pAd) || P2P_CLI_ON(pAd))
		pRaEntry->fgDisableCCK = TRUE;
	else
#endif /* P2P_SUPPORT */
	{
		pRaEntry->fgDisableCCK = FALSE;
	}

	pRaEntry->fgHtCapMcs32 = (pEntry->HTCapability.MCSSet[4] & 0x1) ? TRUE : FALSE;
	pRaEntry->fgHtCapInfoGF = pEntry->HTCapability.HtCapInfo.GF;
	pRaEntry->aucHtCapMCSSet[0] = pEntry->HTCapability.MCSSet[0];
	pRaEntry->aucHtCapMCSSet[1] = pEntry->HTCapability.MCSSet[1];
	pRaEntry->aucHtCapMCSSet[2] = pEntry->HTCapability.MCSSet[2];
	pRaEntry->aucHtCapMCSSet[3] = pEntry->HTCapability.MCSSet[3];
	pRaEntry->ucMmpsMode = pEntry->MmpsMode;

	if (pEntry->fgGband256QAMSupport == TRUE)
		pRaEntry->ucGband256QAMSupport = RA_G_BAND_256QAM_ENABLE;

#ifdef DOT11_VHT_AC
	else if (pAd->CommonCfg.g_band_256_qam == TRUE)
		pRaEntry->ucGband256QAMSupport = RA_G_BAND_256QAM_PROBING;

#endif /*DOT11_VHT_AC*/
	else
		pRaEntry->ucGband256QAMSupport = RA_G_BAND_256QAM_DISABLE;

	pRaEntry->ucMaxAmpduFactor = pEntry->MaxRAmpduFactor;
	pRaEntry->RateLen = pEntry->RateLen;
	pRaEntry->ucSupportRateMode = pEntry->SupportRateMode;
	pRaEntry->ucSupportCCKMCS = pEntry->SupportCCKMCS;
	pRaEntry->ucSupportOFDMMCS = pEntry->SupportOFDMMCS;
#ifdef DOT11_N_SUPPORT
	pRaEntry->u4SupportHTMCS = pEntry->SupportHTMCS;
#ifdef DOT11_VHT_AC
	pRaEntry->u2SupportVHTMCS1SS = pEntry->SupportVHTMCS1SS;
	pRaEntry->u2SupportVHTMCS2SS = pEntry->SupportVHTMCS2SS;
	if ((pEntry->MaxHTPhyMode.field.BW < BW_160) || IS_PHY_CAPS(pChipCap->phy_caps, fPHY_CAP_BW160C_STD)) {
		pRaEntry->u2SupportVHTMCS3SS = pEntry->SupportVHTMCS3SS;
		pRaEntry->u2SupportVHTMCS4SS = pEntry->SupportVHTMCS4SS;
	}
	pRaEntry->force_op_mode = pEntry->force_op_mode;

	if (pEntry->operating_mode.ch_width == 2)
		pRaEntry->vhtOpModeChWidth = (pEntry->operating_mode.ch_width + pEntry->operating_mode.bw160_bw8080);
	else
		pRaEntry->vhtOpModeChWidth = pEntry->operating_mode.ch_width;
	pRaEntry->vhtOpModeRxNss = pEntry->operating_mode.rx_nss;
	pRaEntry->vhtOpModeRxNssType = pEntry->operating_mode.rx_nss_type;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pRaEntry->AvgRssiSample[0] = pEntry->RssiSample.AvgRssi[0];
	pRaEntry->AvgRssiSample[1] = pEntry->RssiSample.AvgRssi[1];
	pRaEntry->AvgRssiSample[2] = pEntry->RssiSample.AvgRssi[2];
	{
		pRaEntry->fgAuthWapiMode = FALSE;
	}

	pRaEntry->ClientStatusFlags = pEntry->ClientStatusFlags;
	pRaEntry->MaxPhyCfg.MODE = pEntry->MaxHTPhyMode.field.MODE;
	pRaEntry->MaxPhyCfg.STBC = pEntry->MaxHTPhyMode.field.STBC;
	pRaEntry->MaxPhyCfg.ShortGI = pEntry->MaxHTPhyMode.field.ShortGI;
	pRaEntry->MaxPhyCfg.BW = pEntry->MaxHTPhyMode.field.BW;
	/* will not sync with VHT cap ? honor operating_mode first */
	pEntry->MaxHTPhyMode.field.ldpc = pEntry->operating_mode.no_ldpc ? 0 : 1;
	pRaEntry->MaxPhyCfg.ldpc = pEntry->MaxHTPhyMode.field.ldpc;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

	if (pRaEntry->MaxPhyCfg.MODE >= MODE_VHT) {
		pRaEntry->MaxPhyCfg.MCS = pEntry->MaxHTPhyMode.field.MCS & 0xf;
		pRaEntry->MaxPhyCfg.VhtNss = ((pEntry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
	} else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	{
		pRaEntry->MaxPhyCfg.MCS = pEntry->MaxHTPhyMode.field.MCS;
		pRaEntry->MaxPhyCfg.VhtNss = 0;
	}

	pRaEntry->TxPhyCfg.MODE = pEntry->HTPhyMode.field.MODE;
	pRaEntry->TxPhyCfg.STBC = pEntry->HTPhyMode.field.STBC;
	pRaEntry->TxPhyCfg.ShortGI = pEntry->HTPhyMode.field.ShortGI;
	pRaEntry->TxPhyCfg.BW = pEntry->HTPhyMode.field.BW;
	pRaEntry->TxPhyCfg.ldpc = pEntry->HTPhyMode.field.ldpc;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

	if (pRaEntry->TxPhyCfg.MODE >= MODE_VHT) {
		pRaEntry->TxPhyCfg.MCS = pEntry->HTPhyMode.field.MCS & 0xf;
		pRaEntry->TxPhyCfg.VhtNss = ((pEntry->HTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
	} else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	{
		pRaEntry->TxPhyCfg.MCS = pEntry->HTPhyMode.field.MCS;
		pRaEntry->TxPhyCfg.VhtNss = 0;
	}

#ifdef CONFIG_6G_SUPPORT
	if (IS_HE_6G_STA(pEntry->cap.modes))
		ra_6ghe_entry_set(pEntry, pRaEntry);
#endif
}

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
/*----------------------------------------------------------------------------*/
/*!
 * \brief     Set RaEntry by pEntry for eap setting
 *
 * \param[in] pAd
 * \param[in] pEntry
 * \param[out] pRaEntry
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
eaprawrapperentryset(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY	*entry,
	OUT RA_ENTRY_INFO_T *raentry
)
{
	if (entry->wdev->eap.eap_htsuprate_en == TRUE) {
		raentry->aucHtCapMCSSet[0] = entry->HTCapability.MCSSet[0] & entry->wdev->eap.eapmcsset[0];
		raentry->aucHtCapMCSSet[1] = entry->HTCapability.MCSSet[1] & entry->wdev->eap.eapmcsset[1];
		raentry->aucHtCapMCSSet[2] = entry->HTCapability.MCSSet[2] & entry->wdev->eap.eapmcsset[2];
		raentry->aucHtCapMCSSet[3] = entry->HTCapability.MCSSet[3] & entry->wdev->eap.eapmcsset[3];
	}

	if (entry->wdev->eap.eap_suprate_en == TRUE) {
		raentry->ucSupportCCKMCS = entry->SupportCCKMCS & entry->wdev->eap.eapsupportcckmcs;
		raentry->ucSupportOFDMMCS = entry->SupportOFDMMCS & entry->wdev->eap.eapsupportofdmmcs;
	}

#ifdef DOT11_N_SUPPORT
	if (entry->wdev->eap.eap_htsuprate_en == TRUE)
		raentry->u4SupportHTMCS = entry->SupportHTMCS & entry->wdev->eap.eapsupporthtmcs;

#ifdef DOT11_VHT_AC
	if (entry->wdev->eap.eap_vhtsuprate_en == TRUE) {
		if (entry->wdev->eap.rx_mcs_map.mcs_ss1 == 0)
			raentry->u2SupportVHTMCS1SS = entry->SupportVHTMCS1SS & 255;

		if (entry->wdev->eap.rx_mcs_map.mcs_ss1 == 1)
			raentry->u2SupportVHTMCS1SS = entry->SupportVHTMCS1SS & 511;

		if (entry->wdev->eap.rx_mcs_map.mcs_ss1 == 2)
			raentry->u2SupportVHTMCS1SS = entry->SupportVHTMCS1SS & 1023;

		if (entry->wdev->eap.rx_mcs_map.mcs_ss2 == 0)
			raentry->u2SupportVHTMCS2SS = entry->SupportVHTMCS2SS & 255;

		if (entry->wdev->eap.rx_mcs_map.mcs_ss2 == 1)
			raentry->u2SupportVHTMCS2SS = entry->SupportVHTMCS2SS & 511;

		if (entry->wdev->eap.rx_mcs_map.mcs_ss2 == 2)
			raentry->u2SupportVHTMCS2SS = entry->SupportVHTMCS2SS & 1023;

		if (entry->MaxHTPhyMode.field.BW < BW_160) {
			if (entry->wdev->eap.rx_mcs_map.mcs_ss3 == 0)
				raentry->u2SupportVHTMCS3SS = entry->SupportVHTMCS3SS & 255;

			if (entry->wdev->eap.rx_mcs_map.mcs_ss3 == 1)
				raentry->u2SupportVHTMCS3SS = entry->SupportVHTMCS3SS & 511;

			if (entry->wdev->eap.rx_mcs_map.mcs_ss3 == 2)
				raentry->u2SupportVHTMCS3SS = entry->SupportVHTMCS3SS & 1023;

			if (entry->wdev->eap.rx_mcs_map.mcs_ss4 == 0)
				raentry->u2SupportVHTMCS4SS = entry->SupportVHTMCS4SS & 255;

			if (entry->wdev->eap.rx_mcs_map.mcs_ss4 == 1)
				raentry->u2SupportVHTMCS4SS = entry->SupportVHTMCS4SS & 511;

			if (entry->wdev->eap.rx_mcs_map.mcs_ss4 == 2)
				raentry->u2SupportVHTMCS4SS = entry->SupportVHTMCS4SS & 1023;
		}
	}
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_HE_AX
	if (entry->wdev->eap.eap_hesuprate_en) {
		struct he_mcs_info *mcs =
			&pAd->tr_ctl.tr_entry[entry->wcid].StaRec.he_sta.max_nss_mcs;
		struct rate_caps *rate = &entry->wdev->eap.rate;
		INT i;

		for (i = 0; i < HE_MAX_SUPPORT_STREAM; i++ ) {
#define SET_MCS_BY_EAP(mcs, eap_mcs)					\
			do {						\
				if (((mcs) < 3) &&			\
				    (((mcs) > (eap_mcs)) ||(eap_mcs) == 3)) \
					(mcs) = (eap_mcs);		\
			} while (0)

			SET_MCS_BY_EAP(mcs->bw80_mcs[i], rate->he80_rx_nss_mcs[i]);
			SET_MCS_BY_EAP(mcs->bw160_mcs[i], rate->he160_rx_nss_mcs[i]);
			SET_MCS_BY_EAP(mcs->bw8080_mcs[i], rate->he8080_rx_nss_mcs[i]);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "%s, wcid %d: stream %d, bw80_mcs %x, bw160_mcs %x, bw8080_mcs %x\n",
				  entry->wdev->if_dev->name, entry->wcid,
				  i, mcs->bw80_mcs[i], mcs->bw160_mcs[i],
				  mcs->bw8080_mcs[i]);
		}
	}
#endif /* DOT11_HE_AX */
}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


/*----------------------------------------------------------------------------*/
/*!
 * \brief     Restore RaEntry to pEntry
 *
 * \param[in] pAd
 * \param[in] pEntry
 * \param[in] pRaEntry
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
raWrapperEntryRestore(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN P_RA_ENTRY_INFO_T pRaEntry
)
{
	pEntry->MaxHTPhyMode.field.MODE = pRaEntry->MaxPhyCfg.MODE;
	pEntry->MaxHTPhyMode.field.STBC = pRaEntry->MaxPhyCfg.STBC;
	pEntry->MaxHTPhyMode.field.ShortGI = pRaEntry->MaxPhyCfg.ShortGI ? 1 : 0;
	pEntry->MaxHTPhyMode.field.BW = pRaEntry->MaxPhyCfg.BW;
	pEntry->MaxHTPhyMode.field.ldpc = pRaEntry->MaxPhyCfg.ldpc ? 1 : 0;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

	if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT
#ifdef DOT11_HE_AX
		|| pEntry->MaxHTPhyMode.field.MODE == MODE_HE
#endif
		)
		pEntry->MaxHTPhyMode.field.MCS = (((pRaEntry->MaxPhyCfg.VhtNss - 1) & 0x3) << 4) + pRaEntry->MaxPhyCfg.MCS;
	else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	{
		pEntry->MaxHTPhyMode.field.MCS = pRaEntry->MaxPhyCfg.MCS;
	}

	pEntry->HTPhyMode.field.MODE = pRaEntry->TxPhyCfg.MODE;
	pEntry->HTPhyMode.field.STBC = pRaEntry->TxPhyCfg.STBC;
	pEntry->HTPhyMode.field.ShortGI = pRaEntry->TxPhyCfg.ShortGI ? 1 : 0;
	pEntry->HTPhyMode.field.BW = pRaEntry->TxPhyCfg.BW;
	pEntry->HTPhyMode.field.ldpc = pRaEntry->TxPhyCfg.ldpc ? 1 : 0;
	pEntry->HTPhyMode.field.MCS = pRaEntry->TxPhyCfg.MCS;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

	if (pRaEntry->TxPhyCfg.MODE == MODE_VHT
#ifdef DOT11_HE_AX
		|| pRaEntry->TxPhyCfg.MODE == MODE_HE
#endif
			)
		pEntry->HTPhyMode.field.MCS = (((pRaEntry->TxPhyCfg.VhtNss - 1) & 0x3) << 4) + pRaEntry->TxPhyCfg.MCS;
	else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	{
		pEntry->HTPhyMode.field.MCS = pRaEntry->TxPhyCfg.MCS;
	}

	pEntry->LastTxRate = pEntry->HTPhyMode.word;
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief     Set RaCfg according pAd and pAd->CommonCfg.
 *
 * \param[in] pAd
 * \param[out] pRaCfg
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
raWrapperConfigSet(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT P_RA_COMMON_INFO_T pRaCfg)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 TxPath = pAd->Antenna.field.TxPath;
#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (pAd->bAntennaSetAPEnable[BandIdx])
			TxPath = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */


	pRaCfg->OpMode = pAd->OpMode;
	pRaCfg->fgAdHocOn = ADHOC_ON(pAd);
	pRaCfg->fgShortPreamble = OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) ? TRUE : FALSE;
	pRaCfg->TxStream = wlan_operate_get_tx_stream(wdev);
	pRaCfg->RxStream = wlan_operate_get_rx_stream(wdev);
	pRaCfg->ucRateAlg = pAd->rateAlg;
	pRaCfg->TestbedForceShortGI = pAd->WIFItestbed.bShortGI;
	pRaCfg->TestbedForceGreenField = pAd->WIFItestbed.bGreenField;
#ifdef DOT11_N_SUPPORT
	pRaCfg->HtMode = pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
	pRaCfg->fAnyStation20Only = pAd->MacTab.fAnyStation20Only;
	pRaCfg->bRcvBSSWidthTriggerEvents = pAd->CommonCfg.bRcvBSSWidthTriggerEvents;
#ifdef DOT11_VHT_AC
	pRaCfg->vht_nss_cap = pAd->CommonCfg.vht_nss_cap;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pRaCfg->fgSeOff = pAd->CommonCfg.bSeOff;
	if ((pAd->CommonCfg.dbdc_mode == FALSE) && (TxPath == 4))
		pRaCfg->ucAntennaIndex = pAd->CommonCfg.ucAntennaIndex;
	pRaCfg->fgThermalProtectToggle = pAd->fgThermalProtectToggle;
	pRaCfg->force_one_tx_stream = pAd->force_one_tx_stream;
	pRaCfg->TrainUpRule = pAd->CommonCfg.TrainUpRule;
	pRaCfg->TrainUpHighThrd = pAd->CommonCfg.TrainUpHighThrd;
	pRaCfg->TrainUpRuleRSSI = pAd->CommonCfg.TrainUpRuleRSSI;
	pRaCfg->lowTrafficThrd = pAd->CommonCfg.lowTrafficThrd;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
#ifdef RACTRL_LIMIT_MAX_PHY_RATE

	if (pAd->fgRaLimitPhyRate == TRUE)
		pRaCfg->u2MaxPhyRate = RACTRL_LIMIT_MAX_PHY_RATE;
	else
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
	{
		pRaCfg->u2MaxPhyRate = 0;
	}

#endif
	pRaCfg->PhyCaps = cap->phy_caps;
	pRaCfg->u4RaInterval = pAd->ra_interval;
	pRaCfg->u4RaFastInterval = pAd->ra_fast_interval;
}


#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
/*----------------------------------------------------------------------------*/
/*!
 * \brief     The wrapper function of QuickResponeForRateAdaptMTCore()
 *
 * \param[in] pAd
 * \param[in] idx
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
QuickResponeForRateAdaptMT(/* actually for both up and down */
	IN PRTMP_ADAPTER pAd,
	IN UINT_8 idx)
{
	P_RA_ENTRY_INFO_T pRaEntry;
	P_RA_INTERNAL_INFO_T pRaInternal;
	RA_COMMON_INFO_T RaCfg;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR TableSize = 0;
	UCHAR InitTxRateIdx;

	pEntry = &pAd->MacTab.Content[idx]; /* point to information of the individual station */
	pRaEntry = &pEntry->RaEntry;
	pRaInternal = &pEntry->RaInternal;

	if (pRaInternal->ucLastSecTxRateChangeAction == RATE_NO_CHANGE)
		return;

	/* os_zero_mem(pRaEntry, sizeof(RA_ENTRY_INFO_T)); */
	os_zero_mem(&RaCfg, sizeof(RaCfg));
	raWrapperEntrySet(pAd, pEntry, pRaEntry);
	raWrapperConfigSet(pAd, pEntry->wdev, &RaCfg);

	raSelectTxRateTable(pRaEntry, &RaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &InitTxRateIdx);
#ifdef NEW_RATE_ADAPT_SUPPORT

	if (RaCfg.ucRateAlg == RATE_ALG_GRP)
		QuickResponeForRateAdaptMTCore(pAd, pRaEntry, &RaCfg, pRaInternal);

#endif /* NEW_RATE_ADAPT_SUPPORT */
#if defined(RATE_ADAPT_AGBS_SUPPORT) && (!defined(RACTRL_FW_OFFLOAD_SUPPORT) || defined(WIFI_BUILD_RAM))

	if (RaCfg.ucRateAlg == RATE_ALG_AGBS)
		QuickResponeForRateAdaptAGBSMTCore(pAd, pRaEntry, &RaCfg, pRaInternal);

#endif /* RATE_ADAPT_AGBS_SUPPORT */
	raWrapperEntryRestore(pAd, pEntry, pRaEntry);
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief     The wrapper function of DynamicTxRateSwitchingAdaptMtCore()
 *
 * \param[in] pAd
 * \param[in] idx
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
DynamicTxRateSwitchingAdaptMT(
	PRTMP_ADAPTER pAd,
	UINT_8 idx
)
{
	P_RA_ENTRY_INFO_T pRaEntry;
	P_RA_INTERNAL_INFO_T pRaInternal;
	RA_COMMON_INFO_T RaCfg;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR TableSize = 0;
	UCHAR InitTxRateIdx;

	pEntry = &pAd->MacTab.Content[idx]; /* point to information of the individual station */
	pRaEntry = &pEntry->RaEntry;
	pRaInternal = &pEntry->RaInternal;
	/* os_zero_mem(pRaEntry, sizeof(RA_ENTRY_INFO_T)); */
	os_zero_mem(&RaCfg, sizeof(RaCfg));
	raWrapperEntrySet(pAd, pEntry, pRaEntry);
	raWrapperConfigSet(pAd, pEntry->wdev, &RaCfg);

	raSelectTxRateTable(pRaEntry, &RaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &InitTxRateIdx);
#ifdef NEW_RATE_ADAPT_SUPPORT

	if (RaCfg.ucRateAlg == RATE_ALG_GRP)
		DynamicTxRateSwitchingAdaptMtCore(pAd, pRaEntry, &RaCfg, pRaInternal);

#endif /* NEW_RATE_ADAPT_SUPPORT */
#if defined(RATE_ADAPT_AGBS_SUPPORT) && (!defined(RACTRL_FW_OFFLOAD_SUPPORT) || defined(WIFI_BUILD_RAM))

	if (RaCfg.ucRateAlg == RATE_ALG_AGBS)
		DynamicTxRateSwitchingAGBSMtCore(pAd, pRaEntry, &RaCfg, pRaInternal);

#endif /* RATE_ADAPT_AGBS_SUPPORT */
	raWrapperEntryRestore(pAd, pEntry, pRaEntry);
}
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
#endif /* MT_MAC */


#ifdef CONFIG_AP_SUPPORT
/*----------------------------------------------------------------------------*/
/*!
 * \brief     This routine walks through the MAC table, see if TX rate change is
 *            required for each associated client.
 *
 * \param[in] pAd
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
APMlmeDynamicTxRateSwitching(
	PRTMP_ADAPTER pAd
)
{
	UINT i;
	MAC_TABLE_ENTRY *pEntry;
	UINT32 ret;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
	RTMP_SEM_EVENT_WAIT(&pAd->AutoRateLock, ret);

	/* walk through MAC table, see if need to change AP's TX rate toward each entry */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		/* point to information of the individual station */
		pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;

#ifdef APCLI_SUPPORT

		if (IS_ENTRY_PEER_AP(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;

#ifdef MAC_REPEATER_SUPPORT

		if (IS_ENTRY_REPEATER(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;

#endif
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT

		/* if (IS_ENTRY_WDS(pEntry) && !WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx)) */
		if (IS_ENTRY_WDS(pEntry))
			continue;

#endif /* WDS_SUPPORT */

		/* check if this entry need to switch rate automatically */
		if (RTMPCheckEntryEnableAutoRateSwitch(pAd, pEntry) == FALSE)
			continue;

#ifdef MT_MAC
#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)

		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			DynamicTxRateSwitchingAdaptMT(pAd, (UINT_8)i);
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP) {
				UCHAR pkt_num = wlan_operate_get_rts_pkt_thld(pEntry->wdev);
				UINT32 length = wlan_operate_get_rts_len_thld(pEntry->wdev);

				if (pAd->MacTab.Size == 1) {
					if (((pEntry->RaInternal.pucTable == RateSwitchTableAdapt11N2S) && pEntry->HTPhyMode.field.MCS >= 14) ||
						((pEntry->RaInternal.pucTable == RateSwitchTableAdapt11N1S) && pEntry->HTPhyMode.field.MCS >= 6)) {
						if (pAd->bDisableRtsProtect != TRUE) {
							pkt_num = MAX_RTS_PKT_THRESHOLD;
							length = MAX_RTS_THRESHOLD;
							pAd->bDisableRtsProtect = TRUE;
						}
					} else {
						if (pAd->bDisableRtsProtect != FALSE) {
							pAd->bDisableRtsProtect = FALSE;
						}
					}
				} else {
					if (pAd->bDisableRtsProtect != FALSE) {
						pAd->bDisableRtsProtect = FALSE;
					}
				}
				HW_SET_RTS_THLD(pAd, pEntry->wdev, pkt_num, length);
			}
#endif /* NEW_RATE_ADAPT_SUPPORT */
			continue;
		}

#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
#endif /* MT_MAC */
	}
	pAd->fgThermalProtectToggle = FALSE;

	RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief     AP side, Auto TxRate faster train up timer call back function.
 *
 * \param[in] SystemSpecific1
 * \param[in] FunctionContext    Pointer to our Adapter context.
 * \param[in] SystemSpecific2
 * \param[in] SystemSpecific3
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
APQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	UINT i;
	MAC_TABLE_ENTRY *pEntry;

	pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;

	/* walk through MAC table, see if need to change AP's TX rate toward each entry */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;

#ifdef APCLI_SUPPORT

		if (IS_ENTRY_PEER_AP(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;

#ifdef MAC_REPEATER_SUPPORT

		if (IS_ENTRY_REPEATER(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;

#endif
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT

		/* if (IS_ENTRY_WDS(pEntry) && !WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx)) */
		if (IS_ENTRY_WDS(pEntry))
			continue;

#endif /* WDS_SUPPORT */
#ifdef MT_MAC
#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)

		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			QuickResponeForRateAdaptMT(pAd, (UINT_8)i);
			continue;
		}

#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
#endif /* MT_MAC */

		/* Do nothing if this entry didn't change */
		if (pEntry->LastSecTxRateChangeAction == RATE_NO_CHANGE)
			continue;
	}
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
/*----------------------------------------------------------------------------*/
/*!
 * \brief     This routine calculates the acumulated TxPER of eaxh TxRate. And
 *            according to the calculation result, change CommonCfg.TxRate which
 *            is the stable TX Rate we expect the Radio situation could sustained.
 *
 * \param[in] pAd
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID MlmeDynamicTxRateSwitching(
	IN PRTMP_ADAPTER pAd)
{
	ULONG i;
	MAC_TABLE_ENTRY *pEntry;
	UINT32 ret;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

#ifdef CONFIG_STA_SUPPORT
	for (i = 0; i < pAd->MSTANum; i++)
		if ((pAd->StaCfg[i].wdev.DevInfo.WdevActive) && (pAd->StaCfg[i].PwrMgmt.bDoze)) {
			MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"H/W in PM4, return\n");
			return;
		}
#endif /* CONFIG_STA_SUPPORT */

	RTMP_SEM_EVENT_WAIT(&pAd->AutoRateLock, ret);

	/* walk through MAC table, see if need to change AP's TX rate toward each entry */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		/* check if this entry need to switch rate automatically */
		if (RTMPCheckEntryEnableAutoRateSwitch(pAd, pEntry) == FALSE)
			continue;

#ifdef MT_MAC
#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)

		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			DynamicTxRateSwitchingAdaptMT(pAd, (UINT_8)i);
#ifdef NEW_RATE_ADAPT_SUPPORT

			if (pAd->rateAlg == RATE_ALG_GRP) {
				UCHAR pkt_num = wlan_operate_get_rts_pkt_thld(pEntry->wdev);
				UINT32 length = wlan_operate_get_rts_len_thld(pEntry->wdev);

				if (pAd->MacTab.Size == 1) {
					if (((pEntry->RaInternal.pucTable == RateSwitchTableAdapt11N2S) && pEntry->HTPhyMode.field.MCS >= 14) ||
						((pEntry->RaInternal.pucTable == RateSwitchTableAdapt11N1S) && pEntry->HTPhyMode.field.MCS >= 6)) {
						if (pAd->bDisableRtsProtect != TRUE) {
							pkt_num = MAX_RTS_PKT_THRESHOLD;
							length = MAX_RTS_THRESHOLD;
							pAd->bDisableRtsProtect = TRUE;
						}
					} else {
						if (pAd->bDisableRtsProtect != FALSE) {
							pAd->bDisableRtsProtect = FALSE;
						}
					}
				} else {
					if (pAd->bDisableRtsProtect != FALSE) {
						pAd->bDisableRtsProtect = FALSE;
					}
				}
				HW_SET_RTS_THLD(pAd, pEntry->wdev, pkt_num, length);
			}

#endif /* NEW_RATE_ADAPT_SUPPORT */
			continue;
		}

#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
#endif /* MT_MAC */
	}
	pAd->fgThermalProtectToggle = FALSE;

	RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief     Station side, Auto TxRate faster train up timer call back function.
 *
 * \param[in] SystemSpecific1
 * \param[in] FunctionContext    Pointer to our Adapter context.
 * \param[in] SystemSpecific2
 * \param[in] SystemSpecific3
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
StaQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	ULONG i;
	MAC_TABLE_ENTRY *pEntry;

	pAd->StaCfg[0].StaQuickResponeForRateUpTimerRunning = FALSE;

	/* walk through MAC table, see if need to change AP's TX rate toward each entry */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		/* check if this entry need to switch rate automatically */
		if (RTMPCheckEntryEnableAutoRateSwitch(pAd, pEntry) == FALSE)
			continue;

#ifdef MT_MAC
#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)

		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			QuickResponeForRateAdaptMT(pAd, (UINT_8)i);
			continue;
		}

#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
#endif /* MT_MAC */

		/* Do nothing if this entry didn't change */
		if (pEntry->LastSecTxRateChangeAction == RATE_NO_CHANGE)
			continue;
	}
}
#endif /* CONFIG_STA_SUPPORT */


/*----------------------------------------------------------------------------*/
/*!
 * \brief     This routine parse rate IEs and ouput the supported MCS table.
 *
 * \param[in] pAd
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID RTMPSetSupportMCS(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR OpMode,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN struct legacy_rate *rate,
#ifdef DOT11_VHT_AC
	IN BOOLEAN has_vht_cap,
	IN VHT_CAP_IE * vht_cap,
#endif /* DOT11_VHT_AC */
	IN HT_CAPABILITY_IE * pHtCapability,
	IN BOOLEAN has_ht_cap
)
{
	UCHAR idx, sum_rate_len = 0;
	UCHAR sum_rate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR sup_rate_len, ext_rate_len, *sup_rate, *ext_rate;

	sup_rate_len = rate->sup_rate_len;
	ext_rate_len = rate->ext_rate_len;
	sup_rate = rate->sup_rate;
	ext_rate = rate->ext_rate;

	if (sup_rate_len > 0) {
		if (sup_rate_len <= MAX_LEN_OF_SUPPORTED_RATES) {
			os_move_mem(sum_rate, sup_rate, sup_rate_len);
			sum_rate_len = sup_rate_len;
		} else {
			UCHAR RateDefault[8] = {0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c};

			os_move_mem(sum_rate, RateDefault, 8);
			MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_WARN, "wrong SUPP RATES., Len=%d\n", sup_rate_len);
			sup_rate_len = 8;
		}
	}

	if (ext_rate_len > 0) {
		if ((sup_rate_len + ext_rate_len) <= MAX_LEN_OF_SUPPORTED_RATES) {
			os_move_mem(&sum_rate[sup_rate_len], ext_rate, ext_rate_len);
			sum_rate_len += ext_rate_len;
		} else if (sup_rate_len < MAX_LEN_OF_SUPPORTED_RATES) {
			os_move_mem(&sum_rate[sup_rate_len], ext_rate, MAX_LEN_OF_SUPPORTED_RATES - sup_rate_len);
			sum_rate_len = MAX_LEN_OF_SUPPORTED_RATES;
		}
	}

	/* Clear Supported MCS Table */
	pEntry->SupportCCKMCS = 0;
	pEntry->SupportOFDMMCS = 0;
	pEntry->SupportHTMCS = 0;
#ifdef DOT11_VHT_AC
	pEntry->SupportVHTMCS1SS = 0;
	pEntry->SupportVHTMCS2SS = 0;
	pEntry->SupportVHTMCS3SS = 0;
	pEntry->SupportVHTMCS4SS = 0;
#endif /* DOT11_VHT_AC */
	pEntry->SupportRateMode = 0;

	MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "sum_rate_len: %d,  sum_rate: ", sum_rate_len);
	for (idx = 0; idx < sum_rate_len; idx++)
		MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "0x%x ", sum_rate[idx]);
	MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\n");

	for (idx = 0; idx < sum_rate_len; idx++) {
		switch ((sum_rate[idx] & 0x7F) * 5) {
		case 10:
			pEntry->SupportCCKMCS |= (1 << MCS_0);
			pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
			break;

		case 20:
			pEntry->SupportCCKMCS |= (1 << MCS_1);
			pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
			break;

		case 55:
			pEntry->SupportCCKMCS |= (1 << MCS_2);
			pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
			break;

		case 110:
			pEntry->SupportCCKMCS |= (1 << MCS_3);
			pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
			break;

		case 60:
			pEntry->SupportOFDMMCS |= (1 << MCS_0);
			pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
			break;

		case 90:
			pEntry->SupportOFDMMCS |= (1 << MCS_1);
			pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
			break;

		case 120:
			pEntry->SupportOFDMMCS |= (1 << MCS_2);
			pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
			break;

		case 180:
			pEntry->SupportOFDMMCS |= (1 << MCS_3);
			pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
			break;

		case 240:
			pEntry->SupportOFDMMCS |= (1 << MCS_4);
			pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
			break;

		case 360:
			pEntry->SupportOFDMMCS |= (1 << MCS_5);
			pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
			break;

		case 480:
			pEntry->SupportOFDMMCS |= (1 << MCS_6);
			pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
			break;

		case 540:
			pEntry->SupportOFDMMCS |= (1 << MCS_7);
			pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
			break;
		}
	}

	if (has_ht_cap) {
		RT_PHY_INFO *pDesired_ht_phy = NULL;
		UCHAR j, bitmask;
		CHAR i;
#ifdef CONFIG_STA_SUPPORT

		if (OpMode == OPMODE_STA)
			pDesired_ht_phy = &pEntry->wdev->DesiredHtPhyInfo;

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (OpMode == OPMODE_AP) {
#ifdef WDS_SUPPORT

			if (IS_ENTRY_WDS(pEntry))
				pDesired_ht_phy = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
			else
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
				if (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
					pDesired_ht_phy = &pEntry->wdev->DesiredHtPhyInfo;
				else
#endif /* APCLI_SUPPORT */
					{
						pDesired_ht_phy = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
					}
		}

#endif /* CONFIG_AP_SUPPORT */

		if (pDesired_ht_phy == NULL)
			return;

		for (i = 31; i >= 0; i--) {
			j = i / 8;
			bitmask = (1 << (i - (j * 8)));

			if ((pDesired_ht_phy->MCSSet[j] & bitmask)
				&& (pHtCapability->MCSSet[j] & bitmask)) {
				pEntry->SupportHTMCS |= 1 << i;
				pEntry->SupportRateMode |= SUPPORT_HT_MODE;
			}
		}

#ifdef DOT11_VHT_AC

		if (has_vht_cap && (vht_cap != NULL) && pDesired_ht_phy->bVhtEnable) {
			UINT8 nTxStream = wlan_operate_get_tx_stream(pEntry->wdev);
			/* Currently we only support for MCS0~MCS7, so don't check mcs_map */
			pEntry->SupportVHTMCS1SS = 0;
			pEntry->SupportVHTMCS2SS = 0;
			pEntry->SupportVHTMCS3SS = 0;
			pEntry->SupportVHTMCS4SS = 0;
			if (nTxStream > 4)
				nTxStream = 4;

			for (j = nTxStream ; j > 0; j--) {
				switch (j) {
				case 1:
					if (vht_cap->mcs_set.rx_mcs_map.mcs_ss1 < VHT_MCS_CAP_NA) {
						for (i = 0; i <= 7; i++)
							pEntry->SupportVHTMCS1SS |= 1 << i;

						if (vht_cap->mcs_set.rx_mcs_map.mcs_ss1 == VHT_MCS_CAP_8)
							pEntry->SupportVHTMCS1SS |= 1 << 8;
						else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss1 == VHT_MCS_CAP_9) {
							pEntry->SupportVHTMCS1SS |= 1 << 8;
							pEntry->SupportVHTMCS1SS |= 1 << 9;
						}

						pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
					}

					break;

				case 2:
					if (vht_cap->mcs_set.rx_mcs_map.mcs_ss2 < VHT_MCS_CAP_NA) {
						for (i = 0; i <= 7; i++)
							pEntry->SupportVHTMCS2SS |= 1 << i;

						if (vht_cap->mcs_set.rx_mcs_map.mcs_ss2 == VHT_MCS_CAP_8)
							pEntry->SupportVHTMCS2SS |= 1 << 8;
						else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss2 == VHT_MCS_CAP_9) {
							pEntry->SupportVHTMCS2SS |= 1 << 8;
							pEntry->SupportVHTMCS2SS |= 1 << 9;
						}

						pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
					}

					break;

				case 3:
					if (vht_cap->mcs_set.rx_mcs_map.mcs_ss3 < VHT_MCS_CAP_NA) {
						for (i = 0; i <= 7; i++)
							pEntry->SupportVHTMCS3SS |= 1 << i;

						if (vht_cap->mcs_set.rx_mcs_map.mcs_ss3 == VHT_MCS_CAP_8)
							pEntry->SupportVHTMCS3SS |= 1 << 8;
						else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss3 == VHT_MCS_CAP_9) {
							pEntry->SupportVHTMCS3SS |= 1 << 8;
							pEntry->SupportVHTMCS3SS |= 1 << 9;
						}

						pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
					}

					break;

				case 4:
					if (vht_cap->mcs_set.rx_mcs_map.mcs_ss4 < VHT_MCS_CAP_NA) {
						for (i = 0; i <= 7; i++)
							pEntry->SupportVHTMCS4SS |= 1 << i;

						if (vht_cap->mcs_set.rx_mcs_map.mcs_ss4 == VHT_MCS_CAP_8)
							pEntry->SupportVHTMCS4SS |= 1 << 8;
						else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss4 == VHT_MCS_CAP_9) {
							pEntry->SupportVHTMCS4SS |= 1 << 8;
							pEntry->SupportVHTMCS4SS |= 1 << 9;
						}

						pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
					}

					break;

				default:
					break;
				}
			}
		}
#endif /* DOT11_VHT_AC */
	}
}
