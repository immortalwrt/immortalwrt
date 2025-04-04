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
#ifdef SW_CONNECT_SUPPORT
/* return S/W wcid , arg 4 : H/W wcid */
static UINT16 wtc_acquire_groupkey_wcid(struct hdev_ctrl *ctrl, WTBL_CFG *pWtblCfg, struct hdev_obj *obj, UINT16 *wcid_hw)
#else /* SW_CONNECT_SUPPORT */
static UINT16 wtc_acquire_groupkey_wcid(struct hdev_ctrl *ctrl, WTBL_CFG *pWtblCfg, struct hdev_obj *obj)
#endif /* !SW_CONNECT_SUPPORT */
{
	UINT16 AvailableWcid = WCID_INVALID;
	UCHAR OmacIdx, WdevType;
	UINT16 i, j;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UINT16 min_hw = pWtblCfg->MinMcastWcid;
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT16 max_hw = WTBL_MAX_NUM(pAd);

#ifdef SW_CONNECT_SUPPORT
	/* for SW , return the SW wcid */
	UINT16 min_sw = pWtblCfg->MinMcastWcidSw;
	UINT16 max_sw = SW_ENTRY_MAX_NUM(pAd);
#else /* SW_CONNECT_SUPPORT */
	/* for HW , return the HW wcid (equal to SW wcid)*/
	UINT16 min_sw = min_hw;
	UINT16 max_sw = max_hw;
#endif /* !SW_CONNECT_SUPPORT */

	OmacIdx = obj->OmacIdx;
	WdevType = obj->Type;
	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);

	for (i = (max_sw - 1), j = (max_hw - 1); (i >= min_sw) && (j >= min_hw); i--, j--) {
		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

		if (pWtblIdxRec->State != WTBL_STATE_NONE_OCCUPIED)
			continue;
		else {
			pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
#ifdef SW_CONNECT_SUPPORT
			*wcid_hw = pWtblIdxRec->WtblIdx = j;
			pWtblIdxRec->bSw = FALSE;

			/* Fill HW Table */
			pWtblCfg->Hw2SwTbl[j].bDummy = FALSE;
			pWtblCfg->Hw2SwTbl[j].wcid_sw = i;
			pWtblCfg->Hw2SwTbl[j].State = WTBL_STATE_SW_OCCUPIED;
			pWtblCfg->Hw2SwTbl[j].type = WTBL_TYPE_MCAST;
#else /* SW_CONNECT_SUPPORT */
			pWtblIdxRec->WtblIdx = i;
#endif /* !SW_CONNECT_SUPPORT */
			pWtblIdxRec->LinkToOmacIdx = OmacIdx;
			pWtblIdxRec->LinkToWdevType = WdevType;
			pWtblIdxRec->type = WTBL_TYPE_MCAST;
			AvailableWcid = i;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
					 " Found a non-occupied wtbl_idx:%d for WDEV_TYPE:%d\n"
					  " LinkToOmacIdx = %x, LinkToWdevType = %d\n",
					  i, WdevType, OmacIdx, WdevType);
			NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
			return AvailableWcid;
		}
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);

	if (i < min_sw) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "no available wtbl_idx for WDEV_TYPE:%d\n",
				  WdevType);
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
#ifdef SW_CONNECT_SUPPORT
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
#endif /* SW_CONNECT_SUPPORT */

	os_zero_mem(pWtblCfg, sizeof(WTBL_CFG));
	NdisAllocateSpinLock(NULL, &pWtblCfg->WtblIdxRecLock);

#ifdef SW_CONNECT_SUPPORT
	pWtblCfg->Hw2SwTbl = NULL;
	os_alloc_mem(NULL, (UCHAR **)&pWtblCfg->Hw2SwTbl, wtbl_max_num * sizeof(HW2SW_ENTRY));
	ASSERT(pWtblCfg->Hw2SwTbl != NULL);

	for (i = 0; i < wtbl_max_num; i++) {
		atomic_set(&(pWtblCfg->Hw2SwTbl[i].ref_cnt), 0);
		pWtblCfg->Hw2SwTbl[i].bDummy = FALSE;
		pWtblCfg->Hw2SwTbl[i].wcid_sw = 0;
		pWtblCfg->Hw2SwTbl[i].State = WTBL_STATE_NONE_OCCUPIED;
		pWtblCfg->Hw2SwTbl[i].type = WTBL_TYPE_NONE;
	}
