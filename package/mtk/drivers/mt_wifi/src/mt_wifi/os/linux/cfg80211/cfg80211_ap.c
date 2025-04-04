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
 *
 *	Abstract:
 *
 *	All related CFG80211 function body.
 *
 *	History:
 *
 ***************************************************************************/
#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT
#ifdef CONFIG_AP_SUPPORT

#include "rt_config.h"

#ifdef MT_MAC
#endif /* MT_MAC */

INT CFG80211_FindMbssApIdxByNetDevice(RTMP_ADAPTER *pAd, PNET_DEV pNetDev)
{
	USHORT index = 0;
	BOOLEAN found = FALSE;

	for (index = 0; index < MAX_MBSSID_NUM(pAd); index++) {
		if (pAd->ApCfg.MBSSID[index].wdev.if_dev == pNetDev) {
			found = TRUE;
			break;
		}
#ifdef CONFIG_VLAN_GTK_SUPPORT
		else if (CFG80211_MatchVlandev(&pAd->ApCfg.MBSSID[index].wdev, pNetDev)) {
			found = TRUE;
			break;
		}
#endif
	}

	return (found) ? index : WDEV_NOT_FOUND;
}

/*shailesh : commented out CfgAsicSetPreTbtt function as it is not used */
static INT CFG80211DRV_UpdateTimIE(PRTMP_ADAPTER pAd, UINT mbss_idx, PUCHAR pBeaconFrame, UINT32 tim_ie_pos)
{
	UCHAR  ID_1B, TimFirst, TimLast, *pTim, *ptr, New_Tim_Len;
	UINT  i;
	struct wifi_dev *wdev = NULL;
	BCN_BUF_STRUCT *bcn_buf = NULL;

	ptr = pBeaconFrame + tim_ie_pos; /* TIM LOCATION */
	*ptr = IE_TIM;
	*(ptr + 2) = pAd->ApCfg.DtimCount;
	*(ptr + 3) = pAd->ApCfg.DtimPeriod;
	TimFirst = 0; /* record first TIM byte != 0x00 */
	TimLast = 0;  /* record last  TIM byte != 0x00 */
	wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
	bcn_buf = &wdev->bcn_buf;
	pTim = bcn_buf->TimBitmaps;

	for (ID_1B = 0; ID_1B < WLAN_MAX_NUM_OF_TIM; ID_1B++) {
		/* get the TIM indicating PS packets for 8 stations */
		UCHAR tim_1B = pTim[ID_1B];

		if (ID_1B == 0)
			tim_1B &= 0xfe; /* skip bit0 bc/mc */

		if (tim_1B == 0)
			continue; /* find next 1B */

		if (TimFirst == 0)
			TimFirst = ID_1B;

		TimLast = ID_1B;
	}

	/* fill TIM content to beacon buffer */
	if (TimFirst & 0x01)
		TimFirst--; /* find the even offset byte */

	*(ptr + 1) = 3 + (TimLast - TimFirst + 1); /* TIM IE length */
	*(ptr + 4) = TimFirst;

	for (i = TimFirst; i <= TimLast; i++)
		*(ptr + 5 + i - TimFirst) = pTim[i];

	/* bit0 means backlogged mcast/bcast */
	if (pAd->ApCfg.DtimCount == 0)
		*(ptr + 4) |= (bcn_buf->TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] & 0x01);

	/* adjust BEACON length according to the new TIM */
	New_Tim_Len = (2 + *(ptr + 1));
	return New_Tim_Len;
}

