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

    Abstract:


 */

#include "bgnd_scan_cmm.h"

#ifndef __BGND_SCAN_H__
#define __BGND_SCAN_H__

#define GET_BGND_STATE(_pAd, _state) \
	((_pAd->BgndScanCtrl.BgndScanStatMachine.CurrState == _state))

VOID BackgroundScanCancelAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanStartAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanTimeoutAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID dedicated_rx_hist_scan_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundSwitchChannelAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundChannelSwitchAnnouncementAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanPartialAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanWaitAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanNextChannel(IN PRTMP_ADAPTER pAd, IN UINT8 ScanType);
VOID BackgroundScanInit(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);
VOID BackgroundScanDeInit(IN PRTMP_ADAPTER pAd);
VOID BackgroundScanStart(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN UINT8 BgndscanType);
void BackgroundScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID BackgroundScanTest(IN PRTMP_ADAPTER pAd,	IN MT_BGND_SCAN_CFG BgndScanCfg);

VOID ChannelQualityDetection(IN PRTMP_ADAPTER pAd);
VOID mt_bgnd_scan(IN PRTMP_ADAPTER pAd, IN UCHAR reason, IN UCHAR bgnd_scan_type);
VOID mt_off_ch_scan(IN PRTMP_ADAPTER pAd, IN UCHAR reason, IN UCHAR bgnd_scan_type);
VOID bgnd_scan_cr_init(IN PRTMP_ADAPTER pAd);
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
VOID dfs_zero_wait_ch_init_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif
#if (RDD_2_SUPPORTED == 1)
VOID mt_off_ch_scan_dedicated(IN PRTMP_ADAPTER pAd, IN UCHAR reason, IN UCHAR bgnd_scan_type);
VOID bgnd_scan_ipi_cr_init(IN PRTMP_ADAPTER pAd);
#endif

/* Dedicated RX */
NDIS_STATUS set_dfs_dedicated_rx_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
NDIS_STATUS set_dedicated_rx_hist_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);

#ifdef IPI_SCAN_SUPPORT
NDIS_STATUS set_ipi_scan_ctrl_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
NDIS_STATUS set_ipi_scan_hist_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
#endif

VOID dedicated_rx_hist_scan_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

#ifdef MT_DFS_SUPPORT
VOID DedicatedZeroWaitStartAction(
		IN RTMP_ADAPTER *pAd,
		IN MLME_QUEUE_ELEM *Elem);
VOID DedicatedZeroWaitRunningAction(
		IN RTMP_ADAPTER *pAd,
		IN MLME_QUEUE_ELEM *Elem);
VOID DedicatedZeroWaitStop(
		IN RTMP_ADAPTER *pAd, BOOLEAN apply_cur_ch);
#endif
VOID BfSwitch(IN PRTMP_ADAPTER pAd, IN UCHAR enabled);
VOID MuSwitch(IN PRTMP_ADAPTER pAd, IN UCHAR enabled);
#endif /* __BGND_SCAN_H__ */