#endif /* SW_CONNECT_SUPPORT */

	for (i = 0; IS_TR_WCID_VALID(pAd, i); i++) {
		pWtblParm = &pWtblCfg->WtblIdxRec[i];
		pWtblParm->State = WTBL_STATE_NONE_OCCUPIED;
		pWtblParm->type = WTBL_TYPE_NONE;
#ifdef SW_CONNECT_SUPPORT
		pWtblParm->bSw = FALSE;
		if (i == WCID_NO_MATCHED(pAd)) { /* S/W need to skip WCID 1023 */
			pWtblParm->State = WTBL_STATE_SW_OCCUPIED;
			pWtblParm->type = WTBL_TYPE_RESERVED;
		}
#endif /* SW_CONNECT_SUPPORT */
	}
}


/*
*
*/
VOID WtcExit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
#ifdef SW_CONNECT_SUPPORT
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	UINT16 i = 0;
	if (pWtblCfg->Hw2SwTbl) {
		for (i = 0; i < wtbl_max_num; i++) {
			atomic_set(&(pWtblCfg->Hw2SwTbl[i].ref_cnt), 0);
			pWtblCfg->Hw2SwTbl[i].bDummy = FALSE;
			pWtblCfg->Hw2SwTbl[i].wcid_sw = 0;
			pWtblCfg->Hw2SwTbl[i].State = WTBL_STATE_NONE_OCCUPIED;
			pWtblCfg->Hw2SwTbl[i].type = WTBL_TYPE_NONE;
		}
		os_free_mem(pWtblCfg->Hw2SwTbl);
		pWtblCfg->Hw2SwTbl = NULL;
	}
#endif /* SW_CONNECT_SUPPORT */

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
#ifdef SW_CONNECT_SUPPORT
	UINT16 dummy_cnt = 0;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UINT16 max_sw_entry_num = 0;
	UINT16 sw_entry_num_use_for_ucast = 0;
	UINT16 MaxUcastEntryNumSw = 0;
	UINT16 dummy = 0;
	UINT i;
#endif /* SW_CONNECT_SUPPORT */
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;
	MaxNumChipRept = GET_MAX_REPEATER_ENTRY_NUM(cap);
#endif /*MAC_REPEATER_SUPPROT*/
#endif /*CONFIG_AP_SUPPORT*/

#ifdef SW_CONNECT_SUPPORT
	max_sw_entry_num = SW_ENTRY_MAX_NUM(pAd);
	if (IS_SW_STA_ENABLED(pAd))
		dummy_cnt = pHwResource->concurrent_bands; /* run time on this feature , one physical dummy wcid for sw usage on two bands */
	else
		dummy_cnt = 0; /* run time off  this feature, on physical dummy wcid can be use for normal H/W Cap STA */
#endif /* SW_CONNECT_SUPPORT */

#ifdef CONFIG_VLAN_GTK_SUPPORT
	wtbl_num_resv_for_mcast = (BssidNum + MSTANum) * (1 + MAX_VLAN_NET_DEVICE);
#else
	wtbl_num_resv_for_mcast = BssidNum + MSTANum;
#endif

	wtbl_num_use_for_ucast = WdsNum + MaxNumChipRept + MSTANum;


	wtbl_num_use_for_sta = wtbl_max_num -
						   wtbl_num_resv_for_mcast -
						   wtbl_num_use_for_ucast;
	MaxUcastEntryNum = wtbl_num_use_for_sta + wtbl_num_use_for_ucast;