static INT CFG80211DRV_UpdateApSettingFromBeacon(PRTMP_ADAPTER pAd, UINT mbss_idx, CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[mbss_idx];
	struct wifi_dev *wdev = &pMbss->wdev;
	const UCHAR *ssid_ie = NULL, *wpa_ie = NULL, *rsn_ie = NULL;
#ifdef HOSTAPD_WPA3R3_SUPPORT
	const UCHAR *rsnxe_ie = NULL;
#endif
	const UCHAR *supp_rates_ie = NULL;
	const UCHAR *ext_supp_rates_ie = NULL, *ht_cap = NULL, *ht_info = NULL;
	UINT16 radio_measurement = 0x00;
	UCHAR BandIdx = HcGetBandByWdev(wdev);

#ifdef CONFIG_AP_SUPPORT
    INT idx;
#endif

#ifndef DISABLE_HOSTAPD_BEACON
#ifdef HOSTAPD_AUTO_CH_SUPPORT
	const UCHAR *dsparam_ie = NULL, *ht_operation = NULL, *vht_operation = NULL;
	PADD_HT_INFO_IE phtinfo;
	VHT_OP_IE	*vhtinfo;
	UCHAR channel = 0;
	PEID_STRUCT pEid;
#endif
#endif

#ifdef DISABLE_HOSTAPD_BEACON
#ifdef WSC_AP_SUPPORT
	const UCHAR *wsc_ie = NULL;
	const UINT WFA_OUI = 0x0050F2;
#endif
#endif

#ifdef HOSTAPD_11R_SUPPORT
	const UCHAR *md_ie = NULL;
#endif /* HOSTAPD_11R_SUPPORT */
#if defined(HOSTAPD_OWE_SUPPORT) || defined(HOSTAPD_HS_R2_SUPPORT)
	const UINT OUI_WFA = 0x506f9a;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
	const UCHAR *trans_ie = NULL;
	UINT8 OWE_OUI_TYPE = 28;
#endif
#ifdef HOSTAPD_HS_R2_SUPPORT
	UINT8 HS2_OUI_TYPE = 16;
	UINT8 P2P_OUI_TYPE = 9;
	#define HS20_OSEN_OUI_TYPE  18
	const UCHAR *hs2_indication_ie = NULL;
	const UCHAR *p2p_ie = NULL;
	const UCHAR *interworking_ie = NULL;
	const UCHAR *adv_proto_ie = NULL;
	const UCHAR *roam_consort_ie = NULL;
	const UCHAR *hs2_osen_ie = NULL;
	PGAS_CTRL	pGasCtrl = &pMbss->GASCtrl;
	INT32 Ret;
	PUCHAR tmp_buf_ptr = NULL;
#endif
#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
	PHOTSPOT_CTRL pHSCtrl = &pMbss->HotSpotCtrl;
	const UCHAR *ext_cap_ie = NULL;
#endif
#ifdef HOSTAPD_11K_SUPPORT
	const UCHAR *rrm_caps = NULL;
#endif

#if (KERNEL_VERSION(3, 5, 0) <= LINUX_VERSION_CODE)
	const UCHAR CFG_HT_OP_EID = WLAN_EID_HT_OPERATION;
#else
	const UCHAR CFG_HT_OP_EID = WLAN_EID_HT_INFORMATION;
#endif /* LINUX_VERSION_CODE: 3.5.0 */
#ifdef HOSTAPD_11K_SUPPORT
	const UCHAR CFG_RRM_OP_EID = WLAN_EID_RRM_ENABLED_CAPABILITIES;
#endif

#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	const UCHAR CFG_WPA_EID = WLAN_EID_VENDOR_SPECIFIC;
#else
	const UCHAR CFG_WPA_EID = WLAN_EID_WPA;
#endif /* LINUX_VERSION_CODE: 3.8.0 */

#ifndef DISABLE_HOSTAPD_BEACON
#ifdef HOSTAPD_AUTO_CH_SUPPORT
	if (WMODE_CAP_2G(wdev->PhyMode))
		channel = HcGetChannelByRf(pAd, RFIC_24GHZ);
	else
		channel = HcGetChannelByRf(pAd, RFIC_5GHZ);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Channel from Auto selection is :%d\n", channel);
#endif
#endif

	ssid_ie = cfg80211_find_ie(WLAN_EID_SSID, pBeacon->beacon_head + 36, pBeacon->beacon_head_len - 36);
	supp_rates_ie = cfg80211_find_ie(WLAN_EID_SUPP_RATES, pBeacon->beacon_head + 36, pBeacon->beacon_head_len - 36);
	/* if it doesn't find WPA_IE in tail first 30 bytes. treat it as is not found */
	wpa_ie = cfg80211_find_ie(CFG_WPA_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	rsn_ie = cfg80211_find_ie(WLAN_EID_RSN, pBeacon->beacon_tail, pBeacon->beacon_tail_len);/* wpa2 case. */
	ext_supp_rates_ie = cfg80211_find_ie(WLAN_EID_EXT_SUPP_RATES, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	ht_cap = cfg80211_find_ie(WLAN_EID_HT_CAPABILITY, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	ht_info = cfg80211_find_ie(CFG_HT_OP_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
#ifdef HOSTAPD_WPA3R3_SUPPORT
	/*find	RSNXE IE in hostapd beacon tail*/
	rsnxe_ie = cfg80211_find_ie(IE_RSNXE, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
#endif

#ifdef HOSTAPD_11K_SUPPORT
	rrm_caps = cfg80211_find_ie(CFG_RRM_OP_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
#endif
#ifdef HOSTAPD_11R_SUPPORT
	md_ie = cfg80211_find_ie(WLAN_EID_MOBILITY_DOMAIN, pBeacon->beacon_tail, pBeacon->beacon_tail_len); /* WLAN_EID_MOBILITY_DOMAIN=54 */
#endif
#ifndef DISABLE_HOSTAPD_BEACON
#ifdef HOSTAPD_AUTO_CH_SUPPORT
	dsparam_ie = cfg80211_find_ie(WLAN_EID_DS_PARAMS, pBeacon->beacon_head+36, pBeacon->beacon_head_len-36);
	ht_operation = cfg80211_find_ie(WLAN_EID_HT_OPERATION, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	vht_operation = cfg80211_find_ie(WLAN_EID_VHT_OPERATION, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
#endif
#endif

#ifndef HOSTAPD_11K_SUPPORT
	radio_measurement = 0x1000;
#endif

#ifdef HOSTAPD_11K_SUPPORT
	if (rrm_caps != NULL) {

		EID_STRUCT *eid;
		INT loop;
		UINT8 bit_nr, bit_lci;
		PRRM_CONFIG pRrmCfg;

		eid = (EID_STRUCT *)rrm_caps;

		printk("RRM : len %d eid %d octet %d\n", eid->Len, eid->Eid, eid->Octet[0]);
		pRrmCfg = &pAd->ApCfg.MBSSID[loop].RrmCfg;

		pMbss->RrmCfg.bDot11kRRMEnable = 1;
		pMbss->RrmCfg.bDot11kRRMEnableSet = 1;
		radio_measurement = 0x1000;
		pMbss->CapabilityInfo |= RRM_CAP_BIT;

		bit_nr = (eid->Octet[0] >> 1) & 1; /*checking bit position 1:neighbor report */
		bit_lci = (eid->Octet[1] >> 4) & 1; /*checking LCI */

		printk("bit_nr bit_lci: %d %d\n", bit_nr, bit_lci);

		if (bit_nr)
			pRrmCfg->hstapd_nei_rep = TRUE;

		if (bit_lci)
			pRrmCfg->hstapd_lci = TRUE;

		for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
			pRrmCfg->max_rrm_capabilities.word = 0;
			pRrmCfg->max_rrm_capabilities.field.NeighborRepCap = 1;
			pRrmCfg->rrm_capabilities.word = pRrmCfg->max_rrm_capabilities.word;
		}

		if (pRrmCfg->hstapd_nei_rep)
			pRrmCfg->hstapd_nei_rep = FALSE;

		if (pRrmCfg->hstapd_lci)
			pRrmCfg->hstapd_lci = FALSE;
		} else
			printk("No RRM capabilities enabled from hostapd\n");
#endif

#ifdef HOSTAPD_OWE_SUPPORT
		/*owe trans oui */
	trans_ie = (UCHAR *)cfg80211_find_vendor_ie(OUI_WFA, OWE_OUI_TYPE,
			pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	if (trans_ie != NULL) {
		EID_STRUCT *eid;

		eid = (EID_STRUCT *)trans_ie;
		if (eid->Len + 2 <= MAX_LEN_OF_TRANS_IE) {
			NdisCopyMemory(pMbss->TRANS_IE, trans_ie, eid->Len+2);
			pMbss->TRANSIE_Len = eid->Len + 2;
		}
	}
#endif
#ifdef HOSTAPD_HS_R2_SUPPORT
	hs2_indication_ie = (UCHAR *)cfg80211_find_vendor_ie(OUI_WFA, HS2_OUI_TYPE, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	if (hs2_indication_ie != NULL) {
		EID_STRUCT *eid = (EID_STRUCT *)hs2_indication_ie;

		Ret = os_alloc_mem(NULL, &tmp_buf_ptr, eid->Len + 2);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Not enough memory\n");
			return FALSE;
		}
		NdisMoveMemory(tmp_buf_ptr, hs2_indication_ie, eid->Len + 2);

		RTMP_SEM_LOCK(&pHSCtrl->IeLock);
		if (pHSCtrl->HSIndicationIE)
			os_free_mem(pHSCtrl->HSIndicationIE);
		pHSCtrl->HSIndicationIE = tmp_buf_ptr;
		pHSCtrl->HSIndicationIELen = eid->Len + 2;
		pHSCtrl->HotSpotEnable = 1;
		hotspot_update_bssflag(pAd, fgHotspotEnable, 1, pHSCtrl);
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
	} else {
		pHSCtrl->HotSpotEnable = 0;
		hotspot_update_bssflag(pAd, fgHotspotEnable, 0, pHSCtrl);
	}

	p2p_ie = (UCHAR *)cfg80211_find_vendor_ie(OUI_WFA, P2P_OUI_TYPE, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	if (p2p_ie != NULL) {
		EID_STRUCT *eid = (EID_STRUCT *)p2p_ie;

		Ret = os_alloc_mem(NULL, &tmp_buf_ptr, eid->Len + 2);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Not enough memory\n");
			return FALSE;
		}
		NdisMoveMemory(tmp_buf_ptr, p2p_ie, eid->Len + 2);

		RTMP_SEM_LOCK(&pHSCtrl->IeLock);
		if (pHSCtrl->P2PIE)
			os_free_mem(pHSCtrl->P2PIE);
		pHSCtrl->P2PIE = tmp_buf_ptr;
		pHSCtrl->P2PIELen = eid->Len + 2;
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
	}

	interworking_ie = cfg80211_find_ie(IE_INTERWORKING, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	if (interworking_ie != NULL) {
		EID_STRUCT *eid = (EID_STRUCT *)interworking_ie;

		Ret = os_alloc_mem(NULL, &tmp_buf_ptr, eid->Len + 2);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Not enough memory\n");
			return FALSE;
		}
		NdisMoveMemory(tmp_buf_ptr, interworking_ie, eid->Len + 2);

		RTMP_SEM_LOCK(&pGasCtrl->IeLock);
		if (pGasCtrl->InterWorkingIE)
			os_free_mem(pGasCtrl->InterWorkingIE);
		pGasCtrl->InterWorkingIE = tmp_buf_ptr;
		pGasCtrl->InterWorkingIELen = eid->Len + 2;
		RTMP_SEM_UNLOCK(&pGasCtrl->IeLock);
	}
	adv_proto_ie = cfg80211_find_ie(IE_ADVERTISEMENT_PROTO, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	if (adv_proto_ie != NULL) {
		EID_STRUCT *eid = (EID_STRUCT *)adv_proto_ie;

		Ret = os_alloc_mem(NULL, &tmp_buf_ptr, eid->Len + 2);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Not enough memory\n");
			return FALSE;
		}
		NdisMoveMemory(tmp_buf_ptr, adv_proto_ie, eid->Len + 2);

		RTMP_SEM_LOCK(&pGasCtrl->IeLock);
		if (pGasCtrl->AdvertisementProtoIE)
			os_free_mem(pGasCtrl->AdvertisementProtoIE);
		pGasCtrl->AdvertisementProtoIE = tmp_buf_ptr;
		pGasCtrl->AdvertisementProtoIELen = eid->Len + 2;
		RTMP_SEM_UNLOCK(&pGasCtrl->IeLock);
	}
	roam_consort_ie = cfg80211_find_ie(IE_ROAMING_CONSORTIUM, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	if (roam_consort_ie != NULL) {
		EID_STRUCT *eid = (EID_STRUCT *)roam_consort_ie;

		Ret = os_alloc_mem(NULL, &tmp_buf_ptr, eid->Len + 2);
		if (Ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Not enough memory\n");
			return FALSE;
		}
		NdisMoveMemory(tmp_buf_ptr, roam_consort_ie, eid->Len + 2);

		RTMP_SEM_LOCK(&pHSCtrl->IeLock);
		if (pHSCtrl->RoamingConsortiumIE)
			os_free_mem(pHSCtrl->RoamingConsortiumIE);
		pHSCtrl->RoamingConsortiumIE = tmp_buf_ptr;
		pHSCtrl->RoamingConsortiumIELen = eid->Len + 2;
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
	}
	hs2_osen_ie = (UCHAR *)cfg80211_find_vendor_ie(OUI_WFA, HS20_OSEN_OUI_TYPE, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	if (hs2_osen_ie != NULL)
		hotspot_update_bssflag(pAd, fgASANEnable, 1, pHSCtrl);
	else
		hotspot_update_bssflag(pAd, fgASANEnable, 0, pHSCtrl);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Updated hotspot flag %d for MBSS %d bssIndex %d to CR4\n",
				pHSCtrl->HotspotBSSFlags, mbss_idx, wdev->bss_info_argument.ucBssIndex);
#endif
#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)

	ext_cap_ie = cfg80211_find_ie(IE_EXT_CAPABILITY, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	if (ext_cap_ie != NULL) {
		EXT_CAP_INFO_ELEMENT ext_cap;
		EID_STRUCT *eid = (EID_STRUCT *)ext_cap_ie;
		if (eid->Len < sizeof(EXT_CAP_INFO_ELEMENT))
			NdisMoveMemory(&ext_cap, &eid->Octet[0], eid->Len);
		else
			NdisMoveMemory(&ext_cap, &eid->Octet[0], sizeof(EXT_CAP_INFO_ELEMENT));
#ifdef HOSTAPD_HS_R2_SUPPORT
		if (ext_cap.wnm_notification)
			pMbss->WNMCtrl.WNMNotifyEnable = 1;
		else
			pMbss->WNMCtrl.WNMNotifyEnable = 0;
#endif
		if (ext_cap.proxy_arp) {
			pMbss->WNMCtrl.ProxyARPEnable = 1;
			hotspot_update_bssflag(pAd, fgProxyArpEnable, 1, pHSCtrl);
		} else {
			pMbss->WNMCtrl.ProxyARPEnable = 0;
			hotspot_update_bssflag(pAd, fgProxyArpEnable, 0, pHSCtrl);
		}
#ifdef HOSTAPD_HS_R2_SUPPORT
		if (ext_cap.BssTransitionManmt)
			pMbss->WNMCtrl.WNMBTMEnable = 1;
		else
			pMbss->WNMCtrl.WNMBTMEnable = 0;

		if (ext_cap.qosmap) {
			hotspot_update_bssflag(pAd, fgQosMapEnable, 1, pHSCtrl);
			pMbss->HotSpotCtrl.QosMapEnable = 1;
		} else {
			pMbss->HotSpotCtrl.QosMapEnable = 0;
			hotspot_update_bssflag(pAd, fgQosMapEnable, 0, pHSCtrl);
		}
#endif
	}

	hotspot_update_bss_info_to_cr4(pAd, mbss_idx, wdev->bss_info_argument.ucBssIndex);
#endif

	/* SSID */

	if (ssid_ie == NULL) {
		os_move_mem(pMbss->Ssid, "CFG_Linux_GO", 12);
		pMbss->SsidLen = 12;
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CFG: SSID Not Found In Packet\n");
	} else if (pBeacon->ssid_len != 0) {
		os_zero_mem(pMbss->Ssid, MAX_LEN_OF_SSID + 1);
		NdisZeroMemory(pMbss->Ssid, MAX_LEN_OF_SSID + 1);
		pMbss->SsidLen = pBeacon->ssid_len;
		NdisCopyMemory(pMbss->Ssid, ssid_ie + 2, pMbss->SsidLen);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\nCFG : SSID: %s, %d\n", pMbss->Ssid, pMbss->SsidLen);
	}

#ifdef DISABLE_HOSTAPD_BEACON
	else if (*(ssid_ie+1) != 0) {
		os_zero_mem(pMbss->Ssid, pMbss->SsidLen);
		NdisZeroMemory(pMbss->Ssid, pMbss->SsidLen);
		pMbss->SsidLen = *(ssid_ie+1);
		NdisCopyMemory(pMbss->Ssid, ssid_ie+2, pMbss->SsidLen);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\nCFG : SSID: %s, %d\n", pMbss->Ssid, pMbss->SsidLen);
	}
#ifdef WSC_AP_SUPPORT
	wsc_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, 4, pBeacon->beacon_tail, pBeacon->beacon_tail_len);

		wdev->WscIEBeacon.ValueLen = 0;
		wdev->WscIEProbeResp.ValueLen = 0;

	if (wsc_ie != NULL) {
#ifdef HOSTAPD_MAP_SUPPORT
		if ((IS_MAP_ENABLE(pAd)) && (wdev) &&
			(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
			(!(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_FRONTHAUL_BSS)))) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							("Strictly BH BSS: %s, donot BC WPS cap\n", pMbss->Ssid));
		} else
#endif
		{
			EID_STRUCT *eid;

			eid = (EID_STRUCT *)wsc_ie;

			if (eid->Len + 2 <= 500) {
				NdisCopyMemory(wdev->WscIEBeacon.Value, wsc_ie, eid->Len+2);
				wdev->WscIEBeacon.ValueLen = eid->Len + 2;
			}
		}
	}
#endif

#ifdef HOSTAPD_11R_SUPPORT
	if (md_ie != NULL) {
		PFT_CFG pFtCfg = &pAd->ApCfg.MBSSID[mbss_idx].wdev.FtCfg;

		NdisCopyMemory(pFtCfg->FtMdId, md_ie+2, FT_MDID_LEN);
		pFtCfg->FtCapFlag.FtOverDs = (0x01)&(*(md_ie + 4));
		pFtCfg->FtCapFlag.RsrReqCap = (0x02)&(*(md_ie + 4));
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"MDID::%x %x FToverDS:%d RsrCap:%d\n",
				pFtCfg->FtMdId[0], pFtCfg->FtMdId[1],
				pFtCfg->FtCapFlag.FtOverDs, pFtCfg->FtCapFlag.RsrReqCap);
	} else
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MDIE is NULL\n");

#endif /* HOSTAPD_11R_SUPPORT */
#endif

#ifndef DISABLE_HOSTAPD_BEACON
#ifdef HOSTAPD_AUTO_CH_SUPPORT
	if (dsparam_ie != NULL) {
		pEid = (PEID_STRUCT)dsparam_ie;
		*pEid->Octet = channel;
	}

	if (ht_operation != NULL) {
		pEid = (PEID_STRUCT)ht_operation;
		phtinfo = (PADD_HT_INFO_IE)pEid->Octet;
		phtinfo->ControlChan = channel;
		phtinfo->AddHtInfo.RecomWidth = wlan_operate_get_ht_bw(&pMbss->wdev);
		/* phtinfo->AddHtInfo.ExtChanOffset = 3; */
		phtinfo->AddHtInfo.ExtChanOffset = HcGetExtCha(pAd, channel);
	}
	if (vht_operation != NULL) {
		UCHAR bw = pAd->CommonCfg.vht_bw;
		UCHAR ch_band = wlan_config_get_ch_band(wdev);
		UCHAR cent_ch = vht_cent_ch_freq(channel, bw, ch_band);
		pEid = (PEID_STRUCT)vht_operation;
		vhtinfo = (VHT_OP_IE *)pEid->Octet;

		switch (bw) {
		case  VHT_BW_2040:
			vhtinfo->vht_op_info.ch_width = 0;
			vhtinfo->vht_op_info.center_freq_1 = 0;
			vhtinfo->vht_op_info.center_freq_2 = 0;
			break;

		case VHT_BW_80:
			vhtinfo->vht_op_info.ch_width = 1;
			vhtinfo->vht_op_info.center_freq_1 = cent_ch;
			vhtinfo->vht_op_info.center_freq_2 = 0;
			break;

		case VHT_BW_160:
			vhtinfo->vht_op_info.ch_width = 2;
			vhtinfo->vht_op_info.center_freq_1 = cent_ch;
			vhtinfo->vht_op_info.center_freq_2 = pAd->CommonCfg.vht_cent_ch2;
			break;

		case VHT_BW_8080:

			vhtinfo->vht_op_info.ch_width = 3;
			vhtinfo->vht_op_info.center_freq_1 = cent_ch;
			vhtinfo->vht_op_info.center_freq_2 = pAd->CommonCfg.vht_cent_ch2;
			break;
		}
	}

#endif
#endif


#if (KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE)

	if ((pBeacon->hidden_ssid > 0 && pBeacon->hidden_ssid < 3) || (pMbss->bHideSsid)) {
		pMbss->bHideSsid = TRUE;

		if ((pBeacon->ssid_len != 0)
			&& (pBeacon->ssid_len <= MAX_LEN_OF_SSID)) {
			pMbss->SsidLen = pBeacon->ssid_len;
			NdisCopyMemory(pMbss->Ssid, pBeacon->ssid, pMbss->SsidLen);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "80211> [Hidden] SSID: %s, %d\n", pMbss->Ssid, pMbss->SsidLen);
		}
	} else
		pMbss->bHideSsid = FALSE;

#endif /* LINUX_VERSION_CODE 3.4.0 */
	/* WMM EDCA Paramter */
	CFG80211_SyncPacketWmmIe(pAd, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	pMbss->RSNIE_Len[0] = 0;
	pMbss->RSNIE_Len[1] = 0;
	NdisZeroMemory(pMbss->RSN_IE[0], MAX_LEN_OF_RSNIE);
	NdisZeroMemory(pMbss->RSN_IE[1], MAX_LEN_OF_RSNIE);
#ifdef HOSTAPD_WPA3R3_SUPPORT
	/*reset RSNXE previous value*/
	wdev->SecConfig.RSNXE_Val = 0;
#endif
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "80211> pBeacon->privacy = %d\n", pBeacon->privacy);

	if (pBeacon->privacy) {
		/* Security */
		if (pBeacon->auth_type == NL80211_AUTHTYPE_SHARED_KEY) {
			/* Shared WEP */
			/* wdev->WepStatus = Ndis802_11WEPEnabled; */
			/* wdev->AuthMode = Ndis802_11AuthModeShared; */
			CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
			CLEAR_CIPHER(wdev->SecConfig.PairwiseCipher);
			CLEAR_CIPHER(wdev->SecConfig.GroupCipher);
			SET_AKM_SHARED(wdev->SecConfig.AKMMap);
			SET_CIPHER_WEP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_WEP(wdev->SecConfig.GroupCipher);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("\nCFG80211 BEACON => AuthMode = %s ,wdev->PairwiseCipher = %s wdev->SecConfig.GroupCipher = %s\n"
					  , GetAuthModeStr(wdev->SecConfig.AKMMap), GetEncryModeStr(wdev->SecConfig.PairwiseCipher), GetEncryModeStr(wdev->SecConfig.GroupCipher)));
		}
#ifdef HOSTAPD_HS_R2_SUPPORT
		else if (hs2_osen_ie != NULL) {
			CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
			CLEAR_PAIRWISE_CIPHER(&wdev->SecConfig);
			CLEAR_GROUP_CIPHER(&wdev->SecConfig);
#ifdef DOT11W_PMF_SUPPORT
			wdev->SecConfig.PmfCfg.MFPC = 0;
			wdev->SecConfig.PmfCfg.MFPR = 0;
			wdev->SecConfig.PmfCfg.igtk_cipher = 0;
#endif
			pMbss->HotSpotCtrl.bASANEnable = 1;
			SET_AKM_WPA2(wdev->SecConfig.AKMMap);
			SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
		}
#endif
		else
			CFG80211_ParseBeaconIE(pAd, pMbss, wdev, (UCHAR *)wpa_ie, (UCHAR *)rsn_ie);
#ifdef HOSTAPD_WPA3R3_SUPPORT
		/*Set RSNXE Value from RSNXE IE of hostapd*/
		wdev->SecConfig.RSNXE_Val = 0;
		wdev->SecConfig.SaePwe = 0;
		if (rsnxe_ie != NULL) {
			CFG80211DBG(DBG_LVL_INFO, ("80211> %s RSNXE_IE %d %d %d\n",
				__func__, *rsnxe_ie, *(rsnxe_ie+1), *(rsnxe_ie+2)));

			wdev->SecConfig.RSNXE_Val = *(rsnxe_ie+2);
		}
		if (pBeacon->crypto.sae_pwe) {
			CFG80211DBG(DBG_LVL_INFO, ("80211> %s SAE_PWE = %d\n",
				__func__, pBeacon->crypto.sae_pwe));
			wdev->SecConfig.SaePwe = pBeacon->crypto.sae_pwe;
		}
#endif


		if ((IS_CIPHER_NONE(wdev->SecConfig.PairwiseCipher)) &&
			(IS_AKM_OPEN(wdev->SecConfig.AKMMap))) {
			/* WEP Auto */
			/* wdev->WepStatus = Ndis802_11WEPEnabled; */
			/* wdev->AuthMode = Ndis802_11AuthModeAutoSwitch; */
			CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
			CLEAR_CIPHER(wdev->SecConfig.PairwiseCipher);
			CLEAR_CIPHER(wdev->SecConfig.GroupCipher);
			SET_AKM_OPEN(wdev->SecConfig.AKMMap);
			SET_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap);
			SET_CIPHER_WEP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_WEP(wdev->SecConfig.GroupCipher);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("\nCFG80211 BEACON => AuthMode = %s ,wdev->PairwiseCipher = %s wdev->SecConfig.GroupCipher = %s\n"
					  , GetAuthModeStr(wdev->SecConfig.AKMMap), GetEncryModeStr(wdev->SecConfig.PairwiseCipher), GetEncryModeStr(wdev->SecConfig.GroupCipher)));
		}

	}
#ifdef HOSTAPD_HS_R3_SUPPORT
	else if (hs2_osen_ie != NULL) {
		CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
		CLEAR_PAIRWISE_CIPHER(&wdev->SecConfig);
		CLEAR_GROUP_CIPHER(&wdev->SecConfig);
#ifdef DOT11W_PMF_SUPPORT
		wdev->SecConfig.PmfCfg.MFPC = 0;
		wdev->SecConfig.PmfCfg.MFPR = 0;
		wdev->SecConfig.PmfCfg.igtk_cipher = 0;
#endif
		pMbss->HotSpotCtrl.bASANEnable = 1;
		SET_AKM_WPA2(wdev->SecConfig.AKMMap);
		SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
		SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
	}
#endif
	else {
		/* wdev->WepStatus = Ndis802_11EncryptionDisabled; */
		/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		CFG80211_ParseBeaconIE(pAd, pMbss, wdev, (UCHAR *)wpa_ie, (UCHAR *)rsn_ie);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			RTMPSetIndividualHT(pAd, idx);
	}
#endif /* CONFIG_AP_SUPPORT */

	pMbss->CapabilityInfo =	CAP_GENERATE(1, 0, (!IS_CIPHER_NONE(wdev->SecConfig.PairwiseCipher)),
					(pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1),
					pAd->CommonCfg.bUseShortSlotTime[BandIdx], /*SpectrumMgmt*/FALSE);
#ifdef DOT11K_RRM_SUPPORT
	if (IS_RRM_ENABLE(wdev))
		pMbss->CapabilityInfo |= RRM_CAP_BIT;
#endif /* DOT11K_RRM_SUPPORT */

	/* Disable Driver-Internal Rekey */
	pMbss->WPAREKEY.ReKeyInterval = 0;
	pMbss->WPAREKEY.ReKeyMethod = DISABLE_REKEY;

#ifndef DISABLE_HOSTAPD_BEACON
    if (pBeacon->interval != 0) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG_TIM New BI %d\n", pBeacon->interval);
		pAd->CommonCfg.BeaconPeriod = pBeacon->interval;
	}

	if (pBeacon->dtim_period != 0) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG_TIM New DP %d\n", pBeacon->dtim_period);
		pAd->ApCfg.DtimPeriod = pBeacon->dtim_period;
	}
#endif

#ifdef CONFIG_6G_SUPPORT
	bssmnger_dereg_bmg_entry(wdev);
	bssmnger_reg_bmg_entry(wdev);
#endif

	return TRUE;
}

