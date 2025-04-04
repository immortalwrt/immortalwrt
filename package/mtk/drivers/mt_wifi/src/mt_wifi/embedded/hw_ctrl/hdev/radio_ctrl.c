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

*/

#include "rt_config.h"
#include "hdev/hdev.h"

/*Radio controller*/


/*
 *
*/

/*Local functions*/
static UCHAR rcGetRfByIdx(struct hdev_ctrl *ctrl, UCHAR DbdcMode, UCHAR BandIdx)
{
	/* TODO: Should remove when antenna move to rdev */
#ifdef DBDC_MODE
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	if (ctrl->chip_ops.BandGetByIdx && DbdcMode)
		return ctrl->chip_ops.BandGetByIdx(pAd, BandIdx);
	else
		return (RFIC_24GHZ | RFIC_5GHZ | RFIC_6GHZ);

#endif /*DBDC_MODE*/
	return (RFIC_24GHZ | RFIC_5GHZ | RFIC_6GHZ);
}


/*Get RfIC Band from EEPORM content*/
static UINT8 rcGetBandSupport(struct hdev_ctrl *ctrl, UCHAR DbdcMode, UCHAR BandIdx)
{
	/* TODO: Should remove when antenna move to rdev */
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	if (BOARD_IS_5G_ONLY(pAd))
		return RFIC_5GHZ;
	else if (BOARD_IS_2G_ONLY(pAd))
		return RFIC_24GHZ;
	else if (RFIC_IS_5G_BAND(pAd))
		return rcGetRfByIdx(ctrl, DbdcMode, BandIdx);
	else
		return RFIC_24GHZ;
}

static UCHAR rcGetDefaultChannel(USHORT PhyMode)
{
	/*priority must the same as Default PhyMode*/
	if (WMODE_CAP_2G(PhyMode))
		return 1;
	else if (WMODE_CAP_6G(PhyMode))
		return 37;
	else if (WMODE_CAP_5G(PhyMode))
		return 36;
	return 0;
}

static USHORT rcGetDefaultPhyMode(UCHAR rf_band_cap)
{
	/*priority must the same as Default Channel*/
	if (rf_band_cap & RFIC_24GHZ)
		return WMODE_AX_24G;
	else
	if (rf_band_cap & RFIC_5GHZ)
		return WMODE_AX_5G;
	else
	if (rf_band_cap & RFIC_6GHZ)
		return WMODE_AX_6G;
	/*should return 0 since no rf_band_cap*/
	return 0;
}

static BOOLEAN rcCheckIsTheSameBand(USHORT target_phymode, USHORT cur_phymode)
{
	return (wmode_2_rfic(target_phymode) == wmode_2_rfic(cur_phymode));
}

static UCHAR get_cur_rfic_by_phymode(USHORT PhyMode)
{
	UCHAR rf_mode = 0;

	if (WMODE_CAP_6G(PhyMode))
		rf_mode = RFIC_6GHZ;
	else
	if (WMODE_CAP_5G(PhyMode))
		rf_mode = RFIC_5GHZ;
	else
	if (WMODE_CAP_2G(PhyMode))
		rf_mode = RFIC_24GHZ;

	return rf_mode;
}

static INT32 rcUpdatePhyMode(struct radio_dev *rdev, USHORT PhyMode)
{
	INT32 ret = 0;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;
	/*band is not changed or not*/
	if (rcCheckIsTheSameBand(PhyMode, pRadioCtrl->PhyMode)) {
		pRadioCtrl->PhyMode |= PhyMode;
	} else {
		/*band is changed*/
		pRadioCtrl->PhyMode = PhyMode;
		pRadioCtrl->Channel = rcGetDefaultChannel(PhyMode);
		RcUpdateBandCtrl(rdev->priv);
	}
	pRadioCtrl->cur_rfic_type = get_cur_rfic_by_phymode(PhyMode);
	return ret;
}

static struct radio_dev *rcGetHdevByRf(struct hdev_ctrl *ctrl, UCHAR RfType)
{
	INT i;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	RADIO_CTRL *radio_ctrl = NULL;
	/*get hdev by phymode first*/
	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		radio_ctrl = &pHwResource->PhyCtrl[i].RadioCtrl;
		if (wmode_2_rfic(radio_ctrl->PhyMode) & RfType)
			return &ctrl->rdev[i];
	}
	/*get hdev by cap*/
	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (pHwResource->PhyCtrl[i].rf_band_cap & RfType)
			return &ctrl->rdev[i];
	}
	return NULL;
}