#ifdef SW_CONNECT_SUPPORT
	sw_entry_num_use_for_ucast = max_sw_entry_num -
								wtbl_num_resv_for_mcast -
								wtbl_num_use_for_ucast;
	MaxUcastEntryNumSw = sw_entry_num_use_for_ucast + wtbl_num_use_for_ucast;
#endif /* SW_CONNECT_SUPPORT */
#ifdef WTBL_TDD_SUPPORT
	if (IS_MT7915(pAd)) {
		/* MT7915 is large than 256, but 257 found F/W CMD TIME OUT */
		if (MaxUcastEntryNum > MAX_NUM_OF_MT7915_STA)
			MaxUcastEntryNum = MAX_NUM_OF_MT7915_STA;
	}
#endif /* WTBL_TDD_SUPPORT */

	ctrl->HwResourceCfg.WtblCfg.MinMcastWcid = wtbl_max_num - wtbl_num_resv_for_mcast;
	ctrl->HwResourceCfg.WtblCfg.MaxUcastEntryNum = (ctrl->HwResourceCfg.WtblCfg.MinMcastWcid >= 1)  ?  (ctrl->HwResourceCfg.WtblCfg.MinMcastWcid - 1) : 0;

#ifdef SW_CONNECT_SUPPORT
	/* record the S/W boundaris */
	ctrl->HwResourceCfg.WtblCfg.MaxUcastEntryNumSw = MaxUcastEntryNumSw;
	ctrl->HwResourceCfg.WtblCfg.MinMcastWcidSw = max_sw_entry_num - wtbl_num_resv_for_mcast;

	/* Assign dummy Wcid for each bands */
	for (i = 0, dummy = ctrl->HwResourceCfg.WtblCfg.MaxUcastEntryNum ; i < dummy_cnt && (dummy > 0) ; i++, dummy--) {
		ctrl->HwResourceCfg.WtblCfg.dummy_wcid_obj[i].HwWcid = dummy;
		ctrl->HwResourceCfg.WtblCfg.dummy_wcid_obj[i].State = WTBL_STATE_SW_OCCUPIED;
	}
#endif /* SW_CONNECT_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 " MaxStaNum: HW(%u),SW(%u), dummy_cnt(%u), Dummy b[0]:(%u), Dummy b[1]:(%u), BssidNum:%u, WdsNum:%u, MSTANum:%u, MaxNumChipRept:%u, MinMcastWcid: HW(%u), SW:(%u)\n",
			  ctrl->HwResourceCfg.WtblCfg.MaxUcastEntryNum,
			  ctrl->HwResourceCfg.WtblCfg.MaxUcastEntryNumSw,
			  dummy_cnt,
			  ctrl->HwResourceCfg.WtblCfg.dummy_wcid_obj[0].HwWcid,
			  ctrl->HwResourceCfg.WtblCfg.dummy_wcid_obj[1].HwWcid,
			  BssidNum,
			  WdsNum,
			  MSTANum,
			  MaxNumChipRept,
			  ctrl->HwResourceCfg.WtblCfg.MinMcastWcid,
			  ctrl->HwResourceCfg.WtblCfg.MinMcastWcidSw);
#else /* SW_CONNECT_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 " MaxStaNum:%d, BssidNum:%d, WdsNum:%d, MSTANum:%d, MaxNumChipRept:%d, MinMcastWcid:%d\n",
			  wtbl_num_use_for_sta,
			  BssidNum,
			  WdsNum,
			  MSTANum,
			  MaxNumChipRept,
			  ctrl->HwResourceCfg.WtblCfg.MinMcastWcid);
#endif /* !SW_CONNECT_SUPPORT */
#ifdef SW_CONNECT_SUPPORT
	/* return the MAX H/W or S/W Entry Num */
	if (ctrl->HwResourceCfg.WtblCfg.MaxUcastEntryNumSw > MaxUcastEntryNum)
		MaxUcastEntryNum = ctrl->HwResourceCfg.WtblCfg.MaxUcastEntryNumSw;
