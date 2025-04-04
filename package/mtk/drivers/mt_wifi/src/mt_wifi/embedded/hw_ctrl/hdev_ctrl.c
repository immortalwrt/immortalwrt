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
	hdev_ctrl.c
*/
#include	"rt_config.h"
#include "hdev/hdev.h"
#include "mgmt/be_internal.h"

/*
* local function
*/
#ifdef DBDC_MODE
static VOID hcGetBandTypeName(UCHAR Type, UCHAR *Str, UINT32 max_len)
{
	INT ret;
	switch (Type) {
	case DBDC_TYPE_WMM:
		ret = snprintf(Str, max_len, "%s", "WMM");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Str snprintf error\n");
			return;
		}
		break;

	case DBDC_TYPE_MGMT:
		ret = snprintf(Str, max_len, "%s", "MGMT");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Str snprintf error\n");
			return;
		}
		break;

	case DBDC_TYPE_BSS:
		ret = snprintf(Str, max_len, "%s", "BSS");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Str snprintf error\n");
			return;
		}
		break;

	case DBDC_TYPE_MBSS:
		ret = snprintf(Str, max_len, "%s", "MBSS");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Str snprintf error\n");
			return;
		}
		break;

	case DBDC_TYPE_REPEATER:
		ret = snprintf(Str, max_len, "%s", "REPEATER");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Str snprintf error\n");
			return;
		}
		break;

	case DBDC_TYPE_MU:
		ret = snprintf(Str, max_len, "%s", "MU");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Str snprintf error\n");
			return;
		}
		break;

	case DBDC_TYPE_BF:
		ret = snprintf(Str, max_len, "%s", "BF");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Str snprintf error\n");
			return;
		}
		break;

	case DBDC_TYPE_PTA:
		ret = snprintf(Str, max_len, "%s", "PTA");
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Str snprintf error\n");
			return;
		}
		break;
	}
}
#endif

/*
*
*/
VOID hdev_resource_init(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = ctrl->priv;

	/*initial hardware resource*/
	HdevHwResourceInit(ctrl);
	/*initial resource*/
	ctrl->cookie = ad->OS_Cookie;
	ctrl->mcu_ctrl = &ad->MCUCtrl;
}

/*
 *
*/
/*Only this function can use pAd*/
INT32 hdev_ctrl_init(RTMP_ADAPTER *pAd, INT type)
{
	struct hdev_ctrl  *ctrl = NULL;
	UINT32  ret;

	ret  =  os_alloc_mem(NULL, (UCHAR **)&ctrl, sizeof(struct hdev_ctrl));

	if (ctrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(): Allocate Hardware device Configure  fail!!\n");
		return -1;
	}

	os_zero_mem(ctrl, sizeof(struct hdev_ctrl));
	ctrl->priv  = (VOID *)pAd;
	pAd->hdev_ctrl = (VOID *)ctrl;

	hif_core_ops_register(ctrl, type);
	return 0;
}

/*
 *
*/
VOID hdev_ctrl_exit(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	/*exist hw resource*/
	HdevHwResourceExit(ctrl);
	/*exist hdevcfg*/
	pAd->hdev_ctrl = NULL;
	os_free_mem(ctrl);
}

/*
 *
*/
VOID HcDevExit(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	UCHAR i;
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++)
		HdevExit(ctrl, i);
}

/*
*
*/
INT32 HcAcquireRadioForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 ret = HC_STATUS_OK;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;
	struct hdev_obj *obj = wdev->pHObj;
#ifdef GREENAP_SUPPORT
	greenap_suspend(pAd, GREENAP_REASON_ACQUIRE_RADIO_FOR_WDEV);
#endif /* GREENAP_SUPPORT */
	rdev = RcAcquiredBandForObj(ctrl, obj, wdev->wdev_idx,
				    wdev->PhyMode, wdev->channel, wdev->wdev_type);
	if (!rdev)
		return HC_STATUS_FAIL;

	/*correct wdev configure, if configure is not sync with hdev */
	if (!wmode_band_equal(wdev->PhyMode, RcGetPhyMode(rdev))) {
		wdev->PhyMode = RcGetPhyMode(rdev);
		wdev->channel = RcGetChannel(rdev);
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	BuildChannelListEx(pAd, wdev);
#else
	BuildChannelList(pAd, wdev);
#endif
	/*temporal set, will be repaced by HcGetOmacIdx*/
	wdev->OmacIdx = obj->OmacIdx;
	/* Initialize the pDot11H of wdev */
	UpdateDot11hForWdev(wdev->sys_handle, wdev, TRUE);
	/*re-init operation*/
	wlan_operate_init(wdev);
#ifdef GREENAP_SUPPORT
	greenap_resume(pAd, GREENAP_REASON_ACQUIRE_RADIO_FOR_WDEV);
#endif /* GREENAP_SUPPORT */
	return ret;
}

/*
*
*/
INT32 HcReleaseRadioForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 ret = 0;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct hdev_obj *obj = wdev->pHObj;

	OS_SPIN_LOCK(&obj->RefCntLock);

	if (obj->RefCnt > 0) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "there are other link reference the Obj\n");
		OS_SPIN_UNLOCK(&obj->RefCntLock);
		return ret;
	}

	OS_SPIN_UNLOCK(&obj->RefCntLock);
	RcReleaseBandForObj(ctrl, obj);
	return ret;
}

/*
*
*/
UCHAR HcGetBandByWdev(struct wifi_dev *wdev)
{
	UCHAR BandIdx = 0;
	struct hdev_obj *obj = wdev->pHObj;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (hdev_obj_state_ready(obj)) {
		if (obj->rdev)
			BandIdx = RcGetBandIdx(obj->rdev);
		else
			BandIdx = 0;
	}
	else {
		if ((ad) && (ad->CommonCfg.dbdc_mode)) {
			if (WMODE_CAP_5G(wdev->PhyMode))
				BandIdx = DBDC_BAND1;
			else
				BandIdx = DBDC_BAND0;
		} else
			BandIdx = 0;
	}

	return BandIdx;
}

/*
*
*/
VOID HcSetRadioCurStatByWdev(struct wifi_dev *wdev, PHY_STATUS CurStat)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (hdev_obj_state_ready(obj))
		RcSetRadioCurStat(obj->rdev, CurStat);
	else
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"obj is not ready!!\n");
}

#ifdef DOT11_HE_AX
struct pe_control *hc_get_pe_ctrl(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct pe_control *pe_ctrl = NULL;

	if (hdev_obj_state_ready(obj))
		pe_ctrl = rc_get_pe_ctrl(obj->rdev);
	else
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"obj is not ready!!\n");
	return pe_ctrl;
}
#endif

/*
*
*/
VOID HcSetRadioCurStatByChannel(RTMP_ADAPTER *pAd, UCHAR Channel, PHY_STATUS CurStat)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	rdev = RcGetHdevByChannel(ctrl, Channel);

	if (!rdev) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): no hdev parking on channel:%d !!!\n",
				 __func__, Channel);
		return;
	}

	RcSetRadioCurStat(rdev, CurStat);
}

/*
*
*/
VOID HcSetAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResourceCfg =  &ctrl->HwResourceCfg;
	struct radio_dev *rdev = NULL;
	UCHAR i;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];
		rdev->pRadioCtrl->CurStat = PHY_RADIOOFF;
	}
}

/*
*
*/
VOID HcSetAllSupportedBandsRadioOn(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResourceCfg =  &ctrl->HwResourceCfg;
	struct radio_dev *rdev = NULL;
	UCHAR i;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];
		rdev->pRadioCtrl->CurStat = PHY_INUSE;
	}
}

/*
*
*/
BOOLEAN IsHcRadioCurStatOffByWdev(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"%s(): wdev_idx %d obj is not ready, return TRUE !!!\n",
			__func__, wdev->wdev_idx);
		return TRUE;
	}

	if (!obj->rdev) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"%s(): no hdev parking on wdev_idx:%d!!!\n",
			__func__, wdev->wdev_idx);
		return TRUE;
	}

	if (RcGetRadioCurStat(obj->rdev) == PHY_RADIOOFF)
		return TRUE;
	else
		return FALSE;
}

/*
*
*/
BOOLEAN IsHcRadioCurStatOffByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	rdev = RcGetHdevByChannel(ctrl, Channel);

	if (!rdev) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): no hdev parking on channel:%d!!!\n",
				 __func__, Channel);
		return TRUE;
	}

	if (RcGetRadioCurStat(rdev) == PHY_RADIOOFF)
		return TRUE;
	else
		return FALSE;
}

