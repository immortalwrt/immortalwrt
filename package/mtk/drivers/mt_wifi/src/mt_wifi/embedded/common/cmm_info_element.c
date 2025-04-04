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
    cmm_info_element.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      --------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/

#include "rt_config.h"

VOID SupportRate(
	IN struct legacy_rate *rate,
	OUT PUCHAR *ppRates,
	OUT PUCHAR RatesLen,
	OUT PUCHAR pMaxSupportRate)
{
	INT i;
	UCHAR sup_rate_len, ext_rate_len, *sup_rate, *ext_rate;

	sup_rate_len = rate->sup_rate_len;
	ext_rate_len = rate->ext_rate_len;
	sup_rate = rate->sup_rate;
	ext_rate = rate->ext_rate;

	*pMaxSupportRate = 0;
	if ((sup_rate_len <= MAX_LEN_OF_SUPPORTED_RATES) && (sup_rate_len > 0)) {
		NdisMoveMemory(*ppRates, sup_rate, sup_rate_len);
		*RatesLen = sup_rate_len;
	} else {
		/* HT rate not ready yet. return true temporarily. rt2860c */
		/*MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PeerAssocReqSanity - wrong IE_SUPP_RATES\n"); */
		*RatesLen = 8;
		*(*ppRates + 0) = 0x82;
		*(*ppRates + 1) = 0x84;
		*(*ppRates + 2) = 0x8b;
		*(*ppRates + 3) = 0x96;
		*(*ppRates + 4) = 0x12;
		*(*ppRates + 5) = 0x24;
		*(*ppRates + 6) = 0x48;
		*(*ppRates + 7) = 0x6c;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "SUPP_RATES., Len=%d\n", sup_rate_len);
	}

	if (ext_rate_len + *RatesLen <= MAX_LEN_OF_SUPPORTED_RATES) {
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ext_rate, ext_rate_len);
		*RatesLen = (*RatesLen) + ext_rate_len;
	} else {
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ext_rate, MAX_LEN_OF_SUPPORTED_RATES - (*RatesLen));
		*RatesLen = MAX_LEN_OF_SUPPORTED_RATES;
	}


	for (i = 0; i < *RatesLen; i++) {
		if (*pMaxSupportRate < (*(*ppRates + i) & 0x7f))
			*pMaxSupportRate = (*(*ppRates + i) & 0x7f);
	}
}

#ifdef DOT11_N_SUPPORT
void build_ext_channel_switch_ie(
	IN PRTMP_ADAPTER pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE * pIE,
	IN UCHAR Channel,
	IN USHORT PhyMode,
	IN struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	pIE->ID = IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT;
	pIE->Length = 4;
#ifdef ZERO_LOSS_CSA_SUPPORT
	/*if radar not detected on old channel, allow frames*/
	if ((pAd->Zero_Loss_Enable == 1) && (pDot11h->RDMode != RD_SILENCE_MODE))
		pIE->ChannelSwitchMode = 0;/*frames allowed*/
	else
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	pIE->ChannelSwitchMode = 1;	/*no further frames */
	pIE->NewRegClass = get_regulatory_class_for_newCh(pAd, Channel, PhyMode, wdev);
	pIE->NewChannelNum = Channel;
	pIE->ChannelSwitchCount = (pDot11h->CSPeriod - pDot11h->CSCount - 1);
}
#endif /* DOT11_N_SUPPORT */

static INT build_wsc_probe_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
#ifdef WSC_INCLUDED
	BOOLEAN bHasWscIe = FALSE;
#ifdef CONFIG_MAP_SUPPORT
	UCHAR ChBandIdx = HcGetBandByWdev(wdev);

/* To prevent WSC overlap in dbdc case ,disable wsc IE in probe request during all channel scan */
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		if ((pAd->ScanCtrl[ChBandIdx].Channel != 0) &&
			(pAd->ScanCtrl[ChBandIdx].ScanReqwdev) &&
			(pAd->ScanCtrl[ChBandIdx].ScanReqwdev->wdev_type == WDEV_TYPE_STA))
			return len;
	}

#endif

	if (IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)) {
#ifdef APCLI_SUPPORT
			/*
				Append WSC information in probe request if WSC state is running
			*/
			if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
				(wdev->WscControl.bWscTrigger))
				bHasWscIe = TRUE;

#if defined(WSC_V2_SUPPORT) && !defined(CONFIG_MAP_SUPPORT) && !defined(CON_WPS)
			/* need to check if !defined(CONFIG_MAP_SUPPORT) is necessary */
			else if (wdev->WscControl.WscV2Info.bEnableWpsV2)
				bHasWscIe = TRUE;
#endif /* WSC_V2_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef CON_WPS
		if (bHasWscIe) {
			PWSC_CTRL pWscControl = NULL;

			/* Do not include wsc ie in case concurrent WPS is running */
			bHasWscIe = FALSE;
			pWscControl = &wdev->WscControl;

			if ((pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED) ||
			    (pAd->ApCfg.ConWpsApCliMode != CON_WPS_APCLI_BAND_AUTO))
				bHasWscIe = TRUE;

			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("[scan_active: %d] ConWpsApCliMode=%d conWscStatus=%d bHasWscIe=%d\n",
				  __LINE__, pAd->ApCfg.ConWpsApCliMode,
				  pWscControl->conWscStatus, bHasWscIe));

		}
#endif /*CON_WPS*/
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
#ifdef CONFIG_STA_SUPPORT
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

		ASSERT(pStaCfg);
#ifdef WSC_STA_SUPPORT

		/*
			Append WSC information in probe request if WSC state is running
		*/
		if ((wdev->WscControl.WscEnProbeReqIE) &&
			(wdev->WscControl.WscConfMode != WSC_DISABLE) &&
			(wdev->WscControl.bWscTrigger == TRUE))
			bHasWscIe = TRUE;

#ifdef WSC_V2_SUPPORT
		else if ((wdev->WscControl.WscEnProbeReqIE) &&
				 (wdev->WscControl.WscV2Info.bEnableWpsV2))
			bHasWscIe = TRUE;

#endif /* WSC_V2_SUPPORT */

#endif /* WSC_STA_SUPPORT */
#endif /*CONFIG_STA_SUPPORT*/
	}

	if (bHasWscIe) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;

		os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);

		if (pWscBuf != NULL) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildProbeReqIE(pAd, wdev, pWscBuf, &WscIeLen);
			MAKE_IE_TO_BUF(buf, pWscBuf, WscIeLen, len);
			os_free_mem(pWscBuf);
		} else
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:: WscBuf Allocate failed!\n", __func__));
	}

