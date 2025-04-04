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
    assoc.c

    Abstract:
    Handle association related requests either from WSTA or from local MLME

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"


extern UCHAR	CISCO_OUI[];
extern UCHAR	WPA_OUI[];
extern UCHAR	RSN_OUI[];
extern UCHAR	WME_INFO_ELEM[];
extern UCHAR	WME_PARM_ELEM[];
extern UCHAR	RALINK_OUI[];
extern UCHAR	BROADCOM_OUI[];
extern UCHAR	WPS_OUI[];

#if defined(RT_CFG80211_SUPPORT) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
const UCHAR apple_oui[3] = {0x00, 0x17, 0xf2};
#define APPLE_IE_OUI_TYPE 0x0A
#endif


#ifdef IGMP_TVM_SUPPORT
extern UCHAR IGMP_TVM_OUI[];
#endif /* IGMP_TVM_SUPPORT */


struct _assoc_api_ops ap_assoc_api;



#ifdef IAPP_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Update Frame to update forwarding table in Layer 2 devices.

 Arguments:
    *mac_p - the STATION MAC address pointer

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN IAPP_L2_Update_Frame_Send(RTMP_ADAPTER *pAd, UINT8 *mac, INT wdev_idx)
{
	NDIS_PACKET	*pNetBuf;
	struct wifi_dev *wdev;

	if ((wdev_idx < 0) || (wdev_idx >= WDEV_NUM_MAX)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev_idx is invalid.\n");
		return FALSE;
	}

	wdev = pAd->wdev_list[wdev_idx];
	pNetBuf = RtmpOsPktIappMakeUp(get_netdev_from_bssid(pAd, wdev_idx), mac);

	if (pNetBuf == NULL)
		return FALSE;

	/*Which band is this packet transfer*/
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if (wf_drv_tbl.wf_fwd_needed_hook != NULL && wf_drv_tbl.wf_fwd_needed_hook() == TRUE)
		set_wf_fwd_cb(pAd, pNetBuf, wdev);

#endif /* CONFIG_WIFI_PKT_FWD */
	/* UCOS: update the built-in bridge, too (don't use gmac.xmit()) */
	announce_802_3_packet(pAd, pNetBuf, OPMODE_AP);
	IAPP_L2_UpdatePostCtrl(pAd, mac, wdev_idx);
	return TRUE;
} /* End of IAPP_L2_Update_Frame_Send */
#endif /* IAPP_SUPPORT */

static USHORT update_associated_mac_entry(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN IE_LISTS * ie_list,
	IN UCHAR MaxSupportedRate,
	IN BOOLEAN isReassoc)
{
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
#ifdef TXBF_SUPPORT
	BOOLEAN	 supportsETxBF = FALSE;
#endif /* TXBF_SUPPORT // */
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
	USHORT PhyMode;
	UCHAR Channel;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	BOOLEAN need_clr_set_wtbl = FALSE;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	BOOLEAN is_cap_ht = FALSE, is_cap_vht = FALSE, is_cap_he = FALSE;
	UCHAR i = 0;
	UCHAR peer_bw_by_opclass = BW_NUM;
	USHORT peer_phymode;

	ASSERT((pEntry->func_tb_idx < pAd->ApCfg.BssidNum));
	mbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
	wdev = &mbss->wdev;
	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
	PhyMode = wdev->PhyMode;
	Channel = wdev->channel;
	/* use this to decide reassociation rsp need to clear set wtbl or not */
	if (pEntry->Sst == SST_ASSOC && isReassoc == TRUE)
		need_clr_set_wtbl = TRUE;

	/* Update auth, wep, legacy transmit rate setting . */
	pEntry->Sst = SST_ASSOC;
	/* Use peer mode to get peer working bw by op class IE. */
	peer_phymode = wdev->PhyMode;
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(peer_phymode) && !(HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)))
		peer_phymode &= ~WMODE_AC;
#endif
#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(peer_phymode) && !(HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)))
		peer_phymode &= ~(WMODE_AX_24G | WMODE_AX_5G | WMODE_AX_6G);
#endif
	get_spacing_by_reg_class(pAd, ie_list->current_opclass, peer_phymode, &peer_bw_by_opclass);
	pEntry->MaxSupportedRate = min(wdev->rate.MaxTxRate, MaxSupportedRate);
	MacTableSetEntryPhyCfg(pAd, pEntry);
	pEntry->CapabilityInfo = ie_list->CapabilityInfo;

	if (IS_AKM_PSK_Entry(pEntry)) {
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
	}

#ifdef DOT1X_SUPPORT
	else if (IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(wdev)) {
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION;
	}

#endif /* DOT1X_SUPPORT */
	/* Ralink proprietary Piggyback and Aggregation support for legacy RT61 chip */
	MacTableSetEntryRaCap(pAd, pEntry, &ie_list->cmm_ies.vendor_ie);
#ifdef DOT11_VHT_AC
	if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
		MacTableEntryCheck2GVHT(pAd, pEntry);
#endif /* DOT11_VHT_AC */


		tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	/* There are some situation to need to encryption by software
	   1. The Client support PMF. It shall ony support AES cipher.
	   2. The Client support WAPI.
	   If use RT3883 or later, HW can handle the above.
	   */
#ifdef DOT11W_PMF_SUPPORT

	if ((cap->FlgPMFEncrtptMode == PMF_ENCRYPT_MODE_0)
		&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE))
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT);

#endif /* DOT11W_PMF_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
		if ((IS_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher) ||
			IS_CIPHER_WEP40(pEntry->SecConfig.PairwiseCipher) ||
			IS_CIPHER_WEP104(pEntry->SecConfig.PairwiseCipher)) && pEntry->bSw == TRUE) {
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT);
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "wcid(%u), hw_wcid(%u), SW Encrypt!\n", pEntry->wcid, pEntry->hw_wcid);
		}
#endif /* SW_CONNECT_SUPPORT */

#endif /* SOFT_ENCRYPT */

	if (wdev->bWmmCapable && ie_list->bWmmCapable)
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
	else
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

#ifdef DOT11_N_SUPPORT

	/*
		WFA recommend to restrict the encryption type in 11n-HT mode.
		So, the WEP and TKIP are not allowed in HT rate.
	*/
	if (pAd->CommonCfg.HT_DisallowTKIP &&
		IS_INVALID_HT_SECURITY(pEntry->SecConfig.PairwiseCipher)) {
		/* Force to None-HT mode due to WiFi 11n policy */
		CLR_HT_CAPS_EXIST(cmm_ies->ie_exists);
#ifdef DOT11_VHT_AC
		CLR_VHT_CAPS_EXIST(cmm_ies->ie_exists);
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
		CLR_HE_CAPS_EXIST(cmm_ies->ie_exists);
#endif /* DOT11_HE_AX */
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Force the STA as Non-HT mode\n");
	}

	/* If this Entry supports 802.11n, upgrade to HT rate. */
	if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists) &&
		(wdev->DesiredHtPhyInfo.bHtEnable) &&
		WMODE_CAP_N(PhyMode)) {
		is_cap_ht = TRUE;
		ht_mode_adjust(pAd, pEntry, &cmm_ies->ht_cap);

#ifdef DOT11N_DRAFT3

		if (ie_list->ExtCapInfo.BssCoexistMgmtSupport)
			pEntry->BSS2040CoexistenceMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#if defined(CONFIG_HOTSPOT_R2) || defined(QOS_R1)
		if (ie_list->ExtCapInfo.qosmap)
			pEntry->QosMapSupport = 1;
#endif
#ifdef QOS_R1
if (ie_list->ExtCapInfo.dot11MSCSActivated)
	pEntry->MSCSSupport = 1;
#endif
#ifdef CONFIG_DOT11V_WNM
			if (ie_list->ExtCapInfo.BssTransitionManmt)
				pEntry->BssTransitionManmtSupport = 1;
#endif /* CONFIG_DOT11V_WNM */

		/* 40Mhz BSS Width Trigger events */
		if (
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		/* Soft AP to follow BW of Root AP */
		(IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd, HcGetBandByWdev(wdev)) == FALSE) &&
#endif
			ie_list->cmm_ies.ht_cap.HtCapInfo.Forty_Mhz_Intolerant) {

#ifdef DOT11N_DRAFT3
			UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
			UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
			UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

			pEntry->bForty_Mhz_Intolerant = TRUE;
			pAd->MacTab.fAnyStaFortyIntolerant = TRUE;

			if (((cfg_ht_bw == HT_BW_40) &&
				WMODE_CAP_2G(wdev->PhyMode)) &&
				((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
				 (op_ht_bw != HT_BW_20) &&
				 (op_ext_cha != 0))
			   ) {
				pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
				wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
				pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
				MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "%s, Update Beacon for mbss_idx:%d\n", __func__, pEntry->func_tb_idx);
				UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
			}

			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "pEntry set 40MHz Intolerant as 1\n");
#endif /* DOT11N_DRAFT3 */
			Handle_BSS_Width_Trigger_Events(pAd, Channel);
		}

#ifdef TXBF_SUPPORT
#ifdef DOT11_VHT_AC

		if (WMODE_CAP_AC(PhyMode) && WMODE_CAP_5G(PhyMode) &&
			HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists))
			supportsETxBF = mt_WrapClientSupportsVhtETxBF(pAd, &ie_list->cmm_ies.vht_cap.vht_cap);
		else
#endif /* DOT11_VHT_AC */
			supportsETxBF = mt_WrapClientSupportsETxBF(pAd, &cmm_ies->ht_cap.TxBFCap);

#endif /* TXBF_SUPPORT */
		/* find max fixed rate */
		pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(&wdev->DesiredHtPhyInfo.MCSSet[0],
										 &cmm_ies->ht_cap.MCSSet[0]);

		if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					  "@@@ IF-ra%d DesiredTransmitSetting.field.MCS = %d\n",
					  pEntry->func_tb_idx,
					  wdev->DesiredTransmitSetting.field.MCS);
			set_ht_fixed_mcs(pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
		}

		set_sta_ht_cap(pAd, pEntry, &cmm_ies->ht_cap);
		/* Record the received capability from association request */
		NdisMoveMemory(&pEntry->HTCapability, &cmm_ies->ht_cap, sizeof(HT_CAPABILITY_IE));
#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(PhyMode) &&
			RADIO_IN_ABAND(wdev) &&
			HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)) {
			is_cap_vht = TRUE;
			vht_mode_adjust(pAd, pEntry, &cmm_ies->vht_cap,
					HAS_VHT_OP_EXIST(cmm_ies->ie_exists) ? &cmm_ies->vht_op : NULL,
					(cmm_ies->operating_mode_len == 0) ? NULL :  &cmm_ies->operating_mode,
					(peer_bw_by_opclass != BW_NUM) ? &peer_bw_by_opclass:NULL);
			dot11_vht_mcs_to_internal_mcs(pAd, wdev, &cmm_ies->vht_cap, &pEntry->MaxHTPhyMode);

			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					  "Peer's PhyCap=>Mode:%s, BW:%s, NSS:%d, MCS:%d\n",
					  get_phymode_str(pEntry->MaxHTPhyMode.field.MODE),
					  get_bw_str(pEntry->MaxHTPhyMode.field.BW),
					  ((pEntry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1,
					  (pEntry->MaxHTPhyMode.field.MCS & 0xf));

			set_vht_cap(pAd, pEntry, &cmm_ies->vht_cap);
			NdisMoveMemory(&pEntry->vht_cap_ie, &cmm_ies->vht_cap, sizeof(VHT_CAP_IE));
		}

		if (ie_list->cmm_ies.operating_mode_len == sizeof(OPERATING_MODE) &&
			ie_list->cmm_ies.operating_mode.rx_nss_type == 0) {
			pEntry->operating_mode = ie_list->cmm_ies.operating_mode;
			pEntry->force_op_mode = TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Peer's OperatingMode=>RxNssType: %d, RxNss: %d, ChBW: %d\n",
					  pEntry->operating_mode.rx_nss_type,
					  pEntry->operating_mode.rx_nss,
					  pEntry->operating_mode.ch_width);
		} else
			pEntry->force_op_mode = FALSE;

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Peer's bw: %d, extChanOffset: %d, RecomWidth: %d\n",
			pEntry->MaxHTPhyMode.field.BW,
			cmm_ies->ht_op.AddHtInfo.ExtChanOffset,
			cmm_ies->ht_op.AddHtInfo.RecomWidth);
		if ((pEntry->MaxHTPhyMode.field.BW == HT_BW_40) &&
			WMODE_CAP_2G(wdev->PhyMode) &&
			HAS_HT_OP_EXIST(cmm_ies->ie_exists) &&
			(cmm_ies->ht_op.AddHtInfo.ExtChanOffset == EXTCHA_NONE) &&
			(cmm_ies->ht_op.AddHtInfo.RecomWidth == 0)) {
			pEntry->operating_mode.ch_width = 0;
			pEntry->operating_mode.rx_nss_type = 0;
			pEntry->operating_mode.rx_nss = (wlan_operate_get_rx_stream(wdev) - 1);
			pEntry->force_op_mode = TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Special Peer's OperatingMode=>RxNssType: %d, RxNss: %d, ChBW: %d\n",
				pEntry->operating_mode.rx_nss_type,
				pEntry->operating_mode.rx_nss,
				pEntry->operating_mode.ch_width);
		}

#endif /* DOT11_VHT_AC */
	}

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode) && HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
		is_cap_he = TRUE;
		update_peer_he_caps(pEntry, cmm_ies);
		update_peer_he_operation(pEntry, cmm_ies);
		he_mode_adjust(wdev, pEntry, (peer_bw_by_opclass != BW_NUM) ? &peer_bw_by_opclass:NULL);

		for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
			if(pEntry->cap.rate.he80_rx_nss_mcs[i] == 3)
				break ;
		}
		if(i != 0) {
			i = i <= wlan_operate_get_tx_stream(wdev) ? i : wlan_operate_get_tx_stream(wdev);
			pEntry->MaxHTPhyMode.field.MCS = ((i-1) << 4);
		}
		else {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"STA antenna information provided is incorrect");
		}

		if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 2)
			pEntry->MaxHTPhyMode.field.MCS += HE_MCS_11 ;
		else if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 1)
			pEntry->MaxHTPhyMode.field.MCS += HE_MCS_9 ;
		else if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 0)
			pEntry->MaxHTPhyMode.field.MCS += HE_MCS_7;
		else
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"STA MCS information provided is incorrect");
	}
#endif /*DOT11_HE_AX*/
#ifdef QOS_R1
		if (ie_list->ExtCapInfo.qosmap)
			pEntry->QosMapSupport = 1;
#endif

	if (!is_cap_ht && !is_cap_vht && !is_cap_he) {
#ifdef CONFIG_HOTSPOT_R2
		if (ie_list->ExtCapInfo.qosmap)
			pEntry->QosMapSupport = 1;
#endif
		pAd->MacTab.fAnyStationIsLegacy = TRUE;
		NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
		pEntry->SupportHTMCS = 0;
		pEntry->SupportRateMode &= (~SUPPORT_HT_MODE);
#ifdef DOT11_VHT_AC
		/* TODO: shiang-usw, it's ugly and need to revise it */
		NdisZeroMemory(&pEntry->vht_cap_ie, sizeof(VHT_CAP_IE));
		pEntry->SupportVHTMCS1SS = 0;
		pEntry->SupportVHTMCS2SS = 0;
		pEntry->SupportVHTMCS3SS = 0;
		pEntry->SupportVHTMCS4SS = 0;
		pEntry->SupportRateMode &= (~SUPPORT_VHT_MODE);
#endif /* DOT11_VHT_AC */
	}

#endif /* DOT11_N_SUPPORT */
	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
		pEntry->CurrTxRate = pEntry->MaxSupportedRate;

#ifdef MFB_SUPPORT
	pEntry->lastLegalMfb = 0;
	pEntry->isMfbChanged = FALSE;
	pEntry->fLastChangeAccordingMfb = FALSE;
	pEntry->toTxMrq = TRUE;
	pEntry->msiToTx = 0;/*has to increment whenever a mrq is sent */
	pEntry->mrqCnt = 0;
	pEntry->pendingMfsi = 0;
	pEntry->toTxMfb = FALSE;
	pEntry->mfbToTx = 0;
	pEntry->mfb0 = 0;
	pEntry->mfb1 = 0;
#endif	/* MFB_SUPPORT */
	pEntry->freqOffsetValid = FALSE;
