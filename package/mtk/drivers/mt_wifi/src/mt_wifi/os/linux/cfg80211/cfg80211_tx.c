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
 *	All related CFG80211 P2P function body.
 *
 *	History:
 *
 ****************************************************************************/
#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

UCHAR    OSEN_OUI[4] = {0x50, 0x6F, 0x9A, 0x01};


#ifdef GREENAP_SUPPORT
BOOLEAN greenap_get_allow_status(RTMP_ADAPTER *ad);
#endif

VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data)
{
	/* UCHAR lock_channel = CFG80211_getCenCh(pAd, Data); */
	UCHAR lock_channel = Data;

    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    struct wifi_dev *wdev;

	if (RTMP_CFG80211_HOSTAPD_ON(pAd))
		return;
    wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
	struct wifi_dev *wdev = &pMbss->wdev;

	if (pAd->Mlme.bStartMcc == TRUE)
		return;

	if (pAd->Mlme.bStartScc == TRUE) {
		/* MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SCC Enabled, Do not switch channel for Tx  %d\n",lock_channel); */
		return;
	}

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd) && (wdev->channel == lock_channel) && (wlan_operate_get_ht_bw(wdev) == HT_BW_40)) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "40 BW Enabled || GO enable , wait for CLI connect, Do not switch channel for Tx\n");
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "GO wdev->channel  %d  lock_channel %d\n", wdev->channel, lock_channel);
		return;
	}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	struct wifi_dev *p2p_dev = &pAd->StaCfg[0].wdev;
	UCHAR cen_ch = wlan_operate_get_cen_ch_1(p2p_dev);

	if (INFRA_ON(pAd) &&
		(((pAd->LatchRfRegs.Channel != cen_ch) && (cen_ch != 0)))
		|| (pAd->LatchRfRegs.Channel != lock_channel))
#else
	if (pAd->LatchRfRegs.Channel != lock_channel)
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	{

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
		wlan_operate_set_prim_ch(p2p_dev, lock_channel);
#else
		wlan_operate_set_prim_ch(wdev, lock_channel);
#endif

		MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Off-Channel Send Packet: From(%d)-To(%d)\n",
				 pAd->LatchRfRegs.Channel, lock_channel);
	} else
		MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Off-Channel Channel Equal: %d\n", pAd->LatchRfRegs.Channel);
}

#ifdef CONFIG_AP_SUPPORT

#ifdef DISABLE_HOSTAPD_PROBE_RESP
/*
	==========================================================================
	Description:
		Process the received ProbeRequest from clients for hostapd
	Parameters:
		apidx
	==========================================================================
 */
extern INT build_country_power_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf);
extern INT build_ch_switch_announcement_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf);

VOID ProbeResponseHandler(
	IN PRTMP_ADAPTER pAd,
	IN PEER_PROBE_REQ_PARAM *ProbeReqParam,
	IN UINT8 apidx)

{
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0, TmpLen;
	struct legacy_rate *rate;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL * pQloadCtrl = NULL;
#endif /* AP_QLOAD_SUPPORT */
	ADD_HT_INFO_IE *addht;
	UCHAR cfg_ht_bw;
	UCHAR op_ht_bw;
	LARGE_INTEGER FakeTimestamp;
	UCHAR DsLen = 1;
	UCHAR ErpIeLen = 1;
	UCHAR PhyMode, SupRateLen;
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
	CHAR rsne_idx = 0;
	struct _build_ie_info ie_info = {0};
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	struct customer_vendor_ie *ap_vendor_ie;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#ifdef QOS_R2
	UCHAR	tmpbuf[50] = {0}, ielen = 0;
#endif

	mbss = &pAd->ApCfg.MBSSID[apidx];

	wdev = &mbss->wdev;
	rate = &wdev->rate.legacy_rate;
	addht = wlan_operate_get_addht(wdev);
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	op_ht_bw = wlan_config_get_ht_bw(wdev);
	PhyMode = wdev->PhyMode;
	ie_info.frame_subtype = SUBTYPE_PROBE_RSP;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev;

	if (((((ProbeReqParam->SsidLen == 0) && (!mbss->bHideSsid)) ||
		((ProbeReqParam->SsidLen == mbss->SsidLen) && NdisEqualMemory(ProbeReqParam->Ssid, mbss->Ssid, (ULONG) ProbeReqParam->SsidLen)))
#ifdef CONFIG_HOTSPOT
			   && ProbeReqforHSAP(pAd, apidx, ProbeReqParam)
#endif
	     )
		) {
			;
		} else {
						return;
		}

#ifdef DOT11V_MBSSID_SUPPORT
	if (wdev && IS_BSSID_11V_NON_TRANS(pAd, wdev->func_dev, HcGetBandByWdev(wdev))) {
		if ((ProbeReqParam->SsidLen == mbss->SsidLen) &&
			NdisEqualMemory(ProbeReqParam->Ssid, mbss->Ssid, (ULONG)ProbeReqParam->SsidLen)) {
			UINT8 DbdcIdx = HcGetBandByWdev(wdev);

			wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.dot11v_trans_bss_idx[DbdcIdx]].wdev;
			rate = &wdev->rate.legacy_rate;
			addht = wlan_operate_get_addht(wdev);
			cfg_ht_bw = wlan_config_get_ht_bw(wdev);
			op_ht_bw = wlan_config_get_ht_bw(wdev);
			PhyMode = wdev->PhyMode;
			ie_info.wdev = wdev;
			ie_info.channel = wdev->channel;
			mbss = (BSS_STRUCT *)wdev->func_dev;
			if (!IS_MBSSID_IE_NEEDED(pAd, mbss, DbdcIdx))
				return;

			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s(): Transmitting probe response for Non Tx SSID with Trans bss idx %d\n",
				__func__, wdev->wdev_idx);
		} else
			return;
	}
#endif /* DOT11V_MBSSID_SUPPORT */


#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	if ((ProbeReqParam->report_param.vendor_ie.element_id == IE_VENDOR_SPECIFIC) &&
		(ProbeReqParam->report_param.vendor_ie.len > 0)) {
		struct probe_req_report pProbeReqReportTemp;

		memset(&pProbeReqReportTemp, 0, sizeof(struct probe_req_report));
		pProbeReqReportTemp.band = (WMODE_CAP_2G(wdev->PhyMode) && wdev->channel <= 14) ? 0 : 1;
		COPY_MAC_ADDR(pProbeReqReportTemp.sta_mac, ProbeReqParam->Addr2);
		pProbeReqReportTemp.vendor_ie.element_id = ProbeReqParam->report_param.vendor_ie.element_id;
		pProbeReqReportTemp.vendor_ie.len = ProbeReqParam->report_param.vendor_ie.len;
		NdisMoveMemory(pProbeReqReportTemp.vendor_ie.custom_ie,
				ProbeReqParam->report_param.vendor_ie.custom_ie,
				ProbeReqParam->report_param.vendor_ie.len);
		RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, RT_PROBE_REQ_REPORT_EVENT,
					NULL, (PUCHAR)&pProbeReqReportTemp,
					MAC_ADDR_LEN + 3 + ProbeReqParam->report_param.vendor_ie.len);
	}
#endif

		/* allocate and send out ProbeRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

#ifdef RED_SUPPORT
	if (MAC_ADDR_EQUAL(ProbeReqParam->Addr2, IXIA_PROBE_ADDR))
		pAd->ixia_mode_ctl.fgProbeRspDetect = TRUE;
#endif
#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev)
		&& MAC_ADDR_EQUAL(ProbeReqParam->Addr1, BROADCAST_ADDR)
		&& ProbeReqParam->IsOceCapability) /* broadcast probe request && is OCE STA*/
		MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, BROADCAST_ADDR,
						 wdev->if_addr, wdev->bssid); /* broadcast probe response */
	else
#endif /* OCE_SUPPORT */
		MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, ProbeReqParam->Addr2,
							wdev->if_addr, wdev->bssid);


		{
		SupRateLen = rate->sup_rate_len;

		if (PhyMode == WMODE_B)
			SupRateLen = 4;

		MakeOutgoingFrame(pOutBuffer,				  &FrameLen,
						  sizeof(HEADER_802_11),	  &ProbeRspHdr,
						  TIMESTAMP_LEN,			  &FakeTimestamp,
						  2,						  &pAd->CommonCfg.BeaconPeriod,
						  2,						  &mbss->CapabilityInfo,
						  1,						  &SsidIe,
						  1,						  &mbss->SsidLen,
						  mbss->SsidLen,	 mbss->Ssid,
						  1,						  &SupRateIe,
						  1,						  &SupRateLen,
						  SupRateLen,				  rate->sup_rate,
						  1,						  &DsIe,
						  1,						  &DsLen,
						  1,						  &wdev->channel,
						  END_OF_ARGS);
		}

	if ((rate->ext_rate_len) && (PhyMode != WMODE_B)) {
			MakeOutgoingFrame(pOutBuffer+FrameLen,		&TmpLen,
							  1,						&ErpIe,
							  1,						&ErpIeLen,
							  1,						&pAd->ApCfg.ErpIeContent,
							  END_OF_ARGS);
			FrameLen += TmpLen;
	}


	FrameLen += build_support_ext_rate_ie(wdev, rate->sup_rate_len,
			rate->ext_rate, rate->ext_rate_len, pOutBuffer + FrameLen);

#ifdef CONFIG_HOTSPOT_R2
	if ((mbss->HotSpotCtrl.HotSpotEnable == 0) && (mbss->HotSpotCtrl.bASANEnable == 1) && (IS_AKM_WPA2_Entry(wdev))) {
			/* replace RSN IE with OSEN IE if it's OSEN wdev */
		UCHAR RSNIe = IE_WPA;
		extern UCHAR			OSEN_IE[];
		extern UCHAR			OSEN_IELEN;

			MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							  1, &RSNIe,
							  1, &OSEN_IELEN,
							  OSEN_IELEN, OSEN_IE,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		} else
#endif /* CONFIG_HOTSPOT_R2 */
		{
#ifdef DISABLE_HOSTAPD_PROBE_RESP
		for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
				BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
				if (mbss->RSNIE_Len[rsne_idx] != 0) {
					MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
						1, &mbss->RSNIE_ID[rsne_idx],
						1, &mbss->RSNIE_Len[rsne_idx],
						mbss->RSNIE_Len[rsne_idx], &mbss->RSN_IE[rsne_idx][0],
						END_OF_ARGS);
					FrameLen += TmpLen;
				}
		}
#else
			FrameLen += build_rsn_ie(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));
#endif  /*DISABLE_HOSTAPD_PROBE_RESP */
		}
#ifdef DOT11V_MBSSID_SUPPORT
	make_multiple_bssid_ie(pAd, wdev, &FrameLen, pOutBuffer,
				pAd->ApCfg.dot11v_mbssid_bitmap[HcGetBandByWdev(wdev)], TRUE);
#endif /* DOT11V_MBSSID_SUPPORT */

#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(PhyMode) &&
		(wdev->DesiredHtPhyInfo.bHtEnable)) {
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_ht_ies(pAd, &ie_info);
	}

#endif /* DOT11_N_SUPPORT */
#ifdef HOSTAPD_OWE_SUPPORT
	if (mbss->TRANSIE_Len) {
		ULONG TmpLen;

		MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							 mbss->TRANSIE_Len, mbss->TRANS_IE,
							 END_OF_ARGS);
		FrameLen += TmpLen;
	}

#endif
#ifdef QOS_R2
		if (mbss->bDSCPPolicyEnable) {
			QoS_Build_WFACapaIE(tmpbuf, &ielen, mbss->bDSCPPolicyEnable);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen, ielen, tmpbuf, END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif

		/* Extended Capabilities IE */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_extended_cap_ie(pAd, &ie_info);
#ifdef AP_QLOAD_SUPPORT
		if (pAd->CommonCfg.dbdc_mode == 0)
			pQloadCtrl = HcGetQloadCtrl(pAd);
		else
			pQloadCtrl = (wdev->channel > 14) ? HcGetQloadCtrlByRf(pAd, RFIC_5GHZ) : HcGetQloadCtrlByRf(pAd, RFIC_24GHZ);

		if (pQloadCtrl != NULL) {
			if (pQloadCtrl->FlgQloadEnable != 0) {
#ifdef CONFIG_HOTSPOT_R2

			if (mbss->HotSpotCtrl.QLoadTestEnable == 1)
				FrameLen += QBSS_LoadElementAppend_HSTEST(pAd, pOutBuffer + FrameLen, wdev->func_idx);
			else if (mbss->HotSpotCtrl.QLoadTestEnable == 0)
#endif /* CONFIG_HOTSPOT_R2 */
				FrameLen += QBSS_LoadElementAppend(pAd, pOutBuffer+FrameLen, pQloadCtrl, wdev->func_idx);
			}
		}
#endif /* AP_QLOAD_SUPPORT */
#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)
	if (mbss->HotSpotCtrl.HotSpotEnable)
		MakeHotSpotIE(wdev, &FrameLen, pOutBuffer);

#endif /* defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT) */
#ifdef CONFIG_DOT11U_INTERWORKING
	if (mbss->GASCtrl.b11U_enable) {
		ULONG TmpLen;
		RTMP_SEM_LOCK(&mbss->GASCtrl.IeLock);
		/* Interworking element */
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  mbss->GASCtrl.InterWorkingIELen,
						  mbss->GASCtrl.InterWorkingIE, END_OF_ARGS);
		FrameLen += TmpLen;
		/* Advertisement Protocol element */
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  mbss->GASCtrl.AdvertisementProtoIELen,
						  mbss->GASCtrl.AdvertisementProtoIE, END_OF_ARGS);
		FrameLen += TmpLen;
		RTMP_SEM_UNLOCK(&mbss->GASCtrl.IeLock);
	}