#ifdef DBDC_MODE
static RADIO_CTRL *rcGetRadioCtrlByRf(struct hdev_ctrl *ctrl, UCHAR RfType)
{
	INT i;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (RfType &  pHwResource->PhyCtrl[i].rf_band_cap)
			return &pHwResource->PhyCtrl[i].RadioCtrl;
	}

	return NULL;
}


static VOID rcFillEntry(BCTRL_ENTRY_T *pEntry, UINT8 Type, UINT8 BandIdx, UINT8 Index)
{
	pEntry->Type = Type;
	pEntry->BandIdx = BandIdx;
	pEntry->Index = Index;
}



static INT32 rcUpdateBandForMBSS(struct hdev_obj *obj, BCTRL_ENTRY_T *pEntry)
{
	struct radio_dev *rdev = obj->rdev;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;
	UCHAR MbssIdx;
	struct hdev_ctrl *ctrl = rdev->priv;

	if (obj->OmacIdx == 0)
		rcFillEntry(pEntry, DBDC_TYPE_BSS, pRadioCtrl->BandIdx, 0);
	else {
		/*ctrl->chipCap.ExtMbssOmacStartIdx+1 since 0x10 will control by 0x10*/
		MbssIdx = obj->OmacIdx - (ctrl->chip_cap.ExtMbssOmacStartIdx+1);
		rcFillEntry(pEntry, DBDC_TYPE_MBSS, pRadioCtrl->BandIdx, MbssIdx);
	}

	return 0;
}


static INT32 rcUpdateBandForBSS(struct hdev_obj *obj, BCTRL_ENTRY_T *pEntry)
{
	struct radio_dev *rdev = obj->rdev;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	rcFillEntry(pEntry, DBDC_TYPE_BSS, pRadioCtrl->BandIdx, obj->OmacIdx);
	return 0;
}


static INT32 rcUpdateBandByType(struct hdev_obj *obj, BCTRL_ENTRY_T *pEntry)
{
	switch (obj->Type) {
	case WDEV_TYPE_AP:
	case WDEV_TYPE_ATE_AP:
	case WDEV_TYPE_SERVICE_TXC:
	{
		rcUpdateBandForMBSS(obj, pEntry);
	}
	break;

	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_GO:
	case WDEV_TYPE_GC:
	case WDEV_TYPE_ATE_STA:
	case WDEV_TYPE_SERVICE_TXD:
	{
		rcUpdateBandForBSS(obj, pEntry);
	}
	break;

	case WDEV_TYPE_WDS:
	case WDEV_TYPE_MESH:
	default: {
		/* TODO: STAR for DBDC */
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s(): Current not support this type of WdevType=%d\n", __func__, obj->Type);
		return -1;
	}
	break;
	}

	return 0;
}



/*Must call after update ownmac*/
static INT32 rcUpdateBandForBFMU(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	struct radio_dev *rdev;
	RADIO_CTRL *pRadioCtrl = NULL;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	BCTRL_ENTRY_T *pEntry = NULL;
	UINT32 i;
#ifdef TXBF_SUPPORT
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /*TXBF_SUPPORT*/

	/*first choice 5G as the BF/MU band*/
	rdev = rcGetHdevByRf(ctrl, RFIC_5GHZ);

	/*else 2.4G*/
	if (!rdev || (rdev->DevNum == 0))
		rdev = rcGetHdevByRf(ctrl, RFIC_24GHZ);

	/*If MU is not enable & 5G not support , else select first dev as bf band*/
	if (!rdev)
		rdev = &ctrl->rdev[0];

	pRadioCtrl = rdev->pRadioCtrl;

	/*If get phyCtrl, set bf to this band*/
	if (pRadioCtrl == NULL)
		return -1;

	/*support MU & enable MU, BF & MU should be 5G only*/
	pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
	rcFillEntry(pEntry, DBDC_TYPE_MU, pRadioCtrl->BandIdx, 0);
	pBInfo->TotalNum++;

	for (i = 0; i < 3; i++) {
		pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
		rcFillEntry(pEntry, DBDC_TYPE_BF, pRadioCtrl->BandIdx, i);
		pBInfo->TotalNum++;
	}

#ifdef TXBF_SUPPORT
	if ((pChipCap->FlgHwTxBfCap & TXBF_HW_CAP)
		&& (pChipCap->FlgHwTxBfCap & TXBF_HW_2BF)) {
		for (i = 0; i < pHwResource->concurrent_bands; i++) {
			pHwResource->PhyCtrl[i].RadioCtrl.IsBfBand = TRUE;
		}
	} else
#endif /*TXBF_SUPPORT*/
	{
		pRadioCtrl->IsBfBand = TRUE;

		for (i = 0; i < pHwResource->concurrent_bands; i++) {
			if (pHwResource->PhyCtrl[i].RadioCtrl.IsBfBand &&
				(pHwResource->PhyCtrl[i].RadioCtrl.BandIdx != pRadioCtrl->BandIdx))
				pHwResource->PhyCtrl[i].RadioCtrl.IsBfBand = FALSE;
		}
	}

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
	   MTWF_DBG(NULL, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s	PhyCtrl[%d].RadioCtrl.IsBfBand = %d\n", __func__, i, pHwResource->PhyCtrl[i].RadioCtrl.IsBfBand);
	}
#ifdef TXBF_SUPPORT
	TxBfModuleEnCtrl(pAd);
#endif /*TXBF_SUPPORT*/
	return 0;
}