VOID CFG80211DRV_DisableApInterface(PRTMP_ADAPTER pAd)
{
#ifdef RT_CFG80211_P2P_SUPPORT
	UINT apidx = CFG_GO_BSSID_IDX;
#else
	UINT apidx = MAIN_MBSSID;
#endif /*RT_CFG80211_P2P_SUPPORT*/
	/*CFG_TODO: IT Should be set fRTMP_ADAPTER_HALT_IN_PROGRESS */
	struct wifi_dev *pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.bBcnSntReq = FALSE;

	/* For AP - STA switch */
	if (wlan_operate_get_vht_bw(pWdev) != BW_40) {
		CFG80211DBG(DBG_LVL_INFO, ("80211> %s, switch to BW_20\n", __func__));
		wlan_operate_set_ht_bw(pWdev, HT_BW_20, EXTCHA_NONE);
	}

	/* Disable pre-TBTT interrupt */
	AsicSetPreTbtt(pAd, FALSE, HW_BSSID_0);

	if (1) { /* !INFRA_ON(pAd)) */
		/* Disable piggyback */
		AsicSetPiggyBack(pAd, FALSE);
		/*shailesh disabling call for HW_SET_PROTECT */
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		AsicDisableSync(pAd, HW_BSSID_0);


	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SINGLE_DEVICE
	/* re-assoc to STA's wdev */
	RTMP_OS_NETDEV_SET_WDEV(pAd->net_dev, &pAd->StaCfg[0].wdev);
#endif /* P2P_SINGLE_DEVICE */
#endif /*CONFIG_STA_SUPPORT*/
}

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
PCHAR rtstrstr2(PCHAR s1, const PCHAR s2, INT s1_len, INT s2_len)
{
	INT offset = 0;

	while (s1_len >= s2_len) {
		s1_len--;

		if (!memcmp(s1, s2, s2_len))
			return offset;

		s1++;
		offset++;
	}

	return NULL;
}
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */

VOID CFG80211_UpdateBeacon(
	VOID                                            *pAdOrg,
	UCHAR										    *beacon_head_buf,
	UINT32											beacon_head_len,
	UCHAR										    *beacon_tail_buf,
	UINT32											beacon_tail_len,
	BOOLEAN											isAllUpdate,
	UINT32											apidx)

{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	HTTRANSMIT_SETTING BeaconTransmit;   /* MGMT frame PHY rate setting when operatin at Ht rate. */
	PUCHAR pBeaconFrame;
	UCHAR *tmac_info, New_Tim_Len = 0;
	UINT32 beacon_len = 0;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	COMMON_CONFIG *pComCfg;
#ifdef RT_CFG80211_SUPPORT
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
#endif
#ifdef RT_CFG80211_P2P_SUPPORT
	UINT apidx = CFG_GO_BSSID_IDX;
#else /* RT_CFG80211_P2P_SUPPORT */
#endif /* !RT_CFG80211_P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	ULONG	Value;
	ULONG	TimeTillTbtt;
	ULONG	temp;
	INT		bufferoffset = 0;
	USHORT		bufferoffset2 = 0;
	CHAR	temp_buf[512] = {0};
	CHAR	P2POUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x9};
	INT	temp_len;
	INT P2P_IE = 4;
	USHORT p2p_ie_len;
	UCHAR Count;
	ULONG StartTime;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UINT8 TXWISize = cap->TXWISize;
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
	UINT8 max_v2_bcn_num = cap->max_v2_bcn_num;
#endif
	BCN_BUF_STRUCT *pbcn_buf = NULL;

	pComCfg = &pAd->CommonCfg;
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	pbcn_buf = &wdev->bcn_buf;

	if (!pMbss || !pMbss->wdev.bcn_buf.BeaconPkt)
		return;

	RTMP_SEM_LOCK(&pbcn_buf->BcnContentLock);
	tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(pMbss->wdev.bcn_buf.BeaconPkt);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		pBeaconFrame = (UCHAR *)(tmac_info + tx_hw_hdr_len);
	else
#endif /* MT_MAC */
	{
		pBeaconFrame = (UCHAR *)(tmac_info + TXWISize);
	}

	if (isAllUpdate) { /* Invoke From CFG80211 OPS For setting Beacon buffer */
		/* 1. Update the Buf before TIM IE */
		NdisCopyMemory(pBeaconFrame, beacon_head_buf, beacon_head_len);
		/* 2. Update the Location of TIM IE */
		pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon = beacon_head_len;

		/* 3. Store the Tail Part For appending later */
		if (pCfg80211_ctrl->beacon_tail_buf != NULL)
			os_free_mem(pCfg80211_ctrl->beacon_tail_buf);

		os_alloc_mem(NULL, (UCHAR **)&pCfg80211_ctrl->beacon_tail_buf, beacon_tail_len);

		if (pCfg80211_ctrl->beacon_tail_buf != NULL) {
			NdisCopyMemory(pCfg80211_ctrl->beacon_tail_buf, beacon_tail_buf, beacon_tail_len);
			pCfg80211_ctrl->beacon_tail_len = beacon_tail_len;
		} else {
			pCfg80211_ctrl->beacon_tail_len = 0;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CFG80211 Beacon: MEM ALLOC ERROR\n");
		}

		/* return; */
	} else { /* Invoke From Beacon Timer */
		if (pAd->ApCfg.DtimCount == 0)
			pAd->ApCfg.DtimCount = pAd->ApCfg.DtimPeriod - 1;
		else
			pAd->ApCfg.DtimCount -= 1;

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
		/*
		 *	3 mode:
		 *		1. infra scan  7 channel  ( Duration(30+3) *7   interval (+120)  *   count  1 ),
		 *		2. p2p find    3 channel   (Duration (65 ) *3     interval (+130))  * count 2   > 120 sec
		 *		3. mcc  tw channel switch (Duration )  (Infra time )  interval (+ GO time )  count 3  mcc enabel always;
		 */
		if (pAd->cfg80211_ctrl.GONoASchedule.Count > 0) {
			if (pAd->cfg80211_ctrl.GONoASchedule.Count != 200)
				pAd->cfg80211_ctrl.GONoASchedule.Count--;

			os_move_mem(temp_buf, pCfg80211_ctrl->beacon_tail_buf, pCfg80211_ctrl->beacon_tail_len);
			bufferoffset = rtstrstr2(temp_buf, P2POUIBYTE, pCfg80211_ctrl->beacon_tail_len, P2P_IE);

			while (bufferoffset2 <= (pCfg80211_ctrl->beacon_tail_len - bufferoffset - 4 - bufferoffset2 - 3)) {
				if ((pCfg80211_ctrl->beacon_tail_buf)[bufferoffset + 4 + bufferoffset2] == 12)
					break;

				bufferoffset2 = pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + 1 + bufferoffset2] + bufferoffset2;
				bufferoffset2 = bufferoffset2 + 3;
			}

			NdisCopyMemory(&pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + bufferoffset2 + 5], &pAd->cfg80211_ctrl.GONoASchedule.Count, 1);
			NdisCopyMemory(&pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + bufferoffset2 + 6], &pAd->cfg80211_ctrl.GONoASchedule.Duration, 4);
			NdisCopyMemory(&pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + bufferoffset2 + 10], &pAd->cfg80211_ctrl.GONoASchedule.Interval, 4);
			NdisCopyMemory(&pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + bufferoffset2 + 14], &pAd->cfg80211_ctrl.GONoASchedule.StartTime, 4);
		}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	}

