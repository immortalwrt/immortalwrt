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
	txbf_wrapper_embedded.c
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "rt_config.h"

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
#ifdef TXBF_SUPPORT
#ifdef MT_MAC
/*----------------------------------------------------------------------------*/

/* Wrap function for TxBFInit */
VOID mt_WrapTxBFInit(
	IN PRTMP_ADAPTER	pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN IE_LISTS * ie_list,
	IN BOOLEAN			supportsETxBF)
{
	TXBF_MAC_TABLE_ENTRY TxBfMacEntry;
	TXBF_STATUS_INFO  TxBfInfo;

	TxBfInfo.ucETxBfTxEn = (UCHAR) pAd->CommonCfg.ETxBfEnCond;
	mt_TxBFInit(pAd, &TxBfInfo, &TxBfMacEntry, supportsETxBF);
	pEntry->eTxBfEnCond    = TxBfMacEntry.eTxBfEnCond;
}

/* Wrap function for clientSupportsETxBF */
BOOLEAN mt_WrapClientSupportsETxBF(
	IN  PRTMP_ADAPTER    pAd,
	IN  HT_BF_CAP       *pTxBFCap)
{
	TXBF_STATUS_INFO  TxBfInfo;

	TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
	return mt_clientSupportsETxBF(pAd, pTxBFCap, TxBfInfo.cmmCfgETxBfNoncompress);
}


#ifdef VHT_TXBF_SUPPORT
/* Wrap function for clientSupportsVHTETxBF */
BOOLEAN mt_WrapClientSupportsVhtETxBF(
	IN  PRTMP_ADAPTER    pAd,
	IN  VHT_CAP_INFO * pTxBFCap)
{
	return mt_clientSupportsVhtETxBF(pAd, pTxBFCap);
}
#endif /* VHT_TXBF_SUPPORT */


/* wrapper for mt_chk_itxbf_calibration */
BOOLEAN mt_Wrap_chk_itxbf_calibration(
	IN RTMP_ADAPTER * pAd, struct wifi_dev *wdev)
{
	TXBF_STATUS_INFO TxBfInfo;

	TxBfInfo.u2Channel = wdev->channel;
	/* return mt_chk_itxbf_calibration(pAd,&TxBfInfo); */
	return TRUE;
}

/* wrapper for setETxBFCap */
void mt_WrapSetETxBFCap(
	IN  RTMP_ADAPTER * pAd,
	IN  struct wifi_dev *wdev,
	IN  HT_BF_CAP         *pTxBFCap)
{
	TXBF_STATUS_INFO   TxBfInfo;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	TxBfInfo.pHtTxBFCap = pTxBFCap;
	TxBfInfo.cmmCfgETxBfEnCond = wlan_config_get_etxbf(wdev);
	TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
	TxBfInfo.ucRxPathNum = pAd->Antenna.field.RxPath;
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			TxBfInfo.ucRxPathNum = pAd->dbdc_band0_rx_path;
		else
			TxBfInfo.ucRxPathNum = pAd->dbdc_band1_rx_path;
	}
#endif

	if (ops->setETxBFCap)
		ops->setETxBFCap(pAd, &TxBfInfo);
}

#ifdef VHT_TXBF_SUPPORT
/* Wrapper for mt_setVHTETxBFCap */
void mt_WrapSetVHTETxBFCap(
	IN  RTMP_ADAPTER * pAd,
	IN  struct wifi_dev *wdev,
	IN  VHT_CAP_INFO * pTxBFCap)
{
	TXBF_STATUS_INFO   TxBfInfo;
	UINT8   ucTxPath = pAd->Antenna.field.TxPath;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg;
#endif

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			ucTxPath = pAd->dbdc_band0_tx_path;
		else
			ucTxPath = pAd->dbdc_band1_tx_path;
	}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (pAd->bAntennaSetAPEnable[BandIdx])
			ucTxPath = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	TxBfInfo.pVhtTxBFCap = pTxBFCap;
	TxBfInfo.ucTxPathNum = ucTxPath;
	TxBfInfo.cmmCfgETxBfEnCond = wlan_config_get_etxbf(wdev);