#endif /* CONFIG_DOT11U_INTERWORKING */

		/* add WMM IE here */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);
#ifdef DOT11K_RRM_SUPPORT
	if (IS_RRM_ENABLE(wdev)) {
		RRM_InsertRRMEnCapIE(pAd, wdev, pOutBuffer + FrameLen, &FrameLen, wdev->func_idx);
			InsertChannelRepIE(pAd, pOutBuffer+FrameLen, &FrameLen,
				(RTMP_STRING *)pAd->CommonCfg.CountryCode,
				get_regulatory_class(pAd, mbss->wdev.channel, mbss->wdev.PhyMode, &mbss->wdev),
					   NULL, PhyMode, wdev->func_idx);
#ifndef APPLE_11K_IOT
			/* Insert BSS AC Access Delay IE. */
			RRM_InsertBssACDelayIE(pAd, pOutBuffer+FrameLen, &FrameLen);
			/* Insert BSS Available Access Capacity IE. */
			RRM_InsertBssAvailableACIE(pAd, pOutBuffer+FrameLen, &FrameLen);
#endif /* !APPLE_11K_IOT */
	}
#endif /* DOT11K_RRM_SUPPORT */
#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev)) { /* some OCE STA may only have files ie(without oce CAP) */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += oce_build_ies(pAd, &ie_info, TRUE);
				}
#endif /* OCE_SUPPORT */
	FrameLen += build_country_power_ie(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));
		/* add Channel switch announcement IE */
	FrameLen += build_ch_switch_announcement_ie(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));
#ifdef DOT11_N_SUPPORT

		if (WMODE_CAP_N(PhyMode) &&
			(wdev->DesiredHtPhyInfo.bHtEnable)) {
		if (pAd->bBroadComHT == TRUE) {
				ie_info.is_draft_n_type = TRUE;
				ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
				FrameLen += build_ht_ies(pAd, &ie_info);
			}
#ifdef DOT11_VHT_AC
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			ucETxBfCap = wlan_config_get_etxbf(wdev);

			if (HcIsBfCapSupport(wdev) == FALSE)
				wlan_config_set_etxbf(wdev, SUBF_OFF);

#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_vht_ies(pAd, &ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#ifdef DOT11_HE_AX
		if (WMODE_CAP_AX(wdev->PhyMode) && wdev->DesiredHtPhyInfo.bHtEnable)
			FrameLen += add_probe_rsp_he_ies(wdev, (UINT8 *)pOutBuffer, FrameLen);
#endif /*DOT11_HE_AX*/
#endif /* DOT11_VHT_AC */
	}

#endif /* DOT11_N_SUPPORT */

#ifdef WSC_AP_SUPPORT
		/* for windows 7 logo test */
		if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
#ifdef DOT1X_SUPPORT
				(!IS_IEEE8021X_Entry(wdev)) &&
#endif /* DOT1X_SUPPORT */
				(IS_CIPHER_WEP(wdev->SecConfig.PairwiseCipher))) {
			/*
				Non-WPS Windows XP and Vista PCs are unable to determine if a WEP enalbed network is static key based
				or 802.1X based. If the legacy station gets an EAP-Request/Identity from the AP, it assume the WEP
				network is 802.1X enabled & will prompt the user for 802.1X credentials. If the legacy station doesn't
				receive anything after sending an EAPOL-Start, it will assume the WEP network is static key based and
				prompt user for the WEP key. <<from "WPS and Static Key WEP Networks">>
				A WPS enabled AP should include this IE in the beacon when the AP is hosting a static WEP key network.
				The IE would be 7 bytes long with the Extended Capability field set to 0 (all bits zero)
				http://msdn.microsoft.com/library/default.asp?url=/library/en-us/randz/protocol/securing_public_wi-fi_hotspots.asp
			*/
			ULONG TempLen1 = 0;
			UCHAR PROVISION_SERVICE_IE[7] = {0xDD, 0x05, 0x00, 0x50, 0xF2, 0x05, 0x00};

			MakeOutgoingFrame(pOutBuffer+FrameLen,		  &TempLen1,
								7,							  PROVISION_SERVICE_IE,
								END_OF_ARGS);
			FrameLen += TempLen1;
		}

		/* add Simple Config Information Element */
#ifdef DISABLE_HOSTAPD_PROBE_RESP
		if (wdev->WscIEProbeResp.ValueLen)
#else
		if ((wdev->WscControl.WscConfMode > WSC_DISABLE) && (wdev->WscIEProbeResp.ValueLen))
#endif
		{
			ULONG WscTmpLen = 0;

			MakeOutgoingFrame(pOutBuffer+FrameLen,									&WscTmpLen,
							  wdev->WscIEProbeResp.ValueLen,   wdev->WscIEProbeResp.Value,
							  END_OF_ARGS);
			FrameLen += WscTmpLen;
		}
#endif /* WSC_AP_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

	/*
	   The Mobility Domain information element (MDIE) is present in Probe-
	 * Request frame when dot11FastBssTransitionEnable is set to true.
	  */
	if (wdev->FtCfg.FtCapFlag.Dot11rFtEnable) {
		PFT_CFG pFtCfg = &wdev->FtCfg;
			FT_CAP_AND_POLICY FtCap;

			FtCap.field.FtOverDs = pFtCfg->FtCapFlag.FtOverDs;
			FtCap.field.RsrReqCap = pFtCfg->FtCapFlag.RsrReqCap;
			FT_InsertMdIE(pOutBuffer + FrameLen, &FrameLen,
							pFtCfg->FtMdId, FtCap);
		}

#endif /* DOT11R_FT_SUPPORT */
#if defined(HOSTAPD_MAPR3_SUPPORT) && defined(DPP_R2_SUPPORT)
	/* Add CCE IE in probe Response for DPP*/
	if (wdev->DPPCfg.cce_ie_len) {
		ULONG DPPIeTmpLen = 0;

		MakeOutgoingFrame(pOutBuffer + FrameLen, &DPPIeTmpLen,
						wdev->DPPCfg.cce_ie_len, wdev->DPPCfg.cce_ie_buf,
						END_OF_ARGS);
		FrameLen += DPPIeTmpLen;
	}
#endif


	/*
		add Ralink-specific IE here - Byte0.b0=1 for aggregation,
		Byte0.b1=1 for piggy-back, Byte0.b3=1 for rssi-feedback
	*/
	FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_PROBE_RESP
								);
#if defined(MBO_SUPPORT) || defined(OCE_SUPPORT)
	if (IS_MBO_ENABLE(wdev) || IS_OCE_ENABLE(wdev))
		MakeMboOceIE(pAd, wdev, NULL, pOutBuffer+FrameLen, &FrameLen, MBO_FRAME_TYPE_PROBE_RSP);
#endif /* MBO_SUPPORT OCE_SUPPORT*/

	{
		/* Question to Rorscha: bit4 in old chip is used? but currently is using for 2.4G 256QAM */
#ifdef RSSI_FEEDBACK
		UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
		ULONG TmpLen;

		if (ProbeReqParam->bRequestRssi == TRUE) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SYNC - Send PROBE_RSP to "MACSTR"...\n",
										MAC2STR(ProbeReqParam->Addr2));
			RalinkSpecificIe[5] |= 0x8;
			pEntry = MacTableLookup(pAd, ProbeReqParam->Addr2);

			if (pEntry != NULL) {
				RalinkSpecificIe[6] = (UCHAR)pEntry->RssiSample.AvgRssi[0];
				RalinkSpecificIe[7] = (UCHAR)pEntry->RssiSample.AvgRssi[1];
				RalinkSpecificIe[8] = (UCHAR)pEntry->RssiSample.AvgRssi[2];
			}
		}

		MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							9, RalinkSpecificIe,
							END_OF_ARGS);
		FrameLen += TmpLen;
#endif /* RSSI_FEEDBACK */
	}

#ifdef OCE_FILS_SUPPORT
	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += oce_build_ies(pAd, &ie_info, TRUE);
#endif /*OCE_FILS_SUPPORT */

#ifdef HOSTAPD_WPA3R3_SUPPORT
	FrameLen +=  build_rsnxe_ie(wdev, &wdev->SecConfig, (UCHAR *)pOutBuffer + FrameLen);
#endif

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	{
		MAC_TABLE_ENTRY *pEntry = NULL;

		pEntry = MacTableLookup(pAd, ProbeReqParam->Addr2);
		if (pEntry != NULL) {
			ap_vendor_ie = &pAd->ApCfg.MBSSID[pEntry->apidx].ap_vendor_ie;
			RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);
			if (ap_vendor_ie->pointer != NULL) {
				ULONG TmpLen;

				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"SYNC - Send Probe response to "MACSTR"...and add vendor ie\n",
					MAC2STR(ProbeReqParam->Addr2));
				MakeOutgoingFrame(pOutBuffer + FrameLen,
						&TmpLen,
						ap_vendor_ie->length,
						ap_vendor_ie->pointer,
						END_OF_ARGS);
				FrameLen += TmpLen;
			}
			RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);
	}
	}
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	/* 802.11n 11.1.3.2.2 active scanning. sending probe response with MCS rate is */
	/* configure to better support Multi-Sta */
	{
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		UINT8 idx = 0;
		UINT8 num = cap->ProbeRspTimes;

		num = (pAd->ApCfg.BssidNum >= 8) ? 1 : num;
		for (idx = 0; idx < num; idx++)
			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	}
	MlmeFreeMemory(pOutBuffer);
	return;
}

VOID CFG80211_SyncPacketWpsIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen, UINT8 apidx, UINT8 *da)
{

	const UCHAR *ssid_ie = NULL;
#ifdef WSC_AP_SUPPORT
	const UCHAR *wsc_ie  = NULL;
	const UINT WFA_OUI = 0x0050F2;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
#endif
	EID_STRUCT *eid;

	PEER_PROBE_REQ_PARAM ProbeReqParam;

	NdisZeroMemory(&ProbeReqParam, sizeof(PEER_PROBE_REQ_PARAM));
	ssid_ie = cfg80211_find_ie(WLAN_EID_SSID, pData, dataLen);
	if (ssid_ie != NULL) {
		eid = (EID_STRUCT *)ssid_ie;
		ProbeReqParam.SsidLen = eid->Len;
		NdisCopyMemory(ProbeReqParam.Ssid, ssid_ie+2, eid->Len);
		NdisCopyMemory(ProbeReqParam.Addr2, da, MAC_ADDR_LEN);

	}
#ifdef WSC_AP_SUPPORT
	wsc_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, 4, pData, dataLen);
	if (wsc_ie != NULL) {

		eid = (EID_STRUCT *)wsc_ie;

		if (eid->Len + 2 <= 500) {
			NdisCopyMemory(pMbss->wdev.WscIEProbeResp.Value, wsc_ie, eid->Len + 2);
			pMbss->wdev.WscIEProbeResp.ValueLen = eid->Len + 2;
		}

	}
#endif
	ProbeResponseHandler(pAd, &ProbeReqParam, apidx);
}

#endif /*DISABLE_HOSTAPD_PROBE_RESP */