#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef RTMP_PCI_SUPPORT

		if (IS_PCI_INF(pAd)) {
			BOOLEAN is_pretbtt_int = FALSE;
			UCHAR resource_idx = 0;
			USHORT FreeNum;

			resource_idx = hif_get_resource_idx(pAd->hdev_ctrl, wdev, TX_DATA, 0);

			FreeNum = hif_get_tx_resource_free_num(pAd->hdev_ctrl, resource_idx);

			if (FreeNum < 0) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "=>BSS0:BcnRing FreeNum is not enough!\n");
				return;
			}

			if (pMbss->wdev.bcn_buf.bcn_state != BCN_TX_IDLE) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "=>BSS0:BcnPkt not idle(%d)!\n",
						 pMbss->wdev.bcn_buf.bcn_state);
				APCheckBcnQHandler(pAd, apidx, &is_pretbtt_int);

				if (is_pretbtt_int == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "==============> pretbtt_int not init\n");
					return;
				}
			}
		}

#endif /* RTMP_PCI_SUPPORT */
	}

#endif /* MT_MAC */
	/* 4. Update the TIM IE */
	New_Tim_Len = CFG80211DRV_UpdateTimIE(pAd, apidx, pBeaconFrame,
										  pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon);

	/* 5. Update the Buffer AFTER TIM IE */
	if (pCfg80211_ctrl->beacon_tail_buf != NULL) {
		NdisCopyMemory(pBeaconFrame + pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon + New_Tim_Len,
					   pCfg80211_ctrl->beacon_tail_buf, pCfg80211_ctrl->beacon_tail_len);
		beacon_len = pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon + pCfg80211_ctrl->beacon_tail_len
					 + New_Tim_Len;
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "BEACON ====> CFG80211_UpdateBeacon OOPS\n");
		return;
	}

	BeaconTransmit.word = 0;
	/* Should be Find the P2P IE Then Set Basic Rate to 6M */
#ifdef RT_CFG80211_P2P_SUPPORT

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
		BeaconTransmit.field.MODE = MODE_OFDM; /* Use 6Mbps */
	else
#endif /*RT_CFG80211_P2P_SUPPORT*/
		BeaconTransmit.field.MODE = MODE_CCK;

	BeaconTransmit.field.MCS = MCS_RATE_6;
	RTMP_SEM_UNLOCK(&pbcn_buf->BcnContentLock);
	/* CFG_TODO */
#ifdef BCN_OFFLOAD_SUPPORT

	if (cap->fgBcnOffloadSupport == TRUE) {
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
#ifndef RT_CFG80211_SUPPORT
	if (wdev->func_idx < max_v2_bcn_num)
		RT28xx_UpdateBcnAndTimToMcu(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len,
			pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_V2_BCN);
	else
		RT28xx_UpdateBcnAndTimToMcu(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len, pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_V2_BCN);
#else
	arch_ops->archUpdateBeacon(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, TRUE, BCN_UPDATE_RESERVE);
#endif
#else
#ifdef RT_CFG80211_SUPPORT
	arch_ops->archUpdateBeacon(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, TRUE, BCN_UPDATE_RESERVE);
#else

	RT28xx_UpdateBcnAndTimToMcu(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len,
				pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_V1_BCN);
#endif
#endif
		}
	else
#endif /* BCN_OFFLOAD_SUPPORT */
	{
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
#ifndef RT_CFG80211_SUPPORT
	if (wdev->func_idx < max_v2_bcn_num)
		RT28xx_UpdateBeaconToAsic(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len,
			pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_V2_BCN);
	else
		RT28xx_UpdateBeaconToAsic(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len,
					pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_V1_BCN);
#else
	arch_ops->archUpdateBeacon(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, TRUE, BCN_UPDATE_RESERVE);
#endif
#else
#ifdef RT_CFG80211_SUPPORT
	arch_ops->archUpdateBeacon(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, TRUE, BCN_UPDATE_RESERVE);
#else

	RT28xx_UpdateBeaconToAsic(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len,
				pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_V1_BCN);
#endif /* RT_CFG80211_SUPPORT */
#endif
	}
}

BOOLEAN CFG80211DRV_OpsBeaconSet(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon;

#ifdef DISABLE_HOSTAPD_BEACON
    BSS_STRUCT *pMbss;
    struct wifi_dev *wdev;
    UINT16 Frame_Len = 0;
#endif
#ifdef RT_CFG80211_P2P_SUPPORT
	UINT apidx = CFG_GO_BSSID_IDX;
#else
#endif /*RT_CFG80211_P2P_SUPPORT*/
	pBeacon = (CMD_RTPRIV_IOCTL_80211_BEACON *)pData;
#ifdef DISABLE_HOSTAPD_BEACON
	pMbss = &pAd->ApCfg.MBSSID[pBeacon->apidx];
	wdev = &pMbss->wdev;
#endif
	CFG80211DRV_UpdateApSettingFromBeacon(pAd, pBeacon->apidx, pBeacon);
#ifdef DISABLE_HOSTAPD_BEACON
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"############MakeBeacon for apidx %d OpsBeaconSet \n",
		pBeacon->apidx);
	Frame_Len = MakeBeacon(pAd, wdev, FALSE);
	AsicUpdateBeacon(pAd, wdev, TRUE, BCN_UPDATE_RESERVE);
#else
	CFG80211_UpdateBeacon(pAd, pBeacon->beacon_head, pBeacon->beacon_head_len,
		pBeacon->beacon_tail, pBeacon->beacon_tail_len,
		TRUE, pBeacon->apidx);
#endif
	return TRUE;
}

#ifdef HOSTAPD_HS_R2_SUPPORT