/*
*
*/
BOOLEAN IsHcAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResourceCfg =  &ctrl->HwResourceCfg;
	struct radio_dev *rdev = NULL;
	UCHAR i;
	BOOLEAN AllSupportedBandsRadioOff = TRUE;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];

		if ((rdev->pRadioCtrl->CurStat == PHY_INUSE) && (rdev->pRadioCtrl->CurStat != PHY_RADIOOFF)) {
			AllSupportedBandsRadioOff = FALSE;
			break;
		}
	}

	return AllSupportedBandsRadioOff;
}

#ifdef GREENAP_SUPPORT
/*
*
*/
VOID HcSetGreenAPActiveByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bGreenAPActive)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	if (!ctrl)
		return;

	rdev = &ctrl->rdev[BandIdx];

	if (!rdev)
		return;

	rdev->pRadioCtrl->bGreenAPActive = bGreenAPActive;
}

/*
*
*/
BOOLEAN IsHcGreenAPActiveByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	if (!ctrl)
		return FALSE;

	rdev = &ctrl->rdev[BandIdx];

	if (!rdev)
		return FALSE;

	return rdev->pRadioCtrl->bGreenAPActive;
}

/*
*
*/
BOOLEAN IsHcGreenAPActiveByWdev(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct radio_dev *rdev = NULL;

	if (!hdev_obj_state_ready(obj))
		return FALSE;

	rdev = obj->rdev;

	return rdev->pRadioCtrl->bGreenAPActive;
}
#endif /* GREENAP_SUPPORT */

/*
*
*/
UCHAR HcGetChannelByBf(RTMP_ADAPTER *pAd)
{
	struct radio_dev *rdev = RcGetBandIdxByBf(pAd->hdev_ctrl);

	if (rdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "no hdev can support beamform!\n");
		return 0;
	}

	return rdev->pRadioCtrl->Channel;
}

/*
*
*/
BOOLEAN HcIsBfCapSupport(struct wifi_dev *wdev)
{
	if (wdev && wdev->pHObj && hdev_obj_state_ready(wdev->pHObj))
		return RcIsBfCapSupport(wdev->pHObj);
	else
		return FALSE;
}

#ifdef MAC_REPEATER_SUPPORT
/*
*
*/
INT32 HcAddRepeaterEntry(struct wifi_dev *wdev)
{
	INT32 ret = 0;
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return HC_STATUS_FAIL;
	}

	/*Acquire Repeater OMACIdx*/
	OcAddRepeaterEntry(obj, wdev->func_idx);
	RcUpdateRepeaterEntry(obj->rdev, wdev->func_idx);
	return ret;
}

/*
*
*/
INT32 HcDelRepeaterEntry(struct wifi_dev *wdev)
{
	INT32 ret = 0;
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return HC_STATUS_FAIL;
	}

	/*Acquire Repeater OMACIdx*/
	OcDelRepeaterEntry(obj, wdev->func_idx);
	return ret;
}

/*
*
*/
UCHAR HcGetRepeaterOmac(struct wifi_dev *wdev)
{
	HD_REPT_ENRTY *pHReptEntry = NULL;
	UCHAR ReptOmacIdx = 0xff;

	if (wdev && wdev->wdev_type == WDEV_TYPE_REPEATER) {
		pHReptEntry = OcGetRepeaterEntry(wdev->pHObj, wdev->func_idx);

		if (pHReptEntry)
			ReptOmacIdx = pHReptEntry->ReptOmacIdx;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s(): Get ReptOmacIdx: %d!\n", __func__, ReptOmacIdx);
	return ReptOmacIdx;
}
#endif /*#MAC_REPEATER_SUPPORT*/

/*
*
*/
INT32 hc_radio_init(struct _RTMP_ADAPTER *pAd, UCHAR rfic, UCHAR dbdc_mode)
{
	INT32 ret = 0;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	rc_radio_init(ctrl, rfic, dbdc_mode);

	return ret;
}

INT32 hc_radio_exit(struct _RTMP_ADAPTER *pAd, UCHAR dbdc_mode)
{
	INT32 ret = 0;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	rc_radio_exit(ctrl, dbdc_mode);

	return ret;
}

/*
*
*/
INT32 HcUpdateCsaCntByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	INT32 ret = 0;
	UCHAR band_idx = DBDC_BAND0;
	struct radio_dev *rdev = NULL;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct hdev_obj *obj;
	struct wifi_dev *wdev;
	struct DOT11_H *pDot11h = NULL;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "Channel(%d).\n", Channel);

	rdev = RcGetHdevByChannel(ctrl, Channel);
	if (!rdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Update Channel %d faild, not support this RF\n",
				  Channel);
		return -1;
	}

	band_idx = rdev->pRadioCtrl->BandIdx;

#ifdef ZERO_LOSS_CSA_SUPPORT
	if (pAd->Zero_Loss_Enable) {
		/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s():\n", __FUNCTION__));*/
		pAd->Dot11_H[band_idx].ChnlSwitchState = SET_CHANNEL_COMMAND;
	}
#endif /*ZERO_LOSS_CSA_SUPPORT*/

	DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
		wdev = pAd->wdev_list[obj->Idx];

		if (wdev == NULL || !WDEV_WITH_BCN_ABILITY(wdev))
			continue;

		pDot11h = wdev->pDot11_H;

		if (pDot11h == NULL) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pDot11h is NULL!\n");
			continue;
		}

		if (pDot11h->RDMode != RD_SILENCE_MODE
			|| ((Channel <= 14) && (pAd->CommonCfg.ChannelSwitchFor2G.CHSWMode == CHANNEL_SWITCHING_MODE))) {
			pAd->ApCfg.set_ch_async_flag = TRUE;
			pDot11h->wdev_count++;
			wdev->csa_count = pDot11h->CSPeriod;
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_CSA);
		}
	}

	/*Send CSA cmd to FW and Wait CSA Event*/
	pDot11h = &pAd->Dot11_H[band_idx];
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"\x1b[41m pAd->Dot11_H[%d] is NULL !!\x1b[m\n", band_idx);
		return -1;
	} else if (pDot11h->wdev_count != 0) {
		RTMPSetTimer(&pDot11h->CSAEventTimer, 4000);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			"CSAEventTimer start(wdev_counter=%d)\n", pDot11h->wdev_count);
	}
#ifdef ZERO_LOSS_CSA_SUPPORT
	pAd->chan_switch_time[0] = jiffies_to_msecs(jiffies);
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	return ret;
}

#ifdef DBDC_MODE
/*
*
*/
VOID HcShowBandInfo(RTMP_ADAPTER *pAd)
{
	UINT32 i;
	BCTRL_INFO_T BctrlInfo;
	BCTRL_ENTRY_T *pEntry = NULL;
	CHAR TempStr[16] = "";

	os_zero_mem(&BctrlInfo, sizeof(BCTRL_INFO_T));
	AsicGetDbdcCtrl(pAd, &BctrlInfo);
	MTWF_PRINT("\tDbdcEnable: %d\n", BctrlInfo.DBDCEnable);

	for (i = 0; i < BctrlInfo.TotalNum; i++) {
		pEntry = &BctrlInfo.BctrlEntries[i];
		hcGetBandTypeName(pEntry->Type, TempStr, sizeof(TempStr));

		if (pEntry->Type != DBDC_TYPE_MBSS)
			MTWF_PRINT("\t(%s,%d): Band %d\n", TempStr, pEntry->Index, pEntry->BandIdx);
		else
			MTWF_PRINT("\t(%s,0-%d): Band %d\n", TempStr, pEntry->Index+1, pEntry->BandIdx);
	}
}
#endif

