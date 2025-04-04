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
 *	Abstract:
 *
 *	All related CFG80211 function body.
 *
 *	History:
 *		1. 2009/09/17	Sample Lin
 *			(1) Init version.
 *		2. 2009/10/27	Sample Lin
 *			(1) Do not use ieee80211_register_hw() to create virtual interface.
 *				Use wiphy_register() to register nl80211 command handlers.
 *			(2) Support iw utility.
 *		3. 2009/11/03	Sample Lin
 *			(1) Change name MAC80211 to CFG80211.
 *			(2) Modify CFG80211_OpsChannelSet().
 *			(3) Move CFG80211_Register()/CFG80211_UnRegister() to open/close.
 *		4. 2009/12/16	Sample Lin
 *			(1) Patch for Linux 2.6.32.
 *			(2) Add more supported functions in CFG80211_Ops.
 *		5. 2010/12/10	Sample Lin
 *			(1) Modify for OS_ABL.
 *		6. 2011/04/19	Sample Lin
 *			(1) Add more supported functions in CFG80211_Ops v33 ~ 38.
 *
 *	Note:
 *		The feature is supported only in "LINUX" 2.6.28 ~ 2.6.38.
 *
 ***************************************************************************/


#define RTMP_MODULE_OS

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include "rt_config.h"
#include <net/netlink.h>


#if (KERNEL_VERSION(2, 6, 28) <= LINUX_VERSION_CODE)
#ifdef RT_CFG80211_SUPPORT

/* 36 ~ 64, 100 ~ 136, 140 ~ 161 */
#define CFG80211_NUM_OF_CHAN_5GHZ	34

#ifdef OS_ABL_FUNC_SUPPORT
/*
 *	Array of bitrates the hardware can operate with
 *	in this band. Must be sorted to give a valid "supported
 *	rates" IE, i.e. CCK rates first, then OFDM.
 *
 *	For HT, assign MCS in another structure, ieee80211_sta_ht_cap.
 */
const struct ieee80211_rate Cfg80211_SupRate[12] = {
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 10,    /* bitrate in units of 100 Kbps */
		.hw_value = 0,
		.hw_value_short = 0,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 20,
		.hw_value = 1,
		.hw_value_short = 1,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 55,
		.hw_value = 2,
		.hw_value_short = 2,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 110,
		.hw_value = 3,
		.hw_value_short = 3,
	},
	{
		.flags = 0,
		.bitrate = 60,
		.hw_value = 4,
		.hw_value_short = 4,
	},
	{
		.flags = 0,
		.bitrate = 90,
		.hw_value = 5,
		.hw_value_short = 5,
	},
	{
		.flags = 0,
		.bitrate = 120,
		.hw_value = 6,
		.hw_value_short = 6,
	},
	{
		.flags = 0,
		.bitrate = 180,
		.hw_value = 7,
		.hw_value_short = 7,
	},
	{
		.flags = 0,
		.bitrate = 240,
		.hw_value = 8,
		.hw_value_short = 8,
	},
	{
		.flags = 0,
		.bitrate = 360,
		.hw_value = 9,
		.hw_value_short = 9,
	},
	{
		.flags = 0,
		.bitrate = 480,
		.hw_value = 10,
		.hw_value_short = 10,
	},
	{
		.flags = 0,
		.bitrate = 540,
		.hw_value = 11,
		.hw_value_short = 11,
	},
};
#endif /* OS_ABL_FUNC_SUPPORT */

static const UINT32 CipherSuites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
#ifdef DOT11W_PMF_SUPPORT
	WLAN_CIPHER_SUITE_AES_CMAC,
#ifdef HOSTAPD_SUITEB_SUPPORT
	WLAN_CIPHER_SUITE_BIP_GMAC_256,
#endif
#endif /*DOT11W_PMF_SUPPORT*/
	WLAN_CIPHER_SUITE_GCMP,
#if (KERNEL_VERSION(4, 0, 0) <= LINUX_VERSION_CODE)
	WLAN_CIPHER_SUITE_CCMP_256,
#ifdef HOSTAPD_SUITEB_SUPPORT
	WLAN_CIPHER_SUITE_GCMP_256,
#endif
#endif

};


/* get RALINK pAd control block in 80211 Ops */
#define MAC80211_PAD_GET(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL) {											\
			MTWF_DBG(__pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,								\
					 "80211> pAd = NULL!");	\
			return -EINVAL;											\
		}															\
	}

#define MAC80211_PAD_GET_NO_RV(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL) {											\
			MTWF_DBG(__pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,								\
					 "80211> pAd = NULL!");	\
			return;											\
		}															\
	}

#define MAC80211_PAD_GET_RETURN_NULL(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL) {											\
			MTWF_DBG(__pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,								\
					 "80211> pAd = NULL!");	\
			return NULL;											\
		}															\
	}


/*
 * ========================================================================
 * Routine Description:
 *	Set channel.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pChan			- Channel information
 *	ChannelType		- Channel type
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: set channel, set freq
 *
 *	enum nl80211_channel_type {
 *		NL80211_CHAN_NO_HT,
 *		NL80211_CHAN_HT20,
 *		NL80211_CHAN_HT40MINUS,
 *		NL80211_CHAN_HT40PLUS
 *	};
 * ========================================================================
 */
#if (KERNEL_VERSION(3, 6, 0) > LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE)
static int CFG80211_OpsChannelSet(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pDev,
	IN struct ieee80211_channel		*pChan,
	IN enum nl80211_channel_type	ChannelType)

#else
static int CFG80211_OpsChannelSet(
	IN struct wiphy					*pWiphy,
	IN struct ieee80211_channel		*pChan,
	IN enum nl80211_channel_type	ChannelType)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;
	CFG80211_CB *p80211CB;
	CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;
	UINT32 ChanId;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	CFG80211DBG(DBG_LVL_INFO, ("80211> Channel = %d, Type = %d\n", ChanId, ChannelType));
	/* init */
	memset(&ChanInfo, 0, sizeof(ChanInfo));
	ChanInfo.ChanId = ChanId;
	p80211CB = NULL;
	RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

	if (p80211CB == NULL) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> p80211CB == NULL!\n"));
		return 0;
	}

	ChanInfo.IfType = pDev->ieee80211_ptr->iftype;

	if (ChannelType == NL80211_CHAN_NO_HT)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
	else if (ChannelType == NL80211_CHAN_HT20)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT20;
	else if (ChannelType == NL80211_CHAN_HT40MINUS)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40MINUS;
	else if (ChannelType == NL80211_CHAN_HT40PLUS)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40PLUS;

	ChanInfo.MonFilterFlag = p80211CB->MonFilterFlag;
	/* set channel */
	RTMP_DRIVER_80211_CHAN_SET(pAd, &ChanInfo);
	return 0;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0) */

/*
 * ========================================================================
 * Routine Description:
 *	Change type/configuration of virtual interface.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	IfIndex			- Interface index
 *	Type			- Interface type, managed/adhoc/ap/station, etc.
 *	pFlags			- Monitor flags
 *	pParams			- Mesh parameters
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: set type, set monitor
 * ========================================================================
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static int CFG80211_OpsVirtualInfChg(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNetDevIn,
	IN enum nl80211_iftype			Type,
	struct vif_params				*pParams)
#elif (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
static int CFG80211_OpsVirtualInfChg(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNetDevIn,
	IN enum nl80211_iftype			Type,
	IN UINT32							*pFlags,
	struct vif_params				*pParams)
#else
static int CFG80211_OpsVirtualInfChg(
	IN struct wiphy					*pWiphy,
	IN int							IfIndex,
	IN enum nl80211_iftype			Type,
	IN UINT32							*pFlags,
	struct vif_params				*pParams)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;
	CFG80211_CB *pCfg80211_CB = NULL;
	struct net_device *pNetDev;
	CMD_RTPRIV_IOCTL_80211_VIF_PARM VifInfo;
	UINT oldType = pNetDevIn->ieee80211_ptr->iftype;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	CFG80211DBG(DBG_LVL_INFO, ("80211> IfTypeChange %d ==> %d\n", oldType, Type));
	MAC80211_PAD_GET(pAd, pWiphy);
	/* sanity check */
#ifdef CONFIG_STA_SUPPORT

	if ((Type != NL80211_IFTYPE_ADHOC) &&
		(Type != NL80211_IFTYPE_STATION) &&
		(Type != NL80211_IFTYPE_MONITOR) &&
		(Type != NL80211_IFTYPE_AP)
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
		&& (Type != NL80211_IFTYPE_P2P_CLIENT)
		&& (Type != NL80211_IFTYPE_P2P_GO)
#endif /* LINUX_VERSION_CODE 2.6.37 */
	   )
#endif /* CONFIG_STA_SUPPORT */
	{
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "80211> Wrong interface type %d!\n", Type);
		return -EINVAL;
	} /* End of if */

	/* update interface type */
#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
	pNetDev = pNetDevIn;
#else
	pNetDev = __dev_get_by_index(&init_net, IfIndex);
#endif /* LINUX_VERSION_CODE */

	if (pNetDev == NULL)
		return -ENODEV;

	memset(&VifInfo, 0, sizeof(VifInfo));
	pNetDev->ieee80211_ptr->iftype = Type;
	VifInfo.net_dev = pNetDev;
	VifInfo.newIfType = Type;
	VifInfo.oldIfType = oldType;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	if (pParams != NULL) {
		VifInfo.MonFilterFlag = 0;

		if (((pParams->flags) & NL80211_MNTR_FLAG_FCSFAIL) == NL80211_MNTR_FLAG_FCSFAIL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_FCSFAIL;

		if (((pParams->flags) & NL80211_MNTR_FLAG_PLCPFAIL) == NL80211_MNTR_FLAG_PLCPFAIL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_PLCPFAIL;

		if (((pParams->flags) & NL80211_MNTR_FLAG_CONTROL) == NL80211_MNTR_FLAG_CONTROL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_CONTROL;

		if (((pParams->flags) & NL80211_MNTR_FLAG_OTHER_BSS) == NL80211_MNTR_FLAG_OTHER_BSS)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_OTHER_BSS;
	}
#else
	if (pFlags != NULL) {
		VifInfo.MonFilterFlag = 0;

		if (((*pFlags) & NL80211_MNTR_FLAG_FCSFAIL) == NL80211_MNTR_FLAG_FCSFAIL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_FCSFAIL;

		if (((*pFlags) & NL80211_MNTR_FLAG_FCSFAIL) == NL80211_MNTR_FLAG_PLCPFAIL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_PLCPFAIL;

		if (((*pFlags) & NL80211_MNTR_FLAG_CONTROL) == NL80211_MNTR_FLAG_CONTROL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_CONTROL;

		if (((*pFlags) & NL80211_MNTR_FLAG_CONTROL) == NL80211_MNTR_FLAG_OTHER_BSS)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_OTHER_BSS;
	}
#endif
	RTMP_DRIVER_80211_VIF_CHG(pAd, &VifInfo);
	/*CFG_TODO*/
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);
	if (pCfg80211_CB == NULL) {
			CFG80211DBG(DBG_LVL_ERROR, ("80211> p80211CB == NULL!\n"));
			return 0;
		}
	pCfg80211_CB->MonFilterFlag = VifInfo.MonFilterFlag;
	return 0;
}

#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
#if defined(SIOCGIWSCAN) || defined(RT_CFG80211_SUPPORT)
extern int rt_ioctl_siwscan(struct net_device *dev,
							struct iw_request_info *info,
							union iwreq_data *wreq, char *extra);
#endif /* LINUX_VERSION_CODE: 2.6.30 */
/*
 * ========================================================================
 * Routine Description:
 *	Request to do a scan. If returning zero, the scan request is given
 *	the driver, and will be valid until passed to cfg80211_scan_done().
 *	For scan results, call cfg80211_inform_bss(); you can call this outside
 *	the scan/scan_done bracket too.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pRequest		- Scan request
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: scan
 *
 *	struct cfg80211_scan_request {
 *		struct cfg80211_ssid *ssids;
 *		int n_ssids;
 *		struct ieee80211_channel **channels;
 *		UINT32 n_channels;
 *		const u8 *ie;
 *		size_t ie_len;
 *
 *	 * @ssids: SSIDs to scan for (active scan only)
 *	 * @n_ssids: number of SSIDs
 *	 * @channels: channels to scan on.
 *	 * @n_channels: number of channels for each band
 *	 * @ie: optional information element(s) to add into Probe Request or %NULL
 *	 * @ie_len: length of ie in octets
 * ========================================================================
 */
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
static int CFG80211_OpsScan(
	IN struct wiphy					*pWiphy,
	IN struct cfg80211_scan_request *pRequest)
#else
static int CFG80211_OpsScan(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNdev,
	IN struct cfg80211_scan_request *pRequest)
#endif /* LINUX_VERSION_CODE: 3.6.0 */
{
	VOID *pAd;
	CFG80211_CB *pCfg80211_CB = NULL;
#ifdef APCLI_CFG80211_SUPPORT
	RT_CMD_STA_IOCTL_SCAN scan_cmd;
	CHAR staIndex;
	UCHAR ssid[32];
#else
#ifdef CONFIG_STA_SUPPORT
	struct iw_scan_req IwReq;
	union iwreq_data Wreq;
#endif /* CONFIG_STA_SUPPORT */
#endif /* APCLI_CFG80211_SUPPORT */

#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct net_device *pNdev = NULL;
#ifdef APCLI_CFG80211_SUPPORT
	struct wireless_dev *pWdev = NULL;
#endif /* APCLI_CFG80211_SUPPORT */
#ifdef WIFI_IAP_IW_SCAN_FEATURE
	/*request dev infos*/
	struct wifi_dev *preq_wdev = NULL;
	struct net_device *preq_ndev = NULL;
	NDIS_802_11_SSID scan_ssid;
#endif/*WIFI_IAP_IW_SCAN_FEATURE*/

#endif /* LINUX_VERSION_CODE: 3.6.0 */
	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);
	if (pCfg80211_CB == NULL) {
			CFG80211DBG(DBG_LVL_ERROR, ("80211> p80211CB == NULL!\n"));
			return 0;
	}
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
#ifdef APCLI_CFG80211_SUPPORT
	pWdev = pRequest->wdev;
	pNdev = pWdev->netdev;
	staIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNdev);
	if (staIndex != WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Scan Request for Apcli i/f proceed for scanning\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Scan Request for non APCLI i/f end scan\n");
		CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
		return 0;
	}
#else
	RTMP_DRIVER_NET_DEV_GET(pAd, &pNdev);
#endif
#ifdef WIFI_IAP_IW_SCAN_FEATURE
	if (pRequest && pRequest->wdev && pRequest->wdev->netdev
		&& RTMP_OS_NETDEV_GET_WDEV(pRequest->wdev->netdev)) {
		preq_ndev = pRequest->wdev->netdev;
		preq_wdev = RTMP_OS_NETDEV_GET_WDEV(preq_ndev);

		CFG80211DBG(DBG_LVL_INFO, ("[%s](%d):request infname: %s, mac:"MACSTR";\n",
		__func__, __LINE__, RTMP_OS_NETDEV_GET_DEVNAME(preq_ndev),
		MAC2STR(preq_wdev->if_addr)));
	} else {
		CFG80211DBG(DBG_LVL_ERROR, ("[%s](%d):ERROR! pRequest=%p; pRequest wireless wdev=%p; wifi_dev=%p\n",
			__func__, __LINE__, pRequest, pRequest->wdev, RTMP_OS_NETDEV_GET_WDEV(preq_ndev)));
		return -EOPNOTSUPP;
	}

#endif/*WIFI_IAP_IW_SCAN_FEATURE*/
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	CFG80211DBG(DBG_LVL_INFO, ("========================================================================\n"));
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==> %s(%d)\n", __func__, pNdev->name, pNdev->ieee80211_ptr->iftype));
	/* YF_TODO: record the scan_req per netdevice */
	pCfg80211_CB->pCfg80211_ScanReq = pRequest; /* used in scan end */
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
		/* sanity check */
#ifdef APCLI_CFG80211_SUPPORT
	if (pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_STATION)
#else
	/* sanity check */
	if ((pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_STATION) &&
		(pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_AP) &&
		(pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_ADHOC)
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
		&& (pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_P2P_CLIENT)
#endif /* LINUX_VERSION_CODE: 2.6.37 */
	   )
#endif
	   {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> DeviceType Not Support Scan ==> %d\n", pNdev->ieee80211_ptr->iftype));
		CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
		return -EOPNOTSUPP;
	}

	/* Driver Internal SCAN SM Check */
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "80211> Network is down!\n");
		CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
		return -ENETDOWN;
	}

	if (RTMP_DRIVER_80211_SCAN(pAd, pNdev->ieee80211_ptr->iftype) != NDIS_STATUS_SUCCESS) {
		CFG80211DBG(DBG_LVL_ERROR, ("\n\n\n\n\n80211> BUSY - SCANING\n\n\n\n\n"));
		CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
		return 0;
	}

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	UCHAR Flag = 0;

	RTMP_DRIVER_ADAPTER_MCC_DHCP_PROTECT_STATUS(pAd, &Flag);
	MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "mcc  Flag %d\n", Flag);

	if (Flag && (strcmp(pNdev->name, "p2p0") == 0)) {
		CFG80211DBG(DBG_LVL_ERROR, ("MCC Protect DHCP - Aborting Scan\n"));
		return 0;
	}

	RTMP_DRIVER_80211_SET_NOA(pAd, pNdev->name);
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */

	if (pRequest->ie_len != 0) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "80211> ExtraIEs Not Null in ProbeRequest from upper layer...\n");
		/* YF@20120321: Using Cfg80211_CB carry on pAd struct to overwirte the pWpsProbeReqIe. */
		RTMP_DRIVER_80211_SCAN_EXTRA_IE_SET(pAd);
	} else
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "80211> ExtraIEs Null in ProbeRequest from upper layer...\n");
	MTWF_DBG(NULL, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "80211> Num %d of SSID from upper layer...\n",	pRequest->n_ssids);

	/* Set Channel List for this Scan Action */
	MTWF_DBG(NULL, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "80211> [%d] Channels In ProbeRequest.\n",  pRequest->n_channels);

	if (pRequest->n_channels > 0) {
		UINT32 *pChanList;
		UINT32  idx;

		os_alloc_mem(NULL, (UCHAR **)&pChanList, sizeof(UINT32 *) * pRequest->n_channels);

		if (pChanList == NULL) {
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Alloc memory fail\n");
			return FALSE;
		}

		for (idx = 0; idx < pRequest->n_channels; idx++) {
			pChanList[idx] = ieee80211_frequency_to_channel(pRequest->channels[idx]->center_freq);
			CFG80211DBG(DBG_LVL_DEBUG, ("%d,", pChanList[idx]));
		}

		CFG80211DBG(DBG_LVL_DEBUG, ("\n"));
		RTMP_DRIVER_80211_SCAN_CHANNEL_LIST_SET(pAd, pChanList, pRequest->n_channels);

		if (pChanList)
			os_free_mem(pChanList);
	}