BOOLEAN CFG80211DRV_SetQosParam(VOID *pAdOrg, VOID *pData, INT apindex)
{

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	struct cfg80211_qos_map *qos_map = (struct cfg80211_qos_map *)pData;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apindex];
	PHOTSPOT_CTRL pHSCtrl = &pMbss->HotSpotCtrl;
	PUCHAR pos;
	int tmp = 0;

	RTMP_SEM_LOCK(&pHSCtrl->IeLock);
	if (pHSCtrl->QosMapSetIE) {
		os_free_mem(pHSCtrl->QosMapSetIE);
		pHSCtrl->QosMapSetIE = NULL;
	}
	if (qos_map == NULL) {
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
		return TRUE;
	}
    os_alloc_mem(NULL, &pHSCtrl->QosMapSetIE, (2+(qos_map->num_des * 2) + 16));
	if (pHSCtrl->QosMapSetIE == NULL) {
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "QosMapSet Alloc Fail \n");
		return FALSE;
	}
	pos = pHSCtrl->QosMapSetIE;
	*pos = IE_QOS_MAP_SET;
	pos++;
	*pos = 16 + (qos_map->num_des * 2);
	pos++;
	if (qos_map->num_des > 0) {
		memcpy(pos, (PUCHAR)qos_map->dscp_exception, (qos_map->num_des*2));
		pos += qos_map->num_des*2;
	}
	memcpy(pos, (PUCHAR)qos_map->up, 16);
    pHSCtrl->QosMapSetIELen = 2+(qos_map->num_des * 2)+16;
	RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
	for (tmp = 0; tmp < 21; tmp++) {
		pHSCtrl->DscpException[tmp] = 0xff;
		pHSCtrl->DscpException[tmp] |= (0xff << 8);
	}
	for (tmp = 0; tmp < 8; tmp++) {
		pHSCtrl->DscpRange[tmp] = 0xff;
		pHSCtrl->DscpRange[tmp] |= (0xff << 8);
	}
	for (tmp = 0; tmp < qos_map->num_des; tmp++) {
		pHSCtrl->DscpException[tmp] = (qos_map->dscp_exception[tmp].dscp) & 0xff;
		pHSCtrl->DscpException[tmp] |= ((qos_map->dscp_exception[tmp].up) & 0xff) << 8;
	}
	for (tmp = 0; tmp < 8; tmp++) {
		pHSCtrl->DscpRange[tmp] = (qos_map->up[tmp].low) & 0xff;
		pHSCtrl->DscpRange[tmp] |= ((qos_map->up[tmp].high) & 0xff) << 8;
	}
	return TRUE;
}
#endif

extern struct wifi_dev_ops ap_wdev_ops;
extern struct wifi_dev_ops go_wdev_ops;

BOOLEAN CFG80211DRV_OpsBeaconAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	/* BOOLEAN Cancelled; */
	UINT i = 0;
	INT32 Ret = 0;
	EDCA_PARM *pEdca, *pBssEdca = NULL;
	STA_REC_CTRL_T *sta_rec;

#ifdef DISABLE_HOSTAPD_BEACON
	UINT16 FrameLen = 0;
#ifdef DOT11V_MBSSID_SUPPORT
	UINT8 DbdcIdx;
#endif
#endif
	UINT16 tr_tb_idx;
	PNET_DEV pNetDev;
	CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon =  (CMD_RTPRIV_IOCTL_80211_BEACON *)pData;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	UCHAR ext_cha;
	UCHAR ht_bw;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
	UINT apidx = CFG_GO_BSSID_IDX;
#else
	UINT apidx = pBeacon->apidx;
#endif /*RT_CFG80211_P2P_SUPPORT*/
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	struct wifi_dev *wdev = &pMbss->wdev;
	HT_CAPABILITY_IE *ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	BCN_BUF_STRUCT *bcn_buf = &wdev->bcn_buf;
	tr_tb_idx = wdev->tr_tb_idx;
	pNetDev = pBeacon->pNetDev;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	PNET_DEV pNetDev = NULL;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#ifdef RT_CFG80211_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
#endif
		wdev->Hostapd = Hostapd_CFG;

#endif
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
#ifdef UAPSD_SUPPORT
	wdev->UapsdInfo.bAPSDCapable = TRUE;
	pMbss->CapabilityInfo |= 0x0800;
#endif /* UAPSD_SUPPORT */
	ap_send_broadcast_deauth(pAd, wdev);
#ifndef DISABLE_HOSTAPD_BEACON
    pAd->cfg80211_ctrl.beaconIsSetFromHostapd = TRUE; /* set here to prevent MakeBeacon do further modifications about BCN */
#endif
	CFG80211DRV_UpdateApSettingFromBeacon(pAd, apidx, pBeacon);
#define MCAST_WCID_TO_REMOVE 0
	MgmtTableSetMcastEntry(pAd, MCAST_WCID_TO_REMOVE);
	APSecInit(pAd, wdev);
	sta_rec = &pAd->MacTab.tr_entry[tr_tb_idx].StaRec;
	ap_key_table_init(pAd, wdev);
	ap_set_key_for_sta_rec(pAd, wdev, sta_rec);
	AsicSetRxFilter(pAd);
	/* Start from 0 & MT_MAC using HW_BSSID 1, TODO */
#ifdef RT_CFG80211_P2P_SUPPORT
	pAd->ApCfg.BssidNum = (CFG_GO_BSSID_IDX + 1);
#else
#endif /*RT_CFG80211_P2P_SUPPORT*/
	pAd->MacTab.MsduLifeTime = 20; /* pEntry's UAPSD Q Idle Threshold */
	/* CFG_TODO */
	bcn_buf->pWdev = wdev;

	for (i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
		bcn_buf->TimBitmaps[i] = 0;

	bcn_buf->bBcnSntReq = TRUE;
	/* For GO Timeout */
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	pAd->ApCfg.StaIdleTimeout = 300;
	pMbss->StationKeepAliveTime = 60;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	AsicDisableSync(pAd, HW_BSSID_0);

#ifndef DISABLE_HOSTAPD_BEACON
	if (wdev->channel > 14)
		wdev->PhyMode = (WMODE_A | WMODE_AN);
	else
		wdev->PhyMode = (WMODE_B | WMODE_G | WMODE_GN);
#endif /* DISABLE_HOSTAPD_BEACON */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_GO);

	/* Using netDev ptr from VifList if VifDevList Exist */
	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		(pNetDev != NULL)) {
		Ret = wdev_init(pAd, wdev, WDEV_TYPE_GO, pNetDev, apidx, (VOID *)&pAd->ApCfg.MBSSID[apidx], (VOID *)pAd);
		wdev_attr_update(pAd, &pMbss->wdev);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "register wdev fail\n");
			RtmpOSNetDevFree(NetDev);
			return FALSE;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_GO, &go_wdev_ops,
								cap->wmm_detect_method);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "register wdev_ops %s failed, free net device!\n",
					  RTMP_OS_NETDEV_GET_DEVNAME(NetDev));
			RtmpOSNetDevFree(NetDev);
			return FALSE;
		}

		COPY_MAC_ADDR(wdev->bssid, pNetDev->dev_addr);
		COPY_MAC_ADDR(wdev->if_addr, pNetDev->dev_addr);
		os_move_mem(wdev->bss_info_argument.Bssid, wdev->bssid, MAC_ADDR_LEN);
		RTMP_OS_NETDEV_SET_WDEV(pNetDev, wdev);
		RTMP_OS_NETDEV_SET_PRIV(pNetDev, pAd);
	} else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
	{
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
		if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
			IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
			IS_MT7981(pAd)) {
			;  /* don't reinit wdev, for tr_tbl was acquired in previous flow */
		} else
#endif
			Ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, pAd->net_dev, apidx, (VOID *)&pAd->ApCfg.MBSSID[apidx], (VOID *)pAd);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "register wdev fail\n");
		}

	wdev_attr_update(pAd, wdev);
	COPY_MAC_ADDR(wdev->bssid, pNetDev->dev_addr);
	COPY_MAC_ADDR(wdev->if_addr, pNetDev->dev_addr);
	os_move_mem(wdev->bss_info_argument.Bssid, wdev->bssid, MAC_ADDR_LEN);
	}

	if (WMODE_CAP_N(wdev->PhyMode) && wlan_operate_get_ht_bw(wdev) == BW_40)
		ht_cap->MCSSet[4] = 0x1; /* MCS 32*/

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode)) {
		struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
		struct bss_color_ctrl *bss_color = &bss_info->bss_color;

		/* update wlan_operation state */
		wlan_operate_set_he_bss_color(wdev, bss_color->color, bss_color->disabled);

		/* reset parameters in BSS Color Change Announcement IE */
		wlan_operate_set_he_bss_next_color(wdev, bss_color->next_color, 0/* countdown */);
	}
#endif
	/* cfg_todo */
	wdev->bWmmCapable = TRUE;
	os_move_mem(wdev->bss_info_argument.Bssid, wdev->bssid, MAC_ADDR_LEN);
	/* BC/MC Handling */
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
		IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
		IS_MT7981(pAd)) {
		if (IS_CIPHER_WEP(wdev->SecConfig.GroupCipher))
			CFG80211DRV_ApKeyAdd(pAdOrg, &pAd->cfg80211_ctrl.WepKeyInfoBackup);
	} else
#endif
		TRTableInsertMcastEntry(pAd, tr_tb_idx, wdev);

#ifdef RT_CFG80211_P2P_SUPPORT
	bcn_buf_init(pAd, &pAd->ApCfg.MBSSID[apidx].wdev);
#endif /*RT_CFG80211_P2P_SUPPORT*/
	MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
	WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
	wdev->bss_info_argument.CipherSuit = SecHWCipherSuitMapping(wdev->SecConfig.PairwiseCipher);
#ifdef MBSS_DTIM_SUPPORT
	wdev->bss_info_argument.dtim_period = pMbss->DtimPeriod;
#endif
	wdev->bss_info_argument.u4BssInfoFeature = (BSS_INFO_OWN_MAC_FEATURE |
			BSS_INFO_BASIC_FEATURE |
			BSS_INFO_RF_CH_FEATURE |
			BSS_INFO_SYNC_MODE_FEATURE);
	/* AsicBssInfoUpdate(pAd, wdev->bss_info_argument); */
	os_msec_delay(200);
	HW_UPDATE_BSSINFO(pAd, &wdev->bss_info_argument);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "New AP BSSID "MACSTR" (%d)\n",
			 MAC2STR(wdev->bssid), wdev->PhyMode);
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode) && (pAd->Antenna.field.TxPath == 2))
		bbp_set_txdac(pAd, 2);
	else
#endif /* DOT11_N_SUPPORT */
		bbp_set_txdac(pAd, 0);

	/* Receiver Antenna selection */
	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)) {
		if (WMODE_CAP_N(wdev->PhyMode) || wdev->bWmmCapable) {
			pEdca = &pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx];

			/* EDCA parameters used for AP's own transmission */
			if (pEdca->bValid == FALSE)
				set_default_ap_edca_param(pEdca);

			pBssEdca = wlan_config_get_ht_edca(wdev);
			if (pBssEdca) {
				/* EDCA parameters to be annouced in outgoing BEACON, used by WMM STA */
				if (pBssEdca->bValid == FALSE)
					set_default_sta_edca_param(pBssEdca);
			}

			HcAcquiredEdca(pAd, wdev, pEdca);
			HcSetEdca(wdev);
		} else
			HcReleaseEdca(pAd, wdev);
	}

#ifdef DOT11_N_SUPPORT
	if (pAd->CommonCfg.bRdg)
		AsicSetRDG(pAd, WCID_ALL, 0, 0, 0);

	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);
#endif /* DOT11_N_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Reset WCID Table\n");
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7615(pAd) || IS_MT7622(pAd)  || IS_MT7626(pAd) ||
		IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
		IS_MT7981(pAd))  {
		;  /* don't reset WCID table , for 7615 has set in previous flow */
	} else
#endif
	HW_SET_DEL_ASIC_WCID(pAd, WCID_ALL);
	pAd->MacTab.Content[0].Addr[0] = 0x01;

	pAd->MacTab.Content[0].HTPhyMode.field.MODE = MODE_OFDM;
	pAd->MacTab.Content[0].HTPhyMode.field.MCS = 3;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	wlan_operate_set_prim_ch(wdev, wdev->channel);
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	/* MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble); */
	/* MlmeUpdateTxRates(pAd, FALSE, MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO + apidx); */
#ifdef RT_CFG80211_P2P_SUPPORT
	MlmeUpdateTxRates(pAd, FALSE, MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO + apidx);
#else
	MlmeUpdateTxRates(pAd, FALSE, apidx);
#endif /*RT_CFG80211_P2P_SUPPORT*/
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode))
		MlmeUpdateHtTxRates(pAd, wdev);

#endif /* DOT11_N_SUPPORT */

	/* Disable Protection first. */
	if (1)/* !INFRA_ON(pAd)) */
		/*shailesh: commenting out AsicUpdateProtect */

	ApUpdateCapabilityAndErpIe(pAd, pMbss);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd, wdev);