VOID HcShowChCtrlInfo(struct _RTMP_ADAPTER *pAd)
{
	UCHAR BandIdx, ChIdx;
	CHANNEL_CTRL *pChCtrl = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev == NULL)
		MTWF_PRINT("Get Wdev Fail!");
	else {
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		if (pChCtrl->ChListNum == 0) {
			MTWF_PRINT("\x1b[1;33mBandIdx = %d\x1b[m, ChannelListNum = %d (it is not available)\n ", BandIdx, pChCtrl->ChListNum);
		} else {
			MTWF_PRINT("\x1b[1;33mBandIdx = %d\x1b[m, ChannelListNum = %d\nChGrpABandEn = %d\nChannel list information:\n",
					BandIdx,
					pChCtrl->ChListNum,
					pChCtrl->ChGrpABandEn);
#ifdef CONFIG_6G_SUPPORT
			if (WMODE_CAP_6G(wdev->PhyMode))
				MTWF_PRINT("Channel   Pwr0/1   Flags   DFSEnable PSC\n");
			else
				MTWF_PRINT("Channel   Pwr0/1   Flags   DFSEnable\n");
#else
			MTWF_PRINT("Channel   Pwr0/1   Flags   DFSEnable\n");
#endif
			for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
#ifdef CONFIG_6G_SUPPORT
				if (WMODE_CAP_6G(wdev->PhyMode))
					MTWF_PRINT("#%-7d%4d/%d%8x%6d%10d\n",
						pChCtrl->ChList[ChIdx].Channel,
						pChCtrl->ChList[ChIdx].Power,
						pChCtrl->ChList[ChIdx].Power2,
						pChCtrl->ChList[ChIdx].Flags,
						pChCtrl->ChList[ChIdx].DfsReq,
						pChCtrl->ChList[ChIdx].PSC_Ch);
				else
					MTWF_PRINT("#%-7d%4d/%d%8x%6d\n",
						pChCtrl->ChList[ChIdx].Channel,
						pChCtrl->ChList[ChIdx].Power,
						pChCtrl->ChList[ChIdx].Power2,
						pChCtrl->ChList[ChIdx].Flags,
						pChCtrl->ChList[ChIdx].DfsReq);

#else
				MTWF_PRINT("#%-7d%4d/%d%8x%6d\n",
					pChCtrl->ChList[ChIdx].Channel,
					pChCtrl->ChList[ChIdx].Power,
					pChCtrl->ChList[ChIdx].Power2,
					pChCtrl->ChList[ChIdx].Flags,
					pChCtrl->ChList[ChIdx].DfsReq);
#endif
			}
		}
	}
}

#ifdef GREENAP_SUPPORT
/*
 *
 */
VOID HcShowGreenAPInfo(RTMP_ADAPTER *pAd)
{
	greenap_show(pAd);
}
#endif /* GREENAP_SUPPORT */

/*
*
*/
void hc_show_edca_info(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	wmm_ctrl_show_entry(&ctrl->HwResourceCfg.wmm_ctrl);
}

/*
*
*/
void hc_show_radio_info(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	HdevCfgShow(ctrl);
}

/*
*
*/
void hc_show_hdev_obj(struct wifi_dev *wdev)
{
	if (!hdev_obj_state_ready(wdev->pHObj))
		return;

	HdevObjShow(wdev->pHObj);
}

/*
*
*/
void hc_set_txcmd_mode(VOID *ctrl, UCHAR txcmd_mode)
{
	struct hdev_ctrl *hctrl = (struct hdev_ctrl *) ctrl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ctrl);

	if (txcmd_mode == HOBJ_TX_MODE_TXCMD && (cap->asic_caps & fASIC_CAP_TXCMD)) {
		hctrl->HwResourceCfg.txcmd_mode = HOBJ_TX_MODE_TXCMD;
	}
}

/*
*
*/
BOOLEAN HcAcquiredEdca(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, EDCA_PARM *pEdca)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct wmm_entry *entry;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return FALSE;
	}

	entry = wmm_ctrl_acquire_entry(obj, pEdca);

	if (!entry)
		return FALSE;

	return TRUE;
}

/*
*
*/
VOID HcReleaseEdca(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}
	wmm_ctrl_release_entry(obj);
}

/*
*
*/
VOID HcSetEdca(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}
	wmm_ctrl_set_edca(obj);
}


#ifdef SW_CONNECT_SUPPORT
BOOLEAN HcAcquiredDummyObj(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR dbdc_idx;
	struct hdev_obj *obj = wdev->pHObj;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pHwResource->WtblCfg;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return FALSE;
	}

	dbdc_idx = RcGetBandIdx(obj->rdev);

	if (dbdc_idx < DBDC_BAND_NUM) {
		if (wdev->pDummy_obj == NULL) {
			if (pWtblCfg->dummy_wcid_obj[dbdc_idx].State == WTBL_STATE_SW_OCCUPIED) {
				if (pWtblCfg) {
					wdev->pDummy_obj = &pWtblCfg->dummy_wcid_obj[dbdc_idx];

					if (atomic_read(&(pWtblCfg->dummy_wcid_obj[dbdc_idx].ref_cnt)) == 0) {
						/* set default to OFDM 54M */
						pWtblCfg->dummy_wcid_obj[dbdc_idx].HTPhyMode.field.MODE = MODE_OFDM;
						pWtblCfg->dummy_wcid_obj[dbdc_idx].HTPhyMode.field.BW = BW_20;
						pWtblCfg->dummy_wcid_obj[dbdc_idx].HTPhyMode.field.MCS = MCS_RATE_54;
						pWtblCfg->dummy_wcid_obj[dbdc_idx].bFixedRateSet = TRUE;
					}

					atomic_inc(&(pWtblCfg->dummy_wcid_obj[dbdc_idx].ref_cnt));
				}
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "wdev=%d, wdev->pDummy_obj=%p is double acuired!\n",  wdev->wdev_idx, wdev->pDummy_obj);
			ASSERT(0);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "wdev->wdev_idx=%u, dbdc_idx=%u, wdev->pDummy_obj=%p\n", wdev->wdev_idx, dbdc_idx, wdev->pDummy_obj);

	if (!wdev->pDummy_obj)
		return FALSE;

	return TRUE;
}


VOID HcReleaseDummyObj(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR dbdc_idx;
	struct hdev_obj *obj = wdev->pHObj;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pHwResource->WtblCfg;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}

	dbdc_idx = RcGetBandIdx(obj->rdev);

	if (dbdc_idx < DBDC_BAND_NUM) {
		if (wdev->pDummy_obj) {
			wdev->pDummy_obj = NULL;
			if (atomic_read(&(pWtblCfg->dummy_wcid_obj[dbdc_idx].ref_cnt)) > 0)
				atomic_dec(&(pWtblCfg->dummy_wcid_obj[dbdc_idx].ref_cnt));

			if (atomic_read(&(pWtblCfg->dummy_wcid_obj[dbdc_idx].ref_cnt)) == 0)
				pWtblCfg->dummy_wcid_obj[dbdc_idx].bFixedRateSet = FALSE;
		}
	}
}
#endif /* SW_CONNECT_SUPPORT */


/*
*
*/
UCHAR HcGetOmacIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return 0xff;
	}
	return obj->OmacIdx;
}

/*
*  Need refine
*/


/*
* Only temporal usage, should remove when cmm_asic_xxx.c is not apply pAd
*/

UCHAR  HcGetChannelByRf(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UCHAR i;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
			return pHwResource->PhyCtrl[i].RadioCtrl.Channel;
	}

	return 0;
}

/*
* for Single Band Usage
*/
UCHAR  HcGetRadioChannel(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return pHwResource->PhyCtrl[0].RadioCtrl.Channel;
}

/*
*
*/
USHORT HcGetRadioPhyMode(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return pHwResource->PhyCtrl[0].RadioCtrl.PhyMode;
}

USHORT HcGetRadioPhyModeByBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return pHwResource->PhyCtrl[BandIdx].RadioCtrl.PhyMode;
}
/*
*
*/
UCHAR HcGetRadioRfIC(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	return ctrl->HwResourceCfg.PhyCtrl[0].rf_band_cap;
}

/*
*
*/
BOOLEAN  HcIsRfSupport(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UCHAR i;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
			return TRUE;
	}

	return FALSE;
}

/*
*
*/
BOOLEAN  HcIsRfRun(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	struct radio_dev *rdev;
	UCHAR i;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];

		/* do not change sequence due to 6GHz might include AC/GN then confused */
		if (WMODE_CAP_6G(rdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_6GHZ))
			return TRUE;
		else if (WMODE_CAP_2G(rdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_24GHZ))
			return TRUE;
		else if (WMODE_CAP_5G(rdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_5GHZ))
			return TRUE;
	}

	return FALSE;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
/*
*
*/
QLOAD_CTRL *HcGetQloadCtrlByRf(RTMP_ADAPTER *pAd, UINT32 RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UCHAR i;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
			return &pHwResource->PhyCtrl[i].QloadCtrl;
	}

	return 0;
}

