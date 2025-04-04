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

#include "ap_autoChSel_cmm.h"

#ifndef __AUTOCHSELECT_H__
#define __AUTOCHSELECT_H__

enum _WIFI_CH_BAND {
	WIFI_CH_BAND_2G = 0,
	WIFI_CH_BAND_5G,
	WIFI_CH_BAND_6G,
	WIFI_CH_BAND_NUM,
};

ULONG AutoChBssSearchWithSSID(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR Bssid,
	IN PUCHAR pSsid,
	IN UCHAR SsidLen,
	IN UCHAR Channel,
	IN struct wifi_dev *pwdev);

VOID APAutoChannelInit(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev);

VOID UpdateChannelInfo(
	IN PRTMP_ADAPTER pAd,
	IN int ch,
	IN ChannelSel_Alg Alg,
	IN struct wifi_dev *pwdev);
VOID UpdatePreACSInfo(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#define IS_V10_BOOTACS_INVALID(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10BootACSValid == TRUE)

#define SET_V10_BOOTACS_INVALID(_pAd, valid) \
	(_pAd->CommonCfg.DfsParameter.bV10BootACSValid = valid)

#define IS_V10_APINTF_DOWN(_pAd) \
		 (_pAd->CommonCfg.DfsParameter.bV10APInterfaceDownEnbl == TRUE)

#define SET_V10_APINTF_DOWN(_pAd, valid) \
		(_pAd->CommonCfg.DfsParameter.bV10APInterfaceDownEnbl = valid)

#define IS_V10_W56_AP_DOWN_ENBLE(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10W56APDownEnbl == TRUE)

#define SET_V10_W56_AP_DOWN(_pAd, valid) \
	(_pAd->CommonCfg.DfsParameter.bV10W56APDownEnbl = valid)

#define IS_V10_W56_AP_UP_CH_UPDATE(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10APUpChUpdate == TRUE)

#define SET_V10_W56_AP_UP_CH_UPDATE(_pAd, valid) \
	(_pAd->CommonCfg.DfsParameter.bV10APUpChUpdate = valid)

#define SET_V10_AP_BCN_UPDATE_ENBL(_pAd, enable) \
	(_pAd->CommonCfg.DfsParameter.bV10APBcnUpdateEnbl = enable)

#define IS_V10_AP_BCN_UPDATE_ENBL(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10APBcnUpdateEnbl == TRUE)

#define SET_V10_AP_ZWDFS_ACS_ENBL(_pAd, enable) \
	(_pAd->CommonCfg.DfsParameter.bV10ZWDFSACSEnbl = enable)

#define IS_V10_AP_ZWDFS_ACS_ENBL(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10ZWDFSACSEnbl == TRUE)


VOID AutoChannelSkipListAppend(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Ch);

VOID AutoChannelSkipChannels(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR size,
	IN UINT16 grpStart);

VOID AutoChannelSkipListClear(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsV10ACSMarkChnlConsumed(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel);
#endif

ULONG AutoChBssInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR ChannelNo,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi,
	IN struct wifi_dev *pwdev);

VOID AutoChBssTableInit(
	IN PRTMP_ADAPTER pAd);

VOID ChannelInfoInit(
	IN PRTMP_ADAPTER pAd);

VOID AutoChBssTableDestroy(
	IN PRTMP_ADAPTER pAd);

VOID ChannelInfoDestroy(
	IN PRTMP_ADAPTER pAd);

VOID CheckPhyModeIsABand(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx);

UCHAR SelectBestChannel(
	IN PRTMP_ADAPTER pAd,
	IN ChannelSel_Alg Alg,
	IN struct wifi_dev *pwdev);
UCHAR APAutoSelectChannel(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev,
	IN ChannelSel_Alg Alg,
	IN BOOLEAN IsABand);

UCHAR MTAPAutoSelectChannel(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *pwdev,
	IN ChannelSel_Alg Alg,
	IN BOOLEAN IsABand);

#ifdef AP_SCAN_SUPPORT
VOID AutoChannelSelCheck(
	IN PRTMP_ADAPTER pAd);
#endif /* AP_SCAN_SUPPORT */

VOID AutoChSelBuildChannelList(
	IN RTMP_ADAPTER * pAd,
	IN BOOLEAN IsABand,
	IN struct wifi_dev *pwdev);

VOID AutoChSelBuildChannelListFor2G(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev);

VOID AutoChSelBuildChannelListFor56G(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev,
	IN UCHAR ucChBand);

VOID AutoChSelUpdateChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN BOOLEAN IsABand,
	IN struct wifi_dev *pwdev);

CHAR AutoChSelFindScanChIdx(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *pwdev,
	IN CHAR LastScanChIdx);

VOID AutoChSelScanNextChannel(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *pwdev);

VOID AutoChSelScanReqAction(
	IN RTMP_ADAPTER * pAd,
	IN MLME_QUEUE_ELEM * pElem);

VOID AutoChSelScanTimeoutAction(
	IN RTMP_ADAPTER * pAd,
	IN MLME_QUEUE_ELEM * pElem);

VOID AutoChSelScanStart(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *pwdev);

VOID AutoChSelScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID AutoChSelStateMachineInit(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR BandIdx,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID AutoChSelInit(
	IN PRTMP_ADAPTER pAd);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
UINT8 SelectBestV10Chnl_From_List(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR band_idx);
#endif

#ifdef OFFCHANNEL_SCAN_FEATURE
VOID ChannelInfoResetNew(
	IN PRTMP_ADAPTER pAd);
#endif
VOID AutoChSelRelease(
	IN PRTMP_ADAPTER pAd);

VOID auto_ch_select_set_cfg(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *buffer);

#ifdef CONFIG_6G_SUPPORT
VOID auto_ch_select_PSC_cfg(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *buffer);
#endif

VOID auto_ch_select_reset_sm(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev);

NDIS_STATUS set_idle_pwr_test(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);
#endif /* __AUTOCHSELECT_H__ */

