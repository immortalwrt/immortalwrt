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
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All MAC80211/CFG80211 Related Structure & Definition.

***************************************************************************/
#ifndef __CFG80211_H__
#define __CFG80211_H__

#ifdef RT_CFG80211_SUPPORT

#include <linux/ieee80211.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#define IEEE80211_NUM_BANDS NUM_NL80211_BANDS
#define IEEE80211_BAND_2GHZ NL80211_BAND_2GHZ
#define IEEE80211_BAND_5GHZ NL80211_BAND_5GHZ
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
#define IEEE80211_BAND_6GHZ NL80211_BAND_6GHZ
#endif
#endif

typedef enum _NDIS_HOSTAPD_STATUS {
	Hostapd_Disable = 0,
	Hostapd_EXT,
	Hostapd_CFG
} NDIS_HOSTAPD_STATUS, *PNDIS_HOSTAPD_STATUS;

#ifdef SUPP_SAE_SUPPORT
typedef struct __MTK_PMKSA_EVENT {
	UINT8 pmkid[16];
	UINT8 pmk[64];
	UINT32 pmk_len;
	UINT32 akmp;
	UINT8 aa[ETH_ALEN];
} MTK_PMKSA_EVENT;
#endif

typedef struct __CFG80211_CB {

	/* we can change channel/rate information on the fly so we backup them */
	struct ieee80211_supported_band Cfg80211_bands[IEEE80211_NUM_BANDS];
	struct ieee80211_channel *pCfg80211_Channels;
	struct ieee80211_rate *pCfg80211_Rates;

	/* used in wiphy_unregister */
	struct wireless_dev *pCfg80211_Wdev;

	/* used in scan end */
	struct cfg80211_scan_request *pCfg80211_ScanReq;

	/* monitor filter */
	UINT32 MonFilterFlag;

	/* channel information */
	struct ieee80211_channel ChanInfo[MAX_NUM_OF_CHS];

	/* to protect scan status */
	spinlock_t scan_notify_lock;

} CFG80211_CB;




/*
========================================================================
Routine Description:
	Register MAC80211 Module.

Arguments:
	pAd				- WLAN control block pointer
	pDev			- Generic device interface
	pNetDev			- Network device

Return Value:
	NONE

Note:
	pDev != pNetDev
	#define SET_NETDEV_DEV(net, pdev)	((net)->dev.parent = (pdev))

	Can not use pNetDev to replace pDev; Or kernel panic.
========================================================================
*/
BOOLEAN CFG80211_Register(
	VOID * pAd,
	struct device				*pDev,
	struct net_device			*pNetDev);

#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211_H__ */

/* End of cfg80211.h */