static INT32 rcUpdateBandForRepeater(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	INT32 i;
	BCTRL_ENTRY_T *pEntry;
	struct radio_dev  *rdev;
	struct hdev_obj *obj;
	HD_REPT_ENRTY *pReptEntry = NULL, *tmp = NULL;

	if (ctrl->HwResourceCfg.concurrent_bands == 2) { /* DBDC mode */
		for (i = 0; i < ctrl->HwResourceCfg.concurrent_bands; i++) {
			/* search repeater entry from all obj */
			rdev = &ctrl->rdev[i];
			DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
				DlListForEachSafe(pReptEntry, tmp, &obj->RepeaterList, struct _HD_REPT_ENRTY, list) {
					pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
					rcFillEntry(pEntry, DBDC_TYPE_REPEATER,
						rdev->pRadioCtrl->BandIdx, pReptEntry->CliIdx);
					pBInfo->TotalNum++;
				}
			}
		}
	} else {
		for (i = 0; i < ctrl->chip_cap.MaxRepeaterNum; i++) {
			pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
			/*always bind band 0*/
			rcFillEntry(pEntry, DBDC_TYPE_REPEATER, 0, i);
			pBInfo->TotalNum++;
		}
	}

	return 0;
}



static INT32 rcUpdateBandForWMM(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	INT32 i;
	struct wmm_entry *wentry;
	struct wmm_ctrl *wctrl = &ctrl->HwResourceCfg.wmm_ctrl;
	BCTRL_ENTRY_T *pEntry;

	for (i = 0; i < wctrl->num; i++) {
		wentry = wmm_ctrl_get_entry_by_idx(ctrl, i);

		if (!wentry->edca.bValid)
			continue;

		pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
		rcFillEntry(pEntry, DBDC_TYPE_WMM, wentry->dbdc_idx, i);
		pBInfo->TotalNum++;
	}

	return 0;
}


static INT32 rcUpdateBandForMGMT(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	INT32 i;
	BCTRL_ENTRY_T *pEntry;

	for (i = 0; i < 2; i++) {
		pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
		rcFillEntry(pEntry, DBDC_TYPE_MGMT, i, i);
		pBInfo->TotalNum++;
	}

	return 0;
}


static INT32 rcUpdateBandForPTA(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	BCTRL_ENTRY_T *pEntry;
	RADIO_CTRL *pRadioCtrl;

	pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
	/*fix to bind band 0 for 2.4G band*/
	pRadioCtrl = rcGetRadioCtrlByRf(ctrl, RFIC_24GHZ);

	if (pRadioCtrl != NULL)
		rcFillEntry(pEntry, DBDC_TYPE_PTA, pRadioCtrl->BandIdx, 0);
	else
		rcFillEntry(pEntry, DBDC_TYPE_PTA, 0, 0);

	pBInfo->TotalNum++;
	return 0;
}

static INT32 rcUpdateBandForOwnMac(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	INT32 i, ret = 0;
	struct hdev_obj *obj;
	struct radio_dev *rdev;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	BCTRL_ENTRY_T *pEntry = NULL;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];
		DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
			pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
			rcUpdateBandByType(obj, pEntry);
			pBInfo->TotalNum++;
		}
	}

	return ret;
}
#endif /*DBDC_MODE*/

/*
* Get rdev by PhyMode & Channel, can't find pHdev may rdev is full
*/
static struct radio_dev *rcGetHdevByPhyMode(struct hdev_ctrl *ctrl, USHORT phymode, UCHAR channel, USHORT ObjType)
{
	UCHAR i;
	struct radio_dev *rdev = NULL;
	struct _HD_RESOURCE_CFG *hw_res = &ctrl->HwResourceCfg;
	struct rtmp_phy_ctrl *phy_ctrl = NULL;

	/*single band case*/
	if (hw_res->concurrent_bands == 1)
		return &ctrl->rdev[0];