/*
*
*/
VOID *hc_get_qload_by_wdev(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct hdev_ctrl *ctrl;
	HD_RESOURCE_CFG *hwres;
	struct radio_dev *rdev;
	RADIO_CTRL *r_ctrl;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return NULL;
	}

	ctrl = obj->h_ctrl;
	hwres = &ctrl->HwResourceCfg;
	rdev = obj->rdev;
	r_ctrl = rdev->pRadioCtrl;

	return &hwres->PhyCtrl[r_ctrl->BandIdx].QloadCtrl;
}

/*
*
*/
QLOAD_CTRL *HcGetQloadCtrl(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return &pHwResource->PhyCtrl[0].QloadCtrl;
}
#endif /*AP_QLOAD_SUPPORT*/

/*
*
*/
AUTO_CH_CTRL *HcGetAutoChCtrl(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource =  &ctrl->HwResourceCfg;

	return &pHwResource->PhyCtrl[0].AutoChCtrl;
}
#endif /*CONFIG_AP_SUPPORT*/
#ifdef CONFIG_AP_SUPPORT
AUTO_CH_CTRL *HcGetAutoChCtrlbyBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource =  &ctrl->HwResourceCfg;

	return &pHwResource->PhyCtrl[BandIdx].AutoChCtrl;
}
#endif

/*
*
*/
UCHAR HcGetBw(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return 0xff;
	}

	return RcGetBw(obj->rdev);
}

/*
*
*/
UINT32 HcGetMgmtQueueIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return TxQ_IDX_ALTX0;
	}

	return RcGetMgmtQueueIdx(obj, pkt_type);
}

/*
*
*/
UINT32 HcGetBcnQueueIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return TxQ_IDX_BCN0;
	}

	return RcGetBcnQueueIdx(obj);
}

/*
*
*/
UINT32 HcGetWmmIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return 0;
	}

	return RcGetWmmIdx(obj);
}

/*
*Use ChannelRange to get BandIdx
*When DBDC disabled, we return BAND0 as default
*When DBDC enabled, if channel > 14, we return BAND1,else we return BAND0
*/
UCHAR HcGetBandByChannelRange(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	UCHAR BandIdx = BAND0;
	BOOLEAN Is2GRun = FALSE;
	BOOLEAN Is5GRun = FALSE;
	BOOLEAN Is6GRun = FALSE;

	if (pAd->CommonCfg.dbdc_mode == FALSE)
		return BandIdx;

	Is2GRun = HcIsRfSupport(pAd, RFIC_24GHZ);
	Is5GRun = HcIsRfSupport(pAd, RFIC_5GHZ);
	Is6GRun = HcIsRfSupport(pAd, RFIC_6GHZ);
	if (Is2GRun && (Is5GRun || Is6GRun)) {
		if (Channel > 14)
			return BAND1;

		return BandIdx;
	}

	ASSERT(FALSE);
	return BandIdx;
}
/*
*
*/
UCHAR HcGetBandByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;
	UCHAR BandIdx;

	rdev = RcGetHdevByChannel(ctrl, Channel);

	if (!rdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"No hdev parking on channel:%d, just return a default band_idx 0!\n", Channel);
		return 0;
	}

	BandIdx = RcGetBandIdx(rdev);
	return BandIdx;
}

/*
*
*/
EDCA_PARM *HcGetEdca(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct wmm_entry *entry;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return NULL;
	}

	entry = wmm_ctrl_get_entry_by_idx(ctrl, obj->WmmIdx);

	if (entry)
		return &entry->edca;
	else
		return NULL;
}

/*
*
*/
VOID HcCrossChannelCheck(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel)
{
	USHORT PhyMode = wdev->PhyMode;
	UCHAR WChannel = wdev->channel;

	/*check channel is belong to differet band*/
	if (WMODE_CAP_6G(PhyMode) && Channel <= CHANNEL_6G_MAX && WChannel <= CHANNEL_6G_MAX)
		return;

	if (WMODE_CAP_5G(PhyMode) && Channel > 14 && WChannel > 14)
		return;

	if (WMODE_CAP_2G(PhyMode) && Channel <= 14 && WChannel <= 14)
		return;

	/*is mixed mode, change default channel and */
	if (!WMODE_5G_ONLY(PhyMode) || !WMODE_2G_ONLY(PhyMode) || !WMODE_6G_ONLY(PhyMode)) {
		/*update wdev channel to new band*/
		wdev->channel = Channel;
		/*need change to other band*/
		HcAcquireRadioForWdev(pAd, wdev);
	}

	return;
}

/*
 * Description:
 *
 * the function will check all enabled function,
 * check the bssid num is defined,
 *
 * preserve the group key wtbl num will be used.
 * then decide the max station number could be used.
 */
UINT16 HcGetMaxStaNum(RTMP_ADAPTER *pAd)
{
	return WtcGetMaxStaNum(pAd->hdev_ctrl);
}

UINT16 HcSetMaxStaNum(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	UCHAR BssidNum = 0, MSTANum = 0;
#ifdef CONFIG_AP_SUPPORT
	BssidNum = pAd->ApCfg.BssidNum;
#endif /*CONFIG_AP_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
	MSTANum = pAd->MSTANum;
#endif
	return WtcSetMaxStaNum(ctrl, BssidNum, MSTANum);
}

#ifdef SW_CONNECT_SUPPORT
BOOLEAN HcSetDummyWcidFixedRate(struct _RTMP_ADAPTER *pAd, UINT16 wcid, HTTRANSMIT_SETTING HTPhyMode)
{
	UINT8 i;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;

	if (!IS_SW_STA_ENABLED(pAd))
		return FALSE;

	for (i = 0; i < DBDC_BAND_NUM ; i++) {
		if (pWtblCfg->dummy_wcid_obj[i].State == WTBL_STATE_SW_OCCUPIED)
			if (wcid == pWtblCfg->dummy_wcid_obj[i].HwWcid) {
				pWtblCfg->dummy_wcid_obj[i].HTPhyMode.word = HTPhyMode.word;
				pWtblCfg->dummy_wcid_obj[i].bFixedRateSet = TRUE;
				return TRUE;
			}
	}

	return FALSE;
}

BOOLEAN HcIsDummyWcid(struct _RTMP_ADAPTER *pAd, UINT16 wcid)
{
	UINT8 i;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;

	if (!IS_SW_STA_ENABLED(pAd))
		return FALSE;

	for (i = 0; i < DBDC_BAND_NUM ; i++) {
		if (pWtblCfg->dummy_wcid_obj[i].State == WTBL_STATE_SW_OCCUPIED)
			if (wcid == pWtblCfg->dummy_wcid_obj[i].HwWcid)
				return TRUE;
	}

	return FALSE;
}

UINT16 HcGetDummyWcid(struct wifi_dev *wdev)
{
	return WtcGetDummyWcid(wdev);
}

UINT16 HcGetMaxStaNumSw(RTMP_ADAPTER *pAd)
{
	return WtcGetMaxStaNumSw(pAd->hdev_ctrl);
}


UINT16 HcGetSwWcid(RTMP_ADAPTER *pAd, UINT16 hw_wcid)
{
	ASSERT(pAd);
	return WtcGetSwWcid(pAd->hdev_ctrl, hw_wcid);
}


#endif /* SW_CONNECT_SUPPORT */

/*
*
*/
UINT16 HcAcquireGroupKeyWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return WCID_INVALID;
	}

#ifdef CONFIG_VLAN_GTK_SUPPORT
	if (wdev->tr_tb_idx == WCID_INVALID) {
		/* for non-vlan dev */
		wdev->tr_tb_idx = WtcAcquireGroupKeyWcid(pAd->hdev_ctrl, obj
#ifdef SW_CONNECT_SUPPORT
							, &(wdev->hw_bmc_wcid)
#endif /* SW_CONNECT_SUPPORT */
		);
		return wdev->tr_tb_idx;
	} else {
		/* for vlan_dev, the return value will be stored to vlan_gtk_info, so just return a bmc_idx */
		return WtcAcquireGroupKeyWcid(pAd->hdev_ctrl, obj);
	}
#else
	wdev->tr_tb_idx = WtcAcquireGroupKeyWcid(pAd->hdev_ctrl, obj
#ifdef SW_CONNECT_SUPPORT
						, &(wdev->hw_bmc_wcid)
#endif /* SW_CONNECT_SUPPORT */
	);

	return wdev->tr_tb_idx;
