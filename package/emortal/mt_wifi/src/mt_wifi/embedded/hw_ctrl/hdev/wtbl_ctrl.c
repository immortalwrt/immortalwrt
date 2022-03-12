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

/*
* local function
*/

/*
*
*/
static UINT16 wtc_acquire_groupkey_wcid(struct hdev_ctrl *ctrl, WTBL_CFG *pWtblCfg, struct hdev_obj *obj)
{
	UINT16 AvailableWcid = WCID_INVALID;
	UCHAR OmacIdx, WdevType;
	UINT16 i;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UINT16 min_wcid = pWtblCfg->MinMcastWcid;
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	OmacIdx = obj->OmacIdx;
	WdevType = obj->Type;
	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);

	for (i = (wtbl_max_num - 1); i >= min_wcid; i--) {
		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

		if (pWtblIdxRec->State != WTBL_STATE_NONE_OCCUPIED)
			continue;
		else {
			pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
			pWtblIdxRec->WtblIdx = i;
			pWtblIdxRec->LinkToOmacIdx = OmacIdx;
			pWtblIdxRec->LinkToWdevType = WdevType;
			pWtblIdxRec->type = WTBL_TYPE_MCAST;
			AvailableWcid = i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Found a non-occupied wtbl_idx:%d for WDEV_TYPE:%d\n"
					  " LinkToOmacIdx = %x, LinkToWdevType = %d\n",
					  __func__, i, WdevType, OmacIdx, WdevType));
			NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
			return AvailableWcid;
		}
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);

	if (i < min_wcid) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: no available wtbl_idx for WDEV_TYPE:%d\n",
				  __func__, WdevType));
	}

	return AvailableWcid;
}


/*Wtable control*/
/*
*
*/
VOID WtcInit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblParm = NULL;
	UINT16 i = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	os_zero_mem(pWtblCfg, sizeof(WTBL_CFG));
	NdisAllocateSpinLock(NULL, &pWtblCfg->WtblIdxRecLock);

	for (i = 0; IS_TR_WCID_VALID(pAd, i); i++) {
		pWtblParm = &pWtblCfg->WtblIdxRec[i];
		pWtblParm->State = WTBL_STATE_NONE_OCCUPIED;
		pWtblParm->type = WTBL_TYPE_NONE;
	}
}


/*
*
*/
VOID WtcExit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;

	NdisFreeSpinLock(&pWtblCfg->WtblIdxRecLock);
	os_zero_mem(pWtblCfg, sizeof(WTBL_CFG));
}


/*
*
*/
UINT16 WtcSetMaxStaNum(struct hdev_ctrl *ctrl, UCHAR BssidNum, UCHAR MSTANum)
{
	UINT16 wtbl_num_resv_for_mcast = 0;
	UINT16 wtbl_num_use_for_ucast = 0;
	UINT16 wtbl_num_use_for_sta = 0;
	UINT16 MaxNumChipRept = 0;
	UINT16 WdsNum = MAX_WDS_ENTRY;
	UINT16 MaxUcastEntryNum = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;
	MaxNumChipRept = GET_MAX_REPEATER_ENTRY_NUM(cap);
#endif /*MAC_REPEATER_SUPPROT*/
#endif /*CONFIG_AP_SUPPORT*/
	wtbl_num_resv_for_mcast = BssidNum + MSTANum;
	wtbl_num_use_for_ucast = WdsNum + MaxNumChipRept + MSTANum;
	wtbl_num_use_for_sta = wtbl_max_num -
						   wtbl_num_resv_for_mcast -
						   wtbl_num_use_for_ucast;
	MaxUcastEntryNum = wtbl_num_use_for_sta + wtbl_num_use_for_ucast;
	ctrl->HwResourceCfg.WtblCfg.MaxUcastEntryNum = MaxUcastEntryNum;
	ctrl->HwResourceCfg.WtblCfg.MinMcastWcid = wtbl_max_num - wtbl_num_resv_for_mcast;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: MaxStaNum:%d, BssidNum:%d, WdsNum:%d, MSTANum:%d, MaxNumChipRept:%d, MinMcastWcid:%d\n",
			  __func__,
			  wtbl_num_use_for_sta,
			  BssidNum,
			  WdsNum,
			  MSTANum,
			  MaxNumChipRept,
			  ctrl->HwResourceCfg.WtblCfg.MinMcastWcid));
	return MaxUcastEntryNum;
}

/*
*
*/
UINT16 WtcAcquireGroupKeyWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;

	return wtc_acquire_groupkey_wcid(ctrl, pWtblCfg, obj);
}

/*
*
*/
UINT16 WtcReleaseGroupKeyWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT16 wcid)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UINT16 ReleaseWcid = WCID_INVALID;
	RTMP_ADAPTER *pAd = ctrl->priv;

	if (!IS_WCID_VALID(pAd, wcid)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: idx:%d > WTBL_MAX_NUM\n", __func__, wcid));
		return wcid;
	}

	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
	pWtblIdxRec = &pWtblCfg->WtblIdxRec[wcid];

	if (pWtblIdxRec->type != WTBL_TYPE_MCAST) {
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
		return wcid;
	}

	if (pWtblIdxRec->State == WTBL_STATE_NONE_OCCUPIED) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: try to release non-occupied idx:%d, something wrong?\n",
				  __func__, wcid));
		ReleaseWcid = wcid;
	} else {
		os_zero_mem(pWtblIdxRec, sizeof(WTBL_IDX_PARAMETER));
		/*make sure entry is cleared to usable one.*/
		pWtblIdxRec->State = WTBL_STATE_NONE_OCCUPIED;
		pWtblIdxRec->type = WTBL_TYPE_NONE;
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	return ReleaseWcid;
}