#endif /* WSC_INCLUDED */
	return len;
}

static INT build_wsc_probe_rsp_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;

	switch (wdev->wdev_type) {
#ifdef WSC_AP_SUPPORT

	case WDEV_TYPE_AP: {
		/* for windows 7 logo test */
		if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
#ifdef DOT1X_SUPPORT
			(!IS_IEEE8021X_Entry(wdev)) &&
#endif /* DOT1X_SUPPORT */
			(IS_CIPHER_WEP(wdev->SecConfig.PairwiseCipher))) {
			/*
				Non-WPS Windows XP and Vista PCs are unable to determine if a WEP enalbed network is static key based
				or 802.1X based. If the legacy station gets an EAP-Rquest/Identity from the AP, it assume the WEP
				network is 802.1X enabled & will prompt the user for 802.1X credentials. If the legacy station doesn't
				receive anything after sending an EAPOL-Start, it will assume the WEP network is static key based and
				prompt user for the WEP key. <<from "WPS and Static Key WEP Networks">>
				A WPS enabled AP should include this IE in the beacon when the AP is hosting a static WEP key network.
				The IE would be 7 bytes long with the Extended Capability field set to 0 (all bits zero)
				http://msdn.microsoft.com/library/default.asp?url=/library/en-us/randz/protocol/securing_public_wi-fi_hotspots.asp
			*/
			const UCHAR PROVISION_SERVICE_IE[7] = {0xDD, 0x05, 0x00, 0x50, 0xF2, 0x05, 0x00};

			MAKE_IE_TO_BUF(buf, PROVISION_SERVICE_IE, 7, len);
		}

		/* add Simple Config Information Element */
		if ((wdev->WscControl.WscConfMode > WSC_DISABLE) && (wdev->WscIEProbeResp.ValueLen))
			MAKE_IE_TO_BUF(buf, wdev->WscIEProbeResp.Value, wdev->WscIEProbeResp.ValueLen, len);

		break;
	}

#endif /* WSC_AP_SUPPORT */
#ifdef WSC_STA_SUPPORT

	case WDEV_TYPE_ADHOC: {
		/* PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd,wdev); */
		/* add Simple Config Information Element */
		if (wdev->WscIEProbeResp.ValueLen != 0)
			MAKE_IE_TO_BUF(buf, wdev->WscIEProbeResp.Value, wdev->WscIEProbeResp.ValueLen, len);

		break;
	}

#endif /* WSC_STA_SUPPORT */
	}

	return len;
}

static INT build_wsc_assoc_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
#ifdef WSC_STA_SUPPORT

	/* Add WSC IE if we are connecting to WSC AP */
	if ((wdev->WscControl.WscEnAssociateIE) &&
		(wdev->WscControl.WscConfMode != WSC_DISABLE) &&
		(wdev->WscControl.bWscTrigger)) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;

		os_alloc_mem(pAd, (UCHAR **) &pWscBuf, 512);

		if (pWscBuf != NULL) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildAssocReqIE(&wdev->WscControl, pWscBuf, &WscIeLen);
			MAKE_IE_TO_BUF(buf, pWscBuf, WscIeLen, len);
			os_free_mem(pWscBuf);
		} else
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("%s:: WscBuf Allocate failed!\n",
					  __func__));
	}

#endif /* WSC_STA_SUPPORT */
	return len;
}

INT build_wsc_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;

	if (info->frame_subtype == SUBTYPE_PROBE_REQ)
		len += build_wsc_probe_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else if (info->frame_subtype == SUBTYPE_PROBE_RSP)
		len += build_wsc_probe_rsp_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else if (info->frame_subtype == SUBTYPE_ASSOC_REQ)
		len += build_wsc_assoc_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));

	return len;
}

/* rsp */
INT build_rsn_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	pSecConfig = &wdev->SecConfig;

	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP: {
		BSS_STRUCT *mbss;
		CHAR rsne_idx = 0;

		mbss = wdev->func_dev;
#ifdef CONFIG_HOTSPOT_R2

		if ((mbss->HotSpotCtrl.HotSpotEnable == 0) &&
			(mbss->HotSpotCtrl.bASANEnable == 1) &&
			(IS_AKM_WPA2_Entry(wdev))) {
			/* replace RSN IE with OSEN IE if it's OSEN wdev */
			extern UCHAR OSEN_IE[];
			extern UCHAR OSEN_IELEN;
			UCHAR RSNIe = IE_WPA;

			MAKE_IE_TO_BUF(buf, &RSNIe, 1, len);
			MAKE_IE_TO_BUF(buf, &OSEN_IELEN,  1, len);
			MAKE_IE_TO_BUF(buf, OSEN_IE, OSEN_IELEN, len);
		} else
#endif /* CONFIG_HOTSPOT_R2 */
		{
			for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
				if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
					continue;

				MAKE_IE_TO_BUF(buf, &pSecConfig->RSNE_EID[rsne_idx][0], 1, len);
				MAKE_IE_TO_BUF(buf, &pSecConfig->RSNE_Len[rsne_idx], 1, len);
				MAKE_IE_TO_BUF(buf, &pSecConfig->RSNE_Content[rsne_idx][0],
							   pSecConfig->RSNE_Len[rsne_idx], len);
			}
		}

		break;
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	case WDEV_TYPE_ADHOC: {
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
		UCHAR RSNIe = IE_WPA;

		/* Modify by Eddy, support WPA2PSK in Adhoc mode */
		if (IS_AKM_WPANONE(wdev->SecConfig.AKMMap)
#ifdef ADHOC_WPA2PSK_SUPPORT
			|| IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)
#endif /* ADHOC_WPA2PSK_SUPPORT */
		   ) {
#ifdef ADHOC_WPA2PSK_SUPPORT
			RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, BSS0);

			if (IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))
				RSNIe = IE_RSN;

#endif /* ADHOC_WPA2PSK_SUPPORT */
			MAKE_IE_TO_BUF(buf, &RSNIe, 1, len);
			MAKE_IE_TO_BUF(buf, &pStaCfg->RSNIE_Len, 1, len);
			MAKE_IE_TO_BUF(buf, pStaCfg->RSN_IE, pStaCfg->RSNIE_Len, len);
		}

		break;
	}

#endif /* CONFIG_STA_SUPPORT */
	}

	return len;
}