#endif
}

/*
*
*/
VOID HcReleaseGroupKeyWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT16 wcid)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}

#ifdef CONFIG_VLAN_GTK_SUPPORT
	if (wdev->tr_tb_idx == wcid)
		wdev->tr_tb_idx = WtcReleaseGroupKeyWcid(pAd->hdev_ctrl, obj, wcid);
	else
		WtcReleaseGroupKeyWcid(pAd->hdev_ctrl, obj, wcid);
#else
	wdev->tr_tb_idx = WtcReleaseGroupKeyWcid(pAd->hdev_ctrl, obj, wcid);
#endif
}

/*
*
*/
UCHAR HcGetWcidLinkType(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	return WtcGetWcidLinkType(pAd->hdev_ctrl, wcid);
}


/*
*
*/
#ifdef SW_CONNECT_SUPPORT
UINT16 HcAcquireUcastWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN is_A4, BOOLEAN is_apcli, UINT16 *wcid_hw, BOOLEAN *bSw)
#else /* SW_CONNECT_SUPPORT */
UINT16 HcAcquireUcastWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN is_A4, BOOLEAN is_apcli)
#endif /* !SW_CONNECT_SUPPORT */
{
	struct hdev_obj *obj = wdev->pHObj;
	UINT16 FirstWcid = 1;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return WCID_INVALID;
	}
#ifdef SW_CONNECT_SUPPORT
	return WtcAcquireUcastWcid(pAd->hdev_ctrl, obj, FirstWcid, is_A4, is_apcli, wdev, wcid_hw, bSw);
#else /* SW_CONNECT_SUPPORT */
	return WtcAcquireUcastWcid(pAd->hdev_ctrl, obj, FirstWcid, is_A4, is_apcli);
#endif /* !SW_CONNECT_SUPPORT */
}


/*
*
*/
UINT16 HcReleaseUcastWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT16 wcid)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		if (wcid > 0) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				     "releasing wcid %d, hobj is not ready!\n", wcid);
		} else
			return WCID_INVALID;
	}

	return WtcReleaseUcastWcid(pAd->hdev_ctrl, obj, wcid);
}

/*
*
*/
VOID HcWtblRecDump(RTMP_ADAPTER *pAd)
{
	WtcRecDump(pAd->hdev_ctrl);
}


/*
*
*/
BOOLEAN HcIsRadioAcq(struct wifi_dev *wdev)
{
	return hdev_obj_state_ready(wdev->pHObj);
}

UCHAR HcGetAmountOfBand(struct _RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	return ctrl->HwResourceCfg.concurrent_bands;
}

static INT32 HcSuspendMSDUTx(struct radio_dev *rdev)
{
	INT32 ret = 0;
	struct hdev_obj *obj;
	struct hdev_ctrl *ctrl = rdev->priv;
	struct _RTMP_ADAPTER *ad = ctrl->priv;
	struct wifi_dev *wdev;

	/*update all of wdev*/
	DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
		wdev = ad->wdev_list[obj->Idx];
		RTMPSuspendMsduTransmission(wdev->sys_handle, wdev);
	}
#ifdef OFFCHANNEL_ZERO_LOSS
		ctrl->SuspendMsduTx[RcGetBandIdx(rdev)] = 1;
		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"():set suspended MSDU Tx\n");
		/*Disable Mac Level Tx enable = 0 |band<<4 */
		if (ad->ScanCtrl[RcGetBandIdx(rdev)].Num_Of_Channels == 1) {
			UCHAR Band = RcGetBandIdx(rdev);
			UCHAR enable = (Band<<4)|0;
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"():set skip Mac Tx \n");
			MtCmdSetMacTxEnable(ad, enable);
		}
#endif /*OFFCHANNEL_ZERO_LOSS*/

	return ret;
}

INT32 HcUpdateMSDUTxAllow(struct radio_dev *rdev)
{
	INT32 ret = 0;
	struct hdev_obj *obj;
	struct hdev_ctrl *ctrl = rdev->priv;
	struct _RTMP_ADAPTER *ad = ctrl->priv;
	struct wifi_dev *wdev;
#ifdef OFFCHANNEL_ZERO_LOSS
	BOOLEAN resume = 0;
#endif

	/*update all of wdev*/
	DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
		wdev = ad->wdev_list[obj->Idx];

		if (
			(wdev->channel == rdev->pRadioCtrl->Channel)
#ifdef OFFCHANNEL_ZERO_LOSS
				&& (ad->ScanCtrl[RcGetBandIdx(rdev)].state != OFFCHANNEL_SCAN_START)
#endif /*OFFCHANNEL_ZERO_LOSS*/
		) {
#ifdef OFFCHANNEL_ZERO_LOSS
				resume = 1;
#endif /*OFFCHANNEL_ZERO_LOSS*/

			RTMPResumeMsduTransmission(wdev->sys_handle, wdev);
		} else {
#ifdef OFFCHANNEL_ZERO_LOSS
			if (ad->ScanCtrl[RcGetBandIdx(rdev)].Num_Of_Channels == 1) {
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s():set skip Tx\n", __func__);
			}
			if (ctrl->SuspendMsduTx[RcGetBandIdx(rdev)] == 0)
#endif /*OFFCHANNEL_ZERO_LOSS*/
			RTMPSuspendMsduTransmission(wdev->sys_handle, wdev);
		}
	}
#ifdef OFFCHANNEL_ZERO_LOSS
	if ((resume == 1) && (ctrl->SuspendMsduTx[RcGetBandIdx(rdev)] == 1)) {
		ctrl->SuspendMsduTx[RcGetBandIdx(rdev)] = 0;
		if (ad->ScanCtrl[RcGetBandIdx(rdev)].Num_Of_Channels == 1) {
			UCHAR Band = RcGetBandIdx(rdev);
			UCHAR enable = (Band<<4)|1;
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"():set Mac Tx Enable \n");
			MtCmdSetMacTxEnable(ad, enable);
		}
	}
#endif
	return ret;
}

/*
*
*/
static VOID hc_radio_update(struct wifi_dev *wdev, struct radio_res *res)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct radio_dev  *rdev;
	struct freq_oper *oper = res->oper;
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif
	BOOLEAN scan = (res->reason == REASON_NORMAL_SCAN) ? TRUE:FALSE;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}
	rdev = obj->rdev;
#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(wdev);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef ZERO_LOSS_CSA_SUPPORT
	ad->chan_switch_time[7] = jiffies_to_msecs(jiffies);
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	HcSuspendMSDUTx(rdev);
#ifdef ZERO_LOSS_CSA_SUPPORT
	ad->chan_switch_time[8] = jiffies_to_msecs(jiffies);
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	AsicSwitchChannel(wdev->sys_handle, rdev->Idx, oper, scan);
	AsicSetBW(wdev->sys_handle, oper->bw, rdev->Idx);
#ifdef ZERO_LOSS_CSA_SUPPORT
	ad->chan_switch_time[12] = jiffies_to_msecs(jiffies);
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	RcUpdateRadio(rdev, oper->bw, oper->cen_ch_1, oper->cen_ch_2, oper->ext_cha, oper->rx_stream);
	RcUpdateChannel(rdev, oper->prim_ch, scan);
	/*after update channel resum tx*/
	HcUpdateMSDUTxAllow(rdev);
#ifdef ZERO_LOSS_CSA_SUPPORT
	ad->chan_switch_time[13] = jiffies_to_msecs(jiffies);
#endif /*ZERO_LOSS_CSA_SUPPORT*/

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DfsInitDedicatedScanStart(ad);
#endif
}

/*
*
*/
BOOLEAN hc_radio_res_request(struct wifi_dev *wdev, struct radio_res *res)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct radio_dev *rdev;
#if defined (MT_WOW_SUPPORT) || defined(OFFCHANNEL_ZERO_LOSS)
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif /*MT_WOW_SUPPORT*/
#ifdef OFFCHANNEL_ZERO_LOSS
	SCAN_CTRL *ScanCtrl = NULL;
	ScanCtrl = get_scan_ctrl_by_wdev(ad, wdev);
#endif
#ifdef ANTENNA_CONTROL_SUPPORT
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UINT8 BandIdx = HcGetBandByWdev(wdev);
#endif /* ANTENNA_CONTROL_SUPPORT */