#endif /* DOT11_N_SUPPORT */

#ifdef DISABLE_HOSTAPD_BEACON
#ifdef DOT11V_MBSSID_SUPPORT
	/* if BSSID is non-transmitted, must do update by transmitted BSSID */
	DbdcIdx = HcGetBandByWdev(wdev);
	if (IS_BSSID_11V_NON_TRANS(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx], DbdcIdx)) {
		UCHAR OrigWdevIdx = wdev->wdev_idx;

		wdev->bAllowBeaconing = FALSE;
                WDEV_BSS_STATE(wdev) = BSS_READY;
		wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.dot11v_trans_bss_idx[DbdcIdx]].wdev;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"wdev(%d) is Nontransmitted Bssid, update to BssIdx %d wdev(%d)\n",
				OrigWdevIdx, pAd->ApCfg.dot11v_trans_bss_idx[DbdcIdx], wdev->wdev_idx);
	}
#endif

	FrameLen =  MakeBeacon(pAd, wdev, FALSE);
	AsicUpdateBeacon(pAd, wdev, TRUE, BCN_UPDATE_RESERVE);
#else
	CFG80211_UpdateBeacon(pAd, pBeacon->beacon_head, pBeacon->beacon_head_len,
	pBeacon->beacon_tail, pBeacon->beacon_tail_len, TRUE, pBeacon->apidx);
#endif /*DISABLE_HOSTAPD_BEACON */


#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

	if (INFRA_ON(pAd)) {
		ULONG BPtoJiffies;
		LONG timeDiff;
		INT starttime = pAd->Mlme.channel_1st_staytime;

		NdisGetSystemUpTime(&pAd->Mlme.BeaconNow32);
		timeDiff = (pAd->Mlme.BeaconNow32 - pAd->StaCfg[0].LastBeaconRxTime) % (pAd->CommonCfg.BeaconPeriod);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "#####pAd->Mlme.Now32 %d pAd->StaCfg[0].LastBeaconRxTime %d\n", pAd->Mlme.BeaconNow32, pAd->StaCfg[0].LastBeaconRxTime);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "####    timeDiff %d\n", timeDiff);

		if (starttime > timeDiff)
			OS_WAIT((starttime - timeDiff));
		else
			OS_WAIT((starttime + (pAd->CommonCfg.BeaconPeriod - timeDiff)));
	}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	/* Enable BSS Sync*/
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

	if (INFRA_ON(pAd)) {
		ULONG BPtoJiffies;
		LONG timeDiff;
		INT starttime = pAd->Mlme.channel_1st_staytime;

		NdisGetSystemUpTime(&pAd->Mlme.BeaconNow32);
		timeDiff = (pAd->Mlme.BeaconNow32 - pAd->StaCfg[0].LastBeaconRxTime) % (pAd->CommonCfg.BeaconPeriod);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "#####pAd->Mlme.Now32 %d pAd->StaCfg[0].LastBeaconRxTime %d\n", pAd->Mlme.BeaconNow32, pAd->StaCfg[0].LastBeaconRxTime);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "####    timeDiff %d\n", timeDiff);

		if (starttime > timeDiff)
			OS_WAIT((starttime - timeDiff));
		else
			OS_WAIT((starttime + (pAd->CommonCfg.BeaconPeriod - timeDiff)));
	}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	/* Enable AP BSS Sync */
	/* AsicEnableApBssSync(pAd, pAd->CommonCfg.BeaconPeriod); */
	/* AsicEnableBcnSntReq(pAd); */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);
	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
#if defined(RT_CFG80211_SUPPORT) || defined(MT7622)
	WDEV_BSS_STATE(wdev) = BSS_READY;
#endif
	return TRUE;
}

BOOLEAN CFG80211DRV_ApKeyDel(
	VOID                                            *pAdOrg,
	VOID                                            *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
	MAC_TABLE_ENTRY *pEntry;
	struct _ASIC_SEC_INFO *info = NULL;

	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)

	if (pKeyInfo->bPairwise == FALSE)
#else
	if (pKeyInfo->KeyId > 0)
#endif
	{
		UINT Wcid = 0;
		UINT apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pKeyInfo->pNetDev);
		BSS_STRUCT *pMbss;
		struct wifi_dev *pWdev;

		if (apidx == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "failed - [ERROR]can't find wdev in driver MBSS. \n");
			return FALSE;
		}
		pMbss = &pAd->ApCfg.MBSSID[apidx];
		pWdev  = &pMbss->wdev;
		GET_GroupKey_WCID(pWdev, Wcid);
		/* Set key material to Asic */
		os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
		os_zero_mem(info, sizeof(ASIC_SEC_INFO));
		info->Operation = SEC_ASIC_REMOVE_GROUP_KEY;
		info->Wcid = Wcid;
		/* Set key material to Asic */
		HW_ADDREMOVE_KEYTABLE(pAd, info);
		os_free_mem(info);
	} else {
		pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

		if (pEntry && (pEntry->Aid != 0)) {
			/* Set key material to Asic */
			os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
			os_zero_mem(info, sizeof(ASIC_SEC_INFO));
			info->Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
			info->Wcid = pEntry->wcid;
			/* Set key material to Asic */
			HW_ADDREMOVE_KEYTABLE(pAd, info);
			os_free_mem(info);
		}
	}

	return TRUE;
}

VOID CFG80211DRV_RtsThresholdAdd(
	VOID                                            *pAdOrg,
	struct wifi_dev *wdev,
	UINT                                            threshold)
{
	UINT32 len_thld = MAX_RTS_THRESHOLD;
	PRTMP_ADAPTER pAd = NULL;

	if ((threshold > 0) && (threshold <= MAX_RTS_THRESHOLD))
		len_thld = (UINT32)threshold;
	pAd = (PRTMP_ADAPTER)pAdOrg;
	wlan_operate_set_rts_len_thld(wdev, len_thld);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			("%s =====>threshold %d\n", __func__, len_thld));
}


VOID CFG80211DRV_FragThresholdAdd(
	VOID                                            *pAdOrg,
	struct wifi_dev *wdev,
	UINT                                            threshold)
{
	PRTMP_ADAPTER pAd = NULL;

	if (threshold > MAX_FRAG_THRESHOLD || threshold < MIN_FRAG_THRESHOLD)
		threshold =  MAX_FRAG_THRESHOLD;
	else if (threshold % 2 == 1)
		threshold -= 1;
	pAd = (PRTMP_ADAPTER)pAdOrg;
	wlan_operate_set_frag_thld(wdev, threshold);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			("%s =====>operate: frag_thld=%d\n", __func__, threshold));
}

#ifdef ACK_CTS_TIMEOUT_SUPPORT
BOOLEAN CFG80211DRV_AckThresholdAdd(
	VOID * pAdOrg,
	struct wifi_dev	*wdev,
	UINT threshold)
{
	UINT32 len_thld = MAX_ACK_THRESHOLD;
	UCHAR band_idx = 0;
	PRTMP_ADAPTER pAd = NULL;

	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"threshold = %d\n", threshold);

	if ((threshold > 0) && (threshold <= MAX_ACK_THRESHOLD))
		len_thld = (UINT32)threshold;

		pAd = (PRTMP_ADAPTER)pAdOrg;
		if (NULL == wdev || NULL == pAd) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s](%d): wdev is null or pAd is null.\n",
			__func__, __LINE__));
			return FALSE;
		}

		band_idx = HcGetBandByWdev(wdev);

		if (FALSE == set_ack_timeout_mode_byband(pAd,
			threshold, band_idx, ACK_ALL_TIME_OUT)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"SET CTS/ACK Timeout Fail!!\n");
			return FALSE;
		}
		return TRUE;

		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			("[%s](%d): NOT support this function.\n", __func__, __LINE__));
		return FALSE;
}

#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

BOOLEAN CFG80211DRV_ApKeyAdd(
	VOID                                            *pAdOrg,
	VOID                                            *pData)
{
#ifdef CONFIG_AP_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT apidx;
	BSS_STRUCT *pMbss;
	struct wifi_dev *pWdev;

	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;
	/* UINT Wcid = 0; */
#ifdef RT_CFG80211_P2P_SUPPORT
	UINT apidx = CFG_GO_BSSID_IDX;
#else
	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pKeyInfo->pNetDev);
#endif /*RT_CFG80211_P2P_SUPPORT*/
	if (!VALID_MBSS(pAd, apidx))
		return FALSE;
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	pWdev = &pMbss->wdev;

	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, " =====>\n");
	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;

	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40 || pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP104) {
		SET_CIPHER_WEP(pWdev->SecConfig.PairwiseCipher);
		SET_CIPHER_WEP(pWdev->SecConfig.GroupCipher);
		{
			CIPHER_KEY	*pSharedKey;
			POS_COOKIE pObj;

			pObj = (POS_COOKIE) pAd->OS_Cookie;
			pSharedKey = &pAd->SharedKey[apidx][pKeyInfo->KeyId];
			pSharedKey->KeyLen = pKeyInfo->KeyLen;
			os_move_mem(pSharedKey->Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);

			if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40)
				pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_WEP64;
			else
				pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_WEP128;

			AsicAddSharedKeyEntry(pAd, apidx, pKeyInfo->KeyId, pSharedKey);
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)

			if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
				IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
				IS_MT7981(pAd)) {
				if (pKeyInfo->bPairwise == FALSE) {
					struct _ASIC_SEC_INFO *info = NULL;
					UINT Wcid = 0;

					NdisCopyMemory(&pAd->cfg80211_ctrl.WepKeyInfoBackup, pKeyInfo, sizeof(CMD_RTPRIV_IOCTL_80211_KEY));
					pWdev->SecConfig.WepKey[pKeyInfo->KeyId].KeyLen = pKeyInfo->KeyLen;
					os_move_mem(pWdev->SecConfig.WepKey[pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
					pWdev->SecConfig.GroupKeyId = pKeyInfo->KeyId;
					os_move_mem(pWdev->SecConfig.GTK, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
					/* Get a specific WCID to record this MBSS key attribute */
					GET_GroupKey_WCID(pWdev, Wcid);
					/* Set key material to Asic */
					os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
					os_zero_mem(info, sizeof(ASIC_SEC_INFO));
					info->Operation = SEC_ASIC_ADD_GROUP_KEY;
					info->Direction = SEC_ASIC_KEY_TX;
					info->Wcid = Wcid;
					info->BssIndex = apidx;
					info->Cipher = pWdev->SecConfig.GroupCipher;
					info->KeyIdx = pKeyInfo->KeyId;
					os_move_mem(&info->PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
					/* Install Shared key */
					os_move_mem(info->Key.Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
					info->Key.KeyLen = pKeyInfo->KeyLen;
					HW_ADDREMOVE_KEYTABLE(pAd, info);
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "%u B/MC KEY pKeyInfo->KeyId %d pWdev->SecConfig.WepKey[pKeyInfo->KeyId].KeyLen %d\n"
							  , __LINE__, pKeyInfo->KeyId, pWdev->SecConfig.WepKey[pKeyInfo->KeyId].KeyLen);
					os_free_mem(info);
				} else {
						pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

					if (pEntry) {
						struct _ASIC_SEC_INFO *info = NULL;

						pEntry->SecConfig.PairwiseKeyId = pKeyInfo->KeyId;
						SET_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher);
						/* Set key material to Asic */
						os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
						os_zero_mem(info, sizeof(ASIC_SEC_INFO));
						info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
						info->Direction = SEC_ASIC_KEY_BOTH;
						info->Wcid = pEntry->wcid;
						info->BssIndex = pEntry->func_tb_idx;
						info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						info->Cipher = pEntry->SecConfig.PairwiseCipher;
						info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						os_move_mem(info->Key.Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
						os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
						info->Key.KeyLen = pKeyInfo->KeyLen;
						HW_ADDREMOVE_KEYTABLE(pAd, info);
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								 "%u UNICAST Info.Key.KeyLen %d pKeyInfo->KeyId %d Info.Key.KeyLen %d\n"
								  , __LINE__, info->Key.KeyLen, pKeyInfo->KeyId, info->Key.KeyLen);
						os_free_mem(info);
					}
				}
			}

#endif
		}
	} else if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WPA) {
			/* AES */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
			if (pKeyInfo->bPairwise == FALSE)
#else
			if (pKeyInfo->KeyId > 0)
#endif	/* LINUX_VERSION_CODE 2.6.37 */
			{
				USHORT Wcid;
				/* Get a specific WCID to record this MBSS key attribute */
				GET_GroupKey_WCID(pWdev, Wcid);
				pAd->SharedKey[apidx][pKeyInfo->KeyId].KeyLen = LEN_TK;

				switch (pKeyInfo->cipher) {
				case Ndis802_11GCMP256Enable:
					if (!IS_CIPHER_GCMP256(pWdev->SecConfig.GroupCipher)) {
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher);
						return FALSE;
					}
						SET_CIPHER_GCMP256(pWdev->SecConfig.GroupCipher);
						break;

				case Ndis802_11AESEnable:
					if (!IS_CIPHER_CCMP128(pWdev->SecConfig.GroupCipher) &&
						pKeyInfo->pNetDev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_AP_VLAN) {
						/* skip cipher check for AP_VLAN because .add_key of AP_VLAN is prior to cipher setup */
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher);
						return FALSE;
					}
					pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_AES;
					SET_CIPHER_CCMP128(pWdev->SecConfig.GroupCipher);
					break;

				case Ndis802_11TKIPEnable:
					if (!IS_CIPHER_TKIP(pWdev->SecConfig.GroupCipher)) {
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher);
						return FALSE;
					}
					pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_TKIP;
					SET_CIPHER_TKIP(pWdev->SecConfig.GroupCipher);
					break;
				case Ndis802_11GCMP128Enable:
					if (!IS_CIPHER_GCMP128(pWdev->SecConfig.GroupCipher)) {
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher);
						return FALSE;
					}
					SET_CIPHER_GCMP128(pWdev->SecConfig.GroupCipher);

					break;

				case Ndis802_11CCMP256Enable:
					if (!IS_CIPHER_CCMP256(pWdev->SecConfig.GroupCipher)) {
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher);
						return FALSE;
					}
					SET_CIPHER_CCMP256(pWdev->SecConfig.GroupCipher);

					break;
			}