#ifdef TXBF_SUPPORT

	if (cap->FlgHwTxBfCap)
		chip_tx_bf_init(pAd, pEntry, ie_list, supportsETxBF);

#endif /* TXBF_SUPPORT // */
#ifdef MT_MAC

	if (cap->hif_type == HIF_MT) {
		if (wdev->bAutoTxRateSwitch == TRUE)
			pEntry->bAutoTxRateSwitch = TRUE;
		else {
			pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
			pEntry->bAutoTxRateSwitch = FALSE;

			if (pEntry->HTPhyMode.field.MODE >= MODE_VHT) {
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS +
											  ((wlan_operate_get_tx_stream(wdev) - 1) << 4);
			}

			/* If the legacy mode is set, overwrite the transmit setting of this entry. */
			RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
		}

#if !defined(MT7615) && !defined(MT7637) && !defined(MT7622) && !defined(P18) && \
	!defined(MT7663) && !defined(MT7626) && !defined(AXE) && !defined(MT7915) && \
	!defined(MT7986) && !defined(MT7916) && !defined(MT7981)

		if (!IS_MT7615(pAd) && !IS_MT7637(pAd) && !IS_MT7622(pAd) && !IS_P18(pAd) &&
			!IS_MT7663(pAd) && !IS_MT7626(pAd) && !IS_AXE(pAd) && !IS_MT7915(pAd) &&
			!IS_MT7986(pAd) && !IS_MT7916(pAd) && !IS_MT7981(pAd))
			RAInit(pAd, pEntry);

#endif
	}

#endif /* MT_MAC */

	if (IS_NO_SECURITY(&pEntry->SecConfig) || IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher))
		ApLogEvent(pAd, pEntry->Addr, EVENT_ASSOCIATED);


	if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) ||
		(pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
		pAd->MacTab.fAnyStationIsLegacy = TRUE;

	NdisAcquireSpinLock(&pAd->MacTabLock);
	nonerp_sta_num(pEntry, PEER_JOIN);
	NdisReleaseSpinLock(&pAd->MacTabLock);

	ApUpdateCapabilityAndErpIe(pAd, mbss);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	return MLME_SUCCESS;
}


/*
    ==========================================================================
    Description:
       assign a new AID to the newly associated/re-associated STA and
       decide its MaxSupportedRate and CurrTxRate. Both rates should not
       exceed AP's capapbility
    Return:
       MLME_SUCCESS - association successfully built
       others - association failed due to resource issue
    ==========================================================================
 */
#if defined(HOSTAPD_WPA3_SUPPORT) || defined(HOSTAPD_11R_SUPPORT)
USHORT APBuildAssociation
#else
static USHORT APBuildAssociation
#endif
(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN IE_LISTS * ie_list,
	IN UCHAR MaxSupportedRateIn500Kbps,
	OUT USHORT *pAid,
	IN BOOLEAN isReassoc)
{
	USHORT StatusCode = MLME_SUCCESS;
	UCHAR MaxSupportedRate;
	struct wifi_dev *wdev;
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_OWE_SUPPORT
	PUINT8 pPmkid = NULL;
	UINT8 pmkid_count = 0;
#endif /*CONFIG_OWE_SUPPORT*/
#endif /*HOSTAPD_WPA3_SUPPORT*/
#ifdef RATE_PRIOR_SUPPORT
	PBLACK_STA pBlackSta = NULL;
#endif/*RATE_PRIOR_SUPPORT*/


	if (!pEntry)
		return MLME_UNSPECIFY_FAIL;

	wdev = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev;
	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);

	if ((WMODE_EQUAL(wdev->PhyMode, WMODE_G)
#ifdef DOT11_N_SUPPORT
		 || WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
#endif /* DOT11_N_SUPPORT */
		)
		&& (MaxSupportedRate < RATE_FIRST_OFDM_RATE)
	   )
		return MLME_ASSOC_REJ_DATA_RATE;

#ifdef GN_MIXMODE_SUPPORT
	if (pAd->CommonCfg.GNMixMode
		&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
			|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
			|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))
		&& pEntry->FlgIs11bSta)
		return MLME_ASSOC_REJ_DATA_RATE;
#endif /* GN_MIXMODE_SUPPORT */

#ifdef DOT11_N_SUPPORT

	/* 11n only */
	if (WMODE_HT_ONLY(wdev->PhyMode) && !HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
		return MLME_ASSOC_REJ_DATA_RATE;

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode) &&
		WMODE_CAP_5G(wdev->PhyMode) &&
		!HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists) &&
		(pAd->CommonCfg.bNonVhtDisallow))
		return MLME_ASSOC_REJ_DATA_RATE;

#endif /* DOT11_VHT_AC */

#ifdef RATE_PRIOR_SUPPORT
			if (pAd->LowRateCtrl.RatePrior) {
				if (WMODE_CAP_2G(wdev->PhyMode) && !HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
					return MLME_UNSPECIFY_FAIL;
#ifdef DOT11_VHT_AC
				if (WMODE_CAP_5G(wdev->PhyMode) && !HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
					return MLME_UNSPECIFY_FAIL;
#endif /* DOT11_VHT_AC */
				RTMP_SEM_LOCK(&pAd->LowRateCtrl.BlackListLock);
				DlListForEach(pBlackSta, &pAd->LowRateCtrl.BlackList, BLACK_STA, List) {
					if (NdisCmpMemory(pBlackSta->Addr, pEntry->Addr, MAC_ADDR_LEN) == 0) {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"Reject blk sta: "MACSTR"\n", MAC2STR(pBlackSta->Addr));
						RTMP_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
						return MLME_UNSPECIFY_FAIL;
					}
				}
				RTMP_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
			}
#endif /*RATE_PRIOR_SUPPORT*/


	if ((pEntry->Sst == SST_AUTH) || (pEntry->Sst == SST_ASSOC)) {
		/* TODO:
			should qualify other parameters, for example -
			capablity, supported rates, listen interval, etc., to
			decide the Status Code
		*/
		tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
		*pAid = pEntry->Aid;
		pEntry->NoDataIdleCount = 0;
		tr_ctl->tr_entry[pEntry->wcid].NoDataIdleCount = 0;
		pEntry->StaConnectTime = 0;
		pEntry->sleep_from = 0;
#ifdef CONFIG_HOTSPOT_R2

		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE))
#endif
		{
#ifdef WSC_AP_SUPPORT

			if (pEntry->bWscCapable == FALSE)
#endif /* WSC_AP_SUPPORT */
			{
				/* check the validity of the received RSNIE */
				StatusCode = WPAValidateRSNIE(&wdev->SecConfig,
							      &pEntry->SecConfig,
							      &ie_list->RSN_IE[0],
							      ie_list->RSNIE_Len);
#ifdef DOT11R_FT_SUPPORT
				if (!IS_FT_STA(pEntry)) /* IS_FT_RSN_STA should be use at 4-way only due to rnsie is assigned at assoc state */
#endif
				{
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef DOT11_SAE_SUPPORT
					INT cacheidx;

					if ((StatusCode == MLME_SUCCESS)
						&& IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap)
						&& (is_rsne_pmkid_cache_match(ie_list->RSN_IE,
									   ie_list->RSNIE_Len,
									   &pAd->ApCfg.PMKIDCache,
									   pEntry->func_tb_idx,
									   pEntry->Addr,
									   &cacheidx))
						&& (cacheidx == INVALID_PMKID_IDX))
						StatusCode = MLME_INVALID_PMKID;
					else if ((StatusCode == MLME_SUCCESS)
						&& IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap)
						&& !sae_get_pmk_cache(&pAd->SaeCfg, wdev->bssid, pEntry->Addr, NULL, NULL))
						StatusCode = MLME_UNSPECIFY_FAIL;

#endif /* DOT11_SAE_SUPPORT */
#ifdef CONFIG_OWE_SUPPORT
					if ((StatusCode == MLME_SUCCESS)
						&& IS_AKM_OWE(pEntry->SecConfig.AKMMap))
						StatusCode = owe_pmkid_ecdh_process(pAd,
										    pEntry,
										    ie_list->RSN_IE,
										    ie_list->RSNIE_Len,
										    &ie_list->ecdh_ie,
										    ie_list->ecdh_ie.length,
										    pPmkid,
										    &pmkid_count,
										    SUBTYPE_ASSOC_REQ);
#endif /*CONFIG_OWE_SUPPORT*/
#endif /*HOSTAPD_WPA3_SUPPORT*/
				}


#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef MAP_R3
				if ((IS_MAP_ENABLE(pAd) && !IS_MAP_CERT_ENABLE(pAd))
					|| !IS_MAP_ENABLE(pAd)) {
#endif
					if (StatusCode == MLME_SUCCESS)
						StatusCode = parse_rsnxe_ie(&pEntry->SecConfig,
							&ie_list->rsnxe_ie[0], ie_list->rsnxe_ie_len, TRUE);
#ifdef MAP_R3
				}
#endif
#endif /*HOSTAPD_WPA3_SUPPORT*/

				if (StatusCode != MLME_SUCCESS) {
					/* send wireless event - for RSN IE sanity check fail */
					RTMPSendWirelessEvent(pAd, IW_RSNIE_SANITY_FAIL_EVENT_FLAG, pEntry->Addr, 0, 0);
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid status code(%d) !!!\n", StatusCode);
					return StatusCode;
				}

				if (pEntry->SecConfig.AKMMap == 0x0) {
					SET_AKM_OPEN(pEntry->SecConfig.AKMMap);
					SET_CIPHER_NONE(pEntry->SecConfig.PairwiseCipher);
					SET_CIPHER_NONE(pEntry->SecConfig.GroupCipher);
				}

				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(AID#%d AKM=0x%x, PairwiseCipher=0x%x)\n",
						 pEntry->Aid, pEntry->SecConfig.AKMMap, pEntry->SecConfig.PairwiseCipher);
			}
		}

		NdisMoveMemory(pEntry->RSN_IE, &ie_list->RSN_IE[0], ie_list->RSNIE_Len);
		pEntry->RSNIE_Len = ie_list->RSNIE_Len;

		if (*pAid == 0 || *pAid == INVALID_AID)
			/* Here should be MAC table full */
			StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
		else if ((pEntry->RSNIE_Len == 0) &&
				 (IS_AKM_WPA_CAPABILITY_Entry(pEntry))
#ifdef RT_CFG80211_SUPPORT
				 && (wdev->Hostapd == Hostapd_CFG)
#endif /*RT_CFG80211_SUPPORT*/
				) {
#ifdef WSC_AP_SUPPORT
			wsc_ctrl = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl;

			if (((wsc_ctrl->WscConfMode != WSC_DISABLE) &&
				 pEntry->bWscCapable
#ifdef WSC_V2_SUPPORT
				 && (wsc_ctrl->WscV2Info.bWpsEnable ||
					 (wsc_ctrl->WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
				)
#ifdef RT_CFG80211_SUPPORT
				|| wdev->Hostapd == Hostapd_CFG
#endif /*RT_CFG80211_SUPPORT*/
			   ) {
				pEntry->Sst = SST_ASSOC;
				StatusCode = MLME_SUCCESS;

				/* In WPA or 802.1x mode, the port is not secured. */
				if (IS_AKM_WPA_CAPABILITY(pEntry->SecConfig.AKMMap)
#ifdef DOT1X_SUPPORT
					|| IS_IEEE8021X_Entry(wdev)
#endif /* DOT1X_SUPPORT */
				   )
					tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
				else
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

				if (IS_AKM_PSK(pEntry->SecConfig.AKMMap)) {
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
					pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
				}

#ifdef DOT1X_SUPPORT
				else if (IS_AKM_1X(pEntry->SecConfig.AKMMap) || IS_IEEE8021X_Entry(wdev)) {
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
					pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION;
				}

#endif /* DOT1X_SUPPORT */
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						  "ASSOC - WSC_STATE_MACHINE is OFF.<WscConfMode = %d, apidx =%d>\n",
						  wsc_ctrl->WscConfMode, pEntry->func_tb_idx);
				StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
			}

#else  /* WSC_AP_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
			/* CFG_TODO: due to WPS_AP flag */
			pEntry->Sst = SST_ASSOC;
			StatusCode = MLME_SUCCESS;
#else
			StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
#endif /* RT_CFG80211_P2P_SUPPORT */
#endif /* WSC_AP_SUPPORT */
		} else
		StatusCode = update_associated_mac_entry(pAd, pEntry, ie_list, MaxSupportedRate, isReassoc);
	}


	if (StatusCode == MLME_SUCCESS) {

#ifdef LINK_TEST_SUPPORT
		/* STA Link up Handler */
		LinkTestStaLinkUpHandler(pAd, pEntry);
#endif /* LINK_TEST_SUPPORT */
	}

	return StatusCode;
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
static BOOLEAN PeerDisassocReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pDA,
	OUT PUCHAR pAddr2,
	OUT	UINT16 *SeqNum,
	OUT USHORT *Reason)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pDA, &Fr->Hdr.Addr1);
	COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
	*SeqNum = Fr->Hdr.Sequence;
	NdisMoveMemory(Reason, &Fr->Octet[0], 2);
	return TRUE;
}

#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_WPA3_SUPPORT)
BOOLEAN PeerAssocReqCmmSanity
#else
static BOOLEAN PeerAssocReqCmmSanity
#endif
(
	RTMP_ADAPTER *pAd,
	BOOLEAN isReassoc,
	VOID *Msg,
	INT MsgLen,
	IE_LISTS *ie_lists)
{
	CHAR *Ptr;
	PFRAME_802_11	Fr = (PFRAME_802_11)Msg;
	PEID_STRUCT eid_ptr;
	UCHAR Sanity = 0;
	UCHAR WPA1_OUI[4] = { 0x00, 0x50, 0xF2, 0x01 };
#ifdef CONFIG_HOTSPOT_R2
	UCHAR HS2_OSEN_OUI[4] = { 0x50, 0x6f, 0x9a, 0x12 };
	UCHAR HS2OUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x10};
#endif
#ifdef CONFIG_HOTSPOT_R3
	UCHAR HS2_ROAMING_CONSORTIUM_SELECTION_OUI[4] = {0x50, 0x6f, 0x9a, 0x1D};
#endif /* CONFIG_HOTSPOT_R3*/
#ifdef CONFIG_MAP_SUPPORT
	unsigned char map_cap = 0;
#ifdef MAP_R2
	UCHAR map_profile;
	UINT16 map_vid;
#endif
#endif
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;
#ifdef P2P_SUPPORT
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	UCHAR	P2POUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x9};
#endif /* P2P_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_INFO *pFtInfo = &ie_lists->FtInfo;
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	RRM_EN_CAP_IE *pRrmEnCap = &ie_lists->RrmEnCap;
#endif /* DOT11K_RRM_SUPPORT */
#ifdef P2P_SUPPORT
	UCHAR *pP2pSubelement = &ie_lists->P2pSubelement[0];
#endif /* P2P_SUPPORT */
	HT_CAPABILITY_IE *pHtCapability = &ie_lists->cmm_ies.ht_cap;
	UCHAR i;
	struct legacy_rate *rate = &ie_lists->rate;
#ifdef RT_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
	UINT16 tmp;
	UCHAR *pextCapInfo = NULL;
#endif
	pEntry = MacTableLookup(pAd, &Fr->Hdr.Addr2[0]);

	if (pEntry == NULL)
		return FALSE;

#ifdef TXBF_SUPPORT
	/* Reset OUI when recieving a new Assoc Request */
	txbf_clear_oui();