#ifdef APCLI_CFG80211_SUPPORT
	memset(&scan_cmd, 0, sizeof(RT_CMD_STA_IOCTL_SCAN));
	memset(ssid, 0, sizeof(ssid));

	if (pRequest->n_ssids && pRequest->ssids) {
		scan_cmd.SsidLen = pRequest->ssids->ssid_len;
		memcpy(ssid, pRequest->ssids->ssid, scan_cmd.SsidLen);
		scan_cmd.pSsid = ssid;
	}
	scan_cmd.StaIndex = staIndex;
	scan_cmd.ScanType = SCAN_ACTIVE;
	RTMP_DRIVER_80211_APCLI_SCAN(pAd, &scan_cmd);
	return 0;
#else

#ifdef CONFIG_STA_SUPPORT
	memset(&Wreq, 0, sizeof(Wreq));
	memset(&IwReq, 0, sizeof(IwReq));
	/* n_ssids could be 0 and ssids could be invalid */
	MTWF_DBG(NULL, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "80211> Num %d of SSID from upper layer...\n",
								pRequest->n_ssids);

	/* %NULL or zero-length SSID is used to indicate wildcard */
	if ((pRequest->n_ssids == 0) || !pRequest->ssids) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"80211> n_ssids == 0 or null ssids Wildcard SSID.\n");
		Wreq.data.flags |= IW_SCAN_ALL_ESSID;
	} else if ((pRequest->n_ssids == 1) && (pRequest->ssids->ssid_len == 0)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "80211> Wildcard SSID In ProbeRequest.\n");
		Wreq.data.flags |= IW_SCAN_ALL_ESSID;
	} else {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "80211> Named SSID [%s] In ProbeRequest.\n",
				pRequest->ssids->ssid);
		Wreq.data.flags |= IW_SCAN_THIS_ESSID;
		/* WCNCR10228: remove useless check on pRequest->ssids->ssid */
	}
	if (pRequest->n_ssids && pRequest->ssids) {
		IwReq.essid_len = pRequest->ssids->ssid_len;
		memcpy(IwReq.essid, pRequest->ssids->ssid, sizeof(IwReq.essid));
	}

	Wreq.data.length = sizeof(struct iw_scan_req);
	IwReq.scan_type = SCAN_ACTIVE;
#ifdef RT_CFG80211_P2P_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_CLIENT)
		|| (pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_GO)
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
		|| (pNdev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_DEVICE)
#endif /* LINUX_VERSION_CODE: 3.7.0 */
	   )
		IwReq.scan_type = SCAN_P2P;
#endif /* RT_CFG80211_P2P_SUPPORT */
#ifdef WIFI_IAP_IW_SCAN_FEATURE
	/*check wdev status*/
	NdisZeroMemory(&scan_ssid, sizeof(scan_ssid));
	scan_ssid.SsidLength = IwReq.essid_len;
	NdisMoveMemory(scan_ssid.Ssid, IwReq.essid, IwReq.essid_len);
	if (preq_wdev->if_up_down_state == TRUE) {
		if (scan_ssid.SsidLength > 0 && scan_ssid.SsidLength <= MAX_LEN_OF_SSID) {
			CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): SCAN_ACTIVE\n", __func__, __LINE__));

			ApSiteSurvey_by_wdev(pAd, &scan_ssid, SCAN_ACTIVE, FALSE, preq_wdev);
		} else {
			scan_ssid.SsidLength = 0;
			CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): SCAN_PASSIVE\n", __func__, __LINE__));

			ApSiteSurvey_by_wdev(pAd, &scan_ssid, SCAN_PASSIVE, FALSE, preq_wdev);
		}
	} else {
		CFG80211DBG(DBG_LVL_ERROR, ("[%s](%d):infname: %s, Mac:"MACSTR"; interface is DOWN\n",
			__func__, __LINE__, RTMP_OS_NETDEV_GET_DEVNAME(preq_ndev), MAC2STR(preq_wdev->if_addr)));
		CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
		return -ENETDOWN;
	}
#else
	rt_ioctl_siwscan(pNdev, NULL, &Wreq, (char *)&IwReq);
#endif
	return 0;

#endif   /* CONFIG_STA_SUPPORT */
#endif   /*APCLI_CFG80211_SUPPORT */

#else
	CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
	return 0;
	/* return -EOPNOTSUPP; */
#endif/* CONFIG_STA_SUPPORT || APCLI_CFG80211_SUPPORT */
}

#endif/* LINUX_VERSION_CODE */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE

INT CFG80211_OpsChanWithSet(
	struct wiphy *wiphy,
	struct net_device *dev,
	struct cfg80211_chan_def *chandef) {
	VOID *pAd = NULL;
	CMD_RTPRIV_IOCTL_80211_CHAN chaninfo;
	struct wifi_dev *pwifi_dev = NULL;


	if (NULL == wiphy || NULL == dev || NULL == chandef) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, Invalid parameter: pWiphy = %p;net_dev = %p; chandef = %p\n",
		wiphy, dev, chandef);
		return -EINVAL;
	}

	/*interface infos*/
	pwifi_dev = RTMP_OS_NETDEV_GET_WDEV(dev);
	CFG80211DBG(DBG_LVL_INFO, ("[%s](%d):request infname: %s, mac:"MACSTR";\n",
		__func__, __LINE__, RTMP_OS_NETDEV_GET_DEVNAME(dev),
		MAC2STR(pwifi_dev->if_addr)));

	/*pad infos*/
	MAC80211_PAD_GET(pAd, wiphy);

	/*init data*/
	NdisZeroMemory(&chaninfo, sizeof(CMD_RTPRIV_IOCTL_80211_CHAN));

	/*get wireless device*/
	chaninfo.pWdev = dev->ieee80211_ptr;/*wireless device kernel struct*/

	/*channel*/
	chaninfo.ChanId = ieee80211_frequency_to_channel(chandef->chan->center_freq);
	chaninfo.CenterChanId = ieee80211_frequency_to_channel(chandef->center_freq1);
	CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): Channel = %d, CenterChanId = %d\n",
		__func__, __LINE__, chaninfo.ChanId, chaninfo.CenterChanId));

	/*bw*/
	/*bw infos*/
	if (chandef->width == NL80211_CHAN_WIDTH_20_NOHT)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_20_NOHT;
	else if (chandef->width == NL80211_CHAN_WIDTH_20)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_20;
	else if (chandef->width == NL80211_CHAN_WIDTH_40)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_40;
#ifdef DOT11_VHT_AC
	else if (chandef->width == NL80211_CHAN_WIDTH_80)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_80;
	else if (chandef->width == NL80211_CHAN_WIDTH_80P80)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_80P80;
	else if (chandef->width == NL80211_CHAN_WIDTH_160)
		chaninfo.ChanType = MTK_NL80211_CHAN_WIDTH_160;
#endif /*DOT11_VHT_AC*/
	else {
		CFG80211DBG(DBG_LVL_ERROR, ("[%s](%d): ERROR! channel bandwith: %d not support.\n",
		__func__, __LINE__, chandef->width));

		return -EOPNOTSUPP;
	};

	CFG80211DBG(DBG_LVL_DEBUG, ("[%s](%d): ChanType = %d\n", __func__, __LINE__, chaninfo.ChanType));

	if (NDIS_STATUS_SUCCESS != RTMP_DRIVER_AP_80211_CHAN_SET(pAd, &chaninfo)) {
		CFG80211DBG(DBG_LVL_ERROR, ("[%s](%d):ERROR! RTMP_DRIVER_AP_80211_CHAN_SET: FAIL\n",
			__func__, __LINE__));
		return -EOPNOTSUPP;
	}

	return 0;
}

#endif/*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/
#endif/*KERNEL_VERSION(4, 0, 0)*/


#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(3, 8, 0) > LINUX_VERSION_CODE)
/*
 * ========================================================================
 * Routine Description:
 *	Set the transmit power according to the parameters.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	Type			-
 *	dBm				- dBm
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	Type -
 *	enum nl80211_tx_power_setting - TX power adjustment
 *	 @NL80211_TX_POWER_AUTOMATIC: automatically determine transmit power
 *	 @NL80211_TX_POWER_LIMITED: limit TX power by the mBm parameter
 *	 @NL80211_TX_POWER_FIXED: fix TX power to the mBm parameter
 * ========================================================================
 */
static int CFG80211_OpsTxPwrSet(
	IN struct wiphy						*pWiphy,
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *wdev,
#endif
#if (KERNEL_VERSION(2, 6, 36) <= LINUX_VERSION_CODE)
	IN enum nl80211_tx_power_setting	Type,
#else
	IN enum tx_power_setting			Type,
#endif /* LINUX_VERSION_CODE */
	IN int								dBm)
{
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	return -EOPNOTSUPP;
}


/*
 * ========================================================================
 * Routine Description:
 *	Store the current TX power into the dbm variable.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pdBm			- dBm
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 *Note:
 * ========================================================================
 */
static int CFG80211_OpsTxPwrGet(
	IN struct wiphy						*pWiphy,
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *wdev,
#endif
	IN int								*pdBm)
{
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	return -EOPNOTSUPP;
} /* End of CFG80211_OpsTxPwrGet */
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0) */

/*
 * ========================================================================
 * Routine Description:
 *	Power management.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	FlgIsEnabled	-
 *	Timeout			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsPwrMgmt(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN bool							 enabled,
	IN INT32							 timeout)
{
	VOID *pAd;
#ifdef WIFI_IAP_POWER_SAVE_FEATURE
	struct wifi_dev *inf_wdev = NULL;
#endif/*WIFI_IAP_POWER_SAVE_FEATURE*/

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==> power save %s\n", __func__, (enabled ? "enable" : "disable")));
	MAC80211_PAD_GET(pAd, pWiphy);
#ifdef WIFI_IAP_POWER_SAVE_FEATURE
	inf_wdev = RTMP_OS_NETDEV_GET_WDEV(pNdev);
	if (!inf_wdev) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, Invalid data: inf_wdev = %p\n",
		inf_wdev);
		return -ENOENT;
	}
	CFG80211DBG(DBG_LVL_INFO, ("[%s](%d):request infname: %s, mac:"MACSTR";\n",
		__func__, __LINE__, RTMP_OS_NETDEV_GET_DEVNAME(pNdev),
		MAC2STR(inf_wdev->if_addr)));

	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(%d): enabled = %d;\n",
		__LINE__, enabled);
	RTMP_DRIVER_80211_AP_POWER_MGMT_SET(pAd, inf_wdev, enabled);
#else/*WIFI_IAP_POWER_SAVE_FEATURE*/
	RTMP_DRIVER_80211_POWER_MGMT_SET(pAd, enabled);
#endif/*NO-WIFI_IAP_POWER_SAVE_FEATURE*/

	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Get information for a specific station.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	pMac			- STA MAC
 *	pSinfo			- STA INFO
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsStaGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	IN const UINT8							*pMac,
#else
	IN UINT8							*pMac,
#endif
	IN struct station_info				*pSinfo)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_STA StaInfo;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __FUNCTION__));
	MAC80211_PAD_GET(pAd, pWiphy);

	/* init */
	memset(pSinfo, 0, sizeof(*pSinfo));
	memset(&StaInfo, 0, sizeof(StaInfo));

	memcpy(StaInfo.MAC, pMac, 6);

	/* get sta information */
	if (RTMP_DRIVER_80211_STA_GET(pAd, &StaInfo) != NDIS_STATUS_SUCCESS)
		return -ENOENT;

	if (StaInfo.TxRateFlags != RT_CMD_80211_TXRATE_LEGACY) {
		pSinfo->txrate.flags = RATE_INFO_FLAGS_MCS;
		if (StaInfo.TxRateFlags & RT_CMD_80211_TXRATE_BW_40)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
			pSinfo->txrate.bw = RATE_INFO_BW_40;
#else
			pSinfo->txrate.flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
#endif

		if (StaInfo.TxRateFlags & RT_CMD_80211_TXRATE_SHORT_GI)
			pSinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;


		pSinfo->txrate.mcs = StaInfo.TxRateMCS;
	} else {
		pSinfo->txrate.legacy = StaInfo.TxRateMCS;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	pSinfo->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);
#else
	pSinfo->filled |= STATION_INFO_TX_BITRATE;
#endif

	/* fill signal */
	pSinfo->signal = StaInfo.Signal;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
		pSinfo->filled |= BIT(NL80211_STA_INFO_SIGNAL);
#else
		pSinfo->filled |= STATION_INFO_SIGNAL;
#endif

#ifdef CONFIG_AP_SUPPORT
	/* fill tx count */
	/*pSinfo->tx_packets = StaInfo.TxPacketCnt;*/
	pSinfo->rx_bytes = StaInfo.rx_bytes;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	pSinfo->filled |= BIT(NL80211_STA_INFO_RX_BYTES);
#else
	pSinfo->filled |= STATION_INFO_RX_BYTES;
#endif

	pSinfo->tx_bytes = StaInfo.tx_bytes;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	pSinfo->filled |= BIT(NL80211_STA_INFO_TX_BYTES);
#else
	pSinfo->filled |= STATION_INFO_TX_BYTES;
#endif

	pSinfo->rx_packets = StaInfo.rx_packets;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	pSinfo->filled |= BIT(NL80211_STA_INFO_RX_PACKETS);
#else
	pSinfo->filled |= STATION_INFO_RX_PACKETS;
#endif


	pSinfo->tx_packets = StaInfo.tx_packets;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
		pSinfo->filled |= BIT(NL80211_STA_INFO_TX_PACKETS);
#else
	pSinfo->filled |= STATION_INFO_TX_PACKETS;
#endif

	/* fill inactive time */
	pSinfo->inactive_time = StaInfo.InactiveTime;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
		pSinfo->filled |= BIT(NL80211_STA_INFO_INACTIVE_TIME);
#else
	pSinfo->filled |= STATION_INFO_INACTIVE_TIME;
#endif

#else /*CONFIG_STA_SUPPORT */
	/* fill tx/rx count */
	pSinfo->tx_packets = StaInfo.tx_packets;
	pSinfo->filled |= STATION_INFO_TX_PACKETS;
	pSinfo->tx_retries = StaInfo.tx_retries;
	pSinfo->filled |= STATION_INFO_TX_RETRIES;
	pSinfo->tx_failed = StaInfo.tx_failed;
	pSinfo->filled |= STATION_INFO_TX_FAILED;
	pSinfo->rx_packets = StaInfo.rx_packets;
	pSinfo->filled |= STATION_INFO_RX_PACKETS;
	/* fill inactive time */
	pSinfo->inactive_time = StaInfo.InactiveTime;
	pSinfo->filled |= STATION_INFO_INACTIVE_TIME;
#endif /* CONFIG_AP_SUPPORT */

	return 0;
}

#ifdef WIFI_IAP_STA_DUMP_FEATURE
#define CFG_IS_VALID_MAC(addr) \
	((addr[0])|(addr[1])|(addr[2])|(addr[3])|(addr[4])|(addr[5]))

static BOOLEAN CFG80211_FILL_STA_FLAGS(
	struct mtk_nl80211_sta_flag drv_sta_flags,
	struct station_info *pstainfo)
{
	struct nl80211_sta_flag_update *sta_flags;

