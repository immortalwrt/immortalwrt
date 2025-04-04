/*
 * Copyright (c) [2021], MediaTek Inc. All rights reserved.
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
/****************************************************************************
 ****************************************************************************

    Abstract:

 */

#include "rt_config.h"
#ifndef __DYNWMM_H__
#define __DYNWMM_H__

enum _ENUM_DYN_WMM_MODE {
	DYN_WMM_DISABLE = 0,
	DYN_WMM_ENABLE,
	DYN_WMM_ENABLE_DEBUG_LOG,
	DYN_WMM_DISABLE_DEBUG_LOG,
	DYN_WMM_MODE_MAX,
};

enum _ENUM_DYN_WMM_CMD_TYPE {
	DYN_WMM_CMD_ENABLE = 1,
	DYN_WMM_CMD_SEND_DATA,
	DYN_WMM_CMD_MAX,
};

struct _CMD_DYNAMIC_WMM_ENABLE {
	UINT_8 u1BandIdx;
	UINT_8 u1DynWmmEnable;
	UINT_8 au1Reserved[2];
};

struct _CMD_DYMWMM_CTRL_DATA_T {
	UINT8  u1BandIdx;
	UINT8  ActiveSTA;
	BOOL   fgRxOnly;
	UINT8  u1Reserved;
};

struct DYNAMIC_WMM_CTRL {
	BOOL   DynWmmEnable[DBDC_BAND_NUM];	/* 0:Disable, 1:Enable */
	UINT32 OneSecTxByteCount[DBDC_BAND_NUM];
	UINT32 OneSecRxByteCount[DBDC_BAND_NUM];
};

VOID SetDynamicWmmEnable(RTMP_ADAPTER *pAd, UCHAR BandIdx, UINT32 value);
VOID DynWmm_init(RTMP_ADAPTER *pAd);
VOID SetDynamicWmmProcess(RTMP_ADAPTER *pAd);
INT DynWmmSetDynamicWmmEnable(RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT32 value);

#endif /* __DYNWMM_H__ */