#endif

	COPY_MAC_ADDR(&ie_lists->Addr1[0], &Fr->Hdr.Addr1[0]);
	COPY_MAC_ADDR(&ie_lists->Addr2[0], &Fr->Hdr.Addr2[0]);
	Ptr = (PCHAR)Fr->Octet;
	NdisMoveMemory(&ie_lists->CapabilityInfo, &Fr->Octet[0], 2);
	NdisMoveMemory(&ie_lists->ListenInterval, &Fr->Octet[2], 2);

	if (isReassoc) {
		NdisMoveMemory(&ie_lists->ApAddr[0], &Fr->Octet[4], 6);
		eid_ptr = (PEID_STRUCT) &Fr->Octet[10];
	} else
		eid_ptr = (PEID_STRUCT) &Fr->Octet[4];

	/* get variable fields from payload and advance the pointer */
	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)Fr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_SSID:
			if (((Sanity & 0x1) == 1))
				break;

			if (parse_ssid_ie(eid_ptr)) {
				Sanity |= 0x01;
				NdisMoveMemory(&ie_lists->Ssid[0], eid_ptr->Octet, eid_ptr->Len);
				ie_lists->SsidLen = eid_ptr->Len;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "PeerAssocReqSanity - SsidLen = %d\n", ie_lists->SsidLen);
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PeerAssocReqSanity - wrong IE_SSID\n");
				return FALSE;
			}

			break;

		case IE_SUPP_RATES:
			if (parse_support_rate_ie(rate, eid_ptr))
				Sanity |= 0x2;
			else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"wrong IE_SUPP_RATES (len=%d)\n",
						eid_ptr->Len);
				return FALSE;
			}
			break;

		case IE_EXT_SUPP_RATES:
			if (parse_support_ext_rate_ie(rate, eid_ptr) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"wrong IE_EXT_SUPP_RATES\n");
				return FALSE;
			}

			break;

		case IE_HT_CAP:
			if (parse_ht_cap_ie(eid_ptr->Len)) {
				NdisMoveMemory(pHtCapability, eid_ptr->Octet, SIZE_HT_CAP_IE);
				*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
				{
					EXT_HT_CAP_INFO extHtCapInfo;

					NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
					NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				}
#else
				*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
#ifdef RT_BIG_ENDIAN
				*(USHORT *)(&pHtCapability->TxBFCap) = le2cpu32(*(USHORT *)(&pHtCapability->TxBFCap));
#endif
				SET_HT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists);
				Sanity |= 0x10;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "PeerAssocReqSanity - IE_HT_CAP\n");
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PeerAssocReqSanity - wrong IE_HT_CAP.eid_ptr->Len = %d\n",
						 eid_ptr->Len);
				return FALSE;
			}

			break;

		case IE_EXT_CAPABILITY:
#ifdef OOB_CHK_SUPPORT
			(VOID) parse_ext_cap_ie(&ie_lists->ExtCapInfo, eid_ptr);
#else /* OOB_CHK_SUPPORT */
			if (eid_ptr->Len) {
				INT ext_len = eid_ptr->Len;
				ext_len = ext_len > sizeof(EXT_CAP_INFO_ELEMENT) ? sizeof(EXT_CAP_INFO_ELEMENT) : ext_len;
				NdisMoveMemory(&ie_lists->ExtCapInfo, eid_ptr->Octet, ext_len);
#ifdef RT_BIG_ENDIAN
				pextCapInfo = (UCHAR *)&ie_lists->ExtCapInfo;
				*((UINT32 *)pextCapInfo) = cpu2le32(*((UINT32 *)pextCapInfo));
				*((UINT32 *)(pextCapInfo + 4)) = cpu2le32(*((UINT32 *)(pextCapInfo + 4)));
#endif
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "PeerAssocReqSanity - IE_EXT_CAPABILITY!\n");
			}
#endif /* !OOB_CHK_SUPPORT */
			break;

		case IE_WPA:    /* same as IE_VENDOR_SPECIFIC */
#ifdef QOS_R2
			if (NdisEqualMemory(eid_ptr->Octet, wfa_oui, 3) &&
				(eid_ptr->Octet[3] == WFA_CAPA_IE_OUI_TYPE) && (eid_ptr->Len >= 6)) {
				ie_lists->DSCPPolicyEnable = eid_ptr->Octet[5] & 0x01;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"DSCPPolicyEnable = %d\n", ie_lists->DSCPPolicyEnable);
			}
#endif

#if defined(RT_CFG80211_SUPPORT) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
			if ((NdisEqualMemory(eid_ptr->Octet, apple_oui, 3) &&
				(eid_ptr->Octet[3] == APPLE_IE_OUI_TYPE))) {
					pEntry->isIphone = TRUE;
					break;
				}
#endif


#ifdef MBO_SUPPORT
			if (IS_MBO_ENABLE(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev) &&
					(eid_ptr->Octet[0] == MBO_OCE_OUI_0) &&
					(eid_ptr->Octet[1] == MBO_OCE_OUI_1) &&
					(eid_ptr->Octet[2] == MBO_OCE_OUI_2) &&
					(eid_ptr->Octet[3] == MBO_OCE_OUI_TYPE) &&
					(eid_ptr->Len >= 5)
			   ) {
				MboParseStaMboIE(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev, pEntry, eid_ptr->Octet, eid_ptr->Len, MBO_FRAME_TYPE_ASSOC_REQ);
				break;
			}
#ifdef OCE_SUPPORT
			if (IS_OCE_ENABLE(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev) && (eid_ptr->Octet[0] == MBO_OCE_OUI_0) &&
					(eid_ptr->Octet[1] == MBO_OCE_OUI_1) &&
					(eid_ptr->Octet[2] == MBO_OCE_OUI_2) &&
					(eid_ptr->Octet[3] == MBO_OCE_OUI_TYPE) &&
					(eid_ptr->Len >= 5)
			   ) {
				OceParseStaAssoc(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev, pEntry, eid_ptr->Octet, eid_ptr->Len, OCE_FRAME_TYPE_ASSOC_REQ);
				break;
			}
#endif /* OCE_SUPPORT */
#endif /* MBO_SUPPORT */

#ifdef IGMP_TVM_SUPPORT
			if (IS_IGMP_TVM_MODE_EN(pEntry->wdev->IsTVModeEnable)) {
				if (NdisEqualMemory(eid_ptr->Octet, IGMP_TVM_OUI, 4) &&
					(eid_ptr->Len == IGMP_TVM_IE_LENGTH)) {
					RTMPMoveMemory(&ie_lists->tvm_ie, &eid_ptr->Eid, IGMP_TVM_IE_LENGTH+2);
					break;
				}
			}
#endif /* IGMP_TVM_SUPPORT */


#ifdef CONFIG_MAP_SUPPORT
			if (map_check_cap_ie(eid_ptr, &map_cap
#ifdef MAP_R2
				, &map_profile, &map_vid
#endif
				) == TRUE) {
				ie_lists->MAP_AttriValue = map_cap;
#ifdef MAP_R2
				ie_lists->MAP_ProfileValue = map_profile;
#endif
			}
#endif /* CONFIG_MAP_SUPPORT */
#ifdef MWDS

			if (NdisEqualMemory(MTK_OUI, eid_ptr->Octet, 3)) {
				if (MWDS_SUPPORT(eid_ptr->Octet[3])) {
					pEntry->bSupportMWDS = TRUE;
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Peer supports MWDS\n");
				} else
					pEntry->bSupportMWDS = FALSE;
			}

#endif /* MWDS */
#ifdef WH_EVENT_NOTIFIER

			if (pAd->ApCfg.EventNotifyCfg.CustomOUILen &&
				(eid_ptr->Len >= pAd->ApCfg.EventNotifyCfg.CustomOUILen) &&
				NdisEqualMemory(eid_ptr->Octet, pAd->ApCfg.EventNotifyCfg.CustomOUI, pAd->ApCfg.EventNotifyCfg.CustomOUILen)) {
				pEntry->custom_ie_len = eid_ptr->Len;
				NdisMoveMemory(pEntry->custom_ie, eid_ptr->Octet, eid_ptr->Len);
				break;
			}

#endif /* WH_EVENT_NOTIFIER */
#ifdef TXBF_SUPPORT
			/*
			 * For DWA-192 BFee disable workaround. Use "no OUI" to recognize DWA-192.
			 * Most vendors have its own OUI other than 00:50:f2 (Microsoft): BCM, QCM, MTK.
			 * Have no OUI but Microsoft 00:50:f2: RTK, Intel.
			 */
			if (!NdisEqualMemory(WPA_OUI, eid_ptr->Octet, 3)) { /* Find OUI, which is not 00:50:f2 */
				pEntry->has_oui++;
			}
			MTWF_DBG(NULL, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "[%d] has_oui: %d\n", pEntry->wcid, pEntry->has_oui);

			/* Record Broadcom OUI in Assoc Request for BF IOT */
			MTWF_DBG(NULL, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): eid_ptr->Octet[0-2] = %x, %x, %x\n"
																, __func__, eid_ptr->Octet[0], eid_ptr->Octet[1], eid_ptr->Octet[2]);
			if (NdisEqualMemory(BROADCOM_OUI, eid_ptr->Octet, 3)) {
				txbf_set_oui(ENUM_BF_OUI_BROADCOM);
			}
#endif /* TXBF_SUPPORT */
			/* fall through */
		case IE_WPA2:
#ifdef DOT11R_FT_SUPPORT
#endif /* DOT11R_FT_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2

			if (NdisEqualMemory(eid_ptr->Octet, HS2OUIBYTE, sizeof(HS2OUIBYTE)) && (eid_ptr->Len >= 5)) {
				/* UCHAR tmp2 = 0x12; */
				UCHAR *hs2_config = (UCHAR *)&eid_ptr->Octet[4];
				UCHAR ppomo_exist = ((*hs2_config) >> 1) & 0x01;
				UCHAR hs2_version = ((*hs2_config) >> 4) & 0x0f;
				/* UCHAR *tmp3 = (UCHAR *)&pEntry->hs_info.ppsmo_id; */
				/* UCHAR tmp[2] = {0x12,0x34}; */
				pEntry->hs_info.version = hs2_version;
				pEntry->hs_info.ppsmo_exist = ppomo_exist;

				if (pEntry->hs_info.ppsmo_exist) {
					NdisMoveMemory(&pEntry->hs_info.ppsmo_id, &eid_ptr->Octet[5], 2);
					/* NdisMoveMemory(tmp3, tmp, 2); */
				}

				break;
			}

#endif /* CONFIG_HOTSPOT_R2 */
#ifdef CONFIG_HOTSPOT_R3
			if (NdisEqualMemory(eid_ptr->Octet, HS2_ROAMING_CONSORTIUM_SELECTION_OUI, sizeof(HS2_ROAMING_CONSORTIUM_SELECTION_OUI))) {
				NdisZeroMemory(&pEntry->hs_consortium_oi, sizeof(STA_HS_CONSORTIUM_OI));
				pEntry->hs_consortium_oi.sta_wcid = pEntry->wcid;
				pEntry->hs_consortium_oi.oi_len = eid_ptr->Len - sizeof(HS2_ROAMING_CONSORTIUM_SELECTION_OUI);

				if (pEntry->hs_consortium_oi.oi_len == 3 || pEntry->hs_consortium_oi.oi_len == 5) {
					NdisMoveMemory(&pEntry->hs_consortium_oi.selected_roaming_consortium_oi[0], &eid_ptr->Octet[4], pEntry->hs_consortium_oi.oi_len);
				}

				break;
			}
#endif /* CONFIG_HOTSPOT_R3 */
#ifdef P2P_SUPPORT

			if (NdisEqualMemory(eid_ptr->Octet, P2POUIBYTE, sizeof(P2POUIBYTE)) && (eid_ptr->Len >= 4)) {
				if (ie_lists->P2PSubelementLen == 0) {
					RTMPMoveMemory(pP2pSubelement, &eid_ptr->Eid, (eid_ptr->Len + 2));
					ie_lists->P2PSubelementLen = (eid_ptr->Len + 2);
				} else if (ie_lists->P2PSubelementLen > 0) {
					if (((ie_lists->P2PSubelementLen) + (eid_ptr->Len + 2)) <= MAX_VIE_LEN) {
						RTMPMoveMemory(pP2pSubelement + ie_lists->P2PSubelementLen, &eid_ptr->Eid, (eid_ptr->Len + 2));
						ie_lists->P2PSubelementLen += (eid_ptr->Len + 2);
					} else {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR!! 111 Sum of P2PSubelementLen= %lu, > MAX_VIE_LEN !!\n",
								 (ie_lists->P2PSubelementLen + (eid_ptr->Len + 2)));
						return FALSE;
					}
				}

				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, " ! ===>P2P - PeerAssocReqSanity  P2P IE Len becomes = %d.   %s\n",
						 ie_lists->P2PSubelementLen, decodeP2PState(pP2PCtrl->P2PConnectState));
				break;
			}

#endif /* P2P_SUPPORT */

			if (NdisEqualMemory(eid_ptr->Octet, WPS_OUI, 4)) {
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

				if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscV2Info.bWpsEnable) ||
					(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
					ie_lists->bWscCapable = TRUE;

#ifdef RT_CFG80211_SUPPORT
				ie_lists->bWscCapable = TRUE;
#endif
#endif /* WSC_AP_SUPPORT */
				break;
			}

			/* Handle Atheros and Broadcom draft 11n STAs */
			if (NdisEqualMemory(eid_ptr->Octet, BROADCOM_OUI, 3)) {
				switch (eid_ptr->Octet[3]) {
				case 0x33:
					if ((eid_ptr->Len - 4) == sizeof(HT_CAPABILITY_IE)) {
						NdisMoveMemory(pHtCapability, &eid_ptr->Octet[4], SIZE_HT_CAP_IE);
						*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
						{
							EXT_HT_CAP_INFO extHtCapInfo;

							NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
							*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
							NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
						}
#else
						*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
						SET_HT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists);
					}

					break;

				default:
					/* ignore other cases */
					break;
				}
			}

			check_vendor_ie(pAd, (UCHAR *)eid_ptr, &(ie_lists->cmm_ies.vendor_ie));

#ifdef VHT_TXBF_2G_EPIGRAM_IE
			if (ie_lists->vendor_ie.is_brcm_etxbf_2G
					&& HAS_HT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists)) {
				if (WMODE_CAP_2G(wdev->PhyMode))
					pEntry->rStaBfRecVendorUpdate.fgIsBrcm2GeTxBFIe = TRUE;
				pEntry->rStaBfRecVendorUpdate.Nrow =
					get_max_nss_by_htcap_ie_mcs(
						&(ie_lists->cmm_ies.ht_cap.MCSSet[0]));
			}
#endif /* VHT_TXBF_2G_EPIGRAM_IE */

			/* WMM_IE */
			if (NdisEqualMemory(eid_ptr->Octet, WME_INFO_ELEM, 6) && (eid_ptr->Len == 7)) {
				ie_lists->bWmmCapable = TRUE;
#ifdef UAPSD_SUPPORT

				if (pEntry) {
					UAPSD_AssocParse(pAd,
									 pEntry, (UINT8 *)&eid_ptr->Octet[6],
									 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.UapsdInfo.bAPSDCapable);
				}

#endif /* UAPSD_SUPPORT */
				break;
			}

			if (IS_NO_SECURITY(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig)
				|| IS_CIPHER_WEP(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PairwiseCipher))
				break;

			/*	If this IE did not begins with 00:0x50:0xf2:0x01,
				it would be proprietary. So we ignore it. */
			if (!NdisEqualMemory(eid_ptr->Octet, WPA1_OUI, sizeof(WPA1_OUI))
				&& !(eid_ptr->Eid == IE_WPA2)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Not RSN IE, maybe WMM IE!!!\n");
#ifdef CONFIG_HOTSPOT_R2

				if (!NdisEqualMemory(eid_ptr->Octet, HS2_OSEN_OUI, sizeof(HS2_OSEN_OUI))) {
					unsigned char *tmp = (unsigned char *)eid_ptr->Octet;

					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!!!!!not found OSEN IE,%x:%x:%x:%x\n", *tmp, *(tmp + 1),
							 *(tmp + 2), *(tmp + 3));
					pEntry->OSEN_IE_Len = 0;
					CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE);
					break;
				}

				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE);
				NdisMoveMemory(pEntry->OSEN_IE, eid_ptr, eid_ptr->Len + 2);
				pEntry->OSEN_IE_Len = eid_ptr->Len + 2;
				SET_AKM_WPA2(pEntry->SecConfig.AKMMap);
				SET_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher);
				SET_CIPHER_CCMP128(pEntry->SecConfig.GroupCipher);
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!!!!!found OSEN IE Entry AuthMode %s, EncryptType %s\n",
						 GetAuthModeStr(pEntry->SecConfig.AKMMap), GetEncryModeStr(pEntry->SecConfig.PairwiseCipher));
#else
				break;
#endif
			}

			/* Copy whole RSNIE context */
			NdisMoveMemory(&ie_lists->RSN_IE[0], eid_ptr, eid_ptr->Len + 2);
			ie_lists->RSNIE_Len = eid_ptr->Len + 2;
#ifdef DOT11R_FT_SUPPORT
			NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
			pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
#endif /* DOT11R_FT_SUPPORT */

			break;