BOOLEAN CFG80211_SyncPacketWmmIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen)
{
	const UINT WFA_OUI = 0x0050F2;
	const UCHAR WMM_OUI_TYPE = 0x2;
	UCHAR *wmm_ie = NULL;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[BSS0].wdev;
	EDCA_PARM *pBssEdca = NULL;
	/* hex_dump("probe_rsp_in:", pData, dataLen); */
	wmm_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, WMM_OUI_TYPE, pData, dataLen);

	if (wmm_ie != NULL) {
		UINT i = 0;
#ifdef UAPSD_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX].wdev;
		if (wdev->UapsdInfo.bAPSDCapable == TRUE) {
			wmm_ie[8] |= 0x80;
		}
#endif /* RT_CFG80211_P2P_SUPPORT */
#endif /* UAPSD_SUPPORT */

		pBssEdca = wlan_config_get_ht_edca(wdev);
		if (pBssEdca != NULL) {
			/* WMM: sync from driver's EDCA parameter */
			for (i = QID_AC_BK; i <= QID_AC_VO; i++) {
				wmm_ie[10 + (i * 4)] = (i << 5) +                                  /* b5-6 is ACI */
									   ((UCHAR)pBssEdca->bACM[i] << 4) +           /* b4 is ACM */
									   (pBssEdca->Aifsn[i] & 0x0f);                /* b0-3 is AIFSN */
				wmm_ie[11 + (i * 4)] = (pBssEdca->Cwmax[i] << 4) +                 /* b5-8 is CWMAX */
									   (pBssEdca->Cwmin[i] & 0x0f);                /* b0-3 is CWMIN */
				wmm_ie[12 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);          /* low byte of TXOP */
				wmm_ie[13 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);            /* high byte of TXOP */
			}
		}
		return TRUE;
	} else
		MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: can't find the wmm ie\n", __func__);

	return FALSE;
}
#endif /* CONFIG_AP_SUPPORT */


#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
VOID CFG80211_AuthRespHandler(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	AUTH_FRAME_INFO *auth_info;	/* auth info from hostapd */
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg;
	PFT_INFO pFtInfo;
#endif /* DOT11R_FT_SUPPORT */
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;

	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)pData;

	UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "AUTH - %s\n", __func__);

	os_alloc_mem(pAd, (UCHAR **)&auth_info, sizeof(AUTH_FRAME_INFO));
	NdisZeroMemory(auth_info, sizeof(AUTH_FRAME_INFO));



	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	ASSERT((wdev->func_idx == apidx));

	pEntry = MacTableLookup(pAd, mgmt->da);
	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
#ifdef DOT11W_PMF_SUPPORT
	if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
		&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
	goto SendAuth;
#endif /* DOT11W_PMF_SUPPORT */

		if (!RTMPEqualMemory(mgmt->sa, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN)) {
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			pEntry = NULL;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("AUTH - Bssid does not match\n"));
		} else {
#ifdef DOT11_N_SUPPORT
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:ENTRY ALREADY EXIST, TERADOWN BLOCKACK SESSION\n", __func__));
			ba_session_tear_down_all(pAd, pEntry->wcid, FALSE);
#endif /* DOT11_N_SUPPORT */
		}
	}

#ifdef DOT11W_PMF_SUPPORT
SendAuth:
#endif /* DOT11W_PMF_SUPPORT */

#ifdef HOSTAPD_11R_SUPPORT
	pFtCfg = &wdev->FtCfg;
	if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
		&& (mgmt->u.auth.auth_alg == AUTH_MODE_FT)) {
		/* USHORT result; */

		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, mgmt->da, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

		if (pEntry != NULL) {
			/* fill auth info from upper layer response */
			COPY_MAC_ADDR(auth_info->addr2, mgmt->da);
			COPY_MAC_ADDR(auth_info->addr1, wdev->if_addr);
			auth_info->auth_alg = mgmt->u.auth.auth_alg;
			auth_info->auth_seq = mgmt->u.auth.auth_transaction;
			auth_info->auth_status = mgmt->u.auth.status_code;

			/* os_alloc_mem(pAd, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO)); */
			pFtInfo = &(pEntry->auth_info_resp.FtInfo);
			PEID_STRUCT eid_ptr;
			UCHAR *Ptr;
			UCHAR WPA2_OUI[3] = {0x00, 0x0F, 0xAC};
			/* PFT_INFO pFtInfo = &auth_info->FtInfo; */

			NdisZeroMemory(pFtInfo, sizeof(FT_INFO));

			/* Ptr = &Fr->Octet[6]; */
			Ptr = mgmt->u.auth.variable;
			eid_ptr = (PEID_STRUCT) Ptr;

			/* get variable fields from payload and advance the pointer */
			while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)mgmt + Data)) {
				switch (eid_ptr->Eid) {
				case IE_FT_MDIE:
					if (FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo) == FALSE) {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_FT_MDIE\n", __func__);
						if (auth_info)
							os_free_mem(auth_info);
						return FALSE;
					}
					break;

				case IE_FT_FTIE:
					if (FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo) == FALSE) {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_FT_FTIE\n", __func__);
						if (auth_info)
							os_free_mem(auth_info);
						return FALSE;
					}
					break;

				case IE_FT_RIC_DATA:
					/* record the pointer of first RDIE. */
					if (pFtInfo->RicInfo.pRicInfo == NULL) {
						pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
						pFtInfo->RicInfo.Len = ((UCHAR *)mgmt + Data)
							- (UCHAR *)eid_ptr + 1;
					}

					if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
						NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
								&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					} else {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_FT_RIC_DATA\n", __func__);
						if (auth_info)
							os_free_mem(auth_info);
						return FALSE;
					}
					break;


				case IE_FT_RIC_DESCRIPTOR:
					if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
						NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
								&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					} else {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_FT_RIC_DESCRIPTOR\n", __func__);
						if (auth_info)
							os_free_mem(auth_info);
						return FALSE;
					}
					break;

				case IE_RSN:
					if (parse_rsn_ie(eid_ptr) &&
							(NdisEqualMemory(&eid_ptr->Octet[2],
									 WPA2_OUI, sizeof(WPA2_OUI)))) {
						NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
						pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
					} else {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_RSN\n", __func__);
						if (auth_info)
							os_free_mem(auth_info);
						return FALSE;
					}
					break;

					default:
						break;
				}
				eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
			}

			if (mgmt->u.auth.status_code == MLME_SUCCESS) {
				NdisMoveMemory(&pEntry->MdIeInfo, &auth_info->FtInfo.MdIeInfo, sizeof(FT_MDIE_INFO));

				pEntry->AuthState = AS_AUTH_OPEN;
				/*According to specific, if it already in SST_ASSOC, it can not go back */
				if (pEntry->Sst != SST_ASSOC)
					pEntry->Sst = SST_AUTH;
#ifdef RADIUS_MAC_AUTH_SUPPORT
				if (pEntry->wdev->radius_mac_auth_enable)
					pEntry->bAllowTraffic = TRUE;
#endif
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: AuthState:%d, Sst:%d\n", __func__, pEntry->AuthState, pEntry->Sst);
			}
#ifdef RADIUS_MAC_AUTH_SUPPORT
			else {
				if (pEntry->wdev->radius_mac_auth_enable) {
					pEntry->bAllowTraffic = FALSE;
					MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				}
			}
#endif
		}
		os_free_mem(auth_info);
		return;
	} else
#endif /* DOT11R_FT_SUPPORT */
#ifdef HOSTAPD_SAE_SUPPORT
	if (mgmt->u.auth.auth_alg == AUTH_MODE_SAE) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SAE Auth Response Sequence %d \n",
					mgmt->u.auth.auth_transaction);
		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, mgmt->da, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);


		if (pEntry) {
			if (mgmt->u.auth.status_code == MLME_SUCCESS) {
				pEntry->AuthState = AS_AUTH_OPEN;
				/*According to specific, if it already in SST_ASSOC, it can not go back */
				if (pEntry->Sst != SST_ASSOC)
					pEntry->Sst = SST_AUTH;
#ifdef RADIUS_MAC_AUTH_SUPPORT
				if (pEntry->wdev->radius_mac_auth_enable)
					pEntry->bAllowTraffic = TRUE;
#endif
			} else {
#ifdef RADIUS_MAC_AUTH_SUPPORT
				if (pEntry->wdev->radius_mac_auth_enable) {
					pEntry->bAllowTraffic = FALSE;
					MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				}
#endif
			}
		}
	} else
#endif
	if ((auth_info->auth_alg == AUTH_MODE_OPEN) &&
		(!IS_AKM_SHARED(pMbss->wdev.SecConfig.AKMMap))) {
		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, auth_info->addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

		if (pEntry) {
			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
#ifdef DOT11W_PMF_SUPPORT
		if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE)
			|| (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED))
#endif /* DOT11W_PMF_SUPPORT */
			{
				pEntry->AuthState = AS_AUTH_OPEN;
				/*According to specific, if it already in SST_ASSOC, it can not go back */
				if (pEntry->Sst != SST_ASSOC)
					pEntry->Sst = SST_AUTH;
			}
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: pEntry created: auth state:%d, Sst:%d", __func__, pEntry->AuthState, pEntry->Sst);
			/* APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_SUCCESS); */

		} else
			; /* MAC table full, what should we respond ????? */
	} else {
		/* wrong algorithm */
		/* APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_ALG_NOT_SUPPORT); */

		/* If this STA exists, delete it. */
		if (pEntry)
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AUTH - Alg=%d, Seq=%d\n",
				auth_info->auth_alg, auth_info->auth_seq);
	}
	os_free_mem(auth_info);
}

VOID CFG80211_AssocRespHandler(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	struct wifi_dev *wdev = NULL;
	struct dev_rate_info *rate;
	BSS_STRUCT *pMbss;
	/* BOOLEAN bAssocSkip = FALSE; */
	/* CHAR rssi; */
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocRspHdr;
	/* HEADER_802_11 AssocReqHdr; */
	USHORT CapabilityInfoForAssocResp;
	USHORT StatusCode = MLME_SUCCESS;
	USHORT Aid;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	UCHAR SupRateLen, PhyMode, FlgIs11bSta;
	UCHAR i;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	UCHAR isReassoc;
	UCHAR SubType;
	/* BOOLEAN bACLReject = FALSE; */
#ifdef DOT1X_SUPPORT
	PUINT8 pPmkid = NULL;
	UINT8 pmkid_count = 0;
#endif /* DOT1X_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg = NULL;
	PFT_INFO pFtInfoBuf = NULL; 	/*Wframe-larger-than=1024 warning  removal*/
	PEID_STRUCT pFtIe = NULL;
#endif /* DOT11R_FT_SUPPORT */
#ifdef HOSTAPD_OWE_SUPPORT
	PEID_STRUCT pEcdhIe = NULL;
	PEID_STRUCT pRsnIe = NULL;
#endif
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
	ADD_HT_INFO_IE *addht;
	struct _build_ie_info ie_info;
#ifdef WAPP_SUPPORT
/*	UINT8 wapp_cnnct_stage = WAPP_ASSOC; */
	UINT16 wapp_assoc_fail = NOT_FAILURE;
#endif /* WAPP_SUPPORT */

	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)pData;

	UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ASSOC - %s\n", __func__);

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Wrong Addr1 - "MACSTR"\n",
					MAC2STR(mgmt->sa));
		goto LabelOK;
	}
	ASSERT((wdev->func_idx == apidx));

#ifdef WSC_AP_SUPPORT
	wsc_ctrl = &pMbss->wdev.WscControl;
#endif /* WSC_AP_SUPPORT */

	PhyMode = wdev->PhyMode;
	rate = &wdev->rate;
	addht = wlan_operate_get_addht(wdev);
	FlgIs11bSta = 1;

	pEntry = MacTableLookup(pAd, mgmt->da);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"NoAuth MAC - "MACSTR"\n",
					MAC2STR(mgmt->da));
		goto LabelOK;
	}

	ie_list = pEntry->ie_list;

#ifdef DOT11R_FT_SUPPORT
	os_alloc_mem(NULL, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO));
	if (pFtInfoBuf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pFtInfoBuf mem alloc failed\n");
		goto LabelOK;
	}
	NdisZeroMemory(pFtInfoBuf, sizeof(FT_INFO));
#endif /* DOT11R_FT_SUPPORT */

	for (i = 0; i < ie_list->rate.sup_rate_len; i++) {
		if (((ie_list->rate.sup_rate[i] & 0x7F) != 2) &&
			((ie_list->rate.sup_rate[i] & 0x7F) != 4) &&
			((ie_list->rate.sup_rate[i] & 0x7F) != 11) &&
			((ie_list->rate.sup_rate[i] & 0x7F) != 22)) {
			FlgIs11bSta = 0;
			break;
		}
	}

	if (!VALID_MBSS(pAd, pEntry->func_tb_idx)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"pEntry bounding invalid wdev(apidx=%d)\n",
					pEntry->func_tb_idx);
		goto LabelOK;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"pEntry->func_tb_idx=%d\n",
				pEntry->func_tb_idx);

	tr_entry = &pAd->tr_ctl.tr_entry[pEntry->tr_tb_idx];

	isReassoc = ieee80211_is_reassoc_resp(mgmt->frame_control);

	/* 2. qualify this STA's auth_asoc status in the MAC table, decide StatusCode */
	StatusCode = APBuildAssociation(pAd, pEntry, pEntry->ie_list, pEntry->MaxSupportedRate, &Aid, isReassoc);
	if (mgmt->u.reassoc_resp.status_code != MLME_SUCCESS)
		StatusCode = mgmt->u.reassoc_resp.status_code;

