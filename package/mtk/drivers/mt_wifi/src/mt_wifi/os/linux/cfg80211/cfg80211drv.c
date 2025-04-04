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
 *
 ***************************************************************************/
#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"
#define BSSID_WCID_TO_REMOVE 1 /* Pat:TODO */

extern struct notifier_block cfg80211_netdev_notifier;

extern INT RtmpIoctl_rt_ioctl_siwauth(
	IN      RTMP_ADAPTER * pAd,
	IN      VOID * pData,
	IN      ULONG                            Data);

extern INT RtmpIoctl_rt_ioctl_siwauth(
	IN      RTMP_ADAPTER * pAd,
	IN      VOID * pData,
	IN      ULONG                            Data);


INT CFG80211DRV_IoctlHandle(
	IN	VOID * pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID * pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef CONFIG_MULTI_CHANNEL
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
#endif /* CONFIG_MULTI_CHANNEL */

	switch (cmd) {
	case CMD_RTPRIV_IOCTL_80211_START:
	case CMD_RTPRIV_IOCTL_80211_END:
		/* nothing to do */
		break;

	case CMD_RTPRIV_IOCTL_80211_CB_GET:
		*(VOID **)pData = (VOID *)(pAd->pCfg80211_CB);
		break;

	case CMD_RTPRIV_IOCTL_80211_CB_SET:
		pAd->pCfg80211_CB = pData;
		break;

	case CMD_RTPRIV_IOCTL_80211_CHAN_SET:
		if (CFG80211DRV_OpsSetChannel(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;
#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	case CMD_RTPRIV_IOCTL_80211_AP_CHAN_SET:
		if (CFG80211DRV_AP_OpsSetChannel(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;
		break;
#endif/*KERNEL_VERSION(4, 0, 0)*/
#endif/*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/

	case CMD_RTPRIV_IOCTL_80211_VIF_CHG:
		if (CFG80211DRV_OpsChgVirtualInf(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_80211_SCAN:
		if (CFG80211DRV_OpsScanCheckStatus(pAd, Data) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_80211_SCAN_STATUS_LOCK_INIT:
		CFG80211_ScanStatusLockInit(pAd, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_IBSS_JOIN:
		CFG80211DRV_OpsJoinIbss(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_STA_LEAVE:
		CFG80211DRV_OpsLeave(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_STA_GET:
		if (CFG80211DRV_StaGet(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;
#ifdef WIFI_IAP_STA_DUMP_FEATURE
		case CMD_RTPRIV_IOCTL_80211_AP_STA_GET:
			if (CFG80211DRV_Ap_StaGet(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/

#ifdef CFG_TDLS_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_STA_TDLS_INSERT_PENTRY:
		CFG80211DRV_StaTdlsInsertDeletepEntry(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_STA_TDLS_SET_KEY_COPY_FLAG:
		CFG80211DRV_StaTdlsSetKeyCopyFlag(pAd);
		break;
#endif /* CFG_TDLS_SUPPORT */

	case CMD_RTPRIV_IOCTL_80211_STA_KEY_ADD: {
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;

		pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;

		if (
#ifdef CFG80211_MULTI_STA
			RTMP_CFG80211_MULTI_STA_ON(pAd, pKeyInfo->pNetDev) ||
#endif /* CFG80211_MULTI_STA */
			(pKeyInfo->pNetDev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT)
		)
			CFG80211DRV_P2pClientKeyAdd(pAd, pData);
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA*/
#ifdef APCLI_CFG80211_SUPPORT
			CFG80211DRV_ApClientKeyAdd(pAd, pData);
#else
			CFG80211DRV_StaKeyAdd(pAd, pData);
#endif /* APCLI_CFG80211_SUPPORT */
	}
	break;
#ifdef CONFIG_STA_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_STA_KEY_DEFAULT_SET:
		CFG80211_setStaDefaultKey(pAd, pData, Data);
		break;
#endif /*CONFIG_STA_SUPPORT*/

	case CMD_RTPRIV_IOCTL_80211_CONNECT_TO: {
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		CMD_RTPRIV_IOCTL_80211_CONNECT *pConnInfo;

		pConnInfo = (CMD_RTPRIV_IOCTL_80211_CONNECT *)pData;

		if (
#ifdef CFG80211_MULTI_STA
			(RTMP_CFG80211_MULTI_STA_ON(pAd, pConnInfo->pNetDev)) ||
#endif /* CFG80211_MULTI_STA */
			(Data == RT_CMD_80211_IFTYPE_P2P_CLIENT))
			CFG80211DRV_P2pClientConnect(pAd, pData);
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
			CFG80211DRV_Connect(pAd, pData);
	}
	break;

#ifdef SUPP_SAE_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_EXT_CONNECT: {
		UINT8 staIndex = Data;
		SECURITY_CONFIG *pSecConfig;
		struct wifi_dev *pWdev;
		STA_ADMIN_CONFIG *pstacfg = &pAd->StaCfg[staIndex];
		CMD_RTPRIV_IOCTL_80211_CONNECT_PARAM *pConnParam;
		pConnParam = (CMD_RTPRIV_IOCTL_80211_CONNECT_PARAM *)pData;
		pWdev = &pAd->StaCfg[staIndex].wdev;
		pSecConfig = &pWdev->SecConfig;

		if (pConnParam->psk_len < 65) {
			os_move_mem(pSecConfig->PSK, pConnParam->psk, pConnParam->psk_len);
			pstacfg->sae_cfg_group = (UCHAR)pConnParam->sae_group;
			pSecConfig->PSK[pConnParam->psk_len] = '\0';
		}
	}
		break;
#endif
	case CMD_RTPRIV_IOCTL_80211_REG_NOTIFY_TO:
		CFG80211DRV_RegNotify(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_UNREGISTER:

		/* Only main net_dev needs to do CFG80211_UnRegister. */
		if (pAd->net_dev == pData)
			CFG80211_UnRegister(pAd, pData);

		break;

	case CMD_RTPRIV_IOCTL_80211_BANDINFO_GET: {
		CFG80211_BAND *pBandInfo = (CFG80211_BAND *)pData;
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		CFG80211_BANDINFO_FILL(pAd, wdev, pBandInfo);

		if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_24G))
			pBandInfo->RFICType |= RFIC_24GHZ;

		if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_5G))
			pBandInfo->RFICType |= RFIC_5GHZ;

#ifdef CONFIG_6G_SUPPORT
		if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_6G))
			pBandInfo->RFICType |= RFIC_6GHZ;
#endif
	}
	break;

	case CMD_RTPRIV_IOCTL_80211_SURVEY_GET:
		CFG80211DRV_SurveyGet(pAd, pData);
		break;

#ifdef APCLI_CFG80211_SUPPORT
		case CMD_RTPRIV_IOCTL_APCLI_SITE_SURVEY:
			CFG80211DRV_ApcliSiteSurvey(pAd, pData);
			break;
#endif /* APCLI_CFG80211_SUPPORT */


	case CMD_RTPRIV_IOCTL_80211_EXTRA_IES_SET:
		CFG80211DRV_OpsScanExtraIesSet(pAd);
		break;
#ifdef RT_CFG80211_P2P_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_REMAIN_ON_CHAN_SET:
		CFG80211DRV_OpsRemainOnChannel(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_CANCEL_REMAIN_ON_CHAN_SET:
		CFG80211DRV_OpsCancelRemainOnChannel(pAd, Data);
		break;
#endif /* RT_CFG80211_P2P_SUPPORT */

	/* CFG_TODO */
	case CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_REG:
		CFG80211DRV_OpsMgmtFrameProbeRegister(pAd, pData, Data);
		break;

	/* CFG_TODO */
	case CMD_RTPRIV_IOCTL_80211_ACTION_FRAME_REG:
		CFG80211DRV_OpsMgmtFrameActionRegister(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_CHANNEL_LOCK:
		CFG80211_SwitchTxChannel(pAd, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_CHANNEL_RESTORE:
		break;

	case CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_SEND:
		CFG80211_SendMgmtFrame(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_CHANNEL_LIST_SET:
		return CFG80211DRV_OpsScanSetSpecifyChannel(pAd, pData, Data);
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

	case CMD_RTPRIV_IOCTL_MCC_DHCP_PROTECT_STATUS:
		pApCliEntry = &pAd->StaCfg[0];
		*(UCHAR *)pData = pApCliEntry->ApcliInfStat.Valid;
		break;

	case CMD_RTPRIV_IOCTL_80211_SET_NOA:
		CFG80211DRV_Set_NOA(pAd, Data);
		break;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_POWER_MGMT_SET:
		CFG80211_setPowerMgmt(pAd, Data);
		break;
#endif /* RT_CFG80211_P2P_SUPPORT */
#ifdef WIFI_IAP_POWER_SAVE_FEATURE
		case CMD_RTPRIV_IOCTL_80211_AP_POWER_MGMT_SET:
			CFG80211DRV_AP_SetPowerMgmt(pAd, pData, Data);
			break;
#endif/*WIFI_IAP_POWER_SAVE_FEATURE */

	case CMD_RTPRIV_IOCTL_80211_BEACON_SET:
		CFG80211DRV_OpsBeaconSet(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_BEACON_ADD:
		CFG80211DRV_OpsBeaconAdd(pAd, pData);
		break;

#ifdef HOSTAPD_HS_R2_SUPPORT
		case CMD_RTPRIV_IOCTL_SET_QOS_PARAM:
			CFG80211DRV_SetQosParam(pAd, pData, Data);
			break;
#endif

	case CMD_RTPRIV_IOCTL_80211_BEACON_DEL: {

		INT i, apidx = Data;

			for (i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
				pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[i] = 0;
			if (pAd->cfg80211_ctrl.beacon_tail_buf != NULL) {
				os_free_mem(pAd->cfg80211_ctrl.beacon_tail_buf);
				pAd->cfg80211_ctrl.beacon_tail_buf = NULL;
			}
			pAd->cfg80211_ctrl.beacon_tail_len = 0;
	}
	break;

	case CMD_RTPRIV_IOCTL_80211_AP_KEY_ADD:
		CFG80211DRV_ApKeyAdd(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_RTS_THRESHOLD_ADD:
		CFG80211DRV_RtsThresholdAdd(pAd, wdev, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_FRAG_THRESHOLD_ADD:
		CFG80211DRV_FragThresholdAdd(pAd, wdev, Data);
		break;
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_ACK_THRESHOLD_ADD:
	if (CFG80211DRV_AckThresholdAdd(pAd, wdev, Data) != TRUE)
		return NDIS_STATUS_FAILURE;
	break;
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/
	case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEL:
		CFG80211DRV_ApKeyDel(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_SET:
		CFG80211_setApDefaultKey(pAd, pData, Data);
		break;

#ifdef DOT11W_PMF_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_MGMT_SET:
		CFG80211_setApDefaultMgmtKey(pAd, pData, Data);
		break;
#endif /*DOT11W_PMF_SUPPORT*/


	case CMD_RTPRIV_IOCTL_80211_PORT_SECURED:
		CFG80211_StaPortSecured(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_AP_STA_DEL:
		CFG80211_ApStaDel(pAd, pData, Data);
		break;

#ifdef HOSTAPD_PMKID_IN_DRIVER_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_AP_UPDATE_STA_PMKID:
		CFG80211_ApUpdateStaPmkid(pAd, pData);
		break;
#endif /*HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/

#endif /* CONFIG_AP_SUPPORT */

	case CMD_RTPRIV_IOCTL_80211_CHANGE_BSS_PARM:
		CFG80211DRV_OpsChangeBssParm(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_AP_PROBE_RSP_EXTRA_IE:
		break;

	case CMD_RTPRIV_IOCTL_80211_BITRATE_SET:
		CFG80211DRV_OpsBitRateParm(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_RESET:
		CFG80211_reSetToDefault(pAd);
		break;

	case CMD_RTPRIV_IOCTL_80211_NETDEV_EVENT: {
		/*
		 * CFG_TODO: For Scan_req per netdevice
		 * PNET_DEV pNetDev = (PNET_DEV) pData;
		 * struct wireless_dev *pWdev = pAd->pCfg80211_CB->pCfg80211_Wdev;
		 * if (RTMPEqualMemory(pNetDev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
		 */
		if (pAd->cfg80211_ctrl.FlgCfg80211Scanning == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CFG_SCAN: close the scan cmd in device close phase\n");
			CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
			pAd->cfg80211_ctrl.FlgCfg80211Scanning = FALSE;
		}
	}
	break;
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)

	case CMD_RTPRIV_IOCTL_80211_P2PCLI_ASSSOC_IE_SET: {
		CMD_RTPRIV_IOCTL_80211_ASSOC_IE *pAssocIe;

		pAssocIe = (CMD_RTPRIV_IOCTL_80211_ASSOC_IE *)pData;
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)

		if (
#ifdef CFG80211_MULTI_STA
			RTMP_CFG80211_MULTI_STA_ON(pAd, pAssocIe->pNetDev) ||
#endif /* CFG80211_MULTI_STA */
			(Data == RT_CMD_80211_IFTYPE_P2P_CLIENT)
		)
			CFG80211DRV_SetP2pCliAssocIe(pAd, pAssocIe->ie, pAssocIe->ie_len);
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#ifdef APCLI_CFG80211_SUPPORT
			CFG80211DRV_SetApCliAssocIe(pAd, pAssocIe->pNetDev, pAssocIe->ie, pAssocIe->ie_len);
#else
			RTMP_DRIVER_80211_GEN_IE_SET(pAd, pAssocIe->ie, pAssocIe->ie_len);
#endif

	}
	break;
#endif /*CONFIG_STA_SUPPORT || APCLI_CFG80211_SUPPORT*/
#if defined(IWCOMMAND_CFG80211_SUPPORT) &&  !defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
	case CMD_RTPRIV_IOCTL_80211_VIF_ADD:
		if (CFG80211DRV_OpsVifAdd(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_80211_VIF_DEL:
		RTMP_CFG80211_VirtualIF_Remove(pAd, pData, Data);
		break;
#endif /* IWCOMMAND_CFG80211_SUPPORT && !RT_CFG80211_P2P_CONCURRENT_DEVICE */

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)

	case CMD_RTPRIV_IOCTL_80211_VIF_ADD:
		if (CFG80211DRV_OpsVifAdd(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_80211_VIF_DEL:
		RTMP_CFG80211_VirtualIF_Remove(pAd, pData, Data);
		break;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#ifdef RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_ANDROID_PRIV_CMD:
		/* rt_android_private_command_entry(pAd, ); */
		break;
#endif /* RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT */
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT

	case CMD_RTPRIV_IOCTL_80211_SEND_WIRELESS_EVENT:
		CFG80211_SendWirelessEvent(pAd, pData);
		break;
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
#ifdef RFKILL_HW_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_RFKILL: {
		UINT32 data = 0;
		BOOLEAN active;
		/* Read GPIO pin2 as Hardware controlled radio state */
		RTMP_IO_READ32(pAd->hdev_ctrl, GPIO_CTRL_CFG, &data);
		active = !!(data & 0x04);

		if (!active) {
			RTMPSetLED(pAd, LED_RADIO_OFF, DBDC_BAND0);
			*(UINT8 *)pData = 0;
		} else
			*(UINT8 *)pData = 1;
	}
	break;
#endif /* RFKILL_HW_SUPPORT */

	case CMD_RTPRIV_IOCTL_80211_REGISTER:

		/* Only main net_dev needs to do CFG80211_Register. */
		if (pAd->net_dev == pData)
			CFG80211_Register(pAd, pObj->pDev, pAd->net_dev);

		break;

#ifdef ANTENNA_CONTROL_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_ANTENNA_CTRL:
	{
		BOOLEAN is_get = (Data == 1) ? TRUE : FALSE;
		CMD_RTPRIV_IOCTL_80211_ANTENNA *ant_cnt = (CMD_RTPRIV_IOCTL_80211_ANTENNA *)pData;
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		struct mcs_nss_caps *nss_cap = &cap->mcs_nss;

		if (is_get) {
			if (OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED) &&
				(wdev != NULL) && (wdev->if_up_down_state == TRUE)) {
#ifdef DOT11_HE_AX
				if (WMODE_CAP_AX(wdev->PhyMode)) {
					ant_cnt->rx_ant = wlan_config_get_he_rx_nss(wdev);
					ant_cnt->tx_ant = wlan_config_get_he_tx_nss(wdev);
				} else
#endif /* DOT11_HE_AX */
				{
					ant_cnt->tx_ant = wlan_config_get_tx_stream(wdev);
					ant_cnt->rx_ant = wlan_config_get_rx_stream(wdev);
				}
			} else {
				UINT tx_max_nss;
				UINT rx_max_nss;
#ifdef DBDC_MODE
				if (pAd->CommonCfg.dbdc_mode) {
					tx_max_nss = pAd->dbdc_band0_tx_path;
					rx_max_nss = pAd->dbdc_band0_rx_path;
				} else
#endif
				{
					tx_max_nss = nss_cap->max_nss;
					rx_max_nss = nss_cap->max_nss;
				}
				ant_cnt->tx_ant = tx_max_nss;
				ant_cnt->rx_ant = rx_max_nss;
			}
		} else {
			UINT8 Txstream = ant_cnt->tx_ant;
			UINT8 Rxstream = ant_cnt->rx_ant;
			UINT8 i;

			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev is null, return!!!\n");
				return NDIS_STATUS_FAILURE;
			}

			if (Txstream != Rxstream) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Wrong Input: Tx & Rx Antenna number different!\n");
				return NDIS_STATUS_FAILURE;
			}
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				UINT dbdc_tx_max_nss;
				UINT dbdc_rx_max_nss;

				dbdc_tx_max_nss = pAd->dbdc_band0_tx_path;
				dbdc_rx_max_nss = pAd->dbdc_band0_rx_path;

				if ((Txstream > dbdc_tx_max_nss) || (Rxstream > dbdc_rx_max_nss)) {

					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Wrong Input: More than max DBDC ant cap (%d)!!", dbdc_tx_max_nss);
					return NDIS_STATUS_FAILURE;
				}
			} else
#endif
			{
				if (Txstream > nss_cap->max_nss) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Wrong Input: More than max ant cap (%d)!!\n", nss_cap->max_nss);
					return NDIS_STATUS_FAILURE;
				}
			}

			for (i = 0; i < DBDC_BAND_NUM; i++) {
				pAd->TxStream[i] = Txstream;
				pAd->RxStream[i] = Rxstream;
				pAd->bAntennaSetAPEnable[i] = 1;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s : Band_idx=%d, Tx_Stream=%d, Rx_Stream=%d\n",
					__func__, i, pAd->TxStream[i], pAd->RxStream[i]));
			}

			wlan_config_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
			wlan_config_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));
			wlan_operate_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
			wlan_operate_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));
#ifdef DOT11_HE_AX
			wlan_config_set_he_tx_nss(wdev, min(Txstream, nss_cap->max_nss));
			wlan_config_set_he_rx_nss(wdev, min(Rxstream, nss_cap->max_nss));
#endif /* DOT11_HE_AX */
			SetCommonHtVht(pAd, wdev);
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				APStop(pAd, NULL, AP_BSS_OPER_ALL);
				APStartUp(pAd, NULL, AP_BSS_OPER_ALL);
			}
#endif /* CONFIG_AP_SUPPORT */
		}
	}
	break;
#endif /* ANTENNA_CONTROL_SUPPORT */

	default:
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

VOID CFG80211DRV_OpsMgmtFrameProbeRegister(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	BOOLEAN                                          isReg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	PNET_DEV pNewNetDev = (PNET_DEV) pData;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV       pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;
	/* Search the CFG80211 VIF List First */
	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
			break;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	/* Check The Registration is for VIF Device */
	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		(pDevEntry != NULL)) {
		if (isReg) {
			if (pDevEntry->Cfg80211ProbeReqCount < 255)
				pDevEntry->Cfg80211ProbeReqCount++;
		} else {
			if (pDevEntry->Cfg80211ProbeReqCount > 0)
				pDevEntry->Cfg80211ProbeReqCount--;
		}

		if (pDevEntry->Cfg80211ProbeReqCount > 0)
			pDevEntry->Cfg80211RegisterProbeReqFrame = TRUE;
		else
			pDevEntry->Cfg80211RegisterProbeReqFrame = FALSE;

		return;
	}

#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

	/* IF Not Exist on VIF List, the device must be MAIN_DEV */
	if (isReg) {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount < 255)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount++;
	} else {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount > 0)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount--;
	}

	if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount > 0)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = TRUE;
	else {
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "[%d] pAd->Cfg80211RegisterProbeReqFrame=%d[%d]\n",
			 isReg, pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame,
			 pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount);
}

VOID CFG80211DRV_OpsMgmtFrameActionRegister(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	BOOLEAN                                          isReg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	PNET_DEV pNewNetDev = (PNET_DEV) pData;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV       pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;
	/* Search the CFG80211 VIF List First */
	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
			break;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	/* Check The Registration is for VIF Device */
	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		(pDevEntry != NULL)) {
		if (isReg) {
			if (pDevEntry->Cfg80211ActionCount < 255)
				pDevEntry->Cfg80211ActionCount++;
		} else {
			if (pDevEntry->Cfg80211ActionCount > 0)
				pDevEntry->Cfg80211ActionCount--;
		}

		if (pDevEntry->Cfg80211ActionCount > 0)
			pDevEntry->Cfg80211RegisterActionFrame = TRUE;
		else
			pDevEntry->Cfg80211RegisterActionFrame = FALSE;

		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "[%d] TYPE pDevEntry->Cfg80211RegisterActionFrame=%d[%d]\n",
				 isReg, pDevEntry->Cfg80211RegisterActionFrame, pDevEntry->Cfg80211ActionCount);
		return;
	}

#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

	/* IF Not Exist on VIF List, the device must be MAIN_DEV */
	if (isReg) {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount < 255)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount++;
	} else {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount > 0)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount--;
	}

	if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount > 0)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = TRUE;
	else {
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE;
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "[%d] TYPE pAd->Cfg80211RegisterActionFrame=%d[%d]\n",
			 isReg, pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame,
			 pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount);
}

VOID CFG80211DRV_OpsChangeBssParm(
	VOID                                            *pAdOrg,
	VOID                                            *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_BSS_PARM *pBssInfo;
	BOOLEAN TxPreamble;

	CFG80211DBG(DBG_LVL_INFO, ("%s\n", __func__));
	pBssInfo = (CMD_RTPRIV_IOCTL_80211_BSS_PARM *)pData;

	/* Short Preamble */
	if (pBssInfo->use_short_preamble != -1) {
		CFG80211DBG(DBG_LVL_INFO, ("%s: ShortPreamble %d\n", __func__, pBssInfo->use_short_preamble));
		pAd->CommonCfg.TxPreamble = (pBssInfo->use_short_preamble == 0 ? Rt802_11PreambleLong : Rt802_11PreambleShort);
		TxPreamble = (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1);
		MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);
	}

	/* CTS Protection */
	if (pBssInfo->use_cts_prot != -1)
		CFG80211DBG(DBG_LVL_INFO, ("%s: CTS Protection %d\n", __func__, pBssInfo->use_cts_prot));

	/* Short Slot */
	if (pBssInfo->use_short_slot_time != -1)
		CFG80211DBG(DBG_LVL_INFO, ("%s: Short Slot %d\n", __func__, pBssInfo->use_short_slot_time));
}


VOID CFG80211DRV_OpsBitRateParm(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	INT												apidx)
{
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	struct wifi_dev *wdev = NULL;
	struct cfg80211_bitrate_mask *mask;
	INT band = 0, i = 0;
	UCHAR rate[] = { 0x82, 0x84, 0x8b, 0x96, 0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c};
	struct legacy_rate *eap_legacy_rate = NULL;
	UCHAR tx_stream = 0;

	mask = (struct cfg80211_bitrate_mask *)pData;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	eap_legacy_rate = &wdev->eap.eap_legacy_rate;

	if (wdev->channel > 14) {
		band = 1;
		/* legacy mode*/
		if ((mask->control[1].legacy == 0) || (mask->control[1].legacy == 0xff)) {
			wdev->eap.eap_suprate_en = FALSE;
			RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
		} else {
			wdev->eap.eap_suprate_en = TRUE;
			eap_legacy_rate->sup_rate_len = 0;
			eap_legacy_rate->ext_rate_len = 0;

			for (i = 0; i < 8; i++) {
				if (mask->control[1].legacy & (1 << i)) {
					eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[i+4];
					eap_legacy_rate->sup_rate_len++;
					wdev->eap.eapsupportofdmmcs |= (1 << i);
					wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
				}
			}
			rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);
		}
	} else {
		band = 0;
		if ((mask->control[0].legacy == 0) || (mask->control[0].legacy == 0xfff)) {
			wdev->eap.eap_suprate_en = FALSE;
			RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
		} else {
			wdev->eap.eap_suprate_en = TRUE;
			eap_legacy_rate->sup_rate_len = 0;
			eap_legacy_rate->ext_rate_len = 0;
			for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
			    if (mask->control[0].legacy & (1 << i)) {
				if (WMODE_EQUAL(wdev->PhyMode, WMODE_B) && (wdev->channel <= 14)) {
					eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[i];
					eap_legacy_rate->sup_rate_len++;
					wdev->eap.eapsupportcckmcs |= (1 << i);
					wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
				} else {
					if ((i < 4) || (i == 5) || (i == 7) || (i == 9) || (i == 11)) {
						eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len]
												= rate[i];
						eap_legacy_rate->sup_rate_len++;
						if (i < 4) {
							wdev->eap.eapsupportcckmcs |= (1 << i);
							wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
						} else {
							wdev->eap.eapsupportofdmmcs |= (1 << (i - 4));
							wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
						}
					} else {
						eap_legacy_rate->ext_rate[eap_legacy_rate->ext_rate_len]
											= rate[i] & 0x7f;
						eap_legacy_rate->ext_rate_len++;
						wdev->eap.eapsupportofdmmcs |= (1 << (i - 4));
						wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
					}
				}
			    }
			}
			rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);
		}
	}

	tx_stream = wlan_config_get_tx_stream(wdev);
	if (tx_stream == 1) {
		/* HT mode*/
		if ((mask->control[band].ht_mcs[0] == 0) || (mask->control[band].ht_mcs[0] == 0xff)) {
			wdev->eap.eap_htsuprate_en = FALSE;
		} else {
			wdev->eap.eap_htsuprate_en = TRUE;
			wdev->eap.eapmcsset[0] = mask->control[band].ht_mcs[0];
			wdev->eap.eapmcsset[1] = 0;
			wdev->eap.eapmcsset[2] = 0;
			wdev->eap.eapmcsset[3] = 0;
		}
		/* VHT mode*/
		if ((mask->control[band].vht_mcs[0] == 0) || (mask->control[band].vht_mcs[0] == 0x3ff)) {
			wdev->eap.eap_vhtsuprate_en = FALSE;
		} else {
			wdev->eap.eap_vhtsuprate_en = TRUE;
			if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3)
				wdev->eap.rx_mcs_map.mcs_ss1 = 2;
			else
				wdev->eap.rx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
			wdev->eap.rx_mcs_map.mcs_ss2 = 0;
			wdev->eap.rx_mcs_map.mcs_ss3 = 0;
			wdev->eap.rx_mcs_map.mcs_ss4 = 0;
			if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3)
				wdev->eap.tx_mcs_map.mcs_ss1 = 2;
			else
				wdev->eap.tx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
			wdev->eap.tx_mcs_map.mcs_ss2 = 0;
			wdev->eap.tx_mcs_map.mcs_ss3 = 0;
			wdev->eap.tx_mcs_map.mcs_ss4 = 0;
		}
	} else if (tx_stream == 2) {
		/* HT mode*/
		if (((mask->control[band].ht_mcs[0] == 0) && (mask->control[band].ht_mcs[1] == 0))
			|| ((mask->control[band].ht_mcs[0] == 0xff) && (mask->control[band].ht_mcs[1] == 0xff))) {
			wdev->eap.eap_htsuprate_en = FALSE;
		} else {
			wdev->eap.eap_htsuprate_en = TRUE;
			wdev->eap.eapmcsset[0] = mask->control[band].ht_mcs[0];
			wdev->eap.eapmcsset[1] = mask->control[band].ht_mcs[1];
			wdev->eap.eapmcsset[2] = 0;
			wdev->eap.eapmcsset[3] = 0;
		}
		/* VHT mode*/
		if (((mask->control[band].vht_mcs[0] == 0) && (mask->control[band].vht_mcs[1] == 0))
				|| ((mask->control[band].vht_mcs[0] == 0x3ff)
							&& (mask->control[band].vht_mcs[1] == 0x3ff))) {
			wdev->eap.eap_vhtsuprate_en = FALSE;
		} else {
			wdev->eap.eap_vhtsuprate_en = TRUE;
			if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3)
				wdev->eap.rx_mcs_map.mcs_ss1 = 2;
			else
				wdev->eap.rx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;

			if (((mask->control[band].vht_mcs[1] & 0x00000300) >> 8) == 3)
				wdev->eap.rx_mcs_map.mcs_ss2 = 2;
			else
				wdev->eap.rx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
			wdev->eap.rx_mcs_map.mcs_ss3 = 0;
			wdev->eap.rx_mcs_map.mcs_ss4 = 0;
			if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3)
				wdev->eap.tx_mcs_map.mcs_ss1 = 2;
			else
				wdev->eap.tx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;

			if (((mask->control[band].vht_mcs[1] & 0x00000300) >> 8) == 3)
				wdev->eap.tx_mcs_map.mcs_ss2 = 2;
			else
				wdev->eap.tx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
			wdev->eap.tx_mcs_map.mcs_ss3 = 0;
			wdev->eap.tx_mcs_map.mcs_ss4 = 0;
		}
	} else if (tx_stream == 3) {
		/* HT mode*/
		if (((mask->control[band].ht_mcs[0] == 0) && (mask->control[band].ht_mcs[1] == 0)
			&& (mask->control[band].ht_mcs[2] == 0))
			|| ((mask->control[band].ht_mcs[0] == 0xff) && (mask->control[band].ht_mcs[1] == 0xff)
			&& (mask->control[band].ht_mcs[2] == 0xff))) {
			wdev->eap.eap_htsuprate_en = FALSE;
		} else {
			wdev->eap.eap_htsuprate_en = TRUE;
			wdev->eap.eapmcsset[0] = mask->control[band].ht_mcs[0];
			wdev->eap.eapmcsset[1] = mask->control[band].ht_mcs[1];
			wdev->eap.eapmcsset[2] = mask->control[band].ht_mcs[2];
			wdev->eap.eapmcsset[3] = 0;
		}
		/* VHT mode*/
		if (((mask->control[band].vht_mcs[0] == 0) && (mask->control[band].vht_mcs[1] == 0)
			 && (mask->control[band].vht_mcs[2] == 0))
			|| ((mask->control[band].vht_mcs[0] == 0x3ff) && (mask->control[band].vht_mcs[1] == 0x3ff)
			 && (mask->control[band].vht_mcs[2] == 0x3ff))) {
			wdev->eap.eap_vhtsuprate_en = FALSE;
		} else {
			wdev->eap.eap_vhtsuprate_en = TRUE;
			if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3) {
				wdev->eap.rx_mcs_map.mcs_ss1 = 2;
				wdev->eap.tx_mcs_map.mcs_ss1 = 2;
			} else {
				wdev->eap.rx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
				wdev->eap.tx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
			}

			if (((mask->control[band].vht_mcs[1] & 0x00000300) >> 8) == 3) {
				wdev->eap.rx_mcs_map.mcs_ss2 = 2;
				wdev->eap.tx_mcs_map.mcs_ss2 = 2;
			} else {
				wdev->eap.rx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
				wdev->eap.tx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
			}

			if (((mask->control[band].vht_mcs[2] & 0x00000300) >> 8) == 3) {
				wdev->eap.rx_mcs_map.mcs_ss3 = 2;
				wdev->eap.tx_mcs_map.mcs_ss3 = 2;
			} else {
				wdev->eap.rx_mcs_map.mcs_ss3 = (mask->control[band].vht_mcs[2] & 0x00000300) >> 8;
				wdev->eap.tx_mcs_map.mcs_ss3 = (mask->control[band].vht_mcs[2] & 0x00000300) >> 8;
			}

			wdev->eap.rx_mcs_map.mcs_ss4 = 0;
			wdev->eap.tx_mcs_map.mcs_ss4 = 0;
		}
	} else if (tx_stream == 4) {
		/* HT mode*/
		if (((mask->control[band].ht_mcs[0] == 0) && (mask->control[band].ht_mcs[1] == 0)
			  && (mask->control[band].ht_mcs[2] == 0) && (mask->control[band].ht_mcs[3] == 0))
			|| ((mask->control[band].ht_mcs[0] == 0xff) && (mask->control[band].ht_mcs[1] == 0xff)
			  && (mask->control[band].ht_mcs[2] == 0xff) && (mask->control[band].ht_mcs[3] == 0xff))) {
			wdev->eap.eap_htsuprate_en = FALSE;
		} else {
			wdev->eap.eap_htsuprate_en = TRUE;
			wdev->eap.eapmcsset[0] = mask->control[band].ht_mcs[0];
			wdev->eap.eapmcsset[1] = mask->control[band].ht_mcs[1];
			wdev->eap.eapmcsset[2] = mask->control[band].ht_mcs[2];
			wdev->eap.eapmcsset[3] = mask->control[band].ht_mcs[3];
		}
		/* VHT mode*/
		if (((mask->control[band].vht_mcs[0] == 0) && (mask->control[band].vht_mcs[1] == 0)
			  && (mask->control[band].vht_mcs[2] == 0) && (mask->control[band].vht_mcs[3] == 0))
			|| ((mask->control[band].vht_mcs[0] == 0x3ff) && (mask->control[band].vht_mcs[1] == 0x3ff)
			  && (mask->control[band].vht_mcs[2] == 0x3ff) && (mask->control[band].vht_mcs[3] == 0x3ff))) {
			wdev->eap.eap_vhtsuprate_en = FALSE;
		} else {
			wdev->eap.eap_vhtsuprate_en = TRUE;
			if (((mask->control[band].vht_mcs[0] & 0x00000300) >> 8) == 3) {
				wdev->eap.rx_mcs_map.mcs_ss1 = 2;
				wdev->eap.tx_mcs_map.mcs_ss1 = 2;
			} else {
				wdev->eap.rx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
				wdev->eap.tx_mcs_map.mcs_ss1 = (mask->control[band].vht_mcs[0] & 0x00000300) >> 8;
			}

			if (((mask->control[band].vht_mcs[1] & 0x00000300) >> 8) == 3) {
				wdev->eap.rx_mcs_map.mcs_ss2 = 2;
				wdev->eap.tx_mcs_map.mcs_ss2 = 2;
			} else {
				wdev->eap.rx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
				wdev->eap.tx_mcs_map.mcs_ss2 = (mask->control[band].vht_mcs[1] & 0x00000300) >> 8;
			}

			if (((mask->control[band].vht_mcs[2] & 0x00000300) >> 8) == 3) {
				wdev->eap.rx_mcs_map.mcs_ss3 = 2;
				wdev->eap.tx_mcs_map.mcs_ss3 = 2;
			} else {
				wdev->eap.rx_mcs_map.mcs_ss3 = (mask->control[band].vht_mcs[2] & 0x00000300) >> 8;
				wdev->eap.tx_mcs_map.mcs_ss3 = (mask->control[band].vht_mcs[2] & 0x00000300) >> 8;
			}

			if (((mask->control[band].vht_mcs[3] & 0x00000300) >> 8) == 3) {
				wdev->eap.rx_mcs_map.mcs_ss4 = 2;
				wdev->eap.tx_mcs_map.mcs_ss4 = 2;
			} else {
				wdev->eap.rx_mcs_map.mcs_ss4 = (mask->control[band].vht_mcs[3] & 0x00000300) >> 8;
				wdev->eap.tx_mcs_map.mcs_ss4 = (mask->control[band].vht_mcs[3] & 0x00000300) >> 8;
			}
		}
	}