#ifdef DOT11R_FT_SUPPORT

		case IE_FT_MDIE:
			if (FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
						"wrong IE_FT_MDIE\n");
				return FALSE;
			}
			break;

		case IE_FT_FTIE:
			if (FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
						"wrong IE_FT_FTIE\n");
				return FALSE;
			}
			break;

		case IE_FT_RIC_DATA:

			/* record the pointer of first RDIE. */
			if (pFtInfo->RicInfo.pRicInfo == NULL) {
				pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
				pFtInfo->RicInfo.Len = ((UCHAR *)Fr + MsgLen) - (UCHAR *)eid_ptr + 1;
			}
			break;

		case IE_FT_RIC_DESCRIPTOR:
			if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
				NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
							   &eid_ptr->Eid, eid_ptr->Len + 2);
				pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
			} else {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
						"wrong IE_FT_RIC_DESCRIPTOR\n");
				return FALSE;
			}

			break;
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT

		case IE_RRM_EN_CAP: {
			UINT64 value;
			if (parse_rm_enable_cap_ie(eid_ptr)) {
				NdisMoveMemory(&value, eid_ptr->Octet, eid_ptr->Len);
				pRrmEnCap->word = le2cpu64(value);
			} else {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
						"wrong IE_RRM_EN_CAP\n");
				return FALSE;
			}
		}
		break;
#endif /* DOT11K_RRM_SUPPORT */
#ifdef DOT11_VHT_AC

		case IE_VHT_CAP:
			if (parse_vht_cap_ie(eid_ptr->Len)) {
				NdisMoveMemory(&ie_lists->cmm_ies.vht_cap, eid_ptr->Octet, sizeof(VHT_CAP_IE));
				SET_VHT_CAPS_EXIST(ie_lists->cmm_ies.ie_exists);
#ifdef RT_BIG_ENDIAN
				NdisCopyMemory(&tmp_1, &ie_lists->cmm_ies.vht_cap.vht_cap, 4);
				tmp_1 = le2cpu32(tmp_1);
				NdisCopyMemory(&ie_lists->cmm_ies.vht_cap.vht_cap, &tmp_1, 4);

				NdisCopyMemory(&tmp_2, &(ie_lists->cmm_ies.vht_cap.mcs_set), 8);
				tmp_2 = le2cpu64(tmp_2);
				NdisCopyMemory(&(ie_lists->cmm_ies.vht_cap.mcs_set), &tmp_2, 8);
#endif
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s():IE_VHT_CAP\n", __func__);
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "wrong IE_VHT_CAP, eid->Len = %d\n",
						 eid_ptr->Len);
				return FALSE;
			}
			break;

		case IE_VHT_OP:
			if (parse_vht_op_ie(eid_ptr->Len)) {
				NdisMoveMemory(&ie_lists->cmm_ies.vht_op, eid_ptr->Octet, sizeof(VHT_OP_IE));
				SET_VHT_OP_EXIST(ie_lists->cmm_ies.ie_exists);
#ifdef RT_BIG_ENDIAN
				NdisCopyMemory(&tmp, &ie_lists->cmm_ies.vht_op.basic_mcs_set, sizeof(VHT_MCS_MAP));
				tmp = le2cpu16(tmp);
				NdisCopyMemory(&ie_lists->cmm_ies.vht_op.basic_mcs_set, &tmp, sizeof(VHT_MCS_MAP));
#endif
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IE_VHT_OP\n");
			} else
				return FALSE;
			break;

		case IE_OPERATING_MODE_NOTIFY:
			if (parse_operating_mode_notification_ie(eid_ptr->Len)) {
				ie_lists->cmm_ies.operating_mode_len = sizeof(OPERATING_MODE);
				NdisMoveMemory(&ie_lists->cmm_ies.operating_mode,
						&eid_ptr->Octet[0], sizeof(OPERATING_MODE));
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"IE_OPERATING_MODE_NOTIFY!\n");
			} else
				return FALSE;

			break;
#endif /* DOT11_VHT_AC */

		case IE_SUPP_CHANNELS:
			if (eid_ptr->Len > MAX_LEN_OF_SUPPORTED_CHL || (eid_ptr->Len % 2)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wrong IE_SUPP_CHANNELS, eid->Len = %d\n",
						 eid_ptr->Len);
			} else if (eid_ptr->Len + ie_lists->SupportedChlLen <= MAX_LEN_OF_SUPPORTED_CHL) {
				UINT32 _ChlIdx = ie_lists->SupportedChlLen %
								 MAX_LEN_OF_SUPPORTED_CHL;
				NdisMoveMemory(&ie_lists->SupportedChl[_ChlIdx], eid_ptr->Octet,
							   eid_ptr->Len);
				ie_lists->SupportedChlLen += eid_ptr->Len;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "IE_SUPP_CHANNELS, eid->Len = %d\n",
						 eid_ptr->Len);
			} else  if (ie_lists->SupportedChlLen <  MAX_LEN_OF_SUPPORTED_CHL)  {
				NdisMoveMemory(&ie_lists->SupportedChl[ie_lists->SupportedChlLen], eid_ptr->Octet,
							   MAX_LEN_OF_SUPPORTED_CHL - (ie_lists->SupportedChlLen));
				ie_lists->SupportedChlLen = MAX_LEN_OF_SUPPORTED_CHL;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "IE_SUPP_CHANNELS, eid->Len = %d (Exceeded)\n",
						 eid_ptr->Len);
			}

			if (ie_lists->SupportedChlLen > MAX_LEN_OF_SUPPORTED_CHL)
				ie_lists->SupportedChlLen = MAX_LEN_OF_SUPPORTED_CHL;

			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "  Supported Channels: [ FirstCh : NumOfCh ]\n");

			for (i = 0; i < ie_lists->SupportedChlLen; i += 2)
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "  [ %4d : %4d ]\n", ie_lists->SupportedChl[i],
						 ie_lists->SupportedChl[i + 1]);

			break;
#ifdef DOT11_HE_AX
		case IE_RSNXE:
			/* Copy whole RSNXEIE context */
			NdisMoveMemory(&ie_lists->rsnxe_ie[0], eid_ptr, eid_ptr->Len + 2);
			ie_lists->rsnxe_ie_len = eid_ptr->Len + 2;
			break;
#endif /* DOT11_HE_AX */
		case IE_SUPP_REG_CLASS:
			ie_lists->current_opclass = eid_ptr->Octet[0];
			break;
		case IE_WLAN_EXTENSION:
		{
			/*parse EXTENSION EID*/
			UCHAR *extension_id = (UCHAR *)eid_ptr + 2;
			switch (*extension_id) {
			case IE_EXTENSION_ID_ECDH:
#if defined(CONFIG_OWE_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)
			{
				UCHAR *ext_ie_length = (UCHAR *)eid_ptr + 1;

				os_zero_mem(ie_lists->ecdh_ie.public_key, *ext_ie_length-3);
				ie_lists->ecdh_ie.ext_ie_id = IE_WLAN_EXTENSION;
				ie_lists->ecdh_ie.length = eid_ptr->Len;
				NdisMoveMemory(&ie_lists->ecdh_ie.ext_id_ecdh, eid_ptr->Octet, eid_ptr->Len);
			}
#endif /*CONFIG_OWE_SUPPORT*/
				break;

#ifdef QOS_R1
			case IE_EXTENSION_ID_MSCS_DESC:
				ie_lists->has_mscs_req = QoS_parse_mscs_descriptor_ies(pEntry->wdev,
								&ie_lists->Addr2[0], eid_ptr->Octet, eid_ptr->Len);
				break;
#endif
#ifdef DOT11_HE_AX
			case IE_EXTENSION_ID_HE_CAP:
			/* fall through */
			case IE_EXTENSION_ID_HE_OP:
			/* fall through */
			case IE_EXTENSION_ID_HE_6G_CAP:
				if (parse_he_assoc_req_ies((UINT8 *)eid_ptr, ie_lists) <= 0) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"IE_WLAN_EXTENSION: parsing HE related ies(%d) unexpected error\n", *extension_id);
				}
				break;
#endif
			default:
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"IE_WLAN_EXTENSION: no handler for extension_id:%d\n", *extension_id);
				break;
			}
		}
			break;

		case IE_ADD_HT:
			if (parse_ht_info_ie(eid_ptr)) {
				NdisMoveMemory(&ie_lists->cmm_ies.ht_op, eid_ptr->Octet, eid_ptr->Len);
				SET_HT_OP_EXIST(ie_lists->cmm_ies.ie_exists);
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"wrong IE_ADD_HT\n");
				return FALSE;
			}
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}


	if ((Sanity & 0x3) != 0x03) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): - missing mandatory field\n", __func__));
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s() - success\n", __func__);
	return TRUE;

}

static BOOLEAN rate_is_11b_only(struct legacy_rate *rate)
{
	BOOLEAN only_11b = TRUE;
	UCHAR idx;

	for (idx = 0; idx < rate->sup_rate_len; idx++) {
		if (((rate->sup_rate[idx] & 0x7F) != 2) &&
			((rate->sup_rate[idx] & 0x7F) != 4) &&
			((rate->sup_rate[idx] & 0x7F) != 11) &&
			((rate->sup_rate[idx] & 0x7F) != 22)) {
			only_11b = FALSE;
			break;
		}
	}

	return only_11b;
}

#ifdef CONFIG_MAP_SUPPORT
static BOOLEAN is_controller_found(struct wifi_dev *wdev)
{
	struct map_vendor_ie *ie = (struct map_vendor_ie *)wdev->MAPCfg.vendor_ie_buf;

	if (ie->connectivity_to_controller)
		return TRUE;

	return FALSE;
}
#endif
static VOID ap_cmm_peer_assoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN BOOLEAN isReassoc)
{
	struct wifi_dev *wdev = NULL;
	struct legacy_rate *rate;
	BSS_STRUCT *pMbss;
	BOOLEAN bAssocSkip = FALSE;
	CHAR rssi;
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocRspHdr;
	USHORT CapabilityInfoForAssocResp;
	USHORT StatusCode = MLME_SUCCESS;
	USHORT Aid = 0, PhyMode;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	UCHAR MaxSupportedRate = 0;
	UCHAR SupRateLen;
	UCHAR sum_rate[MAX_LEN_OF_SUPPORTED_RATES], sum_rate_len, *p_sum_rate;
	BOOLEAN FlgIs11bSta;
	UCHAR check_rsnxe_install = TRUE;
	UCHAR WdevBandIdx;
	UCHAR ChBandIdx;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
#ifdef DBG
	UCHAR *sAssoc = isReassoc ? (PUCHAR)"ReASSOC" : (PUCHAR)"ASSOC";
#endif /* DBG */
	UCHAR SubType;
	BOOLEAN bACLReject = FALSE;
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
	BOOLEAN bBlReject = FALSE;
#endif
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg = NULL;
	PFT_INFO FtInfoBuf = NULL;
#endif /* DOT11R_FT_SUPPORT */
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
	ADD_HT_INFO_IE *addht;
	struct _build_ie_info ie_info = {0};
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	struct customer_vendor_ie *ap_vendor_ie;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#ifdef WAPP_SUPPORT
	UINT8 wapp_cnnct_stage = WAPP_ASSOC;
	UINT16 wapp_assoc_fail = NOT_FAILURE;
#endif /* WAPP_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	struct _ASIC_SEC_INFO *info = NULL;
	PUCHAR rsnxe_ie = NULL;
#endif /* DOT11R_FT_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
		PHOTSPOT_CTRL pHSCtrl = NULL;
#endif
#ifdef MBO_SUPPORT
	UCHAR APIndex;
#endif /* MBO_SUPPORT */
#ifdef WARP_512_SUPPORT
	MAC_TABLE_ENTRY *pA4Entry = NULL;
	BOOLEAN mwds_enable = FALSE;
#endif /* WARP_512_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	os_alloc_mem_suspend(NULL, (UCHAR **)&FtInfoBuf, sizeof(FT_INFO));

	if (FtInfoBuf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "mem alloc failed\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
		goto assoc_check;
	}

	NdisZeroMemory(FtInfoBuf, sizeof(FT_INFO));


/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&rsnxe_ie, MAX_LEN_OF_RSNXEIE);
	NdisZeroMemory(rsnxe_ie, MAX_LEN_OF_RSNXEIE);

	if (rsnxe_ie == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "mem alloc failed\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
		goto assoc_check;
	}
#endif


	/* disallow new association */
	if (pAd->ApCfg.BANClass3Data == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Disallow new Association\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = DISALLOW_NEW_ASSOCI;
#endif /* WAPP_SUPPORT */
		goto assoc_check;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "mem alloc failed\n");
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
		goto assoc_check;
	}

	NdisZeroMemory(ie_list, sizeof(IE_LISTS));

	if (!PeerAssocReqCmmSanity(pAd, isReassoc, Elem->Msg, Elem->MsgLen, ie_list))
		goto LabelOK;

	pEntry = MacTableLookup(pAd, ie_list->Addr2);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "NoAuth MAC - "MACSTR"\n",
				  MAC2STR(ie_list->Addr2));
		goto LabelOK;
	} else
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				 " Recv Assoc from STA - "MACSTR"\n",
				  MAC2STR(ie_list->Addr2));

	if (!VALID_MBSS(pAd, pEntry->func_tb_idx)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "pEntry bounding invalid wdev(apidx=%d)\n",
				  pEntry->func_tb_idx);
		goto LabelOK;
	}
#ifdef WARP_512_SUPPORT
#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT)
	if (pAd->Warp512Support &&
		((pEntry->wcid < A4_APCLI_FIRST_WCID) ||
		(pEntry->wcid >= A4_APCLI_FIRST_WCID + MAX_RESERVE_ENTRY))) {
#ifdef MWDS
		if (pEntry->bSupportMWDS && pEntry->wdev && pEntry->wdev->bSupportMWDS)
			mwds_enable = TRUE;
#endif
#ifdef CONFIG_MAP_SUPPORT
			pEntry->DevPeerRole = ie_list->MAP_AttriValue;
			if (IS_MAP_ENABLE(pAd) &&
			(pEntry->wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
			(pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA))) {
				pA4Entry = ReassignWcidForA4Entry(pAd, pEntry, ie_list);
				if (!pA4Entry)
					goto assoc_check;

				pEntry = pA4Entry;
				mwds_enable = FALSE;
			}
#endif /* CONFIG_MAP_SUPPORT */
#ifdef MWDS
		if (mwds_enable) {
			pA4Entry = ReassignWcidForA4Entry(pAd, pEntry, ie_list);
			if (!pA4Entry)
				goto assoc_check;

			pEntry = pA4Entry;
		}
#endif
	}
#endif /* defined(MWDS) || defined(CONFIG_MAP_SUPPORT) */
#endif /* WARP_512_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 " pEntry->func_tb_idx=%d\n", pEntry->func_tb_idx);
	wdev = wdev_search_by_address(pAd, ie_list->Addr1);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Wrong Addr1 - "MACSTR"\n",
				  MAC2STR(ie_list->Addr1));
		goto LabelOK;
	}

	/* If pkt wdev bandix does not match to pEntry bandidex, remove entry. */
	WdevBandIdx = HcGetBandByWdev(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev);
	ChBandIdx = HcGetBandByWdev(wdev);
	if ((WdevBandIdx != ChBandIdx) || !RTMPEqualMemory(ie_list->Addr1,
		pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN)) {
		MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wcid%d exist in Band %d address-"MACSTR"  but Recv associte Band%d, CH%d address-"MACSTR"=> Del entry, please Send Auth first\n",
				pEntry->wcid, WdevBandIdx, MAC2STR(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid),
				ChBandIdx, Elem->Channel, MAC2STR(ie_list->Addr1));
		pEntry = NULL;
		goto LabelOK;
	}

	/* Correct the pEntry member, when STA is ReAsso to AP  */
	if (wdev->func_idx != pEntry->func_tb_idx) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "@@ ERROR 1! - MAC="MACSTR", wdev->func_idx=%d, pEntry->func_tb_idx=%d, pEntry->wcid=%d\n",
				  MAC2STR(pEntry->Addr), wdev->func_idx, pEntry->func_tb_idx, pEntry->wcid);
		pEntry->func_tb_idx = wdev->func_idx;
#ifdef MBO_SUPPORT
		if (0 == pEntry->is_mbo_bndstr_sta)
			pEntry->is_mbo_bndstr_sta = 1;
#endif /* MBO_SUPPORT */


	}

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
#ifdef WSC_AP_SUPPORT
	wsc_ctrl = &wdev->WscControl;