#ifdef WAPP_SUPPORT
	if (StatusCode != MLME_SUCCESS)
		wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */

	/*is status is success ,update STARec*/
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		if (wdev_do_conn_act(pEntry->wdev, pEntry) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "connect action fail!!\n");
		}
	}
	if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
		BOOLEAN  bExtractIe = FALSE;
#ifdef DOT11R_FT_SUPPORT
		pFtCfg = &(wdev->FtCfg);
		if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
			&& (StatusCode == MLME_SUCCESS))
			bExtractIe = TRUE;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
		if (wdev && IS_AKM_OWE(wdev->SecConfig.AKMMap))
			bExtractIe = TRUE;
#endif
		if (bExtractIe) {
			PEID_STRUCT eid_ptr;
			UCHAR *Ptr;
			UCHAR WPA2_OUI[3] = {0x00, 0x0F, 0xAC};
			/* PFT_INFO pFtInfo = &auth_info->FtInfo; */

			NdisZeroMemory(pFtInfoBuf, sizeof(FT_INFO));

			/* Ptr = &Fr->Octet[6]; */
			Ptr = mgmt->u.reassoc_resp.variable;
			eid_ptr = (PEID_STRUCT) Ptr;

			/* get variable fields from payload and advance the pointer */
			while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)mgmt + Data)) {
				switch (eid_ptr->Eid) {
#ifdef DOT11R_FT_SUPPORT
				case IE_FT_MDIE:
					if (FT_FillMdIeInfo(eid_ptr, &pFtInfoBuf->MdIeInfo) == FALSE) {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_FT_MDIE\n", __func__);
						if (pFtInfoBuf)
							os_free_mem(pFtInfoBuf);
						return FALSE;
					}
					break;

				case IE_FT_FTIE:
					pFtIe = eid_ptr;
					if (FT_FillFtIeInfo(eid_ptr, &pFtInfoBuf->FtIeInfo) == FALSE) {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_FT_FTIE\n", __func__);
						if (pFtInfoBuf)
							os_free_mem(pFtInfoBuf);
						return FALSE;
					}
					break;

				case IE_FT_RIC_DATA:
					/* record the pointer of first RDIE. */
					if (pFtInfoBuf->RicInfo.pRicInfo == NULL) {
						pFtInfoBuf->RicInfo.pRicInfo = &eid_ptr->Eid;
						pFtInfoBuf->RicInfo.Len = ((UCHAR *)mgmt + Data)
							- (UCHAR *)eid_ptr + 1;
					}

					if ((pFtInfoBuf->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
						NdisMoveMemory(&pFtInfoBuf->RicInfo.RicIEs[pFtInfoBuf->RicInfo.RicIEsLen],
								&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfoBuf->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					} else {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_FT_RIC_DATA\n", __func__);
						if (pFtInfoBuf)
							os_free_mem(pFtInfoBuf);
						return FALSE;
					}
					break;

				case IE_FT_RIC_DESCRIPTOR:
					if ((pFtInfoBuf->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
						NdisMoveMemory(&pFtInfoBuf->RicInfo.RicIEs[pFtInfoBuf->RicInfo.RicIEsLen],
								&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfoBuf->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					} else {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_FT_RIC_DESCRIPTOR\n", __func__);
						if (pFtInfoBuf)
							os_free_mem(pFtInfoBuf);
						return FALSE;
					}
					break;
#endif
#if defined(DOT11R_FT_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)

				case IE_RSN:
					if (parse_rsn_ie(eid_ptr) &&
							(NdisEqualMemory(&eid_ptr->Octet[2],
									 WPA2_OUI, sizeof(WPA2_OUI)))) {
#ifdef DOT11R_FT_SUPPORT
						NdisMoveMemory(pFtInfoBuf->RSN_IE, eid_ptr, eid_ptr->Len + 2);
						pFtInfoBuf->RSNIE_Len = eid_ptr->Len + 2;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
						pRsnIe = eid_ptr;
#endif
					} else {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"%s() - wrong IE_RSN\n", __func__);
						if (pFtInfoBuf)
							os_free_mem(pFtInfoBuf);
						return FALSE;
					}
					break;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
				case IE_WLAN_EXTENSION:
					{
						/*parse EXTENSION EID*/
						UCHAR *extension_id = (UCHAR *)eid_ptr + 2;

						switch (*extension_id) {
						case IE_EXTENSION_ID_ECDH:
							pEcdhIe = eid_ptr;
						}
					}
					break;
#endif

				default:
					break;
				}
				eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
			}
		}

			if (mgmt->u.reassoc_resp.status_code == MLME_SUCCESS) {
/*				NdisMoveMemory(&pEntry->MdIeInfo, &auth_info.FtInfo.MdIeInfo, sizeof(FT_MDIE_INFO));
 *
 *				pEntry->AuthState = AS_AUTH_OPEN;
 *				pEntry->Sst = SST_AUTH;
*/
			}
		}
		/* just silencely discard this frame */
		/*if (StatusCode == 0xFFFF)
		*	goto LabelOK;
		*/

#ifdef DOT11K_RRM_SUPPORT
	if ((pEntry->func_tb_idx < pAd->ApCfg.BssidNum)
		&& IS_RRM_ENABLE(wdev))
		pEntry->RrmEnCap.word = ie_list->RrmEnCap.word;
#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11_VHT_AC
	if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
		/* +++Add by shiang for debug */
		if (WMODE_CAP_AC(wdev->PhyMode)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "%s():Peer is VHT capable device!\n", __func__);
			NdisMoveMemory(&pEntry->ext_cap, &ie_list->ExtCapInfo, sizeof(ie_list->ExtCapInfo));
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "\tOperatingModeNotification=%d\n",
					  pEntry->ext_cap.operating_mode_notification);
			/* dump_vht_cap(pAd, &ie_list->vht_cap); */
		}

		/* ---Add by shiang for debug */
	}
#endif /* DOT11_VHT_AC */

	if (StatusCode == MLME_ASSOC_REJ_DATA_RATE)
		RTMPSendWirelessEvent(pAd, IW_STA_MODE_EVENT_FLAG, pEntry->Addr, wdev->wdev_idx, 0);

#ifdef WH_EVENT_NOTIFIER
	if (pEntry && tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		EventHdlr pEventHdlrHook = NULL;

		pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_JOIN);

		if (pEventHdlrHook && pEntry->wdev)
			pEventHdlrHook(pAd, pEntry, Elem);
	}
#endif /* WH_EVENT_NOTIFIER */

#ifdef DOT11W_PMF_SUPPORT
	/* SendAssocResponse: */
#endif /* DOT11W_PMF_SUPPORT */
	/* 3. send Association Response */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		goto LabelOK;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Reassoc - Send reassoc response (Status=%d)...\n",
			  StatusCode);
	Aid |= 0xc000; /* 2 most significant bits should be ON */
	SubType = isReassoc ? SUBTYPE_REASSOC_RSP : SUBTYPE_ASSOC_RSP;
	CapabilityInfoForAssocResp = pMbss->CapabilityInfo; /*use AP's cability */
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if ((wsc_ctrl->WscConfMode != WSC_DISABLE) &&
			(ie_list->CapabilityInfo & 0x0010))
			CapabilityInfoForAssocResp |= 0x0010;
	}

#endif /* WSC_AP_SUPPORT */
		/* fail in ACL checking => send an Assoc-Fail resp. */
	SupRateLen = rate->legacy_rate.sup_rate_len;

	/* TODO: need to check rate in support rate element, not number */
	if (FlgIs11bSta == 1)
		SupRateLen = 4;
	MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2,
					 wdev->if_addr, wdev->bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &AssocRspHdr,
					  2,						&CapabilityInfoForAssocResp,
					  2,						&StatusCode,
					  2,						&Aid,
					  1,						&SupRateIe,
					  1,						&SupRateLen,
					  SupRateLen,				rate->legacy_rate.sup_rate,
					  END_OF_ARGS);

	if ((rate->legacy_rate.ext_rate_len) && (PhyMode != WMODE_B) && (FlgIs11bSta == 0)) {
		ULONG TmpLen;

		MakeOutgoingFrame(pOutBuffer + FrameLen,
						&TmpLen,				1,
						&ExtRateIe,				1,
						&rate->legacy_rate.ext_rate_len,
						rate->legacy_rate.ext_rate_len,
						rate->legacy_rate.ext_rate,
						END_OF_ARGS);
		FrameLen += TmpLen;
	}

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd)) {
		pEntry->DevPeerRole = ie_list->MAP_AttriValue;
		MAP_InsertMapCapIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen);
	}
#endif /* CONFIG_MAP_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		RRM_InsertRRMEnCapIE(pAd, wdev, pOutBuffer + FrameLen, &FrameLen, pEntry->func_tb_idx);

#endif /* DOT11K_RRM_SUPPORT */

	ie_info.frame_subtype = SUBTYPE_ASSOC_RSP;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev_search_by_address(pAd, ie_list->Addr1);

	if (ie_info.wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"wdev is NULL\n");
		goto LabelOK;
	}

	/* add WMM IE here */
	/* printk("%s()=>bWmmCapable=%d,CLINE=%d\n",__FUNCTION__,wdev->bWmmCapable,CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)); */
	ie_info.is_draft_n_type = FALSE;
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);
	}
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev;
#ifdef DOT11W_PMF_SUPPORT

	if (StatusCode == MLME_ASSOC_REJ_TEMPORARILY) {
		ULONG TmpLen;
		UCHAR IEType = IE_TIMEOUT_INTERVAL; /* IE:0x15 */
		UCHAR IELen = 5;
		UCHAR TIType = 3;
		UINT32 units = 1 << 10; /* 1 seconds, should be 0x3E8 */

		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  1, &IEType,
						  1, &IELen,
						  1, &TIType,
						  4, &units,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

#endif /* DOT11W_PMF_SUPPORT */
#ifdef DOT11_N_SUPPORT

		/* HT capability in AssocRsp frame. */
	if (HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists) && WMODE_CAP_N(wdev->PhyMode) &&
		(wdev->DesiredHtPhyInfo.bHtEnable)) {
#ifdef DOT11_VHT_AC
		struct _build_ie_info vht_ie_info;
#endif /* DOT11_VHT_AC */

		ie_info.is_draft_n_type = FALSE;
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_ht_ies(pAd, &ie_info);

		if ((ie_list->cmm_ies.vendor_ie.ra_cap) == 0 || (pAd->bBroadComHT == TRUE)) {
			ie_info.is_draft_n_type = TRUE;
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);

		}
#ifdef DOT11_VHT_AC
		vht_ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		vht_ie_info.frame_subtype = SUBTYPE_ASSOC_RSP;
		vht_ie_info.channel = wdev->channel;
		vht_ie_info.phy_mode = wdev->PhyMode;
		vht_ie_info.wdev = wdev;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		ucETxBfCap = wlan_config_get_etxbf(wdev);

		if (HcIsBfCapSupport(wdev) == FALSE)
			wlan_config_set_etxbf(wdev, SUBF_OFF);

#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
		FrameLen += build_vht_ies(pAd, &vht_ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
	}