	if (NULL == pstainfo) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, Invalid data bw = NULL;\n");
		return FALSE;
	}

	pstainfo->filled |= BIT(NL80211_STA_INFO_STA_FLAGS);
	sta_flags = &pstainfo->sta_flags;

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_AUTHORIZED);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_AUTHORIZED);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_AUTHORIZED));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_SHORT_PREAMBLE));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_WME)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_WME);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_WME)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_WME);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_WME));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_MFP)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_MFP);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_MFP)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_MFP);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_MFP));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_AUTHENTICATED)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_AUTHENTICATED);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_AUTHENTICATED)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_AUTHENTICATED);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_AUTHENTICATED));
		}
	}

	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_TDLS_PEER);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_TDLS_PEER);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_TDLS_PEER));
		}
	}
	if (drv_sta_flags.mask & BIT(NL80211_STA_FLAG_ASSOCIATED)) {
		sta_flags->mask |= BIT(NL80211_STA_FLAG_ASSOCIATED);
		if (drv_sta_flags.set & BIT(NL80211_STA_FLAG_ASSOCIATED)) {
			sta_flags->set |= BIT(NL80211_STA_FLAG_ASSOCIATED);
		} else {
			sta_flags->set &= ~(BIT(NL80211_STA_FLAG_ASSOCIATED));
		}
	}

	return TRUE;
}

static BOOLEAN CFG80211_FILL_BSS_PARAM(
	pmtk_cfg_sta_bss_para pbss_info,
	struct station_info *pstainfo)
{


	if (NULL == pbss_info || NULL == pstainfo) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, pbss_info = %p, pstainfo = %p;\n",
		pbss_info, pstainfo);
		return FALSE;
	}


	pstainfo->filled |= BIT(NL80211_STA_INFO_BSS_PARAM);
	/*dtim and beacon interval*/
	pstainfo->bss_param.dtim_period = pbss_info->dtim_period;
	pstainfo->bss_param.beacon_interval = pbss_info->beacon_interval;

	if (pbss_info->flags & BSS_PARAM_FLAGS_CTS_PROT) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					("\n[%s](%d):BSS_PARAM_FLAGS_CTS_PROT: TRUE\n",
					__func__, __LINE__));
		pstainfo->bss_param.flags |= BSS_PARAM_FLAGS_CTS_PROT;
	} else {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					("\n[%s](%d):BSS_PARAM_FLAGS_CTS_PROT: FALSE\n",
					__func__, __LINE__));
		pstainfo->bss_param.flags &= ~BSS_PARAM_FLAGS_CTS_PROT;
	}

	if (pbss_info->flags & BSS_PARAM_FLAGS_SHORT_PREAMBLE) {
		pstainfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_PREAMBLE;
	} else {
		pstainfo->bss_param.flags &= ~BSS_PARAM_FLAGS_SHORT_PREAMBLE;
	}

	if (pbss_info->flags & BSS_PARAM_FLAGS_SHORT_SLOT_TIME) {
		pstainfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
	} else {
		pstainfo->bss_param.flags &= ~BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
	}

	return TRUE;
}
#ifdef WIFI_IAP_BCN_STAT_FEATURE
static BOOLEAN CFG80211_FILL_BCN_PARAM(
	CMD_RTPRIV_IOCTL_80211_STA * pdrv_stainfo,
	struct station_info *pstainfo)
{

	if (NULL == pdrv_stainfo || NULL == pstainfo) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, pdrv_stainfo = %p, pstainfo = %p;\n",
		pdrv_stainfo, pstainfo);
		return FALSE;
	}

	/*beacon loss*/
	if (pdrv_stainfo->beacon_mask & BIT(NL80211_STA_INFO_BEACON_LOSS)) {
		pstainfo->filled |= BIT(NL80211_STA_INFO_BEACON_LOSS);
		pstainfo->beacon_loss_count = pdrv_stainfo->beacon_loss_count;
	} else {
		pstainfo->filled &= ~(BIT(NL80211_STA_INFO_BEACON_LOSS));
	}
	/*beacon rx*/
	if (pdrv_stainfo->beacon_mask & BIT(NL80211_STA_INFO_BEACON_RX)) {
		pstainfo->filled |= BIT(NL80211_STA_INFO_BEACON_RX);
		pstainfo->rx_beacon = (UINT64)pdrv_stainfo->rx_beacon;
	} else {
		pstainfo->filled &= ~(BIT(NL80211_STA_INFO_BEACON_RX));
	}
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(%d): rx_beacon = %llu, rx_loss = %u\n",
		__LINE__, pstainfo->rx_beacon, pstainfo->beacon_loss_count);

	return TRUE;
}
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/

static INT CFG80211_FILL_BW(CHAR *bw)
{

	if (NULL == bw) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, Invalid data bw = NULL;\n");
		return EINVAL;
	}

	switch (*bw) {
	case BW_5:
		*bw = RATE_INFO_BW_5;
		break;
	case BW_10:
		*bw = RATE_INFO_BW_10;
		break;
	case BW_20:
		*bw = RATE_INFO_BW_20;
		break;
	case BW_40:
		*bw = RATE_INFO_BW_40;
		break;
	case BW_80:
		*bw = RATE_INFO_BW_80;
		break;
	case BW_160:
	case BW_8080:
		*bw = RATE_INFO_BW_160;
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"default ERROR, Invalid data bw = %d;\n", *bw);
		break;
	}

	return 0;
}
/*
 * ========================================================================
 * Routine Description:
 *	Get information for ap station.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	pMac			- STA MAC
 *	pSinfo			- STA INFO
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */

INT CFG80211_OpsAp_StaGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN INT 								idx,
	IN UINT8							*pMac,
	IN struct station_info				*pSinfo)
{
	RTMP_ADAPTER  *pAd = NULL;
	CMD_RTPRIV_IOCTL_80211_STA StaInfo;
	INT i = 0;
	UINT assoc_idx = 0;
	UCHAR is_find = FALSE;
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct wifi_dev *inf_wdev = NULL;

	if (NULL == pWiphy || NULL == pNdev || NULL == pSinfo) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, Invalid data: pWiphy = %p; pNdev = %p; pSinfo = %p\n",
		pWiphy, pNdev, pSinfo);
		return -ENOENT;
	}

	MAC80211_PAD_GET(pAd, pWiphy);

	/*wifi dev interface*/
	inf_wdev = RTMP_OS_NETDEV_GET_WDEV(pNdev);
	if (!inf_wdev) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, Invalid data: inf_wdev = %p\n",
		inf_wdev);
		return -ENOENT;
	}
	CFG80211DBG(DBG_LVL_INFO, ("[%s](%d):request infname: %s, mac:"MACSTR";\n",
		__func__, __LINE__, RTMP_OS_NETDEV_GET_DEVNAME(pNdev),
		MAC2STR(inf_wdev->if_addr)));

	/*init data*/
	NdisZeroMemory(pMac, MAC_ADDR_LEN);
	NdisZeroMemory(pSinfo, sizeof(*pSinfo));
	NdisZeroMemory(&StaInfo, sizeof(StaInfo));

	/*from idx find the assoc_idx sta_mac*/
	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];
		if (pEntry && inf_wdev == pEntry->wdev && (pEntry->Sst == SST_ASSOC) &&
			CFG_IS_VALID_MAC(pEntry->Addr) && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry))) {
			if (assoc_idx == idx) {
				NdisMoveMemory(pMac, pEntry->Addr, MAC_ADDR_LEN);
				NdisMoveMemory(StaInfo.MAC, pMac, MAC_ADDR_LEN);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(%d):idx: %d, mac: "MACSTR";(TYPE:%x)\n",
				__LINE__, idx, MAC2STR(pMac), pEntry->EntryType);
				is_find = TRUE;
				break;
			}

			assoc_idx++;
		}
	}

	/*find check*/
	if (FALSE == is_find) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("[%s](%d):cann't find sta idx(%d) in sta table!\n",
			__func__, __LINE__, idx));
		return -ENOENT;
	}

	/*get ap stainfo*/
	if (RTMP_DRIVER_80211AP_STA_GET(pAd, &StaInfo) != NDIS_STATUS_SUCCESS)
		return -ENOENT;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	/*inactive time*/
	pSinfo->filled |= BIT(NL80211_STA_INFO_INACTIVE_TIME);
	pSinfo->inactive_time =  (UINT32)StaInfo.InactiveTime;

	/*rx bytes*/
	pSinfo->filled |= BIT(NL80211_STA_INFO_RX_BYTES);
	pSinfo->rx_bytes  = (UINT64)StaInfo.rx_bytes;

	/*rx_packets*/
	pSinfo->filled |= BIT(NL80211_STA_INFO_RX_PACKETS);
	pSinfo->rx_packets = (UINT32) StaInfo.rx_packets;

	/*tx_bytes*/
	pSinfo->filled |= BIT(NL80211_STA_INFO_TX_BYTES);
	pSinfo->tx_bytes =  (UINT64)StaInfo.tx_bytes;

	/*tx_packets*/
	pSinfo->filled |= BIT(NL80211_STA_INFO_TX_PACKETS);
	pSinfo->tx_packets = (UINT32)StaInfo.tx_packets;

	/*tx retries*/
	pSinfo->filled |= BIT(NL80211_STA_INFO_TX_RETRIES);
	pSinfo->tx_retries = (UINT32)StaInfo.tx_retries;

	/*tx_failed*/
	pSinfo->filled |= BIT(NL80211_STA_INFO_TX_FAILED);
	pSinfo->tx_failed = (UINT32)StaInfo.tx_failed;

	/*signal*/
	pSinfo->filled |= BIT(NL80211_STA_INFO_SIGNAL);
	pSinfo->signal = (CHAR)StaInfo.Signal;

	/*signal_avg*/
	pSinfo->filled |= BIT(NL80211_STA_INFO_SIGNAL_AVG);
	pSinfo->signal_avg = (CHAR)StaInfo.signal_avg;

	/*tx rate*/
	if (StaInfo.tx_packets > 0) {
		pSinfo->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);
		/*tx rate infos*/
		pSinfo->txrate.flags = StaInfo.txrate.flags;
		pSinfo->txrate.mcs = StaInfo.txrate.mcs;
		pSinfo->txrate.legacy = StaInfo.txrate.legacy * CFG_LEGACY_RATE;
		pSinfo->txrate.nss = StaInfo.txrate.nss;
		if (EINVAL != CFG80211_FILL_BW(&StaInfo.txrate.bw)) {
			pSinfo->txrate.bw = StaInfo.txrate.bw;
		}

		/*tx rate infos*/
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\n(%d):tx_rate: rate=%u, mcs=%u, bw=%u, nss=%u\n",
			__LINE__, pSinfo->txrate.legacy/CFG_LEGACY_RATE,
			pSinfo->txrate.mcs, pSinfo->txrate.bw, pSinfo->txrate.nss);
	}
	/*rx rate*/
	if (StaInfo.rx_packets > 0) {
		pSinfo->filled |= BIT(NL80211_STA_INFO_RX_BITRATE);
		/*rx rate infos*/
		pSinfo->rxrate.flags = StaInfo.rxrate.flags;
		pSinfo->rxrate.mcs = StaInfo.rxrate.mcs;
		pSinfo->rxrate.legacy = StaInfo.rxrate.legacy * CFG_LEGACY_RATE;
		pSinfo->rxrate.nss = StaInfo.rxrate.nss;
		if (EINVAL != CFG80211_FILL_BW(&StaInfo.rxrate.bw)) {
			pSinfo->rxrate.bw = StaInfo.rxrate.bw;
		}
		/*tx rate infos*/
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(%d):rx_rate: rate=%u, mcs=%u, bw=%u, nss=%u\n",
			__LINE__, pSinfo->rxrate.legacy/CFG_LEGACY_RATE,
			pSinfo->rxrate.mcs, pSinfo->rxrate.bw, pSinfo->rxrate.nss);
	}
	/*fill stainfos*/
	if (FALSE == CFG80211_FILL_STA_FLAGS(StaInfo.sta_flags, pSinfo)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, CFG80211_FILL_STA_FLAGS Fail!\n");
	}
	if (StaInfo.sta_flags.mask & BIT(NL80211_STA_FLAG_ASSOCIATED)) {
		pSinfo->filled |= BIT(NL80211_STA_INFO_CONNECTED_TIME);
		pSinfo->connected_time = StaInfo.connected_time;
		/*fill bss parameter*/
		if (FALSE == CFG80211_FILL_BSS_PARAM(&StaInfo.bss_param, pSinfo)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, CFG80211_FILL_BSS_PARAM Fail!\n");
		}
	}

/*beacon statistics*/
#ifdef WIFI_IAP_BCN_STAT_FEATURE
	if (TRUE != CFG80211_FILL_BCN_PARAM(&StaInfo, pSinfo)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR, CFG80211_FILL_BCN_PARAM Fail!\n");
	}
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/
#endif/*KERNEL_VERSION(4, 0, 0)*/

	return 0;

}
#endif/* WIFI_IAP_STA_DUMP_FEATURE */

/*
 * ========================================================================
 * Routine Description:
 *	List all stations known, e.g. the AP on managed interfaces.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	Idx				-
 *	pMac			-
 *	pSinfo			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsStaDump(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN int								Idx,
	IN UINT8 * pMac,
	IN struct station_info				*pSinfo)
{
	VOID *pAd;
#ifdef CONFIG_AP_SUPPORT
#ifdef WIFI_IAP_STA_DUMP_FEATURE
	PRTMP_ADAPTER ap_pAd = NULL;
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/
#endif/*CONFIG_AP_SUPPORT*/

	if (Idx != 0)
		return -ENOENT;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
#ifdef CONFIG_AP_SUPPORT
#ifdef WIFI_IAP_STA_DUMP_FEATURE
	if (NULL == pAd) {
		return -ENOENT;
	}
	ap_pAd = (PRTMP_ADAPTER) pAd;

	IF_DEV_CONFIG_OPMODE_ON_AP(ap_pAd) {
		return CFG80211_OpsAp_StaGet(pWiphy, pNdev, Idx, pMac, pSinfo);
	}
#endif/* WIFI_IAP_STA_DUMP_FEATURE */
#endif/* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT

	if (RTMP_DRIVER_AP_SSID_GET(pAd, pMac) != NDIS_STATUS_SUCCESS)
		return -EBUSY;
	else
		return CFG80211_OpsStaGet(pWiphy, pNdev, pMac, pSinfo);

#endif /* CONFIG_STA_SUPPORT */
	return -EOPNOTSUPP;
} /* End of CFG80211_OpsStaDump */


/*
 * ========================================================================
 * Routine Description:
 *	Notify that wiphy parameters have changed.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	Changed			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsWiphyParamsSet(
	IN struct wiphy						*pWiphy,
	IN UINT32							Changed)
{
	VOID *pAd;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);

	if (Changed & WIPHY_PARAM_RTS_THRESHOLD) {
		RTMP_DRIVER_80211_RTS_THRESHOLD_ADD(pAd, pWiphy->rts_threshold);
		CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==> rts_threshold(%d)\n", __func__, pWiphy->rts_threshold));
		return 0;
	} else if (Changed & WIPHY_PARAM_FRAG_THRESHOLD) {
		RTMP_DRIVER_80211_FRAG_THRESHOLD_ADD(pAd, pWiphy->frag_threshold);
		CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==> frag_threshold(%d)\n", __func__, pWiphy->frag_threshold));
		return 0;
	}
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	else if (Changed & WIPHY_PARAM_COVERAGE_CLASS) {
	UINT ack_time = 0;

	if (pWiphy->coverage_class > 255) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> %s ==>COVERAGE_threshold(%d) is invalid!\n",
		__func__, pWiphy->coverage_class));
		return -EOPNOTSUPP;
	}

	ack_time = (UINT) (pWiphy->coverage_class * 3);

	/* IEEE 802.11-2007 table 7-27*/
	if (0 == ack_time)
		ack_time = 1;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>distance or COVERAGE_threshold(%d), ack_time=%d us\n",
		__func__, pWiphy->coverage_class, ack_time));
	if (NDIS_STATUS_SUCCESS != RTMP_DRIVER_80211_ACK_THRESHOLD_ADD(pAd, ack_time)) {
		CFG80211DBG(DBG_LVL_ERROR, ("[%s](%d):ERROR, SET ACK TIMEOUT FAIL!\n",
		__func__, __LINE__));
		return -EOPNOTSUPP;
	}
	CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): SET ACK TIMEOUT SUCCESS!\n",
		__func__, __LINE__));
	return 0;
}
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/
	return -EOPNOTSUPP;
} /* End of CFG80211_OpsWiphyParamsSet */


/*
 * ========================================================================
 * Routine Description:
 *	Add a key with the given parameters.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *	Pairwise		-
 *	pMacAddr		-
 *	pParams			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	pMacAddr will be NULL when adding a group key.
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
static int CFG80211_OpsKeyAdd(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8 * pMacAddr,
	IN struct key_params				*pParams)
#else

static int CFG80211_OpsKeyAdd(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8 * pMacAddr,
	IN struct key_params				*pParams)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_KEY KeyInfo;
	CFG80211_CB *p80211CB;

	p80211CB = NULL;
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
#ifdef RT_CFG80211_DEBUG
	hex_dump("KeyBuf=", (UINT8 *)pParams->key, pParams->key_len);
#endif /* RT_CFG80211_DEBUG */
	CFG80211DBG(DBG_LVL_INFO, ("80211> KeyIdx = %d\n", KeyIdx));

	if (pParams->key_len >= sizeof(KeyInfo.KeyBuf))
		return -EINVAL;

	/* End of if */
	/* init */
	memset(&KeyInfo, 0, sizeof(KeyInfo));
	memcpy(KeyInfo.KeyBuf, pParams->key, pParams->key_len);
	KeyInfo.KeyBuf[pParams->key_len] = 0x00;
	KeyInfo.KeyId = KeyIdx;
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	KeyInfo.bPairwise = Pairwise;
#endif /* LINUX_VERSION_CODE: 2,6,37 */
	KeyInfo.KeyLen = pParams->key_len;