#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE
void CFG8021DRV_AP_PHYINIT(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 channel)
{
	UINT8 bandidx = 0;
	CHANNEL_CTRL *pchctrl = NULL;

	/*phy mode setting */
	if (channel > 14) {
		wdev->PhyMode = (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G);
	} else {
		wdev->PhyMode = (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G);
	}
	/*change channel state to NONE*/
	bandidx = HcGetBandByWdev(wdev);
	pchctrl = hc_get_channel_ctrl(pAd->hdev_ctrl, bandidx);
	hc_set_ChCtrlChListStat(pchctrl, CH_LIST_STATE_NONE);
#ifdef EXT_BUILD_CHANNEL_LIST
	BuildChannelListEx(pAd, wdev);
#else
	BuildChannelList(pAd, wdev);
#endif
	RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);

	return;
}

BOOLEAN CFG80211DRV_AP_SetChanBw_byWdev(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	CMD_RTPRIV_IOCTL_80211_CHAN *pchan_info)
{
	UCHAR bw = 0;
	UCHAR ext_cha = EXTCHA_BELOW;
	INT32 ret = FALSE;

	if (!pAd || !wdev || !pchan_info) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! invalid parameter. pAd=%p,wdev=%p, pchan_info=%p\n",
				pAd, wdev, pchan_info);
		return FALSE;
	}

	/*kernel4.4 set bw*/
	if (MTK_NL80211_CHAN_WIDTH_20_NOHT == pchan_info->ChanType) {
		/*bw and ht check*/
		bw = wlan_operate_get_ht_bw(wdev);
		if (HT_BW_20 == bw && 1 == pAd->CommonCfg.HT_Disable) {
			CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): bw 20 is already set.\n",
				__func__, __LINE__));
			goto set_channel;
		}

		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		wlan_config_set_ht_bw(wdev, BW_20);
		if (WLAN_OPER_OK == wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE)) {
			pAd->CommonCfg.HT_Disable = 1;/*HT disable*/
			SetCommonHtVht(pAd, wdev);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! wlan_operate_set_ht_bw 20 FAIL\n");
			ret = FALSE;
		}
	} else if (MTK_NL80211_CHAN_WIDTH_20 == pchan_info->ChanType) {
		/*bw and ht check*/
		bw = wlan_operate_get_ht_bw(wdev);
		if (HT_BW_20 == bw && 0 == pAd->CommonCfg.HT_Disable) {
			CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): bw 20 is already set.\n",
				__func__, __LINE__));
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		wlan_config_set_ht_bw(wdev, BW_20);
		if (WLAN_OPER_OK == wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT enable*/
			SetCommonHtVht(pAd, wdev);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! wlan_operate_set_ht_bw 20 FAIL\n");
			ret = FALSE;
		}
	} else if (MTK_NL80211_CHAN_WIDTH_40 == pchan_info->ChanType) {
		/*extra_bw*/
		if (pchan_info->CenterChanId > pchan_info->ChanId)
			ext_cha = EXTCHA_ABOVE;
		else
			ext_cha = EXTCHA_BELOW;

		/*bw and ht check*/
		bw = wlan_operate_get_ht_bw(wdev);

		if (HT_BW_40 == bw && ext_cha == wlan_operate_get_ext_cha(wdev)
			&& 0 == pAd->CommonCfg.HT_Disable) {
			CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): bw 40 ext_bw(%s) is already set.\n",
				__func__, __LINE__,  EXTCHA_ABOVE == ext_cha ? "above":"below"));
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);

		/*set bw*/
		wlan_config_set_ht_bw(wdev, BW_40);
		if (WLAN_OPER_OK == wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT enable*/
			SetCommonHtVht(pAd, wdev);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! wlan_operate_set_ht_bw 40 FAIL\n");
			ret = FALSE;
		}
	} else if (MTK_NL80211_CHAN_WIDTH_80 == pchan_info->ChanType) {
		/*bw and ht check*/
		bw = wlan_operate_get_vht_bw(wdev);
		if (VHT_BW_80 == bw && 0 == pAd->CommonCfg.HT_Disable) {
			CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): bw 80 is already set.\n",
				__func__, __LINE__));
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		/*set bw*/
		if (WLAN_OPER_OK == wlan_operate_set_vht_bw(wdev, VHT_BW_80)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT disable*/
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! wlan_operate_set_ht_bw 80 FAIL\n");
			ret = FALSE;
		}

	} else if (MTK_NL80211_CHAN_WIDTH_80P80 == pchan_info->ChanType) {
		/*bw and vht check*/
		bw = wlan_operate_get_vht_bw(wdev);
		if (VHT_BW_8080 == bw && 0 == pAd->CommonCfg.HT_Disable) {
			CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): bw 8080 is already set.\n",
				__func__, __LINE__));
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		/*set bw*/
		if (WLAN_OPER_OK == wlan_operate_set_vht_bw(wdev, VHT_BW_8080)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT disable*/
			wlan_config_set_he_bw(wdev, HE_BW_8080);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! wlan_operate_set_ht_bw 8080 FAIL\n");
			ret = FALSE;
		}
	} else if (MTK_NL80211_CHAN_WIDTH_160 == pchan_info->ChanType) {
		/*bw and ht check*/
		bw = wlan_operate_get_vht_bw(wdev);
		if (VHT_BW_160 == bw && 0 == pAd->CommonCfg.HT_Disable) {
			CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): bw 160 is already set.\n",
				__func__, __LINE__));
			goto set_channel;
		}
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		/*set vbw*/
		if (WLAN_OPER_OK == wlan_operate_set_vht_bw(wdev, VHT_BW_160)) {
			pAd->CommonCfg.HT_Disable = 0;/*HT disable*/
			wlan_config_set_he_bw(wdev, HE_BW_160);
			ret = TRUE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! wlan_operate_set_ht_bw 160 FAIL\n");
			ret = FALSE;
		}
	} else {
		CFG80211DBG(DBG_LVL_WARN, ("[%s](%d):WARN, bw=%d not support\n",
				__func__, __LINE__, pchan_info->ChanType));
		CFG8021DRV_AP_PHYINIT(pAd, wdev, pchan_info->ChanId);
		ret = FALSE;
	}