#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
	/* qosmap IE */
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "entry wcid %d QosMapSupport=%d\n",
			 pEntry->wcid, pEntry->QosMapSupport);

	if (pEntry->QosMapSupport) {
		ULONG	TmpLen;
		UCHAR	QosMapIE, ielen = 0, explen = 0;
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

		if (pHSCtrl->QosMapEnable) {
			QosMapIE = IE_QOS_MAP_SET;

			/* Fixed field Dscp range:16, len:1 IE_ID:1*/
			if (pHSCtrl->QosMapSetIELen > 18)
				explen = pHSCtrl->QosMapSetIELen - 18;

			pEntry->DscpExceptionCount = explen;
			memcpy((UCHAR *)pEntry->DscpRange, (UCHAR *)pHSCtrl->DscpRange, 16);
			memcpy((UCHAR *)pEntry->DscpException, (UCHAR *)pHSCtrl->DscpException, 42);
			ielen = explen + 16;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1,			&QosMapIE,
							  1,			&ielen,
							  explen,		pEntry->DscpException,
							  16,			pEntry->DscpRange,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}

#endif /* CONFIG_HOTSPOT_R2 */

		/* 7.3.2.27 Extended Capabilities IE */
	{
		ULONG TmpLen, infoPos;
		PUCHAR pInfo;
		UCHAR extInfoLen;
		BOOLEAN bNeedAppendExtIE = FALSE;
		EXT_CAP_INFO_ELEMENT extCapInfo;
#ifdef RT_BIG_ENDIAN
		UCHAR *pextCapInfo;
#endif

		extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
		NdisZeroMemory(&extCapInfo, extInfoLen);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

		/* P802.11n_D1.10, HT Information Exchange Support */
		if (WMODE_CAP_N(wdev->PhyMode)
			&& (wdev->channel <= 14)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		   )
			extCapInfo.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_DOT11V_WNM
		/* #ifdef CONFIG_HOTSPOT_R2 Remove for WNM independance */

		if (ie_list->ExtCapInfo.BssTransitionManmt == 1) {
			pEntry->bBSSMantSTASupport = TRUE;
			if (pMbss->WNMCtrl.WNMBTMEnable)
				extCapInfo.BssTransitionManmt = 1;
		}
		/* #endif CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_DOT11V_WNM */

#ifdef CONFIG_DOT11U_INTERWORKING
		if (pMbss->GASCtrl.b11U_enable)
		extCapInfo.interworking = 1;
#endif /* CONFIG_DOT11U_INTERWORKING */

#ifdef DOT11V_WNM_SUPPORT

		if (IS_BSS_TRANSIT_MANMT_SUPPORT(pAd, pEntry->func_tb_idx)) {
			if (ie_list->ExtCapInfo.BssTransitionManmt == 1) {
				extCapInfo.BssTransitionManmt = 1;
				pEntry->bBSSMantSTASupport = TRUE;
			}
		}

		if (IS_WNMDMS_SUPPORT(pAd, pEntry->func_tb_idx)) {
			if (ie_list->ExtCapInfo.DMSSupport == 1) {
				extCapInfo.DMSSupport = 1;
				pEntry->bDMSSTASupport = TRUE;
			}
		}

#endif /* DOT11V_WNM_SUPPORT */
#ifdef DOT11_VHT_AC

		if (WMODE_CAP_AC(wdev->PhyMode) &&
			(wdev->channel > 14))
			extCapInfo.operating_mode_notification = 1;

#endif /* DOT11_VHT_AC */
#ifdef RT_BIG_ENDIAN
		pextCapInfo = (UCHAR *)&extCapInfo;
		*((UINT32 *)pextCapInfo) = cpu2le32(*((UINT32 *)pextCapInfo));
		pextCapInfo = (UCHAR *)&extCapInfo;
		*((UINT32 *)(pextCapInfo + 4)) = cpu2le32(*((UINT32 *)(pextCapInfo + 4)));
#endif

		pInfo = (UCHAR *)(&extCapInfo);

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

			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1,			&ExtCapIe,
							  1,			&extInfoLen,
							  extInfoLen,	&extCapInfo,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
	/* add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back */
	FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_ASSOC_RESP
							);

#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev))
		MakeMboOceIE(pAd, wdev, pEntry, pOutBuffer+FrameLen, &FrameLen, MBO_FRAME_TYPE_ASSOC_RSP);
#endif /* MBO_SUPPORT */
#ifdef WSC_AP_SUPPORT

	if (pEntry->bWscCapable) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;
		ULONG WscTmpLen = 0;

		os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);

		if (pWscBuf) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildAssocRespIE(pAd, pEntry->func_tb_idx, 0, pWscBuf, &WscIeLen);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &WscTmpLen,
							  WscIeLen, pWscBuf,
							  END_OF_ARGS);
			FrameLen += WscTmpLen;
			os_free_mem(pWscBuf);
		}
	}

#endif /* WSC_AP_SUPPORT */
#ifdef P2P_SUPPORT

	if (ie_list->P2PSubelementLen > 0) {
		ULONG	TmpLen;
		UCHAR	P2pIdx = P2P_NOT_FOUND;
		UCHAR	GroupCap = 0xff, DeviceCap = 0xff, DevAddr[6] = {0}, DeviceType[8], DeviceName[32], DeviceNameLen = 0;
		PUCHAR	pData;
		USHORT	Dpid, ConfigMethod;

		pEntry->bP2pClient = TRUE;
		pEntry->P2pInfo.P2pClientState = P2PSTATE_CLIENT_ASSOC;
		P2pParseSubElmt(pAd, (PVOID)ie_list->P2pSubelement, ie_list->P2PSubelementLen, FALSE, &Dpid, &GroupCap,
						&DeviceCap, DeviceName, &DeviceNameLen, DevAddr, NULL, NULL, NULL, NULL, &ConfigMethod,
						&ConfigMethod, DeviceType, NULL, NULL, NULL, NULL, &StatusCode, NULL,
#ifdef WFD_SUPPORT
						NULL, NULL,
#endif /* WFD_SUPPORT */
						NULL);
		P2pIdx = P2pGroupTabSearch(pAd, DevAddr);

		if (P2pIdx == P2P_NOT_FOUND)
			P2pIdx = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_DISCOVERY_CLIENT, NULL, 0, 0, 0);

		if (P2pIdx != P2P_NOT_FOUND) {
			pEntry->P2pInfo.p2pIndex = P2pIdx;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "ASSOC RSP - Insert P2P IE to "MACSTR"\n",
					  MAC2STR(pEntry->Addr));
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 " %d. DevAddr = "MACSTR"\n",
					  P2pIdx, MAC2STR(DevAddr));
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "DeviceNameLen = %d, DeviceName = %c %c %c %c %c %c %c %c\n",
					  DeviceNameLen,
					  DeviceName[0], DeviceName[1], DeviceName[2], DeviceName[3],
					  DeviceName[4], DeviceName[5], DeviceName[6], DeviceName[7]);
			/* update P2P Interface Address */
			RTMPMoveMemory(pAd->P2pTable.Client[P2pIdx].InterfaceAddr, pEntry->Addr, MAC_ADDR_LEN);
			pData = pOutBuffer + FrameLen;
			P2pMakeP2pIE(pAd, SUBTYPE_ASSOC_RSP, pData, &TmpLen);
			FrameLen += TmpLen;
		}
	} else
		pEntry->bP2pClient = FALSE;

#ifdef WFD_SUPPORT
{
	PUCHAR	pData;
	ULONG	WfdIeLen = 0;

	pData = pOutBuffer + FrameLen;
	WfdMakeWfdIE(pAd, SUBTYPE_ASSOC_RSP, pData, &WfdIeLen);
	FrameLen += WfdIeLen;
}
#endif /* WFD_SUPPORT */
#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_SUPPORT

		/* Append extra IEs provided by wpa_supplicant */
	if (pAd->ApCfg.MBSSID[pEntry->apidx].AssocRespExtraIeLen) {
		ULONG TmpLen = 0;
		INT32 IesLen = pAd->ApCfg.MBSSID[pEntry->apidx].AssocRespExtraIeLen;
		UCHAR *Ies = pAd->ApCfg.MBSSID[pEntry->apidx].AssocRespExtraIe;

		if (RTMPIsValidIEs(Ies, IesLen)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "AssocRespExtraIE Added (Len=%d)\n", IesLen);
			MakeOutgoingFrame(pOutBuffer + FrameLen,
							  &TmpLen,
							  IesLen,
							  Ies,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		} else
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "AssocRespExtraIE len incorrect!\n");
	}

#endif /* RT_CFG80211_SUPPORT */
#ifdef HOSTAPD_OWE_SUPPORT
		if (wdev && IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			if (pRsnIe) {
				ULONG TmpLen = 0;

				MakeOutgoingFrame(pOutBuffer+FrameLen,
								  &TmpLen,
								  pRsnIe->Len + 2,
								  pRsnIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
			if (pEcdhIe) {
				ULONG TmpLen = 0;

				MakeOutgoingFrame(pOutBuffer+FrameLen,
								  &TmpLen,
								  pEcdhIe->Len + 2,
								  pEcdhIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}
#endif
#ifdef DOT11_HE_AX
		if (HAS_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists)
			&& IS_HE_STA(pEntry->cap.modes) && WMODE_CAP_AX(wdev->PhyMode)
				&& wdev->DesiredHtPhyInfo.bHtEnable) {
			ULONG TmpLen = 0;
			TmpLen = add_assoc_rsp_he_ies(wdev, (UINT8 *)pOutBuffer, FrameLen);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"add he assoc_rsp, len=%d\n", TmpLen);
			FrameLen += TmpLen;
		}
#endif /*DOT11_HE_AX*/
#ifdef DOT11R_FT_SUPPORT
	if ((pFtCfg != NULL) && (pFtCfg->FtCapFlag.Dot11rFtEnable)) {
		PUINT8	mdie_ptr;
		UINT8	mdie_len;
		/*PUINT8	ftie_ptr = NULL;*/
		/*UINT8	ftie_len = 0;*/
		/*PUINT8  ricie_ptr = NULL;*/
		/*UINT8   ricie_len = 0;*/
		/* struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig; */

		/* Insert RSNIE if necessary */
		if (pFtInfoBuf->RSNIE_Len != 0) {
			ULONG TmpLen;

			MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
				pFtInfoBuf->RSNIE_Len, pFtInfoBuf->RSN_IE,
				END_OF_ARGS);
			FrameLen += TmpLen;
		}

		/* Insert MDIE. */
		mdie_ptr = pOutBuffer+FrameLen;
		mdie_len = 5;
		FT_InsertMdIE(pOutBuffer+FrameLen,
				&FrameLen,
			pFtInfoBuf->MdIeInfo.MdId,
			pFtInfoBuf->MdIeInfo.FtCapPlc);


		/* Insert FTIE. */
		if (pFtIe) {
			ULONG TmpLen = 0;

			MakeOutgoingFrame(pOutBuffer+FrameLen,
							  &TmpLen,
							  pFtIe->Len + 2,
							  pFtIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

	}
#endif /* DOT11R_FT_SUPPORT */

	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory((PVOID) pOutBuffer);
	pOutBuffer = NULL;

		/* set up BA session */
	if (StatusCode == MLME_SUCCESS) {
	pEntry->PsMode = PWR_ACTIVE;
	/* TODO: shiang-usw, we need to rmove upper setting and migrate to tr_entry->PsMode */
	pAd->MacTab.tr_entry[pEntry->wcid].PsMode = PWR_ACTIVE;
	MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
#ifdef IAPP_SUPPORT
	/*PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg; */
	/*		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie; */
	{
#ifndef RT_CFG80211_SUPPORT
	/* send association ok message to IAPPD */
		IAPP_L2_Update_Frame_Send(pAd, pEntry->Addr, pEntry->wdev->wdev_idx);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "####### Send L2 Frame Mac="MACSTR"\n",
				  MAC2STR(pEntry->Addr));
#endif
	}

	/*		SendSingalToDaemon(SIGUSR2, pObj->IappPid); */
#endif /* IAPP_SUPPORT */
	/* ap_assoc_info_debugshow(pAd, isReassoc, pEntry, ie_list); */
	/* send wireless event - for association */
#ifdef VENDOR_FEATURE7_SUPPORT
	/* Passed in the pEntry->apindx argument */
	RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pEntry->Addr, pEntry->func_tb_idx, 0);
#else
	RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pEntry->Addr, 0, 0);
#endif
	/* This is a reassociation procedure */
	pEntry->IsReassocSta = isReassoc;
	/* clear txBA bitmap */
	pEntry->TXBAbitmap = 0;

	if (pEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX) {
	CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

	if ((wdev->channel <= 14) &&
		addht->AddHtInfo.ExtChanOffset &&
		(ie_list->cmm_ies.ht_cap.HtCapInfo.ChannelWidth == BW_40))
		SendBeaconRequest(pAd, pEntry->wcid);

		ba_ori_session_setup(pAd, pEntry->wcid, 5, 0);
	}

#ifdef RT_CFG80211_SUPPORT
	if (TRUE) { /*CFG_TODO*/
		/* need to update pEntry to  inform later flow to keep ConnectionState in connected */
		pEntry->bWscCapable = ie_list->bWscCapable;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		PNET_DEV pNetDev = NULL;

		pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_GO);

		if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
			(pNetDev != NULL)) {
			MTWF_DBG(pAd, DBG_CAT_AP,
				 DBG_SUBCAT_ALL,
				 DBG_LVL_INFO,
				 "CONCURRENT CFG: NOITFY ASSOCIATED, pEntry->bWscCapable:%d\n",
				   pEntry->bWscCapable);
			CFG80211OS_NewSta(pNetDev, ie_list->Addr2, (PUCHAR)Elem->Msg, Elem->MsgLen);
		} else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
		{
			MTWF_DBG(pAd, DBG_CAT_AP,
				 DBG_SUBCAT_ALL,
				 DBG_LVL_INFO,
				 "SINGLE CFG: NOITFY ASSOCIATED, pEntry->bWscCapable:%d\n",
				  pEntry->bWscCapable);
#ifdef RT_CFG80211_SUPPORT
		/*	CFG80211OS_NewSta(pEntry->wdev->if_dev, ie_list->Addr2,
				(PUCHAR)Elem->Msg, Elem->MsgLen, isReassoc); */
#endif

			if (IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher)) {
				struct _ASIC_SEC_INFO *info = NULL;
				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
				/* Set key material to Asic */
				if (info) {
					os_zero_mem(info, sizeof(ASIC_SEC_INFO));
					info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
					info->Direction = SEC_ASIC_KEY_BOTH;
					info->Wcid = pEntry->wcid;
					info->BssIndex = pEntry->func_tb_idx;
					info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
					info->Cipher = pEntry->SecConfig.PairwiseCipher;
					info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
					os_move_mem(&info->Key,
						&pEntry->SecConfig.WepKey[pEntry->SecConfig.PairwiseKeyId],
						sizeof(SEC_KEY_INFO));
					os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
					HW_ADDREMOVE_KEYTABLE(pAd, info);
					os_free_mem(info);
				}
			}
		}

		/* hex_dump("ASSOC_REQ", Elem->Msg, Elem->MsgLen); */
	} else