#ifdef CONFIG_STA_SUPPORT
	/* Netgear R7000 BF IOT
	  * For apcli, adjust bfee_sts_cap (BFee Nr) with Root AP's num_snd_dimension if Root AP is SU BFer
	  * Set minimum nego-ed bfee_sts_cap
	  * Root AP's BF cap is stored in pStaCfg from Probe req
	*/
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		TxBfInfo.pVhtTxBFCap->bfee_sts_cap = 0;
		pStaCfg = GetStaCfgByWdev(pAd, wdev);
		if (pStaCfg) {
			if (pStaCfg->MlmeAux.vht_cap.vht_cap.bfer_cap_su) {
				TxBfInfo.pVhtTxBFCap->bfee_sts_cap = pStaCfg->MlmeAux.vht_cap.vht_cap.num_snd_dimension;

				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: %s num_snd_dimension=%d, bfee_sts_cap=%d, bfer=%d, bfee=%d\n",
					__func__, wdev->if_dev->name, pStaCfg->MlmeAux.vht_cap.vht_cap.num_snd_dimension, pStaCfg->MlmeAux.vht_cap.vht_cap.bfee_sts_cap,
					pStaCfg->MlmeAux.vht_cap.vht_cap.bfer_cap_su, pStaCfg->MlmeAux.vht_cap.vht_cap.bfee_cap_su);
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	if (ops->setVHTETxBFCap)
		ops->setVHTETxBFCap(pAd, &TxBfInfo);
}
#endif /* VHT_TXBF_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef HE_TXBF_SUPPORT
void mt_wrap_get_he_bf_cap(
	struct wifi_dev *wdev,
	struct he_bf_info *he_bf_struct)
{
	TXBF_STATUS_INFO txbf_status;
	struct mcs_nss_caps *mcs_nss = wlan_config_get_mcs_nss_caps(wdev);
	UINT8 u1BandIdx = HcGetBandByWdev(wdev);
	UINT8 he_bw = wlan_config_get_he_bw(wdev);
	UINT8 u1TxPath = mcs_nss->max_path[u1BandIdx][MAX_PATH_TX];/* Tx Path Num from eeprom: 1 for 1T, 2 for 2T, etc. */
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(hdev_ctrl);

	if (!IS_PHY_CAPS(pChipCap->phy_caps, fPHY_CAP_DUALPHY)) {
		/* for non-dual PHY, Tx Path num for BW160 is different from BW80 */
		if (he_bw > HE_BW_80) {
			if (u1TxPath > (mcs_nss->bw160_max_nss))/* bw160_max_nss: 1 for 1T, 2 for 2T, etc. */
				u1TxPath = mcs_nss->bw160_max_nss;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"%s: mcs_nss->bw160_max_nss=%u, u1TxPath=%u\n", __func__, mcs_nss->bw160_max_nss, u1TxPath);

	txbf_status.he_bf_info = he_bf_struct;
	txbf_status.ucTxPathNum = u1TxPath;
	txbf_status.cmmCfgETxBfEnCond = wlan_config_get_etxbf(wdev);
	if (ops->get_he_etxbf_cap)
		ops->get_he_etxbf_cap(wdev, &txbf_status);
}
#endif /*DOT11_HE_AX*/
#endif /* HE_TXBF_SUPPORT */

void mt_WrapIBfCalGetEBfMemAlloc(
						IN  RTMP_ADAPTER * pAd,
						IN  PCHAR pPfmuMemRow,
						IN  PCHAR pPfmuMemCol)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->iBfCaleBfPfmuMemAlloc)
		ops->iBfCaleBfPfmuMemAlloc(pAd, pPfmuMemRow, pPfmuMemCol);
}

void mt_WrapIBfCalGetIBfMemAlloc(
						IN  RTMP_ADAPTER * pAd,
						IN  PCHAR pPfmuMemRow,
						IN  PCHAR pPfmuMemCol)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->iBfCaliBfPfmuMemAlloc)
		ops->iBfCaliBfPfmuMemAlloc(pAd, pPfmuMemRow, pPfmuMemCol);
}

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