#ifdef DOT11W_PMF_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
#ifdef CONFIG_STA_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION) &&
		(pParams->cipher == WLAN_CIPHER_SUITE_AES_CMAC)) {
		PRTMP_ADAPTER pad = (PRTMP_ADAPTER)pAd;
		struct wifi_dev *wdev = &pad->StaCfg[0].wdev;
		MAC_TABLE_ENTRY *pEntry = NULL;

		pEntry = GetAssociatedAPByWdev(pad, wdev);
		memcpy(KeyInfo.KeyBuf, pEntry->SecConfig.GTK, LEN_TK);
		memcpy(KeyInfo.KeyBuf + LEN_TK, pParams->key, pParams->key_len);
		KeyInfo.KeyLen = LEN_TK + pParams->key_len;
		KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;
		KeyInfo.cipher = Ndis802_11AESEnable;
		KeyInfo.KeyId = pEntry->SecConfig.GroupKeyId;
	} else
#endif /* CONFIG_STA_SUPPORT */
#endif /*APCLI_CFG80211_SUPPORT */
#endif	/* DOT11W_PMF_SUPPORT */
		if (pParams->cipher == WLAN_CIPHER_SUITE_WEP40)
			KeyInfo.KeyType = RT_CMD_80211_KEY_WEP40;
		else if (pParams->cipher == WLAN_CIPHER_SUITE_WEP104)
			KeyInfo.KeyType = RT_CMD_80211_KEY_WEP104;
		else if ((pParams->cipher == WLAN_CIPHER_SUITE_TKIP) ||
				 (pParams->cipher == WLAN_CIPHER_SUITE_CCMP)) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;

			if (pParams->cipher == WLAN_CIPHER_SUITE_TKIP)
				KeyInfo.cipher = Ndis802_11TKIPEnable;
			else if (pParams->cipher == WLAN_CIPHER_SUITE_CCMP)
				KeyInfo.cipher = Ndis802_11AESEnable;
#if (KERNEL_VERSION(4, 0, 0) <= LINUX_VERSION_CODE)
		} else if (pParams->cipher == WLAN_CIPHER_SUITE_GCMP_256) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;
			KeyInfo.cipher = Ndis802_11GCMP256Enable;
#endif
		} else if (pParams->cipher == WLAN_CIPHER_SUITE_GCMP) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;
			KeyInfo.cipher = Ndis802_11GCMP128Enable;
#if (KERNEL_VERSION(4, 0, 0) <= LINUX_VERSION_CODE)
		} else if (pParams->cipher == WLAN_CIPHER_SUITE_CCMP_256) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;
			KeyInfo.cipher = Ndis802_11CCMP256Enable;
#endif
		}

#ifdef DOT11W_PMF_SUPPORT
		else if (pParams->cipher == WLAN_CIPHER_SUITE_AES_CMAC) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_AES_CMAC;
			KeyInfo.KeyId = KeyIdx;
			KeyInfo.bPairwise = FALSE;
			KeyInfo.KeyLen = pParams->key_len;
		}
#ifdef HOSTAPD_SUITEB_SUPPORT
		else if (pParams->cipher == WLAN_CIPHER_SUITE_BIP_GMAC_256) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_AES_CMAC;
			KeyInfo.KeyId = KeyIdx;
			KeyInfo.bPairwise = FALSE;
			KeyInfo.KeyLen = pParams->key_len;
		}
#endif
#endif /* DOT11W_PMF_SUPPORT */
		else
			return -ENOTSUPP;

	KeyInfo.pNetDev = pNdev;
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP_VLAN)) {
		if (pMacAddr) {
			CFG80211DBG(DBG_LVL_ERROR, ("80211> ifname=%s KeyAdd STA("MACSTR") ==>\n", pNdev->name, MAC2STR(pMacAddr)));
			NdisCopyMemory(KeyInfo.MAC, pMacAddr, MAC_ADDR_LEN);
		} else if (KeyInfo.bPairwise == FALSE) {
			CFG80211DBG(DBG_LVL_ERROR, ("80211> ifname=%s KeyAdd GroupKey\n", pNdev->name));
		}

		CFG80211DBG(DBG_LVL_INFO, ("80211> AP Key Add\n"));
		RTMP_DRIVER_80211_AP_KEY_ADD(pAd, &KeyInfo);
	} else
#endif /* CONFIG_AP_SUPPORT */
	{
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	CFG80211DBG(DBG_LVL_INFO, ("80211> STA Key Add\n"));
	RTMP_DRIVER_80211_STA_KEY_ADD(pAd, &KeyInfo);
#endif	/* CONFIG_STA_SUPPORT */
	}

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT

	if (pMacAddr) {
		CFG80211DBG(DBG_LVL_INFO, ("80211> P2pSendWirelessEvent("MACSTR") ==>\n",
									MAC2STR(pMacAddr)));
		RTMP_DRIVER_80211_SEND_WIRELESS_EVENT(pAd, pMacAddr);
	}

#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Get information about the key with the given parameters.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *	Pairwise		-
 *	pMacAddr		-
 *	pCookie			-
 *	pCallback		-
 *
 * Return Value:
 *	0			- success
 *	-x			- fail
 *
 * Note:
 *	pMacAddr will be NULL when requesting information for a group key.
 *
 *	All pointers given to the pCallback function need not be valid after
 *	it returns.
 *
 *	This function should return an error if it is not possible to
 *	retrieve the key, -ENOENT if it doesn't exist.
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
static int CFG80211_OpsKeyGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8						*pMacAddr,
	IN void								*pCookie,
	IN void								(*pCallback)(void *cookie,
			struct key_params *))
#else

static int CFG80211_OpsKeyGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr,
	IN void								*pCookie,
	IN void								(*pCallback)(void *cookie,
			struct key_params *))
#endif /* LINUX_VERSION_CODE */
{
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	return -ENOTSUPP;
}


/*
 * ========================================================================
 * Routine Description:
 *	Remove a key given the pMacAddr (NULL for a group key) and KeyIdx.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *	pMacAddr		-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	return -ENOENT if the key doesn't exist.
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
static int CFG80211_OpsKeyDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8						*pMacAddr)
#else

static int CFG80211_OpsKeyDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_KEY KeyInfo;
	CFG80211_CB *p80211CB;

	p80211CB = NULL;
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));

	if (pMacAddr) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> KeyDel STA("MACSTR") ==>\n", MAC2STR(pMacAddr)));
		NdisCopyMemory(KeyInfo.MAC, pMacAddr, MAC_ADDR_LEN);
	}

	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);
	memset(&KeyInfo, 0, sizeof(KeyInfo));
	KeyInfo.KeyId = KeyIdx;
	KeyInfo.pNetDev = pNdev;
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	CFG80211DBG(DBG_LVL_INFO, ("80211> KeyDel isPairwise %d\n", Pairwise));
	KeyInfo.bPairwise = Pairwise;
#endif /* LINUX_VERSION_CODE 2.6.37 */
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO)) {
		CFG80211DBG(DBG_LVL_INFO, ("80211> AP Key Del\n"));
		RTMP_DRIVER_80211_AP_KEY_DEL(pAd, &KeyInfo);
	} else
#endif /* CONFIG_AP_SUPPORT */
	{
		CFG80211DBG(DBG_LVL_INFO, ("80211> STA Key Del\n"));
			CFG80211DBG(DBG_LVL_ERROR, ("80211> STA Key Del -- DISCONNECT\n"));
			RTMP_DRIVER_80211_STA_LEAVE(pAd, pNdev);
	}

	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Set the default key on an interface.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 38) <= LINUX_VERSION_CODE)
static int CFG80211_OpsKeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Unicast,
	IN bool								Multicast)
#else

static int CFG80211_OpsKeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_INFO, ("80211> Default KeyIdx = %d\n", KeyIdx));
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO))
		RTMP_DRIVER_80211_AP_KEY_DEFAULT_SET(pAd, pNdev, KeyIdx);
	else
#endif /* CONFIG_AP_SUPPORT */
		RTMP_DRIVER_80211_STA_KEY_DEFAULT_SET(pAd, pNdev, KeyIdx);
	return 0;
} /* End of CFG80211_OpsKeyDefaultSet */

#ifdef DOT11W_PMF_SUPPORT
/*
 *========================================================================
 *Routine Description:
 *	Set the default management key on an interface.

 *Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *
 *Return Value:
 *	0				- success
 *	-x				- fail
 *
 *Note:
========================================================================
*/
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
static int CFG80211_OpsMgmtKeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx)

{
	VOID *pAd;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);

	CFG80211DBG(DBG_LVL_INFO, ("80211> Default Mgmt KeyIdx = %d\n", KeyIdx));

#ifdef CONFIG_AP_SUPPORT
	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO))
		RTMP_DRIVER_80211_AP_KEY_DEFAULT_MGMT_SET(pAd, pNdev, KeyIdx);
#endif

	return 0;
} /* End of CFG80211_OpsMgmtKeyDefaultSet */
#endif /* LINUX_VERSION_CODE */
#endif /*DOT11W_PMF_SUPPORT*/



/*
 * ========================================================================
 * Routine Description:
 *	Connect to the ESS with the specified parameters. When connected,
 *	call cfg80211_connect_result() with status code %WLAN_STATUS_SUCCESS.
 *	If the connection fails for some reason, call cfg80211_connect_result()
 *	with the status from the AP.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pSme			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: connect
 *
 *	You must use "iw ra0 connect xxx", then "iw ra0 disconnect";
 *	You can not use "iw ra0 connect xxx" twice without disconnect;
 *	Or you will suffer "command failed: Operation already in progress (-114)".
 *
 *	You must support add_key and set_default_key function;
 *	Or kernel will crash without any error message in linux 2.6.32.
 *
 *
 * struct cfg80211_connect_params - Connection parameters
 *
 * This structure provides information needed to complete IEEE 802.11
 * authentication and association.
 *
 *  @channel: The channel to use or %NULL if not specified (auto-select based
 *	on scan results)
 *  @bssid: The AP BSSID or %NULL if not specified (auto-select based on scan
 *	results)
 * @ssid: SSID
 *  @ssid_len: Length of ssid in octets
 *  @auth_type: Authentication type (algorithm)
 *
 * @ie: IEs for association request
 * @ie_len: Length of assoc_ie in octets
 *
 * @privacy: indicates whether privacy-enabled APs should be used
 * @crypto: crypto settings
 * @key_len: length of WEP key for shared key authentication
 * @key_idx: index of WEP key for shared key authentication
 * @key: WEP key for shared key authentication
 * ========================================================================
 */
static int CFG80211_OpsConnect(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_connect_params	*pSme)
{
#ifdef CONFIG_STA_SUPPORT
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_CONNECT ConnInfo;
	CMD_RTPRIV_IOCTL_80211_ASSOC_IE AssocIe;
	struct ieee80211_channel *pChannel = pSme->channel;
	INT32 Pairwise = 0;
	INT32 Groupwise = 0;
	INT32 Keymgmt = 0;
	INT32 WpaVersion = 0;
	INT32 Chan = -1, Idx;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);

	if (pChannel != NULL)
		Chan = ieee80211_frequency_to_channel(pChannel->center_freq);

	CFG80211DBG(DBG_LVL_INFO, ("Groupwise: %x\n", pSme->crypto.cipher_group));
	Groupwise = pSme->crypto.cipher_group;
	/* for(Idx=0; Idx<pSme->crypto.n_ciphers_pairwise; Idx++) */
	Pairwise |= pSme->crypto.ciphers_pairwise[0];
	CFG80211DBG(DBG_LVL_INFO, ("Pairwise %x\n", pSme->crypto.ciphers_pairwise[0]));

	for (Idx = 0; Idx < pSme->crypto.n_akm_suites; Idx++)
		Keymgmt |= pSme->crypto.akm_suites[Idx];

	WpaVersion = pSme->crypto.wpa_versions;
	CFG80211DBG(DBG_LVL_INFO, ("Wpa_versions %x\n", WpaVersion));
	memset(&ConnInfo, 0, sizeof(ConnInfo));
	ConnInfo.WpaVer = 0;

	if (WpaVersion & NL80211_WPA_VERSION_1)
		ConnInfo.WpaVer = 1;

	if (WpaVersion & NL80211_WPA_VERSION_2)
		ConnInfo.WpaVer = 2;

	CFG80211DBG(DBG_LVL_INFO, ("Keymgmt %x\n", Keymgmt));

	if (Keymgmt ==  WLAN_AKM_SUITE_8021X)
		ConnInfo.FlgIs8021x = TRUE;
	else
		ConnInfo.FlgIs8021x = FALSE;

	CFG80211DBG(DBG_LVL_INFO, ("Auth_type %x\n", pSme->auth_type));

	if (pSme->auth_type == NL80211_AUTHTYPE_SHARED_KEY)
		ConnInfo.AuthType = Ndis802_11AuthModeShared;
	else if (pSme->auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM)
		ConnInfo.AuthType = Ndis802_11AuthModeOpen;
#ifdef SUPP_SAE_SUPPORT
	else if (pSme->auth_type == NL80211_AUTHTYPE_SAE)
		ConnInfo.AuthType = Ndis802_11AuthModeWPA3PSK;
#endif
	else
		ConnInfo.AuthType = Ndis802_11AuthModeAutoSwitch;

#ifdef SUPP_OWE_SUPPORT
	if (Keymgmt == WLAN_AKM_SUITE_OWE)
		ConnInfo.AuthType = Ndis802_11AuthModeOWE;
#endif

	if (Pairwise == WLAN_CIPHER_SUITE_CCMP) {
		CFG80211DBG(DBG_LVL_INFO, ("WLAN_CIPHER_SUITE_CCMP...\n"));
		ConnInfo.PairwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_CCMP;
	} else if (Pairwise == WLAN_CIPHER_SUITE_TKIP) {
		CFG80211DBG(DBG_LVL_INFO, ("WLAN_CIPHER_SUITE_TKIP...\n"));
		ConnInfo.PairwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_TKIP;
	} else if ((Pairwise == WLAN_CIPHER_SUITE_WEP40) ||
			   (Pairwise & WLAN_CIPHER_SUITE_WEP104)) {
		CFG80211DBG(DBG_LVL_INFO, ("WLAN_CIPHER_SUITE_WEP...\n"));
		ConnInfo.PairwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_WEP;
	} else {
		CFG80211DBG(DBG_LVL_INFO, ("NONE...\n"));
		ConnInfo.PairwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_NONE;
	}

	if (Groupwise == WLAN_CIPHER_SUITE_CCMP)
		ConnInfo.GroupwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_CCMP;
	else if (Groupwise == WLAN_CIPHER_SUITE_TKIP)
		ConnInfo.GroupwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_TKIP;
	else
		ConnInfo.GroupwiseEncrypType |= RT_CMD_80211_CONN_ENCRYPT_NONE;

	CFG80211DBG(DBG_LVL_INFO, ("ConnInfo.KeyLen ===> %d\n", pSme->key_len));
	CFG80211DBG(DBG_LVL_INFO, ("ConnInfo.KeyIdx ===> %d\n", pSme->key_idx));
	ConnInfo.pKey = (UINT8 *)(pSme->key);
	ConnInfo.KeyLen = pSme->key_len;
	ConnInfo.pSsid = (UINT8 *)pSme->ssid;
	ConnInfo.SsidLen = pSme->ssid_len;
	ConnInfo.KeyIdx = pSme->key_idx;
	/* YF@20120328: Reset to default */
	ConnInfo.bWpsConnection = FALSE;
	ConnInfo.pNetDev = pNdev;
#ifdef DOT11W_PMF_SUPPORT
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
	CFG80211DBG(DBG_LVL_INFO, ("80211> PMF Connect %d\n", pSme->mfp));

	if (pSme->mfp)
		ConnInfo.mfp = TRUE;
	else
		ConnInfo.mfp = FALSE;

#endif /* LINUX_VERSION_CODE */
#endif /* DOT11W_PMF_SUPPORT */
	/* hex_dump("AssocInfo:", pSme->ie, pSme->ie_len); */
	/* YF@20120328: Use SIOCSIWGENIE to make out the WPA/WPS IEs in AssocReq. */
	memset(&AssocIe, 0, sizeof(AssocIe));
	AssocIe.pNetDev = pNdev;
	AssocIe.ie = (UINT8 *)pSme->ie;
	AssocIe.ie_len = pSme->ie_len;
	RTMP_DRIVER_80211_STA_ASSSOC_IE_SET(pAd, &AssocIe, pNdev->ieee80211_ptr->iftype);

	if ((pSme->ie_len > 6) /* EID(1) + LEN(1) + OUI(4) */ &&
		(pSme->ie[0] == WLAN_EID_VENDOR_SPECIFIC &&
		 pSme->ie[1] >= 4 &&
		 pSme->ie[2] == 0x00 && pSme->ie[3] == 0x50 && pSme->ie[4] == 0xf2 &&
		 pSme->ie[5] == 0x04))
		ConnInfo.bWpsConnection = TRUE;

	/* %NULL if not specified (auto-select based on scan)*/
	if (pSme->bssid != NULL) {
		CFG80211DBG(DBG_LVL_INFO, ("80211> Connect bssid "MACSTR"\n",
								  MAC2STR(pSme->bssid)));
		ConnInfo.pBssid = (UINT8 *)pSme->bssid;
	}

	RTMP_DRIVER_80211_CONNECT(pAd, &ConnInfo, pNdev->ieee80211_ptr->iftype);