#endif /* SW_CONNECT_SUPPORT */
	return MaxUcastEntryNum;
}


/*
*
*/
#ifdef SW_CONNECT_SUPPORT
UINT16 WtcAcquireGroupKeyWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT16 *wcid_hw)
#else /* SW_CONNECT_SUPPORT */
UINT16 WtcAcquireGroupKeyWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj)
#endif /* !SW_CONNECT_SUPPORT */
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;
#ifdef SW_CONNECT_SUPPORT
	return wtc_acquire_groupkey_wcid(ctrl, pWtblCfg, obj, wcid_hw);
#else /* SW_CONNECT_SUPPORT */
	return wtc_acquire_groupkey_wcid(ctrl, pWtblCfg, obj);
#endif /* 1SW_CONNECT_SUPPORT */
}

#ifdef SW_CONNECT_SUPPORT
/*
	@ UINT16 wcid : for S/W cases, input means S/W wcid, for H/W cases , input means H/W wcid
*/
#endif /* SW_CONNECT_SUPPORT */
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
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "idx:%d > WTBL_MAX_NUM\n", wcid);
		return wcid;
	}

	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
	pWtblIdxRec = &pWtblCfg->WtblIdxRec[wcid];

	if (pWtblIdxRec->type != WTBL_TYPE_MCAST) {
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
		return wcid;
	}

	if (pWtblIdxRec->State == WTBL_STATE_NONE_OCCUPIED) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "try to release non-occupied idx:%d, something wrong?\n",
				   wcid);
		ReleaseWcid = wcid;
	} else {
#ifdef SW_CONNECT_SUPPORT
		/* H/W table need to be clear before S/W Table call os_zero_mem() */
		pWtblIdxRec->bSw = FALSE;
		/* clear H/W Entry */
		pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].bDummy = FALSE;
		pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].wcid_sw = 0;
		pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].State = WTBL_STATE_NONE_OCCUPIED;
		pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].type = WTBL_TYPE_NONE;
#endif /* SW_CONNECT_SUPPORT */
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
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "MaxUcastEntryNum=%d >= WTBL_MAX_NUM(%d)\n",
				  pWtblCfg->MaxUcastEntryNum, WTBL_MAX_NUM(pAd));
		return WTBL_MAX_NUM(pAd);
	} else
		return pWtblCfg->MaxUcastEntryNum;
}

#ifdef SW_CONNECT_SUPPORT
UINT16 WtcGetMaxStaNumSw(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;
	RTMP_ADAPTER *pAd = ctrl->priv;

	if (!IS_WCID_VALID(pAd, pWtblCfg->MaxUcastEntryNumSw)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "MaxUcastEntryNumSw=%d >= SW_ENTRY_MAX_NUM(%d)\n",
				  pWtblCfg->MaxUcastEntryNumSw, SW_ENTRY_MAX_NUM(pAd));
		return SW_ENTRY_MAX_NUM(pAd);
	} else
		return pWtblCfg->MaxUcastEntryNumSw;
}


UINT16 WtcGetDummyWcid(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev=%d, hobj is not ready!\n",  wdev->wdev_idx);
		return 0;
	}

	if  ((wdev->pDummy_obj) && (wdev->pDummy_obj->State == WTBL_STATE_SW_OCCUPIED))
		return wdev->pDummy_obj->HwWcid;
	else
		return 0;
}

UINT16 WtcGetSwWcid(struct hdev_ctrl *ctrl, UINT16 hw_wcid)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT16 Wcid = 0;

	if (!IS_WCID_VALID_HW(pAd, hw_wcid)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "idx:%d > WTBL_MAX_NUM\n", hw_wcid);
		return Wcid;
	}

	/* if(HcIsDummyWcid(pAd, hw_wcid) == FALSE) */
		Wcid = pWtblCfg->Hw2SwTbl[hw_wcid].wcid_sw;

	return Wcid;
}