#endif
	/* enqueue a EAPOL_START message to trigger EAP state machine doing the authentication */
	if (IS_AKM_PSK_Entry(pEntry)) {
		pPmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
						   ie_list->RSNIE_Len,
						   PMKID_LIST,
						   &pmkid_count);

		if (pPmkid != NULL) {
			INT CacheIdx;

			CacheIdx = RTMPValidatePMKIDCache(&pAd->ApCfg.PMKIDCache,
							  pEntry->func_tb_idx,
							  pEntry->Addr,
							  pPmkid);

			store_pmkid_cache_in_sec_config(pAd, pEntry, CacheIdx);

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "ASSOC - CacheIdx = %d\n",
				  CacheIdx);

			if (IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap) &&
			   !is_pmkid_cache_in_sec_config(&pEntry->SecConfig)) {
				MTWF_DBG(pAd, DBG_CAT_SEC,
					 CATSEC_SAE,
					 DBG_LVL_ERROR,
					 "ASSOC - SAE - verify pmkid fail\n");
				MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				goto LabelOK;
			}
		}
#ifdef WSC_AP_SUPPORT
		/*
		 * In WPA-PSK mode,
		 * If Association Request of station has RSN/SSN,
		 * WPS AP Must Not send EAP-Request/Identity to station
		 * no matter WPS AP does receive EAPoL-Start from STA or not.
		 * Marvell WPS test bed(v2.1.1.5) will send AssocReq with WPS IE and RSN/SSN IE.
		 */
		if (pEntry->bWscCapable || (ie_list->RSNIE_Len == 0)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "ASSOC - IF(ra%d) This is a WPS Client.\n\n",
					  pEntry->func_tb_idx);
			goto LabelOK;
		} else {
			pEntry->bWscCapable = FALSE;
			pEntry->Receive_EapolStart_EapRspId = (WSC_ENTRY_GET_EAPOL_START |
								   WSC_ENTRY_GET_EAP_RSP_ID);
			/* This STA is not a WPS STA */
			NdisZeroMemory(wsc_ctrl->EntryAddr, 6);
		}

#endif /* WSC_AP_SUPPORT */
		/* Enqueue a EAPOL-start message with the pEntry for WPAPSK State Machine */
		if (1
#ifdef WSC_AP_SUPPORT
			&& !pEntry->bWscCapable
#endif /* WSC_AP_SUPPORT */
		   ) {
			/* Enqueue a EAPOL-start message with the pEntry */
			os_move_mem(&pEntry->SecConfig.Handshake.AAddr, wdev->bssid, MAC_ADDR_LEN);
			os_move_mem(&pEntry->SecConfig.Handshake.SAddr, pEntry->Addr, MAC_ADDR_LEN);

			if (!IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap) &&
				!(IS_AKM_OWE(pEntry->SecConfig.AKMMap)))
				os_move_mem(&pEntry->SecConfig.PMK, &wdev->SecConfig.PMK, LEN_PMK);

			RTMPSetTimer(&pEntry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
		}
	}

#ifdef DOT1X_SUPPORT
	else if (IS_AKM_WPA2_Entry(pEntry) ||
		 IS_AKM_WPA3_192BIT_Entry(pEntry)) {
		pPmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
						   ie_list->RSNIE_Len,
						   PMKID_LIST,
						   &pmkid_count);

		if (pPmkid != NULL) {
			/* Key cache */
			INT CacheIdx;

			CacheIdx = RTMPValidatePMKIDCache(&pAd->ApCfg.PMKIDCache,
							  pEntry->func_tb_idx,
							  pEntry->Addr,
							  pPmkid);

			process_pmkid(pAd, wdev, pEntry, CacheIdx);
		}
	} else if (IS_AKM_1X_Entry(pEntry) ||
		   (IS_IEEE8021X(&pEntry->SecConfig)
#ifdef WSC_AP_SUPPORT
		   && (!pEntry->bWscCapable)
#endif /* WSC_AP_SUPPORT */
		   )) {
		/* Enqueue a EAPOL-start message to trigger EAP SM */
		if (pEntry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
		) {
			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_1X;
			RTMPSetTimer(&pEntry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
		}
	}

#endif /* DOT1X_SUPPORT */

#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT)
	if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef MWDS
	MWDSAPPeerEnable(pAd, pEntry);
#endif
#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
	map_a4_peer_enable(pAd, pEntry, TRUE);
#endif /* CONFIG_MAP_SUPPORT */
#ifdef WAPP_SUPPORT
	wapp_send_cli_join_event(pAd, pEntry);
#endif
	}
#endif
#ifdef SMART_ANTENNA
	{
	unsigned long irqflags;
	/* Check if need to reset the sa training procedures to init stage! */
	RTMP_IRQ_LOCK(&pAd->smartAntLock, irqflags);

	if (RTMP_SA_WORK_ON(pAd)) {
		/* sa_add_train_entry(pAd, &pEntry->Addr[0], FALSE); */
		pAd->pSAParam->bStaChange = TRUE;
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "sta("MACSTR") add!\n",
				  MAC2STR(pEntry->Addr));
	}

	RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqflags);
	}
#endif /* SMART_ANTENNA // */
#ifdef GREENAP_SUPPORT

	if (greenap_get_capability(pAd) && greenap_get_allow_status(pAd)) {
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC))
		greenap_check_peer_connection_at_link_up_down(pAd, wdev);
	}

#endif /* GREENAP_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2

	/* add to cr4 pool */
	if (pEntry->QosMapSupport) {
	PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

	if (pHSCtrl->QosMapEnable) {
		if (!pHSCtrl->QosMapAddToPool) {
			pHSCtrl->QosMapAddToPool = TRUE;
			pHSCtrl->QosMapPoolID = hotspot_qosmap_add_pool(pAd, pEntry);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "add current MBSS qosmap to CR4\n");
		}

		hotspot_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, pHSCtrl->QosMapPoolID);
	}
	}

#endif /* CONFIG_HOTSPOT_R2 */
#ifdef DSCP_QOS_MAP_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
	if (pMbss->DscpQosMapEnable) {
		pEntry->PoolId = pMbss->DscpQosPoolId;
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("[DSCP-QOS-MAP] update sta mapping to CR4 for Pool %d wcid %d",
						pEntry->PoolId, pEntry->wcid));
		dscp_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, pEntry->PoolId);
	}
	}
#endif
	}

#ifdef FAST_EAPOL_WAR
	/*
	*	Moved from WifiSysApPeerLinkUp() in open security mode.
	*	to make sure the STATE_PORT_SECURE flag can be polled by MU N9 module.
	*	then MU Action Frame sent out after Asso Resp.
	*/
	if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
		if (!(IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
			|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
			|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
			|| pEntry->bWscCapable)) {
			WifiSysUpdatePortSecur(pAd, pEntry, NULL);
		}
	}
#endif /* FAST_EAPOL_WAR */
#ifdef BAND_STEERING
	if ((pAd->ApCfg.BandSteering)
	) {
		BndStrg_UpdateEntry(pAd, pEntry, ie_list, TRUE);
	}
#endif

#ifdef VENDOR_FEATURE7_SUPPORT
	if (arris_event_send_hook && pEntry && (StatusCode == MLME_SUCCESS)) {
		UCHAR assoc_event_msg[32] = {0};
		UINT32 count = 0;
		UCHAR *assoc_sta_info = NULL;
		HT_CAP_INFO  *pHTCap = &(ie_list->HTCapability).HtCapInfo;
		HT_CAP_PARM  *pHTCapParm = &(ie_list->HTCapability).HtCapParm;
		/* Send a WLAN_EVENT to ATOM which in turns sends an RPC
		*	to update our client table on the ARM.
		*/
		NdisZeroMemory(assoc_event_msg, sizeof(assoc_event_msg));
		if (WMODE_CAP_5G(PhyMode))
			count = snprintf(assoc_event_msg, sizeof(assoc_event_msg),
					""MACSTR" BSS(%d)",
					MAC2STR(pEntry->Addr), (pEntry->func_tb_idx) + WIFI_50_RADIO);
		else
			count = snprintf(assoc_event_msg, sizeof(assoc_event_msg),
					""MACSTR" BSS(%d)",
					MAC2STR(pEntry->Addr), (pEntry->func_tb_idx) + WIFI_24_RADIO);

		ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_EVENT, STA_ASSOC,
			assoc_event_msg, count);

		/* Log this cleint's capabilities in our nvram */
		/* assoc_sta_info = kmalloc(1300, GFP_ATOMIC); */
		os_alloc_mem(NULL, (UCHAR **)&assoc_sta_info, 1300);
		if (assoc_sta_info) {
			NdisZeroMemory(assoc_sta_info, 1300);
			count = 0;
			count += snprintf((assoc_sta_info+count), (1300-count),
			"Association: ("MACSTR") --> %s%d (%s)\n",
			MAC2STR(pEntry->Addr), INF_MAIN_DEV_NAME, pEntry->func_tb_idx,
			pEntry->pMbss->Ssid);
			if (pHTCap && pHTCapParm && ie_list->ht_cap_len && WMODE_CAP_N(wdev->PhyMode)) {
				count += snprintf((assoc_sta_info+count), (1300-count), "  Station Info:\n");
				count += snprintf((assoc_sta_info+count), (1300-count),
				"\tRSSI0(%d), RSSI1(%d), Mode(%s), BW(%s), MCS(%d), SGI(%d)\n",
				ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
				ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
				get_phymode_str(pEntry->HTPhyMode.field.MODE),
				get_bw_str(pEntry->HTPhyMode.field.BW),
				pEntry->HTPhyMode.field.MCS,
				pEntry->HTPhyMode.field.ShortGI);
			} else {
				count += snprintf((assoc_sta_info+count), (1300-count),
					"	 Station Info (Legacy):\n");
				count += snprintf((assoc_sta_info+count), (1300-count),
					"\tRSSI0(%d), RSSI1(%d), Mode(%s), MCS(%d)\n",
					ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
					ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
					get_phymode_str(pEntry->HTPhyMode.field.MODE),
					pEntry->HTPhyMode.field.MCS);
			}
			ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST,
			WLAN_LOG_SAVE, 0, assoc_sta_info, count);
			os_free_mem(assoc_sta_info);
		}
	}
#endif

LabelOK:
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/* fix memory leak when trigger scan continuously*/
	if (ie_list && ie_list->CustomerVendorIE.pointer)
		os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	if (ie_list != NULL) {
		os_free_mem(ie_list);
		if (pEntry)
			pEntry->ie_list = NULL;
	}
#ifdef RT_CFG80211_SUPPORT

	if (StatusCode != MLME_SUCCESS)
		CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, pEntry->wdev->if_dev);

#endif /* RT_CFG80211_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	if (pFtInfoBuf != NULL)
		os_free_mem(pFtInfoBuf);
#endif /* DOT11R_FT_SUPPORT */

	if (pOutBuffer != NULL)
		MlmeFreeMemory((PVOID) pOutBuffer);

	return;
}
#endif /* HOSTAPD_11R_SUPPORT */


INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
    UCHAR	*pBuf = NULL;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */

	if (pData != NULL) {
#ifdef CONFIG_AP_SUPPORT
		struct ieee80211_mgmt *mgmt;
#endif /* CONFIG_AP_SUPPORT */
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
		UINT8 apidx;
#endif

		{
#ifdef RT_CFG80211_SUPPORT
	os_alloc_mem(NULL, (UCHAR **)&pBuf, Data);
	if (pBuf != NULL)
		NdisCopyMemory(pBuf, pData, Data);
	else {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CFG_TX_STATUS: MEM ALLOC ERROR\n");
		return NDIS_STATUS_FAILURE;
	}
#else
			PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

			pCfg80211_ctrl->TxStatusInUsed = TRUE;
			pCfg80211_ctrl->TxStatusSeq = pAd->Sequence;

			if (pCfg80211_ctrl->pTxStatusBuf != NULL) {
				os_free_mem(pCfg80211_ctrl->pTxStatusBuf);
				pCfg80211_ctrl->pTxStatusBuf = NULL;
			}

			os_alloc_mem(NULL, (UCHAR **)&pCfg80211_ctrl->pTxStatusBuf, Data);

			if (pCfg80211_ctrl->pTxStatusBuf != NULL) {
				NdisCopyMemory(pCfg80211_ctrl->pTxStatusBuf, pData, Data);
				pCfg80211_ctrl->TxStatusBufLen = Data;
			} else {
				pCfg80211_ctrl->TxStatusBufLen = 0;
				MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CFG_TX_STATUS: MEM ALLOC ERROR\n");
				return NDIS_STATUS_FAILURE;
			}

#endif
			CFG80211_CheckActionFrameType(pAd, "TX", pData, Data);
#ifdef CONFIG_AP_SUPPORT
			mgmt = (struct ieee80211_mgmt *)pData;
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
			apidx = get_apidx_by_addr(pAd, mgmt->sa);
#endif
			if (ieee80211_is_probe_resp(mgmt->frame_control)) {
				INT offset = sizeof(HEADER_802_11) + 12;
#ifdef DISABLE_HOSTAPD_PROBE_RESP
#ifndef HOSTAPD_11R_SUPPORT
			UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);
#endif
			CFG80211_SyncPacketWpsIe(pAd, pData + offset, Data - offset, apidx, mgmt->da);
			goto LabelOK;
#else
			CFG80211_SyncPacketWmmIe(pAd, pData + offset, Data - offset);
#endif
			}

			if ((ieee80211_is_auth(mgmt->frame_control)) && (mgmt->u.auth.auth_alg != AUTH_MODE_FT) &&
				(mgmt->u.auth.auth_alg != AUTH_MODE_SAE)) {
#ifdef RADIUS_MAC_AUTH_SUPPORT
				MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAd, mgmt->da);

				if (pEntry != NULL && pEntry->wdev->radius_mac_auth_enable) {
					if (mgmt->u.auth.status_code == MLME_SUCCESS) {
						pEntry->bAllowTraffic = TRUE;
					} else {
					pEntry->bAllowTraffic = FALSE;
					MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				}
			}
#endif
			goto LabelOK;
		}
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
			if (ieee80211_is_auth(mgmt->frame_control) &&
				((mgmt->u.auth.auth_alg == AUTH_MODE_FT) || (mgmt->u.auth.auth_alg == AUTH_MODE_SAE))) {
				CFG80211_AuthRespHandler(pAd, pData, Data);
				MiniportMMRequest(pAd, 0, pData, Data);
				if (pBuf) {
					CFG80211OS_TxStatus(pAd->ApCfg.MBSSID[apidx].wdev.if_dev, 5678,
								pBuf, Data, 1);
				}
				goto LabelOK;
			}
			if (ieee80211_is_reassoc_resp(mgmt->frame_control)
				|| ieee80211_is_assoc_resp(mgmt->frame_control)) {
				CFG80211_AssocRespHandler(pAd, pData, Data);
				if (pBuf) {
					CFG80211OS_TxStatus(pAd->ApCfg.MBSSID[apidx].wdev.if_dev, 5678,
						pBuf, Data, 1);
				}
				goto LabelOK;
			}
#endif

#endif /* CONFIG_AP_SUPPORT */
			MiniportMMRequest(pAd, 0, pData, Data);
		}
	}
LabelOK:
	if (pBuf != NULL)
		os_free_mem(pBuf);

	return 0;
}