set_channel:
	/*bw and channel is not same as input check*/
	CFG80211DBG(DBG_LVL_INFO, ("[%s](%d):ret:%d (FALSE:%d), wdev->channel:%d,pchan_info->ChanId: %d\n",
					__func__, __LINE__, ret, FALSE, wdev->channel, pchan_info->ChanId));

	/*set channel*/
	if (wdev->channel != pchan_info->ChanId) {
		if (FALSE == rtmp_set_channel(pAd, wdev, pchan_info->ChanId)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! channel(%d) set fail\n",
					pchan_info->ChanId);
			return FALSE;
		}
	} else {
		CFG80211DBG(DBG_LVL_INFO, ("[%s](%d): channel(%d) is already set!\n",
				__func__, __LINE__, pchan_info->ChanId));
	}
	return TRUE;

}


BOOLEAN CFG80211DRV_AP_OpsSetChannel(RTMP_ADAPTER *pAd, VOID *pData)
{
	CMD_RTPRIV_IOCTL_80211_CHAN *pchan_info = NULL;
	struct wifi_dev *pwdev = NULL;

	if (!pData || !pAd) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! invalid parametr: pAd = %p, pData=%p\n",
			pAd, pData);
		return FALSE;
	}

	pchan_info = (CMD_RTPRIV_IOCTL_80211_CHAN *)pData;
	/*get wifi_dev*/
	if (pchan_info->pWdev && pchan_info->pWdev->netdev)
	pwdev = RTMP_OS_NETDEV_GET_WDEV(pchan_info->pWdev->netdev);


	if (TRUE != CFG80211DRV_AP_SetChanBw_byWdev(pAd, pwdev, pchan_info)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR! CFG80211DRV_AP_SetChanBw_byWdev FAIL!\n");
		return FALSE;
	}

	return TRUE;
}
#endif/*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/
#endif/*KERNEL_VERSION(4, 0, 0)*/


BOOLEAN CFG80211DRV_OpsSetChannel(RTMP_ADAPTER *pAd, VOID *pData)
{
	CMD_RTPRIV_IOCTL_80211_CHAN *pChan;
	UINT8 ChanId, IfType, ChannelType;
#ifdef DOT11_N_SUPPORT
	BOOLEAN FlgIsChanged;
#endif /* DOT11_N_SUPPORT */

	UCHAR RfIC = 0;
	UCHAR newBW = BW_20;
	UCHAR ext_cha;
	CHANNEL_CTRL *pChCtrl;
	UCHAR BandIdx;
	struct wifi_dev *wdev = NULL;

#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

	wdev = &pMbss->wdev;
#else
	wdev = &pAd->StaCfg[0].wdev;
#endif

	/*
	 *  enum nl80211_channel_type {
	 *	NL80211_CHAN_NO_HT,
	 *	NL80211_CHAN_HT20,
	 *	NL80211_CHAN_HT40MINUS,
	 *	NL80211_CHAN_HT40PLUS
	 *  };
	 */
	/* init */
	pChan = (CMD_RTPRIV_IOCTL_80211_CHAN *)pData;
	ChanId = pChan->ChanId;
	IfType = pChan->IfType;
	ChannelType = pChan->ChanType;

#ifdef HOSTAPD_AUTO_CH_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"HOSTAPD AUTO_CH_SUPPORT Ignore Channel %d from HostAPD \n",
					pChan->ChanId);
	return TRUE;
#else
	/* set phymode by channel number */
	if (ChanId > 14) {
		wdev->PhyMode = (WMODE_A | WMODE_AN | WMODE_AC); /*5G phymode*/
		/* Change channel state to NONE */
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef EXT_BUILD_CHANNEL_LIST
		BuildChannelListEx(pAd, wdev);
#else
		BuildChannelList(pAd, wdev);
#endif
		RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
		RfIC = RFIC_5GHZ;
	} else {
		wdev->PhyMode = (WMODE_B | WMODE_G | WMODE_GN);  /*2G phymode*/
		/* Change channel state to NONE */
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
		BuildChannelList(pAd, wdev);
		RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
		RfIC = RFIC_24GHZ;
	}

	if (IfType != RT_CMD_80211_IFTYPE_MONITOR) {
		/* get channel BW */
		FlgIsChanged = TRUE;

		/* set to new channel BW */
		if (ChannelType == RT_CMD_80211_CHANTYPE_HT20) {
			newBW = BW_20;
			wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
			pAd->CommonCfg.HT_Disable = 0;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;
				wlan_operate_set_prim_ch(wdev, wdev->channel);
		} else if (ChannelType == RT_CMD_80211_CHANTYPE_HT40MINUS) {
			newBW = BW_40;
			wlan_operate_set_ht_bw(wdev, HT_BW_40, EXTCHA_BELOW);
			ext_cha = wlan_operate_get_ext_cha(wdev);
			pAd->CommonCfg.HT_Disable = 0;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;

				wlan_operate_set_prim_ch(wdev, wdev->channel);
		} else if	(ChannelType == RT_CMD_80211_CHANTYPE_HT40PLUS) {
			/* not support NL80211_CHAN_HT40MINUS or NL80211_CHAN_HT40PLUS */
			/* i.e. primary channel = 36, secondary channel must be 40 */
			newBW = BW_40;
			wlan_operate_set_ht_bw(wdev, HT_BW_40, EXTCHA_ABOVE);
			pAd->CommonCfg.HT_Disable = 0;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;

			wlan_operate_set_prim_ch(wdev, wdev->channel);
		} else if  (ChannelType == RT_CMD_80211_CHANTYPE_NOHT) {
			newBW = BW_20;
			wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
			ext_cha = wlan_operate_get_ext_cha(wdev);
			pAd->CommonCfg.HT_Disable = 1;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;

			wlan_operate_set_prim_ch(wdev, wdev->channel);
		} else if  (ChannelType == RT_CMD_80211_CHANTYPE_VHT80) {
			newBW = BW_80;

			if (pChan->CenterChanId > pChan->ChanId)
				ext_cha = EXTCHA_ABOVE;
			else
				ext_cha = EXTCHA_BELOW;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;

			wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
			wlan_operate_set_vht_bw(wdev, VHT_BW_80);
			wlan_operate_set_prim_ch(wdev, wdev->channel);
		}

		CFG80211DBG(DBG_LVL_INFO, ("80211> HT Disable = %d\n", pAd->CommonCfg.HT_Disable));
	} else {
		/* for monitor mode */
		FlgIsChanged = TRUE;
		pAd->CommonCfg.HT_Disable = 0;
		wlan_operate_set_ht_bw(wdev, HT_BW_40, wlan_operate_get_ext_cha(wdev));
	}

	ext_cha = wlan_operate_get_ext_cha(wdev);
	/* switch to the channel with Common Channel */
	wdev->channel = ChanId;
#ifdef CONFIG_STA_SUPPORT
	pAd->StaCfg[0].MlmeAux.Channel = ChanId;
#endif /*CONFIG_STA_SUPPORT*/
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "80211> CentralChannel = %d, New BW = %d with Ext[%d]\n",
								wlan_operate_get_cen_ch_1(wdev), newBW, ext_cha);
#ifdef CONFIG_AP_SUPPORT
	os_msec_delay(1000);
#ifndef RT_CFG80211_SUPPORT
	APStopByRf(pAd, RfIC);
#else
    APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
#endif
	os_msec_delay(1000);
#ifndef RT_CFG80211_SUPPORT
	APStartUpByRf(pAd, RfIC);
#else
    APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
#endif
#endif /* CONFIG_AP_SUPPORT */

	if (IfType == RT_CMD_80211_IFTYPE_AP ||
		IfType == RT_CMD_80211_IFTYPE_P2P_GO) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "80211> Set the channel in AP Mode\n");
		return TRUE;
	}

#ifdef CONFIG_STA_SUPPORT

	if ((IfType == RT_CMD_80211_IFTYPE_STATION) && (FlgIsChanged == TRUE)) {
		/*
		 *	1. Station mode;
		 *	2. New BW settings is 20MHz but current BW is not 20MHz;
		 *	3. New BW settings is 40MHz but current BW is 20MHz;
		 *
		 *	Re-connect to the AP due to BW 20/40 or HT/non-HT change.
		 */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "80211> Set the channel in STA Mode\n");
	}

	if (IfType == RT_CMD_80211_IFTYPE_ADHOC) {
		/* update IBSS beacon */
		MlmeUpdateTxRates(pAd, FALSE, 0);
		UpdateBeaconHandler(
			pAd,
			&pAd->StaCfg[0].wdev,
			BCN_UPDATE_IE_CHG);
		AsicEnableIbssSync(
			pAd,
			pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
			pAd->StaCfg[0].wdev.OmacIdx,
			OPMODE_ADHOC);
		Set_SSID_Proc(pAd, (RTMP_STRING *)pAd->StaCfg[0].Ssid);
	}

#endif /*CONFIG_STA_SUPPORT*/
	return TRUE;
#endif
}

BOOLEAN CFG80211DRV_OpsJoinIbss(
	VOID						*pAdOrg,
	VOID						*pData)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_IBSS *pIbssInfo;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;

	pIbssInfo = (CMD_RTPRIV_IOCTL_80211_IBSS *)pData;
	pAd->StaCfg[0].bAutoReconnect = TRUE;
	pAd->CommonCfg.BeaconPeriod[DBDC_BAND0] = pIbssInfo->BeaconInterval;

	if (pIbssInfo->privacy) {
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);
		SET_CIPHER_WEP(wdev->SecConfig.PairwiseCipher);
		SET_CIPHER_WEP(wdev->SecConfig.GroupCipher);
		SET_CIPHER_WEP(pAd->StaCfg[0].GroupCipher);
		SET_CIPHER_WEP(pAd->StaCfg[0].PairwiseCipher);
	}

	if (pIbssInfo->BeaconExtraIeLen > 0) {
		const UCHAR *ie = NULL;

		if (pCfg80211_ctrl->BeaconExtraIe != NULL) {
			os_free_mem(pCfg80211_ctrl->BeaconExtraIe);
			pCfg80211_ctrl->BeaconExtraIe = NULL;
		}

		os_alloc_mem(NULL, (UCHAR **)&pCfg80211_ctrl->BeaconExtraIe, pIbssInfo->BeaconExtraIeLen);

		if (pCfg80211_ctrl->BeaconExtraIe != NULL) {
			NdisCopyMemory(pCfg80211_ctrl->BeaconExtraIe, pIbssInfo->BeaconExtraIe, pIbssInfo->BeaconExtraIeLen);
			pCfg80211_ctrl->BeaconExtraIeLen = pIbssInfo->BeaconExtraIeLen;
		} else {
			pCfg80211_ctrl->BeaconExtraIeLen = 0;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CFG80211: MEM ALLOC ERROR\n");
		}

		ie = pCfg80211_ctrl->BeaconExtraIe;
		if (ie != NULL) {

			if ((ie[0] == WLAN_EID_VENDOR_SPECIFIC) &&
				(ie[1] >= 4) &&
				(ie[2] == 0x00) && (ie[3] == 0x50) && (ie[4] == 0xf2) && (ie[5] == 0x01)) {
				/* skip wpa_version [6][7] */
				if ((ie[8] == 0x00) && (ie[9] == 0x50) && (ie[10] == 0xf2) && (ie[11] == 0x04)) {
					SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
					SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
					SET_CIPHER_CCMP128(pAd->StaCfg[0].GroupCipher);
					SET_CIPHER_CCMP128(pAd->StaCfg[0].PairwiseCipher);
				} else {
					SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
					SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
					SET_CIPHER_TKIP(pAd->StaCfg[0].GroupCipher);
					SET_CIPHER_TKIP(pAd->StaCfg[0].PairwiseCipher);
				}

				SET_AKM_WPANONE(wdev->SecConfig.AKMMap);
				pAd->StaCfg[0].WpaState = SS_NOTUSE;
			}
		}
	}

	AsicEnableIbssSync(
		pAd,
		pAd->CommonCfg.BeaconPeriod[DBDC_BAND0],
		pAd->StaCfg[0].wdev.OmacIdx,
		OPMODE_ADHOC);
	Set_SSID_Proc(pAd, (RTMP_STRING *)pIbssInfo->Ssid);
