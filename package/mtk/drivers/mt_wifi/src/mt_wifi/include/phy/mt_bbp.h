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
	mt_bbp.h
*/

#ifndef __MT_BBP_H__
#define __MT_BBP_H__

INT32 MTShowAllBBP(struct _RTMP_ADAPTER *pAd);
INT32 MTShowPartialBBP(struct _RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End);
INT mt_phy_probe(struct _RTMP_ADAPTER *pAd);

#endif /* __MT_BBP_H__ */