#endif /* WSC_AP_SUPPORT */
	PhyMode = wdev->PhyMode;
	rate = &wdev->rate.legacy_rate;
	/* prevent setup basic rate with self capability that exceed peer's capability */
	if ((ie_list->rate.sup_rate_len + ie_list->rate.ext_rate_len) < (rate->sup_rate_len + rate->ext_rate_len)) {
		rate = &ie_list->rate;
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Support rate follow STA's settings\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Support rate follow AP's settings\n");
	}
	addht = wlan_operate_get_addht(wdev);

	if (!OPSTATUS_TEST_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "AP is not ready, disallow new Association\n");
		goto LabelOK;
	}

	if (pAd->FragFrame.wcid == pEntry->wcid) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, "\nFrage Attack:Clear Wcid = %d FragBuffer !!!!!\n", pEntry->wcid);
		RESET_FRAGFRAME(pAd->FragFrame);
	}

#ifdef OCE_FILS_SUPPORT
	if ((pEntry->filsInfo.is_post_assoc == TRUE) &&
		(pEntry->filsInfo.auth_algo == AUTH_MODE_FILS) &&
		IS_AKM_FILS(wdev->SecConfig.AKMMap) &&
		IS_AKM_FILS(pEntry->SecConfig.AKMMap)) {

		StatusCode = pEntry->filsInfo.status;
		pEntry->filsInfo.is_post_assoc = FALSE;
		goto assoc_post;
	}
#endif /* OCE_FILS_SUPPORT */

	ie_info.frame_subtype = SUBTYPE_ASSOC_RSP;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev;

#ifdef HTC_DECRYPT_IOT

	if ((pEntry->HTC_ICVErrCnt)
		|| (pEntry->HTC_AAD_OM_Force)
		|| (pEntry->HTC_AAD_OM_CountDown)
		|| (pEntry->HTC_AAD_OM_Freeze)
	   ) {
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "@@@ (wcid=%u), HTC_ICVErrCnt(%u), HTC_AAD_OM_Freeze(%u), HTC_AAD_OM_CountDown(%u),  HTC_AAD_OM_Freeze(%u) is in Asso. stage!\n",
				  pEntry->wcid, pEntry->HTC_ICVErrCnt, pEntry->HTC_AAD_OM_Force, pEntry->HTC_AAD_OM_CountDown,
				  pEntry->HTC_AAD_OM_Freeze);
		/* Force clean. */
		pEntry->HTC_ICVErrCnt = 0;
		pEntry->HTC_AAD_OM_Force = 0;
		pEntry->HTC_AAD_OM_CountDown = 0;
		pEntry->HTC_AAD_OM_Freeze = 0;
	}

#endif /* HTC_DECRYPT_IOT */
	FlgIs11bSta = rate_is_11b_only(rate);

#ifdef CONFIG_MAP_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			" IS_MAP_ENABLE(pAd)=%d IS_MAP_BS_ENABLE(pAd)=%d\n", IS_MAP_ENABLE(pAd), IS_MAP_BS_ENABLE(pAd));
	if ((IS_MAP_ENABLE(pAd)) || (IS_MAP_BS_ENABLE(pAd))) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				" Assoc Req len=%ld, ASSOC_REQ_LEN = %d\n",
				 Elem->MsgLen, ASSOC_REQ_LEN);
		if (Elem->MsgLen > ASSOC_REQ_LEN)
			pEntry->assoc_req_len = ASSOC_REQ_LEN;
		else
			pEntry->assoc_req_len = Elem->MsgLen;
		NdisMoveMemory(pEntry->assoc_req_frame, Elem->Msg, pEntry->assoc_req_len);
	}

#endif

#ifdef GN_MIXMODE_SUPPORT
	pEntry->FlgIs11bSta = FlgIs11bSta;
#endif /*GN_MIXMODE_SUPPORT*/

#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev)) {
		/* Set the Assoc disallow reason to 2 if Max sta num reached */
		if (pAd->ApCfg.EntryClientCount > HcGetMaxStaNum(pAd)) {
			wdev->MboCtrl.AssocDisallowReason = MBO_AP_DISALLOW_MAX_STA_NUM_REACHED;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					" MBO Max Sta num %d reached\n", pAd->ApCfg.EntryClientCount);

			if (isReassoc) {
				for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
					if (MAC_ADDR_EQUAL(ie_list->ApAddr, pAd->ApCfg.MBSSID[APIndex].wdev.bssid)) {
						pEntry->is_mbo_bndstr_sta = 1;
						break;
					}
				}
			}
		}

		/* Disallow assoc with RC = 17 if client entry is greater than MaxStaNum*/
		if (!MBO_AP_ALLOW_ASSOC(wdev) && 1 != pEntry->is_mbo_bndstr_sta) {
			StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
#ifdef WAPP_SUPPORT
			wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */
			goto SendAssocResponse;
		}
	}
#endif /* MBO_SUPPORT */

	/* YF@20120419: Refuse the weak signal of AssocReq */
	rssi = RTMPMaxRssi(pAd,
					   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
					   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
					   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "ra[%d] ASSOC_REQ Threshold = %d, PktMaxRssi=%d\n",
			  pEntry->func_tb_idx, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold,
			  rssi);

	if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold != 0) &&
		(rssi < pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Reject this ASSOC_REQ due to Weak Signal.\n");
#ifdef OCE_SUPPORT
		if (IS_OCE_ENABLE(wdev)) {
			struct oce_info *oceInfo = &pEntry->oceInfo;
			CHAR rssiThres = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold;

			oceInfo->DeltaAssocRSSI = (rssiThres > rssi) ?
				(rssiThres - rssi) : 0;
			StatusCode = MLME_DISASSOC_LOW_ACK;
#ifdef WAPP_SUPPORT
			wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Reject the AssocReq and DeltaRssi :%d\n",
				oceInfo->DeltaAssocRSSI);

			goto SendAssocResponse;
		} else
#endif /* OCE_SUPPORT */
			bAssocSkip = TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Accept RSSI: ===> %d, %d\n",
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold, rssi);
	}

#if defined(RT_CFG80211_SUPPORT) && defined(HOSTAPD_PMKID_IN_DRIVER_SUPPORT)
	{
		/*handle PMKID bug of iphone, sending PMKID even on phone reboot/forget AP*/
		UINT8 pmkid_count = 0;
		PUINT8 pPMKID = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE, ie_list->RSNIE_Len, PMKID_LIST, &pmkid_count);
		if (pEntry->isIphone) {
			if ((pPMKID != NULL) && NdisCmpMemory(pPMKID, pEntry->PmkidByHostapd, LEN_PMKID)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PMK mismatch\n");
				hex_dump_with_lvl("pEntry->PMKID:", pEntry->PmkidByHostapd, LEN_PMKID, DBG_LVL_ERROR);
				hex_dump_with_lvl("Assoc_req->PMKID:", pPMKID, LEN_PMKID, DBG_LVL_ERROR);
				StatusCode = MLME_INVALID_PMKID;
#ifdef WAPP_SUPPORT
				wapp_assoc_fail = MLME_INVALID_PMKID;
#endif /* WAPP_SUPPORT */
				goto SendAssocResponse;
			} else {
				if (pPMKID == NULL)
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, " PMKID NULL\n");
				else
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, " PMK match\n");
				}
			pEntry->isIphone = FALSE;
		}
	}
#endif /*RT_CFG80211_SUPPORT && HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/

#ifdef DOT11W_PMF_SUPPORT

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
#ifdef DOT11R_FT_SUPPORT
		&& (!IS_FT_RSN_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
#ifdef OCE_FILS_SUPPORT
				&& (pEntry->filsInfo.auth_algo != AUTH_MODE_FILS)
#endif /* OCE_FILS_SUPPORT */
		) {
		StatusCode = MLME_ASSOC_REJ_TEMPORARILY;
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_ASSOC_REJ_TEMP;
#endif /* WAPP_SUPPORT */
		goto SendAssocResponse;
	}

#endif /* DOT11W_PMF_SUPPORT */

	/* clear the previous Pairwise key table */
	if ((pEntry->Aid != 0)
#ifdef DOT11R_FT_SUPPORT
	&& (!IS_FT_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
#ifdef OCE_FILS_SUPPORT
		&& (pEntry->filsInfo.auth_algo != AUTH_MODE_FILS)
#endif /* OCE_FILS_SUPPORT */
		&& ((!IS_AKM_OPEN(pEntry->SecConfig.AKMMap)) || (!IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
#ifdef DOT1X_SUPPORT
			|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
		   )) {
		ASIC_SEC_INFO *Info;
		/* clear GTK state */
		pEntry->SecConfig.Handshake.GTKState = REKEY_NEGOTIATING;
		NdisZeroMemory(&pEntry->SecConfig.PTK, LEN_MAX_PTK);
		/* Set key material to Asic */

		os_alloc_mem(pAd, (UCHAR **)&Info, sizeof(ASIC_SEC_INFO));
		os_zero_mem(Info, sizeof(ASIC_SEC_INFO));
		Info->Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;


#ifndef SW_CONNECT_SUPPORT
		Info->Wcid = pEntry->wcid;
#else   /* !SW_CONNECT_SUPPORT */
		/* only need to del the real wcid key */
		Info->Wcid = pEntry->hw_wcid;
		if (!HcIsDummyWcid(pAd, Info->Wcid))
#endif  /* SW_CONNECT_SUPPORT */
		{
			/* Set key material to Asic */
			HW_ADDREMOVE_KEYTABLE(pAd, Info);
		}
#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

		/* Notify 802.1x daemon to clear this sta info */
		if (IS_AKM_1X(pEntry->SecConfig.AKMMap)
			|| IS_IEEE8021X(&pEntry->SecConfig))
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
		os_free_mem(Info);
	}

#ifdef WSC_AP_SUPPORT
	/* since sta has been left, ap should receive EapolStart and EapRspId again. */
	pEntry->Receive_EapolStart_EapRspId = 0;
	pEntry->bWscCapable = ie_list->bWscCapable;
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, wsc_ctrl->EntryAddr)) {
				BOOLEAN Cancelled;

				RTMPZeroMemory(wsc_ctrl->EntryAddr, MAC_ADDR_LEN);
				RTMPCancelTimer(&wsc_ctrl->EapolTimer, &Cancelled);
				wsc_ctrl->EapolTimerRunning = FALSE;
			}
		}

		if ((ie_list->RSNIE_Len == 0) &&
			(IS_AKM_WPA_CAPABILITY_Entry(wdev)) &&
			(wsc_ctrl->WscConfMode != WSC_DISABLE))
			pEntry->bWscCapable = TRUE;
	}

#endif /* WSC_AP_SUPPORT */

		/* for hidden SSID sake, SSID in AssociateRequest should be fully verified */
		if ((ie_list->SsidLen != pMbss->SsidLen) ||
			(NdisEqualMemory(ie_list->Ssid, pMbss->Ssid, ie_list->SsidLen) == 0))
			goto LabelOK;

#ifdef WSC_V2_SUPPORT
	/* Do not check ACL when WPS V2 is enabled and ACL policy is positive. */
	if ((pMbss->wdev.WscControl.WscConfMode != WSC_DISABLE) &&
		(pMbss->wdev.WscControl.bWscTrigger) &&
		(pMbss->wdev.WscControl.WscV2Info.bEnableWpsV2) &&
		(pMbss->wdev.WscControl.WscV2Info.bWpsEnable) &&
		(pMbss->AccessControlList.Policy == 1))
		;
	else
#endif /* WSC_V2_SUPPORT */
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
		/* set a flag for sending Assoc-Fail response to unwanted STA later. */
		if (map_is_entry_bl(pAd, ie_list->Addr2, pEntry->func_tb_idx) == TRUE) {
			bBlReject = TRUE;
		} else
#endif /*  MAP_BL_SUPPORT */

			/* set a flag for sending Assoc-Fail response to unwanted STA later. */
			if (!ApCheckAccessControlList(pAd, ie_list->Addr2, pEntry->func_tb_idx))
				bACLReject = TRUE;

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s MBSS(%d), receive %s request from "MACSTR" Rej BL/ACL %d/%d\n",
			  sAssoc, pEntry->func_tb_idx, sAssoc, MAC2STR(ie_list->Addr2), bBlReject, bACLReject);
#else
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s MBSS(%d), receive %s request from "MACSTR"\n",
			  sAssoc, pEntry->func_tb_idx, sAssoc, MAC2STR(ie_list->Addr2));
#endif

	p_sum_rate = (PUCHAR)sum_rate;
	SupportRate(rate, &p_sum_rate, &sum_rate_len, &MaxSupportedRate);

	/*
		Assign RateLen here or we will select wrong rate table in
		APBuildAssociation() when 11N compile option is disabled.
	*/
	pEntry->RateLen = sum_rate_len;
	RTMPSetSupportMCS(pAd,
					  OPMODE_AP,
					  pEntry,
					  rate,
#ifdef DOT11_VHT_AC
					  HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists),
					  &ie_list->cmm_ies.vht_cap,
#endif /* DOT11_VHT_AC */
					  &ie_list->cmm_ies.ht_cap,
					  HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists));
#ifdef GN_MIXMODE_SUPPORT
	if (pAd->CommonCfg.GNMixMode
		&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
			|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
			|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))) {
		pEntry->SupportRateMode &= ~SUPPORT_CCK_MODE;
		pEntry->SupportCCKMCS &= ~(1 << MCS_0 | 1 << MCS_1 | 1 << MCS_2 | 1 << MCS_3);
	}
#endif /* GN_MIXMODE_SUPPORT */

	/* 2. qualify this STA's auth_asoc status in the MAC table, decide StatusCode */
	StatusCode = APBuildAssociation(pAd, pEntry, ie_list, MaxSupportedRate, &Aid, isReassoc);

	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		update_sta_conn_state(pEntry->wdev, pEntry);
	}

#ifdef WAPP_SUPPORT
	if (StatusCode != MLME_SUCCESS)
		wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */

#ifdef DOT11R_FT_SUPPORT

	if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
		pFtCfg = &(wdev->FtCfg);

		if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
			&& (StatusCode == MLME_SUCCESS))
			StatusCode = FT_AssocReqHandler(pAd, isReassoc, pFtCfg, pEntry,
											&ie_list->FtInfo,
											ie_list->rsnxe_ie,
											ie_list->rsnxe_ie_len,
											FtInfoBuf);

#ifdef WAPP_SUPPORT
		if (StatusCode != MLME_SUCCESS)
			wapp_assoc_fail = FT_ERROR;
#endif /* WAPP_SUPPORT */

		/* just silencely discard this frame */
		if (StatusCode == 0xFFFF)
			goto LabelOK;
	}

#endif /* DOT11R_FT_SUPPORT */
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

#ifdef IGMP_TVM_SUPPORT
		if (IS_IGMP_TVM_MODE_EN(pEntry->wdev->IsTVModeEnable)) {
			/* Check whether the Peer has TV IE or not, because this needs to be set to */
			/* FW to enable/disabled Mcast Packet cloning and conversion */
			if (ie_list->tvm_ie.len == IGMP_TVM_IE_LENGTH) {
				if (ie_list->tvm_ie.data.field.TVMode == IGMP_TVM_IE_MODE_ENABLE)
					pEntry->TVMode = IGMP_TVM_IE_MODE_ENABLE;
				else
					pEntry->TVMode = IGMP_TVM_IE_MODE_AUTO;
			} else {
				pEntry->TVMode = IGMP_TVM_IE_MODE_DISABLE;
			}
		} else {
			pEntry->TVMode = IGMP_TVM_IE_MODE_DISABLE;
		}
#endif /* IGMP_TVM_SUPPORT */


	if (StatusCode == MLME_ASSOC_REJ_DATA_RATE)
		RTMPSendWirelessEvent(pAd, IW_STA_MODE_EVENT_FLAG, pEntry->Addr, wdev->wdev_idx, 0);

SendAssocResponse:

	/* 3. send Association Response */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		goto LabelOK;

#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd) && (ie_list->ecdh_ie.ext_ie_id == IE_WLAN_EXTENSION)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "ASSOC rejected due to Diffie-Hellman IE element!\n");
		StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
	}