static INT build_extra_probe_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	SCAN_INFO *ScanInfo = &wdev->ScanInfo;
#ifdef CONFIG_STA_SUPPORT
#ifdef RT_CFG80211_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (wdev->wdev_type == WDEV_TYPE_STA)
		ASSERT(pStaCfg);
#ifdef WPA_SUPPLICANT_SUPPORT

	if ((wdev->wdev_type == WDEV_TYPE_STA) &&
		(pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
		(pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen != 0)) {
		MAKE_IE_TO_BUF(buf, pStaCfg->wpa_supplicant_info.pWpsProbeReqIe,
					   pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen, len);
	}

#endif /* WPA_SUPPLICANT_SUPPORT */

#if defined(WPA_SUPPLICANT_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	if (pStaCfg &&
		(pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
		(pAd->cfg80211_ctrl.ExtraIeLen > 0)) {
		MAKE_IE_TO_BUF(buf, pAd->cfg80211_ctrl.pExtraIe,
					   pAd->cfg80211_ctrl.ExtraIeLen, len);
	}
#endif

#endif /* RT_CFG80211_SUPPORT */
#endif /*CONFIG_STA_SUPPORT*/

	if (ScanInfo->ExtraIeLen && ScanInfo->ExtraIe) {
		MAKE_IE_TO_BUF(buf, ScanInfo->ExtraIe,
					   ScanInfo->ExtraIeLen, len);
	}

	return len;
}

static INT build_extra_assoc_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
#ifdef WPA_SUPPLICANT_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	/*
		Can not use SIOCSIWGENIE definition, it is used in wireless.h
		We will not see the definition in MODULE.
		The definition can be saw in UTIL and NETIF.
	*/
	if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE) &&
		(pStaCfg->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant == TRUE)) {
		USHORT VarIesOffset = 0;

		MAKE_IE_TO_BUF(buf, pStaCfg->wpa_supplicant_info.pWpaAssocIe,
					   pStaCfg->wpa_supplicant_info.WpaAssocIeLen, len);
		VarIesOffset = pStaCfg->ReqVarIELen;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset,
					   pStaCfg->wpa_supplicant_info.pWpaAssocIe,
					   pStaCfg->wpa_supplicant_info.WpaAssocIeLen);
		VarIesOffset += pStaCfg->wpa_supplicant_info.WpaAssocIeLen;
		/* Set Variable IEs Length */
		pStaCfg->ReqVarIELen = VarIesOffset;
	}

#endif /* WPA_SUPPLICANT_SUPPORT */
	return len;
}

INT build_extra_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;

	if (info->frame_subtype == SUBTYPE_PROBE_REQ)
		len += build_extra_probe_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else if (info->frame_subtype == SUBTYPE_ASSOC_REQ)
		len += build_extra_assoc_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));

	return len;
}

#ifdef CONFIG_AP_SUPPORT
/* Extended Capabilities IE */
INT build_ap_extended_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	ULONG infoPos;
	PUCHAR pInfo;
	BOOLEAN bNeedAppendExtIE = FALSE;
	UCHAR extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
	BSS_STRUCT *mbss;
	EXT_CAP_INFO_ELEMENT extCapInfo = { 0 };

	/* NdisZeroMemory(&extCapInfo, extInfoLen); */
	mbss = wdev->func_dev;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	/* P802.11n_D1.10, HT Information Exchange Support */
	if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
		WMODE_CAP_N(wdev->PhyMode) && WMODE_CAP_2G(wdev->PhyMode) &&
		(wdev->DesiredHtPhyInfo.bHtEnable))
		extCapInfo.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#if defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP)
	if (mbss->WNMCtrl.ProxyARPEnable)
		extCapInfo.proxy_arp = 1;
#endif
#ifdef CONFIG_DOT11V_WNM
	if (mbss->WNMCtrl.WNMBTMEnable)
		extCapInfo.BssTransitionManmt = 1;

#ifdef CONFIG_HOTSPOT_R2

	if (mbss->WNMCtrl.WNMNotifyEnable)
		extCapInfo.wnm_notification = 1;

	if (mbss->HotSpotCtrl.QosMapEnable && mbss->HotSpotCtrl.HotSpotEnable)
		extCapInfo.qosmap = 1;

#endif /* CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_DOT11V_WNM */

#ifdef QOS_R1
	if (IS_QOSR1_ENABLE(pAd)) {
		extCapInfo.qosmap = mbss->QosMapSupport ? 1 : 0;
		extCapInfo.dot11MSCSActivated = 1;
#ifdef QOS_R2
		extCapInfo.dot11SCSActivated = pAd->bScsEnable ? 1 : 0;
#endif
	}
#endif

#if defined(WAPP_SUPPORT) || defined(FTM_SUPPORT) || defined(CONFIG_DOT11U_INTERWORKING)

	if (mbss->GASCtrl.b11U_enable)
		extCapInfo.interworking = 1;

#endif
#ifdef DOT11V_WNM_SUPPORT

	if (IS_BSS_TRANSIT_MANMT_SUPPORT(pAd, wdev->func_idx))
		extCapInfo.BssTransitionManmt = 1;

	if (IS_WNMDMS_SUPPORT(pAd, wdev->func_idx))
		extCapInfo.DMSSupport = 1;

#endif /* DOT11V_WNM_SUPPORT */
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode) &&
		WMODE_CAP_5G(wdev->PhyMode))
		extCapInfo.operating_mode_notification = 1;

#endif /* DOT11_VHT_AC */

#ifdef OCE_FILS_SUPPORT
	if (IS_AKM_FILS(wdev->SecConfig.AKMMap))
		extCapInfo.FILSCap = 1;
#endif /* OCE_FILS_SUPPORT */


#ifdef DOT11V_MBSSID_SUPPORT
	if (IS_MBSSID_IE_NEEDED(pAd, mbss, HcGetBandByWdev(wdev)))
		extCapInfo.mbssid = 1;
	else
		extCapInfo.mbssid = 0;

	if (IS_BSSID_11V_ENABLED(pAd, HcGetBandByWdev(wdev)))
		extCapInfo.cmpl_non_txbssid = 1;
	else
		extCapInfo.cmpl_non_txbssid = 0;
#endif


#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode)) {
#ifdef WIFI_TWT_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			if (wlan_config_get_asic_twt_caps(wdev) &&
				TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))
				extCapInfo.twt_responder_support = 1;
		} else if (wdev->wdev_type == WDEV_TYPE_STA) {
			if (wlan_config_get_asic_twt_caps(wdev) &&
				TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))
				extCapInfo.twt_requester_support = 1;
		}
#endif /* WIFI_TWT_SUPPORT */
    }