VOID CFG80211_SendMgmtFrameDone(RTMP_ADAPTER *pAd, USHORT Sequence, BOOLEAN ack)
{
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (pCfg80211_ctrl->TxStatusInUsed && pCfg80211_ctrl->pTxStatusBuf
		/*&& (pAd->TxStatusSeq == pHeader->Sequence)*/) {
		MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "CFG_TX_STATUS: REAL send %d\n", Sequence);
		CFG80211OS_TxStatus(CFG80211_GetEventDevice(pAd), 5678,
							pCfg80211_ctrl->pTxStatusBuf, pCfg80211_ctrl->TxStatusBufLen,
							ack);
		pCfg80211_ctrl->TxStatusSeq = 0;
		pCfg80211_ctrl->TxStatusInUsed = FALSE;
	}

}
#ifdef CONFIG_AP_SUPPORT
VOID CFG80211_ParseBeaconIE(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, struct wifi_dev *wdev, UCHAR *wpa_ie, UCHAR *rsn_ie)
{
	PEID_STRUCT		 pEid;
	PUCHAR				pTmp;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;
	NDIS_802_11_ENCRYPTION_STATUS	PairCipher;		/* Unicast cipher 1, this one has more secured cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS	PairCipherAux;	/* Unicast cipher 2 if AP announce two unicast cipher suite */
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	BOOLEAN bWPA = FALSE;
	BOOLEAN bWPA2 = FALSE;
	BOOLEAN bMix = FALSE;

#ifdef DISABLE_HOSTAPD_BEACON
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
    const UCHAR CFG_WPA_EID = WLAN_EID_VENDOR_SPECIFIC;
#else
    const UCHAR CFG_WPA_EID = WLAN_EID_WPA;
#endif /* LINUX_VERSION_CODE: 3.8.0 */
#endif

	/* Security */
	PairCipher	 = Ndis802_11WEPDisabled;
	PairCipherAux = Ndis802_11WEPDisabled;
	CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
	CLEAR_PAIRWISE_CIPHER(&wdev->SecConfig);
	CLEAR_GROUP_CIPHER(&wdev->SecConfig);
#ifdef DOT11W_PMF_SUPPORT
	wdev->SecConfig.PmfCfg.MFPC = 0;
	wdev->SecConfig.PmfCfg.MFPR = 0;
	wdev->SecConfig.PmfCfg.igtk_cipher = 0;
	/* Clear Previous values of flags */
	wdev->SecConfig.PmfCfg.Desired_MFPC = 0;
	wdev->SecConfig.PmfCfg.Desired_MFPR = 0;
	wdev->SecConfig.PmfCfg.Desired_PMFSHA256 = 0;
#endif

	if ((wpa_ie == NULL) && (rsn_ie == NULL)) { /* open case */
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s:: Open/None case\n", __func__);
		/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
		/* wdev->WepStatus = Ndis802_11WEPDisabled; */
		/* wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE; */
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
	}

	if (wpa_ie != NULL) { /* wpapsk/tkipaes case */
		pEid = (PEID_STRUCT)wpa_ie;
		pTmp = (PUCHAR)pEid;

		if (os_equal_mem(pEid->Octet, WPA_OUI, 4)) {
			/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
			/* SET_AKM_OPEN(wdev->SecConfig.AKMMap); */
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s:: WPA case\n", __func__);
			bWPA = TRUE;
			pTmp   += 11;

			switch (*pTmp) {
			case 1:
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Group Ndis802_11GroupWEP40Enabled\n");
				/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP40Enabled; */
				SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
				break;

			case 5:
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Group Ndis802_11GroupWEP104Enabled\n");
				/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP104Enabled; */
				SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);
				break;

			case 2:
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Group Ndis802_11TKIPEnable\n");
				/* wdev->GroupKeyWepStatus  = Ndis802_11TKIPEnable; */
				SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
				break;

			case 4:
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Group Ndis802_11AESEnable\n");
				/* wdev->GroupKeyWepStatus  = Ndis802_11AESEnable; */
				SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
				break;

			default:
				break;
			}

			/* number of unicast suite*/
			pTmp   += 1;
			/* skip all unicast cipher suites*/
			/*Count = *(PUSHORT) pTmp;				*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* Parsing all unicast cipher suite*/
			while (Count > 0) {
				/* Skip OUI*/
				pTmp += 3;
				TmpCipher = Ndis802_11WEPDisabled;

				switch (*pTmp) {
				case 1:
				case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
					TmpCipher = Ndis802_11WEPEnabled;
					break;

				case 2:
					TmpCipher = Ndis802_11TKIPEnable;
					break;

				case 4:
					TmpCipher = Ndis802_11AESEnable;
					break;

				default:
					break;
				}

				if (TmpCipher > PairCipher) {
					/* Move the lower cipher suite to PairCipherAux*/
					PairCipherAux = PairCipher;
					PairCipher	= TmpCipher;
				} else
					PairCipherAux = TmpCipher;

				pTmp++;
				Count--;
			}

			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Auth Count in WPA = %d ,we only parse the first for AKM\n", Count);
			pTmp   += 3; /* parse first AuthOUI for AKM */

			switch (*pTmp) {
			case 1:
				/* Set AP support WPA-enterprise mode*/
				/* wdev->AuthMode = Ndis802_11AuthModeWPA; */
				SET_AKM_WPA1(wdev->SecConfig.AKMMap);
				break;

			case 2:
				/* Set AP support WPA-PSK mode*/
				/* wdev->AuthMode = Ndis802_11AuthModeWPAPSK; */
				SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
				break;

			default:
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "UNKNOWN AKM 0x%x IN WPA,please check!\n", *pTmp);
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "AuthMode = 0x%x\n", wdev->SecConfig.AKMMap);

			/* if (wdev->GroupKeyWepStatus == PairCipher) */
			if ((PairCipher == Ndis802_11WEPDisabled && IS_CIPHER_NONE(wdev->SecConfig.GroupCipher)) ||
				(PairCipher == Ndis802_11WEPEnabled && IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) ||
				(PairCipher == Ndis802_11TKIPEnable && IS_CIPHER_TKIP(wdev->SecConfig.GroupCipher)) ||
				(PairCipher == Ndis802_11AESEnable && IS_CIPHER_CCMP128(wdev->SecConfig.GroupCipher))
			   ) {
				/* wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE; */
				/* pMbss->wdev.WepStatus=wdev->GroupKeyWepStatus; */
				wdev->SecConfig.PairwiseCipher = wdev->SecConfig.GroupCipher;
			} else {
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPA Mix TKIPAES\n");
				bMix = TRUE;
			}

			pMbss->RSNIE_Len[0] = wpa_ie[1];
			os_move_mem(pMbss->RSN_IE[0], wpa_ie + 2, wpa_ie[1]); /* copy rsn ie */
#ifdef DISABLE_HOSTAPD_BEACON
			pMbss->RSNIE_ID[0] = CFG_WPA_EID;
#endif
		} else {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:: wpa open/none case\n", __func__);
			/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
			/* wait until wpa/wpa2 all not exist , then set open/none */
		}
	}

	if (rsn_ie != NULL) {
		PRSN_IE_HEADER_STRUCT			pRsnHeader;
		PCIPHER_SUITE_STRUCT			pCipher;

		UCHAR                           Len;

		pEid = (PEID_STRUCT)rsn_ie;
		Len	= pEid->Len + 2;
		pTmp = (PUCHAR)pEid;
		pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

		/* 0. Version must be 1*/
		if (le2cpu16(pRsnHeader->Version) == 1) {
			pTmp   += sizeof(RSN_IE_HEADER_STRUCT);
			Len	   -= sizeof(RSN_IE_HEADER_STRUCT);

			/* 1. Check group cipher*/
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

			if (os_equal_mem(pTmp, RSN_OUI, 3)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s:: WPA2 case\n", __func__);
				bWPA2 = TRUE;

				/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
				/* SET_AKM_OPEN(wdev->SecConfig.AKMMap); */
				switch (pCipher->Type) {
				case 1:
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Group Ndis802_11GroupWEP40Enabled\n");
					/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP40Enabled; */
					SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
					break;

				case 5:
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Group Ndis802_11GroupWEP104Enabled\n");
					/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP104Enabled; */
					SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);
					break;

				case 2:
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Group Ndis802_11TKIPEnable\n");
					/* wdev->GroupKeyWepStatus  = Ndis802_11TKIPEnable; */
					SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
					break;

				case 4:
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Group Ndis802_11AESEnable\n");
					/* wdev->GroupKeyWepStatus  = Ndis802_11AESEnable; */
					SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
					break;
				case 8:
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						(" Group Ndis802_11GCMP128Enable\n"));
					SET_CIPHER_GCMP128(wdev->SecConfig.GroupCipher);
					break;
				case 9:
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						(" Group Ndis802_11GCMP256Enable\n"));
					SET_CIPHER_GCMP256(wdev->SecConfig.GroupCipher);
					break;
				case 10:
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						(" Group Ndis802_11CCMP256Enable\n"));
					SET_CIPHER_CCMP256(wdev->SecConfig.GroupCipher);
					break;
				default:
					break;
				}

				/* set to correct offset for next parsing*/
				pTmp   += sizeof(CIPHER_SUITE_STRUCT);
				Len    -= sizeof(CIPHER_SUITE_STRUCT);

				/* 2. Get pairwise cipher counts*/
				/*Count = *(PUSHORT) pTmp;*/
				Count = (pTmp[1] << 8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				Len    -= sizeof(USHORT);

				/* 3. Get pairwise cipher*/
				/* Parsing all unicast cipher suite*/
				while (Count > 0) {
					/* Skip OUI*/
					pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
					TmpCipher = Ndis802_11WEPDisabled;

					switch (pCipher->Type) {
					case 1:
					case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
						TmpCipher = Ndis802_11WEPEnabled;
						break;

					case 2:
						TmpCipher = Ndis802_11TKIPEnable;
						break;

					case 4:
						TmpCipher = Ndis802_11AESEnable;
						break;

					case 8:
						TmpCipher = Ndis802_11GCMP128Enable;
						break;
							case 9:
								TmpCipher = Ndis802_11GCMP256Enable;
								break;
							case 10:
								TmpCipher = Ndis802_11CCMP256Enable;
								break;
					default:
						break;
					}

					/* pMbss->wdev.WepStatus = TmpCipher; */
					if (TmpCipher > PairCipher) {
						/* Move the lower cipher suite to PairCipherAux*/
						PairCipherAux = PairCipher;
						PairCipher	 = TmpCipher;
					} else
						PairCipherAux = TmpCipher;

					pTmp += sizeof(CIPHER_SUITE_STRUCT);
					Len  -= sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}

				/* 4. get AKM suite counts*/
				/*Count	= *(PUSHORT) pTmp;*/
				Count = (pTmp[1] << 8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				Len    -= sizeof(USHORT);

				/* 5. Get AKM ciphers*/
				/* Parsing all AKM ciphers*/
				while (Count > 0) {
					pAKM = (PAKM_SUITE_STRUCT) pTmp;


#ifdef HOSTAPD_HS_R3_SUPPORT
					if (RTMPEqualMemory(pTmp, OSEN_OUI, 4)) {
						SET_AKM_OSEN(wdev->SecConfig.AKMMap);
						pTmp   += sizeof(AKM_SUITE_STRUCT);
						Len    -= sizeof(AKM_SUITE_STRUCT);
						Count--;
						continue;
					}
#endif
					if (!RTMPEqualMemory(pTmp, RSN_OUI, 3)
#if defined(HOSTAPD_MAPR3_SUPPORT) && defined(DPP_SUPPORT)
						&& !RTMPEqualMemory(pTmp, DPP_OUI, 3)
#endif
					)

						break;
					switch (pAKM->Type) {
					case 0:
						/* wdev->AuthMode = Ndis802_11AuthModeWPANone; */
						SET_AKM_OPEN(wdev->SecConfig.AKMMap);
						break;

					case 1:
						/* Set AP support WPA-enterprise mode*/
						/* wdev->AuthMode = Ndis802_11AuthModeWPA2; */
						SET_AKM_WPA2(wdev->SecConfig.AKMMap);
						break;

					case 2:
						/* Set AP support WPA-PSK or DPP mode*/
						/* wdev->AuthMode = Ndis802_11AuthModeWPA2PSK; */
#if defined(HOSTAPD_MAPR3_SUPPORT) && defined(DPP_SUPPORT)
						if (RTMPEqualMemory(pTmp, RSN_OUI, 3))
							SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
						if (RTMPEqualMemory(pTmp, DPP_OUI, 3)) {
							SET_AKM_DPP(wdev->SecConfig.AKMMap);
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DPP AKM is set\n");
						}
#else
						SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
#endif /* DPP_SUPPORT */
						break;
#ifdef HOSTAPD_11R_SUPPORT
					case 3:
						/* Set AP support FT WPA-enterprise mode*/
						/* wdev->AuthMode = Ndis802_11AuthModeWPA2; */
						SET_AKM_FT_WPA2(wdev->SecConfig.AKMMap);
						break;
					case 4:
						/* Set AP support WPA-PSK mode*/
						/* wdev->AuthMode = Ndis802_11AuthModeWPA2PSK; */
						SET_AKM_FT_WPA2PSK(wdev->SecConfig.AKMMap);
						break;
#endif /* HOSTAPD_11R_SUPPORT */
					case 5:
						/* Set AP support WPA-PSK-EAP256 mode*/
#ifdef DOT11W_PMF_SUPPORT
						SET_AKM_WPA2(wdev->SecConfig.AKMMap);
						wdev->SecConfig.PmfCfg.Desired_PMFSHA256 = 1;
#else
						SET_AKM_WPA2_SHA256(wdev->SecConfig.AKMMap);
#endif /*DOT11W_PMF_SUPPORT*/
						break;
					case 6:
						/* Set AP support WPA-PSK-SHA256 mode*/
#ifdef DOT11W_PMF_SUPPORT
						SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
						wdev->SecConfig.PmfCfg.Desired_PMFSHA256 = 1;
#else
						SET_AKM_WPA2PSK_SHA256(wdev->SecConfig.AKMMap);
#endif /*DOT11W_PMF_SUPPORT*/
						break;

#ifdef HOSTAPD_SAE_SUPPORT
							case 8:
								/*Set AP Support SAE SHA256 */
								SET_AKM_SAE_SHA256(wdev->SecConfig.AKMMap);
								break;
							case 9:
								SET_AKM_FT_SAE_SHA256(wdev->SecConfig.AKMMap);
								break;
#endif
#ifdef HOSTAPD_SUITEB_SUPPORT
							case 11:
								SET_AKM_SUITEB_SHA256(wdev->SecConfig.AKMMap);
								break;

							case 12:
								SET_AKM_SUITEB_SHA384(wdev->SecConfig.AKMMap);
								break;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
							case 18:
								SET_AKM_OWE(wdev->SecConfig.AKMMap);
								break;

#endif
					default:
						/* wdev->AuthMode = Ndis802_11AuthModeMax; */
						SET_AKM_OPEN(wdev->SecConfig.AKMMap);
						break;
					}

					pTmp   += sizeof(AKM_SUITE_STRUCT);
					Len    -= sizeof(AKM_SUITE_STRUCT);
					Count--;
				}

#ifdef DISABLE_HOSTAPD_BEACON
				/*check for no pairwise, pmf, ptksa, gtksa counters */
				if (Len >= 2)
				{
					memcpy(wdev->SecConfig.RsnCap, pTmp, 2);
#ifdef DOT11W_PMF_SUPPORT
					{
						RSN_CAPABILITIES RsnCap;

						NdisMoveMemory(&RsnCap, pTmp, sizeof(RSN_CAPABILITIES));
						RsnCap.word = cpu2le16(RsnCap.word);
						if (RsnCap.field.MFPC == 1)
							wdev->SecConfig.PmfCfg.Desired_MFPC = 1;
						if (RsnCap.field.MFPR == 1) {
							wdev->SecConfig.PmfCfg.Desired_MFPR = 1;
							wdev->SecConfig.PmfCfg.Desired_PMFSHA256 = 1;
						}
					}
#endif	 /*DOT11W_PMF_SUPPORT*/
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Copied Rsn cap %02x %02x \n", wdev->SecConfig.RsnCap[0], wdev->SecConfig.RsnCap[1]);
				}
				pTmp += sizeof(RSN_CAPABILITIES);
				Len  -= sizeof(RSN_CAPABILITIES);
				/*Extract PMKID list */
				if (Len >= sizeof(UINT16)) {
					INT offset = sizeof(UINT16);

					Count = (pTmp[1] << 8) + pTmp[0];
					if (Count > 0) {
						offset += Count*LEN_PMKID;
					}
					pTmp += offset;
					Len -= offset;
				}
#ifdef DOT11W_PMF_SUPPORT
				if (Len >= LEN_OUI_SUITE) {
					UCHAR OUI_PMF_BIP_CMAC_128_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x06};
					UCHAR OUI_PMF_BIP_CMAC_256_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0d};
					UCHAR OUI_PMF_BIP_GMAC_128_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0b};
					UCHAR OUI_PMF_BIP_GMAC_256_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0c};

					if (RTMPEqualMemory(pTmp, OUI_PMF_BIP_CMAC_128_CIPHER, LEN_OUI_SUITE))
						SET_CIPHER_BIP_CMAC128(wdev->SecConfig.PmfCfg.igtk_cipher);
					else if (RTMPEqualMemory(pTmp,
						OUI_PMF_BIP_CMAC_256_CIPHER, LEN_OUI_SUITE))
						SET_CIPHER_BIP_CMAC256(wdev->SecConfig.PmfCfg.igtk_cipher);
					else if (RTMPEqualMemory(pTmp,
						OUI_PMF_BIP_GMAC_128_CIPHER, LEN_OUI_SUITE))
						SET_CIPHER_BIP_GMAC128(wdev->SecConfig.PmfCfg.igtk_cipher);
					else if (RTMPEqualMemory(pTmp,
						OUI_PMF_BIP_GMAC_256_CIPHER, LEN_OUI_SUITE))
						SET_CIPHER_BIP_GMAC256(wdev->SecConfig.PmfCfg.igtk_cipher);
					else
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Group Mgmt Cipher Not Supported \n");
				}
#endif
#endif
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "AuthMode = 0x%x\n", wdev->SecConfig.AKMMap);

				if ((PairCipher == Ndis802_11WEPDisabled && IS_CIPHER_NONE(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11WEPEnabled && IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11TKIPEnable && IS_CIPHER_TKIP(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11AESEnable && IS_CIPHER_CCMP128(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11GCMP128Enable && IS_CIPHER_GCMP128(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11GCMP256Enable && IS_CIPHER_GCMP256(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11CCMP256Enable && IS_CIPHER_CCMP256(wdev->SecConfig.GroupCipher))
				) {
					wdev->SecConfig.PairwiseCipher = wdev->SecConfig.GroupCipher;
				} else {
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPA2 Mix TKIPAES\n");
					bMix = TRUE;
				}

				if (bWPA2 && bWPA) {
					pMbss->RSNIE_Len[1] = rsn_ie[1];
					NdisMoveMemory(pMbss->RSN_IE[1], rsn_ie + 2, rsn_ie[1]); /* copy rsn ie */
#ifdef DISABLE_HOSTAPD_BEACON
					pMbss->RSNIE_ID[1] = WLAN_EID_RSN;
#endif
				} else {
					pMbss->RSNIE_Len[0] = rsn_ie[1];
					os_move_mem(pMbss->RSN_IE[0], rsn_ie + 2, rsn_ie[1]); /* copy rsn ie */
#ifdef DISABLE_HOSTAPD_BEACON
					pMbss->RSNIE_ID[0] = WLAN_EID_RSN;
#endif
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:: wpa2 Open/None case\n", __func__);
				/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
				/* wait until wpa/wpa2 all not exist , then set open/none */
			}
		}
	}

	if (bWPA2 && bWPA) {
		/* wdev->AuthMode = Ndis802_11AuthModeWPA1PSKWPA2PSK; */
		SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
		SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);

		if (bMix) {
			/* wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES; */
			/* wdev->WepStatus = Ndis802_11TKIPAESMix; */
			SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
		}
	} else if (bWPA2) {
		if (bMix) {
			/* wdev->WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES; */
			/* wdev->WepStatus = Ndis802_11TKIPAESMix; */
			SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
			SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
		}
	} else if (bWPA) {
		if (bMix) {
			/* wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE; */
			/* wdev->WepStatus = Ndis802_11TKIPAESMix; */
			SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
			SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
		}
	} else {
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
	}

	if (IS_AKM_WPA1(wdev->SecConfig.AKMMap) || IS_AKM_WPA2(wdev->SecConfig.AKMMap))
		wdev->SecConfig.IEEE8021X = TRUE;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "\nCFG80211 BEACON => bwpa2 %d, bwpa %d, bmix %d,AuthMode = %s ,wdev->PairwiseCipher = %s wdev->SecConfig.GroupCipher = %s\n"
			  , bWPA2, bWPA, bMix
			  , GetAuthModeStr(wdev->SecConfig.AKMMap), GetEncryModeStr(wdev->SecConfig.PairwiseCipher), GetEncryModeStr(wdev->SecConfig.GroupCipher));
}
#endif /* CONFIG_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