#endif /* MAP_R3 */

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "%s Send %s response (Status=%d)...\n",
			  sAssoc, sAssoc, StatusCode);
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
	SupRateLen = rate->sup_rate_len;

	/* TODO: need to check rate in support rate element, not number */
	if (FlgIs11bSta == TRUE)
		SupRateLen = 4;

	if (bACLReject == TRUE || bAssocSkip
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
		|| bBlReject == TRUE
#endif
	) {
		MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2,
						 wdev->if_addr, wdev->bssid);
		StatusCode = MLME_UNSPECIFY_FAIL;
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_UNSPECIFY_FAILURE;
#endif /* WAPP_SUPPORT */
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &AssocRspHdr,
						  2,                        &CapabilityInfoForAssocResp,
						  2,                        &StatusCode,
						  2,                        &Aid,
						  END_OF_ARGS);
		FrameLen += build_support_rate_ie(wdev, rate->sup_rate, SupRateLen, pOutBuffer + FrameLen);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory((PVOID) pOutBuffer);
		pOutBuffer = NULL;
		RTMPSendWirelessEvent(pAd, IW_MAC_FILTER_LIST_EVENT_FLAG, ie_list->Addr2, wdev->wdev_idx, 0);
#ifdef WSC_V2_SUPPORT

		/* If this STA exists, delete it. */
		if (pEntry)
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

#endif /* WSC_V2_SUPPORT */

		if (bAssocSkip == TRUE) {
			pEntry = MacTableLookup(pAd, ie_list->Addr2);

			if (pEntry)
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}

		goto LabelOK;
	}

	MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2,
					 wdev->if_addr, wdev->bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &AssocRspHdr,
					  2,                        &CapabilityInfoForAssocResp,
					  2,                        &StatusCode,
					  2,                        &Aid,
					  END_OF_ARGS);
	FrameLen += build_support_rate_ie(wdev, rate->sup_rate, SupRateLen, pOutBuffer + FrameLen);

	if (FlgIs11bSta == FALSE)
		FrameLen += build_support_ext_rate_ie(wdev, SupRateLen, rate->ext_rate,
						rate->ext_rate_len, pOutBuffer + FrameLen);

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd)) {
		pEntry->DevPeerRole = ie_list->MAP_AttriValue;
#ifdef MAP_R2
		pEntry->profile = ie_list->MAP_ProfileValue;
#endif
		if ((IS_MAP_TURNKEY_ENABLE(pAd)) &&
			(((pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)) &&
			(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
			!(is_controller_found(wdev))) ||
			((!ie_list->MAP_AttriValue) &&
			(!(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_FRONTHAUL_BSS)))))) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"disallowing connection, DevOwnRole=%u,DevPeerRole=%u,controller=%d\n",
					 wdev->MAPCfg.DevOwnRole, pEntry->DevPeerRole, is_controller_found(wdev));
			MlmeDeAuthAction(pAd, pEntry, REASON_DECLINED, FALSE);
			goto LabelOK;
		} else
			MAP_InsertMapCapIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen);
	}
#endif /* CONFIG_MAP_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		RRM_InsertRRMEnCapIE(pAd, wdev, pOutBuffer + FrameLen, &FrameLen, pEntry->func_tb_idx);

#endif /* DOT11K_RRM_SUPPORT */

	/* add WMM IE here */
	/* printk("%s()=>bWmmCapable=%d,CLINE=%d\n",__FUNCTION__,wdev->bWmmCapable,CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)); */

	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);
	}

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
#ifdef CONFIG_OWE_SUPPORT
	if (IS_AKM_OWE_Entry(pEntry)) {
		CHAR rsne_idx;
		ULONG	TmpLen;
		struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
		/*struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;*/

		WPAMakeRSNIE(wdev->wdev_type, pSecConfig, pEntry);

		for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
			if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
				continue;

			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								1, &pSecConfig->RSNE_EID[rsne_idx][0],
								1, &pSecConfig->RSNE_Len[rsne_idx],
								pSecConfig->RSNE_Len[rsne_idx],
								&pSecConfig->RSNE_Content[rsne_idx][0],
								END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
#endif
#ifdef DOT11R_FT_SUPPORT
	if ((pFtCfg != NULL) && (pFtCfg->FtCapFlag.Dot11rFtEnable)) {
		PUINT8	mdie_ptr;
		UINT8	mdie_len;
		PUINT8	ftie_ptr = NULL;
		UINT8	ftie_len = 0;
		PUINT8  ricie_ptr = NULL;
		UINT8   ricie_len = 0;
		UCHAR rsnxe_ie_len = 0;

#ifdef HOSTAPD_WPA3R3_SUPPORT
		rsnxe_ie_len = build_rsnxe_ie(wdev, &wdev->SecConfig, rsnxe_ie);
#else
		rsnxe_ie_len = build_rsnxe_ie(&wdev->SecConfig, rsnxe_ie);
#endif

		if ((rsnxe_ie_len != 0 || wpa3_test_ctrl == 8) && FtInfoBuf->FtIeInfo.MICCtr.field.IECnt)
			FtInfoBuf->FtIeInfo.MICCtr.field.rsnxe_used = 1;

		/* for ft-sae rsnxe interop issue, we should not carry rsnxe if peer not support */
		if (pEntry->SecConfig.rsnxe_len == 0)
			rsnxe_ie_len = 0;

		if (FtInfoBuf->FtIeInfo.MICCtr.field.IECnt) {
			if (rsnxe_ie_len != 0)
				FtInfoBuf->FtIeInfo.MICCtr.field.IECnt++;
			else
				check_rsnxe_install = FALSE;

			if (ie_list->FtInfo.RicInfo.Len)
				FtInfoBuf->FtIeInfo.MICCtr.field.IECnt++;
		}

		/* Insert RSNIE if necessary */
		if (FtInfoBuf->RSNIE_Len != 0) {
			ULONG TmpLen;

			MakeOutgoingFrame(pOutBuffer + FrameLen,      &TmpLen,
							  FtInfoBuf->RSNIE_Len,		FtInfoBuf->RSN_IE,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

		/* Insert MDIE. */
		mdie_ptr = pOutBuffer + FrameLen;
		mdie_len = 5;

		/* Insert MdId only if the Peer has sent one */
		if (FtInfoBuf->MdIeInfo.Len != 0) {
			FT_InsertMdIE(pOutBuffer + FrameLen,
					  &FrameLen,
					  FtInfoBuf->MdIeInfo.MdId,
					  FtInfoBuf->MdIeInfo.FtCapPlc);
		}

		/* Insert FTIE. */
		if (FtInfoBuf->FtIeInfo.Len != 0) {
			ftie_ptr = pOutBuffer + FrameLen;
			ftie_len = (2 + FtInfoBuf->FtIeInfo.Len);
			FT_InsertFTIE(pOutBuffer + FrameLen, &FrameLen,
						  FtInfoBuf->FtIeInfo.Len,
						  FtInfoBuf->FtIeInfo.MICCtr,
						  FtInfoBuf->FtIeInfo.MIC,
						  FtInfoBuf->FtIeInfo.ANonce,
						  FtInfoBuf->FtIeInfo.SNonce);
		}

		/* Insert R1KH IE into FTIE. */
		if (FtInfoBuf->FtIeInfo.R1khIdLen != 0)
			FT_FTIE_InsertKhIdSubIE(pOutBuffer + FrameLen,
									&FrameLen,
									FT_R1KH_ID,
									FtInfoBuf->FtIeInfo.R1khId,
									FtInfoBuf->FtIeInfo.R1khIdLen);

		/* Insert GTK Key info into FTIE. */
		if (FtInfoBuf->FtIeInfo.GtkLen != 0)
			FT_FTIE_InsertSubIE(pOutBuffer + FrameLen,
								   &FrameLen,
								   FT_GTK,
								   FtInfoBuf->FtIeInfo.GtkSubIE,
								   FtInfoBuf->FtIeInfo.GtkLen);

		/* Insert R0KH IE into FTIE. */
		if (FtInfoBuf->FtIeInfo.R0khIdLen != 0)
			FT_FTIE_InsertKhIdSubIE(pOutBuffer + FrameLen,
									&FrameLen,
									FT_R0KH_ID,
									FtInfoBuf->FtIeInfo.R0khId,
									FtInfoBuf->FtIeInfo.R0khIdLen);

		/* Insert RIC. */
		if (ie_list->FtInfo.RicInfo.Len) {
			ULONG TempLen;

			FT_RIC_ResourceRequestHandle(pAd, pEntry,
										 (PUCHAR)ie_list->FtInfo.RicInfo.pRicInfo,
										 ie_list->FtInfo.RicInfo.Len,
										 (PUCHAR)pOutBuffer + FrameLen,
										 (PUINT32)&TempLen);
			ricie_ptr = (PUCHAR)(pOutBuffer + FrameLen);
			ricie_len = TempLen;
			FrameLen += TempLen;
		}
		/* Insert IGTK Key info into FTIE. */

		if (FtInfoBuf->FtIeInfo.IGtkLen != 0) {
			FT_FTIE_InsertSubIE(pOutBuffer+FrameLen,
					&FrameLen,
					FT_IGTK_ID,
					FtInfoBuf->FtIeInfo.IGtkSubIE,
					FtInfoBuf->FtIeInfo.IGtkLen);
		}

		/* Insert OCI into FTIE. */

		if (FtInfoBuf->FtIeInfo.OCILen != 0) {
			FT_FTIE_InsertSubIE(pOutBuffer+FrameLen,
					&FrameLen,
					FT_OCI_ID,
					FtInfoBuf->FtIeInfo.OCISubIE,
					FtInfoBuf->FtIeInfo.OCILen);
		}

		/* Insert BIGTK Key info into FTIE. */

		if (FtInfoBuf->FtIeInfo.BIGtkLen != 0) {
			FT_FTIE_InsertSubIE(pOutBuffer+FrameLen,
					&FrameLen,
					FT_BIGTK_ID,
					FtInfoBuf->FtIeInfo.BIGtkSubIE,
					FtInfoBuf->FtIeInfo.BIGtkLen);
		}

		/* Calculate the FT MIC for FT procedure */
		if (FtInfoBuf->FtIeInfo.MICCtr.field.IECnt) {
			UINT8	ft_mic[FT_MIC_LEN];
			PFT_FTIE	pFtIe;

			FT_CalculateMIC(pEntry->Addr,
							wdev->bssid,
							pEntry->FT_PTK,
							6,
							FtInfoBuf->RSN_IE,
							FtInfoBuf->RSNIE_Len,
							mdie_ptr,
							mdie_len,
							ftie_ptr,
							ftie_len,
							ricie_ptr,
							ricie_len,
							rsnxe_ie,
							rsnxe_ie_len,
							ft_mic);
			/* Update the MIC field of FTIE */
			pFtIe = (PFT_FTIE)(ftie_ptr + 2);
			NdisMoveMemory(pFtIe->MIC, ft_mic, FT_MIC_LEN);
			/* Install pairwise key */
		}

		/*	Record the MDIE & FTIE of (re)association response of
			Initial Mobility Domain Association. It's used in
			FT 4-Way handshaking */
		if ((IS_AKM_WPA2_Entry(pEntry) || IS_AKM_WPA2PSK_Entry(pEntry)
			|| IS_AKM_WPA3PSK_Entry(pEntry) || IS_AKM_WPA3_192BIT_Entry(pEntry))
			&& ie_list->FtInfo.FtIeInfo.Len == 0) {
			NdisMoveMemory(&pEntry->InitialMDIE,
						   mdie_ptr, mdie_len);
			pEntry->InitialFTIE_Len = ftie_len;
			NdisMoveMemory(pEntry->InitialFTIE, ftie_ptr, ftie_len);
		}
	}
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11_N_SUPPORT
	{
		BOOLEAN HtEnable = (HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists) &&
			WMODE_CAP_N(wdev->PhyMode) && wdev->DesiredHtPhyInfo.bHtEnable) ? TRUE : FALSE;

		/* HT capability in AssocRsp frame. */
		if (HtEnable) {
			ie_info.is_draft_n_type = FALSE;
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);

			if ((ie_list->cmm_ies.vendor_ie.ra_cap) == 0 || (pAd->bBroadComHT == TRUE)) {
				ie_info.is_draft_n_type = TRUE;
				ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
				FrameLen += build_ht_ies(pAd, &ie_info);

			}
		}

		/* BSS Max Idle Period element */
		if (pMbss->max_idle_ie_en) {
			FrameLen += build_bss_max_idle_ie(wdev, (UCHAR *)(pOutBuffer + FrameLen),
							pMbss->max_idle_period,
							pMbss->max_idle_option);
		}

		if (HtEnable) {
			struct _build_ie_info vht_ie_info;

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

			txbf_bfee_cap_set(TRUE,
							  ie_list->cmm_ies.vht_cap.vht_cap.bfer_cap_su,
							  ie_list->cmm_ies.vht_cap.vht_cap.num_snd_dimension);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
		if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)
			|| HAS_VHT_OP_EXIST(ie_list->cmm_ies.ie_exists)
			|| !WMODE_CAP_2G(wdev->PhyMode))
			FrameLen += build_vht_ies(pAd, &vht_ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
		}

#ifdef DOT11_HE_AX
	if (HAS_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists)
		&& IS_HE_STA(pEntry->cap.modes) && WMODE_CAP_AX(wdev->PhyMode)
			&& wdev->DesiredHtPhyInfo.bHtEnable) {
		UINT32 offset = 0;

		offset = add_assoc_rsp_he_ies(wdev, (UINT8 *)pOutBuffer, FrameLen);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"add he assoc_rsp, len=%d\n", offset);
		FrameLen += offset;
	}