#endif /*CONFIG_STA_SUPPORT*/
	return 0;
} /* End of CFG80211_OpsConnect */


/*
 * ========================================================================
 * Routine Description:
 *	Disconnect from the BSS/ESS.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	ReasonCode		-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: connect
 *========================================================================
 */
static int CFG80211_OpsDisconnect(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT16								ReasonCode)
{
#ifdef CONFIG_STA_SUPPORT
	VOID *pAd;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	CFG80211DBG(DBG_LVL_INFO, ("80211> ReasonCode = %d\n", ReasonCode));
	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_STA_LEAVE(pAd, pNdev);
#endif /*CONFIG_STA_SUPPORT*/
	return 0;
}
#endif /* LINUX_VERSION_CODE */


#ifdef RFKILL_HW_SUPPORT
static int CFG80211_OpsRFKill(
	IN struct wiphy						*pWiphy)
{
	VOID		*pAd;
	BOOLEAN		active;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_RFKILL(pAd, &active);
	wiphy_rfkill_set_hw_state(pWiphy, !active);
	return active;
}


VOID CFG80211_RFKillStatusUpdate(
	IN PVOID							pAd,
	IN BOOLEAN							active)
{
	struct wiphy *pWiphy;
	CFG80211_CB *pCfg80211_CB;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);
	pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	wiphy_rfkill_set_hw_state(pWiphy, !active);
}
#endif /* RFKILL_HW_SUPPORT */

#if (KERNEL_VERSION(2, 6, 33) <= LINUX_VERSION_CODE)

/*
 * ========================================================================
 * Routine Description:
 *	Cache a PMKID for a BSSID.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pPmksa			- PMKID information
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	This is mostly useful for fullmac devices running firmwares capable of
 *	generating the (re) association RSN IE.
 *	It allows for faster roaming between WPA2 BSSIDs.
 * ========================================================================
 */
static int CFG80211_OpsPmksaSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_pmksa			*pPmksa)
{
#ifdef CONFIG_STA_SUPPORT
#ifdef SUPP_OWE_SUPPORT
	VOID *pAd;
	RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;

	CFG80211DBG(DBG_LVL_ERROR, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);

	if ((pPmksa->bssid == NULL) || (pPmksa->pmkid == NULL))
		return -ENOENT;

	pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_ADD;
	pIoctlPmaSa->pBssid = (UCHAR *)pPmksa->bssid;
	pIoctlPmaSa->pPmkid = (UCHAR *)pPmksa->pmkid;
	RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
#endif
#endif /* CONFIG_STA_SUPPORT */
	return 0;
} /* End of CFG80211_OpsPmksaSet */


/*
 * ========================================================================
 * Routine Description:
 *	Delete a cached PMKID.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pPmksa			- PMKID information
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsPmksaDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_pmksa			*pPmksa)
{
	return 0;
} /* End of CFG80211_OpsPmksaDel */


/*
 * ========================================================================
 * Routine Description:
 *	Flush a cached PMKID.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsPmksaFlush(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev)
{
#ifndef APCLI_CFG80211_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	VOID *pAd;
	RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;

	CFG80211DBG(DBG_LVL_ERROR, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_FLUSH;
	RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
#endif /* CONFIG_STA_SUPPORT */
#endif
	return 0;
} /* End of CFG80211_OpsPmksaFlush */
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
static int CFG80211_OpsRemainOnChannel(
	IN struct wiphy *pWiphy,
	IN struct wireless_dev *pWdev,
	IN struct ieee80211_channel *pChan,
	IN unsigned int duration,
	OUT u64 *cookie)
#else /* LINUX_VERSION_CODE >= 3.8.0 */
static int CFG80211_OpsRemainOnChannel(
	IN struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *pWdev,
#else
	IN struct net_device *dev,
#endif
	IN struct ieee80211_channel *pChan,
	IN enum nl80211_channel_type ChannelType,
	IN unsigned int duration,
	OUT u64 *cookie)
#endif /* LINUX_VERSION_CODE < 3.6.0 */
{
	VOID *pAd;
	UINT32 ChanId;
	CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;
	u32 rndCookie;
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	INT ChannelType = RT_CMD_80211_CHANTYPE_HT20;
#endif /* LINUX_VERSION_CODE: 3.8.0 */
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct net_device *dev = NULL;

	dev = pWdev->netdev;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	rndCookie = MtRandom32() | 1;
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	/*CFG_TODO: Shall check channel type*/
	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	CFG80211DBG(DBG_LVL_INFO, ("%s: CH = %d, Type = %d, duration = %d, cookie=%d\n", __func__,
								ChanId, ChannelType, duration, rndCookie));
	/* init */
	*cookie = rndCookie;
	memset(&ChanInfo, 0, sizeof(ChanInfo));
	ChanInfo.ChanId = ChanId;
	ChanInfo.IfType = dev->ieee80211_ptr->iftype;
	ChanInfo.ChanType = ChannelType;
	ChanInfo.chan = pChan;
	ChanInfo.cookie = rndCookie;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	ChanInfo.pWdev = pWdev;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	/* set channel */
	RTMP_DRIVER_80211_REMAIN_ON_CHAN_SET(pAd, &ChanInfo, duration);
	return 0;
}

static void CFG80211_OpsMgmtFrameRegister(
	struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct wireless_dev *wdev,
#else
	struct net_device *dev,
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	UINT16 frame_type, bool reg)
{
	VOID *pAd;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct net_device *dev = NULL;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	MAC80211_PAD_GET_NO_RV(pAd, pWiphy);
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	RTMP_DRIVER_NET_DEV_GET(pAd, &dev);
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s ==>\n", __func__));
	CFG80211DBG(DBG_LVL_DEBUG, ("frame_type = %x, req = %d , (%d)\n", frame_type, reg,  dev->ieee80211_ptr->iftype));

	if (frame_type == IEEE80211_STYPE_PROBE_REQ)
		RTMP_DRIVER_80211_MGMT_FRAME_REG(pAd, dev, reg);
	else if (frame_type == IEEE80211_STYPE_ACTION)
		RTMP_DRIVER_80211_ACTION_FRAME_REG(pAd, dev, reg);
	else
		CFG80211DBG(DBG_LVL_ERROR, ("Unkown frame_type = %x, req = %d\n", frame_type, reg));
}

/* Supplicant_NEW_TDLS */
#ifdef CFG_TDLS_SUPPORT
static int CFG80211_OpsTdlsMgmt
(
	IN struct wiphy *pWiphy,
	IN struct net_device *pDev,
	IN u8 *peer,
	IN u8 action_code,
	IN u8 dialog_token,
	IN u16 status_code,
	IN const u8 *extra_ies,
	IN size_t extra_ies_len
)
{
	int ret = 0;
	VOID *pAd;

	CFG80211DBG(DBG_LVL_ERROR, ("80211> extra_ies_len : %d ==>\n", extra_ies_len));
	MAC80211_PAD_GET(pAd, pWiphy);

	if (action_code == WLAN_TDLS_SETUP_REQUEST || action_code == WLAN_TDLS_SETUP_RESPONSE)
		RTMP_DRIVER_80211_STA_TDLS_SET_KEY_COPY_FLAG(pAd);

	switch (action_code) {
	case WLAN_TDLS_SETUP_REQUEST:
	case WLAN_TDLS_DISCOVERY_REQUEST:
	case WLAN_TDLS_SETUP_CONFIRM:
	case WLAN_TDLS_TEARDOWN:
	case WLAN_TDLS_SETUP_RESPONSE:
	case WLAN_PUB_ACTION_TDLS_DISCOVER_RES:
		cfg_tdls_build_frame(pAd, peer, dialog_token, action_code, status_code, extra_ies, extra_ies_len, FALSE, 0, 0);
		break;

	case TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_REQ:
		cfg_tdls_TunneledProbeRequest(pAd, peer, extra_ies, extra_ies_len);
		break;

	case TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_RSP:
		cfg_tdls_TunneledProbeResponse(pAd, peer, extra_ies, extra_ies_len);
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

static int CFG80211_OpsTdlsOper(
	IN struct wiphy *pWiphy,
	IN struct net_device *pDev,
	IN u8 *peer,
	IN enum nl80211_tdls_operation oper)
{
	VOID *pAd;

	MAC80211_PAD_GET(pAd, pWiphy);

	switch (oper) {
	case NL80211_TDLS_ENABLE_LINK:
		RTMP_DRIVER_80211_STA_TDLS_INSERT_DELETE_PENTRY(pAd, peer, tdls_insert_entry);
		break;

	case NL80211_TDLS_DISABLE_LINK:
		RTMP_DRIVER_80211_STA_TDLS_INSERT_DELETE_PENTRY(pAd, peer, tdls_delete_entry);
		break;

	default:
		CFG80211DBG(DBG_LVL_ERROR, ("%s Unhandled TdlsOper : %d ==>\n", __func__, oper));
	}

	return 0;
}
#endif /*CFG_TDLS_SUPPORT*/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
static int CFG80211_OpsMgmtTx(
    IN struct wiphy *pWiphy,
    IN struct wireless_dev *wdev,
#if  (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	IN struct cfg80211_mgmt_tx_params *params,
#else
    IN struct ieee80211_channel *pChan,
    IN bool Offchan,
    IN unsigned int Wait,
    IN const u8 *pBuf,
    IN size_t Len,
    IN bool no_cck,
    IN bool done_wait_for_ack,
#endif
    IN u64 *pCookie)
#else
static int CFG80211_OpsMgmtTx(
    IN struct wiphy *pWiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	IN struct wireless_dev *wdev,
#else
    IN struct net_device *pDev,
#endif
    IN struct ieee80211_channel *pChan,
    IN bool Offchan,
    IN enum nl80211_channel_type ChannelType,
    IN bool ChannelTypeValid,
    IN unsigned int Wait,
    IN const u8 *pBuf,
    IN size_t Len,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
    IN bool no_cck,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
    IN bool done_wait_for_ack,
#endif
    IN u64 *pCookie)
#endif /* LINUX_VERSION_CODE: 3.6.0 */
{
    VOID *pAd;
    UINT32 ChanId;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	struct net_device *dev = NULL;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
#if  (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	struct ieee80211_channel *pChan = params->chan;
	/*	bool Offchan = params->offchan; */
	/*      unsigned int Wait = params->wait; */
	const u8 *pBuf = params->buf;
	size_t Len = params->len;
	bool no_cck = params->no_cck;
   /* 	bool done_wait_for_ack = params->dont_wait_for_ack; */
#endif

    CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s ==>\n", __FUNCTION__));
    MAC80211_PAD_GET(pAd, pWiphy);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	RTMP_DRIVER_NET_DEV_GET(pAd, &dev);
#endif /* LINUX_VERSION_CODE: 3.6.0 */

    /* get channel number */
#if  (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	if (pChan == NULL)
		ChanId = 0;
	else
#endif
    ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
    CFG80211DBG(DBG_LVL_DEBUG, ("80211> Mgmt Channel = %d\n", ChanId));

	/* Send the Frame with basic rate 6 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
    if (no_cck)
		; /*pAd->isCfgDeviceInP2p = TRUE; */
#else

#endif

    *pCookie = 5678;
#ifndef HOSTAPD_AUTO_CH_SUPPORT
    if (ChanId != 0)
	RTMP_DRIVER_80211_CHANNEL_LOCK(pAd, ChanId);
#endif
	RTMP_DRIVER_80211_MGMT_FRAME_SEND(pAd, (VOID *)pBuf, Len);

	/* Mark it for using Supplicant-Based off-channel wait
		if (Offchan)
			RTMP_DRIVER_80211_CHANNEL_RESTORE(pAd);
	 */

    return 0;
}

static int CFG80211_OpsTxCancelWait(
	IN struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *wdev,
#else
	IN struct net_device *pDev,
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	u64 cookie)
{
	CFG80211DBG(DBG_LVL_ERROR, ("80211> %s ==>\n", __func__));
	return 0;
}

static int CFG80211_OpsCancelRemainOnChannel(
	struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct wireless_dev *wdev,
#else
	struct net_device *dev,
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	u64 cookie)
{
	VOID *pAd;

	CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	/* It cause the Supplicant-based OffChannel Hang */
	RTMP_DRIVER_80211_CANCEL_REMAIN_ON_CHAN_SET(pAd, cookie);
	return 0;
}

#ifdef CONFIG_AP_SUPPORT
#if (KERNEL_VERSION(3, 4, 0) > LINUX_VERSION_CODE)
static int CFG80211_OpsSetBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct beacon_parameters *info)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	UCHAR *beacon_head_buf, *beacon_tail_buf;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	hex_dump("Beacon head", info->head, info->head_len);
	hex_dump("Beacon tail", info->tail, info->tail_len);
	CFG80211DBG(DBG_LVL_INFO, ("80211>dtim_period = %d\n", info->dtim_period));
	CFG80211DBG(DBG_LVL_INFO, ("80211>interval = %d\n", info->interval));
#if (KERNEL_VERSION(3, 2, 0) <= LINUX_VERSION_CODE)
	CFG80211DBG(DBG_LVL_INFO, ("80211>ssid = %s\n", info->ssid));
	CFG80211DBG(DBG_LVL_INFO, ("80211>ssid_len = %d\n", info->ssid_len));
	CFG80211DBG(DBG_LVL_INFO, ("80211>beacon_ies_len = %d\n", info->beacon_ies_len));
	CFG80211DBG(DBG_LVL_INFO, ("80211>proberesp_ies_len = %d\n", info->proberesp_ies_len));
	CFG80211DBG(DBG_LVL_INFO, ("80211>assocresp_ies_len = %d\n", info->assocresp_ies_len));

	if (info->proberesp_ies_len > 0 && info->proberesp_ies)
		RTMP_DRIVER_80211_AP_PROBE_RSP(pAd, (VOID *)info->proberesp_ies, info->proberesp_ies_len);

	if (info->assocresp_ies_len > 0 && info->assocresp_ies)
		RTMP_DRIVER_80211_AP_ASSOC_RSP(pAd, (VOID *)info->assocresp_ies, info->assocresp_ies_len);

#endif
	os_alloc_mem(NULL, &beacon_head_buf, info->head_len);
	NdisCopyMemory(beacon_head_buf, info->head, info->head_len);
	os_alloc_mem(NULL, &beacon_tail_buf, info->tail_len);
	NdisCopyMemory(beacon_tail_buf, info->tail, info->tail_len);
	bcn.beacon_head_len = info->head_len;
	bcn.beacon_tail_len = info->tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;
	bcn.dtim_period = info->dtim_period;
	bcn.interval = info->interval;
	RTMP_DRIVER_80211_BEACON_SET(pAd, &bcn);

	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);

	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);

	return 0;
}

static int CFG80211_OpsAddBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct beacon_parameters *info)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	UCHAR *beacon_head_buf, *beacon_tail_buf;

	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	hex_dump("Beacon head", info->head, info->head_len);
	hex_dump("Beacon tail", info->tail, info->tail_len);
	CFG80211DBG(DBG_LVL_INFO, ("80211>dtim_period = %d\n", info->dtim_period));
	CFG80211DBG(DBG_LVL_INFO, ("80211>interval = %d\n", info->interval));
#if (KERNEL_VERSION(3, 2, 0) <= LINUX_VERSION_CODE)
	CFG80211DBG(DBG_LVL_INFO, ("80211>ssid = %s\n", info->ssid));
	CFG80211DBG(DBG_LVL_INFO, ("80211>ssid_len = %d\n", info->ssid_len));
	CFG80211DBG(DBG_LVL_INFO, ("80211>beacon_ies_len = %d\n", info->beacon_ies_len));
	CFG80211DBG(DBG_LVL_INFO, ("80211>proberesp_ies_len = %d\n", info->proberesp_ies_len));
	CFG80211DBG(DBG_LVL_INFO, ("80211>assocresp_ies_len = %d\n", info->assocresp_ies_len));

	if (info->proberesp_ies_len > 0 && info->proberesp_ies)
		RTMP_DRIVER_80211_AP_PROBE_RSP(pAd, (VOID *)info->proberesp_ies, info->proberesp_ies_len);

	if (info->assocresp_ies_len > 0 && info->assocresp_ies)
		RTMP_DRIVER_80211_AP_ASSOC_RSP(pAd, (VOID *)info->assocresp_ies, info->assocresp_ies_len);