#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

BOOLEAN CFG80211DRV_OpsLeave(VOID *pAdOrg, PNET_DEV pNetDev)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;

			/*check if net dev corresponding to Apcli entry */
	struct wifi_dev *wdev = NULL;
	PNET_DEV pStaDev = NULL;
	UCHAR BandIdx = 0;

	GET_WDEV_FROM_NET_DEV(wdev, pNetDev);
	if (wdev != NULL) {
		BandIdx = HcGetBandByWdev(wdev);

		pStaDev = pAd->StaCfg[BandIdx].wdev.if_dev;

		if (pStaDev == pNetDev) {
			pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
			sta_deauth_act(wdev);
		}

	}

#endif /* defined(CONFIG_STA_SUPPORT) */
	return TRUE;
}


BOOLEAN CFG80211DRV_StaGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_STA *pIbssInfo;

    HTTRANSMIT_SETTING *HtPhyMode = NULL;
	struct wifi_dev *wdev = NULL;
	RSSI_SAMPLE *RssiSample = NULL;
	RADIUS_ACCOUNT_ENTRY *pFoundEntry = NULL;

	pIbssInfo = (CMD_RTPRIV_IOCTL_80211_STA *)pData;
#ifdef CONFIG_AP_SUPPORT
	{
		MAC_TABLE_ENTRY *pEntry;
		ULONG DataRate = 0;
		UINT32 RSSI;

		pEntry = MacTableLookup(pAd, pIbssInfo->MAC);

		if (pEntry == NULL) {
		    UINT16 i;
		    BOOLEAN found = FALSE;
		    /*search MAC Address in Radius Table */
		    for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
			    RADIUS_ACCOUNT_ENTRY *pRadiusEntry = &pAd->radius_tbl[i];

			    if (MAC_ADDR_EQUAL(pRadiusEntry->Addr, pIbssInfo->MAC)) {
				    /*Found Radius Entry */
				    found = TRUE;
				    pFoundEntry = pRadiusEntry;
				    HtPhyMode = &pRadiusEntry->HTPhyMode;
				    wdev = pRadiusEntry->wdev;
				    RssiSample = &pRadiusEntry->RssiSample;

				    getRate(pRadiusEntry->HTPhyMode, &DataRate);
				    pIbssInfo->rx_bytes = pRadiusEntry->RxBytes;

				    /* fill tx_bytes count */
				    pIbssInfo->tx_bytes = pRadiusEntry->TxBytes;

				    /* fill rx_packets count */
				    pIbssInfo->rx_packets = pRadiusEntry->RxPackets.u.LowPart;

				    /* fill tx_packets count */
				    pIbssInfo->tx_packets = pRadiusEntry->TxPackets.u.LowPart;

				    /* fill inactive time */
				    pIbssInfo->InactiveTime = pRadiusEntry->NoDataIdleCount * 1000; /* unit: ms */
				    break;
			    }
		    }
		    if (!found)
			    return FALSE;
	    } 	else {

		HtPhyMode = &pEntry->HTPhyMode;
		    wdev = pEntry->wdev;
		    RssiSample = &pEntry->RssiSample;
		    getRate(pEntry->HTPhyMode, &DataRate);

		    pIbssInfo->rx_bytes = pEntry->RxBytes;

		    /* fill tx_bytes count */
		    pIbssInfo->tx_bytes = pEntry->TxBytes;

		    /* fill rx_packets count */
		    pIbssInfo->rx_packets = pEntry->RxPackets.u.LowPart;

		    /* fill tx_packets count */
		    pIbssInfo->tx_packets = pEntry->TxPackets.u.LowPart;

		    /* fill inactive time */
		    pIbssInfo->InactiveTime = pEntry->NoDataIdleCount * 1000; /* unit: ms */

	    }

		/* fill tx rate */
	if ((HtPhyMode->field.MODE == MODE_HTMIX) ||
		(HtPhyMode->field.MODE == MODE_HTGREENFIELD)) {
		if (HtPhyMode->field.BW)
			pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_BW_40;

		if (HtPhyMode->field.ShortGI)
			pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_SHORT_GI;

			pIbssInfo->TxRateMCS = HtPhyMode->field.MCS;
		} else {
			pIbssInfo->TxRateFlags = RT_CMD_80211_TXRATE_LEGACY;
			pIbssInfo->TxRateMCS = DataRate * 1000; /* unit: 100kbps */
		}

		/* fill signal */
		RSSI = RTMPAvgRssi(pAd, RssiSample);
		pIbssInfo->Signal = RSSI;
		/* fill tx count */
		if (pEntry != NULL) {
			pIbssInfo->TxPacketCnt = pEntry->OneSecTxNoRetryOkCount +
									pEntry->OneSecTxRetryOkCount +
									pEntry->OneSecTxFailCount;
		}
		/* fill inactive time */
		pIbssInfo->InactiveTime *= MLME_TASK_EXEC_MULTIPLE;
		pIbssInfo->InactiveTime /= 20;

		if (pFoundEntry)
			NdisZeroMemory(pFoundEntry, sizeof(RADIUS_ACCOUNT_ENTRY));
	}
#endif /* CONFIG_AP_SUPPORT */
#ifndef APCLI_CFG80211_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	{
		HTTRANSMIT_SETTING PhyInfo;
		ULONG DataRate = 0;
		UINT32 RSSI;
		struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;

		/* fill tx rate */
		if ((!WMODE_CAP_N(wdev->PhyMode)) ||
			(pAd->MacTab.Content[BSSID_WCID_TO_REMOVE].HTPhyMode.field.MODE <= MODE_OFDM))
			PhyInfo.word = wdev->HTPhyMode.word;
		else
			PhyInfo.word = pAd->MacTab.Content[BSSID_WCID_TO_REMOVE].HTPhyMode.word;

		getRate(PhyInfo, &DataRate);

		if ((PhyInfo.field.MODE == MODE_HTMIX) ||
			(PhyInfo.field.MODE == MODE_HTGREENFIELD)) {
			if (PhyInfo.field.BW)
				pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_BW_40;

			if (PhyInfo.field.ShortGI)
				pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_SHORT_GI;

			pIbssInfo->TxRateMCS = PhyInfo.field.MCS;
		}

#ifdef DOT11_VHT_AC
		else if (PhyInfo.field.MODE == MODE_VHT) {
			/* cfg80211's rato_info structure and rate_info_flags can't support 11ac well in old kernel, we use legacy way to describe the actually vht rate */
			pIbssInfo->TxRateFlags = RT_CMD_80211_TXRATE_LEGACY;
			pIbssInfo->TxRateMCS = (DataRate * 10) * ((PhyInfo.field.MCS >> 4) + 1); /* unit: 100kbps */
		}

#endif
		else {
			pIbssInfo->TxRateFlags = RT_CMD_80211_TXRATE_LEGACY;
			pIbssInfo->TxRateMCS = DataRate * 10; /* unit: 100kbps */
		}

		/* fill tx/rx packet count */
		pIbssInfo->tx_packets = pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart;
		pIbssInfo->tx_retries = pAd->WlanCounters[0].RetryCount.u.LowPart;
		pIbssInfo->tx_failed = pAd->WlanCounters[0].FailedCount.u.LowPart;
		pIbssInfo->rx_packets = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;
		/* fill signal */
		RSSI = RTMPAvgRssi(pAd, &pAd->StaCfg[0].RssiSample);
		pIbssInfo->Signal = RSSI;
	}
#endif /* CONFIG_STA_SUPPORT */
#endif /*APCLI_CFG80211_SUPPORT */
	return TRUE;
}

#ifdef WIFI_IAP_STA_DUMP_FEATURE
BOOLEAN CFG80211DRV_FILL_STAInfo(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	CMD_RTPRIV_IOCTL_80211_STA *pApStaInfo,
	BOOLEAN bReptCli)
{
	INT i = 0;
	ULONG DataRate = 0;
	ULONG DataRate_r = 0;
	ADD_HT_INFO_IE *addht;
	struct cfg80211_rate_info *prate_info = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct wifi_dev *wdev = NULL;
	BSS_STRUCT *mbss = NULL;
	STA_ADMIN_CONFIG *pstacfg = NULL;
	UINT rssi_idx = 0;
	INT Rssi_temp = 0;
	UINT32	rx_stream;
	RSSI_SAMPLE RssiSample;
	USHORT bss_capability;
	BOOLEAN bss_shorttime_en;
	UINT8 bss_dtim;
	UINT16 bss_bcn_interval;
	ULONG time_now = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	if (NULL == pEntry || NULL == pEntry->wdev || NULL == pApStaInfo || NULL == tr_ctl) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR! invalid parameter! pEntry = %p, wdev=%p, pApStaInfo=%p, tr_ctl = %p!\n",
			pEntry, pEntry->wdev, pApStaInfo, tr_ctl);
		return FALSE;
	}
#ifdef MAC_REPEATER_SUPPORT
	if (bReptCli == FALSE && IS_ENTRY_REPEATER(pEntry)) {
		/* only dump the apcli entry which not a RepeaterCli */
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		("[%s](%d): REPEATER not Support\n",
		__func__, __LINE__));
		return TRUE;
	}
#endif /* MAC_REPEATER_SUPPORT */

	/*init data*/
	i = pEntry->wcid;
	wdev = pEntry->wdev;
#ifdef CONFIG_STA_SUPPORT
	pstacfg = (STA_ADMIN_CONFIG *) GetStaCfgByWdev(pAd, wdev);
#endif /*CONFIG_STA_SUPPORT*/


	if (pEntry->Sst != SST_ASSOC) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"idx: %d, mac: "MACSTR" is disassociated\n",
			pEntry->wcid, MAC2STR(pEntry->Addr)));
		return FALSE;
	}

#ifdef MAC_REPEATER_SUPPORT
	if (bReptCli == FALSE) {
		/* only dump the apcli entry which not a RepeaterCli */
		if (IS_REPT_LINK_UP(pEntry->pReptCli)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(%d):idx: %d, mac: "MACSTR" REPT linkup\n",
			__LINE__, pEntry->wcid, MAC2STR(pEntry->Addr));
			return FALSE;
		}
	}
#endif /* MAC_REPEATER_SUPPORT */

	addht = wlan_operate_get_addht(pEntry->wdev);
	DataRate = 0;
	getRate(pEntry->HTPhyMode, &DataRate);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		UCHAR phy_mode, rate, bw, sgi, stbc;
		UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
		UCHAR nss;
		UCHAR nss_r;
		UINT32 RawData;
		UINT32 lastTxRate = pEntry->LastTxRate;
		UINT32 lastRxRate = pEntry->LastRxRate;

		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
		EXT_EVENT_PHY_STATE_RX_RATE rRxStatResult;
		HTTRANSMIT_SETTING LastTxRate;
		HTTRANSMIT_SETTING LastRxRate;

			MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
			LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
			LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
			LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
			LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
			LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

			if (LastTxRate.field.MODE >= MODE_VHT)
				LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
			else if (LastTxRate.field.MODE == MODE_OFDM)
				LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
			else
				LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

			lastTxRate = (UINT32)(LastTxRate.word);
			LastRxRate.word = (USHORT)lastRxRate;
			RawData = lastTxRate;
			phy_mode = (RawData >> 13) & 0x7;
			rate = RawData & 0x3F;
			bw = (RawData >> 7) & 0x3;
			sgi = rTxStatResult.rEntryTxRate.ShortGI;
			stbc = ((RawData >> 10) & 0x1);
			nss = rTxStatResult.rEntryTxRate.VhtNss;

			MtCmdPhyGetRxRate(pAd, CMD_PHY_STATE_CONTENTION_RX_PHYRATE, ucBand, pEntry->wcid, &rRxStatResult);
			LastRxRate.field.MODE = rRxStatResult.u1RxMode;
			LastRxRate.field.BW = rRxStatResult.u1BW;
			LastRxRate.field.ldpc = rRxStatResult.u1Coding;
			LastRxRate.field.ShortGI = rRxStatResult.u1Gi ? 1 : 0;
			LastRxRate.field.STBC = rRxStatResult.u1Stbc;

			if (LastRxRate.field.MODE >= MODE_VHT)
				LastRxRate.field.MCS = ((rRxStatResult.u1RxNsts & 0x3) << 4) + rRxStatResult.u1RxRate;
			else if (LastRxRate.field.MODE == MODE_OFDM)
				LastRxRate.field.MCS = getLegacyOFDMMCSIndex(rRxStatResult.u1RxRate) & 0x0000003F;
			else
				LastRxRate.field.MCS = rRxStatResult.u1RxRate;

			phy_mode_r = rRxStatResult.u1RxMode;
			rate_r = rRxStatResult.u1RxRate & 0x3F;
			bw_r = rRxStatResult.u1BW;
			sgi_r = rRxStatResult.u1Gi;
			stbc_r = rRxStatResult.u1Stbc;
			nss_r = rRxStatResult.u1RxNsts + 1;
/*TX MCS*/
#ifdef DOT11_VHT_AC
			if (phy_mode >= MODE_VHT) {
				rate = rate & 0xF;
			}
#endif /* DOT11_VHT_AC */


/*RX MCS*/
#ifdef DOT11_VHT_AC
			if (phy_mode_r >= MODE_VHT) {
				rate_r = rate_r & 0xF;
			} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
			if (phy_mode_r >= MODE_HTMIX) {
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%d):MODE_HTMIX(%d >= %d): MCS=%d\n",
				__LINE__, phy_mode_r, MODE_HTMIX, rate_r);
			} else