	/*multi-band case, choose single cap radio first*/
	for (i = 0 ; i < hw_res->concurrent_bands; i++) {
		phy_ctrl = &hw_res->PhyCtrl[i];
		if (WMODE_CAP_2G(phymode) && (phy_ctrl->rf_band_cap & RFIC_24GHZ))
			rdev = &ctrl->rdev[i];
		else if ((WMODE_CAP_5G(phymode) && (phy_ctrl->rf_band_cap & RFIC_5GHZ)) ||
			(WMODE_CAP_6G(phymode) && (phy_ctrl->rf_band_cap & RFIC_6GHZ)))
			rdev = &ctrl->rdev[i];
		/*Used for phymod and rf_band_cap mismatch on first time enter testmode*/
		else if (((WMODE_CAP_5G(phymode) && (phy_ctrl->rf_band_cap & RFIC_6GHZ)) ||
			(WMODE_CAP_6G(phymode) && (phy_ctrl->rf_band_cap & RFIC_5GHZ))) &&
			(is_testmode_wdev(ObjType)))
			rdev = &ctrl->rdev[i];
		else
			rdev = NULL;

		if (rdev) {
			/*specific channel case, auto channel*/
			if (!channel) {
				if ((rdev->DevNum == 0) ||
					((channel == RcGetChannel(rdev)) && WMODE_EQUAL(RcGetPhyMode(rdev), phymode))) {
					/*early return*/
					return rdev;
				}
			}

			if (((channel == RcGetChannel(rdev)) && WMODE_EQUAL(RcGetPhyMode(rdev), phymode)) ||
				WMODE_CAP_2G(phymode)) {
				/*early return*/
				return rdev;
			}

			/*generic case*/
			if ((WMODE_CAP_2G(phymode) && phy_ctrl->rf_band_cap == RFIC_24GHZ) ||
				(WMODE_CAP_5G(phymode) && phy_ctrl->rf_band_cap == RFIC_5GHZ) ||
				(WMODE_CAP_6G(phymode) && phy_ctrl->rf_band_cap == RFIC_6GHZ)) {
				/*early return*/
				return rdev;
			}
		}
	}
	if (!rdev) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"can't find radio for phymode:%u, channel: %d !\n",
		phymode, channel);
		/*return radio 0 as default rdev*/
		rdev = &ctrl->rdev[0];
	}
	return rdev;
}

