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
	hdev.c
*/
#include "rt_config.h"
#include "hdev/hdev.h"

/*Local functions*/
VOID HdevHwResourceInit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;

	os_zero_mem(pHwResourceCfg, sizeof(HD_RESOURCE_CFG));
	/*initial radio control*/
	rc_init(ctrl);
	/*initial wmm control*/
	wmm_ctrl_init(ctrl, &pHwResourceCfg->wmm_ctrl);
	/*initial wtbl control*/
	WtcInit(ctrl);
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	/* initial twt control*/
	twt_ctrl_init(ctrl);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#ifdef DOT11_HE_AX
	/* initial bss color table */
	bss_color_table_init(ctrl);
#endif
}

VOID HdevHwResourceExit(struct hdev_ctrl *ctrl)
{
	UCHAR i;
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;

	WtcExit(ctrl);

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++)
		HdevExit(ctrl, i);

	wmm_ctrl_exit(&pHwResourceCfg->wmm_ctrl);

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	twt_ctrl_exit(ctrl);
#endif /* WIFI_TWT_SUPPORT */
	bss_color_table_deinit(ctrl);
#endif /* DOT11_HE_AX */
}


static VOID HdevHwResourceShow(HD_RESOURCE_CFG *pHwResourceCfg)
{
	MTWF_PRINT("band_num: %d\n", pHwResourceCfg->concurrent_bands);
	RcRadioShow(pHwResourceCfg);
}


/*
*
*/
VOID HdevObjShow(struct hdev_obj *obj)
{
	HD_REPT_ENRTY *pEntry;

	MTWF_PRINT("band_id\t: %d\n", obj->rdev->pRadioCtrl->BandIdx);
	MTWF_PRINT("obj_id\t: %d\n", obj->Idx);
	MTWF_PRINT("omac_id\t: %d\n", obj->OmacIdx);
	MTWF_PRINT("wmmcap\t: %d\n", obj->bWmmAcquired);
	MTWF_PRINT("wmm_idx\t: %d\n", obj->WmmIdx);
	MTWF_PRINT("tx_mode\t: %s\n", obj->tx_mode ? "TXCMD" : "TXD");
	DlListForEach(pEntry, &obj->RepeaterList, struct _HD_REPT_ENRTY, list) {
		MTWF_PRINT("\trept_id: %d, omac_id:%x\n", pEntry->CliIdx, pEntry->ReptOmacIdx);
	}
}

/*
*
*/
void bw_2_str(UCHAR bw, CHAR *bw_str, UINT max_str_size);
void extcha_2_str(UCHAR extcha, CHAR *ec_str, UINT max_str_size);

static VOID HdevShow(struct radio_dev *rdev)
{
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;
	USHORT pm = pRadioCtrl->PhyMode;
	UCHAR ch = pRadioCtrl->Channel;
	UCHAR c1 = pRadioCtrl->CentralCh;
	UCHAR bw = pRadioCtrl->Bw;
	UCHAR ech = pRadioCtrl->ExtCha;
	UCHAR *pstr = NULL;
	CHAR str[32] = "";
	INT ret;

	/*for 2.4G check*/
	/* do not change sequence due to 6GHz might include AC/GN then confused */
	if (WMODE_CAP_6G(pm)) {
		ret = sprintf(str, "6GHz");
		if (ret < 0)
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"str sprintf error\n");
	} else if (WMODE_CAP_2G(pm)) {
		ret = sprintf(str, "2.4GHz");
		if (ret < 0)
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"str sprintf error\n");
	} else {
		ret = sprintf(str, "5GHz");
		if (ret < 0)
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"str sprintf error\n");
	}

	MTWF_PRINT("==========%s band==========\n", str);
	pstr = wmode_2_str(pm);

	if (pstr != NULL) {
		MTWF_PRINT("wmode\t: %s\n", pstr);
		os_free_mem(pstr);
	}
	MTWF_PRINT("ch\t: %d\n", ch);
