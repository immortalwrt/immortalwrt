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
/****************************************************************************
 ****************************************************************************

     Module Name:
     ap_ids.c

     Abstract:
     IDS definition

     Revision History:
     Who         When          What
     --------    ----------    ----------------------------------------------
 */

VOID RTMPIdsPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

BOOLEAN RTMPSpoofedMgmtDetection(
	IN RTMP_ADAPTER * pAd,
	IN RX_BLK * rxblk);

VOID RTMPConflictSsidDetection(
	IN PRTMP_ADAPTER	pAd,
	IN PUCHAR			pSsid,
	IN UCHAR			SsidLen,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2);

BOOLEAN RTMPReplayAttackDetection(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR * pAddr2,
	IN RX_BLK * rxblk);

VOID RTMPUpdateStaMgmtCounter(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			type);

VOID RTMPClearAllIdsCounter(
	IN PRTMP_ADAPTER	pAd);

VOID RTMPIdsStart(
	IN PRTMP_ADAPTER	pAd);

VOID RTMPIdsStop(
	IN PRTMP_ADAPTER	pAd);

VOID rtmp_read_ids_from_file(
	IN  PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);

