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
#ifndef __MWDS_H__
#define __MWDS_H__
/*
 ***************************************************************************
 ***************************************************************************


    Module Name:
	mwds.h

    Abstract:
    This is MWDS feature used to process those 4-addr of connected APClient or STA.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef MWDS
#include "rtmp_def.h"

#define MWDS_SUPPORT(B0)            ((B0)&0x80)

VOID MWDSAPPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry);

VOID MWDSAPPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry);

#ifdef APCLI_SUPPORT
VOID MWDSAPCliPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN PSTA_ADMIN_CONFIG pApCliEntry,
	IN PMAC_TABLE_ENTRY pEntry);

VOID MWDSAPCliPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN PSTA_ADMIN_CONFIG pApCliEntry,
	IN PMAC_TABLE_ENTRY pEntry);
#endif /* APCLI_SUPPORT */
INT MWDSEnable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevOpen);

INT MWDSDisable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevClose);

INT Set_Ap_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg);

INT Set_ApCli_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg);

VOID rtmp_read_MWDS_from_file(
	IN  PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer);

#endif /* MWDS */
#endif /* __MWDS_H__*/