#ifdef DOT11_N_SUPPORT
	if (WMODE_CAP_N(pm)) {
		bw_2_str(bw, str, sizeof(str));
		MTWF_PRINT("bw\t: %s\n", str);
		extcha_2_str(ech, str, sizeof(str));
		MTWF_PRINT("extcha\t: %s\n", str);
		MTWF_PRINT("cen_ch\t: %d\n", c1);
	}
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(pm))
		MTWF_PRINT("cen ch2\t: %d\n", pRadioCtrl->Channel2);
#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/

	MTWF_PRINT("band_id\t: %d\n", pRadioCtrl->BandIdx);
	MTWF_PRINT("obj_num\t: %d\n", rdev->DevNum);
	MTWF_PRINT("state\t: %d\n", pRadioCtrl->CurStat);
}


/*
*
*/
static VOID HdevDel(struct radio_dev *rdev)
{
	struct hdev_obj *obj, *pTemp;

	DlListForEachSafe(obj, pTemp, &rdev->DevObjList, struct hdev_obj, list) {
		HdevObjDel(rdev, obj);
	}
	DlListInit(&rdev->DevObjList);
}


/*Export function*/
/*
 *
*/
VOID HdevObjAdd(struct radio_dev *rdev, struct hdev_obj *obj)
{
	DlListAddTail(&rdev->DevObjList, &obj->list);
	obj->rdev = rdev;
	DlListInit(&obj->RepeaterList);
	rdev->DevNum++;
	if (obj->state != HOBJ_STATE_NONE) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"obj state is not free! need to check!\n");
	}
	obj->state = HOBJ_STATE_USED;
}


/*
 *
*/
VOID HdevObjDel(struct radio_dev *rdev, struct hdev_obj *obj)
{
	obj->state = HOBJ_STATE_NONE;
	obj->rdev = NULL;
	DlListDel(&obj->list);
	DlListInit(&obj->RepeaterList);
	rdev->DevNum--;
}



/*
*
*/

INT32 HdevInit(struct hdev_ctrl *ctrl, UCHAR HdevIdx, RADIO_CTRL *pRadioCtrl)
{
	struct radio_dev *rdev = NULL;

	if (HdevIdx >= DBDC_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "HdevIdx:%d >= %d\n", HdevIdx, DBDC_BAND_NUM);
		return 0;
	}

	rdev = &ctrl->rdev[HdevIdx];
	os_zero_mem(rdev, sizeof(struct radio_dev));
	DlListInit(&rdev->DevObjList);
	rdev->pRadioCtrl = pRadioCtrl;
	rdev->priv =  ctrl;
	rdev->Idx = HdevIdx;
	rdev->DevNum = 0;
	if (pRadioCtrl->BandIdx == 1 && (hc_get_asic_cap(ctrl) & fASIC_CAP_SEPARATE_DBDC))
		rdev->omac_ctrl = &ctrl->HwResourceCfg.OmacBssCtl[1];
	else
		rdev->omac_ctrl = &ctrl->HwResourceCfg.OmacBssCtl[0];
	return 0;
}


/*
*
*/

INT32 HdevExit(struct hdev_ctrl *ctrl, UCHAR HdevIdx)
{
	struct radio_dev *rdev = NULL;

	if (HdevIdx >= DBDC_BAND_NUM) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "HdevIdx:%d >= %d\n", HdevIdx, DBDC_BAND_NUM);
		return 0;
	}

	rdev = &ctrl->rdev[HdevIdx];
	HdevDel(rdev);
	os_zero_mem(rdev, sizeof(struct radio_dev));
	return 0;
}

/*
*
*/
VOID HdevCfgShow(struct hdev_ctrl *ctrl)
{
	UCHAR i;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	HdevHwResourceShow(pHwResource);

	for (i = 0; i < pHwResource->concurrent_bands; i++)
		HdevShow(&ctrl->rdev[i]);
}

/*
*
*/
BOOLEAN hdev_obj_state_ready(struct hdev_obj *obj)
{
	if (obj == NULL)
		return FALSE;
	if (obj->state == HOBJ_STATE_USED)
		return TRUE;
	else
		return FALSE;
}