#endif /* SW_CONNECT_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
/*
* Check the wcid is reach H/W bound (dummy wcid) or not
*/
BOOLEAN WtcIsDummyHit(struct hdev_ctrl *ctrl, UINT16 wcid)
{
	UINT8 i;
	RTMP_ADAPTER *pAd = ctrl->priv;
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;

	if (!IS_SW_STA_ENABLED(pAd))
		return FALSE;

	/* Hit one band is need to treat as S/W Entry Wcid */
	for (i = 0; i < DBDC_BAND_NUM ; i++) {
		if ((pWtblCfg->dummy_wcid_obj[i].HwWcid != 0) && (pWtblCfg->dummy_wcid_obj[i].State == WTBL_STATE_SW_OCCUPIED)) {
			if (wcid >= pWtblCfg->dummy_wcid_obj[i].HwWcid)
				return TRUE;
		}
	}

	return FALSE;
}
#endif /* SW_CONNECT_SUPPORT */

/*
*
*/
#ifdef SW_CONNECT_SUPPORT
/* return S/W wcid , arg 6 : H/W wcid */
UINT16 WtcAcquireUcastWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT16 FirstWcid, BOOLEAN is_A4, BOOLEAN is_apcli, struct wifi_dev *wdev, UINT16 *wcid_hw, BOOLEAN *bSw)
#else /* SW_CONNECT_SUPPORT */
UINT16 WtcAcquireUcastWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT16 FirstWcid, BOOLEAN is_A4, BOOLEAN is_apcli)
#endif /* !SW_CONNECT_SUPPORT */
{
	UINT16 i;
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT16 wtbl_max_num = 0;
	UINT16 max_sw_entry_num = 0;

	if (obj == NULL || pResource == NULL || pWtblCfg == NULL || pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 " unexpected NULL please check!!\n");
		return WCID_INVALID;
	}

	max_sw_entry_num = pWtblCfg->MaxUcastEntryNum;
	wtbl_max_num = WTBL_MAX_NUM(pAd);
#ifdef SW_CONNECT_SUPPORT
	max_sw_entry_num = SW_ENTRY_MAX_NUM(pAd);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "wtbl_max_num=%u, max_sw_entry_num=%u, pWtblCfg->MaxUcastEntryNum=%u, DummyWcid=%u\n",
			 wtbl_max_num, max_sw_entry_num, pWtblCfg->MaxUcastEntryNum, HcGetDummyWcid(wdev));
#endif /* SW_CONNECT_SUPPORT */
	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);

	/* assigned 1st wcid manually */
	if (pAd->assignWcid > 0)
		FirstWcid = pAd->assignWcid;
#ifdef WARP_512_SUPPORT
	if (pAd->Warp512Support && (is_A4 || is_apcli)) {
		for (i = A4_APCLI_FIRST_WCID; i < A4_APCLI_FIRST_WCID + MAX_RESERVE_ENTRY; i++) {
			pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

			if (pWtblIdxRec == NULL) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "unexpected NULL please check!!\n");
				NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
				return WCID_INVALID;
			}

			if (pWtblIdxRec->State != WTBL_STATE_NONE_OCCUPIED)
				continue;

			pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
			pWtblIdxRec->WtblIdx = i;
#ifdef SW_CONNECT_SUPPORT
			/* apcli's  always use H/W WCID */
			*bSw = pWtblIdxRec->bSw = FALSE;
			*wcid_hw = i;
			/* Fill HW Table */
			if (i < wtbl_max_num) {
				pWtblCfg->Hw2SwTbl[i].bDummy = FALSE;
				pWtblCfg->Hw2SwTbl[i].wcid_sw = i;
				pWtblCfg->Hw2SwTbl[i].State = WTBL_STATE_SW_OCCUPIED;
				pWtblCfg->Hw2SwTbl[i].type = WTBL_TYPE_UCAST;
			}
