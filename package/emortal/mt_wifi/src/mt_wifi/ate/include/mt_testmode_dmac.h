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
	mt_testmode_dmac.h
*/

#ifndef __MT_TESTMODE_DMAC_H__
#define __MT_TESTMODE_DMAC_H__


INT32 mtd_ate_mac_cr_restore(struct _RTMP_ADAPTER *pAd);
INT32 mtd_ate_mac_cr_backup_and_set(struct _RTMP_ADAPTER *pAd);
INT32 mtd_ate_ampdu_ba_limit(struct _RTMP_ADAPTER *pAd, UINT8 wmm_idx, UINT8 agg_limit);
INT32 mtd_ate_set_sta_pause_cr(struct _RTMP_ADAPTER *pAd, UINT8 band_idx);
INT32 mtd_ate_set_ifs_cr(struct _RTMP_ADAPTER *pAd, UINT8 band_idx);


#endif /*  __MT_TESTMODE_DMAC_H__ */