#endif /*DOT11_HE_AX*/
	}
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_HOTSPOT_R2
	/* qosmap IE */
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "entry wcid %d QosMapSupport=%d\n",
			 pEntry->wcid, pEntry->QosMapSupport);

	if (pEntry->QosMapSupport) {
		ULONG	TmpLen;
		UCHAR	QosMapIE, ielen = 0, explen = 0;

		pHSCtrl = &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

		if (pHSCtrl->QosMapEnable && pHSCtrl->HotSpotEnable) {
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
#ifdef QOS_R1
	if (IS_QOSR1_ENABLE(pAd)) {
		ULONG	TmpLen = 0;
		UCHAR	QosMapIE, tmpbuf[50] = {0}, ielen = 0, explen = 0;
		BSS_STRUCT *pmbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

		if (/*pEntry->QosMapSupport &&*/
			pmbss->QosMapSupport && pmbss->QoSMapIsUP) {
			QosMapIE = IE_QOS_MAP_SET;
			explen = pmbss->DscpExceptionCount;
			pEntry->DscpExceptionCount = explen;
			memcpy((UCHAR *)pEntry->DscpRange, (UCHAR *)pmbss->DscpRange, 16);
			memcpy((UCHAR *)pEntry->DscpException, (UCHAR *)pmbss->DscpException, 42);
			ielen = explen + 16;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1,			&QosMapIE,
							  1,			&ielen,
							  explen,		pEntry->DscpException,
							  16,			pEntry->DscpRange,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
		if (ie_list->has_mscs_req == 1) {
			QoS_get_default_mscs_descriptor(tmpbuf, &ielen);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen, ielen, tmpbuf, END_OF_ARGS);
			FrameLen += TmpLen;
		}
#ifdef QOS_R2
		if (pEntry->DSCPPolicyEnable != ie_list->DSCPPolicyEnable) {
			pEntry->DSCPPolicyEnable = ie_list->DSCPPolicyEnable;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%pM, DSCPPolicyEnable=%d\n", pEntry->Addr, pEntry->DSCPPolicyEnable);
		}

		if (pEntry->DSCPPolicyEnable && pmbss->bDSCPPolicyEnable) {
			QoS_Build_WFACapaIE(tmpbuf, &ielen, pmbss->bDSCPPolicyEnable);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen, ielen, tmpbuf, END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif
	}
#endif

	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += build_extended_cap_ie(pAd, &ie_info);
#ifdef CONFIG_DOT11V_WNM
		/* #ifdef CONFIG_HOTSPOT_R2 Remove for WNM independance */
		if (ie_list->ExtCapInfo.BssTransitionManmt == 1)
			pEntry->bBSSMantSTASupport = TRUE;
#endif /* CONFIG_DOT11V_WNM */

#ifdef DOT11V_MBSSID_SUPPORT
		if (IS_MBSSID_IE_NEEDED(pAd, pMbss, HcGetBandByWdev(wdev)))
			ie_list->ExtCapInfo.mbssid = 1;
		else
			ie_list->ExtCapInfo.mbssid = 0;
#endif

	/* add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back */
	FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_ASSOC_RESP
							   );
#if defined(MBO_SUPPORT) || defined(OCE_SUPPORT)
	if (IS_MBO_ENABLE(wdev) || IS_OCE_ENABLE(wdev))
		MakeMboOceIE(pAd, wdev, pEntry, pOutBuffer+FrameLen, &FrameLen, MBO_FRAME_TYPE_ASSOC_RSP);
#endif /* OCE_SUPPORT MBO_SUPPORT */

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

#ifdef OCE_FILS_SUPPORT
	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += oce_build_ies(pAd, &ie_info, TRUE);
#endif /*OCE_FILS_SUPPORT */

#ifndef HOSTAPD_WPA3_SUPPORT
	if (check_rsnxe_install) {
		if (wpa3_test_ctrl == 9) {
#ifdef HOSTAPD_WPA3R3_SUPPORT
			INT len = build_rsnxe_ie(wdev, &wdev->SecConfig, (UCHAR *)pOutBuffer + FrameLen);
#else
			INT len = build_rsnxe_ie(&wdev->SecConfig, (UCHAR *)pOutBuffer + FrameLen);
#endif
			if (len != 0) {
				UCHAR cap;
				UCHAR *buf = (UCHAR *)pOutBuffer + FrameLen;

				NdisMoveMemory(&cap, buf + 2, sizeof(cap));
				cap |= (1 << IE_RSNXE_CAPAB_PROTECTED_TWT);
				NdisMoveMemory(buf + 2, &cap, sizeof(cap));
				hex_dump_with_cat_and_lvl("rsnxe content", buf, len, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO);

				FrameLen += len;
			}
		} else
#ifdef HOSTAPD_WPA3R3_SUPPORT
			FrameLen +=  build_rsnxe_ie(wdev, &wdev->SecConfig, (UCHAR *)pOutBuffer + FrameLen);
#else
			FrameLen +=  build_rsnxe_ie(&wdev->SecConfig, (UCHAR *)pOutBuffer + FrameLen);
#endif
	}

#ifdef CONFIG_OWE_SUPPORT
	if (IS_AKM_OWE_Entry(pEntry) && (StatusCode == MLME_SUCCESS)) {
		BOOLEAN need_ecdh_ie = FALSE;
		INT CacheIdx;/* Key cache */
		UINT8 *pmkid = NULL;
		UINT8 pmkid_count = 0;

		pmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE, ie_list->RSNIE_Len, PMKID_LIST, &pmkid_count);
		if (pmkid != NULL) {
			CacheIdx = RTMPSearchPMKIDCache(&pAd->ApCfg.PMKIDCache, pEntry->func_tb_idx, pEntry->Addr, FALSE);
			if ((CacheIdx == -1) ||
			    ((RTMPEqualMemory(pmkid,
					      &pAd->ApCfg.PMKIDCache.BSSIDInfo[CacheIdx].PMKID,
					      LEN_PMKID)) == 0)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"AKM_OWE_Entry PMKID not found, do normal ECDH procedure\n");
				need_ecdh_ie = TRUE;
			}
		} else
			need_ecdh_ie = TRUE;

		if (need_ecdh_ie == TRUE) {
			FrameLen +=  build_owe_dh_ie(pAd,
						     pEntry,
						     (UCHAR *)pOutBuffer + FrameLen,
						     pEntry->SecConfig.owe.last_try_group);
		}
	}
#endif /*CONFIG_OWE_SUPPORT*/
#else
#ifdef HOSTAPD_WPA3R3_SUPPORT
			FrameLen +=  build_rsnxe_ie(wdev, &wdev->SecConfig, (UCHAR *)pOutBuffer + FrameLen);
#else
			FrameLen +=  build_rsnxe_ie(&wdev->SecConfig, (UCHAR *)pOutBuffer + FrameLen);
#endif
#endif /* HOSTAPD_WPA3_SUPPORT*/

#ifdef IGMP_TVM_SUPPORT
	/* ADD TV IE to this packet */
	MakeTVMIE(pAd, wdev, pOutBuffer, &FrameLen);
#endif /* IGMP_TVM_SUPPORT*/


#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	ap_vendor_ie = &pAd->ApCfg.MBSSID[pEntry->apidx].ap_vendor_ie;
	RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);
	if (ap_vendor_ie->pointer != NULL) {
		ULONG TmpLen;

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"SYNC - Send Association response to "MACSTR"...and add vendor ie\n",
			MAC2STR(ie_list->Addr2));
		MakeOutgoingFrame(pOutBuffer + FrameLen,
				  &TmpLen,
				  ap_vendor_ie->length,
				  ap_vendor_ie->pointer,
				  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef OCE_FILS_SUPPORT

	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		if ((pEntry->filsInfo.auth_algo == AUTH_MODE_FILS) &&
			IS_AKM_FILS(wdev->SecConfig.AKMMap) &&
			IS_AKM_FILS(pEntry->SecConfig.AKMMap)) {
			struct fils_info *filsInfo = &pEntry->filsInfo;
			PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg;

			if (!filsInfo->is_pending_assoc) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "STA - "MACSTR" do FILS assoc with Pending\n",
						  MAC2STR(ie_list->Addr2));

				if (filsInfo->pending_ie) {
					os_free_mem(filsInfo->pending_ie);
					filsInfo->pending_ie_len = 0;
					filsInfo->pending_ie = NULL;
				}

				if (filsInfo->extra_ie) {
					os_free_mem(filsInfo->extra_ie);
					filsInfo->extra_ie_len = 0;
					filsInfo->extra_ie = NULL;
				}

				filsInfo->pending_ie_len = Elem->MsgLen;
				os_alloc_mem(NULL, (UCHAR **)&filsInfo->pending_ie, filsInfo->pending_ie_len);
				if (!filsInfo->pending_ie) {
					goto LabelOK;
				}
				NdisMoveMemory(filsInfo->pending_ie, Elem->Msg, filsInfo->pending_ie_len);

				filsInfo->extra_ie_len = FrameLen;
				os_alloc_mem(NULL, (UCHAR **)&filsInfo->extra_ie, filsInfo->extra_ie_len);
				if (!filsInfo->extra_ie) {
					goto LabelOK;
				}
				NdisMoveMemory(filsInfo->extra_ie, pOutBuffer, filsInfo->extra_ie_len);

				NdisMoveMemory(&filsInfo->rssi_info, &Elem->rssi_info, sizeof(struct raw_rssi_info));
				if (isReassoc)
					filsInfo->pending_action = ap_assoc_api.peer_reassoc_req_action;
				else
					filsInfo->pending_action = ap_assoc_api.peer_assoc_req_action;

				filsInfo->is_pending_assoc = TRUE;
				filsInfo->last_pending_id = Fr->Hdr.Sequence;

				DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_MLME_EVENT);
				goto free_check;
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "STA - "MACSTR" skip FILS assoc in Pending\n",
						  MAC2STR(ie_list->Addr2));
				goto free_check;
			}
		}
	}
#endif /* OCE_FILS_SUPPORT */

	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	/* For re-assoc resp pkt when FT is enabled along with Beacon Protection
	 * the packet size go beyong to 512 due to which re assoc resp packet
	 * corruption observed in air. Whereas pkt is created correcltly in driver.
	 * The delay helped to prevent the on air packet corruption.
	 * This is a work around till proper fix is identified.
	 */
	if (FrameLen >= 512)
		os_usec_delay(2000);
	MlmeFreeMemory((PVOID) pOutBuffer);
	pOutBuffer = NULL;

#ifdef OCE_FILS_SUPPORT
assoc_post:
#endif /* OCE_FILS_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT

	if (StatusCode == MLME_ASSOC_REJ_TEMPORARILY)
		PMF_MlmeSAQueryReq(pAd, pEntry);

#endif /* DOT11W_PMF_SUPPORT */

	/*is status is success ,update STARec*/
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		if (wdev_do_conn_act(pEntry->wdev, pEntry) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "connect action fail!!\n");
		}

#ifdef WTBL_TDD_SUPPORT
		if (IS_WTBL_TDD_ENABLED(pAd)) {
			pEntry->wtblTddCtrl.state = WTBL_TDD_STA_IDLE;
			pEntry->wtblTddCtrl.guardTime = 0;
		}
#endif /* WTBL_TDD_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
		if ((pFtCfg != NULL) && (pFtCfg->FtCapFlag.Dot11rFtEnable)) {
			struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
			struct _SEC_KEY_INFO *pKey = NULL;

			if (FtInfoBuf->FtIeInfo.MICCtr.field.IECnt) {

#ifdef MBO_SUPPORT
					/* YF_FT */
				if (IS_MBO_ENABLE(wdev))
					/* update STA bssid & security info to daemon */
					MboIndicateStaBssidInfo(pAd, wdev, pEntry->Addr);
#endif /* MBO_SUPPORT */
#ifdef OCE_SUPPORT
		if (IS_OCE_ENABLE(wdev))
			/* update OCE STA info to daemon */
			OceIndicateStaInfo(pAd, wdev, pEntry->Addr);
#endif /* OCE_SUPPORT */

				pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;
				pSecConfig->Handshake.WpaState = AS_PTKINITDONE;
				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
				if (info) {

					os_zero_mem(info, sizeof(ASIC_SEC_INFO));
					NdisCopyMemory(pSecConfig->PTK, pEntry->FT_PTK, LEN_MAX_PTK);
					info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
					info->Direction = SEC_ASIC_KEY_BOTH;
					info->Wcid = pEntry->wcid;
					info->BssIndex = pEntry->func_tb_idx;
					info->Cipher = pSecConfig->PairwiseCipher;
					info->KeyIdx = pSecConfig->PairwiseKeyId;
					os_move_mem(&info->PeerAddr[0],
									pEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(info->Key.Key,
									&pEntry->FT_PTK[LEN_PTK_KCK + LEN_PTK_KEK],
									(LEN_TK + LEN_TK2));
					pKey = &info->Key;

					if (IS_CIPHER_TKIP(info->Cipher))
							pKey->KeyLen = LEN_TKIP_TK;
					else if (IS_CIPHER_CCMP128(info->Cipher))
							pKey->KeyLen = LEN_CCMP128_TK;
					else if (IS_CIPHER_CCMP256(info->Cipher))
							pKey->KeyLen = LEN_CCMP256_TK;
					else if (IS_CIPHER_GCMP128(info->Cipher))
							pKey->KeyLen = LEN_GCMP128_TK;
					else if (IS_CIPHER_GCMP256(info->Cipher))
							pKey->KeyLen = LEN_GCMP256_TK;

					if (IS_CIPHER_TKIP(info->Cipher)) {
							os_move_mem(pKey->TxMic, &pKey->Key[LEN_TK], LEN_TKIP_MIC);
							os_move_mem(pKey->RxMic, &pKey->Key[LEN_TK + 8], LEN_TKIP_MIC);
					}
					WifiSysUpdatePortSecur(pAd, pEntry, info);
				} else {
					WifiSysUpdatePortSecur(pAd, pEntry, NULL);
				}
			}
		}
#endif /* DOT11R_FT_SUPPORT */

#ifdef REDUCE_TCP_ACK_SUPPORT
		/* RACK is enabled only for 1ss sta*/
		if (IS_ENTRY_CLIENT(pEntry) &&
			pEntry->SupportVHTMCS1SS &&
			(pEntry->SupportVHTMCS2SS == 0) &&
			(pEntry->SupportVHTMCS3SS == 0) &&
			(pEntry->SupportVHTMCS4SS == 0) &&
			(pAd->MacTab.Size == 1))
			pEntry->RACKEnalbedSta = TRUE;
		else
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s(): Not RACK Enalbed Sta !!\n");
#endif
	}

	/* set up BA session */
	if (StatusCode == MLME_SUCCESS) {

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
		if (IS_MAP_ENABLE(pAd) && (pAd->CommonCfg.SRMode[HcGetBandByWdev(wdev)] == TRUE))
			SrMeshSrUpdateSTAMode(pAd, wdev, TRUE, IS_HE_STA(pEntry->cap.modes));
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) */

	/* In WPA or 802.1x mode, the port is not secured, otherwise is secued. */
	if (!(IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
		|| IS_IEEE8021X_Entry(wdev)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
		|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
#ifdef WSC_INCLUDED
		|| (pEntry->bWscCapable && IS_AKM_WPA_CAPABILITY_Entry(wdev))
#endif /* WSC_INCLUDED */
	   ))
		tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

#ifdef WH_EVENT_NOTIFIER

	if (pEntry && tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		EventHdlr pEventHdlrHook = NULL;

		pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_JOIN);

		if (pEventHdlrHook && pEntry->wdev)
			pEventHdlrHook(pAd, pEntry, Elem);
	}

#endif /* WH_EVENT_NOTIFIER */

		pEntry->PsMode = PWR_ACTIVE;
		tr_ctl->tr_entry[pEntry->wcid].PsMode = PWR_ACTIVE;
		MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
#ifdef IAPP_SUPPORT
		/*PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg; */
		/*		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie; */
		{
			/* send association ok message to IAPPD */
			IAPP_L2_Update_Frame_Send(pAd, pEntry->Addr, pEntry->wdev->wdev_idx);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					  "####### Send L2 Frame Mac="MACSTR"\n",
					  MAC2STR(pEntry->Addr));
		}

		/*		SendSingalToDaemon(SIGUSR2, pObj->IappPid); */
#ifdef DOT11R_FT_SUPPORT
		{
			/*
				Do not do any check here.
				We need to send MOVE-Req frame to AP1 even open mode.
			*/
			/*		if (IS_FT_RSN_STA(pEntry) && (FtInfo.FtIeInfo.Len != 0)) */
			if (IS_FT_STA(pEntry)) {
				if (isReassoc == 1) {
					/* only for reassociation frame */
					FT_KDP_EVT_REASSOC EvtReAssoc;

					EvtReAssoc.SeqNum = 0;
					NdisMoveMemory(EvtReAssoc.MacAddr, pEntry->Addr, MAC_ADDR_LEN);
					NdisMoveMemory(EvtReAssoc.OldApMacAddr, ie_list->ApAddr, MAC_ADDR_LEN);
					FT_KDP_EVENT_INFORM(pAd, pEntry->func_tb_idx, FT_KDP_SIG_FT_REASSOCIATION,
										&EvtReAssoc, sizeof(EvtReAssoc), NULL);
				}
			}
		}

#endif /* DOT11R_FT_SUPPORT */
#endif /* IAPP_SUPPORT */
		/* ap_assoc_info_debugshow(pAd, isReassoc, pEntry, ie_list); */
		/* send wireless event - for association */
		RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pEntry->Addr, 0, 0);
		/* This is a reassociation procedure */
		pEntry->IsReassocSta = isReassoc;
		/* clear txBA bitmap */
		pEntry->TXBAbitmap = 0;

		if (pEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX) {
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

			if (WMODE_CAP_2G(wdev->PhyMode) &&
				addht->AddHtInfo.ExtChanOffset &&
				(ie_list->cmm_ies.ht_cap.HtCapInfo.ChannelWidth == BW_40))
				SendBeaconRequest(pAd, pEntry->wcid);

			ba_ori_session_start(pAd, tr_entry, 5);
		}

#ifdef DOT11R_FT_SUPPORT

		/*	If the length of FTIE field of the (re)association-request frame
			is larger than zero, it shall indicate the Fast-BSS transition is in progress. */
		if (ie_list->FtInfo.FtIeInfo.Len > 0)
			;
		else
#endif /* DOT11R_FT_SUPPORT */
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
					CFG80211OS_NewSta(pEntry->wdev->if_dev,
							  ie_list->Addr2,
							  (PUCHAR)Elem->Msg,
							  Elem->MsgLen, isReassoc);
#endif
					if (IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher)) {
						ASIC_SEC_INFO *Info;

						os_alloc_mem(pAd, (UCHAR **)&Info, sizeof(ASIC_SEC_INFO));
						os_zero_mem(Info, sizeof(ASIC_SEC_INFO));

						/* Set key material to Asic */
						Info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
						Info->Direction = SEC_ASIC_KEY_BOTH;
						Info->Wcid = pEntry->wcid;
						Info->BssIndex = pEntry->func_tb_idx;
						Info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						Info->Cipher = pEntry->SecConfig.PairwiseCipher;
						Info->KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						os_move_mem(&Info->Key,
							    &pEntry->SecConfig.WepKey[pEntry->SecConfig.PairwiseKeyId],
							    sizeof(SEC_KEY_INFO));
						os_move_mem(&Info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
						HW_ADDREMOVE_KEYTABLE(pAd, Info);
						os_free_mem(Info);
					}
				}

				hex_dump("ASSOC_REQ", Elem->Msg, Elem->MsgLen);
			} else
