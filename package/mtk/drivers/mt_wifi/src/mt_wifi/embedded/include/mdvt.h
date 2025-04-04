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
	mdvt.h
*/
#ifndef __MDVT_H__
#define __MDVT_H__

#ifdef WIFI_MODULE_DVT
INT mdvt_init(struct _RTMP_ADAPTER *ad);
VOID mdvt_exit(struct _RTMP_ADAPTER *ad);
BOOLEAN mdvt_block_command(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);

INT SetMdvtModuleParameterProc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif
#endif /* __MDVT_H__ */