/*Export functions*/
/*
*
*/
INT32 RcUpdateBandCtrl(struct hdev_ctrl *ctrl)
{
#if defined(TXBF_SUPPORT) || defined(DBDC_MODE)
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;
#endif

#ifdef DBDC_MODE
	INT32 ret = 0;
	BCTRL_INFO_T BctrlInfo;

	os_zero_mem(&BctrlInfo, sizeof(BCTRL_INFO_T));
	BctrlInfo.DBDCEnable = pAd->CommonCfg.dbdc_mode;

	/*if enable dbdc, run band selection algorithm*/
	if (IS_CAP_DBDC(ctrl->chip_cap) && BctrlInfo.DBDCEnable) {
		/*Since phyctrl  need to update */
		rcUpdateBandForOwnMac(ctrl, &BctrlInfo);
		rcUpdateBandForBFMU(ctrl, &BctrlInfo);
		rcUpdateBandForWMM(ctrl, &BctrlInfo);
		rcUpdateBandForMGMT(ctrl, &BctrlInfo);
		rcUpdateBandForPTA(ctrl, &BctrlInfo);
		rcUpdateBandForRepeater(ctrl, &BctrlInfo);
		/*Since will add one more time, must minus 1*/
		BctrlInfo.TotalNum = (BctrlInfo.TotalNum-1);

		if (BctrlInfo.TotalNum > MAX_BCTRL_ENTRY)
			BctrlInfo.TotalNum = MAX_BCTRL_ENTRY;
	}

	ret = AsicSetDbdcCtrl(pAd, &BctrlInfo);

	if (ret != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error for conifgure dbdc, ret = %d !\n", ret);

#endif /*DBDC_MODE*/

#ifdef TXBF_SUPPORT
	TxBfCfgBfPhy(pAd);
#endif /*TXBF_SUPPORT*/

	return 0;
}


/*
*
*/
INT32 RcUpdateRepeaterEntry(struct radio_dev *rdev, UINT32 ReptIdx)
{
	INT32 ret = 0;
#ifdef DBDC_MODE
	BCTRL_ENTRY_T *pEntry;
	BCTRL_INFO_T BandInfoValue;
	struct hdev_ctrl *ctrl = rdev->priv;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	if (IS_CAP_DBDC(ctrl->chip_cap) && pAd->CommonCfg.dbdc_mode) {
		os_zero_mem(&BandInfoValue, sizeof(BCTRL_INFO_T));
		BandInfoValue.DBDCEnable = pAd->CommonCfg.dbdc_mode;
		pEntry = &BandInfoValue.BctrlEntries[0];
		/*fix to bind band 0 currently*/
		rcFillEntry(pEntry, DBDC_TYPE_REPEATER, rdev->pRadioCtrl->BandIdx, ReptIdx);
		BandInfoValue.TotalNum++;
		ret = AsicSetDbdcCtrl(pAd, &BandInfoValue);

		if (ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error for conifgure dbdc, ret = %d !\n", ret);
	}

#endif /*DBDC_MODE*/
	return ret;
}


/*
*
*/
INT32 RcUpdateWmmEntry(struct radio_dev *rdev, struct hdev_obj *obj, UINT32 WmmIdx)
{
	INT32 ret = 0;
#ifdef DBDC_MODE
	BCTRL_ENTRY_T *pEntry;
	BCTRL_INFO_T BandInfoValue;
	struct hdev_ctrl *ctrl = rdev->priv;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	if (obj && IS_CAP_DBDC(ctrl->chip_cap) && pAd->CommonCfg.dbdc_mode) {
		os_zero_mem(&BandInfoValue, sizeof(BCTRL_INFO_T));
		BandInfoValue.DBDCEnable = pAd->CommonCfg.dbdc_mode;
		pEntry = &BandInfoValue.BctrlEntries[0];
		/*fix to bind band 0 currently*/
		obj->WmmIdx = WmmIdx;
		rcFillEntry(pEntry, DBDC_TYPE_WMM, rdev->pRadioCtrl->BandIdx, WmmIdx);
		BandInfoValue.TotalNum++;
		ret = AsicSetDbdcCtrl(pAd, &BandInfoValue);

		if (ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error for conifgure dbdc, ret = %d !\n", ret);
	}

#endif /*DBDC_MODE*/
	return ret;
}


/*
* Used for DATA path can get
*/
/*
*
*/
UINT32 RcGetMgmtQueueIdx(struct hdev_obj *obj, enum PACKET_TYPE pkt_type)
{
	struct radio_dev *rdev = obj->rdev;
	struct hdev_ctrl *ctrl = rdev->priv;

	if (pkt_type == TX_ALTX) {
		if (rdev->pRadioCtrl && rdev->pRadioCtrl->BandIdx)
			return TxQ_IDX_ALTX1;

		return TxQ_IDX_ALTX0;
	} else {
		return asic_get_hwq_from_ac((RTMP_ADAPTER *)ctrl->priv, obj->WmmIdx, QID_AC_BE);
	}
}



/*
*
*/
UINT32 RcGetBcnQueueIdx(struct hdev_obj *obj)
{
	struct radio_dev *rdev = obj->rdev;

	if (rdev->pRadioCtrl->BandIdx)
		return TxQ_IDX_BCN1;

	return TxQ_IDX_BCN0;
}

/*
*
*/
UINT32 RcGetWmmIdx(struct hdev_obj *obj)
{
	struct radio_dev *rdev = obj->rdev;

	if (rdev->pRadioCtrl)
		return obj->WmmIdx;

	return 0;
}


#ifdef DOT11_HE_AX
struct pe_control *rc_get_pe_ctrl(struct radio_dev *r_dev)
{
	struct hdev_ctrl *h_ctrl = (struct hdev_ctrl *)r_dev->priv;
	struct radio_control *r_ctrl = r_dev->pRadioCtrl;
	UINT8 band_idx = r_ctrl->BandIdx;

	return &h_ctrl->HwResourceCfg.PhyCtrl[band_idx].pe_ctrl;
}
#endif

static void rc_set_radio_default(struct radio_control *ctrl, UCHAR rf_band_cap)
{
	ctrl->PhyMode = rcGetDefaultPhyMode(rf_band_cap);
	ctrl->Channel = rcGetDefaultChannel(ctrl->PhyMode);
	ctrl->cur_rfic_type = get_cur_rfic_by_phymode(ctrl->PhyMode);
}



/*
*
*/
VOID rc_radio_init(struct hdev_ctrl *ctrl, UCHAR rfic, UCHAR dbdc_mode)
{
	RADIO_CTRL *radio_ctrl = NULL;
	RTMP_PHY_CTRL *phy_ctrl = NULL;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;
	struct wifi_dev *wdev = NULL;
	UCHAR i;
#ifdef DOT11_HE_AX
	UINT8 max_nss;
	UINT8 max_ru_num;
#endif

	pHwResource->concurrent_bands =
		(IS_CAP_DBDC(ctrl->chip_cap) && dbdc_mode) ? 2 : 1;
	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"DBDC MODE=%d, ConcurrentBand=%d\n",
			 dbdc_mode, pHwResource->concurrent_bands);

	/*Allocate PhyCtrl for HwResource*/
	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		phy_ctrl =  &pHwResource->PhyCtrl[i];
		radio_ctrl = &phy_ctrl->RadioCtrl;
		os_zero_mem(radio_ctrl, sizeof(*radio_ctrl));
		if (i == 0) {
			wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
		} else {
			wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;
		}
		if (WMODE_CAP_6G(wdev->PhyMode))
			phy_ctrl->rf_band_cap = RFIC_6GHZ;
		else
			phy_ctrl->rf_band_cap = rcGetBandSupport(ctrl, dbdc_mode, i);
		radio_ctrl->BandIdx = i;
		radio_ctrl->ExtCha = EXTCHA_NOASSIGN;
		rc_set_radio_default(radio_ctrl, phy_ctrl->rf_band_cap);
#ifdef DOT11_HE_AX
		max_nss = ctrl->chip_cap.mcs_nss.max_nss[i];
		max_ru_num = ctrl->chip_cap.mcs_nss.max_24g_ru_num;
		if (((phy_ctrl->rf_band_cap) & RFIC_5GHZ) || ((phy_ctrl->rf_band_cap) & RFIC_6GHZ))
			max_ru_num = ctrl->chip_cap.mcs_nss.max_5g_ru_num;
		init_default_ppe(&phy_ctrl->pe_ctrl.pe_info, max_nss, max_ru_num);
#endif
		radio_ctrl->CurStat = PHY_IDLE;
#ifdef TXBF_SUPPORT
		if (pHwResource->concurrent_bands == 1)
			radio_ctrl->IsBfBand = 1;

#endif /*TXBF_SUPPORT*/
#ifdef GREENAP_SUPPORT
		radio_ctrl->bGreenAPActive = FALSE;
#endif /* GREENAP_SUPPORT */
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"radio_ctrl=%p,Band=%d,rfcap=%d,channel=%d,PhyMode=%d extCha=0x%x\n",
				 radio_ctrl, i, phy_ctrl->rf_band_cap,
				 radio_ctrl->Channel, radio_ctrl->PhyMode, radio_ctrl->ExtCha);
		HdevInit(ctrl, i, radio_ctrl);
	}

	RcUpdateBandCtrl(ctrl);
}