#ifdef BW_VENDOR10_CUSTOM_FEATURE
	/* Sync SoftAp BW for Down Case */
	if (wdev->wdev_type == WDEV_TYPE_AP && wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s(): AP wdev=%d, Interface Down!\n", __func__, wdev->wdev_idx);
		return FALSE;
	}
#endif

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return FALSE;
	}

	rdev = obj->rdev;

	if (
#ifdef ANTENNA_CONTROL_SUPPORT
		(!pAd->bAntennaSetAPEnable[BandIdx]) &&
#endif /* ANTENNA_CONTROL_SUPPORT */
		rc_radio_equal(rdev, res->oper)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): radio is equal, prim_ch=%d, rx stream:%x!\n", __func__, res->oper->prim_ch, res->oper->rx_stream);
#ifdef OFFCHANNEL_ZERO_LOSS
		if (ScanCtrl->state == OFFCHANNEL_SCAN_START) {
			MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): Same channel. Suspending TX!\n", __func__);
			HcSuspendMSDUTx(rdev);
		}
		if (ScanCtrl->state == OFFCHANNEL_SCAN_COMPLETE) {
			MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): Same channel.Resuming TX!\n", __func__);
			HcUpdateMSDUTxAllow(rdev);
		}
#endif
		return TRUE;
	}

	if (rc_radio_res_acquire(rdev, res) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): can't acquire radio resource!\n", __func__);
		return FALSE;
	}

#ifdef MT_WOW_SUPPORT

	if (ad->WOW_Cfg.bWoWRunning) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[%s] WoW is running, skip!\n", __func__);
		return FALSE;
	}

#endif /*MT_WOW_SUPPORT*/
	/*update to radio resouce*/
	hc_radio_update(wdev, res);
	return TRUE;
}

/*
*
*/
UCHAR hc_reset_radio(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct radio_dev *rdev = NULL;
	struct freq_oper freq;
	struct radio_control *r_ctrl;
	UINT8 ch_band = CMD_CH_BAND_24G;
	UCHAR i;

	for (i = 0 ; i < ctrl->HwResourceCfg.concurrent_bands; i++) {
		os_zero_mem(&freq, sizeof(freq));
		rdev = &ctrl->rdev[i];
		r_ctrl = rdev->pRadioCtrl;
		/* do not change sequence due to 6GHz might include AC/GN then confused */
		if (WMODE_CAP_6G(r_ctrl->PhyMode))
			ch_band = CMD_CH_BAND_6G;
		else if (WMODE_CAP_5G(r_ctrl->PhyMode))
			ch_band = CMD_CH_BAND_5G;
		freq.ch_band = ch_band;
		freq.bw = r_ctrl->Bw;
		freq.prim_ch = r_ctrl->Channel;
		freq.cen_ch_1 = r_ctrl->CentralCh;
		freq.cen_ch_2 = r_ctrl->Channel2;
		AsicSwitchChannel(ad, i, &freq, FALSE);
	}
	return TRUE;
}

/*
*
*/
VOID hc_set_rrm_init(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	UINT8 ucTxPath = ad->Antenna.field.TxPath;
	UINT8 ucRxPath = ad->Antenna.field.RxPath;

#ifdef DBDC_MODE
	if (ad->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0) {
			ucTxPath = ad->dbdc_band0_tx_path;
			ucRxPath = ad->dbdc_band0_rx_path;
		} else {
			ucTxPath = ad->dbdc_band1_tx_path;
			ucRxPath = ad->dbdc_band1_rx_path;
		}
	}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (ad->bAntennaSetAPEnable[BandIdx]) {
			ucTxPath = ad->TxStream[BandIdx];
			ucRxPath = ad->RxStream[BandIdx];
		}
	}
#endif /* ANTENNA_CONTROL_SUPPORT */


	AsicSetTxStream(wdev->sys_handle, ucTxPath, OPMODE_AP, TRUE, band_idx);
	AsicSetRxStream(wdev->sys_handle, ucRxPath, band_idx);
}

/*
*
*/
INT  hc_oper_query_by_wdev(struct wifi_dev *wdev, struct freq_oper *oper)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	if (!op) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s: op NULL\n", __func__);
		return 0;
	}

	oper->bw = op->phy_oper.wdev_bw;
	oper->cen_ch_1 = op->phy_oper.cen_ch_1;
	oper->cen_ch_2 = op->phy_oper.cen_ch_2;
	oper->ext_cha = op->ht_oper.ext_cha;
	oper->prim_ch = op->phy_oper.prim_ch;
	oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
	oper->vht_bw = rf_bw_2_vht_bw(oper->bw);

	return HC_STATUS_OK;
}

/*
*
*/
INT  hc_radio_query_by_wdev(struct wifi_dev *wdev, struct freq_oper *oper)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct radio_dev *rdev;
	struct radio_control *r_ctrl;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return HC_STATUS_FAIL;
	}

	rdev = obj->rdev;
	r_ctrl = rdev->pRadioCtrl;
	oper->bw = r_ctrl->Bw;
	oper->cen_ch_1 = r_ctrl->CentralCh;
	oper->cen_ch_2 = r_ctrl->Channel2;
	oper->ext_cha = r_ctrl->ExtCha;
	oper->prim_ch = r_ctrl->Channel;
	oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
	oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
	return HC_STATUS_OK;
}

/*
*
*/
INT  hc_radio_query_by_channel(struct _RTMP_ADAPTER *ad, UCHAR channel, struct freq_oper *oper)
{
	INT ret = HC_STATUS_FAIL;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct _HD_RESOURCE_CFG *res = &ctrl->HwResourceCfg;
	struct radio_control *r_ctrl = NULL;
	UCHAR i;

	for (i = 0 ; i < res->concurrent_bands; i++) {
		r_ctrl = &res->PhyCtrl[i].RadioCtrl;
		if (r_ctrl->Channel == channel) {
			oper->bw = r_ctrl->Bw;
			oper->cen_ch_1 = r_ctrl->CentralCh;
			oper->cen_ch_2 = r_ctrl->Channel2;
			oper->ext_cha = r_ctrl->ExtCha;
			oper->prim_ch = r_ctrl->Channel;
			oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
			oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
			ret = HC_STATUS_OK;
			break;
		}
	}
	return ret;
}

/*
* suggest only used by phy related features, others should use hc_radio_query_by_wdev
*/
INT  hc_radio_query_by_index(struct _RTMP_ADAPTER *ad, UCHAR index, struct freq_oper *oper)
{
	INT ret = HC_STATUS_OK;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct _HD_RESOURCE_CFG *res = &ctrl->HwResourceCfg;
	struct radio_control *r_ctrl = NULL;

	r_ctrl = &res->PhyCtrl[index].RadioCtrl;
	oper->bw = r_ctrl->Bw;
	oper->cen_ch_1 = r_ctrl->CentralCh;
	oper->cen_ch_2 = r_ctrl->Channel2;
	oper->ext_cha = r_ctrl->ExtCha;
	oper->prim_ch = r_ctrl->Channel;
	oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
	oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
	return ret;
}

/*
* temporally use, only query first freq_oper by rfic, not support in 5G+5G or 2G+2G case
*/
INT hc_radio_query_by_rf(struct _RTMP_ADAPTER *ad, UCHAR rfic, struct freq_oper *oper)
{
	INT ret = HC_STATUS_FAIL;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct _HD_RESOURCE_CFG *res = &ctrl->HwResourceCfg;
	struct radio_control *r_ctrl = NULL;
	UCHAR i;

	for (i = 0 ; i < res->concurrent_bands; i++) {
		r_ctrl = &res->PhyCtrl[i].RadioCtrl;
		if (wmode_2_rfic(r_ctrl->PhyMode) & rfic) {
			oper->bw = r_ctrl->Bw;
			oper->cen_ch_1 = r_ctrl->CentralCh;
			oper->cen_ch_2 = r_ctrl->Channel2;
			oper->ext_cha = r_ctrl->ExtCha;
			oper->prim_ch = r_ctrl->Channel;
			oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
			oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
			ret = HC_STATUS_OK;
			break;
		}
	}
	return ret;
}

/*
 *
 */
VOID *hc_get_hdev_ctrl(struct wifi_dev *wdev)
{
	struct hdev_obj *h_obj = (struct hdev_obj *)wdev->pHObj;
	struct hdev_ctrl *h_ctrl = (struct hdev_ctrl *)h_obj->h_ctrl;

	return h_ctrl;
}