#endif

			/* enqueue a EAPOL_START message to trigger EAP state machine doing the authentication */
			if (IS_AKM_PSK_Entry(pEntry)) {
				INT cacheidx;

				if (is_rsne_pmkid_cache_match(ie_list->RSN_IE,
							      ie_list->RSNIE_Len,
							      &pAd->ApCfg.PMKIDCache,
							      pEntry->func_tb_idx,
							      pEntry->Addr,
							      &cacheidx)) {
					store_pmkid_cache_in_sec_config(pAd, pEntry, cacheidx);
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						  "ASSOC - CacheIdx = %d\n",
						  cacheidx);
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
#ifdef DPP_SUPPORT
						!(IS_AKM_DPP(pEntry->SecConfig.AKMMap)) &&
#endif /* DPP_SUPPORT */
					    !(IS_AKM_OWE(pEntry->SecConfig.AKMMap)))
						os_move_mem(&pEntry->SecConfig.PMK, &wdev->SecConfig.PMK, LEN_PMK);

					RTMPSetTimer(&pEntry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
				}
			}

#ifdef DOT1X_SUPPORT
			else if (IS_AKM_WPA2_Entry(pEntry) ||
				 IS_AKM_WPA3_192BIT_Entry(pEntry)) {
				INT	cacheidx;

				if (is_rsne_pmkid_cache_match(ie_list->RSN_IE,
							      ie_list->RSNIE_Len,
							      &pAd->ApCfg.PMKIDCache,
							      pEntry->func_tb_idx,
							      pEntry->Addr,
							      &cacheidx))
					process_pmkid(pAd, wdev, pEntry, cacheidx);
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
#if defined(MWDS) || defined(CONFIG_BS_SUPPORT) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT) || defined(QOS_R1)
		if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef MWDS
			MWDSAPPeerEnable(pAd, pEntry);
#endif
#ifdef CONFIG_MAP_SUPPORT
			map_a4_peer_enable(pAd, pEntry, TRUE);
#endif /* CONFIG_MAP_SUPPORT */
#ifdef WAPP_SUPPORT
			wapp_send_cli_join_event(pAd, pEntry);
#endif
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
			if (IS_AKM_OPEN(wdev->SecConfig.AKMMap) && IS_QOSR1_ENABLE(pAd) &&
					pAd->SupportFastPath && pEntry->MSCSSupport) {
				pEntry->dabs_trans_id = 1;
				Send_DABS_Announce(pAd, pEntry->wcid);
				RTMPSetTimer(&pEntry->DABSRetryTimer, DABS_WAIT_TIME);
				OS_SET_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
			}
#endif
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
						  "%s():sta("MACSTR") add!\n",
						  MAC2STR(pEntry->Addr));
			}

			RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqflags);
		}
#endif /* SMART_ANTENNA // */

#ifdef GREENAP_SUPPORT
		if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC))
			greenap_check_peer_connection_at_link_up_down(pAd, wdev);
#endif /* GREENAP_SUPPORT */

#ifdef CONFIG_HOTSPOT_R2
		pHSCtrl = &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

		/* add to cr4 pool */
		if (pEntry->QosMapSupport && pHSCtrl->HotSpotEnable) {
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

#ifdef MBSS_AS_WDS_AP_SUPPORT
	if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
		if (IS_ENTRY_CLIENT(pEntry)) {
			pEntry->bEnable4Addr = TRUE;
			if (wdev->wds_enable)
				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
			else if (MAC_ADDR_EQUAL(pAd->ApCfg.wds_mac, pEntry->Addr))
				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
			}
		}
#endif

	}

#ifdef BAND_STEERING
		if ((pAd->ApCfg.BandSteering)
		) {
			BndStrg_UpdateEntry(pAd, pEntry, ie_list, TRUE);
		}
#endif


#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		if ((pEntry->CapabilityInfo & 0x0100) != 0)/*STA support TPC*/
			wifi_transmit_tpcreq(pAd, pEntry, wdev);
	}
#endif
#endif
LabelOK:
#ifdef RT_CFG80211_SUPPORT

	if ((StatusCode != MLME_SUCCESS) && (StatusCode != MLME_ASSOC_REJ_TEMPORARILY))
		CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, pEntry->wdev->if_dev);

#endif /* RT_CFG80211_SUPPORT */

assoc_check:
#ifdef WAPP_SUPPORT
	if (StatusCode != MLME_SUCCESS && wapp_assoc_fail != NOT_FAILURE)
		wapp_send_sta_connect_rejected(pAd, wdev, ie_list->Addr2,
			ie_list->Addr1, wapp_cnnct_stage, wapp_assoc_fail, StatusCode, 0);
#endif /* WAPP_SUPPORT */

#ifdef OCE_FILS_SUPPORT
free_check:
#endif /* OCE_FILS_SUPPORT */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/* fix memory leak when trigger scan continuously*/
	if (ie_list && ie_list->CustomerVendorIE.pointer)
		os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	if (ie_list != NULL)
		os_free_mem(ie_list);
#ifdef DOT11R_FT_SUPPORT
	if (rsnxe_ie != NULL)
		os_free_mem(rsnxe_ie);

	if (FtInfoBuf != NULL)
		os_free_mem(FtInfoBuf);
#endif
	if (pOutBuffer != NULL)
		MlmeFreeMemory((PVOID) pOutBuffer);
	return;
}



/*
    ==========================================================================
    Description:
	peer assoc req handling procedure
    Parameters:
	Adapter - Adapter pointer
	Elem - MLME Queue Element
    Pre:
	the station has been authenticated and the following information is stored
    Post  :
	-# An association response frame is generated and sent to the air
    ==========================================================================
 */
static VOID ap_peer_assoc_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	ap_cmm_peer_assoc_req_action(pAd, Elem, 0);
}

/*
    ==========================================================================
    Description:
	mlme reassoc req handling procedure
    Parameters:
	Elem -
    Pre:
	-# SSID  (Adapter->ApCfg.ssid[])
	-# BSSID (AP address, Adapter->ApCfg.bssid)
	-# Supported rates (Adapter->ApCfg.supported_rates[])
	-# Supported rates length (Adapter->ApCfg.supported_rates_len)
	-# Tx power (Adapter->ApCfg.tx_power)
    ==========================================================================
 */
static VOID ap_peer_reassoc_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	ap_cmm_peer_assoc_req_action(pAd, Elem, 1);
}

/*
    ==========================================================================
    Description:
	left part of IEEE 802.11/1999 p.374
    Parameters:
	Elem - MLME message containing the received frame
    ==========================================================================
 */
static VOID ap_peer_disassoc_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Reason;
	UINT16 SeqNum;
	MAC_TABLE_ENTRY *pEntry;
	struct wifi_dev *wdev;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "ASSOC - 1 receive DIS-ASSOC request\n");

	if (!PeerDisassocReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, &SeqNum, &Reason))
		return;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			  "ASSOC - receive DIS-ASSOC(seq-%d) request from "MACSTR", reason=%d\n",
			  SeqNum, MAC2STR(Addr2), Reason);
	pEntry = MacTableLookup(pAd, Addr2);

	if (pEntry == NULL) {
#ifdef WTBL_TDD_SUPPORT
		if (IS_WTBL_TDD_ENABLED(pAd)) {
			pEntry = WtblTdd_InactiveList_Lookup(pAd, Addr2);
			if (pEntry)
				Elem->Wcid = pEntry->wcid;
		} else
#endif /* WTBL_TDD_SUPPORT */
			return;
	}
#ifdef DOT11W_PMF_SUPPORT
	{
		PMF_CFG *pPmfCfg = NULL;
		FRAME_802_11 *Fr = (PFRAME_802_11)Elem->Msg;

		pPmfCfg = &pEntry->SecConfig.PmfCfg;

		if (pPmfCfg->UsePMFConnect == TRUE && Fr->Hdr.FC.Wep == 0) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "drop due to unprotect disassoc frame\n");
			return;
		}
	}
#endif

	if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
		wdev = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev;

		/*
			iPhone sometimes sends disassoc frame which DA is old AP and BSSID is new AP.
			@2016/1/26
		*/
		if (!MAC_ADDR_EQUAL(wdev->if_addr, Addr1)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "ASSOC - The DA of this DIS-ASSOC request is "MACSTR", ignore.\n",
					  MAC2STR(Addr1));
			return;
		}

#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

		/* Notify 802.1x daemon to clear this sta info */
		if (IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(wdev))
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
		/* send wireless event - for disassociation */
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, Addr2, 0, 0);
		ApLogEvent(pAd, Addr2, EVENT_DISASSOCIATED);
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_DEAUTH, REASON_DEAUTH_STA_LEAVING);
#endif
#ifdef CONN_FAIL_EVENT
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				Reason);
#endif

		if (IS_ENTRY_CLIENT(pEntry)) {
#ifdef MAP_R2
			if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
				pEntry->DisconnectReason = Reason;
		/*	// TODO: if the port secured is not true, then send failed assoc.*/
#endif
		}
		MacTableDeleteEntry(pAd, Elem->Wcid, Addr2);

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
		if (IS_MAP_ENABLE(pAd) && (pAd->CommonCfg.SRMode[HcGetBandByWdev(wdev)] == TRUE))
			SrMeshSrUpdateSTAMode(pAd, wdev, FALSE, FALSE);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) */

#ifdef WH_EVENT_NOTIFIER
		{
			EventHdlr pEventHdlrHook = NULL;

			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);

			if (pEventHdlrHook && wdev)
				pEventHdlrHook(pAd, wdev, Addr2, Elem->Channel);
		}
#endif /* WH_EVENT_NOTIFIER */
	}
}

/*
    ==========================================================================
    Description:
	Upper layer orders to disassoc s STA
    Parameters:
	Elem -
    ==========================================================================
 */
static VOID ap_mlme_disassoc_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MLME_DISCONNECT_STRUCT *DisassocReq = (MLME_DISCONNECT_STRUCT *)(Elem->Msg); /* snowpin for cntl mgmt */

	APMlmeKickOutSta(pAd, DisassocReq->addr, Elem->Wcid, DisassocReq->reason);
}


/*
    ==========================================================================
    Description:
	association state machine init, including state transition and timer init
    Parameters:
	S - pointer to the association state machine
    Note:
	The state machine looks like the following

				    AP_ASSOC_IDLE
	APMT2_MLME_DISASSOC_REQ    mlme_disassoc_req_action
	APMT2_PEER_DISASSOC_REQ    peer_disassoc_action
	APMT2_PEER_ASSOC_REQ       drop
	APMT2_PEER_REASSOC_REQ     drop
	APMT2_CLS3ERR              cls3err_action
    ==========================================================================
 */
VOID ap_assoc_init(struct wifi_dev *wdev)
{
	ap_assoc_api.peer_assoc_req_action = ap_peer_assoc_req_action;
	ap_assoc_api.peer_reassoc_req_action = ap_peer_reassoc_req_action;
	ap_assoc_api.mlme_disassoc_req_action = ap_mlme_disassoc_req_action;
	ap_assoc_api.peer_disassoc_action =     ap_peer_disassoc_action;
	wdev->assoc_api = &ap_assoc_api;
}

#ifdef BW_VENDOR10_CUSTOM_FEATURE
BOOLEAN IsClientConnected(RTMP_ADAPTER *pAd)
{
	INT i;
	PMAC_TABLE_ENTRY pEntry;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			return TRUE;
	}

	return FALSE;
}
#endif

/*
    ==========================================================================
    Description:
	delete it from STA and disassoc s STA
    Parameters:
	Elem -
    ==========================================================================
 */
VOID MbssKickOutStas(RTMP_ADAPTER *pAd, INT apidx, USHORT Reason)
{
	INT i;
	PMAC_TABLE_ENTRY pEntry;
	struct wifi_dev *wdev = NULL;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev))
		MboIndicateStaDisassocToDaemon(pAd, NULL, MBO_MSG_AP_TERMINATION);
#endif /* MBO_SUPPORT */

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->func_tb_idx == apidx)
			APMlmeKickOutSta(pAd, pEntry->Addr, pEntry->wcid, Reason);
	}

	/* mbo wait for max 500ms and check all IsKeep is zero */
#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev))
		MboWaitAllStaGone(pAd, apidx);
#endif /* MBO_SUPPORT */

}


/*
    ==========================================================================
    Description:
	delete it from STA and disassoc s STA
    Parameters:
	Elem -
    ==========================================================================
 */
VOID APMlmeKickOutSta(RTMP_ADAPTER *pAd, UCHAR *pStaAddr, UINT16 Wcid, USHORT Reason)
{
	HEADER_802_11 DisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	MAC_TABLE_ENTRY *pEntry;
	UINT16 Aid;
	UCHAR ApIdx;
	struct wifi_dev *wdev;

	pEntry = MacTableLookup(pAd, pStaAddr);

	if (pEntry == NULL)
		return;

	Aid = pEntry->Aid;
	ApIdx = pEntry->func_tb_idx;

	if (ApIdx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid Apidx=%d\n",
				 ApIdx);
		return;
	}

	if (VALID_UCAST_ENTRY_WCID(pAd, Wcid)) {
		/* send wireless event - for disassocation */
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, pStaAddr, 0, 0);
		ApLogEvent(pAd, pStaAddr, EVENT_DISASSOCIATED);
		/* 2. send out a DISASSOC request frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

#ifdef MAP_R2
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
			wapp_handle_sta_disassoc(pAd, Wcid, Reason);
#endif

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				  "ASSOC - MLME disassociates "MACSTR"; Send DISASSOC request\n",
				  MAC2STR(pStaAddr));
		MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pStaAddr,
						 pAd->ApCfg.MBSSID[ApIdx].wdev.if_addr,
						 pAd->ApCfg.MBSSID[ApIdx].wdev.bssid);
		MakeOutgoingFrame(pOutBuffer,            &FrameLen,
						  sizeof(HEADER_802_11), &DisassocHdr,
						  2,                     &Reason,
						  END_OF_ARGS);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				Reason);
#endif
		MacTableDeleteEntry(pAd, Wcid, pStaAddr);

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
		wdev = &pAd->ApCfg.MBSSID[ApIdx].wdev;

		if (IS_MAP_ENABLE(pAd) && (pAd->CommonCfg.SRMode[HcGetBandByWdev(wdev)] == TRUE))
			SrMeshSrUpdateSTAMode(pAd, wdev, FALSE, FALSE);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT) */
	}
}


#ifdef DOT11W_PMF_SUPPORT
VOID APMlmeKickOutAllSta(RTMP_ADAPTER *pAd, UCHAR apidx, USHORT Reason)
{
	HEADER_802_11 DisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS     NStatus;
	UCHAR           BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	PPMF_CFG        pPmfCfg = NULL;

	pPmfCfg = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg;

	if ((apidx < pAd->ApCfg.BssidNum) && (pPmfCfg)) {
		/* Send out a Deauthentication request frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Send DISASSOC Broadcast frame(%d) with ra%d\n", Reason, apidx);
		/* 802.11 Header */
		NdisZeroMemory(&DisassocHdr, sizeof(HEADER_802_11));
		DisassocHdr.FC.Type = FC_TYPE_MGMT;
		DisassocHdr.FC.SubType = SUBTYPE_DISASSOC;
		DisassocHdr.FC.ToDs = 0;
		DisassocHdr.FC.Wep = 0;
		COPY_MAC_ADDR(DisassocHdr.Addr1, BROADCAST_ADDR);
		COPY_MAC_ADDR(DisassocHdr.Addr2, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
		COPY_MAC_ADDR(DisassocHdr.Addr3, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &DisassocHdr,
						  2, &Reason,
						  END_OF_ARGS);

		if (pPmfCfg->MFPC == TRUE) {
			ULONG TmpLen;
			UCHAR res_buf[LEN_PMF_MMIE];
			UCHAR EID, ELen;

			EID = IE_PMF_MMIE;
			ELen = LEN_PMF_MMIE;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1, &EID,
							  1, &ELen,
							  LEN_PMF_MMIE, res_buf,
							  END_OF_ARGS);
			FrameLen += TmpLen;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 "[PMF]: This is a Broadcast Robust management frame, Add 0x4C(76) EID\n");
		}

		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
	}
}
#endif /* DOT11W_PMF_PLUGFEST */



