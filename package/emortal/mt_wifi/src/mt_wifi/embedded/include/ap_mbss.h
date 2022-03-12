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
    ap_mbss.h

    Abstract:
    Support multi-BSS function.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Sample Lin  01-02-2007      created
*/


/* Public function list */
INT	Show_MbssInfo_Display_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID mbss_fill_per_band_idx(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);
VOID MBSS_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps);

VOID MBSS_Remove(RTMP_ADAPTER *pAd);

INT32 RT28xx_MBSS_IdxGet(
	IN PRTMP_ADAPTER	pAd,
	IN PNET_DEV			pDev);

#ifdef MT_MAC
INT ext_mbss_hw_cr_enable(PNET_DEV pDev);
INT ext_mbss_hw_cr_disable(PNET_DEV pDev);
#endif

#ifdef DOT11V_MBSSID_SUPPORT
UCHAR bssid_num_to_max_indicator(UCHAR bssid_num);
#endif