#endif
			if (phy_mode_r == MODE_OFDM) {
				if (rate_r == TMI_TX_RATE_OFDM_6M)
					LastRxRate.field.MCS = 0;
				else if (rate_r == TMI_TX_RATE_OFDM_9M)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_OFDM_12M)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_OFDM_18M)
					LastRxRate.field.MCS = 3;
				else if (rate_r == TMI_TX_RATE_OFDM_24M)
					LastRxRate.field.MCS = 4;
				else if (rate_r == TMI_TX_RATE_OFDM_36M)
					LastRxRate.field.MCS = 5;
				else if (rate_r == TMI_TX_RATE_OFDM_48M)
					LastRxRate.field.MCS = 6;
				else if (rate_r == TMI_TX_RATE_OFDM_54M)
					LastRxRate.field.MCS = 7;
				else
					LastRxRate.field.MCS = 0;

				rate_r = LastRxRate.field.MCS;
				pApStaInfo->rxrate.mcs = rate_r;/*rx mcs: ofdm*/
			} else if (phy_mode_r == MODE_CCK) {
				if (rate_r == TMI_TX_RATE_CCK_1M_LP)
					LastRxRate.field.MCS = 0;
				else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
					LastRxRate.field.MCS = 3;
				else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
					LastRxRate.field.MCS = 1;
				else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
					LastRxRate.field.MCS = 2;
				else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
					LastRxRate.field.MCS = 3;
				else
					LastRxRate.field.MCS = 0;

				rate_r = LastRxRate.field.MCS;
				pApStaInfo->rxrate.mcs = LastRxRate.field.MCS;/*rx_mcs:cck*/
			}

			/*tx_gi*/
			if (sgi)
				pApStaInfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
			/*rx_gi*/
			if (sgi_r)
				pApStaInfo->rxrate.flags |= RATE_INFO_FLAGS_SHORT_GI;

			if (phy_mode >= MODE_HE) {
				/*ax tx */
				get_rate_he((rate & 0xf), bw, nss, 0, &DataRate);
				if (sgi == 1)
					DataRate = (DataRate * 967) >> 10;
				if (sgi == 2)
					DataRate = (DataRate * 870) >> 10;

				get_rate_he((rate_r & 0xf), bw_r, nss_r, 0, &DataRate_r);
				if (sgi == 1)
					DataRate_r = (DataRate_r * 967) >> 10;
				if (sgi == 2)
					DataRate_r = (DataRate_r * 870) >> 10;
				/*tx rate infos*/
				prate_info = &pApStaInfo->txrate;
				/*linux4.4 not support wifi6, set as leagacy*/
				prate_info->flags = 0;
				prate_info->mcs = rate;
				prate_info->legacy = (UINT16)DataRate;
				prate_info->nss = nss;
				prate_info->bw = bw;

				/*rx rate infos*/
				prate_info = &pApStaInfo->rxrate;
				/*linux4.4 not support wifi6, set as leagacy*/
				prate_info->flags = 0;
				prate_info->mcs = rate_r;
				prate_info->legacy = (UINT16)DataRate_r;
				prate_info->nss = nss_r;
				prate_info->bw = bw_r;

			} else {
				getRate(LastTxRate, &DataRate);
				getRate(LastRxRate, &DataRate_r);
				/*tx rate infos*/
				prate_info = &pApStaInfo->txrate;
				if (phy_mode >= MODE_VHT) {
					prate_info->flags |= RATE_INFO_FLAGS_VHT_MCS;
				} else if (phy_mode >= MODE_HTMIX) {
					prate_info->flags |= RATE_INFO_FLAGS_MCS;
				} else {
					prate_info->flags = 0;/*other as legacy*/
				}
				prate_info->mcs = rate;
				prate_info->legacy = (UINT16)DataRate;
				prate_info->nss = nss;
				prate_info->bw = bw;

			/*rx rate infos*/
			prate_info = &pApStaInfo->rxrate;
			if (phy_mode_r >= MODE_VHT) {
				prate_info->flags |= RATE_INFO_FLAGS_VHT_MCS;
			} else if (phy_mode_r >= MODE_HTMIX) {
				prate_info->flags |= RATE_INFO_FLAGS_MCS;
			} else {
				/*other as legacy*/
				prate_info->flags = 0;
			}
			prate_info->mcs = rate_r;
			prate_info->legacy = (UINT16)DataRate_r;
			prate_info->nss = nss_r;
			prate_info->bw = bw_r;
		}
		/*tx rate infos*/
		prate_info = &pApStaInfo->txrate;
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\n(%d):tx_rate: rate=%u, mcs=%u, bw=%u, nss=%u\n",
			__LINE__, prate_info->legacy,
			prate_info->mcs, prate_info->bw, prate_info->nss);
		/*rx rate infos*/
		prate_info = &pApStaInfo->rxrate;
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\n(%d):rx_rate: rate=%u, mcs=%u, bw=%u, nss=%u\n",
			__LINE__, prate_info->legacy,
			prate_info->mcs, prate_info->bw, prate_info->nss);

	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		/*apcli entry infos*/
		if (IS_ENTRY_PEER_AP(pEntry) && pstacfg && pstacfg->ApcliInfStat.Valid) {

			/*Signal_avg*/
			pApStaInfo->signal_avg = RTMPAvgRssi(pAd, &pstacfg->RssiSample);
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\n(%d):pApStaInfo->signal_avg:%d\n",
					__LINE__, pApStaInfo->signal_avg);

			/*Signal*/
			rx_stream = pAd->Antenna.field.RxPath;
			RssiSample = pstacfg->RssiSample;
			for (rssi_idx = 0; rssi_idx < rx_stream; rssi_idx++) {
				Rssi_temp += RssiSample.LastRssi[rssi_idx];
			}
			pApStaInfo->Signal = (CHAR)(Rssi_temp/rx_stream);
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\n(%d):Rssi_temp:%d, rx_stream:%d, pApStaInfo->Signal:%d\n",
					__LINE__, Rssi_temp, rssi_idx, pApStaInfo->Signal);
			/*InactiveTime*/
			pApStaInfo->InactiveTime = pEntry->NoDataIdleCount * 1000; /* unit: ms */

			/*connectedTime*/
			NdisGetSystemUpTime(&time_now);
			time_now -= pstacfg->ApcliInfStat.ApCliLinkUpTime;
			pApStaInfo->connected_time = (UINT32)(time_now/OS_HZ);/*sec*/
			/*tx_packets*/
			pApStaInfo->tx_packets = (UINT32)pstacfg->StaStatistic.TxCount;
			/*tx_bytes*/
			pApStaInfo->tx_bytes = (UINT32)pstacfg->StaStatistic.TransmittedByteCount;
			#ifdef TXRX_STAT_SUPPORT
			/*tx_failed*/
			pApStaInfo->tx_failed = (UINT32)pEntry->TxFailCountByWtbl;
			#endif/*TXRX_STAT_SUPPORT*/
			/*rx_packets*/
			pApStaInfo->rx_packets = (UINT32)pstacfg->StaStatistic.RxCount;
			/*rx_bytes*/
			pApStaInfo->rx_bytes = (UINT32)pstacfg->StaStatistic.ReceivedByteCount;
			/*tx_retries*/
			#ifdef EAP_STATS_SUPPORT
			pApStaInfo->tx_retries = (UINT32)pEntry->mpdu_retries.QuadPart;
			#endif
			#ifdef WIFI_IAP_BCN_STAT_FEATURE
			/*add beacon infos here*/
			pApStaInfo->beacon_mask |= BIT(NL80211_STA_INFO_BEACON_LOSS);
			pApStaInfo->beacon_loss_count = pstacfg->beacon_loss_count;
			pApStaInfo->beacon_mask |= BIT(NL80211_STA_INFO_BEACON_RX);
			pApStaInfo->rx_beacon = pstacfg->rx_beacon;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\n(%d):beacon_loss_count  = %u, rx_beacon = %lu;\n",
				__LINE__, pstacfg->beacon_loss_count, pstacfg->rx_beacon);
			#endif/*WIFI_IAP_BCN_STAT_FEATURE*/
		}  else {

			/*Signal_avg*/
			pApStaInfo->signal_avg = RTMPAvgRssi(pAd, &pEntry->RssiSample);
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\n(%d):pApStaInfo->signal_avg:%d\n",
					__LINE__, pApStaInfo->signal_avg);

			/*Signal*/
			rx_stream = pAd->Antenna.field.RxPath;
			RssiSample = pEntry->RssiSample;
			for (rssi_idx = 0; rssi_idx < rx_stream; rssi_idx++) {
				Rssi_temp += RssiSample.LastRssi[rssi_idx];
			}
			pApStaInfo->Signal = (CHAR)(Rssi_temp/rx_stream);
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\n(%d):Rssi_temp:%d, rx_stream:%d, pApStaInfo->Signal:%d\n",
					__LINE__, Rssi_temp, rssi_idx, pApStaInfo->Signal);
			/*InactiveTime*/
			pApStaInfo->InactiveTime = pEntry->NoDataIdleCount * 1000; /* unit: ms */
			/*connectedtime*/
			pApStaInfo->connected_time = pEntry->StaConnectTime;/*sec*/
		#ifdef TXRX_STAT_SUPPORT
			/*tx_packets*/
			pApStaInfo->tx_packets = (UINT32)pEntry->TxDataPacketCount.QuadPart;
			/*tx_bytes*/
			pApStaInfo->tx_bytes = (UINT32)pEntry->TxDataPacketByte.QuadPart;
			/*tx_failed*/
			pApStaInfo->tx_failed = (UINT32)pEntry->TxFailCountByWtbl;
			/*rx_packets*/
			pApStaInfo->rx_packets = (UINT32)pEntry->RxDataPacketCount.QuadPart;
			/*rx_bytes*/
			pApStaInfo->rx_bytes = (UINT32)pEntry->RxDataPacketByte.QuadPart;
		#endif
		#ifdef EAP_STATS_SUPPORT
			/*tx_retries*/
			pApStaInfo->tx_retries = (UINT32)pEntry->mpdu_retries.QuadPart;
		#endif
}

			/*sta_flages*/
			pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_WME);/*wmm*/
			if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
				pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_WME);
			} else {
				pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_WME));
			}

			pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_MFP);/*pmf*/
#ifdef DOT11W_PMF_SUPPORT
			if (pEntry->SecConfig.PmfCfg.UsePMFConnect) {
				pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_MFP);
			} else
#endif
			{
				pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_MFP));
			}

			pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);/*sta preamble*/
			if (CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo)) {
				pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_SHORT_PREAMBLE);
			} else {
				pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_SHORT_PREAMBLE));
			}

	pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_AUTHENTICATED);/*AUTHENTICATED*/
	pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_ASSOCIATED);/*assoc*/
	if (pEntry->Sst == SST_ASSOC) {
		pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_AUTHENTICATED);
		pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_ASSOCIATED);
	} else {
		pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_ASSOCIATED));
		if (pEntry->Sst == SST_AUTH) {
			pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_AUTHENTICATED);
		} else {
			pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_AUTHENTICATED));
		}
	}

	/*802.x authorized*/
	pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_AUTHORIZED);
	if (pEntry->wcid < MAX_LEN_OF_TR_TABLE) {
		tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
		if (tr_entry && tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
			pApStaInfo->sta_flags.set  |= BIT(NL80211_STA_FLAG_AUTHORIZED);
		} else {
			pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_AUTHORIZED));
		}

	}

	/*TDLS */
	pApStaInfo->sta_flags.mask |= BIT(NL80211_STA_FLAG_TDLS_PEER);
	if (IS_ENTRY_TDLS(pEntry)) {
		pApStaInfo->sta_flags.set |= BIT(NL80211_STA_FLAG_TDLS_PEER);
	} else {
		pApStaInfo->sta_flags.set &= ~(BIT(NL80211_STA_FLAG_TDLS_PEER));
	}

/*apcli or mbss infos fill*/

	/*fill bss from apcli*/
	if (IS_ENTRY_PEER_AP(pEntry) && pstacfg && pstacfg->ApcliInfStat.Valid)  {
		bss_capability = pstacfg->StaActive.CapabilityInfo;
		bss_shorttime_en  = OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
		bss_dtim = pstacfg->DtimPeriod;
		bss_bcn_interval = pstacfg->BeaconPeriod;
	} else {
		for (i = 0; i < MAX_BEACON_NUM ; i++) {
			if (wdev == &pAd->ApCfg.MBSSID[i].wdev) {
				mbss = &pAd->ApCfg.MBSSID[i];
				bss_capability = mbss->CapabilityInfo;
				bss_shorttime_en  = wdev->bUseShortSlotTime;
				bss_dtim = mbss->DtimPeriod;
				bss_bcn_interval = pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)];
				break;
			}
		}

		if (i >= MAX_BEACON_NUM) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("\n[%s](%d):ERROR ! CAN'T FIND BSS INFOS\n",
			__func__, __LINE__));
			return TRUE;
		}
	}

	/*linux4.4 only check ht protection */
	if (wlan_config_get_ht_protect_en(wdev)) {
		pApStaInfo->bss_param.flags |= BSS_PARAM_FLAGS_CTS_PROT;
	} else {
		pApStaInfo->bss_param.flags &= ~BSS_PARAM_FLAGS_CTS_PROT;
	}
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\n(%d):BSS_PARAM_FLAGS_CTS_PROT(%d)\n",
			__LINE__, wlan_config_get_ht_protect_en(wdev));
	/*short preamble*/
	if (CAP_IS_SHORT_PREAMBLE_ON(bss_capability)) {
		pApStaInfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_PREAMBLE;
	} else {
		pApStaInfo->bss_param.flags &= ~BSS_PARAM_FLAGS_SHORT_PREAMBLE;
	}
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\n(%d):BSS_PARAM_FLAGS_SHORT_PREAMBLE(%d)\n",
			__LINE__, CAP_IS_SHORT_PREAMBLE_ON(bss_capability));

	/*slot time*/
	if (bss_shorttime_en) {
		pApStaInfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
	} else {
		pApStaInfo->bss_param.flags &= ~BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
	}
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\n(%d):BSS_PARAM_FLAGS_SHORT_SLOT_TIME(%d)\n",
			__LINE__, bss_shorttime_en);

	/*dtim and beancon_interval value */
	if (bss_dtim > 0) {
		pApStaInfo->bss_param.dtim_period = bss_dtim;/*dtim*/
	} else {
		pApStaInfo->bss_param.dtim_period = 0;
	}
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\n(%d):NL80211_STA_BSS_PARAM_DTIM_PERIOD(%d)\n",
			__LINE__, pApStaInfo->bss_param.dtim_period);

	/*beacon_interval*/
	if (bss_bcn_interval > 0) {
		/* dbdc*/
		pApStaInfo->bss_param.beacon_interval = bss_bcn_interval;
	} else {
		pApStaInfo->bss_param.beacon_interval = 0;
	}
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\n(%d):NL80211_STA_BSS_PARAM_BEACON_INTERVAL:%d\n",
					__LINE__, pApStaInfo->bss_param.beacon_interval);
	return TRUE;
}



BOOLEAN CFG80211DRV_Ap_StaGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_STA *pApStaInfo = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	pApStaInfo = (CMD_RTPRIV_IOCTL_80211_STA *)pData;

	pEntry = MacTableLookup(pAd, pApStaInfo->MAC);

	if (FALSE == CFG80211DRV_FILL_STAInfo(pAd, pEntry, pApStaInfo, FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"ERROR! CFG80211DRV_FILL_STAInfo Fail.\n");
		return FALSE;
	}
	return TRUE;
}

#endif/*WIFI_IAP_STA_DUMP_FEATURE*/


#ifdef WIFI_IAP_POWER_SAVE_FEATURE
#ifdef APCLI_SUPPORT
static INT CFG80211DRV_AP_SetLegacyPS(
	PRTMP_ADAPTER pAd,
	STA_ADMIN_CONFIG *pstacfg,
	UINT enable)
{
	if (NULL == pstacfg || NULL == pAd) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"ERROR! pstacfg = %p or pAd=%p\n",
				pstacfg, pAd);
		return FALSE;
	}


	/*on*/
	if (enable && BSS_INFRA == pstacfg->BssType) {
#ifdef LEGACY_PSMODE
	/* do NOT turn on PSM bit here, wait until MlmeCheckPsmChange() */
		/* to exclude certain situations. */
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

		if (pstacfg->bWindowsACCAMEnable == FALSE) {
			if (pstacfg->WindowsPowerMode == Ndis802_11PowerModeCAM &&
				1 == pstacfg->ApcliInfStat.Valid)
				RTMP_SLEEP_FORCE_AUTO_WAKEUP(pAd, pstacfg);

			pstacfg->WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;
		}

		pstacfg->WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
		pstacfg->DefaultListenCount = 3;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		pstacfg->DefaultListenCount = 1;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) // */
#else /*LEGACY_PSMODE*/
#ifdef FAST_PSMODE
	/* do NOT turn on PSM bit here, wait until MlmeCheckPsmChange() */
		/* to exclude certain situations. */
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

		if (pstacfg->bWindowsACCAMEnable == FALSE)
			pstacfg->WindowsPowerMode = Ndis802_11PowerModeFast_PSP;

		pstacfg->WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
		pstacfg->DefaultListenCount = 3;
#endif/*FAST_PSMODE*/
#endif/*NO LEGACY_PSMODE*/
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(%d):Leagacy_PSMODE: enable(%d)\n",
			__LINE__, enable);
	}
	/*off*/
	else if (!enable && BSS_INFRA == pstacfg->BssType) {
		/*Default Ndis802_11PowerModeCAM */
			/* clear PSM bit immediately */
			RTMP_SET_PSM_BIT(pAd, pstacfg, PWR_ACTIVE);
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pstacfg->bWindowsACCAMEnable == FALSE) {
				if (pstacfg->WindowsPowerMode != Ndis802_11PowerModeCAM &&
					1 == pstacfg->ApcliInfStat.Valid) {
					RTMP_FORCE_WAKEUP(pAd, pstacfg);
					pstacfg->WindowsPowerMode = Ndis802_11PowerModeCAM;
				}
			}

			pstacfg->WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(%d):WARN! invalid parameter pstacfg->BssType=%d;\n",
			__LINE__, pstacfg->BssType);
	}

	return TRUE;
}
#endif/*APCLI_SUPPORT*/

#ifdef UAPSD_SUPPORT
static INT CFG80211DRV_AP_SetUAPSD(
	PRTMP_ADAPTER pAd,
	STA_ADMIN_CONFIG *pstacfg,
	struct wifi_dev *wdev,
	UINT enable)
{

	if (NULL == wdev || NULL == pAd) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"ERROR! wdev = %p or pAd=%p\n",
				wdev, pAd);
		return FALSE;
	}

	if (enable) {
		/*wmm must close txburst*/
		if (wdev->bWmmCapable && TRUE == pAd->CommonCfg.bEnableTxBurst)
			pAd->CommonCfg.bEnableTxBurst = FALSE;

		/*ac uapsd on  for apcli*/
		if (pstacfg && (FALSE == pAd->CommonCfg.bAPSDAC_BE ||
			FALSE == pAd->CommonCfg.bAPSDAC_BK ||
			FALSE == pAd->CommonCfg.bAPSDAC_VI ||
			FALSE == pAd->CommonCfg.bAPSDAC_VO)) {
			pAd->CommonCfg.bAPSDAC_BE = TRUE;
			pAd->CommonCfg.bAPSDAC_BK = TRUE;
			pAd->CommonCfg.bAPSDAC_VI = TRUE;
			pAd->CommonCfg.bAPSDAC_VO = TRUE;
		}
		wdev->UapsdInfo.bAPSDCapable = TRUE;/*on*/
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(%d):UAPSD: enable(%d)\n",
			__LINE__, enable);
	} else {
		/*wmm  close txburst enable*/
		if (!wdev->bWmmCapable && TRUE == pAd->CommonCfg.bEnableTxBurst)
			pAd->CommonCfg.bEnableTxBurst = FALSE;
		/*ac uapsd off for apcli/sta*/
		if (pstacfg && (TRUE == pAd->CommonCfg.bAPSDAC_BE ||
			TRUE == pAd->CommonCfg.bAPSDAC_BK ||
			TRUE == pAd->CommonCfg.bAPSDAC_VI ||
			TRUE == pAd->CommonCfg.bAPSDAC_VO)) {
			pAd->CommonCfg.bAPSDAC_BE = FALSE;
			pAd->CommonCfg.bAPSDAC_BK = FALSE;
			pAd->CommonCfg.bAPSDAC_VI = FALSE;
			pAd->CommonCfg.bAPSDAC_VO = FALSE;
		}

		wdev->UapsdInfo.bAPSDCapable = FALSE;/*off*/
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("[%s](%d):UAPSD: disable(%d)\n",
			__func__, __LINE__, enable));
	}

	return TRUE;
}
#endif/*UAPSD_SUPPORT*/