#endif /* DOT11_HE_AX */

#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef DOT11_SAE_SUPPORT
	{
		struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;

		if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->pwd_id_cnt != 0)
			extCapInfo.sae_pwd_id_in_use = 1;
		else
			extCapInfo.sae_pwd_id_in_use = 0;

		if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_cap.pwd_id_only)
			extCapInfo.sae_pwd_id_used_exclusively = 1;
		else
			extCapInfo.sae_pwd_id_used_exclusively = 0;

		if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_cap.sae_pk_en)
			extCapInfo.sae_pk_pwd_used_exclusively = 1;
		else
			extCapInfo.sae_pk_pwd_used_exclusively = 0;
	}
#endif
#endif /*HOSTAPD_WPA3_SUPPORT*/

#ifdef BCN_PROTECTION_SUPPORT
	{
		struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;

		if (sec_cfg->bcn_prot_cfg.bcn_prot_en)
			extCapInfo.bcn_prot_en = 1;
		else
			extCapInfo.bcn_prot_en = 0;
	}
#endif

#ifdef RT_BIG_ENDIAN
	RTMPEndianChange((UCHAR *)&extCapInfo, 8);
#endif

	pInfo = (PUCHAR)(&extCapInfo);

	for (infoPos = 0; infoPos < extInfoLen; infoPos++) {
		if (pInfo[infoPos] != 0) {
			bNeedAppendExtIE = TRUE;
			break;
		}
	}

	if (bNeedAppendExtIE == TRUE) {
		for (infoPos = (extInfoLen - 1); infoPos >= EXT_CAP_MIN_SAFE_LENGTH; infoPos--) {
			if (pInfo[infoPos] == 0)
				extInfoLen--;
			else
				break;
		}

		MAKE_IE_TO_BUF(buf, &ExtCapIe,   1, len);
		MAKE_IE_TO_BUF(buf, &extInfoLen, 1, len);
		MAKE_IE_TO_BUF(buf, &extCapInfo, extInfoLen, len);
	}

	return len;
}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
INT build_sta_extended_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
#if defined(DOT11N_DRAFT3) || defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	{
		EXT_CAP_INFO_ELEMENT extCapInfo;
		UCHAR extInfoLen;
		ULONG infoPos;
		PUCHAR pInfo;
		BOOLEAN bNeedAppendExtIE = FALSE;

		extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
		NdisZeroMemory(&extCapInfo, extInfoLen);
#ifdef DOT11N_DRAFT3

		if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
			WMODE_CAP_N(wdev->PhyMode)
			&& WMODE_CAP_2G(wdev->PhyMode)
		   )
			extCapInfo.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#ifdef MBO_SUPPORT
		if (IS_MBO_ENABLE(wdev))
			extCapInfo.BssTransitionManmt = 1;
#endif /* MBO_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

		if (IS_TDLS_SUPPORT(pAd)) {
			extCapInfo.UAPSDBufSTASupport = 1;
#ifdef CFG_TDLS_SUPPORT

			if (pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TdlsChSwitchSupp)
#else
			if (pStaCfg->TdlsInfo.TdlsChSwitchSupp)
#endif
			{
				extCapInfo.TDLSChSwitchSupport = 1;
			} else
				extCapInfo.TDLSChSwitchSupport = 0;

			extCapInfo.TDLSSupport = 1;
		}

#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#ifdef DOT11_VHT_AC

		if (WMODE_CAP_AC(wdev->PhyMode) &&
			WMODE_CAP_5G(wdev->PhyMode))
			extCapInfo.operating_mode_notification = 1;

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode)) {
#ifdef DOT11V_MBSSID_SUPPORT
		extCapInfo.mbssid = 1;
#endif /* DOT11V_MBSSID_SUPPORT */
#ifdef WIFI_TWT_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			if (wlan_config_get_asic_twt_caps(wdev) &&
				TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))
				extCapInfo.twt_responder_support = 1;
		} else if (wdev->wdev_type == WDEV_TYPE_STA) {
			if (wlan_config_get_asic_twt_caps(wdev) &&
				TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))
				extCapInfo.twt_requester_support = 1;
		}
#endif /* WIFI_TWT_SUPPORT */
    }
#endif /* DOT11_HE_AX */

#ifdef DOT11_SAE_SUPPORT
		{
			struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;

			if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->pwd_id_cnt != 0)
				extCapInfo.sae_pwd_id_in_use = 1;
			else
				extCapInfo.sae_pwd_id_in_use = 0;

			if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_cap.pwd_id_only)
				extCapInfo.sae_pwd_id_used_exclusively = 1;
			else
				extCapInfo.sae_pwd_id_used_exclusively = 0;
		}
#endif

		pInfo = (PUCHAR)(&extCapInfo);

		for (infoPos = 0; infoPos < extInfoLen; infoPos++) {
			if (pInfo[infoPos] != 0) {
				bNeedAppendExtIE = TRUE;
				break;
			}
		}

		if (bNeedAppendExtIE == TRUE) {
			for (infoPos = (extInfoLen - 1); infoPos >= EXT_CAP_MIN_SAFE_LENGTH; infoPos--) {
				if (pInfo[infoPos] == 0)
					extInfoLen--;
				else
					break;
			}
#ifdef RT_BIG_ENDIAN
			RTMPEndianChange((UCHAR *)&extCapInfo, 8);
#endif

			MAKE_IE_TO_BUF(buf, &ExtCapIe,   1, len);
			MAKE_IE_TO_BUF(buf, &extInfoLen, 1, len);
			MAKE_IE_TO_BUF(buf, &extCapInfo, extInfoLen, len);
		}
	}
#endif /* defined(DOT11N_DRAFT3) || defined(DOT11Z_TDLS_SUPPORT) */
	return len;
}
#endif /* CONFIG_STA_SUPPORT */

INT build_extended_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;
	struct wifi_dev *wdev = info->wdev;

	switch (wdev->wdev_type) {
#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_MESH:
		len += build_sta_extended_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
		break;
#endif /* CONFIG_STA_SUPPORT */
	default:
#ifdef CONFIG_AP_SUPPORT
		len += build_ap_extended_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
#endif /* CONFIG_AP_SUPPORT */
		break;
	}

	return len;
}