/*
*
*/
INT hc_obj_init(struct wifi_dev *wdev, INT idx)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct hdev_obj *h_obj = NULL;

	wdev->pHObj = &ctrl->HObjList[idx];
	h_obj = (struct hdev_obj *)wdev->pHObj;
	h_obj->h_ctrl = ctrl;

	return HC_STATUS_OK;
}

/*
*
*/
VOID hc_obj_exit(struct wifi_dev *wdev)
{
	struct hdev_obj *h_obj = (struct hdev_obj *)wdev->pHObj;

	h_obj->h_ctrl = NULL;
	wdev->pHObj = NULL;
}

/*
*
*/
inline struct _RTMP_CHIP_CAP *hc_get_chip_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->chip_cap;
}
EXPORT_SYMBOL(hc_get_chip_cap);

/*
*
*/
struct _RTMP_CHIP_OP *hc_get_chip_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->chip_ops;
}
EXPORT_SYMBOL(hc_get_chip_ops);

/*
*
*/
void hc_register_chip_ops(void *hdev_ctrl, struct _RTMP_CHIP_OP *ops)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	os_move_mem(&ctrl->chip_ops, ops, sizeof(*ops));
}

/*
*
*/
UCHAR hc_set_ChCtrl(CHANNEL_CTRL *ChCtrl, RTMP_ADAPTER *pAd, UCHAR ChIdx, UCHAR ChIdx2)
{
	os_move_mem(&ChCtrl->ChList[ChIdx], &pAd->TxPower[ChIdx2], sizeof(CHANNEL_TX_POWER));
	return HC_STATUS_OK;
}

UCHAR hc_set_ChCtrlFlags_CAP(CHANNEL_CTRL *ChCtrl, UINT ChannelListFlag, UCHAR ChIdx)
{
	ChCtrl->ChList[ChIdx].Flags |= ChannelListFlag;
	return HC_STATUS_OK;
}

UCHAR hc_set_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat)
{
	ChCtrl->ChListStat = ChListStat;
	return HC_STATUS_OK;
}

CHANNEL_CTRL *hc_get_channel_ctrl(void *hdev_ctrl, UCHAR BandIdx)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	return &ctrl->ChCtrl[BandIdx];
}

UCHAR hc_init_ChCtrl(RTMP_ADAPTER *pAd)
{
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		os_zero_mem(pChCtrl, sizeof(CHANNEL_CTRL));
	}
	return HC_STATUS_OK;
}
#ifdef CONFIG_AP_SUPPORT
UCHAR hc_init_ACSChCtrl(RTMP_ADAPTER *pAd)
{
	UCHAR BandIdx;
	AUTO_CH_CTRL *pAutoChCtrl;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
		pAutoChCtrl->AutoChSelCtrl.pScanReqwdev = NULL;
		pAutoChCtrl->AutoChSelCtrl.ScanChIdx = 0;
		pAutoChCtrl->AutoChSelCtrl.ChListNum = 0;
		pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
		os_zero_mem(pAutoChCtrl->AutoChSelCtrl.AutoChSelChList, (MAX_NUM_OF_CHANNELS+1)*sizeof(AUTOCH_SEL_CH_LIST));
	}
	return HC_STATUS_OK;
}
UCHAR hc_init_ACSChCtrlByBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	pAutoChCtrl->AutoChSelCtrl.pScanReqwdev = NULL;
	pAutoChCtrl->AutoChSelCtrl.ScanChIdx = 0;
	pAutoChCtrl->AutoChSelCtrl.ChListNum = 0;
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
	os_zero_mem(pAutoChCtrl->AutoChSelCtrl.AutoChSelChList, (MAX_NUM_OF_CHANNELS+1)*sizeof(AUTOCH_SEL_CH_LIST));
	return HC_STATUS_OK;
}
#endif

#ifdef DOT11_HE_AX
BOOLEAN hc_bcolor_acquire(struct wifi_dev *wdev, UINT8 *color)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct hdev_obj *obj = wdev->pHObj;
	UINT8 ret = 0;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return FALSE;
	}

	ret = bcolor_acquire_entry(ctrl, obj);
	if (ret < BSS_COLOR_VALUE_MIN || ret > BSS_COLOR_VALUE_MAX)
		return FALSE;

	*color = ret;
	return TRUE;
}

void hc_bcolor_release(struct wifi_dev *wdev, UINT8 color)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}

	if (color < BSS_COLOR_VALUE_MIN || color > BSS_COLOR_VALUE_MAX)
		return;

	bcolor_release_entry(ctrl, obj, color);
}

void hc_bcolor_occupy(struct wifi_dev *wdev, UINT8 color)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}

	if (color < BSS_COLOR_VALUE_MIN || color > BSS_COLOR_VALUE_MAX)
		return;

	bcolor_occupy_entry(ctrl, obj, color);
}

BOOLEAN hc_bcolor_is_occupied(struct wifi_dev *wdev, UINT8 color)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return FALSE;
	}

	if (color < BSS_COLOR_VALUE_MIN || color > BSS_COLOR_VALUE_MAX)
		return FALSE;

	return bcolor_entry_is_occupied(ctrl, obj, color);
}

void hc_bcolor_ageout(struct wifi_dev *wdev, UINT8 sec)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}

	return bcolor_entry_ageout(ctrl, obj, sec);
}

void hc_bcolor_get_bitmap(struct wifi_dev *wdev, UINT8 *bitmap)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}

	return bcolor_get_bitmap(ctrl, obj, bitmap);
}

void hc_bcolor_update_by_bitmap(struct wifi_dev *wdev, UINT8 *bitmap)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return;
	}

	return bcolor_update_by_bitmap(ctrl, obj, bitmap);
}
#endif

UCHAR hc_check_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat)
{
	return (ChCtrl->ChListStat == ChListStat);
}
struct _RTMP_CHIP_DBG *hc_get_chip_dbg(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->chip_dbg;
}

/*
*
*/
UINT32 hc_get_mac_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.mac_caps;
}

/*
*
*/
UINT32 hc_get_phy_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.phy_caps;
}

/*
*
*/
UINT32 hc_get_hif_type(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.hif_type;
}

/*
*
*/
UINT32 hc_get_asic_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.asic_caps;
}

/*
*
*/
VOID hc_set_mac_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	ctrl->chip_cap.mac_caps |= caps;
}

/*
*
*/
VOID hc_set_phy_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	ctrl->chip_cap.phy_caps |= caps;
}

/*
*
*/
VOID hc_set_asic_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	ctrl->chip_cap.asic_caps |= caps;
}

/*
*
*/
VOID hc_clear_asic_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	ctrl->chip_cap.asic_caps &= ~(caps);
}

UCHAR hc_get_cur_rfic(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	return obj->rdev->pRadioCtrl->cur_rfic_type;
}


/*
*
*/
UINT8 hc_get_chip_bcn_max_num(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.BcnMaxNum;
}

/*
*
*/
UINT16 hc_get_chip_wtbl_max_num(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.wtbl_max_entries;
}

#ifdef SW_CONNECT_SUPPORT
/*
*
*/
UINT16 hc_get_chip_sw_sta_max_num(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.sw_sta_max_entries;
}
#endif /* SW_CONNECT_SUPPORT */

/*
 * Used to indicate no WTBL entry is matched after HW search.
 * Different MAC arch could have particular value for it, and it's
 * defined in repsective header file. SW uses the unified value to
 * carry such information in RXBLK after processing the RXD.
 */
UINT16 hc_get_chip_wtbl_no_matched_idx(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.wtbl_no_matched;
}

/*
*
*/
BOOLEAN hc_get_chip_wapi_sup(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.FlgIsHwWapiSup;
}

UINT32 hc_get_chip_tx_token_nums(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.tkn_info.token_tx_cnt;
}
EXPORT_SYMBOL(hc_get_chip_tx_token_nums);

UINT32 hc_get_chip_sw_tx_token_nums(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return (ctrl->chip_cap.tkn_info.token_tx_cnt -
			ctrl->chip_cap.tkn_info.hw_tx_token_cnt);
}
EXPORT_SYMBOL(hc_get_chip_sw_tx_token_nums);

UINT32 hc_get_chip_mac_rxd_size(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.rx_hw_hdr_len;
}
EXPORT_SYMBOL(hc_get_chip_mac_rxd_size);

/*
*
*/
VOID *hc_get_hif_ctrl(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->hif.cfg;
}
EXPORT_SYMBOL(hc_get_hif_ctrl);