#endif
	os_alloc_mem(NULL, &beacon_head_buf, info->head_len);
	NdisCopyMemory(beacon_head_buf, info->head, info->head_len);
	os_alloc_mem(NULL, &beacon_tail_buf, info->tail_len);
	NdisCopyMemory(beacon_tail_buf, info->tail, info->tail_len);
	bcn.beacon_head_len = info->head_len;
	bcn.beacon_tail_len = info->tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;
	bcn.dtim_period = info->dtim_period;
	bcn.interval = info->interval;
	RTMP_DRIVER_80211_BEACON_ADD(pAd, &bcn);

	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);

	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);

	return 0;
}

static int CFG80211_OpsDelBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev)
{
	VOID *pAd;

	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_ERROR, ("80211> %s ==>\n", __func__));
	RTMP_DRIVER_80211_BEACON_DEL(pAd);
	return 0;
}
#else /* ! LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0) */

static int CFG80211_OpsStartAp(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_ap_settings *settings)
{
	VOID *pAdOrg;
	PRTMP_ADAPTER pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	UCHAR *beacon_head_buf = NULL, *beacon_tail_buf = NULL;
	INT apidx;
	struct wifi_dev *pWdev = NULL;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __FUNCTION__));

	MAC80211_PAD_GET(pAdOrg, pWiphy);
	pAd = (PRTMP_ADAPTER)pAdOrg;

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, netdev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS. \n");
		return FALSE;
	}
	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	NdisZeroMemory(&bcn, sizeof(CMD_RTPRIV_IOCTL_80211_BEACON));
	/* update info into bcn structure */
	bcn.apidx = apidx;
	bcn.pNetDev = netdev;

	if (settings->beacon.head_len > 0) {
		os_alloc_mem(NULL, &beacon_head_buf, settings->beacon.head_len);
		NdisCopyMemory(beacon_head_buf, settings->beacon.head, settings->beacon.head_len);
	}

	if (settings->beacon.tail_len > 0) {
		os_alloc_mem(NULL, &beacon_tail_buf, settings->beacon.tail_len);
		NdisCopyMemory(beacon_tail_buf, settings->beacon.tail, settings->beacon.tail_len);
	}

	bcn.beacon_head_len = settings->beacon.head_len;
	bcn.beacon_tail_len = settings->beacon.tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;
	bcn.dtim_period = settings->dtim_period;
	bcn.interval = settings->beacon_interval;
	bcn.ssid_len = settings->ssid_len;
	bcn.privacy = settings->privacy;

	if (settings->crypto.akm_suites[0] == WLAN_AKM_SUITE_8021X) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> This is a 1X wdev\n"));
		pWdev->IsCFG1xWdev = TRUE;
	} else {
		pWdev->IsCFG1xWdev = FALSE;
	}

#ifdef HOSTAPD_WPA3R3_SUPPORT
	bcn.crypto.sae_pwe = settings->crypto.sae_pwe;
#endif
	NdisZeroMemory(&bcn.ssid[0], MAX_LEN_OF_SSID);
	if (settings->ssid && (settings->ssid_len <= 32))
		NdisCopyMemory(&bcn.ssid[0], settings->ssid, settings->ssid_len);
	bcn.auth_type = settings->auth_type;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
	bcn.hidden_ssid = settings->hidden_ssid;
#endif /*LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)*/


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	/* set channel callback has been replaced by using chandef of cfg80211_ap_settings */
	if (settings->chandef.chan) {
		CFG80211_CB *p80211CB;
		CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;

		/* init */
		memset(&ChanInfo, 0, sizeof(ChanInfo));

		p80211CB = NULL;
		RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

		if (p80211CB == NULL) {
			CFG80211DBG(DBG_LVL_ERROR, ("80211> p80211CB == NULL!\n"));
			if (beacon_head_buf)
				os_free_mem(beacon_head_buf);
			if (beacon_tail_buf)
				os_free_mem(beacon_tail_buf);
			return 0;
		}

		/* get channel number */

		ChanInfo.ChanId = ieee80211_frequency_to_channel(settings->chandef.chan->center_freq);
		ChanInfo.CenterChanId = ieee80211_frequency_to_channel(settings->chandef.center_freq1);
		CFG80211DBG(DBG_LVL_ERROR, ("80211> Channel = %d, CenterChanId = %d\n", ChanInfo.ChanId, ChanInfo.CenterChanId));

		ChanInfo.IfType = RT_CMD_80211_IFTYPE_P2P_GO;

		CFG80211DBG(DBG_LVL_ERROR, ("80211> ChanInfo.IfType == %d!\n", ChanInfo.IfType));

		switch (settings->chandef.width) {
		case NL80211_CHAN_WIDTH_20_NOHT:
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
			break;

		case NL80211_CHAN_WIDTH_20:
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT20;
			break;

		case NL80211_CHAN_WIDTH_40:
			if (settings->chandef.center_freq1 > settings->chandef.chan->center_freq)
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40PLUS;
			else
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40MINUS;
			break;

#ifdef DOT11_VHT_AC
		case NL80211_CHAN_WIDTH_80:
			CFG80211DBG(DBG_LVL_ERROR, ("80211> NL80211_CHAN_WIDTH_80 CtrlCh: %d, CentCh: %d\n", ChanInfo.ChanId, ChanInfo.CenterChanId));
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_VHT80;
			break;

			/* Separated BW 80 and BW 160 is not supported yet */
		case NL80211_CHAN_WIDTH_80P80:
		case NL80211_CHAN_WIDTH_160:
#endif /* DOT11_VHT_AC */

		default:
				CFG80211DBG(DBG_LVL_ERROR, ("80211> Unsupported Chan Width: %d\n", settings->chandef.width));
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
			break;
		}

		CFG80211DBG(DBG_LVL_ERROR, ("80211> ChanInfo.ChanType == %d!\n", ChanInfo.ChanType));
		ChanInfo.MonFilterFlag = p80211CB->MonFilterFlag;

		/* set channel */
		RTMP_DRIVER_80211_CHAN_SET(pAd, &ChanInfo);
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0) */


	RTMP_DRIVER_80211_BEACON_ADD(pAd, &bcn);

	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);
	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);

	return 0;
}

VOID CFG80211_UpdateAssocRespExtraIe(
	VOID *pAdOrg,
	UINT32 apidx,
	UCHAR *assocresp_ies,
	UINT32 assocresp_ies_len)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PUCHAR pAssocRespBuf = (PUCHAR)pAd->ApCfg.MBSSID[apidx].AssocRespExtraIe;

    CFG80211DBG(DBG_LVL_INFO, ("%s: IE len = %d\n", __FUNCTION__, assocresp_ies_len));
    if (assocresp_ies_len > sizeof(pAd->ApCfg.MBSSID[apidx].AssocRespExtraIe)) {
	CFG80211DBG(DBG_LVL_INFO, ("%s: AssocResp buf size not enough\n", __FUNCTION__));
	return;
    }
	NdisCopyMemory(pAssocRespBuf, assocresp_ies, assocresp_ies_len);
	pAd->ApCfg.MBSSID[apidx].AssocRespExtraIeLen = assocresp_ies_len;
}

static int CFG80211_OpsChangeBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_beacon_data *info)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	UCHAR *beacon_head_buf = NULL;
	UCHAR *beacon_tail_buf = NULL;
	const UCHAR *ssid_ie = NULL;
	memset(&bcn, 0, sizeof(CMD_RTPRIV_IOCTL_80211_BEACON));

	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __FUNCTION__));

	if (info->head_len > 0) {
		os_alloc_mem(NULL, &beacon_head_buf, info->head_len);
		if (beacon_head_buf != NULL)
			NdisCopyMemory(beacon_head_buf, info->head, info->head_len);
	}

	if (info->tail_len > 0) {
		os_alloc_mem(NULL, &beacon_tail_buf, info->tail_len);
		if (beacon_tail_buf != NULL)
			NdisCopyMemory(beacon_tail_buf, info->tail, info->tail_len);
	}


	if (beacon_tail_buf != NULL) {
		bcn.beacon_tail = beacon_tail_buf;
		bcn.beacon_tail_len = info->tail_len;
	}

	if (beacon_head_buf != NULL) {
		bcn.beacon_head = beacon_head_buf;
		bcn.beacon_head_len = info->head_len;

		bcn.apidx = get_apidx_by_addr(pAd, bcn.beacon_head+10);
		CFG80211DBG(DBG_LVL_INFO, ("%s apidx %d \n", __FUNCTION__, bcn.apidx));

		ssid_ie = cfg80211_find_ie(WLAN_EID_SSID, bcn.beacon_head+36, bcn.beacon_head_len-36);
		bcn.ssid_len = *(ssid_ie + 1);
		/* Update assoc resp extra ie */
		if (info->assocresp_ies_len && info->assocresp_ies) {
			CFG80211_UpdateAssocRespExtraIe(pAd, bcn.apidx, (UCHAR *)info->assocresp_ies, info->assocresp_ies_len);
		}
	}

	RTMP_DRIVER_80211_BEACON_SET(pAd, &bcn);

	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);
	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);
	return 0;

}

static int CFG80211_OpsStopAp(
	struct wiphy *pWiphy,
	struct net_device *netdev)
{
	VOID *pAd;
	INT apidx;
	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_ERROR, ("80211> %s ==>\n", __func__));
	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, netdev);
	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return FALSE;
	}
	RTMP_DRIVER_80211_BEACON_DEL(pAd, apidx);
	return 0;
}
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0) */
#endif /* CONFIG_AP_SUPPORT */

static int CFG80211_OpsChangeBss(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct bss_parameters *params)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_BSS_PARM bssInfo;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	bssInfo.use_short_preamble = params->use_short_preamble;
	bssInfo.use_short_slot_time = params->use_short_slot_time;
	bssInfo.use_cts_prot = params->use_cts_prot;
	RTMP_DRIVER_80211_CHANGE_BSS_PARM(pAd, &bssInfo);
	return 0;
}

#ifdef HOSTAPD_MAP_SUPPORT
static int CFG80211_OpsStaDel(
	struct wiphy *pWiphy,
	struct net_device *dev,
#if (KERNEL_VERSION(3, 19, 0) <= LINUX_VERSION_CODE)
	struct station_del_parameters *params)
#else
	UINT8 *pMacAddr)
#endif
{
	VOID *pAd;
#if (KERNEL_VERSION(3, 19, 0) <= LINUX_VERSION_CODE)
	const UINT8 *pMacAddr = params->mac;
#endif
	CMD_RTPRIV_IOCTL_AP_STA_DEL rApStaDel = {.pSta_MAC = NULL, .pWdev = NULL};

	MAC80211_PAD_GET(pAd, pWiphy);

	if (dev) {
		rApStaDel.pWdev = RTMP_OS_NETDEV_GET_WDEV(dev);
		CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==> for bssid ("MACSTR")\n",
									__func__, MAC2STR(rApStaDel.pWdev->bssid)));
	} else
		CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s ==>", __func__));

	if (pMacAddr) {
		CFG80211DBG(DBG_LVL_INFO, ("80211> Delete STA("MACSTR") ==>\n",
									MAC2STR(pMacAddr)));
		rApStaDel.pSta_MAC = (UINT8 *)pMacAddr;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
			RTMP_DRIVER_80211_AP_STA_DEL(pAd, (VOID *)&rApStaDel, params->reason_code);
#else
			RTMP_DRIVER_80211_AP_STA_DEL(pAd, (VOID *)&rApStaDel, 0);
#endif
	CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s <==", __func__));

	return 0;
}
#else
static int CFG80211_OpsStaDel(
	struct wiphy *pWiphy,
	struct net_device *dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
	struct station_del_parameters *params)
#else
	UINT8 *pMacAddr)
#endif

{
	VOID *pAd;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
	const UINT8 *pMacAddr = params->mac;
#endif

	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));

	if (pMacAddr ==  NULL)
		RTMP_DRIVER_80211_AP_STA_DEL(pAd, NULL, 0);
	else {
		CFG80211DBG(DBG_LVL_INFO, ("80211> Delete STA("MACSTR") ==>\n",
									MAC2STR(pMacAddr)));
#if (KERNEL_VERSION(3, 19, 0) <= LINUX_VERSION_CODE)
		RTMP_DRIVER_80211_AP_STA_DEL(pAd, (VOID *)pMacAddr, params->reason_code);
#else
		RTMP_DRIVER_80211_AP_STA_DEL(pAd, (VOID *)pMacAddr, 0);
#endif
	}

	return 0;
}
#endif /* HOSTAPD_MAP_SUPPORT */

#if defined(CONFIG_NL80211_TESTMODE) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
static int CFG80211_OpsTestModeCmd(
	struct wiphy *pWiphy,
	IN struct wireless_dev *wdev,
	void *data,
	INT datalen)
{
	VOID *pAd;
	RT_CMD_AP_IOCTL_UPDATE_PMKID pmkid_entry = {0};

	/*copy pentry details from data variable*/
	NdisCopyMemory(&pmkid_entry, data, datalen);

	MAC80211_PAD_GET(pAd, pWiphy);

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s datalen:%d==> ", __func__, datalen));

	hex_dump_with_lvl("testmode_buffer", (unsigned char *) data, datalen, DBG_LVL_INFO);

	RTMP_DRIVER_80211_AP_UPDATE_STA_PMKID(pAd, (VOID *)&pmkid_entry);

	return 0;
}
#endif /*HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/

static int CFG80211_OpsStaAdd(
	struct wiphy *wiphy,
	struct net_device *dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	const UINT8 *mac,
#else
	UINT8 *mac,
#endif

	struct station_parameters *params)
{
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	return 0;
}


static int CFG80211_OpsStaChg(
	struct wiphy *pWiphy,
	struct net_device *dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	const UINT8 *pMacAddr,
#else
	UINT8 *pMacAddr,
#endif
	struct station_parameters *params)
{
	void *pAd;
	CFG80211_CB *p80211CB;

	CFG80211DBG(DBG_LVL_INFO, ("80211> Change STA("MACSTR") ==>\n", MAC2STR(pMacAddr)));
	MAC80211_PAD_GET(pAd, pWiphy);

	p80211CB = NULL;
    RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

    if ((dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_AP) &&
	   (dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_P2P_GO) &&
	   (dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_AP_VLAN))
		return -EOPNOTSUPP;
/*  vikas: used for VLAN, along with auth flag
	if(!(params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED)))
	{
		CFG80211DBG(DBG_LVL_ERROR, ("80211> %x ==>\n", params->sta_flags_mask));
		return -EOPNOTSUPP;
	}
*/
	if (params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
		if (params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			CFG80211DBG(DBG_LVL_INFO, ("80211> STA("MACSTR") ==> PortSecured\n",
				MAC2STR(pMacAddr)));
			RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, (VOID *)pMacAddr, 1);
		} else {
			CFG80211DBG(DBG_LVL_INFO, ("80211> STA("MACSTR") ==> PortNotSecured\n",
				MAC2STR(pMacAddr)));
			RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, (VOID *)pMacAddr, 0);
		}
	}

#ifdef CONFIG_VLAN_GTK_SUPPORT
	if (params->vlan) {
		struct wifi_dev *wdev;
		struct vlan_gtk_info *vg_info;
		MAC_TABLE_ENTRY *pEntry;

		wdev = CFG80211_GetWdevByVlandev((PRTMP_ADAPTER)pAd, params->vlan);
		if (!wdev) {
			CFG80211DBG(DBG_LVL_ERROR, ("%s() invalid params->vlan vlan_dev=%p\n", __func__, params->vlan));
			return -EINVAL;
		}

		pEntry = MacTableLookup(pAd, (UCHAR *)pMacAddr);
		if (!pEntry) {
			CFG80211DBG(DBG_LVL_ERROR, ("%s() can't find STA %02X:%02X:%02X:%02X:%02X:%02X, set VLAN failed\n",
						__func__, PRINT_MAC(pMacAddr)));
			return -EINVAL;
		}

		vg_info = CFG80211_GetVlanInfoByVlandev(wdev, params->vlan);
		pEntry->vlan_id = (vg_info) ? vg_info->vlan_id : 0;
		CFG80211DBG(DBG_LVL_ERROR, ("%s() set STA %02X:%02X:%02X:%02X:%02X:%02X to iface %s with vlan_id %d\n",
				__func__, PRINT_MAC(pMacAddr), params->vlan->name, pEntry->vlan_id));
	}
#endif

	return 0;
}