#ifdef CONFIG_AP_SUPPORT
static INT build_ap_wmm_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	UCHAR i;
	UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0};
	UINT8 AIFSN[4];
	struct _EDCA_PARM *pBssEdca = wlan_config_get_ht_edca(wdev);

	if (pBssEdca) {
		WmeParmIe[8] = pBssEdca->EdcaUpdateCount & 0x0f;
#ifdef UAPSD_SUPPORT
		UAPSD_MR_IE_FILL(WmeParmIe[8], &wdev->UapsdInfo);
#endif /* UAPSD_SUPPORT */
		NdisMoveMemory(AIFSN, pBssEdca->Aifsn, sizeof(AIFSN));

		for (i = QID_AC_BK; i <= QID_AC_VO; i++) {
			WmeParmIe[10 + (i * 4)] = (i << 5)							  +	/* b5-6 is ACI */
									  ((UCHAR)pBssEdca->bACM[i] << 4)	  +	/* b4 is ACM */
									  (AIFSN[i] & 0x0f);						/* b0-3 is AIFSN */
			WmeParmIe[11 + (i * 4)] = (pBssEdca->Cwmax[i] << 4)		  +	/* b5-8 is CWMAX */
									  (pBssEdca->Cwmin[i] & 0x0f);				/* b0-3 is CWMIN */
			WmeParmIe[12 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);		/* low byte of TXOP */
			WmeParmIe[13 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);			/* high byte of TXOP */
		}

		MAKE_IE_TO_BUF(buf, WmeParmIe, 26, len);
	}

	return len;
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
static INT build_sta_wmm_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	UCHAR WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (pStaCfg->MlmeAux.APEdcaParm.bValid) {
		if (pStaCfg->wdev.UapsdInfo.bAPSDCapable
			&& pStaCfg->MlmeAux.APEdcaParm.bAPSDCapable) {
			QBSS_STA_INFO_PARM QosInfo;

			NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
			QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
			QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
			QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
			QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
			QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
			WmeIe[8] |= *(PUCHAR)&QosInfo;
		} else {
			/* The Parameter Set Count is set to 0 in the association request frames */
			/* WmeIe[8] |= (pStaCfg->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f); */
		}

		MAKE_IE_TO_BUF(buf, WmeIe, 9, len);
	}

	return len;
}
#endif /* CONFIG_STA_SUPPORT */

INT build_wmm_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;
	struct wifi_dev *wdev = info->wdev;

	if (!wdev->bWmmCapable)
		return len;

	switch (wdev->wdev_type) {
#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_MESH:
	case WDEV_TYPE_REPEATER:
		len += build_sta_wmm_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
		break;
#endif /* CONFIG_STA_SUPPORT */

	default:
#ifdef CONFIG_AP_SUPPORT
		len += build_ap_wmm_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
#endif /* CONFIG_AP_SUPPORT */
		break;
	}

	return len;
}


#ifdef MBO_SUPPORT
INT build_supp_op_class_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	ULONG TmpLen;
	UCHAR OperatingIe = IE_SUPP_REG_CLASS;
	UCHAR OperClassList = 0;
	UCHAR SuppClasslist[50];
	INT len = 0;
	UCHAR RegulatoryClass = get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev);

	OperClassList = get_operating_class_list(pAd, wdev->channel, wdev->PhyMode, wdev, SuppClasslist, &len);
	SuppClasslist[len] = 130;
	len += 2; /* to include Regulatory Class and Delimiter 0x82 */
	if (OperClassList == TRUE) {
		MakeOutgoingFrame(buf,    &TmpLen,
				1, &OperatingIe,
				1, &len,
				1, &RegulatoryClass,
				len-1, &SuppClasslist,
				END_OF_ARGS);
	}
	return TmpLen;
}
#endif /* MBO_SUPPORT */


ULONG build_support_rate_ie(struct wifi_dev *wdev, UCHAR *sup_rate, UCHAR sup_rate_len, UCHAR *buf)
{
	ULONG frame_len = 0;
	ULONG bss_mem_selector_len = 0;
	UCHAR bss_mem_selector_code = BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY;
	USHORT PhyMode = wdev->PhyMode;
	UCHAR real_sup_rate_len = sup_rate_len;
	UCHAR total_len;

	if (sup_rate_len == 0)
		return frame_len;

	if (PhyMode == WMODE_B)
		real_sup_rate_len = 4;
#ifdef RT_CFG80211_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
#endif
#endif /*RT_CFG80211_SUPPORT*/
#ifdef DOT11_SAE_SUPPORT
	if (wdev->SecConfig.sae_cap.gen_pwe_method == PWE_HASH_ONLY && real_sup_rate_len < 8)
		bss_mem_selector_len++;
#endif
#ifdef RT_CFG80211_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
	}
#endif
#endif /*RT_CFG80211_SUPPORT*/

	total_len = real_sup_rate_len + bss_mem_selector_len;

	MakeOutgoingFrame(buf, &frame_len, 1, &SupRateIe,
				1, &total_len,
				real_sup_rate_len, sup_rate,
				bss_mem_selector_len, &bss_mem_selector_code,
				END_OF_ARGS);

	return frame_len;
}


ULONG build_support_ext_rate_ie(struct wifi_dev *wdev, UCHAR sup_rate_len,
	UCHAR *ext_sup_rate, UCHAR ext_sup_rate_len, UCHAR *buf)
{
	ULONG frame_len = 0;
	ULONG bss_mem_selector_len = 0;
	UCHAR bss_mem_selector_code = BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY;
	USHORT PhyMode = wdev->PhyMode;
	UCHAR total_len;

	if (PhyMode == WMODE_B)
		return frame_len;

#ifdef HOSTAPD_WPA3R3_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP
		&& wdev->SecConfig.SaePwe == SAE_PWE_HASH_TO_ELEMENT && sup_rate_len >= 8) {
		bss_mem_selector_len++;
	}
#endif

#ifdef RT_CFG80211_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
#endif
#endif /*RT_CFG80211_SUPPORT*/
#ifdef DOT11_SAE_SUPPORT
	if (sup_rate_len >= 8 && wdev->SecConfig.sae_cap.gen_pwe_method == PWE_HASH_ONLY)
		bss_mem_selector_len++;
#endif
#ifdef RT_CFG80211_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
	}
#endif
#endif /*RT_CFG80211_SUPPORT*/

	if (ext_sup_rate_len == 0 && bss_mem_selector_len == 0)
		return frame_len;

	total_len = ext_sup_rate_len + bss_mem_selector_len;

	MakeOutgoingFrame(buf, &frame_len, 1, &ExtRateIe,
				1, &total_len,
				ext_sup_rate_len, ext_sup_rate,
				bss_mem_selector_len, &bss_mem_selector_code,
				END_OF_ARGS);

	return frame_len;
}