VOID rc_radio_exit(struct hdev_ctrl *ctrl, UCHAR dbdc_mode)
{
	struct _HD_RESOURCE_CFG *hw_res = &ctrl->HwResourceCfg;
	RTMP_PHY_CTRL *phy_ctrl = NULL;
	UINT8 i;

	hw_res->concurrent_bands =
		(IS_CAP_DBDC(ctrl->chip_cap) && dbdc_mode) ? 2 : 1;

	for (i = 0; i < hw_res->concurrent_bands; i++) {
		phy_ctrl =  &hw_res->PhyCtrl[i];
#ifdef DOT11_HE_AX
		os_free_mem(phy_ctrl->pe_ctrl.pe_info.pe_thld);
#endif
	}
}

/*
*
*/
VOID RcReleaseBandForObj(struct hdev_ctrl *ctrl, struct hdev_obj *obj)
{
	struct radio_dev *rdev = NULL;

	if (!obj) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "can't find obj\n");
		return;
	}

	rdev = obj->rdev;
	if (!rdev) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "rdev is null!!!\n");
		return;
	}

	ReleaseOmacIdx(ctrl, obj->Type, rdev, obj->OmacIdx);
	if (rdev) {
		if (obj->bWmmAcquired)
			wmm_ctrl_release_entry(obj);

		HdevObjDel(rdev, obj);
		NdisFreeSpinLock(&obj->RefCntLock);
	}

	return;
}