#endif /* SW_CONNECT_SUPPORT */
			pWtblIdxRec->LinkToOmacIdx = obj->OmacIdx;
			pWtblIdxRec->LinkToWdevType = obj->Type;
			pWtblIdxRec->type = WTBL_TYPE_UCAST;
			NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
			return i;
		}
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Cannot alloc wcid for A4 entry!!\n");
		return WCID_INVALID;
	} else {
#endif /* WARP_512_SUPPORT */
		for (i = FirstWcid; i < max_sw_entry_num; i++) {
#ifdef SW_CONNECT_SUPPORT
			if (!IS_SW_STA_ENABLED(pAd))
#endif /* SW_CONNECT_SUPPORT */
			{
				/* sanity check to avoid out of bound with pAd->MacTab.Content */
				if (i >= wtbl_max_num)
					continue;
			}
#ifdef WARP_512_SUPPORT
			if  (pAd->Warp512Support &&
				((i >= A4_APCLI_FIRST_WCID) && (i < A4_APCLI_FIRST_WCID + MAX_RESERVE_ENTRY)))
				continue;
#endif /* WARP_512_SUPPORT */

			pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

			if (pWtblIdxRec == NULL) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "unexpected NULL please check!!\n");
				return WCID_INVALID;
			}

			if (pWtblIdxRec->State != WTBL_STATE_NONE_OCCUPIED)
				continue;

			pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
			pWtblIdxRec->WtblIdx = i;
#ifdef SW_CONNECT_SUPPORT
			*bSw = pWtblIdxRec->bSw = FALSE;
			*wcid_hw = i;

			if ((WtcIsDummyHit(ctrl, i)) &&
				(wdev->pDummy_obj && (wdev->pDummy_obj->State == WTBL_STATE_SW_OCCUPIED))) {
					*wcid_hw = pWtblIdxRec->WtblIdx = wdev->pDummy_obj->HwWcid;
					*bSw = pWtblIdxRec->bSw = TRUE;
					/* S/W cases */
					if (*wcid_hw < wtbl_max_num) {
						atomic_inc(&(pWtblCfg->Hw2SwTbl[*wcid_hw].ref_cnt));
						pWtblCfg->Hw2SwTbl[*wcid_hw].bDummy = TRUE;
						pWtblCfg->Hw2SwTbl[*wcid_hw].wcid_sw = i;
						pWtblCfg->Hw2SwTbl[*wcid_hw].State = WTBL_STATE_SW_OCCUPIED;
						pWtblCfg->Hw2SwTbl[*wcid_hw].type = WTBL_TYPE_UCAST;
					}
			} else {
					/* Non S/W cases */
					/* Fill HW Table */
					if (i < wtbl_max_num) {
						pWtblCfg->Hw2SwTbl[i].bDummy = FALSE;
						pWtblCfg->Hw2SwTbl[i].wcid_sw = i;
						pWtblCfg->Hw2SwTbl[i].State = WTBL_STATE_SW_OCCUPIED;
						pWtblCfg->Hw2SwTbl[i].type = WTBL_TYPE_UCAST;
					}
			}
#endif /* SW_CONNECT_SUPPORT */
			/*TODO: Carter, check flow when calling this function, OmacIdx might be erroed.*/
			pWtblIdxRec->LinkToOmacIdx = obj->OmacIdx;
			pWtblIdxRec->LinkToWdevType = obj->Type;
			pWtblIdxRec->type = WTBL_TYPE_UCAST;
			NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
			return i;
		}