#if defined(MT7615) || defined(MT7626) || defined(MT7622) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)

					if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
						IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
						IS_MT7981(pAd)) {
						struct _ASIC_SEC_INFO *info = NULL;
						USHORT Wcid;
#ifdef CONFIG_VLAN_GTK_SUPPORT
						PNET_DEV net_dev = pKeyInfo->pNetDev;

						if (net_dev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) {
							/* Get a specific WCID to record this MBSS key attribute */
							GET_GroupKey_WCID(pWdev, Wcid);
						} else if (net_dev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP_VLAN) {
							struct vlan_gtk_info *vg_info = CFG80211_GetVlanInfoByVlandev(pWdev, net_dev);

							if (!vg_info) {
								MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
										"%s(): invalid vlan_dev name=%s addr=%p\n"
										, __func__, net_dev->name, net_dev);
								return FALSE;
							}
							Wcid = vg_info->vlan_bmc_idx;
							os_move_mem(vg_info->vlan_gtk, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
							vg_info->gtk_len = pKeyInfo->KeyLen;
						}
#else
						/* Get a specific WCID to record this MBSS key attribute */
						GET_GroupKey_WCID(pWdev, Wcid);
#endif
						/* Set key material to Asic */
						os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
						os_zero_mem(info, sizeof(ASIC_SEC_INFO));
						info->Operation = SEC_ASIC_ADD_GROUP_KEY;
						info->Direction = SEC_ASIC_KEY_TX;
						info->Wcid = Wcid;
						info->BssIndex = apidx;
						info->Cipher = pWdev->SecConfig.GroupCipher;
						info->KeyIdx = pKeyInfo->KeyId;
						pWdev->SecConfig.GroupKeyId = info->KeyIdx;
						os_move_mem(&info->PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
						/* Install Group key */
						os_move_mem(pWdev->SecConfig.GTK, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
#ifdef RT_CFG80211_SUPPORT
                                                pWdev->Is_hostapd_gtk = 1;
                                                os_move_mem(pWdev->Hostapd_GTK, pWdev->SecConfig.GTK, LEN_MAX_GTK);
#endif
						os_move_mem(info->Key.Key, pWdev->SecConfig.GTK, LEN_MAX_GTK);
						WPAInstallKey(pAd, info, TRUE, TRUE);
						pWdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;
						os_free_mem(info);
					}

#else
					os_move_mem(pAd->SharedKey[apidx][pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
					AsicAddSharedKeyEntry(pAd, apidx, pKeyInfo->KeyId,
										  &pAd->SharedKey[apidx][pKeyInfo->KeyId]);
					GET_GroupKey_WCID(pWdev, Wcid);
#endif
			} else {
					pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

				if (pEntry) {
				switch (pKeyInfo->cipher) {
				case Ndis802_11GCMP256Enable:
					SET_CIPHER_GCMP256(pEntry->SecConfig.PairwiseCipher);
					break;

				case Ndis802_11AESEnable:
					SET_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher);
					break;

				case Ndis802_11TKIPEnable:
					SET_CIPHER_TKIP(pEntry->SecConfig.PairwiseCipher);
					break;

				case Ndis802_11GCMP128Enable:
					SET_CIPHER_GCMP128(pEntry->SecConfig.PairwiseCipher);
					break;

				case Ndis802_11CCMP256Enable:
					SET_CIPHER_CCMP256(pEntry->SecConfig.PairwiseCipher);
					break;

				}
				NdisCopyMemory(&pEntry->SecConfig.PTK[OFFSET_OF_PTK_TK],
						pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);

#if defined(MT7615) || defined(MT7626) || defined(MT7622) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
				if (IS_MT7615(pAd) || IS_MT7626(pAd) || IS_MT7622(pAd) ||
					IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
					IS_MT7981(pAd)) {
						struct _ASIC_SEC_INFO *info = NULL;

						NdisCopyMemory(&pEntry->SecConfig.PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
						/* Set key material to Asic */
						os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));

						if (info) {
							os_zero_mem(info, sizeof(ASIC_SEC_INFO));

							if ((sizeof(pEntry->SecConfig.PTK) - (LEN_PTK_KCK + LEN_PTK_KEK)) >= pKeyInfo->KeyLen) {

								NdisCopyMemory(&pEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
								info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
								info->Direction = SEC_ASIC_KEY_BOTH;
								info->Wcid = pEntry->wcid;
								info->BssIndex = pEntry->func_tb_idx;
								info->Cipher = pEntry->SecConfig.PairwiseCipher;
								info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);/* pEntry->SecConfig.PairwiseKeyId; */
								os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
								os_move_mem(info->Key.Key, &pEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], (LEN_TK + LEN_TK2));
								WPAInstallKey(pAd, info, TRUE, TRUE);
								os_free_mem(info);
							}
							} else {
								MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL,
										 DBG_LVL_ERROR, "struct alloc fail\n");
							}
						}

#else
					PairwiseKey.KeyLen = LEN_TK;
					NdisCopyMemory(&pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
					os_move_mem(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyLen);
					AsicAddPairwiseKeyEntry(pAd, (UCHAR)pEntry->Aid, &pEntry->PairwiseKey);
#endif
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
					{
						UCHAR op_ht_bw1 = wlan_operate_get_ht_bw(pWdev);
						UCHAR op_ht_bw2 = wlan_operate_get_ht_bw(&pAd->StaCfg[0].wdev);

						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "InfraCh=%d, pWdev->channel=%d\n", pAd->MlmeAux.InfraChannel, pWdev->channel);

						if (INFRA_ON(pAd) &&
							(((op_ht_bw2 == op_ht_bw1) && (pAd->StaCfg[0].wdev.channel != pWdev->channel))
							 || !((op_ht_bw2 == op_ht_bw1) && ((pAd->StaCfg[0].wdev.channel == pWdev->channel))))) {
							/*wait 1 s  DHCP  for P2P CLI */
							OS_WAIT(1000);
							MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OS WAIT 1000 FOR DHCP\n");
							/* pAd->MCC_GOConnect_Protect = FALSE; */
							/* pAd->MCC_GOConnect_Count = 0; */
							Start_MCC(pAd);
							MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "infra => GO test\n");
						} else if ((op_ht_bw2 != op_ht_bw1) && ((pAd->StaCfg[0].wdev.channel == pWdev->channel))) {
							MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "start bw !=  && SCC\n");
							pAd->Mlme.bStartScc = TRUE;
						}

						/*after p2p cli connect , neet to change to default configure*/
						if (op_ht_bw1 == HT_BW_20) {
							wlan_operate_set_ht_bw(pWdev, HT_BW_40, EXTCHA_BELOW);
							pAd->CommonCfg.HT_Disable = 0;
							SetCommonHtVht(pAd, pWdev);
						}
					}
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
				} else
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CFG: Set AES Security Set. (PAIRWISE) But pEntry NULL\n");
			}
		}