/*
* Refine when OmacIdx is ready
*/
struct radio_dev *RcAcquiredBandForObj(
	struct hdev_ctrl *ctrl,
	struct hdev_obj *obj,
	UCHAR obj_idx,
	USHORT PhyMode,
	UCHAR Channel,
	USHORT ObjType)
{
	struct radio_dev *rdev = NULL;
	UCHAR is_default = 0;
	RADIO_CTRL *pRadioCtrl = NULL;

	rdev = rcGetHdevByPhyMode(ctrl, PhyMode, Channel, ObjType);

	/*can't get hdev by phymode, use default band*/
	if (!rdev) {
		rdev = &ctrl->rdev[0];
		if (WMODE_CAP_5G(PhyMode)) {
			MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "[%s] rdev received NULL in 5G mode\n", __func__);
		}
		is_default = 1;
	}

	/*Don't release, if rdev changed will return false*/
	if (obj->state == HOBJ_STATE_USED && rdev != obj->rdev) {
		/* RcReleaseBandForObj(ctrl, obj);*/
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"can't change to BandIdx:%d,PhyMode=%d,Channel=%d,from BandIdx:%d,PhyMode=%d,Channel=%d\n",
			rdev->pRadioCtrl->BandIdx, PhyMode, Channel, obj->rdev->pRadioCtrl->BandIdx,
			obj->rdev->pRadioCtrl->PhyMode, obj->rdev->pRadioCtrl->Channel);
		return NULL;
	}

	/*update phy mode for radio control*/
	pRadioCtrl = rdev->pRadioCtrl;
	/*Can get rdev. change phyCtrl to INUSED state*/
	if (pRadioCtrl->CurStat == PHY_IDLE)
		pRadioCtrl->CurStat = PHY_INUSE;
	/*if mixed mode*/
	if ((ObjType == WDEV_TYPE_STA) && (!WMODE_5G_ONLY(PhyMode) || !WMODE_2G_ONLY(PhyMode))) {
		pRadioCtrl->PhyMode = PhyMode;
	} else if (!is_default) {
		/*Make phymode of band should be the maxize*/
		rcUpdatePhyMode(rdev, PhyMode);
	}

	/*check tx_mode*/
	if ((ctrl->HwResourceCfg.txcmd_mode == HOBJ_TX_MODE_TXCMD) && (ObjType != WDEV_TYPE_ATE_STA) && (ObjType != WDEV_TYPE_SERVICE_TXD)) {
		obj->tx_mode = HOBJ_TX_MODE_TXCMD;
	}
	/*update hdev_obj information*/
	obj->Idx = obj_idx;
	obj->Type = ObjType;
	if (obj->state == HOBJ_STATE_NONE) {
		obj->OmacIdx = GetOmacIdx(ctrl, ObjType, rdev, obj_idx);
		HdevObjAdd(rdev, obj);
	}
	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"%s(): BandIdx:%d, PhyMode=%d,Channel=%d,OMACIDX=%d,pHdevObj=%p, tx_mode=%d\n",
		__func__, pRadioCtrl->BandIdx, pRadioCtrl->PhyMode, pRadioCtrl->Channel, obj->OmacIdx, obj, obj->tx_mode);
	RcUpdateBandCtrl(ctrl);
	NdisAllocateSpinLock(NULL, &obj->RefCntLock);
	return rdev;
}



/*
*
*/
struct radio_dev *RcGetHdevByChannel(struct hdev_ctrl *ctrl, UCHAR Channel)
{
	struct radio_dev *rdev;
	UCHAR i = 0;

	for (i = 0 ; i < ctrl->HwResourceCfg.concurrent_bands ; i++) {
		rdev = &ctrl->rdev[i];
		if (rdev->pRadioCtrl->Channel == Channel)
			return rdev;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"%s():Err! Update PhyMode failed, no phyctrl support this channel=%d!\n",
		__func__, Channel);
	return NULL;
}



/*
*
*/
INT32 RcUpdateChannel(struct radio_dev *rdev, UCHAR Channel, BOOLEAN scan)
{
	INT32 ret = 0;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;
#ifdef TR181_SUPPORT
	if ((pRadioCtrl->Channel != Channel) && (scan == 0)) {
		ULONG CurJiffies;

		NdisGetSystemUpTime(&CurJiffies);
		pRadioCtrl->CurChannelUpTime = jiffies_to_usecs(CurJiffies);
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "orig_chan=%d, new_chan=%d, CurChanUpTime=%u\n",
						pRadioCtrl->Channel, Channel, pRadioCtrl->CurChannelUpTime);
	}
#endif /*TR181_SUPPORT*/
	pRadioCtrl->Channel = Channel;
	pRadioCtrl->scan_state = scan;
	return ret;
}



/*
*
*/
INT32 RcUpdateRadio(struct radio_dev *rdev, UCHAR bw, UCHAR central_ch1, UCHAR control_ch2, UCHAR ext_cha, UCHAR rx_stream)
{
	INT32 ret = 0;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	pRadioCtrl->CentralCh = central_ch1;
	pRadioCtrl->Bw = bw;
	pRadioCtrl->Channel2 = control_ch2;
	pRadioCtrl->ExtCha = ext_cha;
	pRadioCtrl->rx_stream = rx_stream;
	return ret;
}