#ifdef WARP_512_SUPPORT
		if (pAd->Warp512Support) {
			for (i = A4_APCLI_FIRST_WCID; i < A4_APCLI_FIRST_WCID + MAX_RESERVE_ENTRY; i++) {
				pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

				if (pWtblIdxRec == NULL) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Unexpected NULL please check!!\n");
					NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
					return WCID_INVALID;
				}

				if (pWtblIdxRec->State != WTBL_STATE_NONE_OCCUPIED)
					continue;

				pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
				pWtblIdxRec->WtblIdx = i;
#ifdef SW_CONNECT_SUPPORT
				/* apcli's  always use H/W WCID */
				*bSw = pWtblIdxRec->bSw = FALSE;
				*wcid_hw = i;

				/* Fill HW Table */
				if (i < wtbl_max_num) {
					pWtblCfg->Hw2SwTbl[i].bDummy = FALSE;
					pWtblCfg->Hw2SwTbl[i].wcid_sw = i;
					pWtblCfg->Hw2SwTbl[i].State = WTBL_STATE_SW_OCCUPIED;
					pWtblCfg->Hw2SwTbl[i].type = WTBL_TYPE_UCAST;
				}
#endif /* SW_CONNECT_SUPPORT */
				pWtblIdxRec->LinkToOmacIdx = obj->OmacIdx;
				pWtblIdxRec->LinkToWdevType = obj->Type;
				pWtblIdxRec->type = WTBL_TYPE_UCAST;
				NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
				return i;
			}
		}
#endif
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);

		return WCID_INVALID;
#ifdef WARP_512_SUPPORT
	}
#endif
}


/*
*
*/
#ifdef SW_CONNECT_SUPPORT
/*
	pWtblCfg->WtblIdxRec[MAX_LEN_OF_MAC_TABLE]  array is S/W size.
	once the flag turn on:
		arg 3 : in put wcid means S/W wcid
	once the flag turn off:
		arg 3 wcid means both S/W wcid and H/W wcid
*/
#endif /* SW_CONNECT_SUPPORT */
UINT16 WtcReleaseUcastWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT16 wcid)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UINT16 ReleaseWcid = WCID_INVALID;
	RTMP_ADAPTER *pAd = ctrl->priv;

	if (!IS_WCID_VALID(pAd, wcid)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "idx:%d > WTBL_MAX_NUM\n", wcid);
		return wcid;
	}

	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
	pWtblIdxRec = &pWtblCfg->WtblIdxRec[wcid];

	if (pWtblIdxRec->type != WTBL_TYPE_UCAST) {
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
		return wcid;
	}

	if (pWtblIdxRec->State == WTBL_STATE_NONE_OCCUPIED) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "try to release non-occupied idx:%d, something wrong?\n",
				  wcid);
		ReleaseWcid = wcid;
	} else {
#ifdef SW_CONNECT_SUPPORT
		if (pWtblIdxRec->bSw) {
			pWtblIdxRec->bSw = FALSE;
			if (atomic_read(&(pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].ref_cnt)) > 0)
				atomic_dec(&(pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].ref_cnt));
		}

		/* clear H/W Entry */
		if (atomic_read(&(pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].ref_cnt)) == 0) {
			pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].bDummy = FALSE;
			pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].wcid_sw = 0;
			pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].State = WTBL_STATE_NONE_OCCUPIED;
			pWtblCfg->Hw2SwTbl[pWtblIdxRec->WtblIdx].type = WTBL_TYPE_NONE;
		}
#endif /* SW_CONNECT_SUPPORT */
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

	MTWF_PRINT("\tWtblRecDump:\n");

	MTWF_PRINT("\t  ChipCap MaxEntries:%d, NoMatched:%x\n",
			 WTBL_MAX_NUM(pAd), WCID_NO_MATCHED(pAd));
	MTWF_PRINT("\t  WtblCfg MaxUcEntryNum:%d, MinMcWcid:%d, MAX_MAC_TABLE:%d\n",
			 pWtblCfg->MaxUcastEntryNum, pWtblCfg->MinMcastWcid, MAX_LEN_OF_MAC_TABLE);

	for (i = 0; i < wtbl_max_num; i++) {
		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

		if (pWtblIdxRec->State == WTBL_STATE_SW_OCCUPIED) {
			MTWF_PRINT("\tWcid[%03d] used by OM:%02x(%s), Type:%s\n",
					 pWtblIdxRec->WtblIdx,
					 pWtblIdxRec->LinkToOmacIdx,
					 wdev_type2str(pWtblIdxRec->LinkToWdevType),
					 WtblTypeStr[pWtblIdxRec->type]);
		}
	}
}

