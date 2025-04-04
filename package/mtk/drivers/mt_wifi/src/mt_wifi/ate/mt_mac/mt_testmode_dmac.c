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
	mt_testmode_dmac.c

*/

#ifdef COMPOS_TESTMODE_WIN
#include "config.h"
#else
#include "rt_config.h"
#endif

INT32 mtd_ate_mac_cr_restore(RTMP_ADAPTER *pAd)
{

	return 0;
}

INT32 mtd_ate_mac_cr_backup_and_set(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	NdisZeroMemory(&ATECtrl->bk_cr, sizeof(struct _TESTMODE_BK_CR)*MAX_TEST_BKCR_NUM);
	return 0;
}

UINT32 agg_cnt_array[] = {AGG_AALCR0, AGG_AALCR1, AGG_AALCR2, AGG_AALCR3};
INT32 mtd_ate_ampdu_ba_limit(RTMP_ADAPTER *pAd, UINT8 wmm_idx, UINT8 agg_limit)
{
	UINT32 value;

	if (wmm_idx > 3) {
		MTWF_DBG(pAd,DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid WmmIdx=%d, set to all!\n",wmm_idx);
		wmm_idx = 0xFF;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"WmmIdx=%d\n", wmm_idx);
	value = ((agg_limit & 0x3F) << 24)
		| ((agg_limit & 0x3F) << 16)
		| ((agg_limit & 0x3F) << 8)
		| ((agg_limit & 0x3F) << 0);

	if (wmm_idx <= 3)
		MAC_IO_WRITE32(pAd->hdev_ctrl, agg_cnt_array[wmm_idx], value);
	else {
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR0, value);
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR1, value);
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR2, value);
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR3, value);
	}

	value = 0x0;
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AWSCR0, value);

	return 0;
}

INT32 mtd_ate_set_sta_pause_cr(RTMP_ADAPTER *pAd, UINT8 ac_idx)
{
	INT32 ret = 0;
	UINT32 value;

	/* Set station pause CRs to 0 for TX after reset WTBL */
	/* The CR meaning in normal mode is that stop to TX packet when STA disconnect */
	value = 0x0;
	MAC_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE0, value);
	MAC_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE1, value);
	MAC_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE2, value);
	MAC_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE3, value);

	return ret;
}

INT mtd_ate_set_ifs_cr(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	INT32 ret = 0;
	struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(pAd, band_idx, ipg_param);
	UINT16 slot_time, sifs_time;
	UINT32 txv_time = 0, i2t_chk_time = 0, tr2t_chk_time = 0;
	UINT32 value = 0;

	slot_time = ipg_param->slot_time;
	sifs_time = ipg_param->sifs_time;
	/* in uint of ns */
	MAC_IO_READ32(pAd, TMAC_ATCR, &txv_time);
	txv_time *= NORMAL_CLOCK_TIME;
	i2t_chk_time = (UINT32)(slot_time * 1000) - txv_time - BBP_PROCESSING_TIME;
	tr2t_chk_time = (UINT32)(sifs_time * 1000) - txv_time - BBP_PROCESSING_TIME;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"txv_time=%dns, i2t_chk_time=%dns, tr2t_chk_time=%dns\n",
		 txv_time, i2t_chk_time, tr2t_chk_time);
	i2t_chk_time /= NORMAL_CLOCK_TIME;
	tr2t_chk_time /= NORMAL_CLOCK_TIME;

	if (band_idx == TESTMODE_BAND0) {
		MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TRCR0, &value);
		value = (value & 0xFE00FE00)
				| ((i2t_chk_time & 0x1FF) << 16)
				| ((tr2t_chk_time & 0x1FF) << 0);
		MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TRCR0, value);
	} else if (band_idx == TESTMODE_BAND1) {
		MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TRCR1, &value);
		value = (value & 0xFE00FE00)
				| ((i2t_chk_time & 0x1FF) << 16)
				| ((tr2t_chk_time & 0x1FF) << 0);
		MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TRCR1, value);
	} else {
		MTWF_DBG(pAd,DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Invalid band_idx!!\n");
		return FALSE;
	}

	return ret;
}