/*
*
*/
UCHAR RcUpdateBw(struct radio_dev *rdev, UCHAR Bw)
{
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	/*Legacy mode, only can support BW20*/
	if (!WMODE_CAP_N(pRadioCtrl->PhyMode) && Bw > BW_20)
		pRadioCtrl->Bw = BW_20;
	else if (!WMODE_CAP_AC(pRadioCtrl->PhyMode) && Bw > BW_40)
		pRadioCtrl->Bw = BW_40;
	else
		pRadioCtrl->Bw = Bw;

	return Bw;
}

/*
*
*/
INT32 RcUpdateExtCha(struct radio_dev *rdev, UCHAR ExtCha)
{
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	pRadioCtrl->ExtCha = ExtCha;
	return -1;
}

/*
*
*/
UCHAR RcGetExtCha(struct radio_dev *rdev)
{
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	return pRadioCtrl->ExtCha;
}

/*
*
*/
USHORT RcGetPhyMode(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->PhyMode;
}


/*
*
*/
UCHAR RcGetChannel(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->Channel;
}


/*
*
*/
UCHAR RcGetCentralCh(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->CentralCh;
}


/*
*
*/
UCHAR RcGetBandIdx(struct radio_dev *rdev)
{
	if (rdev && rdev->pRadioCtrl)
		return rdev->pRadioCtrl->BandIdx;
	else
		return 0;
}

/*
*
*/
PHY_STATUS RcGetRadioCurStat(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->CurStat;
}

/*
*
*/
VOID RcSetRadioCurStat(struct radio_dev *rdev, PHY_STATUS CurStat)
{
	rdev->pRadioCtrl->CurStat = CurStat;
}


/*
*
*/
UCHAR RcGetBw(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->Bw;
}


/*
*
*/
UCHAR RcGetBandIdxByRf(struct hdev_ctrl *ctrl, UCHAR RfIC)
{
	struct radio_dev *rdev = rcGetHdevByRf(ctrl, RfIC);

	if (rdev)
		return RcGetBandIdx(rdev);

	return 0;
}


/*
*
*/
struct radio_dev *RcGetBandIdxByBf(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;
	RADIO_CTRL *pRadioCtrl = NULL;
	UCHAR i;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		pRadioCtrl = &pHwResourceCfg->PhyCtrl[i].RadioCtrl;

		if (pRadioCtrl->IsBfBand)
			return &ctrl->rdev[i];
	}

	return NULL;
}


/*
*
*/
struct radio_dev *rc_init(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	UCHAR i;

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pPhyCtrl = &pHwResourceCfg->PhyCtrl[i];
		os_zero_mem(pPhyCtrl, sizeof(RTMP_PHY_CTRL));
	}

	return NULL;
}


/*
*
*/
VOID RcRadioShow(HD_RESOURCE_CFG *pHwResourceCfg)
{
	UCHAR i;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		MTWF_PRINT("band\t: %d,rfic: %d, cur_rfic: %d, bf_cap: %d\n",
		i, pHwResourceCfg->PhyCtrl[i].rf_band_cap, pHwResourceCfg->PhyCtrl[i].RadioCtrl.cur_rfic_type,
		pHwResourceCfg->PhyCtrl[i].RadioCtrl.IsBfBand ? TRUE:FALSE);
	}
}

/*
*
*/
BOOLEAN RcIsBfCapSupport(struct hdev_obj *obj)
{
	struct radio_dev *rdev = obj->rdev;
	RADIO_CTRL *rc;

	if (!rdev)
		return FALSE;

	rc = rdev->pRadioCtrl;
	return rc->IsBfBand;
}

/*
*
*/
BOOLEAN rc_radio_equal(struct radio_dev *dev, struct freq_oper *oper)
{
	RADIO_CTRL *rc = dev->pRadioCtrl;

	/*if previous action is for scan, always allow to switch channel*/
	if (rc->scan_state == TRUE)
		return FALSE;

	if (rc->Channel != oper->prim_ch)
		return FALSE;

	if (rc->Bw != oper->bw)
		return FALSE;

	if (rc->CentralCh != oper->cen_ch_1)
		return FALSE;

	if (rc->Channel2 != oper->cen_ch_2)
		return FALSE;

	if (rc->rx_stream != oper->rx_stream)
		return FALSE;

	return TRUE;
}

/*
*
*/
BOOLEAN rc_radio_res_acquire(struct radio_dev *dev, struct radio_res *res)
{
	RADIO_CTRL *rc = dev->pRadioCtrl;

	if (rc->CurStat != PHY_INUSE)
		return FALSE;

	return TRUE;
}
