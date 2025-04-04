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

/*WMM control*/


/*Local functions*/
static  BOOLEAN compare_edca(EDCA_PARM *edca1, EDCA_PARM *edca2)
{
	if ((!edca1->bValid) || (!edca2->bValid))
		return FALSE;

	if (os_cmp_mem(edca2->Aifsn, edca1->Aifsn, 4))
		return FALSE;

	if (os_cmp_mem(edca2->Txop, edca1->Txop, sizeof(USHORT)*4))
		return FALSE;

	if (os_cmp_mem(edca2->Cwmax, edca1->Cwmax, 4))
		return FALSE;

	if (os_cmp_mem(edca2->Cwmin, edca1->Cwmin, 4))
		return FALSE;

	return TRUE;
}

/*
  *
 */
static UINT32 wmm_ctrl_get_num(struct hdev_ctrl *ctrl)
{
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;

	return cap->qos.WmmHwNum;
}

/*Export Functions*/


/*
 *
*/
struct wmm_entry *wmm_ctrl_get_entry_by_idx(struct hdev_ctrl *ctrl, UINT32 idx)
{
	return &ctrl->HwResourceCfg.wmm_ctrl.entries[idx];
}

/*
 *
*/
VOID wmm_ctrl_release_entry(struct hdev_obj *obj)
{
	EDCA_PARM *edca;
	struct radio_dev *rdev = NULL;
	struct hdev_ctrl *ctrl = NULL;
	struct wmm_entry *entry = NULL;
	UINT32 wmm_idx;
	/* TODO: Star, should remove it. */
	RTMP_ADAPTER *pAd;

	if (!obj || !obj->bWmmAcquired) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Can't find HdevObj or Edca not required\n");
		return;
	}

	rdev = obj->rdev;
	ctrl = rdev->priv;
	pAd = (RTMP_ADAPTER *) ctrl->priv;
	entry = wmm_ctrl_get_entry_by_idx(ctrl, obj->WmmIdx);
	edca = &entry->edca;

	if (!edca || !edca->bValid) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Can't find Edca for rdev: %d, Obj: %d\n",
				 rdev->Idx, obj->Idx);
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s(): ObjIdx=%d,WmmIdx=%d\n", __func__, obj->Idx, obj->WmmIdx);
	wmm_idx = entry->wmm_set;
	entry->ref_cnt--;

	if (entry->ref_cnt <= 0) {
		os_zero_mem(edca, sizeof(EDCA_PARM));
		edca->bValid = FALSE;
		entry->ref_cnt = 0;
		entry->wmm_set = wmm_idx;
		entry->tx_mode = HOBJ_TX_MODE_TXD;
		AsicSetEdcaParm(pAd, entry, pAd->wdev_list[obj->Idx]);
	}

	obj->WmmIdx = 0;
	obj->bWmmAcquired = FALSE;
#ifdef MT7626_REDUCE_TX_OVERHEAD
	/* Update WmmIdx of wdev */
	pAd->wdev_list[obj->Idx]->WmmIdx = obj->WmmIdx;
#endif /* MT7626_REDUCE_TX_OVERHEAD */
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Release a WMM for ObjIdx: %d\n", obj->Idx);
}

/*
 *
*/
struct wmm_entry *wmm_ctrl_acquire_entry(struct hdev_obj *obj, struct _EDCA_PARM *pEdcaParm)
{
	struct radio_dev *rdev;
	struct hdev_ctrl *ctrl;
	struct wmm_ctrl *wctrl;
	INT32 i;
	UINT32 num;
	EDCA_PARM *edca, ReleaseEdca;
	struct wmm_entry *entry = NULL;
	UCHAR dbdc_idx;