static INT CFG80211DRV_AP_SetPSPM(struct wifi_dev *wdev, UINT enable)
{
	HT_CAPABILITY_IE *ht_cap = NULL;

	if (NULL == wdev || !wlan_operate_get_ht_cap(wdev)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"ERROR! wdev = %p\n",
				wdev);
		return FALSE;
	}
	ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);

	if (enable) {
		if (1 != ht_cap->HtCapInfo.PSMP) {
			ht_cap->HtCapInfo.PSMP = 1;/*on*/
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(%d):PSPM: enable(%d)\n",
				 __LINE__, enable);
		}
	} else {
		if (0 != ht_cap->HtCapInfo.PSMP) {
			ht_cap->HtCapInfo.PSMP = 0;/*off*/
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(%d):PSPM: disable(%d)\n",
				__LINE__, enable);
		}
	}
	return TRUE;
}

INT CFG80211DRV_AP_Set_PSMODE (
	PRTMP_ADAPTER pAd,
	STA_ADMIN_CONFIG *pstacfg,
	struct wifi_dev *wdev,
	UINT enable)
{

	/*LEAGACY_PS*/
	if (NULL != pstacfg && NULL != pAd  &&
		TRUE != CFG80211DRV_AP_SetLegacyPS(pAd, pstacfg, enable)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("[%s](%d):ERROR! CFG80211DRV_AP_SetLeagacyPS FAIL\n",
			__func__, __LINE__));
	}

#ifdef UAPSD_SUPPORT
	/*UAPSD*/
	if (TRUE != CFG80211DRV_AP_SetUAPSD(pAd, pstacfg, wdev, enable)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("[%s](%d):ERROR! CFG80211DRV_AP_SetUAPSD FAIL\n",
			__func__, __LINE__));
	}
#endif/*UAPSD*/

	/*PSPM */
	if (TRUE != CFG80211DRV_AP_SetPSPM(wdev, enable)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("[%s](%d):ERROR! CFG80211DRV_AP_SetPSPM FAIL\n",
			__func__, __LINE__));
	}

	return TRUE;
}


INT CFG80211DRV_AP_SetPowerMgmt (
	VOID	*pAdOrg,
	VOID	*infwdev,
	UINT enable)
{
	PRTMP_ADAPTER pAd = NULL;
	STA_ADMIN_CONFIG *pstacfg = NULL;
	struct wifi_dev *wdev = NULL;

	pAd = (PRTMP_ADAPTER)pAdOrg;
	wdev = (struct wifi_dev *) infwdev;
	if (!pAd || !wdev) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"ERROR! pAd = %p, wdev=%p\n",
				pAd, wdev);
		return FALSE;
	}

	/*set ap interface ps_mode*/
	if (WDEV_TYPE_AP == wdev->wdev_type)  {
		pstacfg = NULL;
		CFG80211DRV_AP_Set_PSMODE(pAd, pstacfg, wdev, enable);
	}
#ifdef CONFIG_STA_SUPPORT
#ifdef APCLI_SUPPORT
	/*set apcli interface ps_mode*/
	else if (WDEV_TYPE_STA == wdev->wdev_type) {
		pstacfg = GetStaCfgByWdev(pAd, wdev);
		if (pstacfg) {
			CFG80211DRV_AP_Set_PSMODE(pAd, pstacfg, wdev, enable);
		} else {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"ERROR! pstacfg = %p\n",
				pstacfg);
			return FALSE;
		}
	}
#endif/*CONFIG_STA_SUPPORT*/
#endif/*APCLI_SUPPORT*/
	else {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"ERROR! wdev->wdev_type = %d not suuport.\n",
				wdev->wdev_type);
		return FALSE;
	}

	return TRUE;
}
#endif/*WIFI_IAP_POWER_SAVE_FEATURE*/


BOOLEAN CFG80211DRV_StaKeyAdd(
	VOID						*pAdOrg,
	VOID						*pData)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	PSTA_ADMIN_CONFIG pApCliEntry = pApCliEntry = &pAd->StaCfg[MAIN_MBSSID];
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
	struct wifi_dev *p2p_wdev = NULL;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;
#ifdef CFG_TDLS_SUPPORT
	/* CFG TDLS */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> TDLS IneedKey = %d\n", pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IneedKey);

	if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IneedKey == 1) {
		os_move_mem(&(pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TPK[16]), pKeyInfo->KeyBuf, LEN_TK);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> TDLS_COPY_KEY\n");
		hex_dump("pKeyInfo=", (UINT8 *)pKeyInfo->KeyBuf, LEN_TK);
		hex_dump("CFG_Tdls_info.TPK=", (UINT8 *) &(pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TPK[16]), LEN_TK);
		return TRUE;  /* to avoid setting key into wrong WCID, overwrite AP WCID 1 */
	}

#endif /* CFG_TDLS_SUPPORT */

	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40 || pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP104) {
		RT_CMD_STA_IOCTL_SECURITY IoctlSec;
		MAC_TABLE_ENTRY *pEntry = NULL;
		INT groupWcid = 0;

		if (ADHOC_ON(pAd))
			groupWcid = pAd->StaCfg[0].wdev.tr_tb_idx;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_CMD_80211_KEY_WEP\n");
		pEntry = &pAd->MacTab.Content[BSSID_WCID_TO_REMOVE];
		IoctlSec.KeyIdx = pKeyInfo->KeyId;
		IoctlSec.pData = pKeyInfo->KeyBuf;
		IoctlSec.length = pKeyInfo->KeyLen;
		IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_WEP;
		IoctlSec.flags = RT_CMD_STA_IOCTL_SECURITY_ENABLED;
		RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT, 0,
							 &IoctlSec, 0, INT_MAIN);
#ifdef MT_MAC
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)

		if (pKeyInfo->bPairwise == FALSE)
#else
		if (pKeyInfo->KeyId > 0)
#endif /* LINU_VERSION_CODE: 2.6.37 */
		{
			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				struct _ASIC_SEC_INFO *info = NULL;

				if (ADHOC_ON(pAd)) {
					UINT i = 0;

					for (i = BSSID_WCID_TO_REMOVE; i < groupWcid/*MAX_LEN_OF_MAC_TABLE*/; i++) {
						pEntry = &pAd->MacTab.Content[i];

						if (pEntry->wcid == 0)
							continue;

						if (IS_ENTRY_ADHOC(pEntry)) {
							/* Set key material to Asic */
							os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
							os_zero_mem(info, sizeof(ASIC_SEC_INFO));
							info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
							info->Direction = SEC_ASIC_KEY_BOTH;
							info->Wcid = pEntry->wcid;
							info->BssIndex = BSS0;
							info->Cipher = pEntry->SecConfig.PairwiseCipher;
							info->KeyIdx = pKeyInfo->KeyId;
							os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
							os_move_mem(&info->Key, &pEntry->SecConfig.WepKey[info->KeyIdx], sizeof(SEC_KEY_INFO));
							HW_ADDREMOVE_KEYTABLE(pAd, info);
							os_free_mem(info);
						} else
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
									 "=====> can't add to [%d]Wcid %d, type=%d\n", i,
									  pEntry->wcid, pEntry->EntryType);
					}
				}

				/* Set key material to Asic */
				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_GROUP_KEY;
				info->Direction = SEC_ASIC_KEY_RX;
				info->Wcid = groupWcid;
				info->BssIndex = BSS0;
				info->Cipher = pEntry->SecConfig.GroupCipher;
				info->KeyIdx = pKeyInfo->KeyId;
				os_move_mem(&info->PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
				os_move_mem(&info->Key, &pEntry->SecConfig.WepKey[info->KeyIdx], sizeof(SEC_KEY_INFO));
				HW_ADDREMOVE_KEYTABLE(pAd, info);
				os_free_mem(info);
			}
		} else {
			struct _ASIC_SEC_INFO *info = NULL;
			/* Set key material to Asic */
			os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
			os_zero_mem(info, sizeof(ASIC_SEC_INFO));
			info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
			info->Direction = SEC_ASIC_KEY_BOTH;
			info->Wcid = pEntry->wcid;
			info->BssIndex = BSS0;
			info->Cipher = pEntry->SecConfig.PairwiseCipher;
			info->KeyIdx = pKeyInfo->KeyId;
			os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
			os_move_mem(&info->Key, &pEntry->SecConfig.WepKey[info->KeyIdx], sizeof(SEC_KEY_INFO));
			HW_ADDREMOVE_KEYTABLE(pAd, info);
			os_free_mem(info);
		}

#endif /* MT_MAC */
	} else {
		RT_CMD_STA_IOCTL_SECURITY IoctlSec;
		IoctlSec.Alg = 0;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set_WPAPSK_Proc ==> id:%d, type:%d, len:%d\n",
				 pKeyInfo->KeyId, pKeyInfo->KeyType, (INT32)strlen(pKeyInfo->KeyBuf));
		IoctlSec.KeyIdx = pKeyInfo->KeyId;
		IoctlSec.pData = pKeyInfo->KeyBuf;
		IoctlSec.length = pKeyInfo->KeyLen;

		/* YF@20120327: Due to WepStatus will be set in the cfg connect function.*/
		if (IS_CIPHER_TKIP_Entry(&pAd->StaCfg[0].wdev))
			IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
		else if (IS_CIPHER_AES_Entry(&pAd->StaCfg[0].wdev))
			IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;

		IoctlSec.flags = RT_CMD_STA_IOCTL_SECURITY_ENABLED;
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)

		if (pKeyInfo->bPairwise == FALSE)
#else
		if (pKeyInfo->KeyId > 0)
#endif
		{
			if (IS_CIPHER_TKIP(pAd->StaCfg[0].GroupCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
			else if (IS_CIPHER_CCMP128(pAd->StaCfg[0].GroupCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Install GTK: %d\n", IoctlSec.Alg);
			IoctlSec.ext_flags = RT_CMD_STA_IOCTL_SECURTIY_EXT_GROUP_KEY;
		} else {
			if (IS_CIPHER_TKIP(pAd->StaCfg[0].PairwiseCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
			else if (IS_CIPHER_CCMP128(pAd->StaCfg[0].PairwiseCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Install PTK: %d\n", IoctlSec.Alg);
			IoctlSec.ext_flags = RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY;
		}

		/*Set_GroupKey_Proc(pAd, &IoctlSec) */
		RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT, 0,
							 &IoctlSec, 0, INT_MAIN);

		if (IS_AKM_WPANONE(pAd->StaCfg[0].wdev.SecConfig.AKMMap)) {
			if (IS_CIPHER_TKIP(pAd->StaCfg[0].PairwiseCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
			else if (IS_CIPHER_CCMP128(pAd->StaCfg[0].PairwiseCipher))
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Install ADHOC PTK: %d\n", IoctlSec.Alg);
			IoctlSec.ext_flags = RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY;
			RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT, 0,
								 &IoctlSec, 0, INT_MAIN);
		}

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

		if (IoctlSec.ext_flags == RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY) { /* only ptk to avoid group key rekey cases */
			if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd)) {
				UCHAR op_ht_bw1 = wlan_operate_get_ht_bw(wdev);
				UCHAR op_ht_bw2;

				p2p_wdev = &pMbss->wdev;
				op_ht_bw2 = wlan_operate_get_ht_bw(p2p_wdev);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"p2p_wdev->channel %d Channel %d\n", p2p_wdev->channel, wdev->channel);

				if ((op_ht_bw1 != op_ht_bw2) && ((wdev->channel == p2p_wdev->channel))) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "start bw !=  && P2P GO SCC\n");
					pAd->Mlme.bStartScc = TRUE;
				} else if ((((op_ht_bw1 == op_ht_bw2) && (wdev->channel != p2p_wdev->channel))
							|| !((op_ht_bw1 == op_ht_bw2) && ((wdev->channel == p2p_wdev->channel))))) {
					LONG timeDiff;
					INT starttime = pAd->Mlme.channel_1st_staytime;

					NdisGetSystemUpTime(&pAd->Mlme.BeaconNow32);
					timeDiff = (pAd->Mlme.BeaconNow32 - pAd->StaCfg[0].LastBeaconRxTime) % (pAd->CommonCfg.BeaconPeriod);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "#####pAd->Mlme.Now32 %d pAd->StaCfg[0].LastBeaconRxTime %d\n", pAd->Mlme.BeaconNow32, pAd->StaCfg[0].LastBeaconRxTime);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "####    timeDiff %d\n", timeDiff);
					AsicDisableSync(pAd);

					if (starttime > timeDiff)
						OS_WAIT((starttime - timeDiff));
					else
						OS_WAIT((starttime + (pAd->CommonCfg.BeaconPeriod - timeDiff)));

					AsicEnableApBssSync(pAd, pAd->CommonCfg.BeaconPeriod);
					Start_MCC(pAd);
					/* pAd->MCC_DHCP_Protect = TRUE; */
				}
			} else	if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd)) { /* check GC is key done , then trigger MCC */
				UCHAR op_ht_bw1 = wlan_operate_get_ht_bw(wdev);
				UCHAR op_ht_bw2;

				pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
				p2p_wdev = &(pApCliEntry->wdev);
				op_ht_bw2 = wlan_operate_get_ht_bw(p2p_wdev);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"p2p_wdev->channel %d Channel %d\n", p2p_wdev->channel, wdev->channel);

				if (pMacEntry) {
					if (IS_ENTRY_PEER_AP(pMacEntry) && (pMacEntry->PairwiseKey.KeyLen == LEN_TK)) { /* P2P GC will have security */
						if ((op_ht_bw1 != op_ht_bw2) && ((wdev->channel == p2p_wdev->channel))) {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "start bw !=  && P2P GC SCC\n");
							pAd->Mlme.bStartScc = TRUE;
						} else if ((((op_ht_bw1 == op_ht_bw2) && (wdev->channel != p2p_wdev->channel))
									|| !((op_ht_bw1 == op_ht_bw2) && ((wdev->channel == p2p_wdev->channel))))) {
							Start_MCC(pAd);
							/* pAd->MCC_DHCP_Protect = TRUE; */
						}
					}
				}
			}
		}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	}

#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

BOOLEAN CFG80211DRV_Connect(
	VOID						*pAdOrg,
	VOID						*pData)
{
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_CONNECT *pConnInfo;
	UCHAR SSID[NDIS_802_11_LENGTH_SSID + 1]; /* Add One for SSID_Len == 32 */
	UINT32 SSIDLen;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, &(pAd->StaCfg[0].wdev));
#ifdef APCLI_CFG80211_SUPPORT
	INT staidx;
#else
	RT_CMD_STA_IOCTL_SECURITY_ADV IoctlWpa;
#endif
	pConnInfo = (CMD_RTPRIV_IOCTL_80211_CONNECT *)pData;
	SSIDLen = pConnInfo->SsidLen;
	if (SSIDLen > NDIS_802_11_LENGTH_SSID)
		SSIDLen = NDIS_802_11_LENGTH_SSID;
	memset(&SSID, 0, sizeof(SSID));
	memcpy(SSID, pConnInfo->pSsid, SSIDLen);
#ifdef APCLI_CFG80211_SUPPORT
	staidx = CFG80211_FindStaIdxByNetDevice(pAd, pConnInfo->pNetDev);
	if (staidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "STATION Interface for connection not found\n");
		return TRUE;
	}
	pStaCfg = GetStaCfgByWdev(pAd, &(pAd->StaCfg[staidx].wdev));
	pStaCfg->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
	/* Check the connection is WPS or not */
	if (pConnInfo->bWpsConnection) {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AP_CLI WPS Connection onGoing.....\n");
		pStaCfg->wpa_supplicant_info.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;
	}


	/* Set authentication mode */
	if (pConnInfo->WpaVer == 2) {
		if (!pConnInfo->FlgIs8021x) {
			MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APCLI WPA2PSK\n");
#ifdef SUPP_SAE_SUPPORT
			if (pConnInfo->AuthType == Ndis802_11AuthModeWPA3PSK)
				Set_ApCli_AuthMode(pAd, staidx, "WPA3PSK");
			else
#endif
#ifdef SUPP_OWE_SUPPORT
			if (pConnInfo->AuthType == Ndis802_11AuthModeOWE)
				Set_ApCli_AuthMode(pAd, staidx, "OWE");
			else
#endif
			Set_ApCli_AuthMode(pAd, staidx, "WPA2PSK");
		}
	} else if (pConnInfo->WpaVer == 1) {
		if (!pConnInfo->FlgIs8021x) {
			MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APCLI WPAPSK\n");
			Set_ApCli_AuthMode(pAd, staidx, "WPAPSK");
		}
	} else if (pConnInfo->AuthType == Ndis802_11AuthModeShared)
		Set_ApCli_AuthMode(pAd, staidx, "SHARED");
	else if (pConnInfo->AuthType == Ndis802_11AuthModeOpen)
		Set_ApCli_AuthMode(pAd, staidx, "OPEN");
	else
		Set_ApCli_AuthMode(pAd, staidx, "WEPAUTO");
	/* Set PTK Encryption Mode */
	if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_CCMP) {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AES\n");
		Set_ApCli_EncrypType(pAd, staidx, "AES");
	} else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_TKIP) {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TKIP\n");
		Set_ApCli_EncrypType(pAd, staidx, "TKIP");
	} else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_WEP) {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WEP\n");
		Set_ApCli_EncrypType(pAd, staidx, "WEP");
	} else {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NONE\n");
		Set_ApCli_EncrypType(pAd, staidx, "NONE");
	}
