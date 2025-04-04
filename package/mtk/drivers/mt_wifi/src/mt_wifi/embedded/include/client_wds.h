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
	client_wds.h

	Abstract:
*/

#ifndef __CLIENT_WDS_H__
#define __CLIENT_WDS_H__

#include "client_wds_cmm.h"

VOID CliWds_ProxyTabInit(
	IN PRTMP_ADAPTER pAd);

VOID CliWds_ProxyTabDestory(
	IN PRTMP_ADAPTER pAd);

PCLIWDS_PROXY_ENTRY CliWdsEntyAlloc(
	IN PRTMP_ADAPTER pAd);


VOID CliWdsEntyFree(
	IN PRTMP_ADAPTER pAd,
	IN PCLIWDS_PROXY_ENTRY pCliWdsEntry);

VOID CliWdsEnryFreeAid(
	 IN RTMP_ADAPTER *pAd,
	 IN SHORT Aid);

UCHAR *CliWds_ProxyLookup(RTMP_ADAPTER *pAd, UCHAR *pMac);


VOID CliWds_ProxyTabUpdate(
	IN PRTMP_ADAPTER pAd,
	IN SHORT Aid,
	IN PUCHAR pMac);


VOID CliWds_ProxyTabMaintain(
	IN PRTMP_ADAPTER pAd);

#endif /* __CLIENT_WDS_H__ */