#ifdef DOT11W_PMF_SUPPORT
			else if (pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_CMAC) {
			/* TKIP */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
			if ((pKeyInfo->bPairwise == FALSE) && (pKeyInfo->KeyId == 4 || pKeyInfo->KeyId == 5))
#else
			if (pKeyInfo->KeyId == 4 || pKeyInfo->KeyId == 5)
#endif	/* LINUX_VERSION_CODE 2.6.37 */
			{
				hex_dump("PMF IGTK pKeyInfo->KeyBuf=", (UINT8 *)pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PMF IGTK pKeyInfo->KeyId=%d\n", pKeyInfo->KeyId);
#if defined(MT7615) || defined(MT7626) || defined(MT7622) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
					if (IS_MT7615(pAd) || IS_MT7626(pAd) || IS_MT7622(pAd) ||
						IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
						IS_MT7981(pAd)) {
						PPMF_CFG pPmfCfg = &pWdev->SecConfig.PmfCfg;
						struct _ASIC_SEC_INFO *info = NULL;
						USHORT Wcid;
						if (pKeyInfo->cipher == Ndis802_11AESEnable)
						SET_CIPHER_CCMP128(pWdev->SecConfig.GroupCipher);
				else if (pKeyInfo->cipher == Ndis802_11TKIPEnable)
						SET_CIPHER_TKIP(pWdev->SecConfig.GroupCipher);
						/* Get a specific WCID to record this MBSS key attribute */
						GET_GroupKey_WCID(pWdev, Wcid);
						/* Set key material to Asic */
						os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
						os_zero_mem(info, sizeof(ASIC_SEC_INFO));
						info->Operation = SEC_ASIC_ADD_GROUP_KEY;
						info->Direction = SEC_ASIC_KEY_TX;
						info->Wcid = Wcid;
						info->BssIndex = apidx;
						info->Cipher = pWdev->SecConfig.GroupCipher;
						info->KeyIdx = pWdev->SecConfig.GroupKeyId;
						info->IGTKKeyLen = LEN_TK;
						os_move_mem(&info->PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
						os_zero_mem(&pPmfCfg->IPN[pKeyInfo->KeyId - 4][0], LEN_WPA_TSC);
						/* Install Shared key */
						os_move_mem(&pPmfCfg->IGTK[pKeyInfo->KeyId - 4][0], pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
						os_move_mem(info->Key.Key, pWdev->SecConfig.GTK, LEN_MAX_GTK);
						os_move_mem(info->IGTK, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
						WPAInstallKey(pAd, info, TRUE, TRUE);
						pWdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;
						os_free_mem(info);
					}

#else
					os_move_mem(pAd->SharedKey[apidx][pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
					AsicAddSharedKeyEntry(pAd, apidx, pKeyInfo->KeyId,
										  &pAd->SharedKey[apidx][pKeyInfo->KeyId]);
					GET_GroupKey_WCID(pWdev, Wcid);
					RTMPSetWcidSecurityInfo(pAd, apidx, (UINT8)(pKeyInfo->KeyId),
					pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg, Wcid, SHAREDKEYTABLE);
#ifdef MT_MAC

					if (IS_HIF_TYPE(pAd, HIF_MT))
				CmdProcAddRemoveKey(pAd, 0, apidx, pKeyInfo->KeyId, Wcid, SHAREDKEYTABLE,
										   &pAd->SharedKey[apidx][pKeyInfo->KeyId], BROADCAST_ADDR);

#endif /* MT_MAC */
#endif /* MT7615 */
				}
		else {
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR !! pKeyInfo->bPairwise = %d pKeyInfo->KeyId=%d \n", pKeyInfo->bPairwise, pKeyInfo->KeyId);
#else
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR !! pKeyInfo->KeyId=%d \n", pKeyInfo->KeyId);
#endif /* LINUX_VERSION_CODE 2.6.37 */
		}
	}
#endif /* DOT11W_PMF_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

INT CFG80211_StaPortSecured(
	IN VOID                                         *pAdCB,
	IN UCHAR					*pMac,
	IN UINT						flag)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;

	pEntry = MacTableLookup(pAd, pMac);

	if (!pEntry)
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Can't find pEntry in CFG80211_StaPortSecured\n");
	else {
		tr_entry = &pAd->tr_ctl.tr_entry[pEntry->wcid];

		if (flag) {
			/* Update status and set Port as Secured */
			pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)

			if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
				IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
				IS_MT7981(pAd)) {
				pEntry->SecConfig.Handshake.WpaState = AS_PTKINITDONE;
				WifiSysUpdatePortSecur(pAd, pEntry, NULL);
			}

#else
			pEntry->WpaState = AS_PTKINITDONE;
#endif
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AID:%d, PortSecured\n", pEntry->Aid);
		} else {
			pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
			tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AID:%d, PortNotSecured\n", pEntry->Aid);
		}
	}

	return 0;
}


#ifdef HOSTAPD_MAP_SUPPORT
INT CFG80211_ApStaDel(
	IN VOID                                         *pAdCB,
	IN VOID                                         *pData,
	IN UINT						reason)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	MAC_TABLE_ENTRY *pEntry;
	CMD_RTPRIV_IOCTL_AP_STA_DEL *pApStaDelInfo = NULL;
	PUCHAR pMac = NULL;

	pApStaDelInfo = (CMD_RTPRIV_IOCTL_AP_STA_DEL *)pData;

	if (pApStaDelInfo->pSta_MAC != NULL)
		pMac = (PUCHAR)pApStaDelInfo->pSta_MAC;

	if (pMac == NULL) {
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		/* From WCID=2 */
		if (INFRA_ON(pAd))
			;/* P2PMacTableReset(pAd); */
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
		{
			if (pApStaDelInfo->pWdev != NULL)
				MacTableResetWdev(pAd, pApStaDelInfo->pWdev);
			else
				MacTableReset(pAd);
			NdisZeroMemory(pAd->radius_tbl, MAX_LEN_OF_MAC_TABLE * sizeof(RADIUS_ACCOUNT_ENTRY));
		}
	} else {
		int i;
		pEntry = MacTableLookup(pAd, pMac);
		if (pEntry) {
			USHORT reason_code = REASON_NO_LONGER_VALID;
#ifdef HOSTAPD_OWE_SUPPORT
			if (pEntry->wdev && (pEntry->Sst == SST_AUTH)
				&& IS_AKM_OWE(pEntry->wdev->SecConfig.AKMMap))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\n OWE mode Ignore Deauth from Hostapd");
			else

#endif
{
			if (reason)
				reason_code = reason;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL,
				DBG_LVL_ERROR,
				"%s, Deauth : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x reason code %d\n", __func__,
				 pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5], reason_code);

				MlmeDeAuthAction(pAd, pEntry, reason_code, FALSE);
}
		} else
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Can't find pEntry in ApStaDel\n");

		/* Find entry in radius tbl and delete it if found */

		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
			RADIUS_ACCOUNT_ENTRY *pRadiusEntry = &pAd->radius_tbl[i];

			if (MAC_ADDR_EQUAL(pRadiusEntry->Addr, pMac))
				NdisZeroMemory(pRadiusEntry, sizeof(RADIUS_ACCOUNT_ENTRY));
		}
	}
	return 0;
}

#else
INT CFG80211_ApStaDel(
	IN VOID                                         *pAdCB,
	IN UCHAR                                        *pMac,
	IN UINT						reason)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	MAC_TABLE_ENTRY *pEntry;

	if (pMac == NULL) {
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		/* From WCID=2 */
		if (INFRA_ON(pAd))
			;/* P2PMacTableReset(pAd); */
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
		{
			/* MacTableReset(pAd); */
			/* NdisZeroMemory(pAd->radius_tbl, MAX_LEN_OF_MAC_TABLE * sizeof(RADIUS_ACCOUNT_ENTRY)); */
		}

	} else {
		int i;
#ifdef	HOSTAPD_HS_R2_SUPPORT
		BSS_STRUCT *pMbss;
#endif

		pEntry = MacTableLookup(pAd, pMac);
		if (pEntry != NULL) {
			USHORT reason_code = REASON_NO_LONGER_VALID;
#ifdef HOSTAPD_OWE_SUPPORT
			if ((pEntry->wdev != NULL) && (pEntry->Sst == SST_AUTH)
				&& IS_AKM_OWE(pEntry->wdev->SecConfig.AKMMap))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\n OWE mode Ignore Deauth from Hostapd");
			else

#endif
{
			if (reason)
				reason_code = reason;
#ifdef	HOSTAPD_HS_R2_SUPPORT
			pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
			if (pMbss && pMbss->HotSpotCtrl.HotSpotEnable) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n Sending Disassoc for Hotspot case");
				APMlmeKickOutSta(pAd, pMac, pEntry->wcid, reason_code);
			}
#endif
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL,
				DBG_LVL_INFO,
				"Deauth : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x reason code %d\n",
				 pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5], reason_code);

					MlmeDeAuthAction(pAd, pEntry, reason_code, FALSE);
}
		} else
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Can't find pEntry in ApStaDel\n");

		/* Find entry in radius tbl and delete it if found */

		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
			RADIUS_ACCOUNT_ENTRY *pRadiusEntry = &pAd->radius_tbl[i];

			if (MAC_ADDR_EQUAL(pRadiusEntry->Addr, pMac))
				NdisZeroMemory(pRadiusEntry, sizeof(RADIUS_ACCOUNT_ENTRY));
		}
	}
	return 0;
}
#endif

#ifdef HOSTAPD_PMKID_IN_DRIVER_SUPPORT
INT CFG80211_ApUpdateStaPmkid(
	IN VOID 								*pAdCB,
	IN VOID 								*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	MAC_TABLE_ENTRY *pEntry = NULL;
	RT_CMD_AP_IOCTL_UPDATE_PMKID *pApPmkidEntry = NULL;

	pApPmkidEntry = (RT_CMD_AP_IOCTL_UPDATE_PMKID *)pData;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"==sta:"MACSTR"\n",
					MAC2STR(pApPmkidEntry->sta));

	pEntry = MacTableLookup(pAd, pApPmkidEntry->sta);

	/*to do: also check if pEntry belong to same bssid, as sent by hostapd*/

	if (pEntry) {
		if (pApPmkidEntry->AddRemove) {
			/*Add pmkid in existing pEntry*/
			NdisCopyMemory(pEntry->PmkidByHostapd, pApPmkidEntry->pmkid, LEN_PMKID);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"added PMKID in pEntryAddress: "MACSTR"\n",
						MAC2STR(pEntry->Addr));
			hex_dump_with_lvl("PMKID:", pEntry->PmkidByHostapd, LEN_PMKID, DBG_LVL_INFO);
		} else {
			/*Remove pmkid from existing pEntry*/
			NdisZeroMemory(pEntry->PmkidByHostapd, LEN_PMKID);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"removed PMKID from pEntryAddress: "MACSTR"\n",
						MAC2STR(pEntry->Addr));
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Can't find pEntry in MacTable to AddRemove:%d\n",
				pApPmkidEntry->AddRemove);
	}

	return 0;
}
#endif


INT CFG80211_setApDefaultKey(
	IN VOID                    *pAdCB,
	IN struct net_device		*pNetdev,
	IN UINT 					Data
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	INT32 apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetdev);

	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS. \n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Ap Default Key: %d\n", Data);
#ifdef RT_CFG80211_P2P_SUPPORT
    pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX].wdev.DefaultKeyId = Data;
#else
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)

	pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId = Data;
#else
	pAd->ApCfg.MBSSID[apidx].wdev.DefaultKeyId = Data;
#endif
#endif /*RT_CFG80211_P2P_SUPPORT*/

	return 0;
}

#ifdef DOT11W_PMF_SUPPORT
INT CFG80211_setApDefaultMgmtKey(
	IN VOID                    *pAdCB,
	IN struct net_device		*pNetdev,
	IN UINT 					Data
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	INT32 apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetdev);
	struct wifi_dev *pWdev = NULL;

	if (apidx == WDEV_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev in driver MBSS. \n");
		return FALSE;
	}
	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;

#if defined(MT7615) || defined(MT7626) || defined(MT7622) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
	if ((Data == 4) || (Data == 5)) {
		pWdev->SecConfig.PmfCfg.IGTK_KeyIdx = (UINT8)Data;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Ap Default Mgmt KeyId: %d\n", Data);
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "fail - [ERROR]Invalid Mgmt KeyId: %d\n", Data);
	}
#endif

	return 0;
}
#endif /*DOT11W_PMF_SUPPORT*/

INT CFG80211_ApStaDelSendEvent(PRTMP_ADAPTER pAd, const PUCHAR mac_addr, IN PNET_DEV pNetDevIn)
{
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	PNET_DEV pNetDev = NULL;

	pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_GO);

	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		(pNetDev != NULL)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CONCURRENT_DEVICE CFG : GO NOITFY THE CLIENT Disconnected\n");
		CFG80211OS_DelSta(pNetDev, mac_addr);
	} else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
	{
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SINGLE_DEVICE CFG : GO NOITFY THE CLIENT Disconnected\n");
		CFG80211OS_DelSta(pNetDevIn, mac_addr);
	}

	return 0;
}

#ifdef APCLI_CFG80211_SUPPORT
VOID CFG80211_ApClientConnectResultInform(
	VOID *pAdCB, UCHAR *pBSSID, UCHAR ifIndex, UCHAR *pReqIe, UINT32 ReqIeLen,
	UCHAR *pRspIe, UINT32 RspIeLen, UCHAR FlgIsSuccess)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	struct wifi_dev *wdev = &pAd->StaCfg[ifIndex].wdev;

	if ((wdev->open_state == TRUE) && (pAd->cfg80211_ctrl.FlgCfg80211Connecting == TRUE)) {
		CFG80211OS_P2pClientConnectResultInform(wdev->if_dev, pBSSID,
						pReqIe, ReqIeLen, pRspIe, RspIeLen, FlgIsSuccess);

		pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
	}
	pAd->StaCfg[ifIndex].ReadyToConnect = FALSE;
}
#endif /* APCLI_CFG80211_SUPPORT */


#endif /* CONFIG_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