/* BSS Max Idle Period element */
ULONG build_bss_max_idle_ie(
	struct wifi_dev *wdev,
	UCHAR *buf,
	UINT16 max_idle_period,
	UINT8 option)
{
	UCHAR ie = IE_BSS_MAX_IDLE;
	UINT8 idle_opt = 0;
	ULONG total_len = 0;
	INT ie_len = 3;

	if (option)
		idle_opt |= 0x1 << 0;	/* B0: Protected Keep-Alive Required */

	MakeOutgoingFrame(buf, &total_len,
			1, &ie,
			1, &ie_len,
			2, &max_idle_period,
			1, &idle_opt,
			END_OF_ARGS);

	return total_len;
}

INT parse_ssid_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len <= MAX_LEN_OF_SSID;
}

INT parse_ds_parm_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 1;
}

INT parse_cf_parm_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 6;
}

INT parse_ibss_parm_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 2;
}

INT parse_qbss_load_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 5;
}

INT parse_tpc_report_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 2;
}

INT parse_ch_switch_announcement_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 3;
}

INT parse_ext_ch_switch_announcement_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 4;
}

INT parse_measurement_ie(UCHAR Len)
{
	return Len >= 3 && Len < 255;
}

INT parse_erp_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 1;
}

INT parse_overlapbss_scan_parm_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 14;
}

INT parse_rm_enable_cap_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 5;
}

INT parse_wapi_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 4;
}

INT parse_ht_info_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len >= SIZE_ADD_HT_INFO_IE;
}

INT parse_sec_ch_offset_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 1;
}

INT parse_ft_timeout_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 5;
}

INT parse_ft_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len >= 82;
}

INT parse_md_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 3;
}

INT parse_country_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len >= 3;
}

INT parse_rsn_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len >= MIN_LEN_OF_RSNIE;
}

INT parse_qos_cap_ie(EID_STRUCT *eid_ptr)
{
	return eid_ptr->Len == 1;
}

INT parse_ht_cap_ie(UCHAR Len)
{
	return Len >= SIZE_HT_CAP_IE;
}

INT parse_support_rate_ie(struct legacy_rate *rate, EID_STRUCT *eid_ptr)
{
	UCHAR i = 0;

	if (rate == NULL) {
		if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) && (eid_ptr->Len > 0))
			return TRUE;
		else
			return FALSE;
	}

	if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) && (eid_ptr->Len > 0)) {
		rate->sup_rate_len = 0;
		for (i = 0; i < eid_ptr->Len; i++)
			if (eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_HT_PHY) &&
					eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_VHT_PHY) &&
					eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY) &&
					eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_HE_PHY))
				rate->sup_rate[rate->sup_rate_len++] = eid_ptr->Octet[i];
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s - IE_SUPP_RATES., Len=%d. Rates[0]=%x\n",
				 __func__, eid_ptr->Len, rate->sup_rate[0]);
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"Rates[1]=%x %x %x %x %x %x %x\n",
				 rate->sup_rate[1], rate->sup_rate[2],
				 rate->sup_rate[3], rate->sup_rate[4],
				 rate->sup_rate[5], rate->sup_rate[6],
				 rate->sup_rate[7]);
	} else {
		UCHAR RateDefault[8] = { 0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c };
		/* HT rate not ready yet. return true temporarily. rt2860c */
		NdisMoveMemory(rate->sup_rate, RateDefault, 8);
		rate->sup_rate_len = 8;
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s - wrong IE_SUPP_RATES., Len=%d\n",
				 __func__, eid_ptr->Len);
		return FALSE;
	}

	return TRUE;
}

INT parse_support_ext_rate_ie(struct legacy_rate *rate, EID_STRUCT *eid_ptr)
{
	UINT16 i = 0;

	if (rate == NULL && eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES)
		return TRUE;

	if (eid_ptr->Len > MAX_LEN_OF_SUPPORTED_RATES) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ext support rate ie size(%d) is large than MAX_LEN_OF_SUPPORTED_RATE(%d))\n",
			eid_ptr->Len, MAX_LEN_OF_SUPPORTED_RATES);
		return FALSE;
	}

	rate->ext_rate_len = 0;
	for (i = 0; i < eid_ptr->Len; i++)
		if (eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_HT_PHY) &&
			eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_VHT_PHY) &&
			eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY) &&
			eid_ptr->Octet[i] != (BSS_MEMBERSHIP_SELECTOR_VALID | BSS_MEMBERSHIP_SELECTOR_HE_PHY))
			rate->ext_rate[rate->ext_rate_len++] = eid_ptr->Octet[i];

	return TRUE;
}
/*API Start: for OOB Task*/
UINT parse_vht_cap_ie(UCHAR eid_len)
{
	if (eid_len == SIZE_OF_VHT_CAP_IE)
		return TRUE;
	else
		return FALSE;
}

UINT parse_vht_op_ie(UCHAR eid_len)
{
	if (eid_len == SIZE_OF_VHT_OP_IE)
		return TRUE;
	else
		return FALSE;
}

UINT parse_wide_bw_channel_switch_ie(UCHAR eid_len)
{
	if (eid_len == 3)
		return TRUE;
	else
		return FALSE;
}

UINT parse_transmit_power_envelope_ie(UCHAR power_cnt, UCHAR eid_len)
{
	if (((power_cnt == 0) && (eid_len == 2))
		|| ((power_cnt == 1) && (eid_len == 3))
		|| ((power_cnt == 2) && (eid_len == 4))
		|| ((power_cnt == 3) && (eid_len == 5))
		|| ((power_cnt == 4) && (eid_len == 9)))
		return TRUE;
	else
		return FALSE;
}