#ifdef HOSTAPD_HS_R2_SUPPORT
static int CFG80211_OpsSetQosMap(
		IN struct wiphy *wiphy,
		IN struct net_device *dev,
		IN struct cfg80211_qos_map *qos_map)
{

	VOID *pAdOrg;
    PRTMP_ADAPTER pAd;
    INT apidx;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __FUNCTION__));

    MAC80211_PAD_GET(pAdOrg, wiphy);
    pAd = (PRTMP_ADAPTER)pAdOrg;
	/*get mbss from net device and for all the clients associated with that mbss set the given qosmap */

	if (qos_map == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Qos Map NULL\n");
		return 0;
	}

	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, dev);
	if (apidx != WDEV_NOT_FOUND) {
		CFG80211DBG(DBG_LVL_INFO, ("Setting Qos Param for apidx %d \n", apidx));
		RTMP_DRIVER_80211_QOS_PARAM_SET(pAd, qos_map, apidx);
	} else
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AP Index for setting qos map not found\n");

	return 0;
}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
static int CFG80211_OpsSetAntenna(
	struct wiphy *wiphy,
	u32 tx_ant,
	u32 rx_ant)
{

	int ret = 0;
	PRTMP_ADAPTER pAd = NULL;
	u8 tx_ant_count = 0;
	u8 rx_ant_count = 0;
	u32 ant_bitmap = 0;
	CMD_RTPRIV_IOCTL_80211_ANTENNA ant_cnt;

	memset(&ant_cnt, 0, sizeof(CMD_RTPRIV_IOCTL_80211_ANTENNA));
    MAC80211_PAD_GET(pAd, wiphy);

	ant_bitmap = tx_ant;
	while (ant_bitmap & 0x1) {
		tx_ant_count++;
		ant_bitmap >>= 1;
	}

	ant_bitmap = rx_ant;
	while (ant_bitmap & 0x1) {
		rx_ant_count++;
		ant_bitmap >>= 1;
	}

	if (tx_ant_count == 0 || rx_ant_count == 0) {
		ret = -1;
		CFG80211DBG(DBG_LVL_INFO, ("80211> Wrong input parameter!\n"));
	} else {
		CFG80211DBG(DBG_LVL_INFO, ("80211> TX ant count = %x, RX ant count = %x\n", tx_ant_count, rx_ant_count));
		ant_cnt.tx_ant = tx_ant_count;
		ant_cnt.rx_ant = rx_ant_count;
		ret = RTMP_DRIVER_80211_SET_ANTENNA(pAd, &ant_cnt);
	}

	return ret;
}

static int CFG80211_OpsGetAntenna(
	struct wiphy *wiphy,
	u32 *tx_ant,
	u32 *rx_ant)
{
	int ret = 0;
	PRTMP_ADAPTER pAd = NULL;
	CMD_RTPRIV_IOCTL_80211_ANTENNA ant_cnt;

	memset(&ant_cnt, 0, sizeof(CMD_RTPRIV_IOCTL_80211_ANTENNA));
	MAC80211_PAD_GET(pAd, wiphy);

	ret = RTMP_DRIVER_80211_GET_ANTENNA(pAd, &ant_cnt);
	if (ret == 0) {
		*tx_ant = BIT(ant_cnt.tx_ant) - 1;
		*rx_ant = BIT(ant_cnt.rx_ant) - 1;
	}
	return ret;
}
#endif /* ANTENNA_CONTROL_SUPPORT */

#if defined(IWCOMMAND_CFG80211_SUPPORT) &&  !defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
static struct wireless_dev *CFG80211_OpsVirtualInfAdd(
	IN struct wiphy *pWiphy,
	IN const char *name,
	IN unsigned char name_assign_type,
	IN enum nl80211_iftype Type,
	struct vif_params *pParams)
#else
static struct net_device *CFG80211_OpsVirtualInfAdd(
	IN struct wiphy *pWiphy,
	IN char *name,
	IN enum nl80211_iftype Type,
	IN UINT32 *pFlags,
	struct vif_params *pParams)
#endif /* LINUX_VERSION_CODE: 3.6.0 */
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_VIF_SET vifInfo;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	PWIRELESS_DEV pDev = NULL;
#else
	PNET_DEV pDev = NULL;
#endif /* LINUX_VERSION_CODE: 3.6.0 */

	MAC80211_PAD_GET_RETURN_NULL(pAd, pWiphy);
	vifInfo.vifType = Type;
	vifInfo.vifNameLen = strlen(name);
	memset(vifInfo.vifName, 0, sizeof(vifInfo.vifName));
	NdisCopyMemory(vifInfo.vifName, name, vifInfo.vifNameLen);

	if (pWiphy->flags && WIPHY_FLAG_4ADDR_STATION)
		CFG80211DBG(DBG_LVL_DEBUG, ("Support WIPHY_FLAG_4ADDR_STATION\n"));

	vifInfo.flags = pWiphy->flags;

	if (RTMP_DRIVER_80211_VIF_ADD(pAd, &vifInfo) != NDIS_STATUS_SUCCESS)
		return NULL;

#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	pDev = RTMP_CFG80211_FindVifEntryWdev_ByName(pAd, vifInfo.vifName);
#else
	/* Return NetDevice */
	pDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, Type);
#endif /* LINUX_VERSION_CODE: 3.6.0 */

	if (pDev == NULL)
		CFG80211DBG(DBG_LVL_ERROR, ("CFG80211_OpsVirtualInfAdd pDev Null\n"));

	return pDev;
}

static int CFG80211_OpsVirtualInfDel(
	IN struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *pwdev
#else
	IN struct net_device *dev
#endif /* LINUX_VERSION_CODE: 3.6.0 */
)
{
		void *pAd;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
		struct net_device *dev = NULL;

		dev = pwdev->netdev;
		if (!dev)
			return 0;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
		CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s, %s [%d]==>\n", __func__, dev->name, dev->ieee80211_ptr->iftype));
		MAC80211_PAD_GET(pAd, pWiphy);
		RTMP_DRIVER_80211_VIF_DEL(pAd, dev, dev->ieee80211_ptr->iftype);

		return 0;
}
#endif /* IWCOMMAND_CFG80211_SUPPORT && !RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
static struct wireless_dev *CFG80211_OpsVirtualInfAdd(
	IN struct wiphy *pWiphy,
	IN const char *name,
	IN enum nl80211_iftype Type,
	IN u32 *pFlags,
	struct vif_params *pParams)
#else
static struct net_device *CFG80211_OpsVirtualInfAdd(
	IN struct wiphy *pWiphy,
	IN char *name,
	IN enum nl80211_iftype Type,
	IN UINT32 *pFlags,
	struct vif_params *pParams)
#endif /* LINUX_VERSION_CODE: 3.6.0 */
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_VIF_SET vifInfo;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	PWIRELESS_DEV pDev = NULL;
#else
	PNET_DEV pDev = NULL;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	MAC80211_PAD_GET_RETURN_NULL(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s [%s,%d, %d] ==>\n", __func__, name, Type, strlen(name)));
	vifInfo.vifType = Type;
	vifInfo.vifNameLen = strlen(name);
	memset(vifInfo.vifName, 0, sizeof(vifInfo.vifName));
	NdisCopyMemory(vifInfo.vifName, name, vifInfo.vifNameLen);

	if (RTMP_DRIVER_80211_VIF_ADD(pAd, &vifInfo) != NDIS_STATUS_SUCCESS)
		return NULL;

#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	pDev = RTMP_CFG80211_FindVifEntryWdev_ByType(pAd, Type);
#else
	/* Return NetDevice */
	pDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, Type);
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	return pDev;
}

static int CFG80211_OpsVirtualInfDel(
	IN struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *pwdev
#else
	IN struct net_device *dev
#endif /* LINUX_VERSION_CODE: 3.6.0 */
)
{
	void *pAd;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct net_device *dev = NULL;

	dev = pwdev->netdev;

	if (!dev)
		return 0;

#endif /* LINUX_VERSION_CODE: 3.6.0 */
	CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s, %s [%d]==>\n", __func__, dev->name, dev->ieee80211_ptr->iftype));
	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_VIF_DEL(pAd, dev, dev->ieee80211_ptr->iftype);
	return 0;
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */


#ifdef RT_CFG80211_P2P_SUPPORT
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
static int CFG80211_start_p2p_device(
	struct wiphy *pWiphy,
	struct wireless_dev *wdev)
{
	void *pAd;
	struct net_device *dev = wdev->netdev;

	CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s, %s [%d]==>\n", __func__, dev->name, dev->ieee80211_ptr->iftype));
	MAC80211_PAD_GET(pAd, pWiphy);
	return 0;
}

static void CFG80211_stop_p2p_device(
	struct wiphy *pWiphy,
	struct wireless_dev *wdev)
{
	void *pAd;
	struct net_device *dev = wdev->netdev;

	CFG80211DBG(DBG_LVL_DEBUG, ("80211> %s, %s [%d]==>\n", __func__, dev->name, dev->ieee80211_ptr->iftype));
	MAC80211_PAD_GET_NO_RV(pAd, pWiphy);
}
#endif /* LINUX_VERSION_CODE: 3.7.0 */
#endif /* RT_CFG80211_P2P_SUPPORT */

#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
static const struct ieee80211_txrx_stypes
	ralink_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_STATION] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_AP] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4),
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4),
	},

};
#endif

#ifdef SUPP_SAE_SUPPORT
static int mtk_cfg80211_set_connect_params(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 const void *data, int len)
{
	VOID *pAd;
	CHAR staIndex;
	const CMD_RTPRIV_IOCTL_80211_CONNECT_PARAM *param = data;
	struct net_device *pNdev = wdev->netdev;

	MAC80211_PAD_GET(pAd, wiphy);
	if (len < sizeof(CMD_RTPRIV_IOCTL_80211_CONNECT_PARAM)) {
		printk("vendor command too short: %d\n", len);
		return -EINVAL;
	}

	staIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNdev);

	if (staIndex == WDEV_NOT_FOUND) {
		CFG80211DBG(DBG_LVL_ERROR, ("StaIndex for wdev %p netdev %p not found\n", wdev, pNdev));
		return 0;
	} else
		printk("Sta Index %d found for setting connect params \n", staIndex);

	RTMP_DRIVER_80211_CONNECT_PARAM(pAd, (void *)param, staIndex);
	return 0;
}

int mtk_cfg80211_event_connect_params(void *pAd,
						 UCHAR *pmk, int pmk_len)
{
	struct sk_buff *skb = NULL;
	CFG80211_CB *pCfg80211_CB = NULL;
	struct wireless_dev *pCfg80211_Wdev = NULL;

	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);

	pCfg80211_Wdev = pCfg80211_CB->pCfg80211_Wdev;

	skb = cfg80211_vendor_event_alloc(pCfg80211_Wdev->wiphy, NULL,
							pmk_len, 0, GFP_KERNEL);

	if (skb != NULL) {
		if (nla_put(skb, NL80211_ATTR_VENDOR_DATA, pmk_len, pmk)) {
			printk("PMK in vendor data returned error \n");
			kfree_skb(skb);
			return -1;
		}
		cfg80211_vendor_event(skb, GFP_KERNEL);
		return 0;
	}
		else {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"skb is null\n");
			return -1;
		}
}


int mtk_cfg80211_event_pmksa(void *pAd,
		UCHAR *pmk, int pmk_len, UCHAR *pmkid, UINT32 akmp, UINT8 *aa)
{
	struct sk_buff *skb = NULL;
	CFG80211_CB *pCfg80211_CB = NULL;
	struct wireless_dev *pCfg80211_Wdev = NULL;
	MTK_PMKSA_EVENT *pmksa_event = NULL;
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);

	pCfg80211_Wdev = pCfg80211_CB->pCfg80211_Wdev;

	os_alloc_mem(pAd, (UCHAR **)&pmksa_event, sizeof(MTK_PMKSA_EVENT));
	if (!pmksa_event)
		return 0;

	skb = cfg80211_vendor_event_alloc(pCfg80211_Wdev->wiphy, NULL,
						sizeof(MTK_PMKSA_EVENT), 1, GFP_KERNEL);

	os_zero_mem(pmksa_event, sizeof(MTK_PMKSA_EVENT));
	os_move_mem(pmksa_event->pmk, pmk, pmk_len);
	pmksa_event->pmk_len = pmk_len;
	os_move_mem(pmksa_event->pmkid, pmkid, 16);
	pmksa_event->akmp = akmp;
	os_move_mem(pmksa_event->aa, aa, 6);
	if (skb != NULL) {
		if (nla_put(skb, NL80211_ATTR_VENDOR_DATA, sizeof(MTK_PMKSA_EVENT), pmksa_event)) {
			printk("PMK in vendor data return error \n");
			kfree_skb(skb);
			os_free_mem(pmksa_event);
			return -1;
		}
		cfg80211_vendor_event(skb, GFP_KERNEL);

		os_free_mem(pmksa_event);
		return 0;
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"skb is null\n");
		os_free_mem(pmksa_event);
		return -1;
	}
}

#endif

#ifdef IWCOMMAND_CFG80211_SUPPORT
static int CFG80211_SetBitRate(struct wiphy *wiphy,
				      struct net_device *netdev,
				      const u8 *addr,
				      const struct cfg80211_bitrate_mask *mask)
{
	VOID *pAdOrg;
	PRTMP_ADAPTER pAd;
	INT apidx;
	struct wifi_dev *pWdev = NULL;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAdOrg, wiphy);
	pAd = (PRTMP_ADAPTER)pAdOrg;
	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, netdev);

	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS.\n");
		return FALSE;
	}

	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	RTMP_DRIVER_80211_BITRATE_SET(pAd, (VOID *)mask, apidx);

	return 0;
}
#endif /* IWCOMMAND_CFG80211_SUPPORT */

#if (KERNEL_VERSION(3, 0, 0) <= LINUX_VERSION_CODE)
static const struct ieee80211_iface_limit ra_p2p_sta_go_limits[] = {
	{
		.max = 16,
		.types = BIT(NL80211_IFTYPE_STATION) |
		BIT(NL80211_IFTYPE_AP),
	},
};

static const struct ieee80211_iface_combination
	ra_iface_combinations_p2p[] = {
	{
#if defined(RT_CFG80211_P2P_MULTI_CHAN_SUPPORT) || defined(DBDC_MODE)
		.num_different_channels = 2,
#else
		.num_different_channels = 1,
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
#ifdef DBDC_MODE
	.max_interfaces = 8,
#else
	.max_interfaces = 4,
#endif
		/* CFG TODO*/
		/* .beacon_int_infra_match = true, */
		.limits = ra_p2p_sta_go_limits,
		.n_limits = ARRAY_SIZE(ra_p2p_sta_go_limits),
	},
};
#endif /* LINUX_VERSION_CODE: 3.8.0 */

#ifdef SUPP_SAE_SUPPORT

#define OUI_MTK 0x000c43

const struct wiphy_vendor_command mtk_vendor_cmds[] = {
    {
		{
			.vendor_id = OUI_MTK,
			.subcmd = 0
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.policy = VENDOR_CMD_RAW_DATA,
		.doit = mtk_cfg80211_set_connect_params
	},
};

static const struct nl80211_vendor_cmd_info mtk_vendor_events[] = {
	{
		.vendor_id = OUI_MTK,
		.subcmd = 0
	},
	{
		.vendor_id = OUI_MTK,
		.subcmd = 1
	},
};

#endif

struct cfg80211_ops CFG80211_Ops = {
#ifdef CFG_TDLS_SUPPORT
#if (KERNEL_VERSION(3, 4, 0) < LINUX_VERSION_CODE)
	.tdls_mgmt = CFG80211_OpsTdlsMgmt,
	.tdls_oper = CFG80211_OpsTdlsOper,
#endif
#endif /* CFG_TDLS_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

#ifdef IWCOMMAND_CFG80211_SUPPORT
	.set_bitrate_mask = CFG80211_SetBitRate,
#endif /* IWCOMMAND_CFG80211_SUPPORT */

#if defined(IWCOMMAND_CFG80211_SUPPORT) &&  !defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
	.add_virtual_intf = CFG80211_OpsVirtualInfAdd,
	.del_virtual_intf = CFG80211_OpsVirtualInfDel,
#endif /* IWCOMMAND_CFG80211_SUPPORT && !RT_CFG80211_P2P_CONCURRENT_DEVICE */

#if (KERNEL_VERSION(3, 4, 0) > LINUX_VERSION_CODE)
	.set_beacon	= CFG80211_OpsSetBeacon,
	.add_beacon	= CFG80211_OpsAddBeacon,
	.del_beacon	= CFG80211_OpsDelBeacon,
#else
	.start_ap	    = CFG80211_OpsStartAp,
	.change_beacon	= CFG80211_OpsChangeBeacon,
	.stop_ap	    = CFG80211_OpsStopAp,
#endif	/* LINUX_VERSION_CODE 3.4 */
#endif /* CONFIG_AP_SUPPORT */
	/* set channel for a given wireless interface */
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	/* CFG_TODO */
	/* .set_monitor_channel = CFG80211_OpsMonitorChannelSet, */
#if (KERNEL_VERSION(4, 0, 0) <= LINUX_VERSION_CODE)
#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE
	.set_ap_chanwidth 	= CFG80211_OpsChanWithSet,
#endif/*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/
#endif/*KERNEL_VERSION(4, 0, 0)*/

#else
	.set_channel	     = CFG80211_OpsChannelSet,
#endif /* LINUX_VERSION_CODE: 3.6.0 */

	/* change type/configuration of virtual interface */
	.change_virtual_intf		= CFG80211_OpsVirtualInfChg,
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	.add_virtual_intf           = CFG80211_OpsVirtualInfAdd,
	.del_virtual_intf           = CFG80211_OpsVirtualInfDel,
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SUPPORT
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
	.start_p2p_device = CFG80211_start_p2p_device,
	.stop_p2p_device = CFG80211_stop_p2p_device,
#endif /* LINUX_VERSION_CODE: 3.6.0 */
#endif /* RT_CFG80211_P2P_SUPPORT */

#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
	/* request to do a scan */
	/*
	 *	Note: must exist whatever AP or STA mode; Or your kernel will crash
	 *	in v2.6.38.
	 */
	.scan						= CFG80211_OpsScan,
#endif /* LINUX_VERSION_CODE */


#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(3, 8, 0) > LINUX_VERSION_CODE)
	/* set the transmit power according to the parameters */
	.set_tx_power				= CFG80211_OpsTxPwrSet,
	/* store the current TX power into the dbm variable */
	.get_tx_power				= CFG80211_OpsTxPwrGet,
#endif /* LINUX_VERSION_CODE: 3.8.0 */
	/* configure WLAN power management */
	.set_power_mgmt				= CFG80211_OpsPwrMgmt,
	/* get station information for the station identified by @mac */
	.get_station				= CFG80211_OpsStaGet,
	/* dump station callback */
	.dump_station				= CFG80211_OpsStaDump,
	/* notify that wiphy parameters have changed */
	.set_wiphy_params			= CFG80211_OpsWiphyParamsSet,
	/* add a key with the given parameters */
	.add_key					= CFG80211_OpsKeyAdd,
	/* get information about the key with the given parameters */
	.get_key					= CFG80211_OpsKeyGet,
	/* remove a key given the @mac_addr */
	.del_key					= CFG80211_OpsKeyDel,
	/* set the default key on an interface */
	.set_default_key			= CFG80211_OpsKeyDefaultSet,
#ifdef DOT11W_PMF_SUPPORT
	/* set the default mgmt key on an interface */
	.set_default_mgmt_key		= CFG80211_OpsMgmtKeyDefaultSet,
#endif /*DOT11W_PMF_SUPPORT*/
	/* connect to the ESS with the specified parameters */
	.connect					= CFG80211_OpsConnect,
	/* disconnect from the BSS/ESS */
	.disconnect					= CFG80211_OpsDisconnect,
#endif /* LINUX_VERSION_CODE */

#ifdef RFKILL_HW_SUPPORT
	/* polls the hw rfkill line */
	.rfkill_poll				= CFG80211_OpsRFKill,
#endif /* RFKILL_HW_SUPPORT */

#if (KERNEL_VERSION(2, 6, 33) <= LINUX_VERSION_CODE)
	/* get site survey information */
	/* .dump_survey				= CFG80211_OpsSurveyGet, */
	/* cache a PMKID for a BSSID */
	.set_pmksa					= CFG80211_OpsPmksaSet,
	/* delete a cached PMKID */
	.del_pmksa					= CFG80211_OpsPmksaDel,
	/* flush all cached PMKIDs */
	.flush_pmksa				= CFG80211_OpsPmksaFlush,
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(2, 6, 34) <= LINUX_VERSION_CODE)
	/*
	 *	Request the driver to remain awake on the specified
	 *	channel for the specified duration to complete an off-channel
	 *	operation (e.g., public action frame exchange).
	 */
	.remain_on_channel			= CFG80211_OpsRemainOnChannel,
	/* cancel an on-going remain-on-channel operation */
	.cancel_remain_on_channel	=  CFG80211_OpsCancelRemainOnChannel,
#if (KERNEL_VERSION(2, 6, 37) > LINUX_VERSION_CODE)
	/* transmit an action frame */
	.action						= NULL,
#else
	.mgmt_tx                    = CFG80211_OpsMgmtTx,
#endif /* LINUX_VERSION_CODE */
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(2, 6, 38) <= LINUX_VERSION_CODE)
	.mgmt_tx_cancel_wait       = CFG80211_OpsTxCancelWait,