/*
*
*/
UCHAR WtcGetWcidLinkType(struct hdev_ctrl *ctrl, UINT16 wcid)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = &pWtblCfg->WtblIdxRec[wcid];

	return pWtblIdxRec->LinkToWdevType;
}



/*
*
*/
UINT16 WtcGetMaxStaNum(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;
	RTMP_ADAPTER *pAd = ctrl->priv;

	if (!IS_WCID_VALID(pAd, pWtblCfg->MaxUcastEntryNum)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MaxUcastEntryNum=%d >= WTBL_MAX_NUM(%d)\n",
				  __func__, pWtblCfg->MaxUcastEntryNum, WTBL_MAX_NUM(pAd)));
		return WTBL_MAX_NUM(pAd);
	} else
		return pWtblCfg->MaxUcastEntryNum;
}



/*
*
*/
UINT16 WtcAcquireUcastWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT16 FirstWcid)
{
	UINT16 i;
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT16 wtbl_max_num = 0;

	if (obj == NULL || pResource == NULL || pWtblCfg == NULL || pAd == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: unexpected NULL please check!!\n", __func__));
		return WCID_INVALID;
	}
	wtbl_max_num = WTBL_MAX_NUM(pAd);
	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);

	/* assigned 1st wcid manually */
	if (pAd->assignWcid > 0)
		FirstWcid = pAd->assignWcid;

	for (i = FirstWcid; i < pWtblCfg->MaxUcastEntryNum; i++) {
		/* sanity check to avoid out of bound with pAd->MacTab.Content */
		if (i >= wtbl_max_num)
			continue;

#if defined(MT7915)
#ifdef DBDC_MODE
		if (MTK_REV_ET(pAd, MT7915, MT7915E1) && pAd->CommonCfg.dbdc_mode) {
			UINT16 s_idx = 4*(i/32);
			UINT16 idx, hit = 0;
			for (idx = s_idx; idx < s_idx+4; idx++) {
				if (i == wtbl_e1_risky_wcid[idx]) {
					hit++;
					break;
				}
			}

			if (hit > 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s: WTBL entry +++\n", __func__));
				continue;
			}
		}
#endif /* DBDC_MODE */
#endif /* MT7915 */

		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

		if (pWtblIdxRec == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: unexpected NULL please check!!\n", __func__));
			return WCID_INVALID;
		}

		if (pWtblIdxRec->State != WTBL_STATE_NONE_OCCUPIED)
			continue;

		pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
		pWtblIdxRec->WtblIdx = i;
		/*TODO: Carter, check flow when calling this function, OmacIdx might be erroed.*/
		pWtblIdxRec->LinkToOmacIdx = obj->OmacIdx;
		pWtblIdxRec->LinkToWdevType = obj->Type;
		pWtblIdxRec->type = WTBL_TYPE_UCAST;
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
		return i;
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);

	return WCID_INVALID;
}


/*
*
*/
UINT16 WtcReleaseUcastWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT16 wcid)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UINT16 ReleaseWcid = WCID_INVALID;
	RTMP_ADAPTER *pAd = ctrl->priv;

	if (!IS_WCID_VALID(pAd, wcid)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: idx:%d > WTBL_MAX_NUM\n", __func__, wcid));
		return wcid;
	}

	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
	pWtblIdxRec = &pWtblCfg->WtblIdxRec[wcid];

	if (pWtblIdxRec->type != WTBL_TYPE_UCAST) {
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
		return wcid;
	}

	if (pWtblIdxRec->State == WTBL_STATE_NONE_OCCUPIED) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: try to release non-occupied idx:%d, something wrong?\n",
				  __func__, wcid));
		ReleaseWcid = wcid;
	} else {
		os_zero_mem(pWtblIdxRec, sizeof(WTBL_IDX_PARAMETER));
		/*make sure entry is cleared to usable one.*/
		pWtblIdxRec->State = WTBL_STATE_NONE_OCCUPIED;
		pWtblIdxRec->type = WTBL_TYPE_NONE;
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	return ReleaseWcid;
}

/*
*
*/
RTMP_STRING *wdev_type2str(int type);

VOID WtcRecDump(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UINT16 i;
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
	CHAR *WtblTypeStr[] = {"None", "Ucast", "Mcast"};

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tWtblRecDump:\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t  ChipCap MaxEntries:%d, NoMatched:%x\n",
			 WTBL_MAX_NUM(pAd), WCID_NO_MATCHED(pAd)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t  WtblCfg MaxUcEntryNum:%d, MinMcWcid:%d, MAX_MAC_TABLE:%d\n",
			 pWtblCfg->MaxUcastEntryNum, pWtblCfg->MinMcastWcid, MAX_LEN_OF_MAC_TABLE));

	for (i = 0; i < wtbl_max_num; i++) {
		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

		if (pWtblIdxRec->State == WTBL_STATE_SW_OCCUPIED) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("\tWcid[%03d] used by OM:%02x(%s), Type:%s\n",
					 pWtblIdxRec->WtblIdx,
					 pWtblIdxRec->LinkToOmacIdx,
					 wdev_type2str(pWtblIdxRec->LinkToWdevType),
					 WtblTypeStr[pWtblIdxRec->type]));
		}
	}
}

