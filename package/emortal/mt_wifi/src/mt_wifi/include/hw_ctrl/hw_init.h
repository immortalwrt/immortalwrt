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
	hw_init.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __HW_INIT_H__
#define __HW_INIT_H__

struct _RTMP_ADAPTER;

typedef struct {
	USHORT Dummy;/* empty struct will cause build error in testmode win */
} HIF_INFO_T;

/*HW related init*/
INT32 WfTopInit(struct _RTMP_ADAPTER *pAd);
INT32 WfHifInit(struct _RTMP_ADAPTER *pAd);
INT32 WfMcuInit(struct _RTMP_ADAPTER *pAd);
INT32 WfMacInit(struct _RTMP_ADAPTER *pAd);
INT32 WfEPROMInit(struct _RTMP_ADAPTER *pAd);
INT32 WfPhyInit(struct _RTMP_ADAPTER *pAd);

/*SW related init*/
INT32 WfSysPreInit(struct _RTMP_ADAPTER *pAd);
INT32 WfSysPosExit(struct _RTMP_ADAPTER *pAd);
INT32 WfSysCfgInit(struct _RTMP_ADAPTER *pAd);
INT32 WfSysCfgExit(struct _RTMP_ADAPTER *pAd);

/*OS dependence function*/
INT32 WfHifSysInit(struct _RTMP_ADAPTER *pAd, HIF_INFO_T *pHifInfo);
INT32 WfHifSysExit(struct _RTMP_ADAPTER *pAd);
INT32 WfMcuSysInit(struct _RTMP_ADAPTER *pAd);
INT32 WfMcuSysExit(struct _RTMP_ADAPTER *pAd);
INT32 WfEPROMSysInit(struct _RTMP_ADAPTER *pAd);
INT32 WfEPROMSysExit(struct _RTMP_ADAPTER *pAd);

/*Global*/
INT32 WfInit(struct _RTMP_ADAPTER *pAd);

#endif