#endif /* LINUX_VERSION_CODE */


#if (KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE)
	/* configure connection quality monitor RSSI threshold */
	.set_cqm_rssi_config		= NULL,
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	/* notify driver that a management frame type was registered */
	.mgmt_frame_register		= CFG80211_OpsMgmtFrameRegister,
#endif /* LINUX_VERSION_CODE : 2.6.37 */

#ifdef HOSTAPD_HS_R2_SUPPORT
	.set_qos_map				= CFG80211_OpsSetQosMap,
#endif
#if (KERNEL_VERSION(2, 6, 38) <= LINUX_VERSION_CODE)
#ifdef ANTENNA_CONTROL_SUPPORT
	/* set antenna configuration (tx_ant, rx_ant) on the device */
	.set_antenna				= CFG80211_OpsSetAntenna,
	/* get current antenna configuration from device (tx_ant, rx_ant) */
	.get_antenna				= CFG80211_OpsGetAntenna,
#endif /* ANTENNA_CONTROL_SUPPORT */
#endif /* LINUX_VERSION_CODE */
	.change_bss                             = CFG80211_OpsChangeBss,
	.del_station                            = CFG80211_OpsStaDel,
	.add_station                            = CFG80211_OpsStaAdd,
	.change_station                         = CFG80211_OpsStaChg,
	/* .set_bitrate_mask                       = CFG80211_OpsBitrateSet, */
#if defined(CONFIG_NL80211_TESTMODE) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
	.testmode_cmd			= CFG80211_OpsTestModeCmd,
#endif /*CONFIG_NL80211_TESTMODE && HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/
};

/* =========================== Global Function ============================== */

static INT CFG80211NetdevNotifierEvent(
	struct notifier_block *nb, ULONG state, VOID *ndev)
{
	VOID *pAd;
	struct net_device *pNev = NULL;
	struct wireless_dev *pWdev = NULL;

	if (!ndev)
		return NOTIFY_DONE;

	pNev = ndev;
	pWdev = pNev->ieee80211_ptr;

	if (!pWdev || !pWdev->wiphy)
		return NOTIFY_DONE;

	MAC80211_PAD_GET(pAd, pWdev->wiphy);

	switch (state) {
	case NETDEV_UNREGISTER:
		break;

	case NETDEV_GOING_DOWN:
		RTMP_DRIVER_80211_NETDEV_EVENT(pAd, pNev, state);
		break;
	}

	return NOTIFY_DONE;
}

struct notifier_block cfg80211_netdev_notifier = {
	.notifier_call = CFG80211NetdevNotifierEvent,
};

/*
 * ========================================================================
 * Routine Description:
 *	Allocate a wireless device.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	pDev			- Generic device interface
 *
 * Return Value:
 *	wireless device
 *
 * Note:
 * ========================================================================
 */
static struct wireless_dev *CFG80211_WdevAlloc(
	IN CFG80211_CB					*pCfg80211_CB,
	IN CFG80211_BAND * pBandInfo,
	IN VOID						*pAd,
	IN struct device				*pDev)
{
	struct wireless_dev *pWdev;
	ULONG *pPriv;
	/*
	 * We're trying to have the following memory layout:
	 *
	 * +------------------------+
	 * | struct wiphy			|
	 * +------------------------+
	 * | pAd pointer			|
	 * +------------------------+
	 */
	os_alloc_mem_suspend(NULL, (UCHAR **)&pWdev, sizeof(*pWdev));
	os_zero_mem((PUCHAR)pWdev, sizeof(*pWdev));

	if (pWdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "80211> Wireless device allocation fail!\n");
		return NULL;
	} /* End of if */

#if defined(PLATFORM_M_STB)
#if (KERNEL_VERSION(3, 0, 0) <= LINUX_VERSION_CODE)
	pWdev->use_4addr =true;
#endif /* LINUX_VERSION_CODE 3.0.0 */
#endif

	pWdev->wiphy = wiphy_new(&CFG80211_Ops, sizeof(ULONG *));

	if (pWdev->wiphy == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "80211> Wiphy device allocation fail!\n");
		goto LabelErrWiphyNew;
	} /* End of if */

	/* keep pAd pointer */
	pPriv = (ULONG *)(wiphy_priv(pWdev->wiphy));
	*pPriv = (ULONG)pAd;
	set_wiphy_dev(pWdev->wiphy, pDev);
#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
	/* max_scan_ssids means in each scan request, how many ssids can driver handle to send probe-req.
	 *  In current design, we only support 1 ssid at a time. So we should set to 1.
	*/
	/* pWdev->wiphy->max_scan_ssids = pBandInfo->MaxBssTable; */
	pWdev->wiphy->max_scan_ssids = 1;
#endif /* KERNEL_VERSION */
#if (KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE)
	/* @NL80211_FEATURE_INACTIVITY_TIMER:
	 * This driver takes care of freeingup
	 * the connected inactive stations in AP mode.
	 */
	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* http://www.permalink.gmane.org/gmane.linux.kernel.wireless.general/86454 */
	pWdev->wiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;
#endif
#ifdef SUPP_SAE_SUPPORT
	pWdev->wiphy->features |= NL80211_FEATURE_SAE;
#endif

	pWdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_AP) | BIT(NL80211_IFTYPE_STATION);
#ifdef CONFIG_VLAN_GTK_SUPPORT
	pWdev->wiphy->interface_modes |= BIT(NL80211_IFTYPE_AP_VLAN);
#endif
#ifdef CONFIG_STA_SUPPORT
	pWdev->wiphy->interface_modes |= BIT(NL80211_IFTYPE_ADHOC);
#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	pWdev->wiphy->interface_modes |= (BIT(NL80211_IFTYPE_P2P_CLIENT)
									  | BIT(NL80211_IFTYPE_P2P_GO));
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
	pWdev->wiphy->software_iftypes |= BIT(NL80211_IFTYPE_P2P_DEVICE);
#endif /* LINUX_VERSION_CODE 3.7.0 */
#endif /* LINUX_VERSION_CODE 2.6.37 */
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
#endif /* CONFIG_STA_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
#if (KERNEL_VERSION(3, 0, 0) <= LINUX_VERSION_CODE)
	pWdev->wiphy->software_iftypes |= (BIT(NL80211_IFTYPE_P2P_CLIENT) | BIT(NL80211_IFTYPE_P2P_GO));
	/* NL80211_IFTYPE_P2P_DEVICE Kernel Symbol start from 3.7 */
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
	pWdev->wiphy->software_iftypes |= BIT(NL80211_IFTYPE_P2P_DEVICE);
#endif /* LINUX_VERSION_CODE 3.7.0 */
#endif /* LINUX_VERSION_CODE 3.0.0 */
#endif /* RT_CFG80211_P2P_SUPPORT */
	/* pWdev->wiphy->reg_notifier = CFG80211_RegNotifier; */
	/* init channel information */
	CFG80211_SupBandInit(pCfg80211_CB, pBandInfo, pWdev->wiphy, NULL, NULL);

#if (KERNEL_VERSION(4, 0, 0) <= LINUX_VERSION_CODE)
#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE
	pWdev->wiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE;
#endif /*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/
#endif /*KERNEL_VERSION(4, 0, 0) */

#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
	/* CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm) */
	pWdev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	pWdev->wiphy->max_scan_ie_len = IEEE80211_MAX_DATA_LEN;
#endif
#if (KERNEL_VERSION(2, 6, 33) <= LINUX_VERSION_CODE)
	pWdev->wiphy->max_num_pmkids = 4;
#endif
#if (KERNEL_VERSION(2, 6, 38) <= LINUX_VERSION_CODE)
	pWdev->wiphy->max_remain_on_channel_duration = 5000;
#endif /* KERNEL_VERSION */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	pWdev->wiphy->mgmt_stypes = ralink_mgmt_stypes;
#endif
#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
	pWdev->wiphy->cipher_suites = CipherSuites;
	pWdev->wiphy->n_cipher_suites = ARRAY_SIZE(CipherSuites);
#endif /* LINUX_VERSION_CODE */
#if (KERNEL_VERSION(3, 2, 0) <= LINUX_VERSION_CODE)
	pWdev->wiphy->flags |= WIPHY_FLAG_AP_UAPSD;
#endif /* LINUX_VERSION_CODE: 3.2.0 */
#if (KERNEL_VERSION(3, 3, 0) <= LINUX_VERSION_CODE)
	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* 018-cfg80211-internal-ap-mlme.patch */
	pWdev->wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;
	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* 008-cfg80211-offchan-flags.patch */
	pWdev->wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
	/* CFG_TODO */
	/* pWdev->wiphy->flags |= WIPHY_FLAG_STRICT_REGULATORY; */
#endif /* LINUX_VERSION_CODE: 3.3.0 */

#ifdef SUPP_SAE_SUPPORT
	/* vendor commands/events support */
	pWdev->wiphy->vendor_commands = mtk_vendor_cmds;
	pWdev->wiphy->n_vendor_commands = 1;
	pWdev->wiphy->vendor_events = 	mtk_vendor_events;
	pWdev->wiphy->n_vendor_events = 2;
#endif
#if defined(PLATFORM_M_STB)
#if (KERNEL_VERSION(3, 0, 0) <= LINUX_VERSION_CODE)
	pWdev->wiphy->flags |= WIPHY_FLAG_4ADDR_STATION;
#endif /* LINUX_VERSION_CODE 3.0.0 */
#endif

	/* Driver Report Support TDLS to supplicant */
#ifdef CFG_TDLS_SUPPORT
	pWdev->wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS;
	pWdev->wiphy->flags |= WIPHY_FLAG_TDLS_EXTERNAL_SETUP;
#endif /* CFG_TDLS_SUPPORT */
	/* CFG_TODO */
	/* pWdev->wiphy->flags |= WIPHY_FLAG_IBSS_RSN; */
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	pWdev->wiphy->iface_combinations = ra_iface_combinations_p2p;
	pWdev->wiphy->n_iface_combinations = ARRAY_SIZE(ra_iface_combinations_p2p);
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		RTMP_ADAPTER *ad = (RTMP_ADAPTER *)pAd;
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
		struct mcs_nss_caps *nss_cap = &cap->mcs_nss;
		pWdev->wiphy->available_antennas_tx = BIT(nss_cap->max_nss) - 1;
		pWdev->wiphy->available_antennas_rx = BIT(nss_cap->max_nss) - 1;
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

#ifdef IWCOMMAND_CFG80211_SUPPORT
	pWdev->wiphy->flags |= WIPHY_FLAG_4ADDR_STATION;
#endif /* IWCOMMAND_CFG80211_SUPPORT */

	if (wiphy_register(pWdev->wiphy) < 0) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "80211> Register wiphy device fail!\n");
		goto LabelErrReg;
	}

	return pWdev;
LabelErrReg:
	wiphy_free(pWdev->wiphy);
LabelErrWiphyNew:
	os_free_mem(pWdev);
	return NULL;
} /* End of CFG80211_WdevAlloc */


/*
 * ========================================================================
 * Routine Description:
 *	Register MAC80211 Module.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pDev			- Generic device interface
 *	pNetDev			- Network device
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	pDev != pNetDev
 *	#define SET_NETDEV_DEV(net, pdev)	((net)->dev.parent = (pdev))
 *
 *	Can not use pNetDev to replace pDev; Or kernel panic.
 * ========================================================================
 */
BOOLEAN CFG80211_Register(
	IN VOID						*pAd,
	IN struct device			*pDev,
	IN struct net_device		*pNetDev)
{
	CFG80211_CB *pCfg80211_CB = NULL;
	CFG80211_BAND BandInfo;
	INT err = 0;
	UINT32 OpMode;
	/* allocate Main Device Info structure */
	os_alloc_mem(NULL, (UCHAR **)&pCfg80211_CB, sizeof(CFG80211_CB));

	if (pCfg80211_CB == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "80211> Allocate MAC80211 CB fail!\n");
		return FALSE;
	}

	/* allocate wireless device */
	RTMP_DRIVER_80211_BANDINFO_GET(pAd, &BandInfo);
	pCfg80211_CB->pCfg80211_Wdev = CFG80211_WdevAlloc(pCfg80211_CB, &BandInfo, pAd, pDev);

	if (pCfg80211_CB->pCfg80211_Wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "80211> Allocate Wdev fail!\n");
		os_free_mem(pCfg80211_CB);
		return FALSE;
	}

	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);

	/* bind wireless device with net device */
#ifdef CONFIG_AP_SUPPORT
	/* default we are AP mode */
	if (OpMode == OPMODE_AP)
		pCfg80211_CB->pCfg80211_Wdev->iftype = NL80211_IFTYPE_AP;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	/* default we are station mode */
	if (OpMode == OPMODE_STA)
		pCfg80211_CB->pCfg80211_Wdev->iftype = NL80211_IFTYPE_STATION;
#endif /* CONFIG_STA_SUPPORT */
	pNetDev->ieee80211_ptr = pCfg80211_CB->pCfg80211_Wdev;
	SET_NETDEV_DEV(pNetDev, wiphy_dev(pCfg80211_CB->pCfg80211_Wdev->wiphy));
	pCfg80211_CB->pCfg80211_Wdev->netdev = pNetDev;
#ifdef RFKILL_HW_SUPPORT
	wiphy_rfkill_start_polling(pCfg80211_CB->pCfg80211_Wdev->wiphy);
#endif /* RFKILL_HW_SUPPORT */
	RTMP_DRIVER_80211_CB_SET(pAd, pCfg80211_CB);
	RTMP_DRIVER_80211_RESET(pAd);
	RTMP_DRIVER_80211_SCAN_STATUS_LOCK_INIT(pAd, TRUE);

	/* TODO */
	/* err = register_netdevice_notifier(&cfg80211_netdev_notifier);	//CFG TODO */
	if (err)
		CFG80211DBG(DBG_LVL_ERROR, ("80211> Failed to register notifierl %d\n", err));

	CFG80211DBG(DBG_LVL_ERROR, ("80211> CFG80211_Register\n"));
	return TRUE;
} /* End of CFG80211_Register */




/* =========================== Local Function =============================== */


#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX_VERSION_CODE */

/* End of crda.c */