/* #endif */

	if (pConnInfo->pBssid != NULL) {
		os_zero_mem(pStaCfg->CfgApCliBssid, MAC_ADDR_LEN);
		NdisCopyMemory(pStaCfg->CfgApCliBssid, pConnInfo->pBssid, MAC_ADDR_LEN);
	}
#ifdef DOT11W_PMF_SUPPORT
	if (pConnInfo->mfp)
		Set_PMFMFPC_Proc(pAd, "1");
#endif /* DOT11W_PMF_SUPPORT */

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = TRUE;
	Set_ApCli_Ssid(pAd, staidx, (RTMP_STRING *)SSID);
	Set_ApCli_Enable(pAd, staidx, "1");
	CFG80211DBG(DBG_LVL_INFO, ("80211> APCLI CONNECTING SSID = %s\n", SSID));

#else
	if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON) &&
		STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED))
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG80211: Connected, disconnect first !\n");
	else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG80211: No Connection\n");

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	/* (when go on , we need to protect  infra connect for 1.2s.) */
	ULONG	Highpart, Lowpart;
	ULONG	NextTbtt;
	ULONG	temp;

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd)) {
		/* update noa */
		AsicGetTsfTime(pAd, &Highpart, &Lowpart);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "!!!!Current Tsf LSB = = %ld\n",  Lowpart);
		RTMP_IO_READ32(pAd->hdev_ctrl, LPON_T1STR, &temp);
		temp = temp & 0x0000FFFF;
		NextTbtt	= temp % pAd->CommonCfg.BeaconPeriod;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "!!!!NextTbtt =  %ld\n", NextTbtt);
		temp = NextTbtt * 1024 + Lowpart;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "!!!!Tsf LSB + TimeTillTbtt= %ld\n", temp);
		pAd->cfg80211_ctrl.GONoASchedule.StartTime = Lowpart + NextTbtt * 1024 + 409600 + 3200;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, " pAd->GONoASchedule.StartTime = %ld\n", pAd->cfg80211_ctrl.GONoASchedule.StartTime);
		pAd->cfg80211_ctrl.GONoASchedule.Count = 20; /*wait 4 beacon + (interval * 4)*/
		pAd->cfg80211_ctrl.GONoASchedule.Duration = 1228800;
		pAd->cfg80211_ctrl.GONoASchedule.Interval =  1638400;
		OS_WAIT(400);
	}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */

	/* change to infrastructure mode if we are in ADHOC mode */
	Set_NetworkType_Proc(pAd, "Infra");
	if (pConnInfo->bWpsConnection) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPS Connection onGoing.....\n");
		/* YF@20120327: Trigger Driver to Enable WPS function. */
		pAd->StaCfg[0].wpa_supplicant_info.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;  /* Set_Wpa_Support(pAd, "3") */
		Set_AuthMode_Proc(pAd, "OPEN");
		Set_EncrypType_Proc(pAd, "NONE");
		Set_SSID_Proc(pAd, (RTMP_STRING *)SSID);
		return TRUE;
	}

	pAd->StaCfg[0].wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE; /* Set_Wpa_Support(pAd, "1")*/

	/* set authentication mode */
	if (pConnInfo->WpaVer == 2) {
		if (pConnInfo->FlgIs8021x == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPA2\n");
			Set_AuthMode_Proc(pAd, "WPA2");
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPA2PSK\n");
			Set_AuthMode_Proc(pAd, "WPA2PSK");
		}
	} else if (pConnInfo->WpaVer == 1) {
		if (pConnInfo->FlgIs8021x == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPA\n");
			Set_AuthMode_Proc(pAd, "WPA");
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPAPSK\n");
			Set_AuthMode_Proc(pAd, "WPAPSK");
		}
	} else if (pConnInfo->AuthType == Ndis802_11AuthModeAutoSwitch)
		Set_AuthMode_Proc(pAd, "WEPAUTO");
	else if (pConnInfo->AuthType == Ndis802_11AuthModeShared)
		Set_AuthMode_Proc(pAd, "SHARED");
	else
		Set_AuthMode_Proc(pAd, "OPEN");

	CFG80211DBG(DBG_LVL_INFO,
				("80211> AuthMode = %d\n", pAd->StaCfg[0].wdev.SecConfig.AKMMap));

	/* set encryption mode */
	if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_CCMP) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AES\n");
		Set_EncrypType_Proc(pAd, "AES");
	} else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_TKIP) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TKIP\n");
		Set_EncrypType_Proc(pAd, "TKIP");
	} else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_WEP) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WEP\n");
		Set_EncrypType_Proc(pAd, "WEP");
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NONE\n");
		Set_EncrypType_Proc(pAd, "NONE");
	}

	/* Groupwise Key Information Setting */
	IoctlWpa.flags = RT_CMD_STA_IOCTL_WPA_GROUP;

	if (pConnInfo->GroupwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_CCMP) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "GTK AES\n");
		IoctlWpa.value = RT_CMD_STA_IOCTL_WPA_GROUP_CCMP;
		RtmpIoctl_rt_ioctl_siwauth(pAd, &IoctlWpa, 0);
	} else if (pConnInfo->GroupwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_TKIP) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "GTK TKIP\n");
		IoctlWpa.value = RT_CMD_STA_IOCTL_WPA_GROUP_TKIP;
		RtmpIoctl_rt_ioctl_siwauth(pAd, &IoctlWpa, 0);
	}

	CFG80211DBG(DBG_LVL_INFO,
				("80211> EncrypType = %d\n", pAd->StaCfg[0].wdev.SecConfig.PairwiseCipher));
	CFG80211DBG(DBG_LVL_INFO, ("80211> Key = %s\n", pConnInfo->pKey));

	/* set channel: STATION will auto-scan */

	/* set WEP key */
	if (pConnInfo->pKey &&
		((pConnInfo->GroupwiseEncrypType | pConnInfo->PairwiseEncrypType) &
		 RT_CMD_80211_CONN_ENCRYPT_WEP)) {
		UCHAR KeyBuf[50];
		/* reset AuthMode and EncrypType */
		Set_EncrypType_Proc(pAd, "WEP");
		/* reset key */
#ifdef RT_CFG80211_DEBUG
		hex_dump("KeyBuf=", (UINT8 *)pConnInfo->pKey, pConnInfo->KeyLen);
#endif /* RT_CFG80211_DEBUG */
		pAd->StaCfg[0].wdev.SecConfig.PairwiseKeyId = pConnInfo->KeyIdx; /* base 0 */

		if (pConnInfo->KeyLen >= sizeof(KeyBuf))
			return FALSE;

		memcpy(KeyBuf, pConnInfo->pKey, pConnInfo->KeyLen);
		KeyBuf[pConnInfo->KeyLen] = 0x00;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"80211> pAd->StaCfg[0].DefaultKeyId = %d\n",
				 pAd->StaCfg[0].wdev.SecConfig.PairwiseKeyId);
		Set_Wep_Key_Proc(pAd, (RTMP_STRING *)KeyBuf, (INT)pConnInfo->KeyLen, (INT)pConnInfo->KeyIdx);
	}

	/* TODO: We need to provide a command to set BSSID to associate a AP */
	pAd->cfg80211_ctrl.FlgCfg80211Connecting = TRUE;
#ifdef DOT11W_PMF_SUPPORT

	if (pConnInfo->mfp)
		Set_PMFMFPC_Proc(pAd, "1");

#endif /* DOT11W_PMF_SUPPORT */
	Set_SSID_Proc(pAd, (RTMP_STRING *)SSID);
	CFG80211DBG(DBG_LVL_INFO, ("80211> Connecting SSID = %s\n", SSID));
#endif /* CONFIG_STA_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}


VOID CFG80211DRV_RegNotify(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_REG_NOTIFY *pRegInfo;

	pRegInfo = (CMD_RTPRIV_IOCTL_80211_REG_NOTIFY *)pData;
	/* keep Alpha2 and we can re-call the function when interface is up */
	pAd->cfg80211_ctrl.Cfg80211_Alpha2[0] = pRegInfo->Alpha2[0];
	pAd->cfg80211_ctrl.Cfg80211_Alpha2[1] = pRegInfo->Alpha2[1];

	/* apply the new regulatory rule */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		/* interface is up */
		CFG80211_RegRuleApply(pAd, pRegInfo->pWiphy, (UCHAR *)pRegInfo->Alpha2);
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "crda> interface is down!\n");
}


VOID CFG80211DRV_SurveyGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_SURVEY *pSurveyInfo;
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
#endif
	pSurveyInfo = (CMD_RTPRIV_IOCTL_80211_SURVEY *)pData;
	pSurveyInfo->pCfg80211 = pAd->pCfg80211_CB;
#ifdef AP_QLOAD_SUPPORT
	pSurveyInfo->ChannelTimeBusy = pQloadCtrl->QloadLatestChannelBusyTimePri;
	pSurveyInfo->ChannelTimeExtBusy = pQloadCtrl->QloadLatestChannelBusyTimeSec;
#endif /* AP_QLOAD_SUPPORT */
}


VOID CFG80211_UnRegister(
	IN VOID						*pAdOrg,
	IN VOID						*pNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	/* sanity check */
	if (pAd->pCfg80211_CB == NULL)
		return;

	CFG80211OS_UnRegister(pAd->pCfg80211_CB, pNetDev);
	RTMP_DRIVER_80211_SCAN_STATUS_LOCK_INIT(pAd, FALSE);
	unregister_netdevice_notifier(&cfg80211_netdev_notifier);
	/* Reset CFG80211 Global Setting Here */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "==========> TYPE Reset CFG80211 Global Setting Here <==========\n");
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE,
									pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	pAd->pCfg80211_CB = NULL;
	pAd->CommonCfg.HT_Disable = 0;

	/* It should be free when ScanEnd, */
	/*  But Hit close the device in Scanning */
	if (pCfg80211_ctrl->pCfg80211ChanList != NULL) {
		os_free_mem(pCfg80211_ctrl->pCfg80211ChanList);
		pCfg80211_ctrl->pCfg80211ChanList = NULL;
	}

	pCfg80211_ctrl->Cfg80211ChanListLen = 0;
	pCfg80211_ctrl->Cfg80211CurChanIndex = 0;

	if (pCfg80211_ctrl->pExtraIe) {
		os_free_mem(pCfg80211_ctrl->pExtraIe);
		pCfg80211_ctrl->pExtraIe = NULL;
	}

	pCfg80211_ctrl->ExtraIeLen = 0;
	/*
	 * CFG_TODO
	 *    if (pAd->pTxStatusBuf != NULL)
	 *    {
	 *	 os_free_mem(pAd->pTxStatusBuf);
	 *	 pAd->pTxStatusBuf = NULL;
	 *   }
	 *	 pAd->TxStatusBufLen = 0;
	 */
#ifdef CONFIG_AP_SUPPORT

	if (pAd->cfg80211_ctrl.beacon_tail_buf != NULL) {
		os_free_mem(pAd->cfg80211_ctrl.beacon_tail_buf);
		pAd->cfg80211_ctrl.beacon_tail_buf = NULL;
	}

	pAd->cfg80211_ctrl.beacon_tail_len = 0;
#endif /* CONFIG_AP_SUPPORT */

	if (pAd->cfg80211_ctrl.BeaconExtraIe != NULL) {
		os_free_mem(pAd->cfg80211_ctrl.BeaconExtraIe);
		pAd->cfg80211_ctrl.BeaconExtraIe = NULL;
	}

	pAd->cfg80211_ctrl.BeaconExtraIeLen = 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Parse and handle country region in beacon from associated AP.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pVIE			- Beacon elements
 *	LenVIE			- Total length of Beacon elements
 *
 * Return Value:
 *	NONE
 * ========================================================================
 */
VOID CFG80211_BeaconCountryRegionParse(
	IN VOID						*pAdCB,
	IN NDIS_802_11_VARIABLE_IEs * pVIE,
	IN UINT16					LenVIE)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	UCHAR *pElement = (UCHAR *)pVIE;
	UINT32 LenEmt;

	while (LenVIE > 0) {
		pVIE = (NDIS_802_11_VARIABLE_IEs *)pElement;

		if (pVIE->ElementID == IE_COUNTRY) {
			/* send command to do regulation hint only when associated */
			/* RT_CFG80211_CRDA_REG_HINT11D(pAd, pVIE->data, pVIE->Length); */
			RTEnqueueInternalCmd(pAd, CMDTHREAD_REG_HINT_11D,
								 pVIE->data, pVIE->Length);
			break;
		}

		LenEmt = pVIE->Length + 2;

		if (LenVIE <= LenEmt)
			break; /* length is not enough */

		pElement += LenEmt;
		LenVIE -= LenEmt;
	}
} /* End of CFG80211_BeaconCountryRegionParse */

/*
 * ========================================================================
 * Routine Description:
 *	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	CFG80211_SupBandInit() is called in xx_probe().
 * ========================================================================
 */

#ifndef APCLI_CFG80211_SUPPORT
#ifdef CONFIG_STA_SUPPORT
VOID CFG80211_LostApInform(
	IN VOID					*pAdCB,
	IN struct wifi_dev *wdev)
{
}
#endif /*CONFIG_STA_SUPPORT*/
#endif



/*
 * ========================================================================
 * Routine Description:
 *	Hint to the wireless core a regulatory domain from driver.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	pCountryIe		- pointer to the country IE
 *	CountryIeLen	- length of the country IE
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Must call the function in kernel thread.
 * ========================================================================
 */
VOID CFG80211_RegHint(
	IN VOID						*pAdCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	CFG80211OS_RegHint(CFG80211CB, pCountryIe, CountryIeLen);
}


/*
 * ========================================================================
 * Routine Description:
 *	Hint to the wireless core a regulatory domain from country element.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pCountryIe		- pointer to the country IE
 *	CountryIeLen	- length of the country IE
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Must call the function in kernel thread.
 * ========================================================================
 */
VOID CFG80211_RegHint11D(
	IN VOID						*pAdCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen)
{
	/* no regulatory_hint_11d() in 2.6.32 */
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	CFG80211OS_RegHint11D(CFG80211CB, pCountryIe, CountryIeLen);
}


/*
 * ========================================================================
 * Routine Description:
 *	Apply new regulatory rule.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pWiphy			- Wireless hardware description
 *	pAlpha2			- Regulation domain (2B)
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Can only be called when interface is up.
 *
 *	For general mac80211 device, it will be set to new power by Ops->config()
 *	In rt2x00/, the settings is done in rt2x00lib_config().
 * ========================================================================
 */
VOID CFG80211_RegRuleApply(
	IN VOID						*pAdCB,
	IN VOID						*pWiphy,
	IN UCHAR					*pAlpha2)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	VOID *pBand24G, *pBand5G;
	UINT32 IdBand, IdChan, IdPwr;
	UINT32 ChanNum, ChanId, Power, RecId, DfsType;
	BOOLEAN FlgIsRadar;
	ULONG IrqFlags;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	CFG80211DBG(DBG_LVL_INFO, ("crda> CFG80211_RegRuleApply ==>\n"));
	/* init */
	pBand24G = NULL;
	pBand5G = NULL;

	if (pAd == NULL)
		return;

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
	/* zero first */
	/* 2.4GHZ & 5GHz */
	RecId = 0;
	/* find the DfsType */
	DfsType = CE;
	pBand24G = NULL;
	pBand5G = NULL;

	if (CFG80211OS_BandInfoGet(CFG80211CB, pWiphy, &pBand24G, &pBand5G) == FALSE)
		return;

#ifdef AUTO_CH_SELECT_ENHANCE
#ifdef EXT_BUILD_CHANNEL_LIST

	if ((pAlpha2[0] != '0') && (pAlpha2[1] != '0')) {
		UINT32 IdReg;

		if (pBand5G != NULL) {
			for (IdReg = 0;; IdReg++) {
				if (ChRegion[IdReg].CountReg[0] == 0x00)
					break;

				if ((pAlpha2[0] == ChRegion[IdReg].CountReg[0]) &&
					(pAlpha2[1] == ChRegion[IdReg].CountReg[1])) {
					if (pAd->CommonCfg.DfsType != MAX_RD_REGION)
						DfsType = pAd->CommonCfg.DfsType;
					else
						DfsType = ChRegion[IdReg].op_class_region;

					CFG80211DBG(DBG_LVL_INFO,
								("crda> find region %c%c, DFS Type %d\n",
								 pAlpha2[0], pAlpha2[1], DfsType));
					break;
				}
			}
		}
	}

#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* AUTO_CH_SELECT_ENHANCE */

	for (IdBand = 0; IdBand < IEEE80211_NUM_BANDS; IdBand++) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
		if (((IdBand == NL80211_BAND_2GHZ) && (pBand24G == NULL)) ||
			((IdBand == NL80211_BAND_5GHZ) && (pBand5G == NULL)))
#else
		if (((IdBand == IEEE80211_BAND_2GHZ) && (pBand24G == NULL)) ||
			((IdBand == IEEE80211_BAND_5GHZ) && (pBand5G == NULL)))
#endif
		continue;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
		if (IdBand == NL80211_BAND_2GHZ)
#else
		if (IdBand == IEEE80211_BAND_2GHZ)