UINT parse_channel_switch_wrapper_ie(PEID_STRUCT pEid, UCHAR eid_len)
{

/*Channel Switch Wrapper element:
EID(1) + Length(1)
+ New Country Subelement(variable {1(id) + 1(len) + 3(country)/3(country) + 3 * N})
+ Wide Bandwidth Channel Switch element(variable{1(id) + 1(len) + 3})
+ New Transmit Power Envelope subelement(variable{1(id) + 1(len) + 2~5})*/
	UCHAR Country_id = 7;
	UCHAR Wide_bw_chl_sw_id = 194;
	UCHAR tx_power_enve_id = 195;
	INT8 ch_sw_wrp_len = pEid->Len;
	UCHAR *subelement = &pEid->Octet[0];
	UCHAR sub_len = 0, total_len = 0, offset = 0, pwr_cnt = 0;

	/* parse the IE follow the sequence definitions in 802.11
	9.4.2.162 Channel Switch Wrapper element*/
	/*Step 1: check New Country subelement */
	if (*subelement == Country_id) {
		total_len += 2;/*Country ID + len*/
		offset += 1;
		sub_len = *(subelement + offset);
		if (sub_len % 2 != 0)/*shall evenly divisible by 2.*/
			return FALSE;
		total_len += 3;/*country string 3 bytes*/
		sub_len -= 3;
		offset += 4;/*skip country string*/
		/*after country string, may be 3 bytes Operating Triplet*/
		/*Subband Triplet fields check start*/
		while (*(subelement + offset) >= 201) {/*Operating Triplet detected.*/
			if (sub_len <= 1)
				break;
			offset += 3;
			total_len += 3;
			sub_len -= 3;
		}
		if (sub_len != 0) {
			offset += 1;/*skip padding byte*/
			total_len += 1;/*padding byte*/
		}
		subelement += offset;
	}
	/*Step 2: check Wide Bandwidth Channel Switch subelement*/
	if (*subelement == Wide_bw_chl_sw_id) {
		sub_len = *(subelement + 1);
		if (parse_wide_bw_channel_switch_ie(sub_len) == FALSE)
			return FALSE;
		total_len += 5;/*Country ID + len + 3Bytes.*/
		offset += 5;
		subelement += offset;
	}
	/*Step 3: check New Transmit Power Envelope subelement*/
	if (*subelement == tx_power_enve_id) {
		total_len += 2;/*ID + len*/
		offset += 1;/*point to len*/
		sub_len = *(subelement + offset);
		offset += 1;/*point to Transmit Power Information*/
		pwr_cnt = *(subelement + offset);
		total_len += 1;
		if (parse_transmit_power_envelope_ie(pwr_cnt, sub_len) == FALSE)
			return FALSE;
		switch (pwr_cnt) {
		case 3:
			total_len += 4;
			break;
		case 2:
			total_len += 3;
			break;
		case 1:
			total_len += 2;
			break;
		case 0:
		default:
			total_len += 1;
			break;
		}

	}
	if (total_len == ch_sw_wrp_len)
		return TRUE;
	else
		return FALSE;
}
UINT parse_operating_mode_notification_ie(UCHAR eid_len)
{
	if (eid_len == 1)
		return TRUE;
	else
		return FALSE;
}
/*
pEid: pointer to RNR IE's ID
*/
UINT parse_reduced_neighbor_report_ie(PEID_STRUCT pEid)
{
	/*EID(1) + Len(1) + Neighbor AP Information Fields(Variable, one or more)*/
	/*Neighbor AP Information Fields:*/
	/*TBTT Information Header(2) + Operating Class(1) + Channel Number(1)
	+ TBTT Information Set(variable)*/
	/*TBTT Information Set: COUNT decided by TBTT Information Count field*/
	/*Neighbor AP TBTT Offset(1) + BSSID (optional,0/6) + Short SSID (optional,0/4)*/
	UCHAR rnr_len = 0, calculate_len = 0;
	UCHAR *prnr =  NULL;
	struct GNU_PACKED neighbor_ap_info * ptbtt_info_hdr = NULL;
	UCHAR tbtt_info_cnt = 0;
	UCHAR tbtt_info_len = 0;

	if (pEid == NULL)
		return FALSE;
	rnr_len = pEid->Len;
	if (rnr_len < 4)/*Header + OP + ChlNum*/
		return FALSE;
	prnr = (UCHAR *)&pEid->Octet[0];
	while (rnr_len > 0) {
		ptbtt_info_hdr = (struct GNU_PACKED neighbor_ap_info *)prnr;
		calculate_len += 4;
		tbtt_info_cnt =
			(ptbtt_info_hdr->tbtt_info_hdr & DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_COUNT_MASK) >> DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_COUNT_SHIFT;
		tbtt_info_len =
			(ptbtt_info_hdr->tbtt_info_hdr & DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN_MASK) >> DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN_SHIFT;
		tbtt_info_cnt &= 0xF;
/*The TBTT Information Count subfield contains the number of TBTT Information fields*/
/*included in the TBTT Information Set field of the Neighbor AP Information field,minus one.*/
		if (tbtt_info_len == 1)
			calculate_len += 1*(tbtt_info_cnt - 1);
		else if (tbtt_info_len == 5)
			calculate_len += 5*(tbtt_info_cnt - 1);
		else if (tbtt_info_len == 7)
			calculate_len += 7*(tbtt_info_cnt - 1);
		else if (tbtt_info_len == 11)
			calculate_len += 11*(tbtt_info_cnt - 1);
		else {
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s,Len valid\n", __func__);
			break;
		}
		rnr_len -= calculate_len;
		ptbtt_info_hdr += calculate_len;/*shift the pointer*/
	}
	if (rnr_len == 0)/*value of len field match the payload bytes count*/
		return TRUE;
	else
		return FALSE;
}