#ifdef CUT_THROUGH
/*
*
*/
VOID *hc_get_ct_cb(void *hdev_ctrl)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	return pci_hif->PktTokenCb;
}
EXPORT_SYMBOL(hc_get_ct_cb);

/*
*
*/
VOID hc_set_ct_cb(void *hdev_ctrl, void *ct_cb)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	pci_hif->PktTokenCb = ct_cb;
}
#endif /*CUT_THROUGH*/

/*
*
*/
inline VOID *hc_get_os_cookie(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->cookie;
}

/*
*
*/
inline VOID *hc_get_mcu_ctrl(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->mcu_ctrl;
}

/*
*
*/
inline struct _RTMP_ARCH_OP *hc_get_arch_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->arch_ops;
}

/*
*
*/
inline struct mt_io_ops *hc_get_io_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->io_ops;
}

/*
*
*/
inline void *hc_get_hdev_privdata(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->priv;
}

VOID *hc_get_hif_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->hif.ops;
}


/*
* hif ops
*/

UINT32 hif_get_resource_type(void *hdev_ctrl, UINT8 resource_idx)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);
	void *ctrl = hc_get_hif_ctrl(hdev_ctrl);

	if (ops->get_resource_type)
		return ops->get_resource_type(ctrl, resource_idx);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
	return 0;
}

BOOLEAN hif_free_txd(struct _RTMP_ADAPTER *ad, UINT8 resource_idx)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct hif_ops *ops = hc_get_hif_ops(ctrl);

	if (ops->free_txd)
		return ops->free_txd(ad, resource_idx);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"This function not support !, caller=%pS, ad=%p, ctrl=%p\n",  OS_TRACE, ad, ctrl);
	return FALSE;
}

VOID hif_free_rx_buf(void *hdev_ctrl, UCHAR resource_idx)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->free_rx_buf)
		ops->free_rx_buf(hdev_ctrl, resource_idx);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

VOID hif_reset_txrx_mem(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->reset_txrx_mem)
		ops->reset_txrx_mem(hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");

}

NDIS_STATUS hif_init_txrx_mem(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->init_txrx_mem)
		return ops->init_txrx_mem(hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return NDIS_STATUS_FAILURE;
}

VOID hif_dma_reset(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->dma_reset)
		ops->dma_reset(hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

VOID hif_dma_enable(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->dma_enable)
		ops->dma_enable(hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

VOID hif_dma_disable(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->dma_disable)
		ops->dma_disable(hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

BOOLEAN hif_poll_txrx_empty(void *hdev_ctrl, UINT8 pcie_port_or_all)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->poll_txrx_empty)
		return ops->poll_txrx_empty(hdev_ctrl, pcie_port_or_all);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return FALSE;
}

NDIS_STATUS hif_init_task_group(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->init_task_group)
		return ops->init_task_group(hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS hif_reset_task_group(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->reset_task_group)
		return ops->reset_task_group(hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS hif_register_irq(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->register_irq)
		return ops->register_irq(hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS hif_free_irq(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->free_irq)
		return ops->free_irq(hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return NDIS_STATUS_FAILURE;
}

/*MCU related*/
VOID hif_mcu_init(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->mcu_init)
		ops->mcu_init(hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

VOID hif_mcu_exit(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->mcu_exit)
		ops->mcu_exit(hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

INT32 hif_kick_out_fwdl_msg(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg)
{
	void *hdev_ctrl = ad->hdev_ctrl;
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->kick_out_fwdl_msg)
		return ops->kick_out_fwdl_msg(ad, msg);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");
	return 0;

}

INT32 hif_kick_out_cmd_msg(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg)
{
	void *hdev_ctrl = ad->hdev_ctrl;
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->kick_out_cmd_msg)
		return ops->kick_out_cmd_msg(ad, msg);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return 0;
}

VOID hif_kickout_data_tx(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk, UCHAR resource_idx)
{
	void *hdev_ctrl = ad->hdev_ctrl;
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->kickout_data_tx)
		ops->kickout_data_tx(ad, tx_blk, resource_idx);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

NDIS_STATUS hif_kickout_nullframe_tx(struct _RTMP_ADAPTER *ad, UCHAR que_idx, UCHAR *data, UINT len)
{
	void *hdev_ctrl = ad->hdev_ctrl;
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->kickout_nullframe_tx)
		return ops->kickout_nullframe_tx(ad, que_idx, data, len);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
	return NDIS_STATUS_SUCCESS;
}

VOID hif_rx_event_process(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg)
{
	void *hdev_ctrl = ad->hdev_ctrl;
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->rx_event_process)
		ops->rx_event_process(ad, msg);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

VOID hif_mcu_fw_init(struct _RTMP_ADAPTER *ad)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->mcu_fw_init)
		ops->mcu_fw_init(ad->hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

VOID hif_mcu_fw_exit(struct _RTMP_ADAPTER *ad)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->mcu_fw_exit)
		ops->mcu_fw_exit(ad->hdev_ctrl);
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"not support !\n");
}

UCHAR *hif_get_tx_buf(void *hdev_ctrl, struct _TX_BLK *tx_blk, UCHAR resource_idx, UCHAR frame_type)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->get_tx_buf)
		return ops->get_tx_buf(hdev_ctrl, tx_blk, resource_idx, frame_type);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return NULL;
}

UINT32 hif_get_tx_resource_free_num(void *hdev_ctrl, UINT8 resource_idx)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->get_tx_resource_free_num)
		return ops->get_tx_resource_free_num(hdev_ctrl, resource_idx);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return 0;
}

NDIS_STATUS hif_sys_init(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->sys_init)
		return ops->sys_init(hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return NDIS_STATUS_FAILURE;
}

inline UINT32 hif_get_resource_idx(void *hdev_ctrl, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type, UCHAR q_idx)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);
	UINT8 band_idx = 0;

	/* QM_V2 wdev may be NULL */
	if (wdev)
		band_idx = HcGetBandByWdev(wdev);

	if (ops->get_resource_idx)
		return ops->get_resource_idx(hdev_ctrl, band_idx, pkt_type, q_idx);

	return 0;
}

/*
*
*/
struct cmd_msg *hif_mcu_alloc_msg(RTMP_ADAPTER *ad, unsigned int length, BOOLEAN bOldCmdFmt)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->mcu_alloc_msg)
		return ops->mcu_alloc_msg(ad, length);
	else
		return AndesAllocCmdMsgGe(ad, length, bOldCmdFmt);
}

#ifdef CONFIG_STA_SUPPORT
/*
*
*/
VOID hif_ps_poll_enq(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->ps_poll_enq)
		return ops->ps_poll_enq(ad, pStaCfg);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");
}

/*
*
*/
VOID hif_sta_wakeup(struct _RTMP_ADAPTER *ad, BOOLEAN bFromTx, struct _STA_ADMIN_CONFIG *pStaCfg)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->sta_wakeup)
		return ops->sta_wakeup(ad, bFromTx, pStaCfg);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

}

/*
*
*/
VOID hif_sta_sleep_auto_wakeup(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg)
{
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->sta_sleep_auto_wakeup)
		return ops->sta_sleep_auto_wakeup(ad, pStaCfg);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");
}
#endif /* CONFIG_STA_SUPPORT */
/*
*
*/
INT hif_cmd_thread(ULONG context)
{
	RTMP_ADAPTER *ad;
	RTMP_OS_TASK *task;
	int status = 0;
	struct hif_ops *ops;

	task = (RTMP_OS_TASK *)context;
	ad = (PRTMP_ADAPTER)task->priv;
	ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->cmd_thread)
		return ops->cmd_thread(context);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");

	return status;
}

/*
*
*/
VOID hif_mcu_unlink_ackq(struct cmd_msg *msg)
{
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct hif_ops *ops = hc_get_hif_ops(ad->hdev_ctrl);

	if (ops->mcu_unlink_ackq)
		return ops->mcu_unlink_ackq(msg);

	MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");
}

/*
*
*/
UINT8 hif_get_tx_res_num(VOID *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->get_tx_res_num)
		return ops->get_tx_res_num(hc_get_hif_ctrl(hdev_ctrl));

		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");
	return 0;
}

/*
*
*/
UINT8 hif_get_rx_res_num(VOID *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	if (ops->get_rx_res_num)
		return ops->get_rx_res_num(hc_get_hif_ctrl(hdev_ctrl));

		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"not support !\n");
	return 0;
}