#endif
			CFG80211DBG(DBG_LVL_INFO, ("crda> reset chan/power for 2.4GHz\n"));
		else
			CFG80211DBG(DBG_LVL_INFO, ("crda> reset chan/power for 5GHz\n"));

		ChanNum = CFG80211OS_ChanNumGet(CFG80211CB, pWiphy, IdBand);

		for (IdChan = 0; IdChan < ChanNum; IdChan++) {
			if (CFG80211OS_ChanInfoGet(CFG80211CB, pWiphy, IdBand, IdChan,
									   &ChanId, &Power, &FlgIsRadar) == FALSE) {
				/* the channel is not allowed in the regulatory domain */
				/* get next channel information */
				continue;
			}

			if (!WMODE_CAP_2G(pAd->CommonCfg.cfg_wmode)) {
				/* 5G-only mode */
				if (ChanId <= CFG80211_NUM_OF_CHAN_2GHZ)
					continue;
			}

			if (!WMODE_CAP_5G(pAd->CommonCfg.cfg_wmode)) {
				/* 2.4G-only mode */
				if (ChanId > CFG80211_NUM_OF_CHAN_2GHZ)
					continue;
			}
			BandIdx = HcGetBandByChannelRange(pAd, ChanId);
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

			/* zero first */
			os_zero_mem(pChCtrl->ChList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));
			for (IdPwr = 0; IdPwr < MAX_NUM_OF_CHANNELS; IdPwr++) {
				/* sachin - TODO */
				/* if (ChanId == pAd->TxPower[IdPwr].Channel) */
				{
					/* sachin - TODO */
					/* init the channel info. */
					/* os_move_mem(&pAd->ChannelList[RecId],&pAd->TxPower[IdPwr],sizeof(CHANNEL_TX_POWER)); */
					/* keep channel number */
					pChCtrl->ChList[RecId].Channel = ChanId;
					/* keep maximum tranmission power */
					pChCtrl->ChList[RecId].MaxTxPwr = Power;

					/* keep DFS flag */
					if (FlgIsRadar == TRUE)
						pChCtrl->ChList[RecId].DfsReq = TRUE;
					else
						pChCtrl->ChList[RecId].DfsReq = FALSE;

					/* keep DFS type */
					pChCtrl->ChList[RecId].RegulatoryDomain = DfsType;
					/* re-set DFS info. */
					pAd->CommonCfg.RDDurRegion = DfsType;
					CFG80211DBG(DBG_LVL_INFO,
								("Chan %03d:\tpower %d dBm, DFS %d, DFS Type %d\n",
								 ChanId, Power,
								 ((FlgIsRadar == TRUE) ? 1 : 0),
								 DfsType));
					/* change to record next channel info. */
					RecId++;
					break;
				}
			}
		}
	}

	pChCtrl->ChListNum = RecId;
	CFG80211DBG(DBG_LVL_INFO, ("[CFG80211_RegRuleApply] - pChCtrl->ChListNum = %d\n", pChCtrl->ChListNum));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
	CFG80211DBG(DBG_LVL_INFO, ("crda> Number of channels = %d\n", RecId));
} /* End of CFG80211_RegRuleApply */

/*
 * ========================================================================
 * Routine Description:
 *	Inform CFG80211 about association status.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pBSSID			- the BSSID of the AP
 *	pReqIe			- the element list in the association request frame
 *	ReqIeLen		- the request element length
 *	pRspIe			- the element list in the association response frame
 *	RspIeLen		- the response element length
 *	FlgIsSuccess	- 1: success; otherwise: fail
 *
 * Return Value:
 *	None
 * ========================================================================
 */
VOID CFG80211_ConnectResultInform(
	IN VOID						*pAdCB,
	IN UCHAR					*pBSSID,
	IN UCHAR					*pReqIe,
	IN UINT32					ReqIeLen,
	IN UCHAR					*pRspIe,
	IN UINT32					RspIeLen,
	IN UCHAR					FlgIsSuccess)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
#ifndef APCLI_CFG80211_SUPPORT
	CFG80211DBG(DBG_LVL_INFO, ("80211> dont inform cfg as apcli uses driver implementation\n"));
#else

	CFG80211DBG(DBG_LVL_INFO, ("80211> CFG80211_ConnectResultInform ==>\n"));

	if (pAd->cfg80211_ctrl.FlgCfg80211Scanning == TRUE ||
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Abort running scan\n");
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
		pAd->cfg80211_ctrl.FlgCfg80211Scanning = FALSE;
		CFG80211OS_ScanEnd(CFG80211CB, TRUE);
	}

	if (pAd->cfg80211_ctrl.FlgCfg80211Connecting == TRUE) {
		CFG80211OS_ConnectResultInform(CFG80211CB,
								   pBSSID,
								   pReqIe,
								   ReqIeLen,
								   pRspIe,
								   RspIeLen,
								   FlgIsSuccess);
		pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
	}
#endif
} /* End of CFG80211_ConnectResultInform */

/*
 * ========================================================================
 * Routine Description:
 *	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *
 * Return Value:
 *	TRUE			- re-init successfully
 *	FALSE			- re-init fail
 *
 * Note:
 *	CFG80211_SupBandInit() is called in xx_probe().
 *	But we do not have complete chip information in xx_probe() so we
 *	need to re-init bands in xx_open().
 * ========================================================================
 */
BOOLEAN CFG80211_SupBandReInit(VOID *pAdCB, VOID *wdev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	struct wifi_dev *curr_wdev = (struct wifi_dev *)wdev;
	CFG80211_BAND BandInfo;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	CFG80211DBG(DBG_LVL_INFO, ("80211> re-init bands...\n"));
	/* re-init bands */
	os_zero_mem(&BandInfo, sizeof(BandInfo));
    CFG80211_BANDINFO_FILL(pAd, curr_wdev, &BandInfo);

	if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_24G))
		BandInfo.RFICType |= RFIC_24GHZ;

	if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_5G))
		BandInfo.RFICType |= RFIC_5GHZ;

#ifdef CONFIG_6G_SUPPORT
	if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_6G))
		BandInfo.RFICType |= RFIC_6GHZ;
#endif

	return CFG80211OS_SupBandReInit(CFG80211CB, &BandInfo);
} /* End of CFG80211_SupBandReInit */

#ifdef CONFIG_STA_SUPPORT
INT CFG80211_setStaDefaultKey(
	IN VOID                     *pAdCB,
	IN struct net_device		*pNetdev,
	IN UINT					Data
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	INT ifIndex = 0;

#ifdef APCLI_CFG80211_SUPPORT
	ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetdev);
#endif
	if (ifIndex == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set Sta Default Key invalid sta\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Sta Default Key: %d\n", Data);
	pAd->StaCfg[ifIndex].wdev.SecConfig.PairwiseKeyId = Data; /* base 0 */
	return 0;
}
#endif /*CONFIG_STA_SUPPORT*/

INT CFG80211_reSetToDefault(
	IN VOID                                         *pAdCB)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, " \n");
#ifdef CONFIG_STA_SUPPORT
	/* Driver Internal Parm */
	pAd->StaCfg[0].bAutoConnectByBssid = FALSE;
#endif /*CONFIG_STA_SUPPORT*/
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	pCfg80211_ctrl->Cfg80211RocTimerInit = FALSE;
	pCfg80211_ctrl->Cfg80211RocTimerRunning = FALSE;
	pCfg80211_ctrl->FlgCfg80211Scanning = FALSE;
	/* pCfg80211_ctrl->isMccOn = FALSE; */
	return TRUE;
}

/* initList(&pAd->Cfg80211VifDevSet.vifDevList); */
/* initList(&pAd->cfg80211_ctrl.cfg80211TxPacketList); */
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(APCLI_CFG80211_SUPPORT)
BOOLEAN CFG80211_checkScanResInKernelCache(
	IN VOID *pAdCB,
	IN UCHAR *pBSSID,
	IN UCHAR *pSsid,
	IN INT ssidLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct cfg80211_bss *bss;

	bss = cfg80211_get_bss(pWiphy, NULL, pBSSID,
						   pSsid, ssidLen,
						   WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);

	if (bss) {
		CFG80211OS_PutBss(pWiphy, bss);
		return TRUE;
	}

	return FALSE;
}

BOOLEAN CFG80211_checkScanTable(
	IN VOID *pAdCB)
{
#ifndef APCLI_CFG80211_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	ULONG bss_idx = BSS_NOT_FOUND;
	struct cfg80211_bss *bss;
	struct ieee80211_channel *chan;
	UINT32 CenFreq;
	UINT64 timestamp;
	struct timeval tv;
	UCHAR *ie, ieLen = 0;
	BOOLEAN isOk = FALSE;
	BSS_ENTRY *pBssEntry;
	USHORT ifIndex = 0;
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
	struct wifi_dev *wdev = NULL;
	BSS_TABLE *ScanTab = NULL;

	pApCliEntry = &pAd->StaCfg[ifIndex];
	wdev = &pApCliEntry->wdev;
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	if (MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ZERO_MAC_ADDR)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pAd->ApCliMlmeAux.Bssid ==> ZERO_MAC_ADDR\n");
		/* ToDo: pAd->StaCfg[0].CfgApCliBssid */
		return FALSE;
	}

	/* Fake TSF */
	do_gettimeofday(&tv);
	timestamp = ((UINT64)tv.tv_sec * 1000000) + tv.tv_usec;
	bss = cfg80211_get_bss(pWiphy, NULL, pApCliEntry->MlmeAux.Bssid,
						   pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen,
#if (KERNEL_VERSION(4, 1, 0) <= LINUX_VERSION_CODE)
						IEEE80211_BSS_TYPE_ESS, IEEE80211_PRIVACY_ANY);
#else
						   WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);
#endif

	if (bss) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Found %s in Kernel_ScanTable with CH[%d]\n", pApCliEntry->MlmeAux.Ssid, bss->channel->center_freq);
#if (KERNEL_VERSION(3, 8, 0) > LINUX_VERSION_CODE)
		bss->tsf = timestamp;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0) */
		CFG80211OS_PutBss(pWiphy, bss);
		return TRUE;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Can't Found %s in Kernel_ScanTable & Try Fake it\n", pApCliEntry->MlmeAux.Ssid);

	bss_idx = BssSsidTableSearchBySSID(ScanTab, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);

	if (bss_idx != BSS_NOT_FOUND) {
		/* Since the cfg80211 kernel scanTable not exist this Entry,
		 * Build an Entry for this connect inform event.
			 */
		pBssEntry = &ScanTab->BssEntry[bss_idx];
#if (KERNEL_VERSION(2, 6, 39) <= LINUX_VERSION_CODE)

		if (ScanTab->BssEntry[bss_idx].Channel > 14)
			CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel, IEEE80211_BAND_5GHZ);
		else
			CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel, IEEE80211_BAND_2GHZ);

#else
		CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel);
#endif
		chan = ieee80211_get_channel(pWiphy, CenFreq);
		ieLen = 2 + pApCliEntry->MlmeAux.SsidLen + pBssEntry->VarIeFromProbeRspLen;
		os_alloc_mem(NULL, (UCHAR **)&ie, ieLen);

		if (!ie) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Memory Allocate Fail in CFG80211_checkScanTable\n");
			return FALSE;
		}

		ie[0] = WLAN_EID_SSID;
		ie[1] = pApCliEntry->MlmeAux.SsidLen;
		NdisCopyMemory(ie + 2, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);
		NdisCopyMemory(ie + 2 + pApCliEntry->MlmeAux.SsidLen, pBssEntry->pVarIeFromProbRsp,
					   pBssEntry->VarIeFromProbeRspLen);
		bss = cfg80211_inform_bss(pWiphy, chan,
								  pApCliEntry->MlmeAux.Bssid, timestamp, WLAN_CAPABILITY_ESS, pApCliEntry->MlmeAux.BeaconPeriod,
								  ie, ieLen,
#ifdef CFG80211_SCAN_SIGNAL_AVG
								  (pBssEntry->AvgRssi * 100),
#else
								  (pBssEntry->Rssi * 100),
#endif
								  GFP_KERNEL);

		if (bss) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Fake New %s("MACSTR") in Kernel_ScanTable with CH[%d][%d] BI:%d len:%d\n",
					  pApCliEntry->MlmeAux.Ssid,
					  MAC2STR(pApCliEntry->MlmeAux.Bssid), bss->channel->center_freq, pBssEntry->Channel,
					  pApCliEntry->MlmeAux.BeaconPeriod, pBssEntry->VarIeFromProbeRspLen);
			CFG80211OS_PutBss(pWiphy, bss);
			isOk = TRUE;
		}

		if (ie != NULL)
			os_free_mem(ie);

		if (isOk)
			return TRUE;
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s Not In Driver Scan Table\n", pApCliEntry->MlmeAux.Ssid);

	return FALSE;
#else
	return TRUE;
#endif /* APCLI_CFG80211_SUPPORT */
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE  || APCLI_CFG80211_SUPPORT */

/* CFG_TODO */
UCHAR CFG80211_getCenCh(RTMP_ADAPTER *pAd, UCHAR prim_ch)
{
	UCHAR ret_channel;
	struct wifi_dev *wdev = NULL;
	UCHAR ht_bw = 0;
	UCHAR ext_cha = 0;

	wdev = get_default_wdev(pAd);

	if (wdev != NULL) {
		ht_bw = wlan_operate_get_ht_bw(wdev);
		ext_cha = wlan_operate_get_ext_cha(wdev);
	}

	if (ht_bw == BW_40) {
		if (ext_cha == EXTCHA_ABOVE)
			ret_channel = prim_ch + 2;
		else {
			if (prim_ch == 14)
				ret_channel = prim_ch - 1;
			else
				ret_channel = prim_ch - 2;
		}
	} else
		ret_channel = prim_ch;

	return ret_channel;
}

VOID CFG80211_JoinIBSS(
	IN VOID						*pAdCB,
	IN UCHAR					*pBSSID)
{
}

#ifdef MT_MAC
VOID CFG80211_InitTxSCallBack(RTMP_ADAPTER *pAd)
{
	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "80211> Only MT_MAC support this feature.\n");
		return;
	}

#ifdef CFG_TDLS_SUPPORT
	AddTxSTypePerPkt(pAd, PID_TDLS, TXS_FORMAT0, TdlsTxSHandler);
	TxSTypeCtlPerPkt(pAd, PID_TDLS, TXS_FORMAT0, TRUE, TRUE, FALSE, 0);
#endif /* CFG_TDLS_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
	AddTxSTypePerPkt(pAd, PID_P2P_ACTION, TXS_FORMAT0, ActionTxSHandler);
	TxSTypeCtlPerPkt(pAd, PID_P2P_ACTION, TXS_FORMAT0, FALSE, TRUE, FALSE, 0);
#endif /* RT_CFG80211_P2P_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
}
#endif /* MT_MAC */

#ifdef CONFIG_VLAN_GTK_SUPPORT
struct wifi_dev *CFG80211_GetWdevByVlandev(PRTMP_ADAPTER pAd, PNET_DEV vlan_dev)
{
	UINT8 apidx;
	struct wifi_dev *wdev = NULL;

	if (!pAd)
		return NULL;

	for (apidx = MAIN_MBSSID; apidx < pAd->ApCfg.BssidNum; apidx++) {
		PNET_DEV tmp_ndev;
		UINT8 ifname_len;
		char *pch;

		tmp_ndev = pAd->ApCfg.MBSSID[apidx].wdev.if_dev;
		if (!tmp_ndev)
			continue;

		pch = strchr(vlan_dev->name, '.');
		ifname_len = pch - tmp_ndev->name;
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ifname=%s, vlan_ifname=%s\n",
					tmp_ndev->name, vlan_dev->name);
		if (strncmp(tmp_ndev->name, vlan_dev->name, ifname_len) == 0) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			break;
		}
	}

	return wdev;
}

BOOLEAN CFG80211_MatchVlandev(struct wifi_dev *wdev, PNET_DEV vlan_dev)
{
	struct list_head *listptr;
	struct vlan_gtk_info *vg_info;

	if (!wdev)
		return FALSE;

	list_for_each(listptr, &wdev->vlan_gtk_list) {
		vg_info = list_entry(listptr, struct vlan_gtk_info, list);
		if (vg_info && vg_info->vlan_dev == vlan_dev)
			return TRUE;
	}

	return FALSE;
}

struct vlan_gtk_info *CFG80211_GetVlanInfoByVlandev(struct wifi_dev *wdev, PNET_DEV vlan_dev)
{
	struct list_head *listptr;
	struct vlan_gtk_info *vg_info;

	if (!wdev)
		return NULL;

	list_for_each(listptr, &wdev->vlan_gtk_list) {
		vg_info = list_entry(listptr, struct vlan_gtk_info, list);
		if (vg_info && vg_info->vlan_dev == vlan_dev)
			return vg_info;
	}

	return NULL;
}

struct vlan_gtk_info *CFG80211_GetVlanInfoByVlanid(struct wifi_dev *wdev, UINT16 vlan_id)
{
	struct list_head *listptr;
	struct vlan_gtk_info *vg_info;

	if (!wdev)
		return NULL;

	list_for_each(listptr, &wdev->vlan_gtk_list) {
		vg_info = list_entry(listptr, struct vlan_gtk_info, list);
		if (vg_info && vg_info->vlan_id == vlan_id)
			return vg_info;
	}

	return NULL;
}

/* return VLAN ID on success */
INT16 CFG80211_IsVlanPkt(PNDIS_PACKET pkt)
{
	UINT16 vlan_id = 0;
	struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);
	UCHAR *data = skb->data;

	if (ntohs(skb->protocol) == ETH_TYPE_VLAN) {
		CFG80211DBG(DBG_LVL_DEBUG, ("%s() get vlan pkt\n", __func__));
		vlan_id = ((data[14] << 8) + data[15]) & 0xfff;
	}

	return vlan_id;
}
#endif
#endif /* RT_CFG80211_SUPPORT */