UINT parse_fine_timing_measu_para_ie(UCHAR ftm_len)
{
	if (ftm_len == 9)
		return TRUE;
	else
		return FALSE;
}
UINT parse_twt_ie(struct itwt_ie *twt_ie)
{
	UCHAR twt_setup_cmd = 0;
	UCHAR ndp_exit = 0;
	UCHAR nego_type = 0, btwt_last = 0, reqType_next = 0;
	UCHAR twt_len_field = 0;
	UCHAR *pbtwt_para = NULL;

	if (twt_ie == NULL)
		return FALSE;
	nego_type = GET_TWT_CTRL_NEGO_TYPE(twt_ie->control);
	twt_len_field = twt_ie->len;
	if ((nego_type == TWT_CTRL_NEGO_TYPE_ITWT)
		|| (nego_type == TWT_CTRL_NEGO_TYPE_NEXT_WAKE_TBTT)) {
	/*individually TWT IE:*/
	/*EID(1) + Len(1) + Control(1) + Req_Type(2) + TWT(0/8)
	+ TWT Wake Duration(1) + TWT Mantissa(2) + TWT Channel(1) + NDP(0/4)*/
		twt_len_field -= 7;
		ndp_exit = GET_TWT_CTRL_NDP_PAGING_INDICATOR(twt_ie->control);
		if (ndp_exit)
			twt_len_field -= 4;
		twt_setup_cmd = GET_TWT_RT_SETUP_CMD(twt_ie->req_type);
		twt_len_field -= 8;/*TWT Field*/
	} else {
	/*broadcast TWT IE:*/
	/*EID(1) + Len(1) + Control(1) + {Req_Type(2) + TWT(2)
	+ TWT Wake Duration(1) + TWT Mantissa(2) + Broadcast TWT Info(2)}*N*/
	/*last broadcast parameter set indicate the last one*/
		twt_len_field -= 1;/*Control*/
		pbtwt_para = (UCHAR *)twt_ie + 3;/*point to req_type*/
		do {
			twt_len_field -= 9;
			if (twt_len_field <= 0)/*avoid attack*/
				break;
			reqType_next = *(pbtwt_para);
			btwt_last = GET_TWT_RT_IMPLICIT_LAST(reqType_next);
			pbtwt_para += 9;
		} while (!btwt_last);
	}
	if (twt_len_field == 0)
		return TRUE;
	else
		return FALSE;
}
UINT parse_fils_indication_ie(PEID_STRUCT pEid)
{
	/*EID(1) + Len(1) + FILS Info(2) + Cache Identifier(0/2)
	+ HESSID(0/6) + Realm Identifier(variable) + Public Key Identifier(variable)*/
	/*Realm Identifier: 2*(FILS_Info[B5:B3])*/
	/*Public Key Identifier: (2 + m)*(FILS_Info[B2:B0])*/
	/*Public Key Indicator:m octets*/
	UCHAR ri_num = 0, pki_num = 0, Cacheid = 0, hessid = 0;
	UINT16 *fils_info = 0;
	UCHAR fils_ie_len = 0, chk_len = 0;
	UCHAR *pPki = NULL;

	if (pEid == NULL)
		return FALSE;
	if (pEid->Len < 2)
		return FALSE;
	fils_ie_len = pEid->Len;
	fils_info = (UINT16 *)&pEid->Octet[0];
	chk_len += 2;
	ri_num = *fils_info & 0x38;
	pki_num = *fils_info & 0x7;
	Cacheid = *fils_info & BIT(7);
	if (Cacheid)
		chk_len += 2;
	hessid = *fils_info & BIT(8);
	if (hessid)
		chk_len += 6;
	chk_len += 2*ri_num;
	pPki = ((UCHAR *)&pEid->Octet[0] + chk_len);
	while (pki_num) {
		chk_len += (2 + *(pPki + 1));/* length of Public Key Identifier field*/
		pPki += (2 + *(pPki + 1));
		pki_num--;
	}
	if (chk_len == fils_ie_len)
		return fils_ie_len;
	else
		return FALSE;
}
UINT parse_rsn_ext_ie(PEID_STRUCT pEid)
{
	/*EID(1) + Len(1) + Extended RSN Capabilities(n)*/
	/*The length of the Extended RSNCapabilities field is a variable n,
	in octets, as indicated by the first 4 bits in the fiel*/
	UCHAR rsn_ext_len = 0;
	UCHAR rsn_indicated_len = 0;

	if (pEid == NULL)
		return FALSE;
	rsn_ext_len = pEid->Len;
	rsn_indicated_len = pEid->Octet[0] & 0xF;
	if (rsn_ext_len == rsn_indicated_len)
		return rsn_ext_len;
	else
		return FALSE;
}
UINT parse_fils_request_para_ie(PEID_STRUCT pEid)
{
	/*EID(1) + Len(1) + EID-EXT(1) + PCB(1) + MaxChT(1)
	+ FILS_Cri(0/1) + Max Delay(0/1) + Minimum DateRate(0/3)
	+ RCPI Limit(0/1) + OUI RSP(0/2)*/
	UCHAR fils_criteria = 0, max_dealy = 0, min_dr = 0;
	UCHAR rcpi_limit = 0, oui_rsp_criteria = 0, pcb = 0;
	UCHAR fils_indic_ie_len = 0;

	if (pEid == NULL)
		return FALSE;
	fils_indic_ie_len = pEid->Len;
	fils_indic_ie_len -= 3;/*minux EID-EXT,PCB,MaxCht 3 octets*/
	pcb = *((UCHAR *)&pEid->Octet[0] + 1);/*skip EID-EXT*/
	fils_criteria = pcb & BIT(0);
	max_dealy = pcb & BIT(1);
	min_dr = pcb & BIT(2);
	rcpi_limit = pcb & BIT(3);
	oui_rsp_criteria = pcb & BIT(4);
	if (fils_criteria)
		fils_indic_ie_len -= 1;
	if (max_dealy)
		fils_indic_ie_len -= 1;
	if (min_dr)
		fils_indic_ie_len -= 3;
	if (rcpi_limit)
		fils_indic_ie_len -= 1;
	if (oui_rsp_criteria)
		fils_indic_ie_len -= 2;
	if (fils_indic_ie_len == 0)
		return pEid->Len;
	else
		return FALSE;
}


/*ie_len: length field - 1(EID-EXT Field)*/
UINT parse_mu_edca_ie(UCHAR ie_len)
{
	if (ie_len == 13)
		return TRUE;
	else
		return FALSE;
}

/*pie: pointer to SR Control field
ie_len: from SR control field to end of IE. not include EID-EXT
*/
UINT parse_spatial_resue_ie(UCHAR *pie, UCHAR ie_len)
{
	UCHAR parse_len = 0;
	UCHAR sr_ctl = 0;

	if (pie == NULL)
		return FALSE;
	sr_ctl = *pie;
	parse_len += 1;/*SR Control*/
	if (sr_ctl & BIT(2))/*Non-SRG Offset Present*/
		parse_len += 1;/*Non-SRG OBSS PD Max offset*/
	if (sr_ctl & BIT(3))/*SRG Information Present*/
		parse_len += (1 + 1 + 8 + 8);
	/*SRG OBSS PD Min Offset + SRG OBSS PD Max Offset +
	SRG BSS Color Bitmap + SRG Partial BSSID Bitmap*/
	if (parse_len == ie_len)
		return TRUE;
	else
		return FALSE;
}

/*pEid: pointer to EID
  pie: pointer to EID-EXT
*/
UINT parse_short_ssid_list_ie(PEID_STRUCT pEid, UCHAR *pie)
{
/*EID(1) + Len(1) + EID-EXT(1) + Short SSID List(variable)*/
/*The Short SSID List field contains one or more four octet Short SSID field*/
	UCHAR ie_len = 0;

	if (pie == NULL)
		return FALSE;
	ie_len = *(pie - 1);
	if (((ie_len - 1) % 4) != 0)
		return FALSE;
	else
		return TRUE;
}
/*API End: for OOB Task*/