	if (!obj) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): Can't find HdevObj\n", __func__);
		goto end;
	}

	rdev = obj->rdev;
	ctrl = rdev->priv;
	wctrl = &ctrl->HwResourceCfg.wmm_ctrl;
	num = wctrl->num;
	dbdc_idx = RcGetBandIdx(rdev);
	os_zero_mem(&ReleaseEdca, sizeof(EDCA_PARM));

	/*if input edca is all zero, assign default APEdca parameter*/
	if (!os_cmp_mem(&ReleaseEdca, pEdcaParm, sizeof(EDCA_PARM)) || pEdcaParm->bValid != TRUE) {
		switch (obj->Type) {
		case WDEV_TYPE_AP:
		case WDEV_TYPE_WDS:
			set_default_ap_edca_param(pEdcaParm);
			break;

		case WDEV_TYPE_STA:
		default:
			set_default_sta_edca_param(pEdcaParm);
			break;
		}
	}

	/*if can't search and WmmAcquired is not found*/
	if (obj->bWmmAcquired) {
		entry = wmm_ctrl_get_entry_by_idx(ctrl, obj->WmmIdx);

		if (compare_edca(&entry->edca, pEdcaParm) &&
			(entry->dbdc_idx == dbdc_idx) &&
			(entry->tx_mode == obj->tx_mode)) {
			MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"WMM already ready, keep use  WmmIdx:%d to  ObjIdx: %d\n", obj->WmmIdx, obj->Idx);
			goto end;
		}

		/*release wmm*/
		wmm_ctrl_release_entry(obj);
	}

	/*search exist wmm entry*/
	for (i = 0; i < num; i++) {

		entry = wmm_ctrl_get_entry_by_idx(ctrl, i);
		if (compare_edca(&entry->edca, pEdcaParm) &&
			(entry->dbdc_idx == dbdc_idx) &&
			(entry->tx_mode == obj->tx_mode)) {

			entry->ref_cnt++;
			obj->WmmIdx = i;
			obj->bWmmAcquired = TRUE;
			MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"WMM already created, assign  WmmIdx:%d to  ObjIdx: %d\n", i, obj->Idx);
			goto end;
		}
	}

	/*Bind a new WMM for band*/
	for (i = 0; i < num; i++) {

		entry = wmm_ctrl_get_entry_by_idx(ctrl, i);
		edca = &entry->edca;
		if (!entry->edca.bValid) {
			obj->WmmIdx = i;
			obj->bWmmAcquired = TRUE;
			os_move_mem(edca->Aifsn, pEdcaParm->Aifsn, 4);
			os_move_mem(edca->Cwmax, pEdcaParm->Cwmax, 4);
			os_move_mem(edca->Cwmin, pEdcaParm->Cwmin, 4);
			os_move_mem(edca->Txop, pEdcaParm->Txop, sizeof(USHORT)*4);
			os_move_mem(edca->bACM, pEdcaParm->bACM, 4);
			edca->bValid = TRUE;
			/*hw resouce maintain*/
			entry->ref_cnt = 1;
			entry->dbdc_idx = dbdc_idx;
			entry->tx_mode = obj->tx_mode;
			entry->wmm_set = i;
			MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Create a new WmmIdx:%d to ObjIdx: %d\n", i, obj->Idx);
			goto end;
		}
	}

	/*allocate a default wmm set when obj can't allocate a new once*/
	for (i = 0 ; i < num ; i++) {
		entry = wmm_ctrl_get_entry_by_idx(ctrl, i);

		if (entry->edca.bValid &&
			(entry->dbdc_idx == dbdc_idx) &&
			(entry->tx_mode == obj->tx_mode)) {
			obj->WmmIdx = i;
			obj->bWmmAcquired = TRUE;
			entry->ref_cnt++;
			MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"assign a old WmmIdx:%d to ObjIdx: %d, but not apply new parameter\n", i, obj->Idx);
			goto end;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Allocate WmmSet to ObjIdx:%d  fail since Wmm is full and no WmmSet can match band\n", obj->Idx);
end:
#ifdef MT7626_REDUCE_TX_OVERHEAD
	{
		/* Update WmmIdx of wdev */
		RTMP_ADAPTER *pAd = ctrl->priv;
		pAd->wdev_list[obj->Idx]->WmmIdx = obj->WmmIdx;
	}
#endif /* MT7626_REDUCE_TX_OVERHEAD */

	return entry;
}

/*
*
*/
VOID wmm_ctrl_set_edca(struct hdev_obj *obj)
{
	struct radio_dev *rdev;
	struct hdev_ctrl *ctrl;
	struct wmm_entry *entry;
	RTMP_ADAPTER *pAd;

	rdev = obj->rdev;
	ctrl = (struct hdev_ctrl *)rdev->priv;
	pAd = (RTMP_ADAPTER *) ctrl->priv;

	if (obj->bWmmAcquired) {
		entry = wmm_ctrl_get_entry_by_idx(ctrl, obj->WmmIdx);
		/*set EDCA parameters from AP*/
		AsicSetEdcaParm(pAd, entry, pAd->wdev_list[obj->Idx]);
		/*Update band control */
		RcUpdateWmmEntry(rdev, obj, obj->WmmIdx);
	}
}

/*
 *
*/
INT32 wmm_ctrl_init(struct hdev_ctrl *ctrl, struct wmm_ctrl *wctrl)
{
	INT32 num = wmm_ctrl_get_num(ctrl);
	struct wmm_entry *entries = NULL;
	struct _EDCA_PARM *edca;
	INT32 i = 0;

	os_alloc_mem(NULL, (UCHAR **)&entries, sizeof(struct wmm_entry) * num);

	if (entries == NULL)
		return -1;

	os_zero_mem(entries, sizeof(struct wmm_entry) * num);
	wctrl->entries = entries;
	wctrl->num = num;

	for (i = 0; i < num; i++) {
		edca = &entries[i].edca;
		edca->bValid = FALSE;
		entries[i].wmm_set = i;
		entries[i].ref_cnt = 0;
		entries[i].dbdc_idx = 0;
		entries[i].tx_mode = HOBJ_TX_MODE_TXD;
	}

	return 0;
}


/*
 *
*/
INT32 wmm_ctrl_exit(struct wmm_ctrl *ctrl)
{
	if (ctrl->entries) {
		os_free_mem(ctrl->entries);
		ctrl->entries = NULL;
	}

	return 0;
}


/*
 *
*/
VOID wmm_ctrl_show_entry(struct wmm_ctrl *ctrl)
{
	INT i;
	struct _EDCA_PARM *edca = NULL;
	struct wmm_entry *entry;

	for (i = 0; i < ctrl->num; i++) {
		entry = &ctrl->entries[i];
		edca = &entry->edca;

		if (edca->bValid) {
			MTWF_PRINT("\tEdcaIdx: %d,BandIdx: %d, RfCnt: %d, TXMODE: %d\n",
				entry->wmm_set, entry->dbdc_idx, entry->ref_cnt, entry->tx_mode);
			MTWF_PRINT("\tAifs: %d/%d/%d/%d\n",
					 edca->Aifsn[0], edca->Aifsn[1], edca->Aifsn[2], edca->Aifsn[3]);
			MTWF_PRINT("\tTxop: %d/%d/%d/%d\n",
					 edca->Txop[0], edca->Txop[1], edca->Txop[2], edca->Txop[3]);
			MTWF_PRINT("\tCwmin: %d/%d/%d/%d\n",
					 edca->Cwmin[0], edca->Cwmin[1], edca->Cwmin[2], edca->Cwmin[3]);
			MTWF_PRINT("\tCwmax: %d/%d/%d/%d\n",
					 edca->Cwmax[0], edca->Cwmax[1], edca->Cwmax[2], edca->Cwmax[3]);
		}
	}
}

